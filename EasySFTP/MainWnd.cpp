/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 MWndConn.cpp - implementations of CMainWindow methods (general methods)
 */

#include "StdAfx.h"
#include "EasySFTP.h"
#include "MainWnd.h"

#include "CallSync.h"
#include "IDList.h"
//#include "MySocket.h"

//#include "FileStrm.h"
//#include "DragData.h"

//#include "Connect.h"
//#include "LinkDlg.h"
#include "Option.h"

struct CToolBarButtonDef { UINT uID; int iImage; };

static const CToolBarButtonDef s_arrAddrButtons[] = {
	{ ID_VIEW_GO_FOLDER, 0 },
	{ ID_VIEW_PARENT_FOLDER, 1 }
};

static const CToolBarButtonDef s_arrServerAddrButtons[] = {
	{ ID_VIEW_SERVER_GO_FOLDER, 0 },
	{ ID_VIEW_SERVER_PARENT_FOLDER, 1 }
};

static const CToolBarButtonDef s_arrToolBarButtons[] = {
	{ ID_FILE_CONNECT, 0 },
	{ ID_FILE_DISCONNECT, 1 },
	{ 0, -1 },
	{ ID_EDIT_DELETE, 3 },
	{ ID_FILE_NEW_FOLDER, 2 },
	{ 0, -1 },
	{ ID_EDIT_DOWNLOAD, 4 },
	{ ID_EDIT_UPLOAD, 5 },
	{ ID_EDIT_DOWNLOAD_ALL, 6 },
	{ ID_EDIT_UPLOAD_ALL, 7 },
	{ 0, -1 },
	{ ID_TRANSFER_LOCAL_MODE, 8 | 0x10000 },
	{ ID_TRANSFER_SERVER_MODE, 11 | 0x10000 },
	{ 0, -1 },
	{ ID_TRANSFER_MODE_AUTO, 14 },
	{ ID_TRANSFER_MODE_TEXT, 15 },
	{ ID_TRANSFER_MODE_BINARY, 16 }
};

static /*const*/ struct { UINT uID; int nWidth; } s_arrBarParts[] = {
	{ 0, -1 },
	{ ID_STATUS_TEXT, 100 },
	{ ID_STATUS_HOST, 100 }//,
	//{ ID_STATUS_SECURE, 18 }
};

static const UINT s_arrMenuParentIDs[] = {
	FCIDM_MENU_FILE,
	FCIDM_MENU_EDIT,
	FCIDM_MENU_VIEW,
	0,  // Transfer
	FCIDM_MENU_TOOLS,
	FCIDM_MENU_HELP
};

bool __stdcall IsWindowOrChildrenFocused(HWND hWnd, HWND hWndFocus)
{
	while (hWndFocus)
	{
		if (hWnd == hWndFocus)
			return true;
		hWndFocus = ::GetParent(hWndFocus);
	}
	return false;
}

typedef UINT(WINAPI* FnGetDpiForWindow)(HWND hWnd);
typedef HRESULT(WINAPI* FnGetDpiForMonitor)(HMONITOR hMonitor, MONITOR_DPI_TYPE dpiType, UINT* dpiX, UINT* dpiY);
static FnGetDpiForWindow s_pfnGetDpiForWindow = reinterpret_cast<FnGetDpiForWindow>(static_cast<INT_PTR>(-1));
static FnGetDpiForMonitor s_pfnGetDpiForMonitor = reinterpret_cast<FnGetDpiForMonitor>(static_cast<INT_PTR>(-1));

static UINT _MyGetDpiForWindowFromMonitor(HWND hWnd)
{
	if (s_pfnGetDpiForMonitor == reinterpret_cast<FnGetDpiForMonitor>(static_cast<INT_PTR>(-1)))
	{
		s_pfnGetDpiForMonitor = NULL;
		auto hShCore = ::GetModuleHandle(_T("shcore.dll"));
		if (!hShCore)
		{
			hShCore = ::LoadLibrary(_T("shcore.dll"));
		}
		if (hShCore)
		{
			s_pfnGetDpiForMonitor = reinterpret_cast<FnGetDpiForMonitor>(::GetProcAddress(hShCore, "GetDpiForMonitor"));
		}
	}
	if (!s_pfnGetDpiForMonitor)
		return 96;
	auto hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	if (!hMonitor)
		return 96;
	UINT x = 0, y = 0;
	if (FAILED(s_pfnGetDpiForMonitor(hMonitor, MDT_DEFAULT, &x, &y)))
	{
		return 96;
	}
	return y;
}

static UINT _MyGetDpiForWindow(HWND hWnd)
{
	if (s_pfnGetDpiForWindow == reinterpret_cast<FnGetDpiForWindow>(static_cast<INT_PTR>(-1)))
	{
		s_pfnGetDpiForWindow = NULL;
		auto hUser32 = ::GetModuleHandle(_T("user32.dll"));
		if (hUser32)
		{
			s_pfnGetDpiForWindow = reinterpret_cast<FnGetDpiForWindow>(::GetProcAddress(hUser32, "GetDpiForWindow"));
		}
	}
	if (!s_pfnGetDpiForWindow)
	{
		return _MyGetDpiForWindowFromMonitor(hWnd);
	}
	return s_pfnGetDpiForWindow(hWnd);
}

typedef BOOL(WINAPI* FnSystemParametersInfoForDpi)(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni, UINT dpi);
static FnSystemParametersInfoForDpi s_pfnSystemParametersInfoForDpi = reinterpret_cast<FnSystemParametersInfoForDpi>(static_cast<INT_PTR>(-1));

static BOOL _MySystemParametersInfoForDpi(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni, UINT dpi)
{
	if (s_pfnSystemParametersInfoForDpi == reinterpret_cast<FnSystemParametersInfoForDpi>(static_cast<INT_PTR>(-1)))
	{
		s_pfnSystemParametersInfoForDpi = NULL;
		auto hUser32 = ::GetModuleHandle(_T("user32.dll"));
		if (hUser32)
		{
			s_pfnSystemParametersInfoForDpi = reinterpret_cast<FnSystemParametersInfoForDpi>(::GetProcAddress(hUser32, "SystemParametersInfoForDpi"));
		}
	}
	if (!s_pfnSystemParametersInfoForDpi)
		return FALSE;
	return s_pfnSystemParametersInfoForDpi(uiAction, uiParam, pvParam, fWinIni, dpi);
}

////////////////////////////////////////////////////////////////////////////////

//#pragma warning(disable:4355)
CMainWindow::CMainWindow()
//	: m_dlgTransfer(this)
//#pragma warning(default:4355)
{
	m_bWindowCreated = false;
	m_bNoRespondToDDE = false;
	m_bUpdateSetMenu = false;
	m_hWndViewForMenu = NULL;
	m_hIconSecure = NULL;
	//m_pConnection = NULL;
	//m_bLoggedIn = false;
	m_uRef = 1;
	m_hMenu = NULL;
	m_nSplitterWidth = -1;
	//m_pObjectOnClipboard = NULL;
	//m_pUser = NULL;
	//m_bSFTPMode = false;
	//m_pClient = NULL;
	//m_pChannel = NULL;
	//m_bFirstMessageReceived = false;
	//m_hDirFile = NULL;
	//m_pSendFileList = NULL;
	//m_pRecvFileList = NULL;
	//m_bQueueMsgSent = false;
	m_hWndFocusSaved = NULL;
	m_bLocalAddressSelChanged = false;
	m_bServerAddressSelChanged = false;
	////m_bInVerifyDirectory = false;
	//m_nInCreatingLabelEditMode = 0;
	//m_bTextMode = TEXTMODE_NO_CONVERT;
	//m_nServerCharset = scsUTF8;
	//m_nTransferMode = TRANSFER_MODE_AUTO;
	m_uLastStatusTextModeID = 0;
	//::InitializeCriticalSection(&m_csListSendFile);
	m_uDpi = 96;
	m_hFontWindow = NULL;

	//m_wndServerAddress.m_pFolderRoot->Release();
	//theApp.m_pEasySFTPRoot->QueryInterface(IID_IShellFolder, (void**) &m_wndServerAddress.m_pFolderRoot);
}

CMainWindow::~CMainWindow()
{
	//::DeleteCriticalSection(&m_csListSendFile);
	//if (m_pChannel)
	//	m_pChannel->Release();
	//if (m_pClient)
	//	m_pClient->Release();
	//if (m_pConnection)
	//	delete m_pConnection;
	if (m_hFontWindow)
		::DeleteObject(m_hFontWindow);
	if (m_hIconSecure)
		::DestroyIcon(m_hIconSecure);
	//m_pEasySFTPRoot->Release();
	theApp.m_pMainWnd = NULL;
}

HWND CMainWindow::CreateEx()
{
	return CMyWindow::CreateExW(0L, CMainApplication::s_szMainWndClass,
		theApp.m_strTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, theApp.m_hInstance, NULL);
}

void CMainWindow::PostNcDestroy()
{
	InternalRelease();
	::PostQuitMessage(0);
}

static bool __stdcall IsEditBox(HWND hWnd)
{
	TCHAR szBuffer[6];
	::GetClassName(hWnd, szBuffer, 6);
	return (_tcsicmp(szBuffer, _T("Edit")) == 0);
}

bool CMainWindow::PreTranslateMessage(LPMSG lpMsg)
{
	//if (::IsDialogMessage(m_dlgTransfer, lpMsg))
	//	return true;
	if (m_wndListViewLocal.m_pView && IsWindowOrChildrenFocused(m_wndListViewLocal, ::GetFocus()))
	{
		HRESULT hr;
		hr = m_wndListViewLocal.m_pView->TranslateAccelerator(lpMsg);
		if (hr == S_OK)
			return true;
	}
	if (!IsEditBox(lpMsg->hwnd))
	{
		if (::TranslateAccelerator(m_hWnd, m_hAccel, lpMsg))
			return true;
	}
	if ((lpMsg->message == WM_KEYDOWN || lpMsg->message == WM_KEYUP || lpMsg->message == WM_CHAR) &&
		(lpMsg->wParam == VK_RETURN || lpMsg->wParam == VK_ESCAPE))
		return (lpMsg->message == WM_CHAR && lpMsg->wParam == VK_ESCAPE);
	return ::IsDialogMessage(m_hWnd, lpMsg) != 0;
}

bool CMainWindow::OnIdle(long lCount)
{
	//if (m_pChannel && m_pChannel->HasQueue())
	//{
	//	m_pChannel->FlushAllMessages(this);
	//	return true;
	//}
	UpdateToolBarEnable();
	UpdateFileSelection();
	HWND h = ::GetFocus();
	if (h != m_hWndLastFocus)
	{
		m_hWndLastFocus = h;
		UpdateViewStatus(h);
	}
	return false;
}

void CMainWindow::UpdateCurrentFolder(PCUIDLIST_RELATIVE lpidl)
{
	HRESULT hr;

	hr = m_wndListViewLocal.ReplaceView(lpidl);
	if (SUCCEEDED(hr))
	{
		//::SetWindowPos(m_wndListViewLocal, m_wndAddrButtons, 0, 0, 0, 0,
		//	SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		m_wndAddress.ChangeCurrentFolder(m_wndListViewLocal.m_lpidlAbsoluteMe);
		::SetFocus(m_wndListViewLocal);
	}
}

void CMainWindow::UpdateCurrentFolderAbsolute(PCUIDLIST_ABSOLUTE lpidl)
{
	HRESULT hr;

	hr = m_wndListViewLocal.ReplaceViewAbsolute(lpidl);
	if (SUCCEEDED(hr))
	{
		//::SetWindowPos(m_wndListViewLocal, m_wndAddrButtons, 0, 0, 0, 0,
		//	SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		m_wndAddress.ChangeCurrentFolder(m_wndListViewLocal.m_lpidlAbsoluteMe);
		::SetFocus(m_wndListViewLocal);
	}
}

void CMainWindow::UpdateCurrentFolderAbsolute(LPCWSTR lpszPath)
{
	IShellFolder* pDesktop;
	HRESULT hr;
	PIDLIST_ABSOLUTE pidl;

	hr = ::SHGetDesktopFolder(&pDesktop);
	if (FAILED(hr))
		return;
	hr = pDesktop->ParseDisplayName(m_hWnd, NULL, (LPWSTR) lpszPath, NULL, (PIDLIST_RELATIVE*) &pidl, NULL);
	if (SUCCEEDED(hr))
	{
		hr = m_wndListViewLocal.ReplaceViewAbsolute(pidl);
		if (SUCCEEDED(hr))
		{
			//::SetWindowPos(m_wndListViewLocal, m_wndAddrButtons, 0, 0, 0, 0,
			//	SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			m_wndAddress.ChangeCurrentFolder(m_wndListViewLocal.m_lpidlAbsoluteMe);
			::SetFocus(m_wndListViewLocal);
		}
		::CoTaskMemFree(pidl);
	}
	pDesktop->Release();
}

//void CMainWindow::UpdateServerFolder(LPCWSTR lpszPath)
//{
//	CMyStringW str(m_wndServerAddress.m_strDirectory);
//	if (str.GetLength() != 1 || *((LPCWSTR) str) != m_wndServerAddress.m_wchDelimiter)
//		str += m_wndServerAddress.m_wchDelimiter;
//	str += lpszPath;
//	UpdateServerFolderAbsolute(str);
//}
//
//void CMainWindow::UpdateServerFolderAbsolute(LPCWSTR lpszPath)
//{
//	if (m_bSFTPMode)
//	{
//		if (!m_pClient)
//			return;
//
//		if (m_hDirFile)
//			return;
//		//m_bInVerifyDirectory = true;
//		// SSH_FXP_REALPATH を送り、返されたパスで SSH_FXP_OPENDIR する
//		m_uSFTPDirChangeMsgID = m_pChannel->RealPath(lpszPath);
//	}
//	else
//	{
//		if (!m_pConnection)
//			return;
//
//		// CWaitMakeDirData を作成し、リストに追加する
//		// (MKD でも CWD 同じレスポンスコードが返るため、区別できるように行う)
//		CWaitMakeDirData* pDir = new CWaitMakeDirData();
//		pDir->nWaitFlags = 0;
//		pDir->strRemoteDirectory = lpszPath;
//		pDir->pvData = NULL;
//		m_aWaitResponse.Enqueue(pDir);
//
//		//m_bInVerifyDirectory = true;
//		SendCommand(L"CWD", lpszPath);
//	}
//}

void CMainWindow::UpdateServerFolder(PCUIDLIST_RELATIVE lpidl)
{
	HRESULT hr;

	hr = m_wndListViewServer.ReplaceView(lpidl);
	if (SUCCEEDED(hr))
	{
		//::SetWindowPos(m_wndListViewServer, m_wndServerAddrButtons, 0, 0, 0, 0,
		//	SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		m_wndServerAddress.ChangeCurrentFolder(m_wndListViewServer.m_lpidlAbsoluteMe);
		SetServerListenerToMe();
		::SetFocus(m_wndListViewServer);
	}
}

void CMainWindow::UpdateServerFolderAbsolute(PCUIDLIST_ABSOLUTE lpidl)
{
	HRESULT hr;
	//IShellFolder* pFolder;

	//hr = theApp.m_pEasySFTPRoot->QueryInterface(IID_IShellFolder, (void**) &pFolder);
	//if (FAILED(hr))
	//	return;
	//hr = m_wndListViewServer.ReplaceViewAbsolute(lpidl, pFolder);
	//pFolder->Release();
	hr = m_wndListViewServer.ReplaceViewAbsolute(lpidl);
	if (SUCCEEDED(hr))
	{
		//::SetWindowPos(m_wndListViewServer, m_wndServerAddrButtons, 0, 0, 0, 0,
		//	SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		m_wndServerAddress.ChangeCurrentFolder(m_wndListViewServer.m_lpidlAbsoluteMe);
		SetServerListenerToMe();
		::SetFocus(m_wndListViewServer);
	}
}

