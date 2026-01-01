/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 RegHook.cpp - registry hook functions
 */

#include "stdafx.h"

#include "ESFTPFld.h"
#include "../ShellDLL/EasySFTP_h.h"
#include "Array.h"
#include "UString.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ImageDirectoryEntryToData
#pragma comment(lib, "Dbghelp.lib")

// Toolhelp snapshot functions (for Win95/2000/XP or later)
typedef HANDLE (WINAPI* T_CreateToolhelp32Snapshot)(DWORD dwFlags, DWORD th32ProcessID);
typedef BOOL (WINAPI* T_Module32First)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);
typedef BOOL (WINAPI* T_Module32Next)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);
// psapi functions (psapi; for WinNT 4.0)
typedef BOOL (WINAPI* T_EnumProcessModules)(HANDLE hProcess, HMODULE* lphModule, DWORD cb, LPDWORD lpcbNeeded);

typedef bool (CALLBACK* PFNENUMMODULECALLBACK)(HMODULE hModule);

static HRESULT __stdcall MyEnumModulesUsingSnapshot(T_CreateToolhelp32Snapshot pfnCreateToolhelp32Snapshot,
	T_Module32First pfnModule32First, T_Module32Next pfnModule32Next, PFNENUMMODULECALLBACK pfnCallback)
{
	HANDLE hSnapshot;
	MODULEENTRY32 me;

	hSnapshot = pfnCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return HRESULT_FROM_WIN32(GetLastError());

	me.dwSize = sizeof(me);
	BOOL bRet = pfnModule32First(hSnapshot, &me);
	while (bRet)
	{        
		if (!pfnCallback(me.hModule))
			break;
		bRet = pfnModule32Next(hSnapshot, &me);
	}
	CloseHandle(hSnapshot);
	return S_OK;
}

static HRESULT __stdcall MyEnumModulesUsingPsapi(T_EnumProcessModules pfnEnumProcessModules, PFNENUMMODULECALLBACK pfnCallback)
{
	DWORD dwCount, dwRet;
	HMODULE* pModules;

	dwCount = 32;
	pModules = (HMODULE*) malloc(sizeof(HMODULE) * dwCount);
	if (!pModules)
		return E_OUTOFMEMORY;

	while (true)
	{
		if (!pfnEnumProcessModules(::GetCurrentProcess(), pModules, sizeof(HMODULE) * dwCount, &dwRet))
		{
			auto err = GetLastError();
			free(pModules);
			return HRESULT_FROM_WIN32(err);
		}
		dwRet /= sizeof(HMODULE);
		if (dwCount < dwRet)
		{
			dwCount = dwRet;
			HMODULE* pModules2 = (HMODULE*) realloc(pModules, sizeof(HMODULE) * dwCount);
			if (!pModules2)
			{
				free(pModules);
				return E_OUTOFMEMORY;
			}
			pModules = pModules2;
		}
		else
		{
			dwCount = dwRet;
			break;
		}
	}
	for (dwRet = 0; dwRet < dwCount; dwRet++)
	{
		if (!pfnCallback(pModules[dwRet]))
			break;
	}
	free(pModules);
	return S_OK;
}

#define SIZE_OF_NT_SIGNATURE       (sizeof(DWORD))
#define RVATOVA(base, offset) ( \
	(LPVOID)((DWORD_PTR)(base) + (DWORD_PTR)(offset)))
// オプションヘッダオフセット
#define OPTHDROFFSET(ptr) ( \
	(LPVOID)((PBYTE)(ptr) + \
	((PIMAGE_DOS_HEADER)(ptr))->e_lfanew + \
	SIZE_OF_NT_SIGNATURE +  \
	sizeof(IMAGE_FILE_HEADER)))

// DLLのインスタンスハンドルと関数名から関数を取り出す
static FARPROC __stdcall GetDLLProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	// hModuleがNULLならばエラー
	if (hModule == NULL)
		return NULL;
	
	// ディレクトリカウント取得
	PIMAGE_OPTIONAL_HEADER poh = (PIMAGE_OPTIONAL_HEADER) OPTHDROFFSET(hModule);
	int nDirCount = poh->NumberOfRvaAndSizes;
	if (nDirCount < 16)
		return FALSE;

	// エクスポートディレクトリテーブル取得
	DWORD dwIDEE = IMAGE_DIRECTORY_ENTRY_EXPORT;
	if (poh->DataDirectory[dwIDEE].Size == 0)
		return NULL;
	DWORD dwAddr = poh->DataDirectory[dwIDEE].VirtualAddress;
	PIMAGE_EXPORT_DIRECTORY ped = 
		(PIMAGE_EXPORT_DIRECTORY) RVATOVA(hModule, dwAddr);	

	// 序数取得
	int nOrdinal = (LOWORD(lpProcName)) - ped->Base;
	
	if (HIWORD(lpProcName) != 0)
	{
		int count = ped->NumberOfNames;
		// 名前と序数を取得
		DWORD *pdwNamePtr = (PDWORD)
			RVATOVA(hModule, ped->AddressOfNames);
		WORD *pwOrdinalPtr = (PWORD)
			RVATOVA(hModule, ped->AddressOfNameOrdinals);
		// 関数検索
		int i;
		for (i=0; i < count; i++, pdwNamePtr++, pwOrdinalPtr++){
			PTCHAR svName = (PTCHAR) RVATOVA(hModule, *pdwNamePtr);
			if (strcmp(svName, lpProcName) == 0){
				nOrdinal = *pwOrdinalPtr;
				break;
			}
		}
		// 見つからなければNULLを返却
		if (i == count)
			return NULL;
	}
	
	// 発見した関数を返す
	PDWORD pAddrTable = (PDWORD)
		RVATOVA(hModule, ped->AddressOfFunctions);
	return (FARPROC) RVATOVA(hModule, pAddrTable[nOrdinal]);
}

// ひとつのモジュールに対してAPIフックを行う関数
static void __stdcall ReplaceIATEntryInOneMod(PCSTR pszModuleName, FARPROC pfnCurrent, FARPROC pfnNew, HMODULE hModCaller) 
{
	ULONG ulSize;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
	pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR) ImageDirectoryEntryToData(
		hModCaller, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ulSize);

	if (pImportDesc == NULL)
		return;

	while (pImportDesc->Name)
	{
		PSTR pszModName = (PSTR) ((PBYTE) hModCaller + pImportDesc->Name);
		if (_stricmp(pszModName, pszModuleName) == 0) 
			break;
		pImportDesc++;
	}

	if (pImportDesc->Name == 0)
		return;

	PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA) 
		((PBYTE) hModCaller + pImportDesc->FirstThunk);

	while (pThunk->u1.Function)
	{
		FARPROC* ppfn = (FARPROC*) &pThunk->u1.Function;
		bool fFound = (*ppfn == pfnCurrent);
		if (fFound)
		{
			DWORD dwDummy;
			VirtualProtect(ppfn, sizeof(ppfn), PAGE_EXECUTE_READWRITE, &dwDummy);
			WriteProcessMemory(GetCurrentProcess(), ppfn, &pfnNew, sizeof(pfnNew), NULL);
			VirtualProtect(ppfn, sizeof(ppfn), dwDummy, &dwDummy);
			return;
		}
		pThunk++;
	}
}

////////////////////////////////////////////////////////////////////////////////

#define IS_PREDEFINED_HKEY(hKey) \
	((hKey) == HKEY_CLASSES_ROOT || \
	(hKey) == HKEY_CURRENT_USER || \
	(hKey) == HKEY_LOCAL_MACHINE || \
	(hKey) == HKEY_USERS)

LSTATUS APIENTRY MyHookRegCloseKey(HKEY hKey);
LSTATUS APIENTRY MyHookRegCreateKeyA(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult);
LSTATUS APIENTRY MyHookRegCreateKeyW(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult);
LSTATUS APIENTRY MyHookRegCreateKeyExA(HKEY hKey,  LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass,
	DWORD dwOptions, REGSAM samDesired, CONST LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition);
LSTATUS APIENTRY MyHookRegCreateKeyExW(HKEY hKey,  LPCWSTR lpSubKey, DWORD Reserved, LPWSTR lpClass,
	DWORD dwOptions, REGSAM samDesired, CONST LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition);
LSTATUS APIENTRY MyHookRegDeleteKeyA(HKEY hKey, LPCSTR lpSubKey);
LSTATUS APIENTRY MyHookRegDeleteKeyW(HKEY hKey, LPCWSTR lpSubKey);
LSTATUS APIENTRY MyHookRegDeleteKeyExA(HKEY hKey, LPCSTR lpSubKey, REGSAM samDesired, DWORD Reserved);
LSTATUS APIENTRY MyHookRegDeleteKeyExW(HKEY hKey, LPCWSTR lpSubKey, REGSAM samDesired, DWORD Reserved);
LSTATUS APIENTRY MyHookRegEnumKeyA(HKEY hKey, DWORD dwIndex, LPSTR lpName, DWORD cchName);
LSTATUS APIENTRY MyHookRegEnumKeyW(HKEY hKey, DWORD dwIndex, LPWSTR lpName, DWORD cchName);
LSTATUS APIENTRY MyHookRegEnumKeyExA(HKEY hKey, DWORD dwIndex, LPSTR lpName, LPDWORD lpcchName,
	LPDWORD lpReserved, LPSTR lpClass, LPDWORD lpcchClass, PFILETIME lpftLastWriteTime);
LSTATUS APIENTRY MyHookRegEnumKeyExW(HKEY hKey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcchName,
	LPDWORD lpReserved, LPWSTR lpClass, LPDWORD lpcchClass, PFILETIME lpftLastWriteTime);
