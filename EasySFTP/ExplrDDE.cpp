/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 ExplrDDE.cpp - implementations of CMainWindow (DDE methods)
 */

#include "stdafx.h"
#include "EasySFTP.h"
#include "ExplrDDE.h"
#include "MainWnd.h"

#include "IDList.h"

/**************************************************************************
   global variables and definitions
**************************************************************************/

static TCHAR g_szOpenFolder[MAX_PATH];
static TCHAR g_szExploreFolder[MAX_PATH];
static TCHAR g_szFoldersApp[MAX_PATH];
static TCHAR g_szFoldersTopic[MAX_PATH];
static TCHAR g_szFindFolder[MAX_PATH];

#define ISSIGN(c)  ((c) == '-' || (c) == '+')
#define ISDIGIT(c)  ((c) >= '0' && (c) <= '9' || ISSIGN((c)))

typedef LPVOID (WINAPI *PFNSHLOCKSHARED)(HANDLE, DWORD);
typedef BOOL (WINAPI *PFNSHUNLOCKSHARED)(LPVOID);
typedef LPVOID (WINAPI *PFNSHFREESHARED)(HANDLE, DWORD);
typedef BOOL (WINAPI *PFNFREE)(LPVOID);

#define NO_COMMAND      0
#define VIEW_COMMAND    1
#define EXPLORE_COMMAND 2
#define FIND_COMMAND    3

static UINT __stdcall ParseDDECommand(LPCWSTR pszCommand, 
	CMyStringW& rstrPath, 
	PIDLIST_ABSOLUTE* ppidl,
	LPINT pnShow);
static int __stdcall GetCommandTypeFromDDEString(LPCWSTR pszCommand);
static BOOL __stdcall GetPathFromDDEString(LPCWSTR pszCommand, CMyStringW& rstrPath);
static PIDLIST_ABSOLUTE __stdcall GetPidlFromDDEString(LPCWSTR pszCommand);
static int __stdcall GetShowCmdFromDDEString(LPCWSTR pszCommand);

/**************************************************************************

   GetDDEVariables()

**************************************************************************/