void CMainWindow::UpdateServerFolderAbsolute(LPCWSTR lpszPath)
{
	IEasySFTPOldDirectory* pDirectory, * pRoot;
	IPersistFolder2* pPersist;
	IShellFolder* pFolder, * pFolder2;
	HRESULT hr;
	PIDLIST_ABSOLUTE pidl, pidl2, pidlBase;

	if (*lpszPath != L'/')
		return;
	if (FAILED(m_wndListViewServer.m_pFolder->QueryInterface(IID_IEasySFTPOldDirectory, (void**) &pDirectory)))
		return;
	hr = pDirectory->GetRootDirectory(&pRoot);
	pDirectory->Release();
	if (FAILED(hr))
		return;
	hr = pRoot->QueryInterface(IID_IShellFolder, (void**) &pFolder);
	pRoot->Release();
	if (FAILED(hr))
		return;
	hr = pFolder->QueryInterface(IID_IPersistFolder2, (void**) &pPersist);
	if (FAILED(hr))
	{
		pFolder->Release();
		return;
	}
	hr = pPersist->GetCurFolder(&pidlBase);
	pPersist->Release();
	if (FAILED(hr))
	{
		pFolder->Release();
		return;
	}
	hr = theApp.m_pEasySFTPRoot->QueryInterface(IID_IShellFolder, (void**) &pFolder2);
	if (FAILED(hr))
	{
		pFolder->Release();
		return;
	}
	lpszPath++;
	hr = pFolder->ParseDisplayName(m_hWnd, NULL, (LPWSTR) lpszPath, NULL, (PIDLIST_RELATIVE*) &pidl2, NULL);
	if (SUCCEEDED(hr))
	{
		pidl = ::AppendItemIDList(pidlBase, (PCUIDLIST_RELATIVE) pidl2);
		::CoTaskMemFree(pidl2);
		::CoTaskMemFree(pidlBase);
		hr = m_wndListViewServer.ReplaceViewAbsolute(pidl, pFolder2);
		if (SUCCEEDED(hr))
		{
			//::SetWindowPos(m_wndListViewServer, m_wndServerAddrButtons, 0, 0, 0, 0,
			//	SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			m_wndServerAddress.ChangeCurrentFolder(m_wndListViewServer.m_lpidlAbsoluteMe);
			SetServerListenerToMe();
			::SetFocus(m_wndListViewServer);
		}
		::CoTaskMemFree(pidl);
	}
	pFolder2->Release();
	pFolder->Release();
}

void CMainWindow::SetServerListenerToMe()
{
	//IEasySFTPRoot* pRoot = NULL;
	////if (m_wndListViewLocal.m_pFolder &&
	////	SUCCEEDED(m_wndListViewLocal.m_pFolder->QueryInterface(IID_IEasySFTPRoot, (void**) &pRoot)) &&
	////	pRoot)
	////{
	////	pRoot->SetListener(this);
	////	pRoot->Release();
	////}
	////pRoot = NULL;
	//if (m_wndListViewServer.m_pFolder &&
	//	SUCCEEDED(m_wndListViewServer.m_pFolder->QueryInterface(IID_IEasySFTPRoot, (void**) &pRoot)) &&
	//	pRoot)
	//{
	//	pRoot->SetListener(this);
	//	pRoot->Release();
	//}
}

void CMainWindow::OnChangeServerFolderFailed()
{
	m_wndServerAddress.RestoreTextBox();
	//m_bInVerifyDirectory = false;
	SetStatusText(MAKEINTRESOURCEW(IDS_DIRCHANGE_FAILED));
	::MessageBeep(MB_ICONEXCLAMATION);
}

void CMainWindow::NavigateParentFolder()
{
	if (!::IsDesktopIDList(m_wndListViewLocal.m_lpidlAbsoluteMe))
	{
		PIDLIST_ABSOLUTE pidl = RemoveOneChild(m_wndListViewLocal.m_lpidlAbsoluteMe);
		UpdateCurrentFolderAbsolute(pidl);
		::CoTaskMemFree(pidl);
	}
}

void CMainWindow::NavigateServerParentFolder()
{
	//if (!(m_bSFTPMode ? m_pClient != NULL : m_pConnection != NULL))
	//	return;

	//if (m_wndServerAddress.m_strDirectory.Compare(L"/") != 0)
	//{
	//	CMyStringW str(m_wndServerAddress.m_strDirectory);
	//	LPWSTR lpw = str.GetBuffer();
	//	LPWSTR lpwLast = wcsrchr(lpw, L'/');
	//	if (lpwLast)
	//	{
	//		if (lpwLast != lpw)
	//			*lpwLast = 0;
	//		else
	//			lpwLast[1] = 0;
	//	}
	//	str.ReleaseBuffer();
	//	UpdateServerFolderAbsolute(str);
	//}

	if (!::IsDesktopIDList(m_wndListViewServer.m_lpidlAbsoluteMe))
	{
		PIDLIST_ABSOLUTE pidl = RemoveOneChild(m_wndListViewServer.m_lpidlAbsoluteMe);
		UpdateServerFolderAbsolute(pidl);
		::CoTaskMemFree(pidl);
	}
}

void CMainWindow::CheckFileTypes(CShellFolderFileView* pViewCur, CShellFolderFileView* pViewOther, PIDLIST_ABSOLUTE* ppidlItems, int nCount, int& nTextCount, int& nDirCount)
{
	IEasySFTPRootDirectory* pDir;
	//PITEMID_CHILD pidlChild;
	PIDLIST_RELATIVE pidlChild;
	if (!nCount)
		return;
	if (pViewCur->m_pRootDirectory)
		pDir = pViewCur->m_pRootDirectory;
	else if (pViewOther->m_pRootDirectory)
		pDir = pViewOther->m_pRootDirectory;
	else
		return;

	CMyStringW str;
	bool bDir;
	HRESULT hr;
	for (int i = 0; i < nCount; i++)
	{
		if (IsMatchParentIDList(pViewCur->m_lpidlAbsoluteMe, ppidlItems[i]))
		{
			bDir = false;
			if (pViewCur->m_pRootDirectory)
			{
				//pidlChild = GetChildItemIDList(ppidlItems[i]);
				pidlChild = PickupRelativeIDList(pViewCur->m_lpidlAbsoluteMe, ppidlItems[i]);
				if (pidlChild)
				{
					if (IsSingleIDList(pidlChild))
					{
						SFGAOF rgf = SFGAO_FOLDER;
						hr = pViewCur->m_pFolder->GetAttributesOf(1, (PCUITEMID_CHILD_ARRAY) &pidlChild, &rgf);
						if (SUCCEEDED(hr) && (rgf & SFGAO_FOLDER) != 0)
							bDir = true;
						else
						{
							STRRET strret;
							strret.uType = STRRET_WSTR;
							hr = pViewCur->m_pFolder->GetDisplayNameOf((PCUITEMID_CHILD) pidlChild, SHGDN_INFOLDER | SHGDN_FORPARSING, &strret);
							if (SUCCEEDED(hr))
							{
								switch (strret.uType)
								{
									case STRRET_WSTR:
										str = strret.pOleStr;
										::CoTaskMemFree(strret.pOleStr);
										break;
									case STRRET_CSTR:
										str = strret.cStr;
										break;
									case STRRET_OFFSET:
										str = (LPCSTR) (((LPCBYTE) pidlChild) + strret.uOffset);
										break;
								}
							}
						}
					}
					::CoTaskMemFree(pidlChild);
				}
			}
			else
			{
				if (!::SHGetPathFromIDListW(ppidlItems[i], str.GetBuffer(MAX_PATH)))
				{
					::SHGetPathFromIDListA(ppidlItems[i], str.GetBufferA(MAX_PATH));
					str.ReleaseBufferA();
				}
				else
					str.ReleaseBuffer();
				if (::MyIsDirectoryW(str))
					bDir = true;
			}
			if (bDir)
				nDirCount++;
			else if (!str.IsEmpty())
			{
				VARIANT_BOOL bRet = VARIANT_FALSE;
				BSTR bstr = MyStringToBSTR(str);
				if (bstr)
				{
					hr = pDir->IsTextFile(bstr, &bRet);
					if (FAILED(hr))
						bRet = VARIANT_FALSE;
					::SysFreeString(bstr);
				}
				if (bRet)
					nTextCount++;
			}
		}
	}
}

void CMainWindow::UpdateFileSelection()
{
	HWND hWndFocus = ::GetFocus();
	int nCount, nTextCount, nDirCount, i;
	CMyStringW str;
	nTextCount = nDirCount = 0;
	//if (hWndFocus == m_wndListViewServer)
	if (IsWindowOrChildrenFocused(m_wndListViewServer, hWndFocus))
	{
		//nCount = m_wndListViewServer.GetSelectedCount();
		//i = -1;
		//for (int i2 = 0; i2 < nCount; i2++)
		//{
		//	CFTPFileItem* pItem;
		//	if (!m_wndListViewServer.GetNextSelectedItem(i, &i, &pItem))
		//		break;
		//	if (pItem->IsDirectory())
		//		nDirCount++;
		//	else if (IsTextFile(pItem->strFileName))
		//		nTextCount++;
		//}

		PIDLIST_ABSOLUTE* ppList = m_wndListViewServer.GetAllSelection(&nCount);
		if (ppList)
		{
			CheckFileTypes(&m_wndListViewServer, &m_wndListViewLocal, ppList, nCount, nTextCount, nDirCount);
			for (i = 0; i < nCount; i++)
			{
				//if (!::SHGetPathFromIDListW(ppList[i], str.GetBuffer(MAX_PATH)))
				//{
				//	::SHGetPathFromIDListA(ppList[i], str.GetBufferA(MAX_PATH));
				//	str.ReleaseBufferA();
				//}
				//else
				//	str.ReleaseBuffer();
				//if (::MyIsDirectoryW(str))
				//	nDirCount++;
				//else if (IsTextFile(str))
				//	nTextCount++;
				::CoTaskMemFree(ppList[i]);
			}
			free(ppList);
		}
		//nCount = 0;
	}
	else if (IsWindowOrChildrenFocused(m_wndListViewLocal, hWndFocus))
	{
		PIDLIST_ABSOLUTE* ppList = m_wndListViewLocal.GetAllSelection(&nCount);
		if (ppList)
		{
			CheckFileTypes(&m_wndListViewLocal, &m_wndListViewServer, ppList, nCount, nTextCount, nDirCount);
			for (i = 0; i < nCount; i++)
			{
				//if (!::SHGetPathFromIDListW(ppList[i], str.GetBuffer(MAX_PATH)))
				//{
				//	::SHGetPathFromIDListA(ppList[i], str.GetBufferA(MAX_PATH));
				//	str.ReleaseBufferA();
				//}
				//else
				//	str.ReleaseBuffer();
				//if (::MyIsDirectoryW(str))
				//	nDirCount++;
				//else if (IsTextFile(str))
				//	nTextCount++;
				::CoTaskMemFree(ppList[i]);
			}
			free(ppList);
		}
	}
	else
		nCount = 0;
	if (!nCount)
		i = IDS_TYPE_NONE;
	else if (nDirCount)
	{
		if (nTextCount || nCount != nDirCount)
			i = IDS_TYPE_MIXED;
		else
			i = IDS_TYPE_DIRECTORY;
	}
	else if (!nTextCount)
		i = IDS_TYPE_BINARY;
	else if (nCount == nTextCount)
		i = IDS_TYPE_TEXT;
	else
		i = IDS_TYPE_MIXED;
	if ((UINT) i != m_uLastStatusTextModeID)
	{
		str.LoadString(m_uLastStatusTextModeID = (UINT) i);
		SetStatusText(ID_STATUS_TEXT, str);
	}
}

void CMainWindow::_SetTransferMode(LONG nTransferMode)
{
	if (m_wndListViewServer.m_pRootDirectory)
		m_wndListViewServer.m_pRootDirectory->put_TransferMode(static_cast<EasySFTPTransferMode>(nTransferMode));
}

void CMainWindow::_SetTextMode(bool bServer, LONG nTextMode)
{
	if (m_wndListViewServer.m_pRootDirectory)
	{
		EasySFTPTextModeFlags nTMode;
		if (SUCCEEDED(m_wndListViewServer.m_pRootDirectory->get_TextMode(&nTMode)))
		{
			if (bServer)
				nTMode = static_cast<EasySFTPTextMode>((nTMode & ~EasySFTPTextMode::StreamMask) | nTextMode);
			else
				nTMode = static_cast<EasySFTPTextMode>((nTMode & ~EasySFTPTextMode::BufferMask) | nTextMode);
			m_wndListViewServer.m_pRootDirectory->put_TextMode(nTMode);
		}
	}
}

void CMainWindow::DeleteSelection()
{
	//if (::GetFocus() == m_wndListViewServer)
	if (IsWindowOrChildrenFocused(m_wndListViewServer, ::GetFocus()))
	{
		//int nCount = m_wndListViewServer.GetSelectedCount();
		//if (!nCount)
		//	return;
		//CMyStringW str;
		//CMySimpleArray<CFTPFileItem*> aItems;
		//int i = -1;
		//for (int i2 = 0; i2 < nCount; i2++)
		//{
		//	CFTPFileItem* pItem;
		//	if (m_wndListViewServer.GetNextSelectedItem(i, &i, &pItem))
		//		aItems.Add(pItem);
		//}
		//nCount = aItems.GetCount();
		//if (!nCount)
		//	return;
		//else if (nCount > 1)
		//	str.Format(IDS_DELETE_MULTIPLE, nCount);
		//else
		//	str.Format(IDS_DELETE_CONFIRM, (LPCWSTR) aItems.GetItem(0)->strFileName);
		//if (::MyMessageBoxW(m_hWnd, str, NULL, MB_ICONQUESTION | MB_YESNO) != IDYES)
		//	return;
		//for (int i = 0; i < nCount; i++)
		//	DoDelete(aItems.GetItem(i));
		m_wndListViewServer.DoDelete(m_hWnd);
	}
	else
	{
		m_wndListViewLocal.DoDelete(m_hWnd);
	}
}

void CMainWindow::ShowOption()
{
	COptionDialog dlg;
	auto ret = dlg.ModalDialogW(m_hWnd);
	if (ret == IDC_REGISTER || ret == IDC_REGISTER_SYSTEM)
	{
		theApp.m_bExitWithRegister = true;
		theApp.m_bIsRegisterForSystem = ret == IDC_REGISTER_SYSTEM;
		theApp.m_bUnregisterOperation = !theApp.m_bEmulatingRegistry;
		DestroyWindow();
	}
}

////////////////////////////////////////////////////////////////////////////////

void CMainWindow::SetStatusText(LPCWSTR lpszStatusText)
{
	if (lpszStatusText && !HIWORD(lpszStatusText))
	{
		CMyStringW str(lpszStatusText);
		if (m_strStatusText.Compare(str) == 0)
			return;
		m_strStatusText = str;
	}
	else
	{
		if (!lpszStatusText)
			lpszStatusText = theApp.m_strTitle;
		if (m_strStatusText.Compare(lpszStatusText) == 0)
			return;
		m_strStatusText = lpszStatusText;
	}
	if (!m_strStatusText.IsEmpty())
	{
		register LPCWSTR lp = m_strStatusText;
		LPCWSTR lpn = wcschr(lp, L'\n');
		if (lpn)
		{
			register DWORD dw = (DWORD) (((DWORD_PTR) lpn - (DWORD_PTR) lp) / sizeof(WCHAR));
			m_strStatusText.DeleteString(dw, m_strStatusText.GetLength() - dw);
		}
	}
	if (m_wndStatusBar.IsWindowUnicode() || ::SendMessage(m_wndStatusBar, CCM_GETUNICODEFORMAT, 0, 0))
		::SendMessage(m_wndStatusBar, SB_SETTEXTW, (WPARAM) (0 | 0), (LPARAM)(LPCWSTR) m_strStatusText);
	else
		::SendMessage(m_wndStatusBar, SB_SETTEXTA, (WPARAM) (0 | 0), (LPARAM)(LPCSTR) m_strStatusText);
}

static int __stdcall CalcTextWidth(HWND hWndStatus, HFONT hFontWindow, const CMyStringW& string)
{
	HFONT hFont;
	HDC hDC;
	HGDIOBJ hgdi;
	SIZE sz;

	hFont = (HFONT) ::SendMessage(hWndStatus, WM_GETFONT, 0, 0);
	if (!hFont)
		hFont = hFontWindow;
	hDC = ::GetDC(hWndStatus);
	hgdi = ::SelectObject(hDC, hFont);
	::GetTextExtentPoint32W(hDC, string, (int) string.GetLength(), &sz);
	::SelectObject(hDC, hgdi);
	::ReleaseDC(hWndStatus, hDC);
	return sz.cx;
}

