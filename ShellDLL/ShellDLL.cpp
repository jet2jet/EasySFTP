/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 ShellDLL.cpp - implementations of CMainDLL and utility classes/functions
 */

#include "stdafx.h"
#include "ShellDLL.h"

#include "RFolder.h"
#include "CFactory.h"
#include "INIFile.h"
#include "MErrDlg.h"

CMainDLL theApp;

EXTERN_C BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	return MyDllMain(hInstance, dwReason, lpReserved);
}

////////////////////////////////////////////////////////////////////////////////

#if !defined(NTDDI_WIN7) || (NTDDI_VERSION < NTDDI_WIN7)
#define INITGUID
#include <guiddef.h>
DEFINE_GUID(SID_SInPlaceBrowser, 0x1D2AE02B, 0x3655, 0x46CC, 0xB6, 0x3A, 0x28, 0x59, 0x88, 0x15, 0x3B, 0xCA);
#undef INITGUID
#include <guiddef.h>
#endif

// {AD29C042-B9E3-4638-9DF6-D7DA5B8D0199}
EXTERN_C const IID IID_IEasySFTPInternal =
{ 0xAD29C042, 0xB9E3, 0x4638, { 0x9D, 0xF6, 0xD7, 0xDA, 0x5B, 0x8D, 0x01, 0x99 } };
// {AD29C042-B9E3-463a-9DF6-D7DA5B8D0199}
EXTERN_C const IID IID_IEasySFTPListener =
{ 0xAD29C042, 0xB9E3, 0x463a, { 0x9D, 0xF6, 0xD7, 0xDA, 0x5B, 0x8D, 0x01, 0x99 } };
// {AD29C042-B9E3-463e-9DF6-D7DA5B8D0199}
EXTERN_C const IID IID_IEasySFTPDirectory =
{ 0xAD29C042, 0xB9E3, 0x463e, { 0x9D, 0xF6, 0xD7, 0xDA, 0x5B, 0x8D, 0x01, 0x99 } };
// {AD29C042-B9E3-463c-9DF6-D7DA5B8D0199}
EXTERN_C const IID IID_IEasySFTPRoot =
{ 0xAD29C042, 0xB9E3, 0x463c, { 0x9D, 0xF6, 0xD7, 0xDA, 0x5B, 0x8D, 0x01, 0x99 } };
// {AD29C042-B9E3-462c-9DF6-D7DA5B8D0199}
EXTERN_C const CLSID CLSID_EasySFTP =
{ 0xAD29C042, 0xB9E3, 0x462c, { 0x9D, 0xF6, 0xD7, 0xDA, 0x5B, 0x8D, 0x01, 0x99 } };