extern "C" BOOL __stdcall GetDDEVariables()
{
	LRESULT  lRes;
	HKEY     hKey;
	TCHAR    szTemp[MAX_PATH];
	DWORD    dwSize;
	DWORD    dwType;

	//get the DDE application string
	lRes = RegOpenKeyEx(HKEY_CLASSES_ROOT,
		_T("Folder\\shell\\open\\ddeexec\\application"),
		0,
		KEY_READ,
		&hKey);

	if (lRes != NOERROR)
		return FALSE;

	dwSize = sizeof(TCHAR) * MAX_PATH;
	lRes = RegQueryValueEx(hKey,
		NULL,
		NULL,
		&dwType,
		(LPBYTE) szTemp,
		&dwSize);

	RegCloseKey(hKey);

	if (lRes == NOERROR)
		_tcscpy_s(g_szFoldersApp, szTemp);

	//get the DDE topic string
	lRes = RegOpenKeyEx(HKEY_CLASSES_ROOT,
		_T("Folder\\shell\\open\\ddeexec\\topic"),
		0,
		KEY_READ,
		&hKey);

	if (lRes != NOERROR)
		return FALSE;

	dwSize = sizeof(TCHAR) * MAX_PATH;
	lRes = RegQueryValueEx(hKey,
		NULL,
		NULL,
		&dwType,
		(LPBYTE) szTemp,
		&dwSize);

	RegCloseKey(hKey);

	if (lRes == NOERROR)
		_tcscpy_s(g_szFoldersTopic, szTemp);

	//get the open folder command string
	lRes = RegOpenKeyEx(HKEY_CLASSES_ROOT,
		_T("Folder\\shell\\open\\ddeexec"),
		0,
		KEY_READ,
		&hKey);

	if (lRes != NOERROR)
		return FALSE;

	dwSize = sizeof(TCHAR) * MAX_PATH;
	lRes = RegQueryValueEx(hKey,
		NULL,
		NULL,
		&dwType,
		(LPBYTE) szTemp,
		&dwSize);

	RegCloseKey(hKey);

	if (lRes == NOERROR)
	{
		LPTSTR pszStart;
		LPTSTR pszEnd;

		//find the beginning of the actual command string
		for (pszStart = szTemp; *pszStart; pszStart++)
		{
			if (_istlead(*pszStart))
				pszStart++;
			else if (*pszStart == _T('['))
			{
				pszStart++;
				break;
			}
		}

		//find the end of the command string
		for (pszEnd = pszStart; *pszEnd; pszEnd++)
		{
			if (_istlead(*pszEnd))
				pszEnd++;
			else if (*pszEnd == _T('('))
			{
				*pszEnd = 0;
				break;
			}
		}

		_tcscpy_s(g_szOpenFolder, pszStart);
	}
	else
		return FALSE;

	//get the explore folder command string
	lRes = RegOpenKeyEx(HKEY_CLASSES_ROOT,
		_T("Folder\\shell\\explore\\ddeexec"),
		0,
		KEY_READ,
		&hKey);

	if (lRes != NOERROR)
		return FALSE;

	dwSize = sizeof(TCHAR) * MAX_PATH;
	lRes = RegQueryValueEx(hKey,
		NULL,
		NULL,
		&dwType,
		(LPBYTE) szTemp,
		&dwSize);

	RegCloseKey(hKey);

	if (lRes == NOERROR)
	{
		LPTSTR pszStart;
		LPTSTR pszEnd;

		//find the beginning of the actual command string
		for (pszStart = szTemp; *pszStart; pszStart++)
		{
			if (_istlead(*pszStart))
				pszStart++;
			else if (*pszStart == _T('['))
			{
				pszStart++;
				break;
			}
		}

		//find the end of the command string
		for (pszEnd = pszStart; *pszEnd; pszEnd++)
		{
			if (_istlead(*pszEnd))
				pszEnd++;
			else if (*pszEnd == _T('('))
			{
				*pszEnd = 0;
				break;
			}
		}

		_tcscpy_s(g_szExploreFolder, pszStart);
	}
	else
		return FALSE;

	//get the find directory command string
	lRes = RegOpenKeyEx( HKEY_CLASSES_ROOT,
		_T("Directory\\shell\\find\\ddeexec"),
		0,
		KEY_READ,
		&hKey);

	if (lRes != NOERROR)
		return FALSE;

	dwSize = sizeof(TCHAR) * MAX_PATH;
	lRes = RegQueryValueEx( hKey,
		NULL,
		NULL,
		&dwType,
		(LPBYTE) szTemp,
		&dwSize);

	RegCloseKey(hKey);

	if (lRes == NOERROR)
	{
		LPTSTR pszStart;
		LPTSTR pszEnd;

		//find the beginning of the actual command string
		for (pszStart = szTemp; *pszStart; pszStart++)
		{
			if (_istlead(*pszStart))
				pszStart++;
			else if (*pszStart == _T('['))
			{
				pszStart++;
				break;
			}
		}

		//find the end of the command string
		for (pszEnd = pszStart; *pszEnd; pszEnd++)
		{
			if (_istlead(*pszEnd))
				pszEnd++;
			else if (*pszEnd == _T('('))
			{
				*pszEnd = 0;
				break;
			}
		}

		_tcscpy_s(g_szFindFolder, pszStart);
	}
	else
		return FALSE;

	return TRUE;
}

/**************************************************************************

   ParseDDECommand()

**************************************************************************/

static UINT __stdcall ParseDDECommand(LPCWSTR pszCommand, 
	CMyStringW& rstrPath, 
	PIDLIST_ABSOLUTE* ppidl,
	LPINT pnShow)
{
	UINT     uCommand;

	*ppidl = NULL;
	*pnShow = 0;

	uCommand = GetCommandTypeFromDDEString(pszCommand);

	/*
	If the command was not recognized, then don't try to free any memory because 
	we have no idea what is actually contained on the command line.
	*/
	if (uCommand != NO_COMMAND)
	{
		GetPathFromDDEString(pszCommand, rstrPath);

		*ppidl = GetPidlFromDDEString(pszCommand);
		*pnShow = GetShowCmdFromDDEString(pszCommand);
	}

	return uCommand;
}

/**************************************************************************

   GetCommandTypeFromDDEString()

**************************************************************************/