void CMainWindow::SetStatusText(UINT uStatusID, LPCWSTR lpszStatusText)
{
	CMyStringW str;
	if (lpszStatusText)
		str = lpszStatusText;
	for (int i = 0; i < sizeof(s_arrBarParts) / sizeof(s_arrBarParts[0]); i++)
	{
		if (s_arrBarParts[i].uID == uStatusID)
		{
			int rgBorders[3];
			::SendMessage(m_wndStatusBar, SB_GETBORDERS, 0, (LPARAM) rgBorders);
			s_arrBarParts[i].nWidth = CalcTextWidth(m_wndStatusBar, m_hFontWindow, str) + 6 + rgBorders[2];
			if (uStatusID == ID_STATUS_HOST)
				s_arrBarParts[i].nWidth += 20;
			UpdateStatusParts();
			if (m_wndStatusBar.IsWindowUnicode() || ::SendMessage(m_wndStatusBar, CCM_GETUNICODEFORMAT, 0, 0))
				::SendMessage(m_wndStatusBar, SB_SETTEXTW, (WPARAM) (i | 0), (LPARAM)(LPCWSTR) str);
			else
				::SendMessage(m_wndStatusBar, SB_SETTEXTA, (WPARAM) (i | 0), (LPARAM)(LPCSTR) str);
			break;
		}
	}
}

//void CMainWindow::SetSFTPStatusText(int nStatus, const CMyStringW& strMessage)
//{
//	if (nStatus == SSH_FX_OK)
//		SetStatusText(MAKEINTRESOURCEW(IDS_COMMAND_OK));
//	else
//	{
//		CMyStringW str;
//		str.Format(IDS_COMMAND_FAILED, nStatus);
//		if (!strMessage.IsEmpty())
//		{
//			str += L" (";
//			str += strMessage;
//			str += L')';
//		}
//		SetStatusText(str);
//	}
//}

void CMainWindow::SetStatusSecureIcon(bool bSecure)
{
	for (int i = 0; i < sizeof(s_arrBarParts) / sizeof(s_arrBarParts[0]); i++)
	{
		//if (s_arrBarParts[i].uID == ID_STATUS_SECURE)
		if (s_arrBarParts[i].uID == ID_STATUS_HOST)
		{
			::SendMessage(m_wndStatusBar, SB_SETICON, (WPARAM) (i), (LPARAM) (bSecure ? m_hIconSecure : NULL));
			::InvalidateRect(m_wndStatusBar, NULL, FALSE);
			break;
		}
	}
}

void CMainWindow::UpdateStatusParts()
{
	RECT rc;
	::GetClientRect(m_hWnd, &rc);
	int nTotalWidth, nGrip;
	int i;
	nTotalWidth = 0;
	if (!::IsZoomed(m_hWnd))
		nGrip = ::GetSystemMetrics(SM_CXHSCROLL);
	else
		nGrip = 0;
	for (i = 0; i < sizeof(s_arrBarParts) / sizeof(s_arrBarParts[0]); i++)
	{
		if (s_arrBarParts[i].nWidth != -1)
			nTotalWidth += s_arrBarParts[i].nWidth;
	}
	nTotalWidth += nGrip;
	int* pnWidths = (int*) malloc(sizeof(int) * (sizeof(s_arrBarParts) / sizeof(s_arrBarParts[0])));
	for (i = 0; i < sizeof(s_arrBarParts) / sizeof(s_arrBarParts[0]); i++)
	{
		if (s_arrBarParts[i].nWidth == -1)
			pnWidths[i] = rc.right - nTotalWidth;
		else
			pnWidths[i] = s_arrBarParts[i].nWidth;
		if (i == sizeof(s_arrBarParts) / sizeof(s_arrBarParts[0]) - 1)
			pnWidths[i] += nGrip;
		if (i > 0)
			pnWidths[i] += pnWidths[i - 1];
	}
	::SendMessage(m_wndStatusBar, SB_SETPARTS,
		(WPARAM) (sizeof(s_arrBarParts) / sizeof(s_arrBarParts[0])), (LPARAM) pnWidths);
	free(pnWidths);
}

static bool __stdcall IsHostItemSelected(HWND hWnd, CShellFolderFileView* pView)
{
	if (!IsEqualIDList(pView->m_lpidlAbsoluteMe, theApp.m_pidlEasySFTP))
		return false;
	if (pView->GetSelectionCount() != 1)
		return false;
	bool bRet = false;
	PIDLIST_ABSOLUTE pidlItem = pView->GetSelectedItem(0);
	if (pidlItem)
	{
		PIDLIST_RELATIVE pidlRel = PickupRelativeIDList(pView->m_lpidlAbsoluteMe, pidlItem);
		if (pidlRel)
		{
			IContextMenu* pMenu;
			if (IsSingleIDList(pidlRel) && SUCCEEDED(pView->m_pFolder->GetUIObjectOf(hWnd, 1, (PCUITEMID_CHILD_ARRAY) &pidlRel, IID_IContextMenu, NULL, (void**) &pMenu)))
			{
				HMENU hMenu = ::CreatePopupMenu();
				HRESULT hr = pMenu->QueryContextMenu(hMenu, 0, 0, 0xFFFF, CMF_DEFAULTONLY);
				if (SUCCEEDED(hr))
				{
					CMyStringW str;
					hr = pMenu->GetCommandString((UINT_PTR) HRESULT_CODE(hr) - 1, GCS_VERBW, NULL, (LPSTR) str.GetBuffer(MAX_PATH), MAX_PATH);
					if (SUCCEEDED(hr))
						bRet = (str.Compare(L"connect", true) == 0);
				}
				::DestroyMenu(hMenu);
				pMenu->Release();
			}
			::CoTaskMemFree(pidlRel);
		}
		::CoTaskMemFree(pidlItem);
	}
	return bRet;
}

bool CMainWindow::CanConnect(bool bServer)
{
	IEasySFTPRootDirectory* pDir = bServer ? m_wndListViewServer.m_pRootDirectory : m_wndListViewLocal.m_pRootDirectory;
	if (pDir)
	{
		VARIANT_BOOL bRet = VARIANT_FALSE;
		if (FAILED(pDir->get_Connected(&bRet)))
			bRet = VARIANT_FALSE;
		return !bRet;
	}
	if (IsHostItemSelected(m_hWnd, bServer ? &m_wndListViewServer : &m_wndListViewLocal))
		return true;
	//if (bServer)
	//{
	//	if (IsEqualIDList(m_wndListViewServer.m_lpidlAbsoluteMe, theApp.m_pidlEasySFTP))
	//	{
	//		 m_wndListViewServer.GetAllSelection(&nCount);
	//	}
	//}
	//if (m_pObjectOnClipboard)
	//{
	//	if (::OleIsCurrentClipboard(m_pObjectOnClipboard) == S_OK)
	//	{
	//		if (::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_DATA_ON_CLIPBOARD), NULL, MB_ICONQUESTION | MB_YESNO) == IDNO)
	//			return false;
	//	}
	//}
	return false;
}

bool CMainWindow::CanDisconnect(bool bServer)
{
	//if (!theApp.CheckExternalApplications())
	//	return false;
	IEasySFTPRootDirectory* pDir = bServer ? m_wndListViewServer.m_pRootDirectory : m_wndListViewLocal.m_pRootDirectory;
	if (pDir)
	{
		VARIANT_BOOL bRet = VARIANT_FALSE;
		if (FAILED(pDir->get_Transferring(&bRet)))
			bRet = VARIANT_FALSE;
		if (bRet)
		{
			if (::MyMessageBoxW(NULL, MAKEINTRESOURCEW(IDS_EXTERNAL_APP_IS_DOWNLOADING), NULL, MB_ICONEXCLAMATION | MB_YESNO) == IDNO)
				return false;
		}
	}
	//if (m_pObjectOnClipboard)
	//{
	//	if (::OleIsCurrentClipboard(m_pObjectOnClipboard) == S_OK)
	//	{
	//		if (::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_DATA_ON_CLIPBOARD), NULL, MB_ICONQUESTION | MB_YESNO) == IDNO)
	//			return false;
	//	}
	//}
	return true;
}

void CMainWindow::UpdateViewStatus(HWND hWndFocus)
{
	if (IsWindowOrChildrenFocused(m_wndListViewLocal.m_hWnd, hWndFocus))
	{
		if (m_wndListViewLocal.m_pRootDirectory)
		{
			EasySFTPConnectionMode mode = EasySFTPConnectionMode::FTP;
			BSTR bstr = NULL;
			m_wndListViewLocal.m_pRootDirectory->get_ConnectionMode(&mode);
			m_wndListViewLocal.m_pRootDirectory->get_HostName(&bstr);
			if (bstr)
			{
				SetStatusText(ID_STATUS_HOST, bstr);
				SetStatusSecureIcon(mode != EasySFTPConnectionMode::FTP);
				::SysFreeString(bstr);
				return;
			}
		}
	}
	else if (IsWindowOrChildrenFocused(m_wndListViewServer.m_hWnd, hWndFocus))
	{
		if (m_wndListViewServer.m_pRootDirectory)
		{
			EasySFTPConnectionMode mode = EasySFTPConnectionMode::FTP;
			BSTR bstr = NULL;
			m_wndListViewServer.m_pRootDirectory->get_ConnectionMode(&mode);
			m_wndListViewServer.m_pRootDirectory->get_HostName(&bstr);
			if (bstr)
			{
				SetStatusText(ID_STATUS_HOST, bstr);
				SetStatusSecureIcon(mode != EasySFTPConnectionMode::FTP);
				::SysFreeString(bstr);
				return;
			}
		}
	}
	SetStatusText(ID_STATUS_HOST, NULL);
	SetStatusSecureIcon(false);
}

////////////////////////////////////////////////////////////////////////////////

#define ID_ADDRESS_CONTROL              101
#define ID_SERVER_ADDRESS_CONTROL       102

LRESULT CMainWindow::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	//REFLECT_MEASUREITEM(&m_wndAddress);
	//REFLECT_DRAWITEM(&m_wndAddress);
	//REFLECT_DELETEITEM(&m_wndAddress);
	//REFLECT_CONTROL_WND(m_wndAddress);
	//REFLECT_MEASUREITEM(&m_wndServerAddress);
	//REFLECT_DRAWITEM(&m_wndServerAddress);
	//REFLECT_DELETEITEM(&m_wndServerAddress);
	//REFLECT_CONTROL_WND(m_wndServerAddress);

	HANDLE_PROC_MESSAGE(WM_CREATE, OnCreate);
	HANDLE_PROC_MESSAGE(WM_CLOSE, OnClose);
	HANDLE_PROC_MESSAGE(WM_DESTROY, OnDestroy);
	HANDLE_PROC_MESSAGE(WM_SIZE, OnSize);
	HANDLE_NOTIFY(ID_ADDRESS_CONTROL, CBEN_ENDEDITA, OnLocalAddressEndEditA);
	HANDLE_NOTIFY(ID_ADDRESS_CONTROL, CBEN_ENDEDITW, OnLocalAddressEndEditW);
	HANDLE_CONTROL(ID_ADDRESS_CONTROL, CBN_SELCHANGE, OnLocalAddressSelChange);
	HANDLE_CONTROL(ID_ADDRESS_CONTROL, CBN_CLOSEUP, OnLocalAddressCloseUp);
	REFLECT_NOTIFY_WND(m_wndAddress);
	HANDLE_NOTIFY(ID_SERVER_ADDRESS_CONTROL, CBEN_ENDEDITA, OnServerAddressEndEditA);
	HANDLE_NOTIFY(ID_SERVER_ADDRESS_CONTROL, CBEN_ENDEDITW, OnServerAddressEndEditW);
	HANDLE_CONTROL(ID_SERVER_ADDRESS_CONTROL, CBN_SELCHANGE, OnServerAddressSelChange);
	HANDLE_CONTROL(ID_SERVER_ADDRESS_CONTROL, CBN_CLOSEUP, OnServerAddressCloseUp);
	REFLECT_NOTIFY_WND(m_wndServerAddress);
	HANDLE_PROC_MESSAGE(WM_COMMAND, OnCommand);
	HANDLE_PROC_MESSAGE(WM_INITMENUPOPUP, OnInitMenuPopup);
	HANDLE_PROC_MESSAGE(WM_MENUSELECT, OnMenuSelect);
	HANDLE_PROC_MESSAGE(WM_CONTEXTMENU, OnContextMenu);
	//REFLECT_NOTIFY(101);
	HANDLE_PROC_MESSAGE(WM_ACTIVATE, OnActivate);
	HANDLE_PROC_MESSAGE(WM_SETTINGCHANGE, OnSettingChange);
	HANDLE_PROC_MESSAGE(WM_DPICHANGED, OnDpiChanged);
	HANDLE_NOTIFY_CODE(TTN_GETDISPINFOA, OnToolTipDispInfoA);
	HANDLE_NOTIFY_CODE(TTN_GETDISPINFOW, OnToolTipDispInfoW);
	HANDLE_NOTIFY_WND(m_wndToolBar, TBN_DROPDOWN, OnToolBarDropDown);
	//HANDLE_NOTIFY_WND(m_wndListViewServer.m_hWnd, NM_DBLCLK, OnServerListViewDblClick);
	//HANDLE_NOTIFY_WND(m_wndListViewServer.m_hWnd, NM_RETURN, OnServerListViewReturn);
	//HANDLE_NOTIFY_WND(m_wndListViewServer.m_hWnd, LVN_BEGINLABELEDITA, OnServerListViewBeginLabelEdit);
	//HANDLE_NOTIFY_WND(m_wndListViewServer.m_hWnd, LVN_BEGINLABELEDITW, OnServerListViewBeginLabelEdit);
	//HANDLE_NOTIFY_WND(m_wndListViewServer.m_hWnd, LVN_ENDLABELEDITA, OnServerListViewEndLabelEdit);
	//HANDLE_NOTIFY_WND(m_wndListViewServer.m_hWnd, LVN_ENDLABELEDITW, OnServerListViewEndLabelEdit);
	//HANDLE_PROC_MESSAGE(MY_WM_SOCKETMESSAGE, OnSocketMessage);
	//HANDLE_PROC_MESSAGE(WM_TIMER, OnTimer);
	//HANDLE_PROC_MESSAGE(MY_WM_SENDQUEUE, OnSendQueue);
	HANDLE_PROC_MESSAGE(MY_WM_BROWSE_VIEW, OnBrowseView);
	HANDLE_PROC_MESSAGE(MY_WM_UPDATESETMENU, OnUpdateSetMenu);
	HANDLE_PROC_MESSAGE(MY_WM_CHANGENOTIFY, OnChangeNotify);

	HANDLE_PROC_MESSAGE(WM_DDE_INITIATE, OnDDEInitialize);
	HANDLE_PROC_MESSAGE(WM_DDE_TERMINATE, OnDDETerminate);
	HANDLE_PROC_MESSAGE(WM_DDE_EXECUTE, OnDDEExecute);

	HANDLE_NOTIFY_WND(m_wndSplitter, SPN_TRACK, OnSplitterTrack);
	HANDLE_NOTIFY_WND(m_wndSplitter, SPN_TRACKING, OnSplitterTracking);

#ifdef _DEBUG
	//if (message == WM_SYSCOMMAND && LOWORD(wParam) == SC_MAXIMIZE)
	//{
	//	CMyStringW str;
	//	str.Format(L"CanReceive: %s\n", m_pConnection->CanReceive() ? L"true" : L"false");
	//	::OutputDebugString(str);
	//}
	//if (message == WM_SYSCOMMAND && LOWORD(wParam) == SC_MAXIMIZE)
	//{
	//	HRESULT hr;
	//	IEnumIDList* pEnum;
	//	hr = m_wndListViewServer.m_pFolder->EnumObjects(m_hWnd, SHCONTF_NONFOLDERS, &pEnum);
	//	if (SUCCEEDED(hr))
	//	{
	//		PITEMID_CHILD idChild;
	//		hr = pEnum->Next(1, &idChild, NULL);
	//		pEnum->Release();
	//		if (hr == S_OK)
	//		{
	//			auto idAbsolute = AppendItemIDList(m_wndListViewServer.m_lpidlAbsoluteMe, idChild);
	//			::CoTaskMemFree(idChild);
	//			IShellItem* pItem;
	//			hr = ::SHCreateItemFromIDList(idAbsolute, IID_IShellItem, (void**)&pItem);
	//			::CoTaskMemFree(idAbsolute);
	//			if (SUCCEEDED(hr))
	//			{
	//				IFileOperation* pOperation;
	//				hr = ::CoCreateInstance(__uuidof(FileOperation), NULL, CLSCTX_INPROC_SERVER, IID_IFileOperation, (void**)&pOperation);
	//				if (SUCCEEDED(hr))
	//				{
	//					pOperation->SetOwnerWindow(m_hWnd);
	//					hr = pOperation->DeleteItem(pItem, NULL);
	//					hr = pOperation->PerformOperations();
	//					pOperation->Release();
	//				}
	//				pItem->Release();
	//			}
	//		}
	//	}
	//}