LSTATUS APIENTRY MyHookRegEnumValueA(HKEY hKey, DWORD dwIndex, LPSTR lpValueName, LPDWORD lpcchValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
LSTATUS APIENTRY MyHookRegEnumValueW(HKEY hKey, DWORD dwIndex, LPWSTR lpValueName, LPDWORD lpcchValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
LSTATUS APIENTRY MyHookRegOpenKeyA(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult);
LSTATUS APIENTRY MyHookRegOpenKeyW(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult);
LSTATUS APIENTRY MyHookRegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
LSTATUS APIENTRY MyHookRegOpenKeyExW(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
LSTATUS APIENTRY MyHookRegOpenUserClassesRoot(HANDLE hToken, DWORD dwOptions, REGSAM samDesired, PHKEY phkResult);
LSTATUS APIENTRY MyHookRegQueryInfoKeyA(HKEY hKey, LPSTR lpClass, LPDWORD lpcchClass,
	LPDWORD lpReserved, LPDWORD lpcSubKeys, LPDWORD lpcbMaxSubKeyLen, LPDWORD lpcbMaxClassLen, LPDWORD lpcValues,
	LPDWORD lpcbMaxValueNameLen, LPDWORD lpcbMaxValueLen, LPDWORD lpcbSecurityDescriptor, PFILETIME lpftLastWriteTime);
LSTATUS APIENTRY MyHookRegQueryInfoKeyW(HKEY hKey, LPWSTR lpClass, LPDWORD lpcchClass,
	LPDWORD lpReserved, LPDWORD lpcSubKeys, LPDWORD lpcbMaxSubKeyLen, LPDWORD lpcbMaxClassLen, LPDWORD lpcValues,
	LPDWORD lpcbMaxValueNameLen, LPDWORD lpcbMaxValueLen, LPDWORD lpcbSecurityDescriptor, PFILETIME lpftLastWriteTime);
LSTATUS APIENTRY MyHookRegQueryValueA(HKEY hKey, LPCSTR lpSubKey, LPSTR lpData, PLONG lpcbData);
LSTATUS APIENTRY MyHookRegQueryValueW(HKEY hKey, LPCWSTR lpSubKey, LPWSTR lpData, PLONG lpcbData);
LSTATUS APIENTRY MyHookRegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
LSTATUS APIENTRY MyHookRegQueryValueExW(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
LSTATUS APIENTRY MyHookRegGetValueA(HKEY hKey, LPCSTR lpSubKey, LPCSTR lpValue, DWORD dwFlags, LPDWORD pdwType, PVOID pvData, LPDWORD pcbData);
LSTATUS APIENTRY MyHookRegGetValueW(HKEY hKey, LPCWSTR lpSubKey, LPCWSTR lpValue, DWORD dwFlags, LPDWORD pdwType, PVOID pvData, LPDWORD pcbData);
LSTATUS APIENTRY MyHookRegSetValueA(HKEY hKey, LPCSTR lpSubKey, DWORD dwType, LPCSTR lpData, DWORD cbData);
LSTATUS APIENTRY MyHookRegSetValueW(HKEY hKey, LPCWSTR lpSubKey, DWORD dwType, LPCWSTR lpData, DWORD cbData);
LSTATUS APIENTRY MyHookRegSetValueExA(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, CONST BYTE* lpData, DWORD cbData);
LSTATUS APIENTRY MyHookRegSetValueExW(HKEY hKey, LPCWSTR lpValueName, DWORD Reserved, DWORD dwType, CONST BYTE* lpData, DWORD cbData);
LSTATUS APIENTRY MyHookRegSetKeyValueA(HKEY hKey, LPCSTR lpSubKey, LPCSTR lpValueName, DWORD dwType, LPCVOID lpData, DWORD cbData);
LSTATUS APIENTRY MyHookRegSetKeyValueW(HKEY hKey, LPCWSTR lpSubKey, LPCWSTR lpValueName, DWORD dwType, LPCVOID lpData, DWORD cbData);

typedef LSTATUS (APIENTRY* T_RegCloseKey)(HKEY hKey);
typedef LSTATUS (APIENTRY* T_RegCreateKeyA)(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult);
typedef LSTATUS (APIENTRY* T_RegCreateKeyW)(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult);
typedef LSTATUS (APIENTRY* T_RegCreateKeyExA)(HKEY hKey,  LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass,
	DWORD dwOptions, REGSAM samDesired, CONST LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition);
typedef LSTATUS (APIENTRY* T_RegCreateKeyExW)(HKEY hKey,  LPCWSTR lpSubKey, DWORD Reserved, LPWSTR lpClass,
	DWORD dwOptions, REGSAM samDesired, CONST LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition);
typedef LSTATUS (APIENTRY* T_RegDeleteKeyA)(HKEY hKey, LPCSTR lpSubKey);
typedef LSTATUS (APIENTRY* T_RegDeleteKeyW)(HKEY hKey, LPCWSTR lpSubKey);
typedef LSTATUS (APIENTRY* T_RegDeleteKeyExA)(HKEY hKey, LPCSTR lpSubKey, REGSAM samDesired, DWORD Reserved);
typedef LSTATUS (APIENTRY* T_RegDeleteKeyExW)(HKEY hKey, LPCWSTR lpSubKey, REGSAM samDesired, DWORD Reserved);
typedef LSTATUS (APIENTRY* T_RegEnumKeyA)(HKEY hKey, DWORD dwIndex, LPSTR lpName, DWORD cchName);
typedef LSTATUS (APIENTRY* T_RegEnumKeyW)(HKEY hKey, DWORD dwIndex, LPWSTR lpName, DWORD cchName);
typedef LSTATUS (APIENTRY* T_RegEnumKeyExA)(HKEY hKey, DWORD dwIndex, LPSTR lpName, LPDWORD lpcchName,
	LPDWORD lpReserved, LPSTR lpClass, LPDWORD lpcchClass, PFILETIME lpftLastWriteTime);
typedef LSTATUS (APIENTRY* T_RegEnumKeyExW)(HKEY hKey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcchName,
	LPDWORD lpReserved, LPWSTR lpClass, LPDWORD lpcchClass, PFILETIME lpftLastWriteTime);
typedef LSTATUS (APIENTRY* T_RegEnumValueA)(HKEY hKey, DWORD dwIndex, LPSTR lpValueName, LPDWORD lpcchValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
typedef LSTATUS (APIENTRY* T_RegEnumValueW)(HKEY hKey, DWORD dwIndex, LPWSTR lpValueName, LPDWORD lpcchValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
typedef LSTATUS (APIENTRY* T_RegOpenKeyA)(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult);
typedef LSTATUS (APIENTRY* T_RegOpenKeyW)(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult);
typedef LSTATUS (APIENTRY* T_RegOpenKeyExA)(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
typedef LSTATUS (APIENTRY* T_RegOpenKeyExW)(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
typedef LSTATUS (APIENTRY* T_RegOpenUserClassesRoot)(HANDLE hToken, DWORD dwOptions, REGSAM samDesired, PHKEY phkResult);
typedef LSTATUS (APIENTRY* T_RegQueryInfoKeyA)(HKEY hKey, LPSTR lpClass, LPDWORD lpcchClass,
	LPDWORD lpReserved, LPDWORD lpcSubKeys, LPDWORD lpcbMaxSubKeyLen, LPDWORD lpcbMaxClassLen, LPDWORD lpcValues,
	LPDWORD lpcbMaxValueNameLen, LPDWORD lpcbMaxValueLen, LPDWORD lpcbSecurityDescriptor, PFILETIME lpftLastWriteTime);
typedef LSTATUS (APIENTRY* T_RegQueryInfoKeyW)(HKEY hKey, LPWSTR lpClass, LPDWORD lpcchClass,
	LPDWORD lpReserved, LPDWORD lpcSubKeys, LPDWORD lpcbMaxSubKeyLen, LPDWORD lpcbMaxClassLen, LPDWORD lpcValues,
	LPDWORD lpcbMaxValueNameLen, LPDWORD lpcbMaxValueLen, LPDWORD lpcbSecurityDescriptor, PFILETIME lpftLastWriteTime);
typedef LSTATUS (APIENTRY* T_RegQueryValueA)(HKEY hKey, LPCSTR lpSubKey, LPSTR lpData, PLONG lpcbData);
typedef LSTATUS (APIENTRY* T_RegQueryValueW)(HKEY hKey, LPCWSTR lpSubKey, LPWSTR lpData, PLONG lpcbData);
typedef LSTATUS (APIENTRY* T_RegQueryValueExA)(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
typedef LSTATUS (APIENTRY* T_RegQueryValueExW)(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
typedef LSTATUS (APIENTRY* T_RegGetValueA)(HKEY hKey, LPCSTR lpSubKey, LPCSTR lpValue, DWORD dwFlags, LPDWORD pdwType, PVOID pvData, LPDWORD pcbData);
typedef LSTATUS (APIENTRY* T_RegGetValueW)(HKEY hKey, LPCWSTR lpSubKey, LPCWSTR lpValue, DWORD dwFlags, LPDWORD pdwType, PVOID pvData, LPDWORD pcbData);
typedef LSTATUS (APIENTRY* T_RegSetValueA)(HKEY hKey, LPCSTR lpSubKey, DWORD dwType, LPCSTR lpData, DWORD cbData);
typedef LSTATUS (APIENTRY* T_RegSetValueW)(HKEY hKey, LPCWSTR lpSubKey, DWORD dwType, LPCWSTR lpData, DWORD cbData);
typedef LSTATUS (APIENTRY* T_RegSetValueExA)(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, CONST BYTE* lpData, DWORD cbData);
typedef LSTATUS (APIENTRY* T_RegSetValueExW)(HKEY hKey, LPCWSTR lpValueName, DWORD Reserved, DWORD dwType, CONST BYTE* lpData, DWORD cbData);
typedef LSTATUS (APIENTRY* T_RegSetKeyValueA)(HKEY hKey, LPCSTR lpSubKey, LPCSTR lpValueName, DWORD dwType, LPCVOID lpData, DWORD cbData);
typedef LSTATUS (APIENTRY* T_RegSetKeyValueW)(HKEY hKey, LPCWSTR lpSubKey, LPCWSTR lpValueName, DWORD dwType, LPCVOID lpData, DWORD cbData);

static T_RegCloseKey s_pfnRegCloseKey = NULL;
static T_RegCreateKeyA s_pfnRegCreateKeyA = NULL;
static T_RegCreateKeyW s_pfnRegCreateKeyW = NULL;
static T_RegCreateKeyExA s_pfnRegCreateKeyExA = NULL;
static T_RegCreateKeyExW s_pfnRegCreateKeyExW = NULL;
static T_RegDeleteKeyA s_pfnRegDeleteKeyA = NULL;
static T_RegDeleteKeyW s_pfnRegDeleteKeyW = NULL;
static T_RegDeleteKeyExA s_pfnRegDeleteKeyExA = NULL;
static T_RegDeleteKeyExW s_pfnRegDeleteKeyExW = NULL;
static T_RegEnumKeyA s_pfnRegEnumKeyA = NULL;
static T_RegEnumKeyW s_pfnRegEnumKeyW = NULL;
static T_RegEnumKeyExA s_pfnRegEnumKeyExA = NULL;
static T_RegEnumKeyExW s_pfnRegEnumKeyExW = NULL;
static T_RegEnumValueA s_pfnRegEnumValueA = NULL;
static T_RegEnumValueW s_pfnRegEnumValueW = NULL;
static T_RegOpenKeyA s_pfnRegOpenKeyA = NULL;
static T_RegOpenKeyW s_pfnRegOpenKeyW = NULL;
static T_RegOpenKeyExA s_pfnRegOpenKeyExA = NULL;
static T_RegOpenKeyExW s_pfnRegOpenKeyExW = NULL;
static T_RegOpenUserClassesRoot s_pfnRegOpenUserClassesRoot = NULL;
static T_RegQueryInfoKeyA s_pfnRegQueryInfoKeyA = NULL;
static T_RegQueryInfoKeyW s_pfnRegQueryInfoKeyW = NULL;
static T_RegQueryValueA s_pfnRegQueryValueA = NULL;
static T_RegQueryValueW s_pfnRegQueryValueW = NULL;
static T_RegQueryValueExA s_pfnRegQueryValueExA = NULL;
static T_RegQueryValueExW s_pfnRegQueryValueExW = NULL;
static T_RegGetValueA s_pfnRegGetValueA = NULL;
static T_RegGetValueW s_pfnRegGetValueW = NULL;
static T_RegSetValueA s_pfnRegSetValueA = NULL;
static T_RegSetValueW s_pfnRegSetValueW = NULL;
static T_RegSetValueExA s_pfnRegSetValueExA = NULL;
static T_RegSetValueExW s_pfnRegSetValueExW = NULL;
static T_RegSetKeyValueA s_pfnRegSetKeyValueA = NULL;
static T_RegSetKeyValueW s_pfnRegSetKeyValueW = NULL;
static LPCSTR s_pszAdvapi32_name = NULL;
static bool s_bUnicodeSupported = false;

static bool CALLBACK MyReplaceRegFunctions(HMODULE hModTarget)
{
	if (s_pfnRegCloseKey)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegCloseKey, (FARPROC) MyHookRegCloseKey, hModTarget);
	if (s_pfnRegCreateKeyA)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegCreateKeyA, (FARPROC) MyHookRegCreateKeyA, hModTarget);
	if (s_pfnRegCreateKeyW)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegCreateKeyW, (FARPROC) MyHookRegCreateKeyW, hModTarget);
	if (s_pfnRegCreateKeyExA)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegCreateKeyExA, (FARPROC) MyHookRegCreateKeyExA, hModTarget);
	if (s_pfnRegCreateKeyExW)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegCreateKeyExW, (FARPROC) MyHookRegCreateKeyExW, hModTarget);
	if (s_pfnRegDeleteKeyA)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegDeleteKeyA, (FARPROC) MyHookRegDeleteKeyA, hModTarget);
	if (s_pfnRegDeleteKeyW)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegDeleteKeyW, (FARPROC) MyHookRegDeleteKeyW, hModTarget);
	if (s_pfnRegDeleteKeyExA)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegDeleteKeyExA, (FARPROC) MyHookRegDeleteKeyExA, hModTarget);
	if (s_pfnRegDeleteKeyExW)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegDeleteKeyExW, (FARPROC) MyHookRegDeleteKeyExW, hModTarget);
	if (s_pfnRegEnumKeyA)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegEnumKeyA, (FARPROC) MyHookRegEnumKeyA, hModTarget);
	if (s_pfnRegEnumKeyW)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegEnumKeyW, (FARPROC) MyHookRegEnumKeyW, hModTarget);
	if (s_pfnRegEnumKeyExA)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegEnumKeyExA, (FARPROC) MyHookRegEnumKeyExA, hModTarget);
	if (s_pfnRegEnumKeyExW)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegEnumKeyExW, (FARPROC) MyHookRegEnumKeyExW, hModTarget);
	if (s_pfnRegEnumValueA)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegEnumValueA, (FARPROC) MyHookRegEnumValueA, hModTarget);
	if (s_pfnRegEnumValueW)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegEnumValueW, (FARPROC) MyHookRegEnumValueW, hModTarget);
	if (s_pfnRegOpenKeyA)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegOpenKeyA, (FARPROC) MyHookRegOpenKeyA, hModTarget);
	if (s_pfnRegOpenKeyW)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegOpenKeyW, (FARPROC) MyHookRegOpenKeyW, hModTarget);
	if (s_pfnRegOpenKeyExA)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegOpenKeyExA, (FARPROC) MyHookRegOpenKeyExA, hModTarget);
	if (s_pfnRegOpenKeyExW)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegOpenKeyExW, (FARPROC) MyHookRegOpenKeyExW, hModTarget);
	if (s_pfnRegOpenUserClassesRoot)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegOpenUserClassesRoot, (FARPROC) MyHookRegOpenUserClassesRoot, hModTarget);
	if (s_pfnRegQueryInfoKeyA)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegQueryInfoKeyA, (FARPROC) MyHookRegQueryInfoKeyA, hModTarget);
	if (s_pfnRegQueryInfoKeyW)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegQueryInfoKeyW, (FARPROC) MyHookRegQueryInfoKeyW, hModTarget);
	if (s_pfnRegQueryValueA)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegQueryValueA, (FARPROC) MyHookRegQueryValueA, hModTarget);
	if (s_pfnRegQueryValueW)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegQueryValueW, (FARPROC) MyHookRegQueryValueW, hModTarget);
	if (s_pfnRegQueryValueExA)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegQueryValueExA, (FARPROC) MyHookRegQueryValueExA, hModTarget);
	if (s_pfnRegQueryValueExW)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegQueryValueExW, (FARPROC) MyHookRegQueryValueExW, hModTarget);
	if (s_pfnRegGetValueA)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegGetValueA, (FARPROC) MyHookRegGetValueA, hModTarget);
	if (s_pfnRegGetValueW)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegGetValueW, (FARPROC) MyHookRegGetValueW, hModTarget);
	if (s_pfnRegSetValueA)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegSetValueA, (FARPROC) MyHookRegSetValueA, hModTarget);
	if (s_pfnRegSetValueW)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegSetValueW, (FARPROC) MyHookRegSetValueW, hModTarget);
	if (s_pfnRegSetValueExA)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegSetValueExA, (FARPROC) MyHookRegSetValueExA, hModTarget);
	if (s_pfnRegSetValueExW)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegSetValueExW, (FARPROC) MyHookRegSetValueExW, hModTarget);
	if (s_pfnRegSetKeyValueA)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegSetKeyValueA, (FARPROC) MyHookRegSetKeyValueA, hModTarget);
	if (s_pfnRegSetKeyValueW)
		ReplaceIATEntryInOneMod(s_pszAdvapi32_name, (FARPROC) s_pfnRegSetKeyValueW, (FARPROC) MyHookRegSetKeyValueW, hModTarget);
	return true;
}