STDAPI EasySFTPCreateRoot(IEasySFTPRoot** ppRoot)
{
	if (!ppRoot)
		return E_POINTER;
	CEasySFTPFolderRoot* pRoot = new CEasySFTPFolderRoot();
	if (!pRoot)
		return E_OUTOFMEMORY;
	HRESULT hr = pRoot->QueryInterface(IID_IEasySFTPRoot, (void**) ppRoot);
	pRoot->Release();
	return hr;
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

LPWSTR __stdcall DuplicateCoMemString(const CMyStringW& string)
{
	size_t dw = string.GetLength() + 1;
	LPWSTR lpw = (LPWSTR) ::CoTaskMemAlloc(sizeof(WCHAR) * dw);
	if (!lpw)
		return lpw;
	if (!string.IsEmpty())
		memcpy(lpw, (LPCWSTR) string, sizeof(WCHAR) * dw);
	else
		lpw[0] = 0;
	return lpw;
}

inline LSTATUS __stdcall RegSetStringValue(HKEY hKey, LPCTSTR lpszName, LPCTSTR lpszValue)
{
	return ::RegSetValueEx(hKey, lpszName, 0, REG_SZ,
		(const BYTE FAR*) lpszValue, (DWORD) (sizeof(TCHAR) * (_tcslen(lpszValue) + 1)));
}

LSTATUS __stdcall RegGetStringValue(HKEY hKey, LPCTSTR lpszName, CMyStringW& rstrValue)
{
	DWORD dw = 0;
	LSTATUS ret = ::RegQueryValueEx(hKey, lpszName, NULL, NULL, NULL, &dw);
	if (ret != ERROR_SUCCESS)
	{
		rstrValue.Empty();
		return ret;
	}
	dw++;
#ifdef _UNICODE
	ret = ::RegQueryValueEx(hKey, lpszName, NULL, NULL, (LPBYTE) rstrValue.GetBuffer(dw), &dw);
	rstrValue.ReleaseBuffer(dw);
#else
	ret = ::RegQueryValueEx(hKey, lpszName, NULL, NULL, (LPBYTE) rstrValue.GetBufferA(dw), &dw);
	rstrValue.ReleaseBufferA(TRUE, dw);
#endif
	return ret;
}

inline LSTATUS __stdcall RegSetDWordValue(HKEY hKey, LPCTSTR lpszName, DWORD dwValue)
{
	return ::RegSetValueEx(hKey, lpszName, 0, REG_DWORD,
		(const BYTE FAR*) &dwValue, sizeof(dwValue));
}

void __stdcall _RegDeleteSubKeyValue(HKEY hKey, LPTSTR& lpBuffer, DWORD& dwBufferLen, LPCTSTR lpszThisKey)
{
	DWORD dwLen, dwIndex;
	DWORD dwKeys, dwMaxKeyLen, dwMaxLen;
	HKEY hKeySub;
	CMyStringW* paStr;
	LSTATUS lError;
	dwMaxLen = dwMaxKeyLen = 1000;
	::RegQueryInfoKey(hKey, NULL, NULL, NULL, &dwKeys, &dwMaxKeyLen, NULL, NULL, &dwMaxLen, NULL, NULL, NULL);
	dwMaxLen++;
	dwMaxKeyLen++;
	dwIndex = 0;
	if (dwBufferLen < dwMaxLen || dwBufferLen < dwMaxKeyLen)
	{
		dwBufferLen = max(dwMaxLen, dwMaxKeyLen);
		if (!lpBuffer)
			lpBuffer = (LPTSTR) malloc(sizeof(TCHAR) * dwBufferLen);
		else
			lpBuffer = (LPTSTR) realloc(lpBuffer, sizeof(TCHAR) * dwBufferLen);
	}
	dwLen = dwMaxKeyLen;
	dwIndex = 0;
	paStr = (CMyStringW*) malloc(sizeof(CMyStringW) * dwKeys);
	while (::RegEnumKeyEx(hKey, dwIndex, lpBuffer, &dwLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
	{
		CallConstructor(&paStr[dwIndex]);
		paStr[dwIndex] = lpBuffer;
		if ((lError = ::RegOpenKeyEx(hKey, lpBuffer, 0, KEY_ENUMERATE_SUB_KEYS | KEY_NOTIFY | KEY_QUERY_VALUE | DELETE, &hKeySub)) != ERROR_SUCCESS)
			;
		else
		{
			::_RegDeleteSubKeyValue(hKeySub, lpBuffer, dwBufferLen, paStr[dwIndex]);
			if (hKeySub)
				::RegCloseKey(hKeySub);
		}
		dwLen = dwMaxKeyLen;
		dwIndex++;
	}
	dwIndex = 0;
	while (dwIndex < dwKeys)
	{
		lError = ::RegDeleteKey(hKey, paStr[dwIndex]);
		paStr[dwIndex].~CMyStringW();
		dwIndex++;
	}
	free(paStr);
}

//LONG __stdcall MyRegDeleteKey(HKEY hKey, LPCTSTR lpszSubKey)
//{
//	HKEY hKeyDest;
//	CPointer<TCHAR> lpBuffer;
//	DWORD dwBufferLen;
//	dwBufferLen = MAX_PATH;
//	lpBuffer = (LPTSTR) malloc(sizeof(TCHAR) * dwBufferLen);
//	if (::RegOpenKeyEx(hKey, lpszSubKey, 0, KEY_ENUMERATE_SUB_KEYS | KEY_NOTIFY | KEY_QUERY_VALUE | DELETE, &hKeyDest) == ERROR_SUCCESS)
//	{
//		::_RegDeleteSubKeyValue(hKeyDest, lpBuffer.m_p, dwBufferLen);
//		::RegCloseKey(hKeyDest);
//	}
//	return ::RegDeleteKey(hKey, lpszSubKey);
//}

LSTATUS __stdcall MyRegDeleteKey2(HKEY hKey, LPCTSTR lpszSubKey, LPTSTR& lpBuffer, DWORD& dwBufferLen)
{
	HKEY hKeyDest;
	if (::RegOpenKeyEx(hKey, lpszSubKey, 0, KEY_ENUMERATE_SUB_KEYS | KEY_NOTIFY | KEY_QUERY_VALUE | DELETE, &hKeyDest) == ERROR_SUCCESS)
	{
		::_RegDeleteSubKeyValue(hKeyDest, lpBuffer, dwBufferLen, lpszSubKey);
		::RegCloseKey(hKeyDest);
	}
	return ::RegDeleteKey(hKey, lpszSubKey);
}

////////////////////////////////////////////////////////////////////////////////

typedef HRESULT (STDAPICALLTYPE* T_SHCreateItemFromIDList)(__in PCIDLIST_ABSOLUTE pidl, __in REFIID riid, __deref_out void **ppv);
typedef HRESULT (STDAPICALLTYPE* T_SHCreateShellItem)(__in_opt PCIDLIST_ABSOLUTE pidlParent, __in_opt IShellFolder *psfParent, __in PCUITEMID_CHILD pidl, __out IShellItem **ppsi);
static bool s_bSHItemFuncInitialized = false;
static T_SHCreateItemFromIDList s_pfnSHCreateItemFromIDList = NULL;
static T_SHCreateShellItem s_pfnSHCreateShellItem = NULL;

STDAPI MyCreateShellItem(PCIDLIST_ABSOLUTE pidl, IShellItem** ppItem)
{
	if (!s_bSHItemFuncInitialized)
	{
		HINSTANCE hInstShell32 = ::GetModuleHandle(_T("shell32.dll"));
		if (hInstShell32)
		{
			s_pfnSHCreateItemFromIDList = (T_SHCreateItemFromIDList) ::GetProcAddress(hInstShell32, "SHCreateItemFromIDList");
			if (!s_pfnSHCreateItemFromIDList)
				s_pfnSHCreateShellItem = (T_SHCreateShellItem) ::GetProcAddress(hInstShell32, "SHCreateShellItem");
		}
		s_bSHItemFuncInitialized = true;
	}

	if (s_pfnSHCreateItemFromIDList)
		return s_pfnSHCreateItemFromIDList(pidl, IID_IShellItem, (void**) ppItem);
	if (s_pfnSHCreateShellItem)
		return s_pfnSHCreateShellItem(NULL, NULL, (PCUITEMID_CHILD) pidl, ppItem);
	return E_NOTIMPL;
}

static bool __stdcall _GetThumbnailProviderCLSID(HKEY hKeyParent, CLSID* pclsid)
{
	HKEY hKey;
	bool bRet = false;
	if (::RegOpenKeyEx(hKeyParent, _T("ShellEx\\{E357FCCD-A995-4576-B01F-234630154E96}"), 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		CMyStringW str;
		if (::RegGetStringValue(hKey, NULL, str) == ERROR_SUCCESS)
			bRet = SUCCEEDED(::CLSIDFromString((LPOLESTR)(LPCOLESTR) str, pclsid));
		::RegCloseKey(hKey);
	}
	return bRet;
}

static bool __stdcall _GetThumbnailProviderCLSIDWithKeyName(HKEY hKeyParent, LPCTSTR lpszKey, CLSID* pclsid)
{
	HKEY hKey;
	bool bRet = false;
	if (::RegOpenKeyEx(hKeyParent, lpszKey, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		bRet = _GetThumbnailProviderCLSID(hKey, pclsid);
		::RegCloseKey(hKey);
	}
	return bRet;
}

STDAPI MyCreateThumbnailProviderFromFileName(LPCWSTR lpszFileName, IThumbnailProvider** ppProvider)
{
	// extract ShellEx
	LPCWSTR lpszExt = wcsrchr(lpszFileName, L'.');
	if (!lpszExt)
		lpszExt = L"Unknown";
	HKEY hKey;
	bool b = false;
	CLSID clsid;
	CMyStringW strKey(lpszExt);
	if (::RegOpenKeyEx(HKEY_CLASSES_ROOT, strKey, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		b = _GetThumbnailProviderCLSID(hKey, &clsid);
		if (!b)
		{
			if (::RegGetStringValue(hKey, NULL, strKey) == ERROR_SUCCESS)
				b = _GetThumbnailProviderCLSIDWithKeyName(HKEY_CLASSES_ROOT, strKey, &clsid);
		}
		if (!b)
		{
			strKey = _T("SystemFileAssociations\\");
			strKey += lpszExt;
			b = _GetThumbnailProviderCLSIDWithKeyName(HKEY_CLASSES_ROOT, strKey, &clsid);
		}
		if (!b)
		{
			if (::RegGetStringValue(hKey, _T("PerceivedType"), strKey) == ERROR_SUCCESS)
			{
				strKey.InsertString(_T("SystemFileAssociations\\"), 0);
				b = _GetThumbnailProviderCLSIDWithKeyName(HKEY_CLASSES_ROOT, strKey, &clsid);
			}
		}
		::RegCloseKey(hKey);
	}
	if (!b)
		return E_FAIL;
	return ::CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IThumbnailProvider, (void**) ppProvider);
}

static HWND __stdcall _MyCreateTimerWindow()
{
	HWND h = ::CreateWindowEx(0, _T("Static"), _T("EasySFTP.dll"),
		WS_DISABLED, 0, 0, 0, 0, NULL, NULL, theApp.m_hInstance, NULL);
	return h;
}

////////////////////////////////////////////////////////////////////////////////

static HRESULT STDMETHODCALLTYPE EasySFTPClassCreateFunc(CClassFactory*, IUnknown* pUnkOuter, REFIID riid, void** ppv)
{
	if (pUnkOuter)
		return CLASS_E_NOAGGREGATION;
	CEasySFTPFolderRoot* pRoot = new CEasySFTPFolderRoot();
	HRESULT hr = pRoot->QueryInterface(riid, ppv);
	pRoot->Release();
	return hr;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID FAR* ppv)
{
	if (!ppv)
		return E_POINTER;

	*ppv = NULL;
	if (IsEqualCLSID(rclsid, CLSID_EasySFTP))
	{
		HRESULT hr = E_OUTOFMEMORY;

		CClassFactory* pClassFactory = new CClassFactory();
		if (pClassFactory != NULL)
		{
			pClassFactory->m_pFunc = EasySFTPClassCreateFunc;
			hr = pClassFactory->QueryInterface(riid, ppv);
			pClassFactory->Release();
		}
		return hr;
	}
	return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllCanUnloadNow()
{
	if (theApp.m_uCFLock > 0)
		return S_FALSE;
	if (theApp.m_aRootRefs.GetCount() > 0)
		return S_FALSE;
	for (int i = 0; i < theApp.m_aHosts.GetCount(); i++)
	{
		CHostFolderData* p = theApp.m_aHosts.GetItem(i);
		if (p->pDirItem->pDirectory)
		{
			p->pDirItem->pDirectory->AddRef();
			ULONG u = p->pDirItem->pDirectory->Release();
			if (u > 1)
				return S_FALSE;
		}
	}
	return S_OK;
}

STDAPI EasySFTPRegisterServer();

STDAPI DllRegisterServer()
{
	return EasySFTPRegisterServer();
}

STDAPI EasySFTPRegisterServer()
{
	HKEY hKeyCLSID, hKeyCLSIDMe, hKeySFTP, hKeyExplorer, hKey2;
	bool bMySFTPProcotol;
	LSTATUS lError;

	if ((lError = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("CLSID"), 0, KEY_WRITE, &hKeyCLSID)) != ERROR_SUCCESS)
		return HRESULT_FROM_WIN32(lError);

	CMyStringW strCLSID, strModule;
	strCLSID.Format(L"{%08lX-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		CLSID_EasySFTP.Data1, (int) CLSID_EasySFTP.Data2, (int) CLSID_EasySFTP.Data3,
		(int) CLSID_EasySFTP.Data4[0], (int) CLSID_EasySFTP.Data4[1], (int) CLSID_EasySFTP.Data4[2],
		(int) CLSID_EasySFTP.Data4[3], (int) CLSID_EasySFTP.Data4[4], (int) CLSID_EasySFTP.Data4[5],
		(int) CLSID_EasySFTP.Data4[6], (int) CLSID_EasySFTP.Data4[7]);
	GetModuleFileNameString(theApp.m_hInstance, strModule);

	lError = ::RegCreateKeyEx(hKeyCLSID, strCLSID, 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_WRITE, NULL, &hKeyCLSIDMe, NULL);
	::RegCloseKey(hKeyCLSID);
	if (lError != ERROR_SUCCESS)
		return HRESULT_FROM_WIN32(lError);
	// CLSID_EasySFTP
		::RegSetStringValue(hKeyCLSIDMe, NULL, theApp.m_strTitle);

		lError = ::RegCreateKeyEx(hKeyCLSIDMe, _T("InprocServer32"), 0, NULL, REG_OPTION_NON_VOLATILE,
			KEY_WRITE, NULL, &hKey2, NULL);
		if (lError != ERROR_SUCCESS)
		{
			::RegCloseKey(hKeyCLSIDMe);
			::RegDeleteKey(HKEY_CLASSES_ROOT, strCLSID);
			return HRESULT_FROM_WIN32(lError);
		}
		// InprocServer32
			::RegSetStringValue(hKey2, NULL, strModule);
			::RegSetStringValue(hKey2, _T("ThreadingModel"), _T("Apartment"));
			::RegCloseKey(hKey2);

		lError = ::RegCreateKeyEx(hKeyCLSIDMe, _T("DefaultIcon"), 0, NULL, REG_OPTION_NON_VOLATILE,
			KEY_WRITE, NULL, &hKey2, NULL);
		if (lError != ERROR_SUCCESS)
		{
			::RegDeleteKey(hKeyCLSIDMe, _T("InprocServer32"));
			::RegCloseKey(hKeyCLSIDMe);
			::RegDeleteKey(HKEY_CLASSES_ROOT, strCLSID);
			return HRESULT_FROM_WIN32(lError);
		}
		// DefaultIcon
		{
			CMyStringW str(strModule);
			str += L',';
			str += L'0';
			::RegSetStringValue(hKey2, NULL, str);
			::RegCloseKey(hKey2);
		}

		lError = ::RegCreateKeyEx(hKeyCLSIDMe, _T("ShellFolder"), 0, NULL, REG_OPTION_NON_VOLATILE,
			KEY_WRITE, NULL, &hKey2, NULL);
		if (lError != ERROR_SUCCESS)
		{
			::RegDeleteKey(hKeyCLSIDMe, _T("DefaultIcon"));
			::RegDeleteKey(hKeyCLSIDMe, _T("InprocServer32"));
			::RegCloseKey(hKeyCLSIDMe);
			::RegDeleteKey(HKEY_CLASSES_ROOT, strCLSID);
			return HRESULT_FROM_WIN32(lError);
		}
		// ShellFolder
			::RegSetDWordValue(hKey2, _T("Attributes"), SFGAO_BROWSABLE | SFGAO_FOLDER | SFGAO_HASSUBFOLDER);
			::RegCloseKey(hKey2);

	lError = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("sftp"), 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hKeySFTP);
	if (lError == ERROR_SUCCESS)
	{
		CMyStringW str;
		// maximum GUID length is 39 (including NULL character)
		DWORD dwType = REG_SZ, dwLength = 40;
		lError = ::RegQueryValueEx(hKeySFTP, _T("ShellFolder"), NULL, &dwType,
#ifdef _UNICODE
			(LPBYTE) str.GetBuffer(dwLength),
#else
			(LPBYTE) str.GetBufferA(dwLength),
#endif
			&dwLength);
		if (lError != ERROR_SUCCESS)
			dwLength = 0;
#ifdef _UNICODE
		str.ReleaseBuffer(dwLength);
#else
		str.ReleaseBufferA(dwLength);
#endif
		bMySFTPProcotol = (str.Compare(strCLSID, true) == 0);
	}
	else
	{
		lError = ::RegCreateKeyEx(HKEY_CLASSES_ROOT, _T("sftp"), 0, NULL,
			REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKeySFTP, NULL);
		if (lError != ERROR_SUCCESS)
		{
			::RegDeleteKey(hKeyCLSIDMe, _T("ShellFolder"));
			::RegDeleteKey(hKeyCLSIDMe, _T("DefaultIcon"));
			::RegDeleteKey(hKeyCLSIDMe, _T("InprocServer32"));
			::RegCloseKey(hKeyCLSIDMe);
			::RegDeleteKey(HKEY_CLASSES_ROOT, strCLSID);
			return HRESULT_FROM_WIN32(lError);
		}
		bMySFTPProcotol = true;
	}
	if (bMySFTPProcotol)
	{
		::RegSetStringValue(hKeySFTP, _T("ShellFolder"), strCLSID);
		::RegSetStringValue(hKeySFTP, _T("URL Protocol"), _T(""));
		::RegSetDWordValue(hKeySFTP, _T("EditFlags"), 0x00000002); // FTA_Show
	}

	lError = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace"),
		0, KEY_WRITE, &hKeyExplorer);
	if (lError != ERROR_SUCCESS)
	{
		::RegDeleteKey(hKeyCLSIDMe, _T("ShellFolder"));
		::RegDeleteKey(hKeyCLSIDMe, _T("DefaultIcon"));
		::RegDeleteKey(hKeyCLSIDMe, _T("InprocServer32"));
		::RegCloseKey(hKeyCLSIDMe);
		::RegDeleteKey(HKEY_CLASSES_ROOT, strCLSID);
		::RegCloseKey(hKeySFTP);
		if (bMySFTPProcotol)
			::RegDeleteKey(HKEY_CLASSES_ROOT, _T("sftp"));
		return HRESULT_FROM_WIN32(lError);
	}
	lError = ::RegCreateKeyEx(hKeyExplorer, strCLSID, 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey2, NULL);
	::RegCloseKey(hKeyExplorer);
	if (lError != ERROR_SUCCESS)
	{
		::RegDeleteKey(hKeyCLSIDMe, _T("ShellFolder"));
		::RegDeleteKey(hKeyCLSIDMe, _T("DefaultIcon"));
		::RegDeleteKey(hKeyCLSIDMe, _T("InprocServer32"));
		::RegCloseKey(hKeyCLSIDMe);
		::RegDeleteKey(HKEY_CLASSES_ROOT, strCLSID);
		::RegCloseKey(hKeySFTP);
		if (bMySFTPProcotol)
			::RegDeleteKey(HKEY_CLASSES_ROOT, _T("sftp"));
		return HRESULT_FROM_WIN32(lError);
	}
	::RegCloseKey(hKey2);
	::RegCloseKey(hKeySFTP);
	::RegCloseKey(hKeyCLSIDMe);
	return S_OK;
}

STDAPI EasySFTPUnregisterServer();

STDAPI DllUnregisterServer()
{
	return EasySFTPUnregisterServer();
}

STDAPI EasySFTPUnregisterServer()
{
	HKEY hKeyCLSID, hKeyCLSIDMe, hKey;
	LSTATUS lError;

	CMyStringW strCLSID;
	strCLSID.Format(L"{%08lX-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		CLSID_EasySFTP.Data1, (int) CLSID_EasySFTP.Data2, (int) CLSID_EasySFTP.Data3,
		(int) CLSID_EasySFTP.Data4[0], (int) CLSID_EasySFTP.Data4[1], (int) CLSID_EasySFTP.Data4[2],
		(int) CLSID_EasySFTP.Data4[3], (int) CLSID_EasySFTP.Data4[4], (int) CLSID_EasySFTP.Data4[5],
		(int) CLSID_EasySFTP.Data4[6], (int) CLSID_EasySFTP.Data4[7]);

	if (::RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("CLSID"), 0, KEY_WRITE, &hKeyCLSID) == ERROR_SUCCESS)
	{
		lError = ::RegOpenKeyEx(hKeyCLSID, strCLSID, 0, KEY_WRITE | DELETE, &hKeyCLSIDMe);
		if (lError == ERROR_SUCCESS)
		{
		// CLSID_EasySFTP
			::RegDeleteKey(hKeyCLSIDMe, _T("ShellFolder"));
			::RegDeleteKey(hKeyCLSIDMe, _T("DefaultIcon"));
			::RegDeleteKey(hKeyCLSIDMe, _T("InprocServer32"));
			::RegCloseKey(hKeyCLSIDMe);
		}
		::RegDeleteKey(hKeyCLSID, strCLSID);
		::RegCloseKey(hKeyCLSID);
	}

	lError = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("sftp"), 0, KEY_QUERY_VALUE, &hKey);
	if (lError == ERROR_SUCCESS)
	{
		CMyStringW str;
		// maximum GUID length is 39 (including NULL character)
		DWORD dwType = REG_SZ, dwLength = 40;
		lError = ::RegQueryValueEx(hKey, _T("ShellFolder"), NULL, &dwType,
#ifdef _UNICODE
			(LPBYTE) str.GetBuffer(dwLength),
#else
			(LPBYTE) str.GetBufferA(dwLength),
#endif
			&dwLength);
		if (lError != ERROR_SUCCESS)
			dwLength = 0;
#ifdef _UNICODE
		str.ReleaseBuffer(dwLength);
#else
		str.ReleaseBufferA(dwLength);
#endif
		::RegCloseKey(hKey);
		if (str.Compare(strCLSID, true) == 0)
		{
			LPTSTR lpBuffer = NULL;
			DWORD dwBufferLen = 0;
			::MyRegDeleteKey2(HKEY_CLASSES_ROOT, _T("sftp"), lpBuffer, dwBufferLen);
			if (lpBuffer)
				free(lpBuffer);
		}
	}

	lError = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace"),
		0, DELETE, &hKey);
	if (lError == ERROR_SUCCESS)
	{
		::RegDeleteKey(hKey, strCLSID);
		::RegCloseKey(hKey);
	}
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

// <host-name>\0<directory>\0[<file>\0]...\0 (all strings are unicode-string)
const TCHAR CMainDLL::s_szCFFTPData[] = _T("EasySFTPFormatData");
const WCHAR CMainDLL::s_szHostINIFile[] = L"Hosts.ini";

static const WCHAR s_wszDefTextFileType[] = L"*.txt;*.htm;*.html;*.css;*.xml;*.xsl;*.xslt;*.cgi;*.pl;*.php;*.sh;*.jsp;*.asp;*.ini";

CMainDLL::CMainDLL()
{
	m_msg.message = WM_QUIT;
	m_msg.wParam = (WPARAM) -1;
	m_bEmulateRegMode = false;
	m_bUseOFNUnicode = true;
	memset(&m_ofnW, 0, sizeof(m_ofnW));
	m_bUseBIUnicode = true;
	m_ofnW.lStructSize = OPENFILENAME_SIZE_VERSION_400W;
	m_uCFLock = 0;
	m_pimlSysIconJumbo = NULL;
	m_pimlSysIconExtraLarge = NULL;
	m_himlIconJumbo = NULL;
	m_himlIconExtraLarge = NULL;
	m_himlIconLarge = NULL;
	m_himlIconSmall = NULL;
	m_hMenuContext = NULL;
	m_hWndTimer = NULL;
	m_pObjectOnClipboard = NULL;
}

bool CMainDLL::InitInstance()
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif
	m_bEmulateRegMode = false;
	m_bEnableRootRefs = false;
	::InitializeCriticalSection(&m_csHosts);
	::InitializeCriticalSection(&m_csRootRefs);

	if (FAILED(::OleInitialize(NULL)))
		return false;
	WSADATA wsaData;
	if (::WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
		return false;

	::InitCommonControls();

	// for SSL library
	SSL_library_init();
	ERR_load_CRYPTO_strings();

	::srand((unsigned int) (time(NULL) * GetTickCount()));

	// Check whether we can use Unicode version of GetOpenFileName/GetSaveFileName
	{
		OSVERSIONINFO vi;
		vi.dwOSVersionInfoSize = sizeof(vi);
		if (!::GetVersionExA(&vi))
			m_bUseOFNUnicode = false;
		else
			m_bUseOFNUnicode = (vi.dwPlatformId == VER_PLATFORM_WIN32_NT);
	}

	// Initializing system image lists
	{
		PIDLIST_ABSOLUTE pidlDesktop;
		SHFILEINFO sfi;
		HRESULT hr = ::SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pidlDesktop);
		if (FAILED(hr))
			return false;
		memset(&sfi, 0, sizeof(sfi));
		m_himlSysIconLarge = (HIMAGELIST) ::SHGetFileInfo((LPCTSTR) pidlDesktop, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_LARGEICON);
		m_himlSysIconSmall = (HIMAGELIST) ::SHGetFileInfo((LPCTSTR) pidlDesktop, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
		::CoTaskMemFree(pidlDesktop);
		if (!m_himlSysIconLarge || !m_himlSysIconSmall)
			return false;

		typedef HRESULT (STDAPICALLTYPE* T_SHGetImageList)(__in int iImageList, __in REFIID riid, __deref_out void** ppvObj);
		T_SHGetImageList pfnSHGetImageList = (T_SHGetImageList) ::GetProcAddress(::GetModuleHandle(_T("shell32.dll")), "SHGetImageList");
		if (pfnSHGetImageList)
		{
			if (FAILED(pfnSHGetImageList(SHIL_EXTRALARGE, IID_IImageList, (void**) &m_pimlSysIconExtraLarge)))
				m_pimlSysIconExtraLarge = NULL;
			if (FAILED(pfnSHGetImageList(SHIL_JUMBO, IID_IImageList, (void**) &m_pimlSysIconJumbo)))
				m_pimlSysIconJumbo = NULL;
		}
		else
		{
			m_pimlSysIconJumbo = NULL;
			m_pimlSysIconExtraLarge = NULL;
		}
	}
	m_himlIconJumbo = ::ImageList_Create(256, 256, ILC_COLOR32 | ILC_MASK, 0, 0);
	if (!m_himlIconJumbo)
		return false;
	m_himlIconExtraLarge = ::ImageList_Create(48, 48, ILC_COLOR32 | ILC_MASK, 0, 0);
	if (!m_himlIconExtraLarge)
		return false;
	m_himlIconLarge = ::ImageList_Create(32, 32, ILC_COLOR32 | ILC_MASK, 0, 0);
	if (!m_himlIconLarge)
		return false;
	m_himlIconSmall = ::ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);
	if (!m_himlIconSmall)
		return false;
	// Loads icons for root folder
	{
		HICON hi;
		hi = (HICON) ::LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_EASYFTP), IMAGE_ICON,
			256, 256, LR_DEFAULTCOLOR);
		if (!hi)
			hi = (HICON) ::LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_EASYFTP), IMAGE_ICON,
				32, 32, LR_DEFAULTCOLOR);
		::ImageList_AddIcon(m_himlIconJumbo, hi);
		if (m_pimlSysIconJumbo)
			m_pimlSysIconJumbo->ReplaceIcon(-1, hi, &m_iEasySFTPIconIndex[iconIndexJumbo]);
		::DestroyIcon(hi);
		hi = (HICON) ::LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_EASYFTP), IMAGE_ICON,
			48, 48, LR_DEFAULTCOLOR);
		if (!hi)
			hi = (HICON) ::LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_EASYFTP), IMAGE_ICON,
				32, 32, LR_DEFAULTCOLOR);
		::ImageList_AddIcon(m_himlIconExtraLarge, hi);
		if (m_pimlSysIconExtraLarge)
			m_pimlSysIconExtraLarge->ReplaceIcon(-1, hi, &m_iEasySFTPIconIndex[iconIndexExtraLarge]);
		::DestroyIcon(hi);
		hi = (HICON) ::LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_EASYFTP), IMAGE_ICON,
			32, 32, LR_DEFAULTCOLOR);
		::ImageList_AddIcon(m_himlIconLarge, hi);
		m_iEasySFTPIconIndex[iconIndexLarge] = ::ImageList_AddIcon(m_himlSysIconLarge, hi);
		::DestroyIcon(hi);
		hi = (HICON) ::LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_EASYFTP), IMAGE_ICON,
			16, 16, LR_DEFAULTCOLOR);
		::ImageList_AddIcon(m_himlIconSmall, hi);
		m_iEasySFTPIconIndex[iconIndexSmall] = ::ImageList_AddIcon(m_himlSysIconSmall, hi);
		::DestroyIcon(hi);

		hi = (HICON) ::LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_NETDRIVE), IMAGE_ICON,
			256, 256, LR_DEFAULTCOLOR);
		::ImageList_AddIcon(m_himlIconJumbo, hi);
		if (m_pimlSysIconJumbo)
			m_pimlSysIconJumbo->ReplaceIcon(-1, hi, &m_iNetDriveIconIndex[iconIndexJumbo]);
		::DestroyIcon(hi);
		hi = (HICON) ::LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_NETDRIVE), IMAGE_ICON,
			48, 48, LR_DEFAULTCOLOR);
		::ImageList_AddIcon(m_himlIconExtraLarge, hi);
		if (m_pimlSysIconExtraLarge)
			m_pimlSysIconExtraLarge->ReplaceIcon(-1, hi, &m_iNetDriveIconIndex[iconIndexExtraLarge]);
		::DestroyIcon(hi);
		hi = (HICON) ::LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_NETDRIVE), IMAGE_ICON,
			32, 32, LR_DEFAULTCOLOR);
		::ImageList_AddIcon(m_himlIconLarge, hi);
		m_iNetDriveIconIndex[iconIndexLarge] = ::ImageList_AddIcon(m_himlSysIconLarge, hi);
		::DestroyIcon(hi);
		hi = (HICON) ::LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_NETDRIVE), IMAGE_ICON,
			16, 16, LR_DEFAULTCOLOR);
		::ImageList_AddIcon(m_himlIconSmall, hi);
		m_iNetDriveIconIndex[iconIndexSmall] = ::ImageList_AddIcon(m_himlSysIconSmall, hi);
		::DestroyIcon(hi);

		hi = (HICON) ::LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_NEWHOST), IMAGE_ICON,
			256, 256, LR_DEFAULTCOLOR);
		::ImageList_AddIcon(m_himlIconJumbo, hi);
		if (m_pimlSysIconJumbo)
			m_pimlSysIconJumbo->ReplaceIcon(-1, hi, &m_iNewHostIconIndex[iconIndexJumbo]);
		::DestroyIcon(hi);
		m_iNewHostIconIndex[iconIndexJumbo] = -1;
		hi = (HICON) ::LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_NEWHOST), IMAGE_ICON,
			48, 48, LR_DEFAULTCOLOR);
		::ImageList_AddIcon(m_himlIconExtraLarge, hi);
		if (m_pimlSysIconExtraLarge)
			m_pimlSysIconExtraLarge->ReplaceIcon(-1, hi, &m_iNewHostIconIndex[iconIndexExtraLarge]);
		::DestroyIcon(hi);
		hi = (HICON) ::LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_NEWHOST), IMAGE_ICON,
			32, 32, LR_DEFAULTCOLOR);
		::ImageList_AddIcon(m_himlIconLarge, hi);
		m_iNewHostIconIndex[iconIndexLarge] = ::ImageList_AddIcon(m_himlSysIconLarge, hi);
		::DestroyIcon(hi);
		hi = (HICON) ::LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_NEWHOST), IMAGE_ICON,
			16, 16, LR_DEFAULTCOLOR);
		::ImageList_AddIcon(m_himlIconSmall, hi);
		m_iNewHostIconIndex[iconIndexSmall] = ::ImageList_AddIcon(m_himlSysIconSmall, hi);
		::DestroyIcon(hi);
	}

	// Create the default font for window
	{
		NONCLIENTMETRICS ncm;
		ncm.cbSize = NONCLIENTMETRICS_SIZE_V1;
		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, NONCLIENTMETRICS_SIZE_V1, &ncm, 0);
		m_hFontWindow = ::CreateFontIndirect(&ncm.lfMessageFont);
	}

	m_hMenuPopup = ::LoadMenu(m_hInstance, MAKEINTRESOURCE(IDR_POPUP));
	m_hMenuContext = ::LoadMenu(m_hInstance, MAKEINTRESOURCE(IDR_SHELLMENU));

	// Build the INI file name
	{
		GetModuleFileNameString(m_hInstance, m_strINIFile);
		LPWSTR lpb = m_strINIFile.GetBuffer();
		LPWSTR lp = wcsrchr(lpb, L'\\');
		if (lp)
		{
			lp++;
			m_strINIFile.ReleaseBuffer((DWORD) (((DWORD_PTR) lp - (DWORD_PTR) lpb) / sizeof(WCHAR)));
		}
		//else
		//	m_strINIFile += L'.';
		//m_strINIFile += L"ini";
		m_strINIFile += s_szHostINIFile;
	}

	// Build the temporary directory name
	{
		if (!::GetTempPathW(MAX_PATH, m_strTempDirectory.GetBuffer(MAX_PATH)))
		{
			bool bRet;
			if ((bRet = (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)) != false)
			{
				bRet = (::GetTempPathA(MAX_PATH, m_strTempDirectory.GetBufferA(MAX_PATH)) > 0);
				if (bRet)
					m_strTempDirectory.ReleaseBufferA();
			}
			if (!bRet)
			{
				// if we cannot retreive the temporary directory,
				// then we use the application (DLL) directory
				GetModuleFileNameString(m_hInstance, m_strTempDirectory);
				LPWSTR lpb = m_strTempDirectory.GetBuffer();
				LPWSTR lp = wcsrchr(lpb, L'\\');
				if (lp)
					m_strTempDirectory.ReleaseBuffer((DWORD) (((DWORD_PTR) lp - (DWORD_PTR) lpb) / sizeof(WCHAR)));
				else
					m_strTempDirectory.Empty();
			}
		}
		else
			m_strTempDirectory.ReleaseBuffer();
		// appends "EasySFTP"
		if (!m_strTempDirectory.IsEmpty() && ((LPCWSTR) m_strTempDirectory)[m_strTempDirectory.GetLength() - 1] != L'\\')
			m_strTempDirectory += L'\\';
		m_strTempDirectory += L"EasySFTP";
	}

	// Parse the default text-file types and store them into the array
	{
		CMyStringW str(s_wszDefTextFileType);
		LPWSTR lpb = str.GetBuffer();
		LPWSTR lp = wcschr(lpb, L';');
		while (lp)
		{
			*lp++ = 0;
			m_arrDefTextFileType.Add(lpb);
			lpb = lp;
			lp = wcschr(lp, L';');
		}
	}

	// Build the filter string for file dialogs
	{
		m_strFilter.LoadString(IDS_ALL_FILTER);
		LPWSTR lpb = m_strFilter.GetBuffer();
		bool bLastIsDelimiter = false;
		while (*lpb)
		{
			if (bLastIsDelimiter = (*lpb == L'|'))
				*lpb = 0;
			lpb++;
		}
		if (!bLastIsDelimiter)
			m_strFilter += L'\0';
	}

	m_strTitle.LoadString(IDS_APP_TITLE);

	// Registers the clipboard formats
	m_nCFShellIDList = ::RegisterClipboardFormat(CFSTR_SHELLIDLIST);
	m_nCFFileContents = ::RegisterClipboardFormat(CFSTR_FILECONTENTS);
	m_nCFFileDescriptorA = ::RegisterClipboardFormat(CFSTR_FILEDESCRIPTORA);
	m_nCFFileDescriptorW = ::RegisterClipboardFormat(CFSTR_FILEDESCRIPTORW);
	m_nCFPerformedDropEffect = ::RegisterClipboardFormat(CFSTR_PERFORMEDDROPEFFECT);
	m_nCFPreferredDropEffect = ::RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
	m_nCFPasteSucceeded = ::RegisterClipboardFormat(CFSTR_PASTESUCCEEDED);

	m_nCFFTPData = ::RegisterClipboardFormat(s_szCFFTPData);
	//m_nCFFTPRenameFlag = ::RegisterClipboardFormat(s_szCFFTPRenameFlag);

	{
		FORMATETC fmt;
		fmt.cfFormat = m_nCFFTPData;
		fmt.dwAspect = DVASPECT_CONTENT;
		fmt.lindex = -1;
		fmt.ptd = NULL;
		fmt.tymed = TYMED_HGLOBAL;
		m_aFTPDataFormats.Add(fmt);
		//fmt.cfFormat = m_nCFFTPRenameFlag;
		//fmt.lindex = -1;
		//fmt.tymed = TYMED_HGLOBAL;
		//m_aFTPDataFormats.Add(fmt);
		fmt.cfFormat = m_nCFPerformedDropEffect;
		fmt.lindex = -1;
		fmt.tymed = TYMED_HGLOBAL;
		m_aFTPDataFormats.Add(fmt);
		fmt.cfFormat = m_nCFPreferredDropEffect;
		fmt.lindex = -1;
		fmt.tymed = TYMED_HGLOBAL;
		m_aFTPDataFormats.Add(fmt);
		fmt.cfFormat = m_nCFPasteSucceeded;
		fmt.lindex = -1;
		fmt.tymed = TYMED_HGLOBAL;
		m_aFTPDataFormats.Add(fmt);
		fmt.cfFormat = m_nCFShellIDList;
		fmt.lindex = -1;
		fmt.tymed = TYMED_HGLOBAL;
		m_aFTPDataFormats.Add(fmt);
		fmt.cfFormat = m_nCFFileContents;
		fmt.lindex = 0;
		fmt.tymed = TYMED_ISTREAM;
		m_aFTPDataFormats.Add(fmt);
		fmt.cfFormat = m_nCFFileDescriptorW;
		fmt.lindex = -1;
		fmt.tymed = TYMED_HGLOBAL;
		m_aFTPDataFormats.Add(fmt);
		fmt.cfFormat = m_nCFFileDescriptorA;
		fmt.tymed = TYMED_HGLOBAL;
		m_aFTPDataFormats.Add(fmt);
	}

	//m_hWndTimer = ::CreateWindowEx(0, _T("Static"), _T("EasySFTP.dll"),
	//	WS_DISABLED, 0, 0, 0, 0, NULL, NULL, m_hInstance, NULL);
	//if (!m_hWndTimer)
	//	return false;
	m_hWndTimer = NULL;

	CMyPtrArrayT<CHostSettings> aHostSettings;
	LoadINISettings(NULL, NULL, &aHostSettings);

	for (int i = 0; i < aHostSettings.GetCount(); i++)
	{
		CHostSettings* pSettings = aHostSettings.GetItem(i);

		CHostFolderData* pData = new CHostFolderData();
		pData->bSFTPMode = pSettings->bSFTPMode;
		pData->nPort = pSettings->nPort;
		pData->pDirItem = new CFTPDirectoryItem();
		pData->pDirItem->strName = pSettings->strHostName;
		pData->pDirItem->pDirectory = NULL;
		pData->pSettings = pSettings;
		m_aHosts.Add(pData);
	}

	// do not 'delete' items in aHostSettings
	aHostSettings.RemoveAll();

	m_bEnableRootRefs = true;

	return true;
}