#endif
	//if (message == WM_INITMENU)
	//{
	//	if (m_hWndViewForMenu)
	//		return ::SendMessage(m_hWndViewForMenu, WM_INITMENU, wParam, lParam);
	//}

	return CMyWindow::WindowProc(message, wParam, lParam);
}

static void __stdcall _SetMenuItemAllInfoIfNeed(HMENU hMenu, CMyStringW* pstrBuffer = NULL)
{
	int i, nCount;
	CMyStringW* p;
	MENUITEMINFOW mii;

	if (pstrBuffer)
		p = pstrBuffer;
	else
		p = new CMyStringW();
	nCount = ::GetMenuItemCount(hMenu);
	mii.cbSize = MENUITEMINFO_SIZE_V1W;
	mii.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_ID;
	mii.dwTypeData = p->GetBuffer(MAX_PATH);
	for (i = 0; i < nCount; i++)
	{
		mii.cch = MAX_PATH;
		mii.hSubMenu = NULL;
		::MyGetMenuItemInfoW(hMenu, (UINT) i, TRUE, &mii);
		if (mii.hSubMenu)
			_SetMenuItemAllInfoIfNeed(mii.hSubMenu, p);
		else
		{
			switch (mii.wID)
			{
				case ID_TRANSFER_MODE_AUTO:
				case ID_TRANSFER_MODE_BINARY:
				case ID_TRANSFER_MODE_TEXT:
				case ID_TRANSFER_LOCAL_CRLF:
				case ID_TRANSFER_LOCAL_CR:
				case ID_TRANSFER_LOCAL_LF:
				case ID_TRANSFER_SERVER_CRLF:
				case ID_TRANSFER_SERVER_CR:
				case ID_TRANSFER_SERVER_LF:
#if (ID_RETURN_MODE_CRLF != ID_TRANSFER_LOCAL_CRLF && ID_RETURN_MODE_CRLF != ID_TRANSFER_SERVER_CRLF)
				case ID_RETURN_MODE_CRLF:
				case ID_RETURN_MODE_CR:
				case ID_RETURN_MODE_LF:
#endif
					mii.fType = MFT_RADIOCHECK;
					break;
				default:
					continue;
			}
			::MySetMenuItemInfoW(hMenu, (UINT) i, TRUE, &mii);
		}
	}
	if (!pstrBuffer)
		delete p;
}

static bool __stdcall DoAddButtons(CMyWindow& wndToolBar, const CToolBarButtonDef* pButtons, int nCount)
{
	TBBUTTON* ptb = (TBBUTTON*) malloc(sizeof(TBBUTTON) * nCount);
	if (!ptb)
		return false;
	for (int i = 0; i < nCount; i++)
	{
		if (!pButtons[i].uID)
		{
			ptb[i].idCommand = 0;
			ptb[i].iBitmap = 0;
			ptb[i].fsStyle = TBSTYLE_SEP;
			ptb[i].fsState = 0;
		}
		else
		{
			ptb[i].idCommand = (int) pButtons[i].uID;
			ptb[i].iBitmap = pButtons[i].iImage & 0xFFFF;
			if (pButtons[i].iImage & 0x10000)
				ptb[i].fsStyle = TBSTYLE_DROPDOWN | BTNS_WHOLEDROPDOWN;
			else
				ptb[i].fsStyle = TBSTYLE_BUTTON;
			ptb[i].fsState = TBSTATE_ENABLED;
		}
		ptb[i].iString = 0;
		ptb[i].dwData = 0;
	}
	if (wndToolBar.IsWindowUnicode() || ::SendMessage(wndToolBar, CCM_GETUNICODEFORMAT, 0, 0))
		::SendMessage(wndToolBar, TB_ADDBUTTONSW,
			(WPARAM) nCount, (LPARAM)(TBBUTTON FAR*) ptb);
	else
		::SendMessage(wndToolBar, TB_ADDBUTTONSA,
			(WPARAM) nCount, (LPARAM)(TBBUTTON FAR*) ptb);
	free(ptb);
	::SendMessage(wndToolBar, TB_AUTOSIZE, 0, 0);
	return true;
}

LRESULT CMainWindow::OnCreate(WPARAM wParam, LPARAM lParam)
{
	if (Default(wParam, lParam) == 1)
		return -1;

	m_uDpi = _MyGetDpiForWindow(m_hWnd);

	m_hMenu = MyLoadMenuW(theApp.m_hInstance, MAKEINTRESOURCEW(IDC_EASYFTP));
	if (!m_hMenu)
		return -1;
	_SetMenuItemAllInfoIfNeed(m_hMenu);
	{
		MENUITEMINFOW mii;
#ifdef _WIN64
		mii.cbSize = sizeof(mii);
#else
		mii.cbSize = MENUITEMINFO_SIZE_V1W;
#endif
		mii.fMask = MIIM_ID;
		int nCount = ::GetMenuItemCount(m_hMenu);
		for (int i = 0; i < nCount; i++)
		{
			mii.wID = s_arrMenuParentIDs[i];
			::MySetMenuItemInfoW(m_hMenu, (UINT) i, TRUE, &mii);
		}
	}

	if (!m_wndToolBar.CreateExW(0, TOOLBARCLASSNAMEW, NULL,
		WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS,
		0, 0, 0, 0, m_hWnd, NULL))
		return -1;
	if (!m_wndAddress.Create(0, 0, 100, 400, m_hWnd, ID_ADDRESS_CONTROL))
		return -1;
	if (!m_wndAddrButtons.CreateExW(0, TOOLBARCLASSNAMEW, NULL,
		WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS | CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER,
		0, 0, 0, 0, m_hWnd, NULL))
		return -1;
	if (!m_xBrowserForLocal.CreateExW(0, CMainApplication::s_szViewParentWndClass, NULL,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		0, 0, 0, 0, m_hWnd, NULL, theApp.m_hInstance))
		return -1;
	if (!m_wndListViewLocal.Create(&m_xBrowserForLocal, m_xBrowserForLocal))
	{
		//pFolder->Release();
		return -1;
	}
	if (!m_wndSplitter.CreateExW(0, SPLITTER_CLASSW, NULL,
		WS_CHILD | WS_VISIBLE | SPS_VERTICAL, 0, 0, 0, 0,
		m_hWnd, NULL, theApp.m_hInstance))
		return -1;
	if (!m_wndServerAddress.Create(0, 0, 100, 400, m_hWnd, ID_SERVER_ADDRESS_CONTROL))
		return -1;
	if (!m_wndServerAddrButtons.CreateExW(0, TOOLBARCLASSNAMEW, NULL,
		WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS | CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER,
		0, 0, 0, 0, m_hWnd, NULL))
		return -1;
	//if (!m_wndListViewServer.Create(m_hWnd))
	//	return -1;
	if (!m_xBrowserForServer.CreateExW(0, CMainApplication::s_szViewParentWndClass, NULL,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		0, 0, 0, 0, m_hWnd, NULL, theApp.m_hInstance))
		return -1;
	{
		IShellFolder* pFolder;
		HRESULT hr = theApp.m_pEasySFTPRoot->QueryInterface(IID_IShellFolder, (void**) &pFolder);
		if (FAILED(hr))
			return -1;
		//Sleep(1000);
		if (!m_wndListViewServer.Create(theApp.m_pidlEasySFTP, pFolder, &m_xBrowserForServer, m_xBrowserForServer))
		//if (!m_wndListViewServer.Create(&m_xBrowserForServer, m_hWnd))
		{
			pFolder->Release();
			return -1;
		}
		////if (FAILED(m_wndListViewServer.ReplaceViewAbsolute(theApp.m_pidlEasySFTP, pFolder)))
		//if (FAILED(m_wndListViewServer.ReplaceViewAbsolute(theApp.m_pidlEasySFTP)))
		//{
		//	pFolder->Release();
		//	return -1;
		//}
		pFolder->Release();
		SetServerListenerToMe();

		if (!theApp.m_strFirstLocalPath.IsEmpty() || theApp.m_strFirstServerPath.IsEmpty())
		{
			IShellFolder* pDesktop;
			if (SUCCEEDED(::SHGetDesktopFolder(&pDesktop)))
			{
				DWORD dwAttrs = SFGAO_FOLDER | SFGAO_FILESYSANCESTOR;
				HRESULT hr;
				PIDLIST_ABSOLUTE pidl;
				if (!theApp.m_strFirstLocalPath.IsEmpty())
				{
					hr = pDesktop->ParseDisplayName(m_hWnd, NULL,
						(LPWSTR)(LPCWSTR) theApp.m_strFirstLocalPath, NULL, (PIDLIST_RELATIVE*) &pidl, &dwAttrs);
					if (SUCCEEDED(hr))
					{
						UpdateCurrentFolderAbsolute(pidl);
						::CoTaskMemFree(pidl);
					}
				}
				if (!theApp.m_strFirstServerPath.IsEmpty())
				{
					hr = pDesktop->ParseDisplayName(m_hWnd, NULL,
						(LPWSTR)(LPCWSTR) theApp.m_strFirstServerPath, NULL, (PIDLIST_RELATIVE*) &pidl, &dwAttrs);
					if (SUCCEEDED(hr))
					{
						UpdateServerFolderAbsolute(pidl);
						::CoTaskMemFree(pidl);
					}
				}
				pDesktop->Release();
			}
		}
	}
	if (!m_wndStatusBar.CreateExW(0, STATUSCLASSNAMEW, NULL,
		WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP | CCS_NORESIZE,
		0, 0, 0, 0, m_hWnd, NULL))
		return -1;
	//if (!m_dlgTransfer.CreateW(m_hWnd))
	//	return -1;
	m_wndListViewLocal.m_pView->UIActivate(SVUIA_ACTIVATE_NOFOCUS);
	m_hWndFocusSaved = m_wndAddress;
	//::SetWindowPos(m_xBrowserForLocal, m_wndAddrButtons, 0, 0, 0, 0,
	//	SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	//::SetWindowPos(m_xBrowserForServer, m_wndServerAddrButtons, 0, 0, 0, 0,
	//	SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	m_nSplitterWidth = (int) (::SendMessage(m_wndSplitter, SPM_GETWIDTH, 0, 0));

	// enable Unicode format
	::SendMessage(m_wndToolBar, CCM_SETUNICODEFORMAT, (WPARAM) TRUE, 0);
	::SendMessage(m_wndAddrButtons, CCM_SETUNICODEFORMAT, (WPARAM) TRUE, 0);
	::SendMessage(m_wndServerAddrButtons, CCM_SETUNICODEFORMAT, (WPARAM) TRUE, 0);
	::SendMessage(m_wndStatusBar, CCM_SETUNICODEFORMAT, (WPARAM) TRUE, 0);

	UpdateFonts();

	m_wndAddress.ChangeCurrentFolder(m_wndListViewLocal.m_lpidlAbsoluteMe);
	m_wndServerAddress.ChangeCurrentFolder(m_wndListViewServer.m_lpidlAbsoluteMe);

	m_hAccel = ::LoadAccelerators(theApp.m_hInstance, MAKEINTRESOURCE(IDC_EASYFTP));
	m_hIconSecure = (HICON) ::LoadImage(theApp.m_hInstance, MAKEINTRESOURCE(ID_STATUS_SECURE),
		IMAGE_ICON, 16, 16, 0);

	//::SetMenu(m_hWnd, m_hMenu);
	m_hMenuReturnMode = ::GetSubMenu(theApp.m_hMenuPopup, POPUP_POS_RETURNMODE);
	_SetMenuItemAllInfoIfNeed(m_hMenuReturnMode);

	{
		::SendMessage(m_wndAddrButtons, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);
		::SendMessage(m_wndServerAddrButtons, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);
		::SendMessage(m_wndToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);
		UpdateToolBarIcons();

		if (!DoAddButtons(m_wndAddrButtons, s_arrAddrButtons, sizeof(s_arrAddrButtons) / sizeof(s_arrAddrButtons[0])))
			return -1;
		if (!DoAddButtons(m_wndServerAddrButtons, s_arrServerAddrButtons, sizeof(s_arrServerAddrButtons) / sizeof(s_arrServerAddrButtons[0])))
			return -1;
		if (!DoAddButtons(m_wndToolBar, s_arrToolBarButtons, sizeof(s_arrToolBarButtons) / sizeof(s_arrToolBarButtons[0])))
			return -1;

		m_nAddrButtonsWidth = -1;
		m_nToolBarHeight = -1;
		//RECT rcBar;
		//::GetClientRect(m_wndToolBar, &rcBar);
		//m_nToolBarHeight = rcBar.bottom;
		//m_nToolBarHeight = HIWORD(::SendMessage(m_wndToolBar, TB_GETBUTTONSIZE, 0, 0))
		//	+ ::GetSystemMetrics(SM_CYBORDER) * 2;
	}

	UpdateStatusParts();
	SetStatusText(NULL);

	theApp.DoAutoComplete((HWND) ::SendMessage(m_wndAddress, CBEM_GETEDITCONTROL, 0, 0));

	{
		ITEMIDLIST idl;
		idl.mkid.cb = 0;
		SHChangeNotifyEntry entry;
		entry.pidl = (PCIDLIST_ABSOLUTE) &idl;
		entry.fRecursive = TRUE;

		ULONG (STDMETHODCALLTYPE* pfnSHChangeNotifyRegister)(HWND hwnd, int fSources, LONG fEvents, UINT wMsg, int cEntries, __in const SHChangeNotifyEntry* pshcne);
		pfnSHChangeNotifyRegister = (ULONG (STDMETHODCALLTYPE*)(HWND hwnd, int fSources, LONG fEvents, UINT wMsg, int cEntries, __in const SHChangeNotifyEntry* pshcne))
			::GetProcAddress(::GetModuleHandle(_T("shell32.dll")), "SHChangeNotifyRegister");
		if (pfnSHChangeNotifyRegister)
			m_uIDChangeNotify = pfnSHChangeNotifyRegister(m_hWnd, SHCNRF_InterruptLevel | SHCNRF_ShellLevel,
				SHCNE_MKDIR | SHCNE_RENAMEFOLDER | SHCNE_RMDIR | SHCNE_DRIVEADD | SHCNE_DRIVEADDGUI | SHCNE_DRIVEREMOVED,
				MY_WM_CHANGENOTIFY, 1, &entry);
		else
			m_uIDChangeNotify = 0;
	}

	m_bWindowCreated = true;
	return 0;
}

LRESULT CMainWindow::OnClose(WPARAM wParam, LPARAM lParam)
{
	if (!CanDisconnect(false))
		return 0;
	if (!CanDisconnect(true))
		return 0;
	//if (theApp.m_uRefThread > 1)
	//{
	//	if (::MyMessageBoxW(NULL, MAKEINTRESOURCEW(IDS_EXTERNAL_APP_IS_DOWNLOADING), NULL, MB_ICONEXCLAMATION | MB_YESNO) == IDNO)
	//		return 0;
	//}
	::GetWindowPlacement(m_hWnd, &theApp.m_wpFrame);
	//m_wndListViewLocal.m_pView->DestroyViewWindow();
	//m_wndListViewLocal.m_pView->Release();
	DestroyWindow();
	return 0;
}

LRESULT CMainWindow::OnDestroy(WPARAM wParam, LPARAM lParam)
{
	//DoCloseConnection(true);
	//m_dlgTransfer.DestroyWindow();
	//PostQuitMessage(0);
	LRESULT lr = Default(wParam, lParam);
	if (m_uIDChangeNotify)
	{
		BOOL (STDMETHODCALLTYPE* pfnSHChangeNotifyDeregister)(unsigned long ulID)
			= (BOOL (STDMETHODCALLTYPE*)(unsigned long ulID))
			::GetProcAddress(::GetModuleHandle(_T("shell32.dll")), "SHChangeNotifyDeregister");
		pfnSHChangeNotifyDeregister(m_uIDChangeNotify);
	}
	m_wndListViewLocal.ReleaseAll();
	m_wndListViewServer.ReleaseAll();
	return lr;
}

LRESULT CMainWindow::OnSize(WPARAM wParam, LPARAM lParam)
{
	Default(wParam, lParam);

	OnResize();

	return 0;
}