static int __stdcall GetCommandTypeFromDDEString(LPCWSTR pszCommand)
{
	CMyStringW strTemp;
	LPCWSTR  pszStart, pszTemp;
	int      nCommand = NO_COMMAND;

	strTemp = pszCommand;

	/*
	Find the end of the command portion if the string by setting the the first 
	'(' character to NULL.
	*/
	for (pszTemp = pszStart = strTemp; *pszTemp; pszTemp++)
	{
		if (*pszTemp == L'(')
		{
			strTemp.ReleaseBuffer((DWORD) (((ULONG_PTR) pszTemp - (ULONG_PTR) pszStart) / sizeof(WCHAR)));
			break;
		}
	}

	/*
	Find the beginning of the command portion by getting the first character 
	after the first '['.
	*/
	for (pszTemp = pszStart = strTemp; *pszTemp; pszTemp++)
	{
		if (*pszTemp == L'[')
		{
			pszTemp++;
			strTemp.DeleteString(0, (DWORD) (((ULONG_PTR) pszTemp - (ULONG_PTR) pszStart) / sizeof(WCHAR)));
			break;
		}
	}

	//check the command
	if (!strTemp.Compare(g_szOpenFolder, true))
		nCommand = VIEW_COMMAND;
	else if (!strTemp.Compare(g_szExploreFolder, true))
		nCommand = EXPLORE_COMMAND;
	else if (!strTemp.Compare(g_szFindFolder, true))
		nCommand = FIND_COMMAND;

	return nCommand;
}

/**************************************************************************

   GetPathFromDDEString()

**************************************************************************/

static BOOL __stdcall GetPathFromDDEString(LPCWSTR pszCommand, CMyStringW& rstrPath)
{
	LPWSTR   pszCopy;
	DWORD    dwChars;

	dwChars = (DWORD) wcslen(pszCommand) + 1;
	pszCopy = (LPWSTR) malloc(sizeof(WCHAR) * dwChars);
	if (pszCopy)
	{
		LPWSTR   pszStart;
		LPWSTR   pszEnd;

		memcpy(pszCopy, pszCommand, sizeof(WCHAR) * dwChars);

		//get the path to the folder if it is present

		//find the first '"' character
		for (pszStart = pszCopy; *pszStart; pszStart++)
		{
			if (*pszStart == L'"')
			{
				pszStart++;
				break;
			}
		}

		//find the next '"' character
		for (pszEnd = pszStart; *pszEnd; pszEnd++)
		{
			if (*pszEnd == L'"')
			{
				*pszEnd = 0;
				break;
			}
		}

		rstrPath = pszStart;

		free(pszCopy);

		if (!rstrPath.IsEmpty())
			return TRUE;
	}

	return FALSE;
}

/**************************************************************************

   GetPidlFromDDEString()

**************************************************************************/

static PIDLIST_ABSOLUTE __stdcall GetPidlFromDDEString(LPCWSTR pszCommand)
{
	LPWSTR         pszCopy;
	DWORD          dwChars;
	PIDLIST_ABSOLUTE pidl = NULL;

	dwChars = (DWORD) wcslen(pszCommand) + 1;
	pszCopy = (LPWSTR) malloc(sizeof(WCHAR) * dwChars);
	if(pszCopy)
	{
		LPWSTR   pszHandle;
		LPWSTR   pszProcessId = NULL;
		LPWSTR   pszEnd;

		memcpy(pszCopy, pszCommand, sizeof(WCHAR) * dwChars);

		//get the PIDL from the command line

		//find the first "," in the command string
		for (pszHandle = pszCopy; *pszHandle; pszHandle++)
		{
			if (*pszHandle == L',')
				break;
		}

		//find the first digit
		while (!ISDIGIT(*pszHandle) && *pszHandle)
		{
			pszHandle++;
		}
		  
		//find the next ':' character, if one exists
		for (pszProcessId = pszHandle; *pszProcessId; pszProcessId++)
		{
			if (*pszProcessId == L':')
			{
				//find the first digit
				while (!ISDIGIT(*pszProcessId) && *pszProcessId)
				{
					pszProcessId++;
				}
				break;
			}
			else if (*pszProcessId == L',')
			{
				pszProcessId = NULL;
				break;
			}
		}

		//NULL terminate the handle portion
		for (pszEnd = pszHandle; *pszEnd; pszEnd++)
		{
			if (!ISDIGIT(*pszEnd))
				break;
		}
		*pszEnd = 0;

		if (pszProcessId)
		{
			//the command line includes a process ID, so parse it accordingly
			HANDLE   hShared;
			DWORD    dwProcessId;

			//NULL terminate the process ID portion
			for (pszEnd = pszProcessId; *pszEnd; pszEnd++)
			{
				if (!ISDIGIT(*pszEnd))
					break;
			}
			*pszEnd = 0;

			hShared = (HANDLE) LongToPtr(_wtol(pszHandle));
			dwProcessId = (DWORD) _wtol(pszProcessId);

			HINSTANCE hinstShell = LoadLibrary(_T("shell32.dll"));

			if (hinstShell)
			{
				PFNSHLOCKSHARED   pfnSHLockShared = (PFNSHLOCKSHARED) GetProcAddress(hinstShell, (LPSTR) 521);
				PFNSHUNLOCKSHARED pfnSHUnlockShared = (PFNSHUNLOCKSHARED) GetProcAddress(hinstShell, (LPSTR) 522);
				PFNSHFREESHARED   pfnSHFreeShared = (PFNSHLOCKSHARED) GetProcAddress(hinstShell, (LPSTR) 523);

				if (pfnSHLockShared && pfnSHUnlockShared && pfnSHFreeShared)
				{
					PIDLIST_ABSOLUTE pidlShared;

					pidlShared = (PIDLIST_ABSOLUTE) (*pfnSHLockShared)(hShared, dwProcessId);

					if(pidlShared)
					{
						//make a local copy of the PIDL
						pidl = (PIDLIST_ABSOLUTE) DuplicateItemIDList(pidlShared);

						(*pfnSHUnlockShared)(pidlShared);
					}

					(*pfnSHFreeShared)(hShared, dwProcessId);
				}

				FreeLibrary(hinstShell);
			}
		}
		else
		{
			//the command line includes a global PIDL
			PIDLIST_ABSOLUTE pidlGlobal;

			pidlGlobal = (PIDLIST_ABSOLUTE) LongToPtr(_wtol(pszHandle));

			pidl = (PIDLIST_ABSOLUTE) DuplicateItemIDList(pidlGlobal);

			/*
			The shared PIDL was allocated by the shell using a heap that the shell 
			maintains. The only way to free this memory is to call the Free 
			function in COMCTL32.DLL at ordinal 73.
			*/
			HINSTANCE   hinstCC = LoadLibrary(_T("comctl32.dll"));

			if (hinstCC)
			{
				PFNFREE  pfnFree = (PFNFREE) GetProcAddress(hinstCC, (LPSTR) 73);

				if (pfnFree)
					(*pfnFree)(pidlGlobal);

				FreeLibrary(hinstCC);
			}
		}

		free(pszCopy);
	}

	return pidl;
}