static void __stdcall _DeleteFileOrDirectory(LPCWSTR lpszPath)
{
	if (::MyIsDirectoryW(lpszPath))
		::MyRemoveDirectoryRecursiveW(lpszPath);
	else
		::MyDeleteFileW(lpszPath);
}

int CMainDLL::ExitInstance()
{
	::EnterCriticalSection(&m_csRootRefs);
	m_bEnableRootRefs = false;
	m_aRootRefs.RemoveAll();
	::LeaveCriticalSection(&m_csRootRefs);

	if (m_pObjectOnClipboard)
	{
		if (::OleIsCurrentClipboard(m_pObjectOnClipboard) == S_OK)
			::OleSetClipboard(NULL);
		//m_pObjectOnClipboard->Release();
		m_pObjectOnClipboard = NULL;
	}

	for (int i = 0; i < m_aHosts.GetCount(); i++)
	{
		CHostFolderData* pData = m_aHosts.GetItem(i);
		if (pData->pDirItem)
		{
			if (pData->pDirItem->pDirectory)
				pData->pDirItem->pDirectory->Release();
			pData->pDirItem->Release();
		}
		if (pData->pSettings)
			delete pData->pSettings;
		delete pData;
	}
	m_aHosts.RemoveAll();
	//SaveINISettings();
	for (int i = 0; i < m_arrTempFileDirectories.GetCount(); i++)
		_DeleteFileOrDirectory(m_arrTempFileDirectories.GetItem(i));
	if (::MyIsEmptyDirectoryW(m_strTempDirectory))
		::MyRemoveDirectoryW(m_strTempDirectory);

	//EmptyKnownFingerPrints(m_aKnownFingerPrints);
	//EmptyHostSettings(m_aHostSettings);

	if (m_hWndTimer)
		::DestroyWindow(m_hWndTimer);

	if (m_hMenuContext)
		::DestroyMenu(m_hMenuContext);
	if (m_hMenuPopup)
		::DestroyMenu(m_hMenuPopup);
	if (m_hFontWindow)
		::DeleteObject(m_hFontWindow);

	if (m_himlIconSmall)
		::ImageList_Destroy(m_himlIconSmall);
	if (m_himlIconLarge)
		::ImageList_Destroy(m_himlIconLarge);
	if (m_himlIconExtraLarge)
		::ImageList_Destroy(m_himlIconExtraLarge);
	if (m_himlIconJumbo)
		::ImageList_Destroy(m_himlIconJumbo);

	if (m_pimlSysIconExtraLarge)
		m_pimlSysIconExtraLarge->Release();
	if (m_pimlSysIconJumbo)
		m_pimlSysIconJumbo->Release();

	::WSACleanup();
	::OleUninitialize();

	::DeleteCriticalSection(&m_csHosts);
	::DeleteCriticalSection(&m_csRootRefs);

	::ERR_remove_state(0);
	//::ENGINE_cleanup();
	//::CONF_modules_unload();
	//::RAND_cleanup();
	ERR_free_strings();
	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();

	return 0;
}