LRESULT CMainWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(wParam) < FCIDM_BROWSERFIRST || LOWORD(wParam) > FCIDM_BROWSERLAST)
	{
		if (IsWindowOrChildrenFocused(m_wndListViewServer, ::GetFocus()))
			return SendMessage(m_wndListViewServer, WM_COMMAND, wParam, lParam);
		else
			return SendMessage(m_wndListViewLocal, WM_COMMAND, wParam, lParam);
	}
	switch (LOWORD(wParam))
	{
		case ID_FILE_CONNECT:
			DoHostConnect(IsWindowOrChildrenFocused(m_wndListViewServer, ::GetFocus()));
			break;
		case ID_FILE_QUICK_CONNECT:
			DoConnect();
			break;
		case ID_FILE_DISCONNECT:
			DoCloseConnection(IsWindowOrChildrenFocused(m_wndListViewServer, ::GetFocus()));
			break;
		case ID_FILE_OPEN:
			//if (::GetFocus() == m_wndListViewServer)
			if (IsWindowOrChildrenFocused(m_wndListViewServer, ::GetFocus()))
			{
				//int nCount = m_wndListViewServer.GetSelectedCount();
				//int i = -1;
				//for (int i2 = 0; i2 < nCount; i2++)
				//{
				//	CFTPFileItem* pItem;
				//	if (m_wndListViewServer.GetNextSelectedItem(i, &i, &pItem))
				//		DoOpen(pItem);
				//}
				m_wndListViewServer.DoOpen(m_hWnd);
			}
			else
			{
				m_wndListViewLocal.DoOpen(m_hWnd);
			}
			break;
		case ID_FILE_SAVE_AS:
			//if (::GetFocus() == m_wndListViewServer)
			//{
			//	int nCount = m_wndListViewServer.GetSelectedCount();
			//	if (!nCount)
			//		break;
			//	CMySimpleArray<CFTPFileItem*> arr;
			//	while (nCount--)
			//	{
			//		CFTPFileItem* pItem;
			//		if (m_wndListViewServer.GetNextSelectedItem(-1, NULL, &pItem))
			//			arr.Add(pItem);
			//	}
			//	DoSaveAs(arr);
			//}
			break;
		case ID_FILE_NEW_FOLDER:
			if (IsWindowOrChildrenFocused(m_wndListViewServer, ::GetFocus()))
			{
				//if (!m_nInCreatingLabelEditMode)
				//{
				//	if ((m_bSFTPMode && m_pChannel) || (!m_bSFTPMode && m_pConnection))
				//	{
				//		m_wndListViewServer.StartCreateNewFolder();
				//		m_nInCreatingLabelEditMode = 1;
				//	}
				//}
				m_wndListViewServer.DoCreateNewFolder(m_hWnd);
			}
			else
			{
				m_wndListViewLocal.DoCreateNewFolder(m_hWnd);
			}
			break;
		case ID_FILE_CREATE_SHORTCUT:
			if (IsWindowOrChildrenFocused(m_wndListViewServer, ::GetFocus()))
			{
				//if (!m_nInCreatingLabelEditMode)
				//{
				//	if (m_bSFTPMode && m_pChannel)
				//	{
				//		if (m_wndListViewServer.GetSelectedCount() > 0)
				//		{
				//			m_wndListViewServer.StartCreateShortcut();
				//			m_nInCreatingLabelEditMode = 2;
				//		}
				//		else
				//		{
				//			CLinkDialog dlg;
				//			dlg.m_strCurDir = m_wndServerAddress.m_strDirectory;
				//			dlg.m_bAllowHardLink = (m_pChannel->GetServerVersion() >= 6);
				//			if (dlg.ModalDialogW(m_hWnd) == IDOK)
				//			{
				//				if (m_pChannel)
				//				{
				//					CMyStringW strNewName(m_wndServerAddress.m_strDirectory);
				//					register DWORD dw = strNewName.GetLength();
				//					if (dw == 0 || ((LPCWSTR) strNewName)[dw - 1] != L'/')
				//						strNewName += L'/';
				//					strNewName += dlg.m_strFileName;
				//					DoCreateShortcut(dlg.m_strLinkTo, strNewName, !dlg.m_bHardLink);
				//				}
				//			}
				//		}
				//	}
				//}
				m_wndListViewServer.DoCreateShortcut(m_hWnd);
			}
			else
			{
				m_wndListViewLocal.DoCreateShortcut(m_hWnd);
			}
			break;
		case ID_FILE_RENAME:
			if (IsWindowOrChildrenFocused(m_wndListViewServer, ::GetFocus()))
			{
				//if (!m_nInCreatingLabelEditMode)
				//{
				//	if ((m_bSFTPMode && m_pChannel) || (!m_bSFTPMode && m_pConnection))
				//		m_wndListViewServer.DoRename();
				//}
				m_wndListViewServer.DoRename(m_hWnd);
			}
			else
			{
				m_wndListViewLocal.DoRename(m_hWnd);
			}
			break;
		case ID_FILE_PROPERTY:
			if (IsWindowOrChildrenFocused(m_wndListViewServer, ::GetFocus()))
				//DoProperty();
				m_wndListViewServer.DoProperty(m_hWnd);
			else
				m_wndListViewLocal.DoProperty(m_hWnd);
			break;
		case ID_FILE_EXIT:
			DestroyWindow();
			break;
		case ID_EDIT_CUT:
		case ID_EDIT_COPY:
			if (IsWindowOrChildrenFocused(m_wndListViewServer, ::GetFocus()))
				//DoCopy(LOWORD(wParam) == ID_EDIT_CUT);
				m_wndListViewServer.DoCopy(m_hWnd, LOWORD(wParam) == ID_EDIT_CUT);
			else
				m_wndListViewLocal.DoCopy(m_hWnd, LOWORD(wParam) == ID_EDIT_CUT);
			break;
		case ID_EDIT_PASTE:
			if (IsWindowOrChildrenFocused(m_wndListViewServer, ::GetFocus()))
				//DoPaste();
				m_wndListViewServer.DoPaste(m_hWnd);
			else
				m_wndListViewLocal.DoPaste(m_hWnd);
			break;
		case ID_EDIT_DELETE:
			DeleteSelection();
			break;
		case ID_EDIT_UPLOAD:
			DoUpload();
			break;
		case ID_EDIT_DOWNLOAD:
			DoDownload();
			break;
		case ID_EDIT_UPLOAD_ALL:
			DoUploadAll();
			break;
		case ID_EDIT_DOWNLOAD_ALL:
			DoDownloadAll();
			break;
		case ID_EDIT_SYNC_LEFT_TO_RIGHT:
			DoSyncLeftToRight();
			break;
		case ID_EDIT_SYNC_RIGHT_TO_LEFT:
			DoSyncRightToLeft();
			break;
		case ID_EDIT_SYNC_DETAIL:
			DoSyncDetail();
			break;
		case ID_EDIT_SELECT_ALL:
			if (IsWindowOrChildrenFocused(m_wndListViewServer, ::GetFocus()))
				//DoPaste();
				m_wndListViewServer.DoSelectAll(m_hWnd);
			else
				m_wndListViewLocal.DoSelectAll(m_hWnd);
			break;
		case ID_EDIT_INVERT_SELECTION:
			if (IsWindowOrChildrenFocused(m_wndListViewServer, ::GetFocus()))
				//DoPaste();
				m_wndListViewServer.DoInvertSelection(m_hWnd);
			else
				m_wndListViewLocal.DoInvertSelection(m_hWnd);
			break;
		//case ID_VIEW_TRANSFER:
		//	if (::IsWindowVisible(m_dlgTransfer))
		//		::ShowWindow(m_dlgTransfer, SW_HIDE);
		//	else
		//		::ShowWindow(m_dlgTransfer, SW_SHOW);
		//	break;
		case ID_VIEW_REFRESH:
			m_wndListViewLocal.Refresh();
			//UpdateServerFolderAbsolute(m_wndServerAddress.m_strDirectory);
			m_wndListViewServer.Refresh();
			break;
		case ID_VIEW_GO_FOLDER:
			OnLocalAddressCloseUp(0, 0);
			break;
		case ID_VIEW_PARENT_FOLDER:
			NavigateParentFolder();
			break;
		case ID_VIEW_SERVER_GO_FOLDER:
			OnServerAddressCloseUp(0, 0);
			break;
		case ID_VIEW_SERVER_PARENT_FOLDER:
			NavigateServerParentFolder();
			break;
		case ID_TRANSFER_MODE_AUTO:
		case ID_TRANSFER_MODE_TEXT:
		case ID_TRANSFER_MODE_BINARY:
#if ((ID_TRANSFER_MODE_TEXT - ID_TRANSFER_MODE_AUTO != TRANSFER_MODE_TEXT) || \
	(ID_TRANSFER_MODE_BINARY - ID_TRANSFER_MODE_AUTO != TRANSFER_MODE_BINARY))
	#error 
#endif
			_SetTransferMode((LONG) (LOWORD(wParam) - ID_TRANSFER_MODE_AUTO));
			break;
		case ID_TRANSFER_LOCAL_CRLF:
			_SetTextMode(false, TEXTMODE_LOCAL_CRLF);
			break;
		case ID_TRANSFER_LOCAL_CR:
			_SetTextMode(false, TEXTMODE_LOCAL_CR);
			break;
		case ID_TRANSFER_LOCAL_LF:
			_SetTextMode(false, TEXTMODE_LOCAL_LF);
			break;
		case ID_TRANSFER_SERVER_CRLF:
			_SetTextMode(true, TEXTMODE_SERVER_CRLF);
			break;
		case ID_TRANSFER_SERVER_CR:
			_SetTextMode(true, TEXTMODE_SERVER_CR);
			break;
		case ID_TRANSFER_SERVER_LF:
			_SetTextMode(true, TEXTMODE_SERVER_LF);
			break;
		case ID_TOOL_OPTION:
			ShowOption();
			break;
		case ID_HELP_ABOUT:
			ShowAboutDialog();
			break;
		default:
			return Default(wParam, lParam);
	}
	return 0;
}

LRESULT CMainWindow::OnInitMenuPopup(WPARAM wParam, LPARAM lParam)
{
	Default(wParam, lParam);

	HMENU hMenu = (HMENU) wParam;
	int nCount = ::GetMenuItemCount(hMenu);

	CMenuItem item(hMenu);
	for (int i = 0; i < nCount; i++)
	{
		item.m_uID = ::GetMenuItemID(hMenu, i);
		UpdateUIItem(&item);
	}

	//HWND hWndFocus = ::GetFocus();
	//if (IsWindowOrChildrenFocused(m_wndListViewLocal, hWndFocus))
	//	SendMessage(m_wndListViewLocal, WM_INITMENUPOPUP, wParam, lParam);
	//else if (IsWindowOrChildrenFocused(m_wndListViewServer, hWndFocus))
	//	SendMessage(m_wndListViewServer, WM_INITMENUPOPUP, wParam, lParam);
	if (m_hWndViewForMenu)
		::SendMessage(m_hWndViewForMenu, WM_INITMENUPOPUP, wParam, lParam);
	return 0;
}

void CMainWindow::UpdateFonts()
{
	HFONT hFontOld = m_hFontWindow;

	union
	{
		NONCLIENTMETRICSW ncmW;
		NONCLIENTMETRICS ncm;
	};
	ncmW.cbSize = sizeof(NONCLIENTMETRICSW);
	if (::_MySystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(ncmW), &ncmW, 0, m_uDpi))
		m_hFontWindow = ::CreateFontIndirectW(&ncmW.lfMessageFont);
	else
	{
		ncm.cbSize = NONCLIENTMETRICS_SIZE_V1;
		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, NONCLIENTMETRICS_SIZE_V1, &ncm, 0);
		m_hFontWindow = ::CreateFontIndirect(&ncm.lfMessageFont);
	}

	::SendMessage(m_wndAddress, WM_SETFONT, (WPARAM) m_hFontWindow, 0);
	::SendMessage(m_wndServerAddress, WM_SETFONT, (WPARAM) m_hFontWindow, 0);

	{
		TEXTMETRIC tm;
		HDC hDC = ::GetDC(m_hWnd);
		HGDIOBJ hgdiFont = ::SelectObject(hDC, m_hFontWindow);
		::GetTextMetrics(hDC, &tm);
		::SelectObject(hDC, hgdiFont);

		int rgBorders[3];
		::SendMessage(m_wndStatusBar, SB_GETBORDERS, 0, (LPARAM) &rgBorders);

		register int cy = tm.tmHeight - tm.tmInternalLeading - 1;
		if (cy < 16)
			cy = 16;
		m_nStatusHeight = cy +
			rgBorders[1] * 2 + ::GetSystemMetrics(SM_CYBORDER) * 2 + 2;
	}

	if (hFontOld)
		::DeleteObject(hFontOld);
}

void CMainWindow::UpdateToolBarEnable()
{
	CToolBarItem item(m_wndToolBar);

	for (int i = 0; i < sizeof(s_arrToolBarButtons) / sizeof(s_arrToolBarButtons[0]); i++)
	{
		if (s_arrToolBarButtons[i].uID)
		{
			item.m_uID = s_arrToolBarButtons[i].uID;
			UpdateUIItem(&item);
		}
	}
	item.m_hWndToolBar = m_wndAddrButtons;
	for (int i = 0; i < sizeof(s_arrAddrButtons) / sizeof(s_arrAddrButtons[0]); i++)
	{
		if (s_arrAddrButtons[i].uID)
		{
			item.m_uID = s_arrAddrButtons[i].uID;
			UpdateUIItem(&item);
		}
	}
	item.m_hWndToolBar = m_wndServerAddrButtons;
	for (int i = 0; i < sizeof(s_arrServerAddrButtons) / sizeof(s_arrServerAddrButtons[0]); i++)
	{
		if (s_arrServerAddrButtons[i].uID)
		{
			item.m_uID = s_arrServerAddrButtons[i].uID;
			UpdateUIItem(&item);
		}
	}
}

void CMainWindow::UpdateToolBarIcons()
{
	if (m_uDpi >= USER_DEFAULT_SCREEN_DPI * 2)
	{
		::SendMessage(m_wndAddrButtons, TB_SETIMAGELIST, 0, (LPARAM) theApp.m_hImageListAddrButtonsL);
		::SendMessage(m_wndServerAddrButtons, TB_SETIMAGELIST, 0, (LPARAM) theApp.m_hImageListAddrButtonsL);
		::SendMessage(m_wndToolBar, TB_SETIMAGELIST, 0, (LPARAM) theApp.m_hImageListToolBarL);
	}
	else
	{
		::SendMessage(m_wndAddrButtons, TB_SETIMAGELIST, 0, (LPARAM) theApp.m_hImageListAddrButtons);
		::SendMessage(m_wndServerAddrButtons, TB_SETIMAGELIST, 0, (LPARAM) theApp.m_hImageListAddrButtons);
		::SendMessage(m_wndToolBar, TB_SETIMAGELIST, 0, (LPARAM) theApp.m_hImageListToolBar);
	}
}

static INT_PTR CALLBACK _AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
		{
			IEasySFTPRoot* pRoot = NULL;
			auto hr = theApp.m_pEasySFTPRoot->QueryInterface(IID_IEasySFTPRoot, reinterpret_cast<void**>(&pRoot));
			if (SUCCEEDED(hr))
			{
				CMyStringW str;
				BSTR bstr = NULL;
				hr = pRoot->get_Version(&bstr);
				if (SUCCEEDED(hr))
				{
					MyBSTRToString(bstr, str);
					::SysFreeString(bstr);
					str.InsertString(L" version ", 0);
					str.InsertString(theApp.m_strTitle, 0);
					::MySetDlgItemTextW(hDlg, IDC_VERSION, str);
				}
				hr = pRoot->GetDependencyLibraryInfo(&bstr);
				if (SUCCEEDED(hr))
				{
					MyBSTRToString(bstr, str);
					::SysFreeString(bstr);
					::MySetDlgItemTextW(hDlg, IDC_FEATURES, str);
				}
				pRoot->Release();
			}
		}
		break;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR) TRUE;
			}
			break;
		case WM_NOTIFY:
		{
			auto lpnmh = reinterpret_cast<LPNMHDR>(lParam);
			if (lpnmh->code == NM_CLICK && lpnmh->idFrom == IDC_LINK_TO_REPOSITORY)
			{
				::ShellExecuteW(hDlg, L"open", reinterpret_cast<PNMLINK>(lpnmh)->item.szUrl, nullptr, nullptr, SW_SHOW);
			}
		}
		break;
	}
	return (INT_PTR) FALSE;
}

void CMainWindow::ShowAboutDialog()
{
	ExDialogBoxParam(theApp.m_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), m_hWnd, _AboutDlgProc, 0);
}