static bool __stdcall InitOldRegFunctions()
{
	HMODULE hInstAdvapi32;
	//hInstAdvapi32 = ::GetModuleHandle(_T("kernel32.dll"));
	//if (hInstAdvapi32)
	//{
	//	if (!::GetProcAddress(hInstAdvapi32, "RegOpenKeyExW"))
	//		hInstAdvapi32 = NULL;
	//	else
	//		s_pszAdvapi32_name = "kernel32.dll";
	//}
	//if (!hInstAdvapi32)
	hInstAdvapi32 = ::GetModuleHandle(_T("advapi32.dll"));
	if (!hInstAdvapi32)
	{
		if (!::LoadLibrary(_T("advapi32.dll")))
			return false;
	}

	{
		// for Windows 8
		hInstAdvapi32 = ::GetModuleHandle(_T("api-ms-win-core-registry-l1-1-0.dll"));
		if (!hInstAdvapi32)
		{
			// for Windows 7
			hInstAdvapi32 = ::GetModuleHandle(_T("api-ms-win-core-localregistry-l1-1-0.dll"));
			if (!hInstAdvapi32)
			{
				hInstAdvapi32 = ::GetModuleHandle(_T("advapi32.dll"));
				if (!hInstAdvapi32)
					return false;
				else
					s_pszAdvapi32_name = "advapi32.dll";
			}
			else
				s_pszAdvapi32_name = "api-ms-win-core-localregistry-l1-1-0.dll";
		}
		else
			s_pszAdvapi32_name = "api-ms-win-core-registry-l1-1-0.dll";
	}

	// In Windows 8, some registry functions are already hooked by system, and
	// GetProcAddress returns the address of a hook function, not the real function.
	// So we use GetDLLProcAddress to retrieve the address of real functions.
#define GetProcAddress GetDLLProcAddress
	s_pfnRegCloseKey = (T_RegCloseKey) ::GetProcAddress(hInstAdvapi32, "RegCloseKey");
	s_pfnRegCreateKeyA = (T_RegCreateKeyA) ::GetProcAddress(hInstAdvapi32, "RegCreateKeyA");
	s_pfnRegCreateKeyW = (T_RegCreateKeyW) ::GetProcAddress(hInstAdvapi32, "RegCreateKeyW");
	s_pfnRegCreateKeyExA = (T_RegCreateKeyExA) ::GetProcAddress(hInstAdvapi32, "RegCreateKeyExA");
	s_pfnRegCreateKeyExW = (T_RegCreateKeyExW) ::GetProcAddress(hInstAdvapi32, "RegCreateKeyExW");
	s_pfnRegDeleteKeyA = (T_RegDeleteKeyA) ::GetProcAddress(hInstAdvapi32, "RegDeleteKeyA");
	s_pfnRegDeleteKeyW = (T_RegDeleteKeyW) ::GetProcAddress(hInstAdvapi32, "RegDeleteKeyW");
	s_pfnRegDeleteKeyExA = (T_RegDeleteKeyExA) ::GetProcAddress(hInstAdvapi32, "RegDeleteKeyExA");
	s_pfnRegDeleteKeyExW = (T_RegDeleteKeyExW) ::GetProcAddress(hInstAdvapi32, "RegDeleteKeyExW");
	s_pfnRegEnumKeyA = (T_RegEnumKeyA) ::GetProcAddress(hInstAdvapi32, "RegEnumKeyA");
	s_pfnRegEnumKeyW = (T_RegEnumKeyW) ::GetProcAddress(hInstAdvapi32, "RegEnumKeyW");
	s_pfnRegEnumKeyExA = (T_RegEnumKeyExA) ::GetProcAddress(hInstAdvapi32, "RegEnumKeyExA");
	s_pfnRegEnumKeyExW = (T_RegEnumKeyExW) ::GetProcAddress(hInstAdvapi32, "RegEnumKeyExW");
	s_pfnRegEnumValueA = (T_RegEnumValueA) ::GetProcAddress(hInstAdvapi32, "RegEnumValueA");
	s_pfnRegEnumValueW = (T_RegEnumValueW) ::GetProcAddress(hInstAdvapi32, "RegEnumValueW");
	s_pfnRegOpenKeyA = (T_RegOpenKeyA) ::GetProcAddress(hInstAdvapi32, "RegOpenKeyA");
	s_pfnRegOpenKeyW = (T_RegOpenKeyW) ::GetProcAddress(hInstAdvapi32, "RegOpenKeyW");
	s_pfnRegOpenKeyExA = (T_RegOpenKeyExA) ::GetProcAddress(hInstAdvapi32, "RegOpenKeyExA");
	s_pfnRegOpenKeyExW = (T_RegOpenKeyExW) ::GetProcAddress(hInstAdvapi32, "RegOpenKeyExW");
	s_pfnRegOpenUserClassesRoot = (T_RegOpenUserClassesRoot) ::GetProcAddress(hInstAdvapi32, "RegOpenUserClassesRoot");
	s_pfnRegQueryInfoKeyA = (T_RegQueryInfoKeyA) ::GetProcAddress(hInstAdvapi32, "RegQueryInfoKeyA");
	s_pfnRegQueryInfoKeyW = (T_RegQueryInfoKeyW) ::GetProcAddress(hInstAdvapi32, "RegQueryInfoKeyW");
	s_pfnRegQueryValueA = (T_RegQueryValueA) ::GetProcAddress(hInstAdvapi32, "RegQueryValueA");
	s_pfnRegQueryValueW = (T_RegQueryValueW) ::GetProcAddress(hInstAdvapi32, "RegQueryValueW");
	s_pfnRegQueryValueExA = (T_RegQueryValueExA) ::GetProcAddress(hInstAdvapi32, "RegQueryValueExA");
	s_pfnRegQueryValueExW = (T_RegQueryValueExW) ::GetProcAddress(hInstAdvapi32, "RegQueryValueExW");
	s_pfnRegGetValueA = (T_RegGetValueA) ::GetProcAddress(hInstAdvapi32, "RegGetValueA");
	s_pfnRegGetValueW = (T_RegGetValueW) ::GetProcAddress(hInstAdvapi32, "RegGetValueW");
	s_pfnRegSetValueA = (T_RegSetValueA) ::GetProcAddress(hInstAdvapi32, "RegSetValueA");
	s_pfnRegSetValueW = (T_RegSetValueW) ::GetProcAddress(hInstAdvapi32, "RegSetValueW");
	s_pfnRegSetValueExA = (T_RegSetValueExA) ::GetProcAddress(hInstAdvapi32, "RegSetValueExA");
	s_pfnRegSetValueExW = (T_RegSetValueExW) ::GetProcAddress(hInstAdvapi32, "RegSetValueExW");
	s_pfnRegSetKeyValueA = (T_RegSetKeyValueA) ::GetProcAddress(hInstAdvapi32, "RegSetKeyValueA");
	s_pfnRegSetKeyValueW = (T_RegSetKeyValueW) ::GetProcAddress(hInstAdvapi32, "RegSetKeyValueW");
#undef GetProcAddress

	{
		OSVERSIONINFO osvi;
		osvi.dwOSVersionInfoSize = sizeof(osvi);
		::GetVersionEx(&osvi);
		s_bUnicodeSupported = (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT);
	}

	return true;
}