bool CMainDLL::MyPumpMessage()
{
	//if (!::PeekMessage(&m_msg, NULL, 0, 0, PM_NOREMOVE))
		return true;
	//return PumpMessage();
}

bool CMainDLL::MyPumpMessage2()
{
	if (!::PeekMessage(&m_msg, NULL, 0, 0, PM_NOREMOVE))
		return true;
	if (m_msg.message == WM_QUIT)
		return false;

	if (!::GetMessage(&m_msg, NULL, 0, 0))
		return false;
	CMyWindow* pWnd = CMyWindow::FromHandle(m_msg.hwnd);
	if (pWnd && pWnd->PreTranslateMessage(&m_msg))
		return true;
	CMyWindow* pWnd2 = CMyWindow::FromHandle(::GetActiveWindow());
	if (pWnd2 && pWnd2 != pWnd && pWnd2->PreTranslateMessage(&m_msg))
		return true;
	::TranslateMessage(&m_msg);
	::DispatchMessage(&m_msg);
	return true;
}

bool CMainDLL::CheckQueueMessage()
{
	return ::PeekMessage(&m_msg, NULL, 0, 0, PM_NOREMOVE) != 0;
}

void __stdcall CMainDLL::EmptyKnownFingerPrints(CMyPtrArrayT<CKnownFingerPrint>& aKnownFingerPrints)
{
	register int i = aKnownFingerPrints.GetCount();
	while (i--)
	{
		register CKnownFingerPrint* pPrint = aKnownFingerPrints.GetItem(i);
		free(pPrint->pFingerPrint);
		delete pPrint;
	}
	aKnownFingerPrints.RemoveAll();
}