void CMainWindow::OnResize()
{
	RECT rc, rc2;
	int nHalf;

	if (!m_hMenu)
		return;

	if (m_nToolBarHeight == -1)
	{
		::GetWindowRect(m_wndToolBar, &rc);
		m_nToolBarHeight = rc.bottom - rc.top;
	}
	if (m_nAddrButtonsWidth == -1)
	{
		::SendMessage(m_wndAddrButtons, TB_GETITEMRECT,
			(WPARAM)((sizeof(s_arrAddrButtons) / sizeof(s_arrAddrButtons[0])) - 1), (LPARAM)(LPRECT)&rc);
		m_nAddrButtonsWidth = rc.right;
	}
	::GetClientRect(m_hWnd, &rc);
	::GetClientRect(m_wndAddress, &rc2);
	if (theApp.m_nSplitterPos == -1)
	{
		if (m_nSplitterWidth == -1)
			nHalf = rc.right / 2;
		else
			nHalf = theApp.m_nSplitterPos = rc.right / 2 - m_nSplitterWidth / 2;
	}
	else
		nHalf = theApp.m_nSplitterPos;
	rc.bottom -= m_nStatusHeight;

	//HDWP hDWP = ::BeginDeferWindowPos(11);
#define MY_RESIZE_WINDOW(hWnd, x, y, cx, cy) MoveWindow(hWnd, x, y, cx, cy, TRUE)
	MY_RESIZE_WINDOW(m_wndToolBar, 0, 0, rc.right, m_nToolBarHeight);

	MY_RESIZE_WINDOW(m_wndAddress, 0, m_nToolBarHeight, nHalf - m_nAddrButtonsWidth, rc2.bottom);
	MY_RESIZE_WINDOW(m_wndAddrButtons, nHalf - m_nAddrButtonsWidth, m_nToolBarHeight,
		m_nAddrButtonsWidth, rc2.bottom);
	MY_RESIZE_WINDOW(m_xBrowserForLocal, 0, rc2.bottom + m_nToolBarHeight,
		nHalf, rc.bottom - rc2.bottom - m_nToolBarHeight);
	if (m_wndListViewLocal.m_hWnd)
		MY_RESIZE_WINDOW(m_wndListViewLocal, 0, 0,
			nHalf, rc.bottom - rc2.bottom - m_nToolBarHeight);

	MY_RESIZE_WINDOW(m_wndSplitter, nHalf, m_nToolBarHeight,
		m_nSplitterWidth, rc.bottom - m_nToolBarHeight);

	nHalf += m_nSplitterWidth;
	MY_RESIZE_WINDOW(m_wndServerAddress, nHalf, m_nToolBarHeight,
		rc.right - nHalf - m_nAddrButtonsWidth, rc2.bottom);
	MY_RESIZE_WINDOW(m_wndServerAddrButtons, rc.right - m_nAddrButtonsWidth, m_nToolBarHeight,
		m_nAddrButtonsWidth, rc2.bottom);
	MY_RESIZE_WINDOW(m_xBrowserForServer, nHalf, rc2.bottom + m_nToolBarHeight,
		rc.right - nHalf, rc.bottom - rc2.bottom - m_nToolBarHeight);
	if (m_wndListViewServer.m_hWnd)
		MY_RESIZE_WINDOW(m_wndListViewServer, 0, 0,
			rc.right - nHalf, rc.bottom - rc2.bottom - m_nToolBarHeight);

	MY_RESIZE_WINDOW(m_wndStatusBar, 0, rc.bottom, rc.right, m_nStatusHeight);
	//::EndDeferWindowPos(hDWP);
	UpdateStatusParts();
}

void CMainWindow::UpdateUIItem(CCommandUIItem* pUIItem)
{
//#define IS_CONNECTED()   (m_bSFTPMode ? m_pClient != NULL : m_pConnection != NULL)
#define IS_CONNECTED()   (true)
	HWND hWndFocus = ::GetFocus();
	switch (pUIItem->GetID())
	{
		case ID_FILE_CONNECT:
			pUIItem->Enable(CanConnect(IsWindowOrChildrenFocused(m_wndListViewServer, hWndFocus)));
			break;
		case ID_FILE_QUICK_CONNECT:
			//pUIItem->Enable(!IS_CONNECTED());
			break;
		case ID_FILE_NEW_FOLDER:
		case ID_FILE_RENAME:
			if (IsWindowOrChildrenFocused(m_wndListViewServer, hWndFocus))
				pUIItem->Enable(IS_CONNECTED());
			else
				pUIItem->Enable(IsWindowOrChildrenFocused(m_wndListViewLocal, hWndFocus));
			break;
		case ID_FILE_OPEN:
		case ID_FILE_CREATE_SHORTCUT:
			if (IsWindowOrChildrenFocused(m_wndListViewServer, ::GetFocus()))
				pUIItem->Enable(IS_CONNECTED());
			else
				pUIItem->Enable(IsWindowOrChildrenFocused(m_wndListViewLocal, hWndFocus) &&
					m_wndListViewLocal.GetSelectionCount() == 1);
			break;
		case ID_FILE_PROPERTY:
			//break;
		case ID_EDIT_CUT:
		case ID_EDIT_COPY:
		case ID_EDIT_DELETE:
			if (IsWindowOrChildrenFocused(m_wndListViewServer, hWndFocus))
				pUIItem->Enable(IS_CONNECTED() && m_wndListViewServer.GetSelectionCount() > 0);
			else
				pUIItem->Enable(IsWindowOrChildrenFocused(m_wndListViewLocal, hWndFocus) &&
					m_wndListViewLocal.GetSelectionCount() > 0);
			break;
		case ID_FILE_DISCONNECT:
		{
			VARIANT_BOOL bConnected = VARIANT_FALSE;
			if (IsWindowOrChildrenFocused(m_wndListViewServer, hWndFocus))
			{
				if (m_wndListViewServer.m_pRootDirectory)
					m_wndListViewServer.m_pRootDirectory->get_Connected(&bConnected);
			}
			else
			{
				if (m_wndListViewLocal.m_pRootDirectory)
					m_wndListViewLocal.m_pRootDirectory->get_Connected(&bConnected);
			}
			pUIItem->Enable(bConnected == VARIANT_TRUE);
		}
		break;
		//case ID_VIEW_REFRESH:
		//case ID_VIEW_SERVER_GO_FOLDER:
		//	pUIItem->Enable(IS_CONNECTED());
		//	break;
		case ID_FILE_SAVE_AS:
		case ID_EDIT_DOWNLOAD:
			pUIItem->Enable(IS_CONNECTED() && IsWindowOrChildrenFocused(m_wndListViewServer, hWndFocus) &&
				m_wndListViewServer.GetSelectionCount() > 0);
			break;
		case ID_EDIT_UPLOAD:
			pUIItem->Enable(IS_CONNECTED() && IsWindowOrChildrenFocused(m_wndListViewLocal, hWndFocus) &&
				m_wndListViewLocal.GetSelectionCount() > 0);
			break;
		case ID_EDIT_DOWNLOAD_ALL:
		case ID_EDIT_UPLOAD_ALL:
			pUIItem->Enable(IS_CONNECTED());
			break;
		case ID_EDIT_SYNC_LEFT_TO_RIGHT:
			pUIItem->Enable(IsSynchronizationSupported(m_wndListViewLocal.m_pFolder, m_wndListViewServer.m_pFolder));
			break;
		case ID_EDIT_SYNC_RIGHT_TO_LEFT:
			pUIItem->Enable(IsSynchronizationSupported(m_wndListViewServer.m_pFolder, m_wndListViewLocal.m_pFolder));
			break;
		//case ID_EDIT_PASTE:
		//	if (IsWindowOrChildrenFocused(m_wndListViewServer, hWndFocus))
		//		//pUIItem->Enable(IS_CONNECTED() && CanPaste());
		//		pUIItem->Enable(IS_CONNECTED());
		//	else
		//		pUIItem->Enable(IsWindowOrChildrenFocused(m_wndListViewLocal, hWndFocus));
		//	break;
		//case ID_VIEW_TRANSFER:
		//	pUIItem->Check(::IsWindowVisible(m_dlgTransfer) != 0);
		//	break;
		case ID_VIEW_PARENT_FOLDER:
			pUIItem->Enable(!IsDesktopIDList(m_wndListViewLocal.m_lpidlAbsoluteMe));
			break;
		case ID_VIEW_SERVER_PARENT_FOLDER:
			//pUIItem->Enable(IS_CONNECTED() && m_wndServerAddress.m_strDirectory.Compare(L"/") != 0);
			pUIItem->Enable(!IsDesktopIDList(m_wndListViewServer.m_lpidlAbsoluteMe));
			break;
		case ID_TRANSFER_MODE_AUTO:
			pUIItem->Enable(m_wndListViewServer.m_pRootDirectory != NULL);
			if (m_wndListViewServer.m_pRootDirectory)
			{
				EasySFTPTransferMode nTMode;
				m_wndListViewServer.m_pRootDirectory->get_TransferMode(&nTMode);
				pUIItem->Check(nTMode == EasySFTPTransferMode::Auto);
			}
			break;
		case ID_TRANSFER_MODE_TEXT:
			pUIItem->Enable(m_wndListViewServer.m_pRootDirectory != NULL);
			if (m_wndListViewServer.m_pRootDirectory)
			{
				EasySFTPTransferMode nTMode;
				m_wndListViewServer.m_pRootDirectory->get_TransferMode(&nTMode);
				pUIItem->Check(nTMode == EasySFTPTransferMode::Text);
			}
			break;
		case ID_TRANSFER_MODE_BINARY:
			pUIItem->Enable(m_wndListViewServer.m_pRootDirectory != NULL);
			if (m_wndListViewServer.m_pRootDirectory)
			{
				EasySFTPTransferMode nTMode;
				m_wndListViewServer.m_pRootDirectory->get_TransferMode(&nTMode);
				pUIItem->Check(nTMode == EasySFTPTransferMode::Binary);
			}
			break;
		case ID_TRANSFER_LOCAL_MODE:
		{
			pUIItem->Enable(m_wndListViewServer.m_pRootDirectory != NULL);
			if (m_wndListViewServer.m_pRootDirectory)
			{
				int iImage;
				EasySFTPTextMode nTMode;
				m_wndListViewServer.m_pRootDirectory->get_TextMode(&nTMode);
				if ((nTMode & EasySFTPTextMode::BufferMask) == EasySFTPTextMode::BufferCr)
					iImage = 9;
				else if ((nTMode & EasySFTPTextMode::BufferMask) == EasySFTPTextMode::BufferLf)
					iImage = 10;
				else
					iImage = 8;
				pUIItem->ChangeBitmap(iImage);
			}
		}
		break;
		case ID_TRANSFER_SERVER_MODE:
		{
			pUIItem->Enable(m_wndListViewServer.m_pRootDirectory != NULL);
			if (m_wndListViewServer.m_pRootDirectory)
			{
				int iImage;
				EasySFTPTextMode nTMode;
				m_wndListViewServer.m_pRootDirectory->get_TextMode(&nTMode);
				if ((nTMode & EasySFTPTextMode::StreamMask) == EasySFTPTextMode::StreamCr)
					iImage = 12;
				else if ((nTMode & EasySFTPTextMode::StreamMask) == EasySFTPTextMode::StreamLf)
					iImage = 13;
				else
					iImage = 11;
				pUIItem->ChangeBitmap(iImage);
			}
		}
		break;
		case ID_TRANSFER_LOCAL_CRLF:
			pUIItem->Enable(m_wndListViewServer.m_pRootDirectory != NULL);
			if (m_wndListViewServer.m_pRootDirectory)
			{
				EasySFTPTextMode nTMode;
				m_wndListViewServer.m_pRootDirectory->get_TextMode(&nTMode);
				pUIItem->Check((nTMode & EasySFTPTextMode::BufferMask) == EasySFTPTextMode::BufferCrLf);
			}
			break;
		case ID_TRANSFER_LOCAL_CR:
			pUIItem->Enable(m_wndListViewServer.m_pRootDirectory != NULL);
			if (m_wndListViewServer.m_pRootDirectory)
			{
				EasySFTPTextMode nTMode;
				m_wndListViewServer.m_pRootDirectory->get_TextMode(&nTMode);
				pUIItem->Check((nTMode & EasySFTPTextMode::BufferMask) == EasySFTPTextMode::BufferCr);
			}
			break;
		case ID_TRANSFER_LOCAL_LF:
			pUIItem->Enable(m_wndListViewServer.m_pRootDirectory != NULL);
			if (m_wndListViewServer.m_pRootDirectory)
			{
				EasySFTPTextMode nTMode;
				m_wndListViewServer.m_pRootDirectory->get_TextMode(&nTMode);
				pUIItem->Check((nTMode & EasySFTPTextMode::BufferMask) == EasySFTPTextMode::BufferLf);
			}
			break;
		case ID_TRANSFER_SERVER_CRLF:
			pUIItem->Enable(m_wndListViewServer.m_pRootDirectory != NULL);
			if (m_wndListViewServer.m_pRootDirectory)
			{
				EasySFTPTextMode nTMode;
				m_wndListViewServer.m_pRootDirectory->get_TextMode(&nTMode);
				pUIItem->Check((nTMode & EasySFTPTextMode::StreamMask) == EasySFTPTextMode::StreamCrLf);
			}
			break;
		case ID_TRANSFER_SERVER_CR:
			pUIItem->Enable(m_wndListViewServer.m_pRootDirectory != NULL);
			if (m_wndListViewServer.m_pRootDirectory)
			{
				EasySFTPTextMode nTMode;
				m_wndListViewServer.m_pRootDirectory->get_TextMode(&nTMode);
				pUIItem->Check((nTMode & EasySFTPTextMode::StreamMask) == EasySFTPTextMode::StreamCr);
			}
			break;
		case ID_TRANSFER_SERVER_LF:
			pUIItem->Enable(m_wndListViewServer.m_pRootDirectory != NULL);
			if (m_wndListViewServer.m_pRootDirectory)
			{
				EasySFTPTextMode nTMode;
				m_wndListViewServer.m_pRootDirectory->get_TextMode(&nTMode);
				pUIItem->Check((nTMode & EasySFTPTextMode::StreamMask) == EasySFTPTextMode::StreamLf);
			}
			break;
	}
#undef IS_CONNECTED
}

LRESULT CMainWindow::OnMenuSelect(WPARAM wParam, LPARAM lParam)
{
	Default(wParam, lParam);

	if (HIWORD(wParam) == 0xFFFF && !lParam)
		SetStatusText(NULL);
	else
	{
		UINT uID = LOWORD(wParam);
		if (!uID)
			SetStatusText(L"");
		else if (uID >= FCIDM_BROWSERFIRST && uID <= FCIDM_BROWSERLAST)
			SetStatusText(MAKEINTRESOURCEW(uID));
		else if (m_hWndViewForMenu)
			::SendMessage(m_hWndViewForMenu, WM_MENUSELECT, wParam, lParam);
	}

	return 0;
}

LRESULT CMainWindow::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	//if ((HWND) wParam == m_wndListViewServer)
	//{
	//	POINT pt;
	//	int i;
	//	pt.x = (int)(short) LOWORD(lParam);
	//	pt.y = (int)(short) HIWORD(lParam);
	//	if (!m_wndListViewServer.GetNextSelectedItem(-1, &i, NULL))
	//		i = -1;
	//	if (pt.x == -1 && pt.y == -1)
	//	{
	//		if (i == -1)
	//			pt.x = pt.y = 3;
	//		else
	//			::SendMessage(m_wndListViewServer, LVM_GETITEMPOSITION, (WPARAM) (i), (LPARAM)(LPPOINT) &pt);
	//		::ClientToScreen(m_wndListViewServer, &pt);
	//	}
	//	else
	//		::GetCursorPos(&pt);
	//	::TrackPopupMenuEx(::GetSubMenu(theApp.m_hMenuPopup,
	//			i == -1 ? POPUP_POS_SERVERLISTVIEW_NOSEL : POPUP_POS_SERVERLISTVIEW),
	//		TPM_LEFTALIGN, pt.x, pt.y, m_hWnd, NULL);
	//	return 0;
	//}
	//else
		return Default(wParam, lParam);
}