/**************************************************************************

   GetShowCmdFromDDEString()

**************************************************************************/

static int __stdcall GetShowCmdFromDDEString(LPCWSTR pszCommand)
{
	LPWSTR   pszCopy;
	DWORD    dwChars;
	int      nShow = SW_SHOWDEFAULT;

	dwChars = (DWORD) wcslen(pszCommand) + 1;
	pszCopy = (LPWSTR) malloc(sizeof(WCHAR) * dwChars);
	if(pszCopy)
	{
		LPWSTR   pszStart;
		LPWSTR   pszEnd;
		BOOL     fFoundOne;

		memcpy(pszCopy, pszCommand, sizeof(WCHAR) * dwChars);

		//find the second ","
		for (pszStart = pszCopy, fFoundOne = FALSE; *pszStart; pszStart++)
		{
			if (*pszStart == L',')
			{
				if (fFoundOne)
					break;
				else
					fFoundOne = TRUE;
			}
		}

		//get to the actual digits of the show command
		while (!ISDIGIT(*pszStart) && *pszStart)
		{
			pszStart++;
		}

		//find the end of the start command digits
		pszEnd = pszStart;
		while (ISDIGIT(*pszEnd) && *pszEnd)
		{
			pszEnd++;
		}

		*pszEnd = 0;

		if (*pszStart)
			nShow = _wtoi(pszStart);

		free(pszCopy);
	}

	return nShow;
}

////////////////////////////////////////////////////////////////////////////////

LRESULT CMainWindow::OnDDEInitialize(WPARAM wParam, LPARAM lParam)
{
	HWND hWndClient = (HWND) wParam;
	ATOM aInApp;
	ATOM aInTopic;
	ATOM aOutApp = 0;
	ATOM aOutTopic = 0;

	if (m_bNoRespondToDDE)
		return 0;

	/*
	 Only handle DDE conversations from windows that are in our process. This 
	 prevents us from catching ShellExecutes from elsewhere in the shell, such 
	 as from Start - Run.
	*/
	DWORD dwProcessID;
	::GetWindowThreadProcessId(hWndClient, &dwProcessID);
	if (::GetCurrentProcessId() != dwProcessID)
		return 0;

	aInApp = LOWORD(lParam);
	aInTopic = HIWORD(lParam);

	aOutApp = ::GlobalAddAtom(g_szFoldersApp);
	aOutTopic = ::GlobalAddAtom(g_szFoldersTopic);

	if (aOutApp && aOutTopic && (aOutApp == aInApp) && (aOutTopic == aInTopic))
	{
		//this is the conversation we are looking for
		::SendMessage(hWndClient, WM_DDE_ACK, (WPARAM) m_hWnd, MAKELPARAM(aOutApp, aOutTopic));
	}

	if (aOutApp)
		GlobalDeleteAtom(aOutApp);

	if (aOutTopic)
		GlobalDeleteAtom(aOutTopic);

	return 0;
}