void __stdcall CMainDLL::EmptyHostSettings(CMyPtrArrayT<CHostSettings>& aHostSettings)
{
	register int i = aHostSettings.GetCount();
	while (i--)
	{
		register CHostSettings* pHostSettings = aHostSettings.GetItem(i);
		delete pHostSettings;
	}
	aHostSettings.RemoveAll();
}

void CMainDLL::LoadINISettings(
		CGeneralSettings* pSettings,
		CMyPtrArrayT<CKnownFingerPrint>* paKnownFingerPrints,
		CMyPtrArrayT<CHostSettings>* paHostSettings
		)
{
	HINIFILE hINI;
	PVOID pvSection;
	CMyStringW str;
	int n;
	CHostSettings* pHost;

	hINI = ::MyLoadINIFileW(m_strINIFile, false);

//	if (pSettings)
//	{
//		pvSection = ::MyGetProfileSectionW(hINI, L"General");
//		//if (pvSection)
//		{
//			::MyEndReadProfileSectionW(pvSection);
//		}
//	}
	if (paKnownFingerPrints)
	{
		pvSection = ::MyGetProfileSectionW(hINI, L"FingerPrint");
		if (pvSection)
		{
			LPWSTR lpw, lpw2;
			n = 0;
			while (true)
			{
				lpw = ::MyGetNextProfileStringW(pvSection, n, &lpw2);
				if (!lpw)
					break;
				size_t nLen = (size_t) UnformatByteStringExW(lpw2, NULL, 0);
				if (nLen)
				{
					CKnownFingerPrint* pPrint = new CKnownFingerPrint();
					pPrint->strHostName = lpw;
					pPrint->nFingerPrintLen = nLen;
					pPrint->pFingerPrint = (BYTE*) malloc(nLen);
					if (pPrint->pFingerPrint)
					{
						UnformatByteStringExW(lpw2, pPrint->pFingerPrint, (DWORD) nLen);
						paKnownFingerPrints->Add(pPrint);
					}
					else
						delete pPrint;
				}
				free(lpw);
				free(lpw2);
				n++;
			}
			::MyEndReadProfileSectionW(pvSection);
		}
	}
	if (paHostSettings)
	{
		n = 1;
		while (true)
		{
			LPWSTR lpw, lpw2;
			str.Format(L"Host%d", n);
			pvSection = ::MyGetProfileSectionW(hINI, str);
			if (!pvSection)
				break;
			lpw = ::MyGetProfileStringW(pvSection, L"Name");
			if (lpw)
			{
				lpw2 = ::MyGetProfileStringW(pvSection, L"Host");
				if (lpw2)
				{
					pHost = new CHostSettings();
					pHost->strDisplayName = lpw;
					pHost->strHostName = lpw2;
					free(lpw);
					free(lpw2);
					//lpw2 = ::MyGetProfileStringW(pvSection, L"UserName");
					//if (lpw2)
					//{
					//	pHost->strUserName = lpw2;
					//	free(lpw2);
					//}
					pHost->bSFTPMode = ::MyGetProfileBooleanW(pvSection, L"SFTPMode", false);
					pHost->nPort = ::MyGetProfileIntW(pvSection, L"Port", pHost->bSFTPMode ? 22 : 21);
					pHost->bTextMode = (BYTE) ::MyGetProfileDWordW(pvSection, L"TextMode", TEXTMODE_NO_CONVERT | TEXTMODE_UTF8);
					pHost->nServerCharset = (char) ::MyGetProfileDWordW(pvSection, L"ServerCharset", scsUTF8);
					lpw2 = ::MyGetProfileStringW(pvSection, L"InitLocalPath");
					if (lpw2)
					{
						pHost->strInitLocalPath = lpw2;
						free(lpw2);
					}
					lpw2 = ::MyGetProfileStringW(pvSection, L"InitServerPath");
					if (lpw2)
					{
						pHost->strInitServerPath = lpw2;
						free(lpw2);
					}
					pHost->bUseThumbnailPreview = ::MyGetProfileBooleanW(pvSection, L"UseThumbnailPreview", true);
					pHost->nTransferMode = (char) ::MyGetProfileDWordW(pvSection, L"TransferMode", TRANSFER_MODE_AUTO);
					lpw2 = ::MyGetProfileStringW(pvSection, L"TextFileType");
					if (lpw2)
					{
						lpw = lpw2;
						while (true)
						{
							LPWSTR lpw3 = wcschr(lpw2, L';');
							if (lpw3)
								*lpw3++ = 0;
							pHost->arrTextFileType.Add(lpw2);
							if (!lpw3)
								break;
							lpw2 = lpw3;
						}
						free(lpw);
					}
					else
						pHost->arrTextFileType.CopyArray(theApp.m_arrDefTextFileType);
					pHost->bUseSystemTextFileType = ::MyGetProfileBooleanW(pvSection, L"UseSystemTextFileType", true);
					pHost->bAdjustRecvModifyTime = ::MyGetProfileBooleanW(pvSection, L"AdjustRecvModifyTime", false);
					pHost->bAdjustSendModifyTime = ::MyGetProfileBooleanW(pvSection, L"AdjustSendModifyTime", false);
					lpw2 = ::MyGetProfileStringW(pvSection, L"ChmodCommand");
					if (lpw2)
					{
						pHost->strChmodCommand = lpw2;
						free(lpw2);
					}
					else
						pHost->strChmodCommand = DEFAULT_CHMOD_COMMAND;
					//lpw2 = ::MyGetProfileStringW(pvSection, L"TouchCommand");
					//if (lpw2)
					//{
					//	pHost->strTouchCommand = lpw2;
					//	free(lpw2);
					//}
					//else
					//	pHost->strTouchCommand = DEFAULT_TOUCH_COMMAND;
					paHostSettings->Add(pHost);
				}
				else
					free(lpw);
				::MyEndReadProfileSectionW(pvSection);
			}
			n++;
		}
	}

	::MyCloseINIFile(hINI);
}