LRESULT CMainWindow::OnActivate(WPARAM wParam, LPARAM lParam)
{
	if (wParam != WA_INACTIVE)
	{
		if (m_wndListViewLocal.m_hWnd == m_hWndFocusSaved)
		{
			if (m_wndListViewServer.m_pView)
				m_wndListViewServer.m_pView->UIActivate(SVUIA_ACTIVATE_NOFOCUS);
			if (m_wndListViewLocal.m_pView)
				m_wndListViewLocal.m_pView->UIActivate(SVUIA_ACTIVATE_FOCUS);
		}
		else if (m_wndListViewServer.m_hWnd == m_hWndFocusSaved)
		{
			if (m_wndListViewLocal.m_pView)
				m_wndListViewLocal.m_pView->UIActivate(SVUIA_ACTIVATE_NOFOCUS);
			if (m_wndListViewServer.m_pView)
				m_wndListViewServer.m_pView->UIActivate(SVUIA_ACTIVATE_FOCUS);
		}
		else
		{
			if (m_wndListViewLocal.m_pView)
				m_wndListViewLocal.m_pView->UIActivate(SVUIA_ACTIVATE_NOFOCUS);
			if (m_wndListViewServer.m_pView)
				m_wndListViewServer.m_pView->UIActivate(SVUIA_ACTIVATE_NOFOCUS);
			if (m_hWndFocusSaved)
				::SetFocus(m_hWndFocusSaved);
		}
	}
	else
	{
		if (m_wndListViewLocal.m_pView)
			m_wndListViewLocal.m_pView->UIActivate(SVUIA_DEACTIVATE);
		if (m_wndListViewServer.m_pView)
			m_wndListViewServer.m_pView->UIActivate(SVUIA_DEACTIVATE);
		m_hWndFocusSaved = ::GetFocus();
	}
	return 0;
}

LRESULT CMainWindow::OnSettingChange(WPARAM wParam, LPARAM lParam)
{
	Default(wParam, lParam);

	UpdateToolBarIcons();
	m_wndAddress.RefreshImageList();
	m_wndServerAddress.RefreshImageList();
	OnResize();
	return 0;
}

LRESULT CMainWindow::OnDpiChanged(WPARAM wParam, LPARAM lParam)
{
	Default(wParam, lParam);

	MyFileIconInit(TRUE);

	UINT newDpi = HIWORD(wParam);
	theApp.m_nSplitterPos = ::MulDiv(theApp.m_nSplitterPos, static_cast<int>(newDpi), static_cast<int>(m_uDpi));
	m_uDpi = newDpi;

	LPRECT lprc = reinterpret_cast<LPRECT>(lParam);
	UpdateToolBarIcons();
	UpdateFonts();
	UpdateStatusParts();
	m_wndAddress.RefreshImageList();
	m_wndServerAddress.RefreshImageList();
	::SetWindowPos(m_hWnd, NULL, lprc->left, lprc->top, lprc->right - lprc->left, lprc->bottom - lprc->top,
		SWP_NOZORDER);
	//OnResize();
	return 0;
}

static bool __stdcall GetToolTipTextFromID(UINT uID, CMyStringW& rstrText)
{
	LPCWSTR lpw, lpw2;
	if (!rstrText.LoadString(uID))
		return false;
	lpw = rstrText;
	lpw2 = wcschr(lpw, L'\n');
	if (!lpw2)
	{
		rstrText.Empty();
		return false;
	}
	rstrText.DeleteString(0, (DWORD) (((DWORD_PTR) lpw2 - (DWORD_PTR) lpw) / sizeof(WCHAR) + 1));
	return true;
}

LRESULT CMainWindow::OnToolTipDispInfoA(WPARAM wParam, LPARAM lParam)
{
	LPNMTTDISPINFOA lptt = (LPNMTTDISPINFOA) lParam;
	//CMyStringW str;
	//if (GetToolTipTextFromID(lptt->hdr.idFrom, str))
	//{
	//	strncpy_s(lptt->szText, str, 80);
	//	lptt->uFlags = TTF_DI_SETITEM;
	//}
	if (GetToolTipTextFromID((UINT) lptt->hdr.idFrom, m_strToolTipTextKeep))
	{
		lptt->lpszText = (LPSTR)(LPCSTR) m_strToolTipTextKeep;
		lptt->uFlags = TTF_DI_SETITEM;
	}
	return 0;
}

LRESULT CMainWindow::OnToolTipDispInfoW(WPARAM wParam, LPARAM lParam)
{
	LPNMTTDISPINFOW lptt = (LPNMTTDISPINFOW) lParam;
	//CMyStringW str;
	//if (GetToolTipTextFromID(lptt->hdr.idFrom, str))
	//{
	//	wcsncpy_s(lptt->szText, str, 80);
	//	lptt->uFlags = TTF_DI_SETITEM;
	//}
	if (GetToolTipTextFromID((UINT) lptt->hdr.idFrom, m_strToolTipTextKeep))
	{
		lptt->lpszText = (LPWSTR)(LPCWSTR) m_strToolTipTextKeep;
		lptt->uFlags = TTF_DI_SETITEM;
	}
	return 0;
}

LRESULT CMainWindow::OnToolBarDropDown(WPARAM wParam, LPARAM lParam)
{
	// NMTOOLBARA and NMTOOLBARW are the same
	LPNMTOOLBAR lptb = (LPNMTOOLBAR) lParam;
	bool bServer;
	EasySFTPTextModeFlags nTMode = EasySFTPTextMode::NoConversion;
	if (m_wndListViewServer.m_pRootDirectory)
		m_wndListViewServer.m_pRootDirectory->get_TextMode(&nTMode);

	CMenuItem mi(m_hMenuReturnMode);
	switch (lptb->iItem)
	{
		case ID_TRANSFER_LOCAL_MODE:
			bServer = false;
			mi.m_uID = ID_RETURN_MODE_CRLF;
			mi.Check((nTMode & EasySFTPTextMode::BufferMask) == EasySFTPTextMode::BufferCrLf);
			mi.m_uID = ID_RETURN_MODE_CR;
			mi.Check((nTMode & EasySFTPTextMode::BufferMask) == EasySFTPTextMode::BufferCr);
			mi.m_uID = ID_RETURN_MODE_LF;
			mi.Check((nTMode & EasySFTPTextMode::BufferMask) == EasySFTPTextMode::BufferLf);
			break;
		case ID_TRANSFER_SERVER_MODE:
			bServer = true;
			mi.m_uID = ID_RETURN_MODE_CRLF;
			mi.Check((nTMode & EasySFTPTextMode::StreamMask) == EasySFTPTextMode::StreamCrLf);
			mi.m_uID = ID_RETURN_MODE_CR;
			mi.Check((nTMode & EasySFTPTextMode::StreamMask) == EasySFTPTextMode::StreamCr);
			mi.m_uID = ID_RETURN_MODE_LF;
			mi.Check((nTMode & EasySFTPTextMode::StreamMask) == EasySFTPTextMode::StreamLf);
			break;
		default:
			return TBDDRET_NODEFAULT;
	}
	int iIndex;
	for (iIndex = 0; iIndex < sizeof(s_arrToolBarButtons) / sizeof(s_arrToolBarButtons[0]); iIndex++)
	{
		if (s_arrToolBarButtons[iIndex].uID == lptb->iItem)
			break;
	}

	TPMPARAMS tp;
	tp.cbSize = sizeof(tp);
	::SendMessage(m_wndToolBar, TB_GETITEMRECT, (WPARAM) iIndex, (LPARAM) &tp.rcExclude);
	::ClientToScreen(m_wndToolBar, ((LPPOINT) &tp.rcExclude));
	::ClientToScreen(m_wndToolBar, ((LPPOINT) &tp.rcExclude) + 1);
	//UINT uRet = (UINT) ::TrackPopupMenuEx(m_hMenuReturnMode, TPM_RETURNCMD | TPM_VERTICAL,
	//	tp.rcExclude.left, tp.rcExclude.top, m_hWnd, &tp);
	UINT uRet = (UINT) ::TrackPopupMenuEx(m_hMenuReturnMode, TPM_RETURNCMD | TPM_VERTICAL,
		tp.rcExclude.left, tp.rcExclude.top, m_wndToolBar, &tp);
	if (uRet != 0 && m_wndListViewServer.m_pRootDirectory)
	{
		if (!bServer)
		{
			nTMode &= ~EasySFTPTextMode::BufferMask;
			switch (uRet)
			{
				case ID_RETURN_MODE_CRLF: nTMode |= EasySFTPTextMode::BufferCrLf; break;
				case ID_RETURN_MODE_CR:   nTMode |= EasySFTPTextMode::BufferCr; break;
				case ID_RETURN_MODE_LF:   nTMode |= EasySFTPTextMode::BufferLf; break;
			}
		}
		else
		{
			nTMode &= ~EasySFTPTextMode::StreamMask;
			switch (uRet)
			{
				case ID_RETURN_MODE_CRLF: nTMode |= EasySFTPTextMode::StreamCrLf; break;
				case ID_RETURN_MODE_CR:   nTMode |= EasySFTPTextMode::StreamCr; break;
				case ID_RETURN_MODE_LF:   nTMode |= EasySFTPTextMode::StreamLf; break;
			}
		}
		m_wndListViewServer.m_pRootDirectory->put_TextMode(nTMode);
	}

	return TBDDRET_DEFAULT;
}

static PIDLIST_ABSOLUTE __stdcall GetEasySFTPItemIfAvailable(HWND hWnd, LPCWSTR lpszAddress)
{
	if (wcschr(lpszAddress, L':') == NULL)
		return NULL;

	IShellFolder* pRoot;
	PIDLIST_ABSOLUTE pidlRet = NULL;
	if (SUCCEEDED(theApp.m_pEasySFTPRoot->QueryInterface(IID_IShellFolder, (void**) &pRoot)))
	{
		PIDLIST_RELATIVE pidlRel = NULL;
		IEasySFTPDirectory* pDir = NULL;
		if (SUCCEEDED(pRoot->ParseDisplayName(hWnd, NULL, (LPWSTR) lpszAddress, NULL, &pidlRel, NULL)))
		{
			IShellFolder* pFld = NULL;
			if (SUCCEEDED(pRoot->BindToObject(pidlRel, NULL, IID_IShellFolder, (void**) &pFld)) && pFld)
			{
				if (FAILED(pFld->QueryInterface(IID_IEasySFTPDirectory, (void**) &pDir)))
					pDir = NULL;
				pFld->Release();
			}
		}
		pRoot->Release();
		if (pDir)
		{
			//VARIANT_BOOL b1, b2;
			//int n1, n2;
			//BSTR bstr1, bstr2;
			//if (SUCCEEDED(pDir->GetHostInfo(&b1, &n1, &bstr1)))
			//{
			//	if (pDirectoryCurrent && SUCCEEDED(pDirectoryCurrent->GetHostInfo(&b2, &n2, &bstr2)))
			//	{
			//		if (b1 == b2 && n1 == n2 && _wcsicmp(bstr1, bstr2) == 0)
			//			pidlRet = ::AppendItemIDList(theApp.m_pidlEasySFTP, pidlRel);
			//		::SysFreeString(bstr2);
			//	}
			//	// ホストが一致しなくても sftp モードであれば IDLIST を生成して成功にする
			//	if (!pidlRet && b1)
					pidlRet = ::AppendItemIDList(theApp.m_pidlEasySFTP, pidlRel);
			//	::SysFreeString(bstr1);
			//}
			//pDir->Release();
		}
		if (pidlRel)
			::CoTaskMemFree(pidlRel);
	}
	return pidlRet;
}

void CMainWindow::OnLocalAddressTextReturn(LPCWSTR lpszText)
{
	PIDLIST_ABSOLUTE pidl;
	pidl = (PIDLIST_ABSOLUTE) m_wndAddress.FindItemFromDisplayName(lpszText);
	if (pidl)
	{
		if (!IsEqualIDList(pidl, m_wndListViewLocal.m_lpidlAbsoluteMe))
			UpdateCurrentFolderAbsolute(pidl);
		return;
	}
	pidl = GetEasySFTPItemIfAvailable(m_hWnd, lpszText);
	if (pidl)
	{
		UpdateCurrentFolderAbsolute(pidl);
		::CoTaskMemFree(pidl);
		return;
	}

	IShellFolder* pDesktop;
	if (SUCCEEDED(::SHGetDesktopFolder(&pDesktop)))
	{
		DWORD dwAttrs = SFGAO_FOLDER | SFGAO_FILESYSANCESTOR;
		HRESULT hr = pDesktop->ParseDisplayName(m_hWnd, NULL, (LPWSTR) lpszText, NULL, (PIDLIST_RELATIVE*) &pidl, &dwAttrs);
		if (SUCCEEDED(hr))
		{
			UpdateCurrentFolderAbsolute(pidl);
			::CoTaskMemFree(pidl);
		}
		else
		{
			m_wndAddress.RestoreTextBox();
			SetStatusText(MAKEINTRESOURCEW(IDS_DIRCHANGE_FAILED));
			::MessageBeep(MB_ICONEXCLAMATION);
		}
		pDesktop->Release();
	}
}

LRESULT CMainWindow::OnLocalAddressEndEditA(WPARAM wParam, LPARAM lParam)
{
	NMCBEENDEDITA* pnmce = (NMCBEENDEDITA*) lParam;
	CMyStringW str(pnmce->szText);
	if (m_wndAddress.HandleEndEdit(pnmce->iWhy, pnmce->fChanged != 0, str, m_wndListViewLocal))
		OnLocalAddressTextReturn(str);
	return 0;
}

LRESULT CMainWindow::OnLocalAddressEndEditW(WPARAM wParam, LPARAM lParam)
{
	NMCBEENDEDITW* pnmce = (NMCBEENDEDITW*) lParam;
	if (m_wndAddress.HandleEndEdit(pnmce->iWhy, pnmce->fChanged != 0, pnmce->szText, m_wndListViewLocal))
		OnLocalAddressTextReturn(pnmce->szText);
	return 0;
}

LRESULT CMainWindow::OnLocalAddressSelChange(WPARAM wParam, LPARAM lParam)
{
	if (!::SendMessage(m_wndAddress, CB_GETDROPPEDSTATE, 0, 0))
	{
		//OnLocalAddressCloseUp(0, 0);
		PCIDLIST_ABSOLUTE lpidl = m_wndAddress.GetSelectedFolder();
		if (lpidl && !IsEqualIDList(lpidl, m_wndListViewLocal.m_lpidlAbsoluteMe))
			UpdateCurrentFolderAbsolute(lpidl);
		else
			m_wndAddress.RestoreTextBox();
	}
	else
		m_bLocalAddressSelChanged = true;
	return 0;
}

LRESULT CMainWindow::OnLocalAddressCloseUp(WPARAM wParam, LPARAM lParam)
{
	if (m_bLocalAddressSelChanged)
	{
		m_bLocalAddressSelChanged = false;
		//PCIDLIST_ABSOLUTE lpidl = m_wndAddress.GetSelectedFolder();
		//if (lpidl && !IsEqualIDList(lpidl, m_wndListViewLocal.m_lpidlAbsoluteMe))
		//	UpdateCurrentFolderAbsolute(lpidl);
		//else
			m_wndAddress.RestoreTextBox();
	}
	return 0;
}

void CMainWindow::OnServerAddressTextReturn(LPCWSTR lpszText)
{
	PIDLIST_ABSOLUTE pidl;
	pidl = (PIDLIST_ABSOLUTE) m_wndServerAddress.FindItemFromDisplayName(lpszText);
	if (pidl)
	{
		if (!IsEqualIDList(pidl, m_wndListViewServer.m_lpidlAbsoluteMe))
			UpdateServerFolderAbsolute(pidl);
		return;
	}
	pidl = GetEasySFTPItemIfAvailable(m_hWnd, lpszText);
	if (pidl)
	{
		UpdateServerFolderAbsolute(pidl);
		::CoTaskMemFree(pidl);
		return;
	}

	IShellFolder* pDesktop;
	if (SUCCEEDED(::SHGetDesktopFolder(&pDesktop)))
	{
		DWORD dwAttrs = SFGAO_FOLDER | SFGAO_FILESYSANCESTOR;
		HRESULT hr = pDesktop->ParseDisplayName(m_hWnd, NULL, (LPWSTR) lpszText, NULL, (PIDLIST_RELATIVE*) &pidl, &dwAttrs);
		if (SUCCEEDED(hr))
		{
			UpdateServerFolderAbsolute(pidl);
			::CoTaskMemFree(pidl);
		}
		else
		{
			m_wndServerAddress.RestoreTextBox();
			SetStatusText(MAKEINTRESOURCEW(IDS_DIRCHANGE_FAILED));
			::MessageBeep(MB_ICONEXCLAMATION);
		}
		pDesktop->Release();
	}
}