struct CMyHookRegEntry
{
	HKEY hKeyParent;
	LPCWSTR lpszKeyParent;
	bool bIsKey;
	DWORD dwType;
	LPCWSTR lpszKeyName;
	LPCVOID lpszValue;
};

class CMyHookRegKeyData
{
public:
	HKEY hKey;
	bool bIsDummy;
	bool bIsClassesRoot;
	CMyStringW strKeyName;
	HKEY hKeyParent;
	const CMyHookRegEntry* pEntry;
	CMyHookRegKeyData* pParent;

	CMyHookRegKeyData() : m_uRef(1), pParent(NULL) { }
	~CMyHookRegKeyData() { if (pParent) pParent->Release(); }
	ULONG AddRef() { return ++m_uRef; }
	ULONG Release() { ULONG u = --m_uRef; if (!u) delete this; return u; }

private:
	ULONG m_uRef;
};

static CMySimpleArray<CMyHookRegKeyData*>* s_pRegData = NULL;
static CRITICAL_SECTION s_csRegData;

EXTERN_C HRESULT __stdcall InitRegHook()
{
	if (!s_pszAdvapi32_name)
	{
		if (!InitOldRegFunctions())
			return HRESULT_FROM_WIN32(GetLastError());
	}

	{
		HMODULE hInstKernel32 = ::GetModuleHandle(_T("kernel32.dll"));
		T_CreateToolhelp32Snapshot pfnCreateToolhelp32Snapshot;

		pfnCreateToolhelp32Snapshot = (T_CreateToolhelp32Snapshot)
			::GetProcAddress(hInstKernel32, "CreateToolhelp32Snapshot");
		if (pfnCreateToolhelp32Snapshot)
		{
			T_Module32First pfnModule32First;
			T_Module32Next pfnModule32Next;

			pfnModule32First = (T_Module32First)
				::GetProcAddress(hInstKernel32, "Module32First");
			pfnModule32Next = (T_Module32Next)
				::GetProcAddress(hInstKernel32, "Module32Next");
			if (!pfnModule32First || !pfnModule32Next)
				return HRESULT_FROM_WIN32(GetLastError());

			auto hr = MyEnumModulesUsingSnapshot(pfnCreateToolhelp32Snapshot, pfnModule32First,
				pfnModule32Next, MyReplaceRegFunctions);
			if (FAILED(hr))
				return hr;
		}
		else
		{
			HINSTANCE hInstPsapi = ::LoadLibrary(_T("psapi.dll"));
			if (!hInstPsapi)
				return HRESULT_FROM_WIN32(GetLastError());

			T_EnumProcessModules pfnEnumProcessModules;

			pfnEnumProcessModules = (T_EnumProcessModules) ::GetProcAddress(hInstPsapi, "EnumProcessModules");
			if (!pfnEnumProcessModules)
			{
				auto err = GetLastError();
				::FreeLibrary(hInstPsapi);
				return HRESULT_FROM_WIN32(err);
			}

			auto hr = MyEnumModulesUsingPsapi(pfnEnumProcessModules, MyReplaceRegFunctions);

			::FreeLibrary(hInstPsapi);
			if (FAILED(hr))
				return hr;
		}
	}

	if (!s_pRegData)
	{
		s_pRegData = new CMySimpleArray<CMyHookRegKeyData*>();
		::InitializeCriticalSection(&s_csRegData);
	}
	return S_OK;
}

EXTERN_C void __stdcall TermRegHook()
{
	if (s_pRegData)
	{
		::EnterCriticalSection(&s_csRegData);
		if (s_pRegData)
		{
			register int c = s_pRegData->GetCount();
			while (c--)
				s_pRegData->GetItem(c)->Release();
			delete s_pRegData;
			s_pRegData = NULL;
		}
		::LeaveCriticalSection(&s_csRegData);
	}
	::DeleteCriticalSection(&s_csRegData);
}

////////////////////////////////////////////////////////////////////////////////