void CMainDLL::UpdateHostSettings(const CHostSettings* pOldSettings, const CHostSettings* pNewSettings, char nCommand, int nCount)
{
	CGeneralSettings settings;
	CMyPtrArrayT<CKnownFingerPrint> aKnownFingerPrints;
	CMyPtrArrayT<CHostSettings> aHostSettings;
	bool bFound = false;
	int n;
	CHostSettings* pFound;

	LoadINISettings(&settings, &aKnownFingerPrints, &aHostSettings);
	while (nCount--)
	{
		if (pOldSettings)
		{
			for (n = 0; n < aHostSettings.GetCount(); n++)
			{
				pFound = aHostSettings.GetItem(n);
				if (pFound->bSFTPMode == pOldSettings->bSFTPMode &&
					pFound->strDisplayName.Compare(pOldSettings->strDisplayName) == 0 &&
					pFound->strHostName.Compare(pOldSettings->strHostName) == 0 &&
					pFound->nPort == pOldSettings->nPort)
				{
					bFound = true;
					break;
				}
			}
		}
		switch (nCommand)
		{
			case -1:
				break;
			case 0: // update
			case 1: // add
				if (bFound)
					pFound->Copy(*pNewSettings);
				else
					aHostSettings.Add(new CHostSettings(*pNewSettings));
				break;
			case 2: // delete
				if (bFound)
				{
					aHostSettings.RemoveItem(n);
					delete pFound;
				}
				break;
		}
		if (pNewSettings)
			pNewSettings++;
		if (pOldSettings)
			pOldSettings++;
	}
	::EnterCriticalSection(&m_csHosts);
	MergeHostSettings(aHostSettings);
	::LeaveCriticalSection(&m_csHosts);
	if (nCommand != -1)
		SaveINISettings(&settings, &aKnownFingerPrints, &aHostSettings);

	EmptyKnownFingerPrints(aKnownFingerPrints);
	EmptyHostSettings(aHostSettings);
}