LRESULT CMainWindow::OnDDETerminate(WPARAM wParam, LPARAM lParam)
{
	::PostMessage((HWND) wParam, WM_DDE_TERMINATE, (WPARAM) m_hWnd, 0);

	return 0;
}

LRESULT CMainWindow::OnDDEExecute(WPARAM wParam, LPARAM lParam)
{
	HWND     hWndClient = (HWND) wParam;
	UINT_PTR uLo;
	HGLOBAL  hgMem;

	if (::UnpackDDElParam(WM_DDE_EXECUTE,
		lParam,
		&uLo,
		(PUINT_PTR) &hgMem))
	{
		DDEACK ddeAck;
		LPVOID pvCommand = ::GlobalLock(hgMem);

		memset(&ddeAck, 0, sizeof(ddeAck));

		if (pvCommand)
		{
			CMyStringW strPath;
			PIDLIST_ABSOLUTE pidl;
			int nShow;
			UINT uCommand;
			CMyStringW strCommand;

			if (m_bUniWindow && ::IsWindowUnicode(hWndClient))
				strCommand = (LPCWSTR) pvCommand;
			else
				strCommand = (LPCSTR) pvCommand;
			uCommand = ParseDDECommand(strCommand, strPath, &pidl, &nShow);

			switch (uCommand)
			{
				case VIEW_COMMAND:
					if (pidl)
						UpdateCurrentFolderAbsolute(pidl);
					else if (!strPath.IsEmpty())
						UpdateCurrentFolderAbsolute(strPath);

					if (pidl)
						::CoTaskMemFree(pidl);

					ddeAck.fAck = 1;
					break;

				case EXPLORE_COMMAND:
				{
					/*
					We don't support the explore command, so set a flag that will 
					cause ShellExecuteEx to be called when this DDE conversation is 
					terminated. This flag will also prevent us from responding to 
					the DDE conversation that ShellExecuteEx will attempt to 
					establish.
					*/
					m_bNoRespondToDDE = true;

					SHELLEXECUTEINFO  sei;

					memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
					sei.cbSize = sizeof(SHELLEXECUTEINFO);

					if (pidl)
					{
						sei.fMask = SEE_MASK_IDLIST | SEE_MASK_CLASSNAME;
						sei.lpIDList = pidl;
					}
					else if (!strPath.IsEmpty())
					{
						sei.fMask = SEE_MASK_CLASSNAME;
						sei.lpFile = strPath;
					}

					sei.lpClass = _T("folder");
					sei.hwnd = NULL;
					sei.nShow = nShow;
					sei.lpVerb = _T("explore");

					::ShellExecuteEx(&sei);

					m_bNoRespondToDDE = false;

					if (pidl)
						::CoTaskMemFree(pidl);

					ddeAck.fAck = 1;
				}
				break;

				case FIND_COMMAND:
				{
					/*
					We don't support the explore command, so set a flag that will 
					cause ShellExecuteEx to be called when this DDE conversation is 
					terminated. This flag will also prevent us from responding to 
					the DDE conversation that ShellExecuteEx will attempt to 
					establish.
					*/
					m_bNoRespondToDDE = true;

					SHELLEXECUTEINFO  sei;

					memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
					sei.cbSize = sizeof(SHELLEXECUTEINFO);

					if (pidl)
					{
						sei.fMask = SEE_MASK_IDLIST;
						sei.lpIDList = pidl;
					}
					else if (!strPath.IsEmpty())
					{
						sei.fMask = 0;
						sei.lpFile = strPath;
					}

					sei.hwnd = NULL;
					sei.nShow = nShow;
					sei.lpVerb = _T("find");

					::ShellExecuteEx(&sei);

					m_bNoRespondToDDE = false;

					if (pidl)
						::CoTaskMemFree(pidl);

					ddeAck.fAck = 1;
				}
				break;

				case NO_COMMAND:
					break;
			}
		}

		GlobalUnlock(hgMem);

		::PostMessage(hWndClient, 
			WM_DDE_ACK, 
			(WPARAM) m_hWnd, 
			::ReuseDDElParam(lParam, WM_DDE_EXECUTE, WM_DDE_ACK, *((PUINT_PTR) &ddeAck), (UINT_PTR) hgMem));

		//the client will terminate the conversation when it receives the ACK
	}

	return 0;
}