static bool __stdcall GetModuleFileNameString(HINSTANCE hInstance, CMyStringW& strResult)
{
	DWORD dw = ::GetModuleFileNameW(hInstance, strResult.GetBuffer(MAX_PATH), MAX_PATH);
	if (!dw && ::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
	{
		dw = ::GetModuleFileNameA(hInstance, strResult.GetBufferA(MAX_PATH), MAX_PATH);
		strResult.ReleaseBufferA(TRUE, dw);
	}
	else
		strResult.ReleaseBuffer(dw);
	return dw != 0;
}

static CMyHookRegKeyData* __stdcall MyFindHookRegKeyData(HKEY hKey, int* pnIndex)
{
	if (!s_pRegData)
		return NULL;
	if (IS_PREDEFINED_HKEY(hKey))
		return NULL;
	for (int i = 0; i < s_pRegData->GetCount(); i++)
	{
		register CMyHookRegKeyData* p = s_pRegData->GetItem(i);
		if (p->hKey == hKey)
		{
			if (pnIndex)
				*pnIndex = i;
			return p;
		}
	}
	return NULL;
}

static void __stdcall _ExpandRegEnvs(CMyStringW& rstrData)
{
	if (rstrData.IsEmpty())
		return;

	CMyStringW strSrc(rstrData);
	CMyStringW str2;
	LPWSTR lpSrc = strSrc.GetBuffer();
	LPWSTR lp, lp2;
	rstrData.Empty();
	while (*lpSrc)
	{
		lp = wcschr(lpSrc, L'%');
		if (lp)
		{
			*lp++ = 0;
			rstrData += lpSrc;

			lp2 = wcschr(lp, L'%');
			if (!lp2)
				break;
			*lp2++ = 0;
			lpSrc = lp2;

			if (_wcsicmp(lp, L"CLSID") == 0)
			{
				str2.Format(L"{%08lX-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}",
					CLSID_EasySFTPRoot.Data1, (int) CLSID_EasySFTPRoot.Data2, (int) CLSID_EasySFTPRoot.Data3,
					(int) CLSID_EasySFTPRoot.Data4[0], (int) CLSID_EasySFTPRoot.Data4[1], (int) CLSID_EasySFTPRoot.Data4[2],
					(int) CLSID_EasySFTPRoot.Data4[3], (int) CLSID_EasySFTPRoot.Data4[4], (int) CLSID_EasySFTPRoot.Data4[5],
					(int) CLSID_EasySFTPRoot.Data4[6], (int) CLSID_EasySFTPRoot.Data4[7]);
				rstrData += str2;
			}
			else if (_wcsicmp(lp, L"MODULE") == 0)
			{
				HINSTANCE hInst = ::GetModuleHandle(_T("EasySFTP.dll"));
				if (!hInst || !::GetModuleFileNameString(hInst, str2))
				{
					if (!::GetModuleFileNameString(NULL, str2))
						continue;
					LPWSTR lpw = str2.GetBuffer();
					LPWSTR lpw2 = wcsrchr(lpw, L'\\');
					if (lpw2)
					{
						lpw2++;
						str2.ReleaseBuffer((DWORD) (((DWORD_PTR) lpw2 - (DWORD_PTR) lpw) / sizeof(WCHAR)));
					}
					else
						str2.Empty();
					str2 += _T("EasySFTP.dll");
				}
				rstrData += str2;
			}
		}
		else
			break;
	}
	rstrData += lpSrc;
}

static void __stdcall _AppendKeyName(LPCWSTR lpszParent, LPCWSTR lpszName, CMyStringW& rstrName)
{
	CMyStringW strName(lpszName);
	if (lpszParent)
	{
		CMyStringW strParent(lpszParent);
		rstrName = strParent;
		if (!rstrName.IsEmpty() && ((LPCWSTR) rstrName)[rstrName.GetLength() - 1] != L'\\')
			rstrName += L'\\';
	}
	else
		rstrName.Empty();
	rstrName += strName;
}

// pEntry, ppData can be NULL
// hKeyParent needn't be a root key
static LSTATUS __stdcall MyDummyOpenRegKey(HKEY hKeyParent, LPCWSTR lpszKeyName, const CMyHookRegEntry* pEntry, CMyHookRegKeyData* pParent, REGSAM samDesired, HKEY* phKey, CMyHookRegKeyData** ppData)
{
	if (samDesired & (KEY_SET_VALUE | KEY_CREATE_SUB_KEY))
		return ERROR_ACCESS_DENIED;

	LSTATUS ret;
	HKEY hKey;
	CMyHookRegKeyData* pData;

	//{
	//	CMyStringW str;
	//	str.Format(L"MyDummyOpenRegKey: hKeyParent = 0x%p, lpszKeyName = %s\n", (LPCVOID) hKeyParent, lpszKeyName);
	//	OutputDebugString(str);
	//}
	//LPCWSTR lp = wcschr(lpszKeyName, L'\\');
	//if (lp)
	//{
	//	{
	//		CMyStringW str;
	//		str.SetString(lpszKeyName, ((DWORD_PTR) lp - (DWORD_PTR) lpszKeyName) / sizeof(WCHAR));
	//		ret = MyDummyOpenRegKey(hKeyParent, str, NULL, pParent, samDesired, &hKey, &pData);
	//	}
	//	if (ret != ERROR_SUCCESS)
	//		return ret;
	//	while (*lp == L'\\')
	//		lp++;
	//	return MyDummyOpenRegKey(hKey, lp, pEntry, pData, samDesired, phKey, ppData);
	//}

	::EnterCriticalSection(&s_csRegData);

	if (!phKey)
		ret = ERROR_INVALID_PARAMETER;
	else
	{
		pData = new CMyHookRegKeyData();
		if (!pData)
			ret = ERROR_OUTOFMEMORY;
		else
		{
			//ret = s_pfnRegOpenKeyExA(HKEY_CURRENT_USER, "Software", 0, samDesired, &hKey);
			hKey = NULL;
			if (!s_bUnicodeSupported)
			{
				CMyStringW str(lpszKeyName);
				ret = s_pfnRegOpenKeyExA(hKeyParent, str, 0, samDesired, &hKey);
			}
			else
				ret = s_pfnRegOpenKeyExW(hKeyParent, lpszKeyName, 0, samDesired, &hKey);
			if ((pData->bIsDummy = (ret != ERROR_SUCCESS || !hKey)))
				ret = s_pfnRegOpenKeyExA(HKEY_CURRENT_USER, "Software", 0, samDesired, &hKey);
			if (ret != ERROR_SUCCESS)
			{
				delete pData;
			}
			else
			{
				if (pParent)
					pParent->AddRef();
				pData->hKey = hKey;
				pData->hKeyParent = hKeyParent;
				pData->bIsClassesRoot = false;
				pData->pEntry = pEntry;
				pData->pParent = pParent;
				//if (pParent)
				//{
				//	CMyStringW str(pParent->strKeyName);
				//	_ExpandRegEnvs(str);
				//	_AppendKeyName(str, lpszKeyName, pData->strKeyName);
				//}
				//else
				//	_AppendKeyName(NULL, lpszKeyName, pData->strKeyName);
				pData->strKeyName = lpszKeyName;
				s_pRegData->Add(pData);

				*phKey = hKey;
				if (ppData)
					*ppData = pData;
			}
		}
	}
	::LeaveCriticalSection(&s_csRegData);
	return ret;
}


static const DWORD s_dwAttributes = SFGAO_BROWSABLE | SFGAO_FOLDER | SFGAO_HASSUBFOLDER | SFGAO_FILESYSTEM;
static const DWORD s_dwCachedValue = 1;
static const CMyHookRegEntry s_arrDummyKeys[] = {
	{ HKEY_CLASSES_ROOT, L"CLSID", true, REG_SZ, L"%CLSID%", L"EasySFTP" },
	{ HKEY_CLASSES_ROOT, L"CLSID\\%CLSID%", true, REG_SZ, L"InProcServer32", L"%MODULE%" },
	{ HKEY_CLASSES_ROOT, L"CLSID\\%CLSID%\\InProcServer32", false, REG_SZ, L"ThreadingModel", L"Apartment" },
	{ HKEY_CLASSES_ROOT, L"CLSID\\%CLSID%", true, REG_SZ, L"ShellFolder", NULL },
	{ HKEY_CLASSES_ROOT, L"CLSID\\%CLSID%", true, REG_SZ, L"DefaultIcon", L"%MODULE%,0" },
	{ HKEY_CLASSES_ROOT, L"CLSID\\%CLSID%\\ShellFolder", false, REG_DWORD, L"Attributes", &s_dwAttributes },
	{ HKEY_CLASSES_ROOT, NULL, true, REG_SZ, L"ftp", NULL },
	{ HKEY_CLASSES_ROOT, L"ftp", false, REG_SZ, L"ShellFolder", "%CLSID%" },
	{ HKEY_CLASSES_ROOT, L"ftp", false, REG_SZ, L"URL Protocol", NULL },
	{ HKEY_CLASSES_ROOT, NULL, true, REG_SZ, L"ftps", NULL },
	{ HKEY_CLASSES_ROOT, L"ftps", false, REG_SZ, L"ShellFolder", "%CLSID%" },
	{ HKEY_CLASSES_ROOT, L"ftps", false, REG_SZ, L"URL Protocol", NULL },
	{ HKEY_CLASSES_ROOT, NULL, true, REG_SZ, L"sftp", NULL },
	{ HKEY_CLASSES_ROOT, L"sftp", false, REG_SZ, L"ShellFolder", "%CLSID%" },
	{ HKEY_CLASSES_ROOT, L"sftp", false, REG_SZ, L"URL Protocol", NULL },
	{ HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace", true, REG_SZ, L"%CLSID%", NULL },
	{ HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Cached", false, REG_DWORD, L"%CLSID% {000214E6-0000-0000-C000-000000000046} 0x401", &s_dwCachedValue },
	{ NULL, NULL, false, REG_NONE, NULL, NULL }
};

// lpszTarget = L"Software\\Microsoft\\Windows", lpszTest = L"Software"
//   --> ret = L"Microsoft\\Windows"
// lpszTarget = L"Software\\Microsoft\\Windows", lpszTest = L"SOFTWARE\\Microsoft"
//   --> ret = L"Windows"
// lpszTarget = L"Software\\Microsoft\\Windows", lpszTest = L"Software\\Microsoft\\Windows"
//   --> ret = L""
// lpszTarget = L"Software\\Microsoft\\Windows", lpszTest = L"System"
//   --> ret = NULL
static LPCWSTR __stdcall _MatchKeyName(LPCWSTR lpszTarget, LPCWSTR lpszTest)
{
	LPCWSTR lpszEnd;
	while (true)
	{
		lpszEnd = wcschr(lpszTarget, L'\\');
		if (lpszEnd)
		{
			register size_t nLen = (((DWORD_PTR) lpszEnd - (DWORD_PTR) lpszTarget) / sizeof(WCHAR));
			if (_wcsnicmp(lpszTarget, lpszTest, nLen) == 0)
			{
				// lpszTest が完全に一致しサブキーを含んでいないかどうか
				if (!lpszTest[nLen])
				{
					while (*lpszEnd == L'\\')
						lpszEnd++;
					return lpszEnd;
				}
				// lpszTest のキー名が一致しサブキーを含んでいるかどうか
				else if (lpszTest[nLen] == L'\\')
				{
					lpszTest += nLen;
					while (*lpszTest == L'\\')
						lpszTest++;
				}
				else
					break;
			}
			else
				break;
			while (*lpszEnd == L'\\')
				lpszEnd++;
			lpszTarget = lpszEnd;
		}
		else
		{
			if (_wcsicmp(lpszTarget, lpszTest) == 0)
			{
				while (*lpszTarget)
					lpszTarget++;
				return lpszTarget;
			}
			else
				break;
		}
	}
	return NULL;
}

static bool __stdcall MyFindHookRegEntry(HKEY hKeyRoot, LPCWSTR lpszKeyName, const CMyHookRegEntry** ppEntry)
{
	if (!s_pRegData)
		return false;

	const CMyHookRegEntry* pEntry = s_arrDummyKeys;
	while (pEntry->hKeyParent)
	{
		if (pEntry->hKeyParent == hKeyRoot)
		{
			CMyStringW str;
			//if (pEntry->bIsKey)
				_AppendKeyName(pEntry->lpszKeyParent, pEntry->lpszKeyName, str);
			//else
			//	str = pEntry->lpszKeyParent;
			_ExpandRegEnvs(str);
			LPCWSTR lp;
			if (lpszKeyName)
				lp = _MatchKeyName(str, lpszKeyName);
			else
				lp = NULL;
			if (!lpszKeyName || lp)
			{
				if (!lp || *lp)
					pEntry = NULL;
				*ppEntry = pEntry;
				return true;
			}
		}
		pEntry++;
	}
	return false;
}

static bool __stdcall MyIsHookRegEntry(HKEY hKey, LPCWSTR lpszKeyName)
{
	if (!s_pRegData)
		return false;

	if (!IS_PREDEFINED_HKEY(hKey))
	{
		CMyHookRegKeyData* p = MyFindHookRegKeyData(hKey, NULL);
		if (!p)
			return false;
		if (p->bIsClassesRoot)
			hKey = HKEY_CLASSES_ROOT;
		else
		{
			CMyStringW str(p->strKeyName);
			_ExpandRegEnvs(str);
			_AppendKeyName(str, lpszKeyName, str);
			return MyIsHookRegEntry(p->hKeyParent, str);
		}
	}
	const CMyHookRegEntry* pEntry;
	if (!MyFindHookRegEntry(hKey, lpszKeyName, &pEntry))
		return false;
	return pEntry != NULL;
}

static bool __stdcall MyParseKeyData(HKEY hKey, LPCWSTR lpSubKey, HKEY* phKeyRoot, CMyStringW& rstrFullKey)
{
	if (!s_pRegData)
		return false;

	CMyStringW strSubKey(lpSubKey);
	while (true)
	{
		if (IS_PREDEFINED_HKEY(hKey))
		{
			*phKeyRoot = hKey;
			rstrFullKey = strSubKey;
			return true;
		}

		CMyHookRegKeyData* pData = MyFindHookRegKeyData(hKey, NULL);
		if (!pData)
			return false;
		if (pData->bIsClassesRoot)
		{
			//*phKeyRoot = hKey;
			*phKeyRoot = HKEY_CLASSES_ROOT;
			rstrFullKey = strSubKey;
			return true;
		}

		CMyStringW str(pData->strKeyName);
		_ExpandRegEnvs(str);
		if (!strSubKey.IsEmpty())
			_AppendKeyName(str, strSubKey, str);
		strSubKey = str;
		hKey = pData->hKeyParent;
	}
}


LSTATUS APIENTRY MyHookRegCloseKey(HKEY hKey)
{
	//{
	//	CMyStringW str;
	//	str.Format(L"MyHookRegCloseKey: hKey = 0x%p\n", hKey);
	//	OutputDebugStringW(str);
	//}
	if (s_pRegData)
	{
		::EnterCriticalSection(&s_csRegData);
		int n;
		CMyHookRegKeyData* pData = MyFindHookRegKeyData(hKey, &n);
		if (pData)
		{
			s_pRegData->RemoveItem(n);
			pData->Release();
		}
		::LeaveCriticalSection(&s_csRegData);
	}
	return s_pfnRegCloseKey(hKey);
}

LSTATUS APIENTRY MyHookRegCreateKeyA(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult)
{
	//return s_pfnRegCreateKeyA(hKey, lpSubKey, phkResult);
	return MyHookRegCreateKeyExA(hKey, lpSubKey, 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, NULL, phkResult, NULL);
}

LSTATUS APIENTRY MyHookRegCreateKeyW(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult)
{
	//return s_pfnRegCreateKeyW(hKey, lpSubKey, phkResult);
	return MyHookRegCreateKeyExW(hKey, lpSubKey, 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, NULL, phkResult, NULL);
}

LSTATUS APIENTRY MyHookRegCreateKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass,
	DWORD dwOptions, REGSAM samDesired, CONST LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition)
{
	//return s_pfnRegCreateKeyExA(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
	CMyStringW strSubKey(lpSubKey);
	CMyStringW strClass;
	if (lpClass)
		strClass = lpClass;
	return MyHookRegCreateKeyExW(hKey, strSubKey, Reserved, strClass.IsEmpty() ? NULL : (LPWSTR)(LPCWSTR) strClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
}

LSTATUS APIENTRY MyHookRegCreateKeyExW(HKEY hKey, LPCWSTR lpSubKey, DWORD Reserved, LPWSTR lpClass,
	DWORD dwOptions, REGSAM samDesired, CONST LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition)
{
	HKEY hKeyRoot;
	CMyStringW strKey;
	if (MyParseKeyData(hKey, lpSubKey, &hKeyRoot, strKey))
	{
		const CMyHookRegEntry* pEntry;
		if (MyFindHookRegEntry(hKeyRoot, strKey, &pEntry))
		{
			CMyHookRegKeyData* pParent;
			pParent = MyFindHookRegKeyData(hKey, NULL);
			return MyDummyOpenRegKey(hKey, lpSubKey, pEntry, pParent, samDesired, phkResult, NULL);
		}
	}

	return s_pfnRegCreateKeyExW(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
}

LSTATUS APIENTRY MyHookRegDeleteKeyA(HKEY hKey, LPCSTR lpSubKey)
{
	{
		CMyStringW str(lpSubKey);
		if (MyIsHookRegEntry(hKey, str))
			return ERROR_ACCESS_DENIED;
	}
	return s_pfnRegDeleteKeyA(hKey, lpSubKey);
}

LSTATUS APIENTRY MyHookRegDeleteKeyW(HKEY hKey, LPCWSTR lpSubKey)
{
	if (MyIsHookRegEntry(hKey, lpSubKey))
		return ERROR_ACCESS_DENIED;
	return s_pfnRegDeleteKeyW(hKey, lpSubKey);
}

LSTATUS APIENTRY MyHookRegDeleteKeyExA(HKEY hKey, LPCSTR lpSubKey, REGSAM samDesired, DWORD Reserved)
{
	{
		CMyStringW str(lpSubKey);
		if (MyIsHookRegEntry(hKey, str))
			return ERROR_ACCESS_DENIED;
	}
	return s_pfnRegDeleteKeyExA(hKey, lpSubKey, samDesired, Reserved);
}

LSTATUS APIENTRY MyHookRegDeleteKeyExW(HKEY hKey, LPCWSTR lpSubKey, REGSAM samDesired, DWORD Reserved)
{
	if (MyIsHookRegEntry(hKey, lpSubKey))
		return ERROR_ACCESS_DENIED;
	return s_pfnRegDeleteKeyExW(hKey, lpSubKey, samDesired, Reserved);
}

LSTATUS APIENTRY MyHookRegEnumKeyA(HKEY hKey, DWORD dwIndex, LPSTR lpName, DWORD cchName)
{
	return s_pfnRegEnumKeyA(hKey, dwIndex, lpName, cchName);
}

LSTATUS APIENTRY MyHookRegEnumKeyW(HKEY hKey, DWORD dwIndex, LPWSTR lpName, DWORD cchName)
{
	return s_pfnRegEnumKeyW(hKey, dwIndex, lpName, cchName);
}

LSTATUS APIENTRY MyHookRegEnumKeyExA(HKEY hKey, DWORD dwIndex, LPSTR lpName, LPDWORD lpcchName,
	LPDWORD lpReserved, LPSTR lpClass, LPDWORD lpcchClass, PFILETIME lpftLastWriteTime)
{
	LSTATUS ret = s_pfnRegEnumKeyExA(hKey, dwIndex, lpName, lpcchName,
		lpReserved, lpClass, lpcchClass, lpftLastWriteTime);
	//if (ret == ERROR_SUCCESS)
	//{
	//	CMyStringW str;
	//	str.Format("MyHookRegEnumKeyExA: hKey = 0x%p, index = %lu, result = %s\n", hKey, dwIndex, lpName);
	//	OutputDebugString(str);
	//}
	return ret;
}

LSTATUS APIENTRY MyHookRegEnumKeyExW(HKEY hKey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcchName,
	LPDWORD lpReserved, LPWSTR lpClass, LPDWORD lpcchClass, PFILETIME lpftLastWriteTime)
{
	LSTATUS ret = s_pfnRegEnumKeyExW(hKey, dwIndex, lpName, lpcchName,
		lpReserved, lpClass, lpcchClass, lpftLastWriteTime);
	//if (ret == ERROR_SUCCESS)
	//{
	//	CMyStringW str;
	//	str.Format(L"MyHookRegEnumKeyExW: hKey = 0x%p, index = %lu, result = %s\n", hKey, dwIndex, lpName);
	//	OutputDebugString(str);
	//}
	return ret;
}

LSTATUS APIENTRY MyHookRegEnumValueA(HKEY hKey, DWORD dwIndex, LPSTR lpValueName, LPDWORD lpcchValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
	LSTATUS ret = s_pfnRegEnumValueA(hKey, dwIndex, lpValueName, lpcchValueName, lpReserved, lpType, lpData, lpcbData);
	//if (ret == ERROR_SUCCESS)
	//{
	//	CMyStringW str;
	//	str.Format("MyHookRegEnumValueA: hKey = 0x%p, index = %lu, valueName = %s\n", hKey, dwIndex, lpValueName);
	//	OutputDebugString(str);
	//}
	return ret;
}

LSTATUS APIENTRY MyHookRegEnumValueW(HKEY hKey, DWORD dwIndex, LPWSTR lpValueName, LPDWORD lpcchValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
	LSTATUS ret = s_pfnRegEnumValueW(hKey, dwIndex, lpValueName, lpcchValueName, lpReserved, lpType, lpData, lpcbData);
	//if (ret == ERROR_SUCCESS)
	//{
	//	CMyStringW str;
	//	str.Format(L"MyHookRegEnumValueW: hKey = 0x%p, index = %lu, valueName = %s\n", hKey, dwIndex, lpValueName);
	//	OutputDebugString(str);
	//}
	return ret;
}

LSTATUS APIENTRY MyHookRegOpenKeyA(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult)
{
	//return s_pfnRegOpenKeyA(hKey, lpSubKey, phkResult);
	return MyHookRegOpenKeyExA(hKey, lpSubKey, 0, KEY_READ, phkResult);
}

LSTATUS APIENTRY MyHookRegOpenKeyW(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult)
{
	//return s_pfnRegOpenKeyW(hKey, lpSubKey, phkResult);
	return MyHookRegOpenKeyExW(hKey, lpSubKey, 0, KEY_READ, phkResult);
}

LSTATUS APIENTRY MyHookRegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult)
{
	CMyStringW str(lpSubKey);
	return MyHookRegOpenKeyExW(hKey, str, ulOptions, samDesired, phkResult);
}

LSTATUS APIENTRY MyHookRegOpenKeyExW(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult)
{
	HKEY hKeyRoot;
	CMyStringW strKey;
	if (MyParseKeyData(hKey, lpSubKey, &hKeyRoot, strKey))
	{
		const CMyHookRegEntry* pEntry;
		if (hKeyRoot == HKEY_LOCAL_MACHINE && !strKey.IsEmpty())
		{
			LPCWSTR lpw = _MatchKeyName(strKey, L"Software\\Classes");
			if (lpw)
			{
				CMyStringW str2(lpw);
				strKey = str2;
				hKeyRoot = HKEY_CLASSES_ROOT;
			}
		}
		if (MyFindHookRegEntry(hKeyRoot, strKey, &pEntry))
		{
			CMyHookRegKeyData* pParent;
			pParent = MyFindHookRegKeyData(hKey, NULL);
			return MyDummyOpenRegKey(hKey, lpSubKey, pEntry, pParent, samDesired, phkResult, NULL);
		}
	}

	LSTATUS ret = s_pfnRegOpenKeyExW(hKey, lpSubKey, ulOptions, samDesired, phkResult);
	//if (ret == ERROR_SUCCESS)
	//{
	//	CMyStringW str;
	//	str.Format(L"MyHookRegOpenKeyExW: opening key: hKey = 0x%p, out-hKey = 0x%p, key = '%s'\n", hKey, *phkResult, lpSubKey);
	//	OutputDebugStringW(str);
	//}
	//else
	//{
	//	CMyStringW str;
	//	str.Format(L"MyHookRegOpenKeyExW: opening key: hKey = 0x%p, [not found] key = '%s'\n", hKey, lpSubKey);
	//	OutputDebugStringW(str);
	//}
	return ret;
}

LSTATUS APIENTRY MyHookRegOpenUserClassesRoot(HANDLE hToken, DWORD dwOptions, REGSAM samDesired, PHKEY phkResult)
{
	if (!phkResult)
		return ERROR_INVALID_PARAMETER;
	HKEY hKey;
	LSTATUS ret = s_pfnRegOpenUserClassesRoot(hToken, dwOptions, samDesired, &hKey);
	if (ret != ERROR_SUCCESS || !s_pRegData)
		return ret;

	::EnterCriticalSection(&s_csRegData);
	CMyHookRegKeyData* pData = new CMyHookRegKeyData();
	pData->hKey = hKey;
	pData->hKeyParent = NULL;
	pData->bIsClassesRoot = true;
	pData->bIsDummy = false;
	pData->pEntry = NULL;
	pData->pParent = NULL;
	//pData->strKeyName = L"";
	s_pRegData->Add(pData);
	::LeaveCriticalSection(&s_csRegData);

	*phkResult = hKey;

	return ERROR_SUCCESS;
}

LSTATUS APIENTRY MyHookRegQueryInfoKeyA(HKEY hKey, LPSTR lpClass, LPDWORD lpcchClass,
	LPDWORD lpReserved, LPDWORD lpcSubKeys, LPDWORD lpcbMaxSubKeyLen, LPDWORD lpcbMaxClassLen, LPDWORD lpcValues,
	LPDWORD lpcbMaxValueNameLen, LPDWORD lpcbMaxValueLen, LPDWORD lpcbSecurityDescriptor, PFILETIME lpftLastWriteTime)
{
	return s_pfnRegQueryInfoKeyA(hKey, lpClass, lpcchClass,
		lpReserved, lpcSubKeys, lpcbMaxSubKeyLen, lpcbMaxClassLen, lpcValues,
		lpcbMaxValueNameLen, lpcbMaxValueLen, lpcbSecurityDescriptor, lpftLastWriteTime);
}

LSTATUS APIENTRY MyHookRegQueryInfoKeyW(HKEY hKey, LPWSTR lpClass, LPDWORD lpcchClass,
	LPDWORD lpReserved, LPDWORD lpcSubKeys, LPDWORD lpcbMaxSubKeyLen, LPDWORD lpcbMaxClassLen, LPDWORD lpcValues,
	LPDWORD lpcbMaxValueNameLen, LPDWORD lpcbMaxValueLen, LPDWORD lpcbSecurityDescriptor, PFILETIME lpftLastWriteTime)
{
	return s_pfnRegQueryInfoKeyW(hKey, lpClass, lpcchClass,
		lpReserved, lpcSubKeys, lpcbMaxSubKeyLen, lpcbMaxClassLen, lpcValues,
		lpcbMaxValueNameLen, lpcbMaxValueLen, lpcbSecurityDescriptor, lpftLastWriteTime);
}

LSTATUS APIENTRY MyHookRegQueryValueA(HKEY hKey, LPCSTR lpSubKey, LPSTR lpData, PLONG lpcbData)
{
	//return s_pfnRegQueryValueA(hKey, lpSubKey, lpData, lpcbData);
	HKEY h;
	LSTATUS r = MyHookRegOpenKeyExA(hKey, lpSubKey, 0, KEY_QUERY_VALUE, &h);
	if (r != ERROR_SUCCESS)
		return r;
	r = MyHookRegQueryValueExA(h, NULL, NULL, NULL, (LPBYTE) lpData, (LPDWORD) lpcbData);
	MyHookRegCloseKey(h);
	return r;
}

LSTATUS APIENTRY MyHookRegQueryValueW(HKEY hKey, LPCWSTR lpSubKey, LPWSTR lpData, PLONG lpcbData)
{
	//return s_pfnRegQueryValueW(hKey, lpSubKey, lpData, lpcbData);
	HKEY h;
	LSTATUS r = MyHookRegOpenKeyExW(hKey, lpSubKey, 0, KEY_QUERY_VALUE, &h);
	if (r != ERROR_SUCCESS)
		return r;
	r = MyHookRegQueryValueExW(h, NULL, NULL, NULL, (LPBYTE) lpData, (LPDWORD) lpcbData);
	MyHookRegCloseKey(h);
	return r;
}

static LSTATUS __stdcall _MyHookRegQueryValueExAW(bool bUnicode, HKEY hKey, LPCVOID lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
	if (lpData && !lpcbData)
		return ERROR_INVALID_PARAMETER;
	CMyHookRegKeyData* pData = MyFindHookRegKeyData(hKey, NULL);
	CMyStringW str;
	//{
	//	if (bUnicode)
	//		str.Format(L"_MyHookRegQueryValueExAW: hKey = 0x%p, lpValueName = %s\n", (LPCVOID) hKey, (LPCWSTR) lpValueName);
	//	else
	//		str.Format(L"_MyHookRegQueryValueExAW: hKey = 0x%p, lpValueName = %S\n", (LPCVOID) hKey, (LPCSTR) lpValueName);
	//	OutputDebugString(str);
	//	if (pData)
	//	{
	//		str.Format(L"  hKey is a hooked data: %s\n", (LPCWSTR) pData->strKeyName);
	//		OutputDebugString(str);
	//	}
	//}
	if (pData)
	{
		const CMyHookRegEntry* pEntry;
		if (!lpValueName || (bUnicode ? !*((LPCWSTR) lpValueName) : !*((LPCSTR) lpValueName)))
			pEntry = pData->pEntry;
		else
		{
			HKEY hKeyRoot;
			if (bUnicode)
				str = (LPCWSTR) lpValueName;
			else
				str = (LPCSTR) lpValueName;
			if (!MyParseKeyData(hKey, str, &hKeyRoot, str))
				pEntry = NULL;
			else if (!MyFindHookRegEntry(hKeyRoot, str, &pEntry))
				pEntry = NULL;
		}
		if (pEntry)
		{
			if (lpType)
				*lpType = pEntry->dwType;
			switch (pEntry->dwType)
			{
				case REG_SZ:
				{
					str = (LPCWSTR) pEntry->lpszValue;
					_ExpandRegEnvs(str);
					DWORD dw;
					dw = (DWORD) (bUnicode ? str.GetLength() : str.GetLengthA()) + 1;
					if (lpData)
					{
						if (*lpcbData < dw)
						{
							*lpcbData = dw;
							return ERROR_MORE_DATA;
						}
						if (str.IsEmpty())
						{
							if (bUnicode)
								*((LPWSTR) lpData) = 0;
							else
								*((LPSTR) lpData) = 0;
						}
						else
						{
							if (bUnicode)
								memcpy(lpData, (LPCWSTR) str, sizeof(WCHAR) * dw);
							else
								memcpy(lpData, (LPCSTR) str, sizeof(CHAR) * dw);
						}
					}
					if (bUnicode)
						dw *= sizeof(WCHAR);
					else
						dw *= sizeof(CHAR);
					*lpcbData = dw;
				}
				break;
				case REG_DWORD:
					if (lpData)
					{
						if (*lpcbData < sizeof(DWORD))
						{
							*lpcbData = sizeof(DWORD);
							return ERROR_MORE_DATA;
						}
						*((LPDWORD) lpData) = *((const DWORD FAR*) pEntry->lpszValue);
					}
					*lpcbData = sizeof(DWORD);
					break;
				default:
#ifdef _DEBUG
					DebugBreak();
#endif
					return ERROR_INTERNAL_ERROR;
			}
			return ERROR_SUCCESS;
		}
		else if (pData->bIsDummy)
			return ERROR_FILE_NOT_FOUND;
	}
	LSTATUS ret;
	if (bUnicode)
		ret = s_pfnRegQueryValueExW(hKey, (LPCWSTR) lpValueName, lpReserved, lpType, lpData, lpcbData);
	else
		ret = s_pfnRegQueryValueExA(hKey, (LPCSTR) lpValueName, lpReserved, lpType, lpData, lpcbData);
	//if (ret == ERROR_SUCCESS)
	//{
	//	if (lpValueName && (
	//		(bUnicode && wcscmp((LPCWSTR) lpValueName, L"Extended") == 0) ||
	//		(!bUnicode && strcmp((LPCSTR) lpValueName, "Extended") == 0)
	//		))
	//	{
	//		static bool s_isBreaked = false;
	//		if (!s_isBreaked)
	//		{
	//			s_isBreaked = true;
	//			DebugBreak();
	//		}
	//	}
	//}
	return ret;
}

LSTATUS APIENTRY MyHookRegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
	return _MyHookRegQueryValueExAW(false, hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

LSTATUS APIENTRY MyHookRegQueryValueExW(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
	return _MyHookRegQueryValueExAW(true, hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

static LSTATUS __stdcall _MyHookRegGetValueAW(bool bUnicode, HKEY hKey, LPCVOID lpSubKey, LPCVOID lpValue, DWORD dwFlags, LPDWORD pdwType, PVOID pvData, LPDWORD pcbData)
{
	if (pvData && !pcbData)
		return ERROR_INVALID_PARAMETER;
	CMyStringW str;
	//{
	//	if (bUnicode)
	//		str.Format(L"_MyHookRegGetValueAW: hKey = 0x%p, lpSubKey = %s, lpValue = %s\n", (LPCVOID) hKey, (LPCWSTR) lpSubKey, (LPCWSTR) lpValue);
	//	else
	//		str.Format(L"_MyHookRegGetValueAW: hKey = 0x%p, lpSubKey = %S, lpValue = %S\n", (LPCVOID) hKey, (LPCSTR) lpSubKey, (LPCSTR) lpValue);
	//	OutputDebugString(str);
	//	str.Empty();
	//}
	CMyHookRegKeyData* pData = MyFindHookRegKeyData(hKey, NULL);
	const CMyHookRegEntry* pEntry = NULL;
	if (pData)
	{
		if ((!lpSubKey || (bUnicode ? !*((LPCWSTR) lpSubKey) : !*((LPCSTR) lpSubKey))) &&
			(!lpValue || (bUnicode ? !*((LPCWSTR) lpValue) : !*((LPCSTR) lpValue))))
			pEntry = pData->pEntry;
		else
		{
			if (lpSubKey && (bUnicode ? *((LPCWSTR) lpSubKey) : *((LPCSTR) lpSubKey)))
			{
				if (bUnicode)
					str = (LPCWSTR) lpSubKey;
				else
					str = (LPCSTR) lpSubKey;
			}
			if (lpValue && (bUnicode ? *((LPCWSTR) lpValue) : *((LPCSTR) lpValue)))
			{
				if (!str.IsEmpty())
					str += L'\\';
				if (bUnicode)
					str += (LPCWSTR) lpValue;
				else
					str += (LPCSTR) lpValue;
			}

			{
				HKEY hKeyRoot;
				if (!MyParseKeyData(hKey, str, &hKeyRoot, str))
					pEntry = NULL;
				else if (!MyFindHookRegEntry(hKeyRoot, str, &pEntry))
					pEntry = NULL;
			}
		}
	}
	else
	{
		HKEY hKeyRoot;
		CMyStringW strKey;
		if (bUnicode)
		{
			strKey = (LPCWSTR) lpSubKey;
			_AppendKeyName(strKey, (LPCWSTR) lpValue, strKey);
		}
		else
		{
			strKey = (LPCSTR) lpSubKey;
			CMyStringW str2((LPCSTR) lpValue);
			_AppendKeyName(strKey, str2, strKey);
		}
		if (MyParseKeyData(hKey, strKey, &hKeyRoot, strKey))
		{
			if (hKeyRoot == HKEY_LOCAL_MACHINE)
			{
				LPCWSTR lpw = _MatchKeyName(strKey, L"Software\\Classes");
				if (lpw)
				{
					CMyStringW str2(lpw);
					strKey = str2;
					hKeyRoot = HKEY_CLASSES_ROOT;
				}
			}
			if (!MyFindHookRegEntry(hKeyRoot, strKey, &pEntry))
				pEntry = NULL;
		}
	}
	if (pEntry)
	{
		if (pdwType)
			*pdwType = pEntry->dwType;
		switch (pEntry->dwType)
		{
			case REG_SZ:
			{
				if (!(dwFlags & RRF_RT_REG_SZ))
					return ERROR_INVALID_PARAMETER;

				str = (LPCWSTR) pEntry->lpszValue;
				_ExpandRegEnvs(str);
				DWORD dw;
				if (str.IsEmpty())
				{
					dw = 0;
				}
				else
				{
					dw = (DWORD)(bUnicode ? str.GetLength() : str.GetLengthA()) + 1;
					if (pvData)
					{
						if (*pcbData < dw)
						{
							if (dwFlags & RRF_ZEROONFAILURE)
								memset(pvData, 0, (size_t)*pcbData);
							*pcbData = dw;
							return ERROR_MORE_DATA;
						}
						if (bUnicode)
							memcpy(pvData, (LPCWSTR)str, sizeof(WCHAR) * dw);
						else
							memcpy(pvData, (LPCSTR)str, sizeof(CHAR) * dw);
					}
					if (bUnicode)
						dw *= sizeof(WCHAR);
					else
						dw *= sizeof(CHAR);
				}
				*pcbData = dw;
			}
			break;
			case REG_DWORD:
				if (!(dwFlags & RRF_RT_DWORD))
					return ERROR_INVALID_PARAMETER;

				if (pvData)
				{
					if (*pcbData < sizeof(DWORD))
					{
						if (dwFlags & RRF_ZEROONFAILURE)
							memset(pvData, 0, (size_t) *pcbData);
						*pcbData = sizeof(DWORD);
						return ERROR_MORE_DATA;
					}
					*((LPDWORD) pvData) = *((const DWORD FAR*) pEntry->lpszValue);
				}
				*pcbData = sizeof(DWORD);
				break;
			default:
#ifdef _DEBUG
				DebugBreak();
#endif
				return ERROR_INTERNAL_ERROR;
		}
		return ERROR_SUCCESS;
	}
	else if (pData && pData->bIsDummy)
		return ERROR_FILE_NOT_FOUND;
	LSTATUS ret;
	if (bUnicode)
		ret = s_pfnRegGetValueW(hKey, (LPCWSTR) lpSubKey, (LPCWSTR) lpValue, dwFlags, pdwType, pvData, pcbData);
	else
		ret = s_pfnRegGetValueA(hKey, (LPCSTR) lpSubKey, (LPCSTR) lpValue, dwFlags, pdwType, pvData, pcbData);
	//if (ret == ERROR_SUCCESS)
	//{
	//	if (lpValue && (
	//		(bUnicode && wcscmp((LPCWSTR)lpValue, L"HideBasedOnVelocityID") == 0) ||
	//		(!bUnicode && strcmp((LPCSTR)lpValue, "HideBasedOnVelocityID") == 0)
	//		))
	//	{
	//		static bool s_isBreaked = false;
	//		if (!s_isBreaked)
	//		{
	//			s_isBreaked = true;
	//			DebugBreak();
	//		}
	//	}
	//}
	return ret;
}

LSTATUS APIENTRY MyHookRegGetValueA(HKEY hKey, LPCSTR lpSubKey, LPCSTR lpValue, DWORD dwFlags, LPDWORD pdwType, PVOID pvData, LPDWORD pcbData)
{
	return _MyHookRegGetValueAW(false, hKey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
}

LSTATUS APIENTRY MyHookRegGetValueW(HKEY hKey, LPCWSTR lpSubKey, LPCWSTR lpValue, DWORD dwFlags, LPDWORD pdwType, PVOID pvData, LPDWORD pcbData)
{
	return _MyHookRegGetValueAW(true, hKey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
}

LSTATUS APIENTRY MyHookRegSetValueA(HKEY hKey, LPCSTR lpSubKey, DWORD dwType, LPCSTR lpData, DWORD cbData)
{
	//return s_pfnRegSetValueA(hKey, lpSubKey, dwType, lpData, cbData);
	HKEY h;
	LSTATUS r = MyHookRegOpenKeyExA(hKey, lpSubKey, 0, KEY_SET_VALUE, &h);
	if (r != ERROR_SUCCESS)
		return r;
	r = MyHookRegSetValueExA(h, NULL, 0, dwType, (const BYTE FAR*) lpData, cbData);
	MyHookRegCloseKey(h);
	return r;
}

LSTATUS APIENTRY MyHookRegSetValueW(HKEY hKey, LPCWSTR lpSubKey, DWORD dwType, LPCWSTR lpData, DWORD cbData)
{
	//return s_pfnRegSetValueW(hKey, lpSubKey, dwType, lpData, cbData);
	HKEY h;
	LSTATUS r = MyHookRegOpenKeyExW(hKey, lpSubKey, 0, KEY_SET_VALUE, &h);
	if (r != ERROR_SUCCESS)
		return r;
	r = MyHookRegSetValueExW(h, NULL, 0, dwType, (const BYTE FAR*) lpData, cbData);
	MyHookRegCloseKey(h);
	return r;
}

static LSTATUS __stdcall _MyHookRegSetValueExAW(bool bUnicode, HKEY hKey, LPCVOID lpValueName, DWORD Reserved, DWORD dwType, CONST BYTE* lpData, DWORD cbData)
{
	CMyStringW str;
	CMyHookRegKeyData* pData = MyFindHookRegKeyData(hKey, NULL);
	if (pData)
	{
		const CMyHookRegEntry* pEntry;
		if (!lpValueName || (bUnicode ? !*((LPCWSTR) lpValueName) : !*((LPCSTR) lpValueName)))
			pEntry = pData->pEntry;
		else
		{
			HKEY hKeyRoot;
			if (bUnicode)
				str = (LPCWSTR) lpValueName;
			else
				str = (LPCSTR) lpValueName;
			if (!MyParseKeyData(hKey, str, &hKeyRoot, str))
				pEntry = NULL;
			else if (!MyFindHookRegEntry(hKeyRoot, str, &pEntry))
				pEntry = NULL;
		}
		if (pEntry)
			return ERROR_ACCESS_DENIED;
	}
	if (bUnicode)
		return s_pfnRegSetValueExW(hKey, (LPCWSTR) lpValueName, Reserved, dwType, lpData, cbData);
	else
		return s_pfnRegSetValueExA(hKey, (LPCSTR) lpValueName, Reserved, dwType, lpData, cbData);
}

LSTATUS APIENTRY MyHookRegSetValueExA(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, CONST BYTE* lpData, DWORD cbData)
{
	return _MyHookRegSetValueExAW(false, hKey, lpValueName, Reserved, dwType, lpData, cbData);
}

LSTATUS APIENTRY MyHookRegSetValueExW(HKEY hKey, LPCWSTR lpValueName, DWORD Reserved, DWORD dwType, CONST BYTE* lpData, DWORD cbData)
{
	return _MyHookRegSetValueExAW(true, hKey, lpValueName, Reserved, dwType, lpData, cbData);
}

LSTATUS APIENTRY MyHookRegSetKeyValueA(HKEY hKey, LPCSTR lpSubKey, LPCSTR lpValueName, DWORD dwType, LPCVOID lpData, DWORD cbData)
{
	HKEY h;
	LSTATUS r = MyHookRegOpenKeyExA(hKey, lpSubKey, 0, KEY_SET_VALUE, &h);
	if (r != ERROR_SUCCESS)
		return r;
	r = MyHookRegSetValueExA(h, lpValueName, 0, dwType, (const BYTE FAR*) lpData, cbData);
	MyHookRegCloseKey(h);
	return r;
}

LSTATUS APIENTRY MyHookRegSetKeyValueW(HKEY hKey, LPCWSTR lpSubKey, LPCWSTR lpValueName, DWORD dwType, LPCVOID lpData, DWORD cbData)
{
	HKEY h;
	LSTATUS r = MyHookRegOpenKeyExW(hKey, lpSubKey, 0, KEY_SET_VALUE, &h);
	if (r != ERROR_SUCCESS)
		return r;
	r = MyHookRegSetValueExW(h, lpValueName, 0, dwType, (const BYTE FAR*) lpData, cbData);
	MyHookRegCloseKey(h);
	return r;
}