void CMainDLL::SaveINISettings(
		CGeneralSettings* pSettings,
		CMyPtrArrayT<CKnownFingerPrint>* paKnownFingerPrints,
		CMyPtrArrayT<CHostSettings>* paHostSettings
		)
{
	HANDLE hFile;
	CMyStringW str;

	hFile = ::CreateFileW(m_strINIFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
		hFile = ::CreateFileA(m_strINIFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return;
//	::MyWriteINISectionW(hFile, L"General");
//	if (settings.nSplitterPos != -1)
//		::MyWriteINIValueW(hFile, L"SplitterPos", settings.nSplitterPos);
//	if (settings.bUsePlacement)
//		::MyWriteINIValueW(hFile, L"WindowPlacement", &settings.wpFrame, sizeof(settings.wpFrame));
//#ifdef _EASYSFTP_USE_VIEWSTATE_STREAM
//	HGLOBAL hglb;
//	if (settings.pStreamViewState && SUCCEEDED(::GetHGlobalFromStream(settings.pStreamViewState, &hglb)))
//	{
//		SIZE_T nSize = ::GlobalSize(hglb);
//		LPVOID lpv = ::GlobalLock(hglb);
//		::MyWriteINIValueW(hFile, L"ViewState", lpv, (DWORD) nSize);
//		::GlobalUnlock(hglb);
//	}
//#endif
	if (paKnownFingerPrints->GetCount())
	{
		::MyWriteCRLFW(hFile);
		::MyWriteINISectionW(hFile, L"FingerPrint");
		for (int n = 0; n < paKnownFingerPrints->GetCount(); n++)
		{
			CKnownFingerPrint* pPrint = paKnownFingerPrints->GetItem(n);
			::MyWriteINIValueW(hFile, pPrint->strHostName, pPrint->pFingerPrint, (DWORD) pPrint->nFingerPrintLen);
		}
	}

	for (int n = 0; n < paHostSettings->GetCount(); n++)
	{
		CHostSettings* pHost = paHostSettings->GetItem(n);
		::MyWriteCRLFW(hFile);
		str.Format(L"Host%d", n + 1);
		::MyWriteINISectionW(hFile, str);
		::MyWriteINIValueW(hFile, L"Name", pHost->strDisplayName);
		::MyWriteINIValueW(hFile, L"Host", pHost->strHostName);
		::MyWriteINIValueW(hFile, L"SFTPMode", pHost->bSFTPMode ? 1 : 0);
		::MyWriteINIValueW(hFile, L"Port", pHost->nPort);
		//::MyWriteINIValueW(hFile, L"UserName", pHost->strUserName);
		::MyWriteINIValueW(hFile, L"TextMode", (int) pHost->bTextMode);
		::MyWriteINIValueW(hFile, L"ServerCharset", (int) pHost->nServerCharset);
		::MyWriteINIValueW(hFile, L"InitLocalPath", pHost->strInitLocalPath);
		::MyWriteINIValueW(hFile, L"InitServerPath", pHost->strInitServerPath);
		::MyWriteINIValueW(hFile, L"TransferMode", (int) pHost->nTransferMode);
		str.Empty();
		for (int n2 = 0; n2 < pHost->arrTextFileType.GetCount(); n2++)
		{
			if (!str.IsEmpty())
				str += L';';
			str += pHost->arrTextFileType.GetItem(n2);
		}
		::MyWriteINIValueW(hFile, L"TextFileType", str);
		::MyWriteINIValueW(hFile, L"UseSystemTextFileType", pHost->bUseSystemTextFileType ? 1 : 0);
		::MyWriteINIValueW(hFile, L"AdjustRecvModifyTime", pHost->bAdjustRecvModifyTime ? 1 : 0);
		::MyWriteINIValueW(hFile, L"AdjustSendModifyTime", pHost->bAdjustSendModifyTime ? 1 : 0);
		::MyWriteINIValueW(hFile, L"ChmodCommand", pHost->strChmodCommand);
		//::MyWriteINIValueW(hFile, L"TouchCommand", pHost->strTouchCommand);
	}
	::CloseHandle(hFile);
}

void CMainDLL::MergeHostSettings(const CMyPtrArrayT<CHostSettings>& aHostSettings)
{
	// update host settings and delete if the host data is not exist in aHostSettings
	for (int iH = 0; iH < m_aHosts.GetCount(); iH++)
	{
		CHostFolderData* pHost = m_aHosts.GetItem(iH);
		bool bFound = false;
		for (int iS = 0; iS < aHostSettings.GetCount(); iS++)
		{
			CHostSettings* pSettings = aHostSettings.GetItem(iS);
			if (pHost->pDirItem->strName.Compare(pSettings->strHostName) == 0 &&
				pHost->bSFTPMode == pSettings->bSFTPMode &&
				pHost->nPort == pSettings->nPort)
			{
				if (pHost->pSettings)
					pHost->pSettings->Copy(*pSettings);
				else
					pHost->pSettings = new CHostSettings(*pSettings);
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			if (pHost->pSettings)
			{
				delete pHost->pSettings;
				pHost->pSettings = NULL;
			}
			if (!pHost->pDirItem->pDirectory)
			{
				m_aHosts.RemoveItem(iH);
				delete pHost;
				iH--;
			}
		}
	}
	// add if the host data is not exist in m_aHosts
	for (int iS = 0; iS < aHostSettings.GetCount(); iS++)
	{
		CHostSettings* pSettings = aHostSettings.GetItem(iS);
		bool bFound = false;
		for (int iH = 0; iH < m_aHosts.GetCount(); iH++)
		{
			CHostFolderData* pHost = m_aHosts.GetItem(iH);
			if (pHost->pDirItem->strName.Compare(pSettings->strHostName) == 0 &&
				pHost->bSFTPMode == pSettings->bSFTPMode &&
				pHost->nPort == pSettings->nPort)
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			CHostFolderData* pHost = new CHostFolderData();
			pHost->bSFTPMode = pSettings->bSFTPMode;
			pHost->pDirItem = new CFTPDirectoryItem();
			pHost->pDirItem->strName = pSettings->strHostName;
			pHost->pDirItem->pDirectory = NULL;
			pHost->nPort = pSettings->nPort;
			pHost->pSettings = new CHostSettings(*pSettings);
			m_aHosts.Add(pHost);
		}
	}
}

CHostFolderData* CMainDLL::FindHostFolderDataUnsafe(bool bSFTP, LPCWSTR lpszHost, int nPort)
{
	CHostFolderData* pHostData;
	bool bFound = false;
	for (int i = 0; i < m_aHosts.GetCount(); i++)
	{
		pHostData = m_aHosts.GetItem(i);
		if (pHostData->bSFTPMode == bSFTP &&
			pHostData->pDirItem->strName.Compare(lpszHost) == 0 &&
			pHostData->nPort == nPort)
		{
			bFound = true;
			break;
		}
	}
	return bFound ? pHostData : NULL;
}

bool CMainDLL::FileDialog(bool bOpen, CMyStringW& rstrFileName, CMyWindow* pWndOwner)
{
	BOOL bRet;
	if (m_bUseOFNUnicode)
	{
		m_ofnW.lpstrFile = rstrFileName.GetBuffer(MAX_PATH);
		m_ofnW.nMaxFile = MAX_PATH;
		if (pWndOwner)
			m_ofnW.hwndOwner = pWndOwner->m_hWnd;
		else
			m_ofnW.hwndOwner = NULL;
			//m_ofnW.hwndOwner = m_pMainWnd->m_hWnd;
		m_ofnW.lpstrFilter = m_strFilter;
	}
	else
	{
		m_ofnA.lpstrFile = rstrFileName.GetBufferA(MAX_PATH);
		m_ofnA.nMaxFile = MAX_PATH;
		if (pWndOwner)
			m_ofnA.hwndOwner = pWndOwner->m_hWnd;
		else
			m_ofnA.hwndOwner = NULL;
			//m_ofnA.hwndOwner = m_pMainWnd->m_hWnd;
		m_ofnA.lpstrFilter = m_strFilter;
	}

	if (bOpen)
	{
		if (m_bUseOFNUnicode)
		{
			m_ofnW.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
			bRet = ::GetOpenFileNameW(&m_ofnW);
			//if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
			//{
			//	register HWND h = m_ofnW.hwndOwner;
			//	memset(&m_ofnA, 0, sizeof(m_ofnA));
			//	m_ofnA.lStructSize = OPENFILENAME_SIZE_VERSION_400A;
			//	m_ofnA.hwndOwner = h;
			//	m_ofnA.lpstrFile = rstrFileName.GetBufferA(MAX_PATH);
			//	m_ofnA.nMaxFile = MAX_PATH;
			//	m_ofnA.lpstrFilter = m_strFilter;
			//	m_bUseOFNUnicode = false;
			//}
		}
		if (!m_bUseOFNUnicode)
		{
			m_ofnA.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
			bRet = ::GetOpenFileNameA(&m_ofnA);
		}
	}
	else
	{
		if (m_bUseOFNUnicode)
		{
			m_ofnW.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
			bRet = ::GetSaveFileNameW(&m_ofnW);
			if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
			{
				register HWND h = m_ofnW.hwndOwner;
				memset(&m_ofnA, 0, sizeof(m_ofnA));
				m_ofnA.lStructSize = OPENFILENAME_SIZE_VERSION_400A;
				m_ofnA.hwndOwner = h;
				m_ofnA.lpstrFile = rstrFileName.GetBufferA(MAX_PATH);
				m_ofnA.nMaxFile = MAX_PATH;
				m_ofnA.lpstrFilter = m_strFilter;
				m_bUseOFNUnicode = false;
			}
		}
		if (!m_bUseOFNUnicode)
		{
			m_ofnA.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
			bRet = ::GetSaveFileNameA(&m_ofnA);
		}
	}
	if (m_bUseOFNUnicode)
		rstrFileName.ReleaseBuffer();
	else
		rstrFileName.ReleaseBufferA();
	return bRet != 0;
}

struct CBrowseInfoData
{
	bool bShown;
	CMyStringW* pstr;
};

static LRESULT CALLBACK BrowseForFolderCallbackA(HWND hWnd, UINT message, LPARAM lParam, CBrowseInfoData FAR* lpData)
{
	if (message == BFFM_INITIALIZED)
	{
		lpData->bShown = true;
		CMyStringW* pstr = lpData->pstr;
		if (pstr && !pstr->IsEmpty())
			::SendMessage(hWnd, BFFM_SETSELECTIONA, (WPARAM) TRUE, (LPARAM)(LPCSTR) *pstr);
	}
	return 0;
}

static LRESULT CALLBACK BrowseForFolderCallbackW(HWND hWnd, UINT message, LPARAM lParam, CBrowseInfoData FAR* lpData)
{
	if (message == BFFM_INITIALIZED)
	{
		lpData->bShown = true;
		CMyStringW* pstr = lpData->pstr;
		if (pstr && !pstr->IsEmpty())
			::SendMessage(hWnd, BFFM_SETSELECTIONW, (WPARAM) TRUE, (LPARAM)(LPCWSTR) *pstr);
	}
	return 0;
}

bool CMainDLL::FolderDialog(CMyStringW& rstrDirectoryName, CMyWindow* pWndOwner)
{
	CBrowseInfoData data;
	CMyStringW strTitle(MAKEINTRESOURCEW(IDS_FOLDER_DIALOG));
	PIDLIST_ABSOLUTE pidlRet;
	data.bShown = false;
	data.pstr = &rstrDirectoryName;
	if (m_bUseBIUnicode)
	{
		m_biW.hwndOwner = pWndOwner->GetSafeHwnd();
		m_biW.ulFlags = BIF_RETURNONLYFSDIRS;
		m_biW.pszDisplayName = rstrDirectoryName.GetBuffer(MAX_PATH);
		m_biW.lpszTitle = strTitle;
		m_biW.lpfn = (BFFCALLBACK) BrowseForFolderCallbackW;
		m_biW.lParam = (LPARAM)(CBrowseInfoData FAR*) &data;
		pidlRet = ::SHBrowseForFolderW(&m_biW);
		if (!pidlRet && !data.bShown)
			m_bUseBIUnicode = false;
	}
	if (!m_bUseBIUnicode)
	{
		m_biA.hwndOwner = pWndOwner->GetSafeHwnd();
		m_biA.ulFlags = BIF_RETURNONLYFSDIRS;
		m_biA.pszDisplayName = rstrDirectoryName.GetBufferA(MAX_PATH);
		m_biA.lpszTitle = strTitle;
		m_biA.lpfn = (BFFCALLBACK) BrowseForFolderCallbackA;
		m_biA.lParam = (LPARAM)(CBrowseInfoData FAR*) &data;
		pidlRet = ::SHBrowseForFolderA(&m_biA);
	}
	if (!pidlRet)
		return false;
	if (!::SHGetPathFromIDListW(pidlRet, rstrDirectoryName.GetBuffer(MAX_PATH)))
	{
		::SHGetPathFromIDListA(pidlRet, rstrDirectoryName.GetBufferA(MAX_PATH));
		rstrDirectoryName.ReleaseBufferA();
	}
	else
		rstrDirectoryName.ReleaseBuffer();
	::CoTaskMemFree(pidlRet);
	return true;
}

void CMainDLL::MultipleErrorMsgBox(HWND hWndOwner, const CMyStringArrayW& astrMessages)
{
	switch (astrMessages.GetCount())
	{
		case 0:
			break;
		case 1:
			::MyMessageBoxW(hWndOwner, astrMessages.GetItem(0), NULL, MB_ICONHAND);
			break;
		default:
		{
			(CMultipleErrorDialog(astrMessages)).ModalDialogW(hWndOwner);
		}
		break;
	}
}

struct CMyTimerData
{
	PFNEASYSFTPTIMERPROC pfnTimerProc;
	LPARAM lParam;
};

static void CALLBACK MyTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	CMyTimerData* pData = (CMyTimerData*) idEvent;
	PFNEASYSFTPTIMERPROC pfnTimerProc = pData->pfnTimerProc;
	LPARAM lParam = pData->lParam;

	pfnTimerProc(idEvent, lParam);
}

UINT_PTR CMainDLL::RegisterTimer(DWORD dwSpan, PFNEASYSFTPTIMERPROC pfnTimerProc, LPARAM lParam)
{
	if (!m_hWndTimer)
	{
		m_hWndTimer = _MyCreateTimerWindow();
		if (!m_hWndTimer)
			return 0;
	}
	CMyTimerData* pData = (CMyTimerData*) malloc(sizeof(CMyTimerData));
	pData->pfnTimerProc = pfnTimerProc;
	pData->lParam = lParam;
	if (!::SetTimer(m_hWndTimer, (UINT_PTR) pData, (UINT) dwSpan, (TIMERPROC) MyTimerProc))
	{
		free(pData);
		if (!m_arrTimers.GetCount())
		{
			::DestroyWindow(m_hWndTimer);
			m_hWndTimer = NULL;
		}
		return 0;
	}
	m_arrTimers.Add(pData);
	return (UINT_PTR) pData;
}

void CMainDLL::UnregisterTimer(UINT_PTR idTimer)
{
	CMyTimerData* pData = (CMyTimerData*) idTimer;
	::KillTimer(m_hWndTimer, (UINT_PTR) pData);
	int i = m_arrTimers.FindItem(pData);
	if (i >= 0)
		m_arrTimers.RemoveItem(i);
	free(pData);

	if (!m_arrTimers.GetCount())
	{
		::DestroyWindow(m_hWndTimer);
		m_hWndTimer = NULL;
	}
}

void CMainDLL::PlaceClipboardData(IDataObject* pObject)
{
	//if (m_pObjectOnClipboard)
	//	m_pObjectOnClipboard->Release();
	::OleSetClipboard(pObject);
	m_pObjectOnClipboard = pObject;
	pObject->AddRef();
}

bool CMainDLL::CheckClipboardForMoved(IDataObject* pObject, DWORD dwEffects)
{
	if (m_pObjectOnClipboard != pObject)
		return false;

	if (dwEffects == DROPEFFECT_MOVE && ::OleIsCurrentClipboard(m_pObjectOnClipboard) == S_OK)
		::OleSetClipboard(NULL);
	//m_pObjectOnClipboard->Release();
	m_pObjectOnClipboard = NULL;
	return true;
}

void CMainDLL::AddReference(CEasySFTPFolderRoot* pRoot)
{
	if (!m_bEnableRootRefs)
		return;
	::EnterCriticalSection(&m_csRootRefs);
	m_aRootRefs.Add(pRoot);
	::LeaveCriticalSection(&m_csRootRefs);
}

void CMainDLL::RemoveReference(CEasySFTPFolderRoot* pRoot)
{
	if (!m_bEnableRootRefs)
		return;
	::EnterCriticalSection(&m_csRootRefs);
	int n = m_aRootRefs.FindItem(pRoot);
	if (n >= 0)
		m_aRootRefs.RemoveItem(n);
	::LeaveCriticalSection(&m_csRootRefs);
}

void CMainDLL::MyChangeNotify(LONG wEventId, UINT uFlags, PIDLIST_ABSOLUTE pidl1, PIDLIST_ABSOLUTE pidl2)
{
	::SHChangeNotify(wEventId, uFlags, pidl1, pidl2);
	//if (m_bEmulateRegMode)
	//{
	//	CMySimpleArray<CEasySFTPFolderRoot*> aRefs;
	//	::EnterCriticalSection(&m_csRootRefs);
	//	aRefs.SetCount(m_aRootRefs.GetCount());
	//	for (int n = 0; n < m_aRootRefs.GetCount(); n++)
	//	{
	//		CEasySFTPFolderRoot* pRoot = m_aRootRefs.GetItem(n);
	//		pRoot->AddRef();
	//		aRefs.SetItem(n, pRoot);

	//		if (pidl1 && ::IsMatchParentIDList(pRoot->m_pidlMe, pidl1))
	//		{
	//			pRoot->UpdateItem
	//		}
	//	}
	//	::LeaveCriticalSection(&m_csRootRefs);
	//	for (int n = 0; n < aRefs.GetCount(); n++)
	//		aRefs.GetItem()->Release();
	//}
}

void CMainDLL::GetTemporaryFileName(LPCWSTR lpszFileName, CMyStringW& rstrResult)
{
	CMyStringW strDir;
	if (!::MyIsDirectoryW(m_strTempDirectory))
		::MyCreateDirectoryW(m_strTempDirectory, NULL);
	if (!::GetTempFileNameW(m_strTempDirectory, L"ESX", 0, strDir.GetBuffer(MAX_PATH)))
	{
		bool bRet = (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED);
		if (bRet)
		{
			bRet = (::GetTempFileNameA(m_strTempDirectory, "ESX", 0, strDir.GetBufferA(MAX_PATH)) != 0);
			if (bRet)
				strDir.ReleaseBufferA();
			else
				strDir.Empty();
		}
		else
			strDir.Empty();
	}
	else
		strDir.ReleaseBuffer();
	::MyGetFullPathStringW(strDir, lpszFileName, rstrResult);
	if (strDir.IsEmpty())
		m_arrTempFileDirectories.Add(rstrResult);
	else
	{
		if (::MyIsExistFileW(strDir))
			::MyDeleteFileW(strDir);
		::MyCreateDirectoryW(strDir, NULL);
		m_arrTempFileDirectories.Add(strDir);
	}
}

static const UUID UUID_EasySFTPLock = CLSID_EasySFTP;

void CMainDLL::SetAttachmentLock(LPCWSTR lpszFileName, LPCWSTR lpszURL)
{
	IAttachmentExecute* pExecute;
	HRESULT hr = ::CoCreateInstance(CLSID_AttachmentServices, NULL, CLSCTX_INPROC_SERVER,
		IID_IAttachmentExecute, (void**) &pExecute);
	if (SUCCEEDED(hr))
	{
		pExecute->SetClientGuid(UUID_EasySFTPLock);
		hr = pExecute->SetLocalPath(lpszFileName);
		if (SUCCEEDED(hr))
		{
			if (lpszURL && *lpszURL)
				pExecute->SetSource(lpszURL);
			pExecute->Save();
		}
		pExecute->Release();
	}
}

////////////////////////////////////////////////////////////////////////////////

EXTERN_C PITEMID_CHILD __stdcall CreateRootCommandItem(IMalloc* pMalloc, WORD wID)
{
	CSFTPCommandItem* pItem;
	SIZE_T nSize;

	nSize = sizeof(CSFTPCommandItem) - (sizeof(DELEGATEITEMID) - sizeof(BYTE));
	if (nSize > 65535)
		return NULL;
	// IMalloc::Alloc に渡すのは中身のサイズのみ
	pItem = (CSFTPCommandItem*) pMalloc->Alloc(nSize);
	if (!pItem)
		return NULL;
	pItem->uSignature = SFTP_HOST_ITEM_SIGNATURE;
	pItem->wID = wID;
	pItem->wReserved = 0;
	((ITEMIDLIST UNALIGNED*) (((LPBYTE) pItem) + pItem->cbSize))->mkid.cb = 0;
	return (PITEMID_CHILD) pItem;
}

EXTERN_C PITEMID_CHILD __stdcall CreateHostItem(IMalloc* pMalloc, bool bSFTPMode, WORD nPort, LPCWSTR lpszHostName)
{
	CSFTPHostItem* pItem;
	SIZE_T nSize;

	if (!nPort)
		return NULL;

	nSize = sizeof(CSFTPHostItem) - (sizeof(DELEGATEITEMID) - sizeof(BYTE))
		- sizeof(WCHAR) + (sizeof(WCHAR) * wcslen(lpszHostName));
	if (nSize > 65535)
		return NULL;
	// IMalloc::Alloc に渡すのは中身のサイズのみ
	pItem = (CSFTPHostItem*) pMalloc->Alloc(nSize);
	if (!pItem)
		return NULL;
	pItem->uSignature = SFTP_HOST_ITEM_SIGNATURE;
	pItem->bSFTP = bSFTPMode;
	pItem->nPort = nPort;
	memcpy(pItem->wchHostName, lpszHostName,
		(nSize - (sizeof(CSFTPHostItem) - (sizeof(DELEGATEITEMID) - sizeof(BYTE)) - sizeof(WCHAR))));
	((ITEMIDLIST UNALIGNED*) (((LPBYTE) pItem) + pItem->cbSize))->mkid.cb = 0;
	return (PITEMID_CHILD) pItem;
}

EXTERN_C PITEMID_CHILD __stdcall CreateFileItem(IMalloc* pMalloc, CFTPFileItem* pFTPItem)
{
	CSFTPFileItem* pItem;
	SIZE_T nSize;

	nSize = sizeof(CSFTPFileItem) - (sizeof(DELEGATEITEMID) - sizeof(BYTE))
		- sizeof(WCHAR) + (sizeof(WCHAR) * pFTPItem->strFileName.GetLength());
	if (nSize > 65535)
		return NULL;
	// IMalloc::Alloc に渡すのは中身のサイズのみ
	pItem = (CSFTPFileItem*) pMalloc->Alloc(nSize);
	if (!pItem)
		return NULL;
	pItem->uSignature = SFTP_FILE_ITEM_SIGNATURE;
#ifdef _DEBUG
	pItem->_padding = 0xCCCC;
	pItem->_padding2 = 0xDDDD;
#else
	pItem->_padding = 0;
	pItem->_padding2 = 0;
#endif
	//pItem->bHasAttribute = true;
	//pItem->bIsDirectory = pFTPItem->IsDirectory();
	//pItem->bIsHidden = pFTPItem->IsHidden();
	//pItem->bIsShortcut = pFTPItem->IsShortcut();
	memcpy(pItem->wchFileName, (LPCWSTR) pFTPItem->strFileName,
		(nSize - (sizeof(CSFTPFileItem) - (sizeof(DELEGATEITEMID) - sizeof(BYTE)) - sizeof(WCHAR))));
	((ITEMIDLIST UNALIGNED*) (((LPBYTE) pItem) + pItem->cbSize))->mkid.cb = 0;
	return (PITEMID_CHILD) pItem;
}

EXTERN_C PITEMID_CHILD __stdcall CreateDummyFileItem(IMalloc* pMalloc, LPCWSTR lpszFileName)
{
	CSFTPFileItem* pItem;
	SIZE_T nSize;

	nSize = sizeof(CSFTPFileItem) - (sizeof(DELEGATEITEMID) - sizeof(BYTE))
		- sizeof(WCHAR) + (sizeof(WCHAR) * wcslen(lpszFileName));
	if (nSize > 65535)
		return NULL;
	// IMalloc::Alloc に渡すのは中身のサイズのみ
	pItem = (CSFTPFileItem*) pMalloc->Alloc(nSize);
	if (!pItem)
		return NULL;
	pItem->uSignature = SFTP_FILE_ITEM_SIGNATURE;
#ifdef _DEBUG
	pItem->_padding = 0xCCCC;
	pItem->_padding2 = 0xDDDD;
#else
	pItem->_padding = 0;
	pItem->_padding2 = 0;
#endif
	//pItem->bHasAttribute = false;
	//pItem->bIsDirectory = pItem->bIsHidden = pItem->bIsShortcut = false;
	memcpy(pItem->wchFileName, lpszFileName,
		(nSize - (sizeof(CSFTPFileItem) - (sizeof(DELEGATEITEMID) - sizeof(BYTE)) - sizeof(WCHAR))));
	((ITEMIDLIST UNALIGNED*) (((LPBYTE) pItem) + pItem->cbSize))->mkid.cb = 0;
	return (PITEMID_CHILD) pItem;
}

EXTERN_C PIDLIST_RELATIVE __stdcall CreateFullPathFileItem(IMalloc* pMalloc, LPCWSTR lpszFileName)
{
	LPCWSTR lpw;
	PIDLIST_RELATIVE pidl = NULL;
	PITEMID_CHILD pidlChild;
	if (*lpszFileName == L'/')
		lpszFileName++;
	if (!*lpszFileName)
		return NULL;
	CMyStringW str;
	while (true)
	{
		lpw = wcschr(lpszFileName, L'/');
		if (lpw)
			str.SetString(lpszFileName, (DWORD) (((DWORD_PTR) lpw) - ((DWORD_PTR) lpszFileName)) / sizeof(WCHAR));
		else
			str = lpszFileName;
		pidlChild = CreateDummyFileItem(pMalloc, str);
		if (!pidlChild)
		{
			if (pidl)
				::CoTaskMemFree(pidl);
			return NULL;
		}
		if (!pidl)
			pidl = (PIDLIST_RELATIVE) pidlChild;
		else
		{
			PIDLIST_RELATIVE p = (PIDLIST_RELATIVE) ::AppendItemIDList((PCUIDLIST_ABSOLUTE) pidl, pidlChild);
			::CoTaskMemFree(pidlChild);
			::CoTaskMemFree(pidl);
			if (!p)
				return NULL;
			pidl = p;
		}
		if (!lpw)
			break;
		lpszFileName = lpw + 1;
	}
	return pidl;
}

extern "C++" bool __stdcall PickupRootCommandItemID(PCUITEMID_CHILD pidlHostItem, WORD& rwID)
{
	const CSFTPCommandItem UNALIGNED* pItem = (const CSFTPCommandItem UNALIGNED*) pidlHostItem;
	if (pItem->uSignature != SFTP_HOST_ITEM_SIGNATURE)
		return false;
	if (pItem->wReserved)
		return false;
	rwID = pItem->wID;
	return true;
}

extern "C++" bool __stdcall PickupHostName(PCUITEMID_CHILD pidlHostItem, CMyStringW& rstrHostName)
{
	const CSFTPHostItem UNALIGNED* pItem = (const CSFTPHostItem UNALIGNED*) pidlHostItem;
	if (pItem->uSignature != SFTP_HOST_ITEM_SIGNATURE)
		return false;
	if (!pItem->nPort)
		return false;
	rstrHostName.SetString(pItem->wchHostName,
		(pItem->cbInner - (sizeof(CSFTPHostItem) - (sizeof(DELEGATEITEMID) - sizeof(BYTE))) + sizeof(WCHAR)) / sizeof(WCHAR));
	return true;
}

extern "C++" bool __stdcall PickupFileName(PCUITEMID_CHILD pidlFileItem, CMyStringW& rstrFileName)
{
	const CSFTPFileItem UNALIGNED* pItem = (const CSFTPFileItem UNALIGNED*) pidlFileItem;
	if (pItem->uSignature != SFTP_FILE_ITEM_SIGNATURE)
		return false;
	rstrFileName.SetString(pItem->wchFileName,
		(pItem->cbInner - (sizeof(CSFTPFileItem) - (sizeof(DELEGATEITEMID) - sizeof(BYTE))) + sizeof(WCHAR)) / sizeof(WCHAR));
	return true;
}