LRESULT CMainWindow::OnServerAddressEndEditA(WPARAM wParam, LPARAM lParam)
{
	NMCBEENDEDITA* pnmce = (NMCBEENDEDITA*) lParam;
	CMyStringW str(pnmce->szText);
	if (m_wndServerAddress.HandleEndEdit(pnmce->iWhy, pnmce->fChanged != 0, str, m_wndListViewServer))
		OnServerAddressTextReturn(str);
	return 0;
}

LRESULT CMainWindow::OnServerAddressEndEditW(WPARAM wParam, LPARAM lParam)
{
	NMCBEENDEDITW* pnmce = (NMCBEENDEDITW*) lParam;
	if (m_wndServerAddress.HandleEndEdit(pnmce->iWhy, pnmce->fChanged != 0, pnmce->szText, m_wndListViewServer))
		OnServerAddressTextReturn(pnmce->szText);
	return 0;
}

LRESULT CMainWindow::OnServerAddressSelChange(WPARAM wParam, LPARAM lParam)
{
	if (!::SendMessage(m_wndServerAddress, CB_GETDROPPEDSTATE, 0, 0))
	{
		//OnServerAddressCloseUp(0, 0);
		PCIDLIST_ABSOLUTE lpidl = m_wndServerAddress.GetSelectedFolder();
		if (lpidl && !IsEqualIDList(lpidl, m_wndListViewServer.m_lpidlAbsoluteMe))
			UpdateServerFolderAbsolute(lpidl);
		else
			m_wndServerAddress.RestoreTextBox();
	}
	else
		m_bServerAddressSelChanged = true;
	return 0;
}

LRESULT CMainWindow::OnServerAddressCloseUp(WPARAM wParam, LPARAM lParam)
{
	if (m_bServerAddressSelChanged)
	{
		m_bServerAddressSelChanged = false;
		////LPCWSTR lpw = m_wndServerAddress.GetSelectedFolder();
		////if (lpw && m_wndServerAddress.m_strDirectory.Compare(lpw))
		////	UpdateServerFolderAbsolute(lpw);
		//PCIDLIST_ABSOLUTE lpidl = m_wndServerAddress.GetSelectedFolder();
		//if (lpidl && !IsEqualIDList(lpidl, m_wndListViewServer.m_lpidlAbsoluteMe))
		//	UpdateServerFolderAbsolute(lpidl);
		//else
			m_wndServerAddress.RestoreTextBox();
	}
	return 0;
}

//LRESULT CMainWindow::OnServerListViewDblClick(WPARAM wParam, LPARAM lParam)
//{
//	LVHITTESTINFO lvh;
//	int i;
//	::GetCursorPos(&lvh.pt);
//	::ScreenToClient(m_wndListViewServer.m_hWnd, &lvh.pt);
//	i = (int) (::SendMessage(m_wndListViewServer.m_hWnd, LVM_HITTEST, 0, (LPARAM) &lvh));
//	if (i != -1)
//	{
//		CFTPFileItem* pItem;
//		if (m_wndListViewServer.GetFileItem(i, &pItem))
//			DoOpen(pItem);
//	}
//	return 0;
//}
//
//LRESULT CMainWindow::OnServerListViewReturn(WPARAM wParam, LPARAM lParam)
//{
//	int i;
//
//	i = (int) (::SendMessage(m_wndListViewServer.m_hWnd, LVM_GETNEXTITEM, (WPARAM) -1, (LPARAM) (LVNI_SELECTED)));
//	if (i != -1)
//	{
//		if ((int) (::SendMessage(m_wndListViewServer.m_hWnd, LVM_GETNEXTITEM, (WPARAM) (i), (LPARAM) (LVNI_SELECTED))) == -1)
//		{
//			CFTPFileItem* pItem;
//			if (m_wndListViewServer.GetFileItem(i, &pItem) && pItem->IsDirectory())
//				UpdateServerFolder(pItem->strFileName);
//		}
//	}
//	return 0;
//}
//
//LRESULT CMainWindow::OnServerListViewBeginLabelEdit(WPARAM wParam, LPARAM lParam)
//{
//	return 0;
//}
//
//LRESULT CMainWindow::OnServerListViewEndLabelEdit(WPARAM wParam, LPARAM lParam)
//{
//	CMyStringW strFileName;
//	bool bSucceeded = false;
//	int iItem;
//	if (((LPNMHDR) lParam)->code == LVN_ENDLABELEDITW)
//	{
//		LPNMLVDISPINFOW lpNMLV = (LPNMLVDISPINFOW) lParam;
//		if (!lpNMLV->item.pszText)
//		{
//			if (m_nInCreatingLabelEditMode)
//				m_wndListViewServer.EndCreateLabelEdit(NULL);
//		}
//		else
//		{
//			iItem = lpNMLV->item.iItem;
//			strFileName = lpNMLV->item.pszText;
//			bSucceeded = true;
//		}
//	}
//	else
//	{
//		LPNMLVDISPINFOA lpNMLV = (LPNMLVDISPINFOA) lParam;
//		if (!lpNMLV->item.pszText)
//		{
//			if (m_nInCreatingLabelEditMode)
//				m_wndListViewServer.EndCreateLabelEdit(NULL);
//		}
//		else
//		{
//			iItem = lpNMLV->item.iItem;
//			strFileName = lpNMLV->item.pszText;
//			bSucceeded = true;
//		}
//	}
//
//	if (bSucceeded)
//	{
//		switch (m_nInCreatingLabelEditMode)
//		{
//			case 1:
//			{
//				CWaitMakeDirData* pData = CreateRemoteDirectory(strFileName);
//				if (!pData)
//				{
//					m_wndListViewServer.EndCreateLabelEdit(NULL);
//					bSucceeded = false;
//				}
//				else
//				{
//					if (m_bSFTPMode)
//						m_uSFTPCreateEditLabelMsgID = pData->uMsgID;
//					else
//						pData->pvData = (void*) true;
//				}
//			}
//			break;
//			case 2:
//			{
//				if (!m_bSFTPMode)
//				{
//					m_wndListViewServer.EndCreateLabelEdit(NULL);
//					bSucceeded = false;
//				}
//				else
//				{
//					CFTPFileItem* pItem;
//					m_wndListViewServer.GetFileItem(iItem, &pItem);
//					CWaitMakeDirData* pData = CreateShortcut(pItem, strFileName);
//					if (!pData)
//					{
//						m_wndListViewServer.EndCreateLabelEdit(NULL);
//						bSucceeded = false;
//					}
//					else
//					{
//						if (m_bSFTPMode)
//							m_uSFTPCreateEditLabelMsgID = pData->uMsgID;
//						else
//							pData->pvData = (void*) true;
//					}
//				}
//			}
//			break;
//			default:
//			{
//				CFTPFileItem* pItem;
//				m_wndListViewServer.GetFileItem(iItem, &pItem);
//				DoRename(pItem, strFileName);
//			}
//			break;
//		}
//	}
//
//	m_nInCreatingLabelEditMode = 0;
//	return bSucceeded ? TRUE : FALSE;
//}
//
//LRESULT CMainWindow::OnSocketMessage(WPARAM wParam, LPARAM lParam)
//{
//	SOCKET s;
//	CFTPSocket* pSocket;
//	CFTPPassiveMessage* pPassive;
//	int iIndex;
//
//	s = (SOCKET) wParam;
//	if (m_bSFTPMode)
//	{
//		if (!m_pClient || m_pClient->m_socket.operator SOCKET() != s)
//			return 0;
//		m_pClient->m_socket.EnableAsyncSelect(false, true);
//		if (!m_pClient->m_socket.CanReceive())
//		{
//			if (m_pClient->m_socket.IsRemoteClosed())
//			{
//				MessageBeep(MB_ICONHAND);
//				DoCloseConnection(true);
//			}
//			else if (m_pChannel && m_pChannel->HasQueue())
//			{
//				if (!m_pChannel->FlushAllMessages(this))
//				{
//#ifdef _DEBUG
//					OutputDebugString(_T("\tFlushAllMessages() returned false\n"));
//#endif
//				}
//			}
//			else
//			{
//#ifdef _DEBUG
//				OutputDebugString(_T("OnSocketMessage was called but no data is available\n"));
//#endif
//				m_pClient->m_socket.EnableAsyncSelect(true, true);
//			}
//			return 0;
//		}
//		while (true)
//		{
//			OnSFTPSocketReceive();
//			if (!m_pClient || !m_pClient->m_socket.HasReceivedData())
//				break;
//		}
//		if (m_pClient)
//			m_pClient->m_socket.EnableAsyncSelect(true, true);
//	}
//	else
//	{
//		if (!m_pConnection)
//			return 0;
//		if (m_pConnection->operator SOCKET() == s)
//			pSocket = m_pConnection;
//		else
//		{
//			pSocket = NULL;
//			for (iIndex = 0; iIndex < m_aDataSockets.GetCount(); iIndex++)
//			{
//				pSocket = m_aDataSockets.GetItem(iIndex);
//				if (pSocket->operator SOCKET() == s)
//					break;
//				pSocket = NULL;
//			}
//		}
//		if (!pSocket)
//			return 0;
//		pSocket->EnableAsyncSelect(false);
//		if (WSAGETSELECTEVENT(lParam) == FD_READ && !pSocket->CanReceive(0))
//		{
//			pSocket->EnableAsyncSelect(true);
//#ifdef _DEBUG
//			//::OutputDebugString(_T("WARNING: FD_READ has been received but no data is available\n"));
//#endif
//			return 0;
//		}
//		pPassive = m_listReceivePassives.GetItem(pSocket);
//		if (pPassive != NULL)
//		{
//			switch (WSAGETSELECTEVENT(lParam))
//			{
//				case FD_READ:
//#ifdef _DEBUG
//					//::OutputDebugString(_T("Passive FD_READ\n"));
//#endif
//					if (pPassive->OnReceive(pSocket))
//						break;
//					break;
//				case FD_CLOSE:
//				{
//					if (WSAGETSELECTEVENT(lParam) == FD_CLOSE)
//					{
//#ifdef _DEBUG
//						//::OutputDebugString(_T("Passive FD_CLOSE\n"));
//#endif
//						while (pPassive->OnReceive(pSocket));
//					}
//					_EndFTPPassive(pPassive, pSocket);
//				}
//				return 0;
//				case FD_WRITE:
//					if (!pPassive->ReadyToWrite(pSocket))
//					{
//						pSocket->EnableAsyncSelect(false);
//						pSocket->Close();
//						_EndFTPPassive(pPassive, pSocket);
//						return 0;
//					}
//					break;
//			}
//		}
//		else
//		{
//			switch (WSAGETSELECTEVENT(lParam))
//			{
//				case FD_READ:
//					if (pSocket == m_pConnection)
//					{
//						OnSocketReceive();
//					}
//					else
//					{
//#ifdef _DEBUG
//						//OutputDebugString(_T("Unknown socket message has received\n"));
//#endif
//					}
//					break;
//				case FD_CLOSE:
//					if (pSocket == m_pConnection)
//						DoCloseConnection();
//					else
//					{
//						bool b = false;
//						for (int i = 0; i < m_aWait150Messages.GetCount(); i++)
//						{
//							if (m_aWait150Messages.GetItem(i)->pPassive == pSocket)
//							{
//								b = true;
//								break;
//							}
//						}
//						if (b)
//							break;
//						m_aDataSockets.RemoveItem(iIndex);
//						delete pSocket;
//#ifdef _DEBUG
//						//OutputDebugString(_T("FD_CLOSE for unknown socket\n"));
//#endif
//					}
//					return 0;
//			}
//		}
//		pSocket->EnableAsyncSelect(true);
//	}
//	return 0;
//}
//
//LRESULT CMainWindow::OnTimer(WPARAM wParam, LPARAM lParam)
//{
//	switch ((int) (wParam))
//	{
//		case TIMERID_KEEP_CONNECTION:
//			// send 'ignore'/no-op message to keep connection
//			if (m_bSFTPMode)
//				m_pClient->m_socket.SendPacket(SSH2_MSG_IGNORE, NULL, 0);
//			else
//				m_pConnection->SendCommand(L"NOOP");
//			return 0;
//		case TIMERID_TRANSFER_CHECK:
//		{
//			register int c = theApp.m_aObjectTransferring.GetCount();
//			if (!c)
//				::KillTimer(m_hWnd, TIMERID_TRANSFER_CHECK);
//			else
//			{
//				while (c--)
//					((CFTPDataObject*) theApp.m_aObjectTransferring.GetItem(c))->UpdateToTransferDialog();
//			}
//		}
//		return 0;
//	}
//	return Default(wParam, lParam);
//}
//
//LRESULT CMainWindow::OnSendQueue(WPARAM wParam, LPARAM lParam)
//{
//	//LPWSTR lpw = m_aSendFileQueue.GetItem(0);
//	//CMyStringW strPath(lpw);
//	//m_aSendFileQueue.RemoveItem(0);
//	//while (*lpw++);
//	//DoSendFile(strPath, lpw);
//	//if (m_aSendFileQueue.GetCount() > 0)
//	//	::PostMessage(m_hWnd, MY_WM_SENDQUEUE, 0, 0);
//	//else
//	//	m_bQueueMsgSent = false;
//	//return 0;
//	CSendQueueData* pQueue = (CSendQueueData*) lParam;
//
//	if (pQueue->findData.cFileName[0] != L'.' || (pQueue->findData.cFileName[1] &&
//		(pQueue->findData.cFileName[1] != L'.' || pQueue->findData.cFileName[2])))
//	{
//		LPWSTR lpw = ::MyGetFullPath2W(pQueue->strParentPath, pQueue->findData.cFileName);
//
//		if (pQueue->findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
//			SendFolder(lpw, false, pQueue->strRemotePath);
//		else
//			SendFile(lpw, pQueue->strRemotePath);
//		free(lpw);
//	}
//
//	if (::MyFindNextFileW(pQueue->hFind, &pQueue->findData))
//		::PostMessage(m_hWnd, MY_WM_SENDQUEUE, 0, lParam);
//	else
//	{
//		::FindClose(pQueue->hFind);
//		delete pQueue;
//	}
//	return 0;
//}

LRESULT CMainWindow::OnSplitterTrack(WPARAM wParam, LPARAM lParam)
{
	LPSPTRACKNOTIFY lpstn = (LPSPTRACKNOTIFY) lParam;
	theApp.m_nSplitterPos = lpstn->nPos;
	OnSize(0, 0);
	return 0;
}

LRESULT CMainWindow::OnSplitterTracking(WPARAM wParam, LPARAM lParam)
{
	LPSPTRACKNOTIFY lpstn = (LPSPTRACKNOTIFY) lParam;
	if (lpstn->nPos < 60 + m_nAddrButtonsWidth)
	{
		lpstn->nPos = 60 + m_nAddrButtonsWidth;
		return 1;
	}
	else if (lpstn->nPos > lpstn->rcParent.right - (60 + m_nAddrButtonsWidth))
	{
		lpstn->nPos = lpstn->rcParent.right - (60 + m_nAddrButtonsWidth);
		return 1;
	}
	return 0;
}

LRESULT CMainWindow::OnChangeNotify(WPARAM wParam, LPARAM lParam)
{
	if (!(lParam & (SHCNE_MKDIR | SHCNE_RENAMEFOLDER | SHCNE_RMDIR | SHCNE_DRIVEADD | SHCNE_DRIVEADDGUI | SHCNE_DRIVEREMOVED)))
		return 0;
	m_wndAddress.NotifyChange(wParam, lParam);
	m_wndServerAddress.NotifyChange(wParam, lParam);
	if (lParam == SHCNE_RMDIR || lParam == SHCNE_DRIVEREMOVED)
	{
		PCIDLIST_ABSOLUTE pidlTarget = *((const PCIDLIST_ABSOLUTE*) wParam);
		if (::IsMatchParentIDList(pidlTarget, m_wndListViewLocal.m_lpidlAbsoluteMe))
		{
			PIDLIST_ABSOLUTE pidl = ::RemoveOneChild(pidlTarget);
			UpdateCurrentFolderAbsolute(pidl);
			::CoTaskMemFree(pidl);
		}
		if (::IsMatchParentIDList(pidlTarget, m_wndListViewServer.m_lpidlAbsoluteMe))
		{
			PIDLIST_ABSOLUTE pidl = ::RemoveOneChild(pidlTarget);
			UpdateServerFolderAbsolute(pidl);
			::CoTaskMemFree(pidl);
		}
	}
	return 0;
}
