/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 EasySFTP.cpp - implementations of CMainApplication
 */

#include "stdafx.h"
#include "EasySFTP.h"

#include "INIFile.h"
#include "MainWnd.h"
//#include "DragData.h"
#include "ExplrDDE.h"

#include "IDList.h"
#include "FileStrm.h"

CMainApplication theApp;

// <host-name>\0<directory>\0[<file>\0]...\0 (all strings are unicode-string)
const TCHAR CMainApplication::s_szCFFTPData[] = _T("EasySFTPFormatData");
//// 1-byte boolean value
//const TCHAR CMainApplication::s_szCFFTPRenameFlag[] = _T("EasySFTPFormatRenameFlag");
const WCHAR CMainApplication::s_szMainWndClass[] = L"EasySFTPMainWnd";
const WCHAR CMainApplication::s_szViewParentWndClass[] = L"EasySFTPViewParentWnd";
const WCHAR CMainApplication::s_szLocalViewStateFile[] = L"LVStat.dat";
const WCHAR CMainApplication::s_szServerViewStateFile[] = L"SVStat.dat";

// {AD29C042-B9E3-4638-9DF6-D7DA5B8D0199}
EXTERN_C const IID IID_IEasySFTPInternal =
{ 0xAD29C042, 0xB9E3, 0x4638, { 0x9D, 0xF6, 0xD7, 0xDA, 0x5B, 0x8D, 0x01, 0x99 } };
// {AD29C042-B9E3-463a-9DF6-D7DA5B8D0199}
EXTERN_C const IID IID_IEasySFTPListener =
{ 0xAD29C042, 0xB9E3, 0x463a, { 0x9D, 0xF6, 0xD7, 0xDA, 0x5B, 0x8D, 0x01, 0x99 } };
// {AD29C042-B9E3-463e-9DF6-D7DA5B8D0199}
EXTERN_C const IID IID_IEasySFTPOldDirectory =
{ 0xAD29C042, 0xB9E3, 0x463e, { 0x9D, 0xF6, 0xD7, 0xDA, 0x5B, 0x8D, 0x01, 0x99 } };
// {AD29C042-B9E3-463c-9DF6-D7DA5B8D0199}
EXTERN_C const IID IID_IEasySFTPOldRoot =
{ 0xAD29C042, 0xB9E3, 0x463c, { 0x9D, 0xF6, 0xD7, 0xDA, 0x5B, 0x8D, 0x01, 0x99 } };
// {AD29C042-B9E3-4636-9DF6-D7DA5B8D0199}
EXTERN_C const IID IID_IEasySFTPOldRoot2 =
{ 0xAD29C042, 0xB9E3, 0x4636, { 0x9D, 0xF6, 0xD7, 0xDA, 0x5B, 0x8D, 0x01, 0x99 } };
// {AD29C042-B9E3-462c-9DF6-D7DA5B8D0199}
EXTERN_C const CLSID CLSID_EasySFTPOld =
{ 0xAD29C042, 0xB9E3, 0x462c, { 0x9D, 0xF6, 0xD7, 0xDA, 0x5B, 0x8D, 0x01, 0x99 } };

#ifdef _DEBUG
static bool s_bRestartApp = false;
#endif

EXTERN_C int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine, int nCmdShow)
{
	int ret;
#ifdef _DEBUG
	while (true)
	{
#endif
		ret = MyWinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
#ifdef _DEBUG
		if (!s_bRestartApp)
			break;
		theApp.~CMainApplication();
		s_bRestartApp = false;
		CallConstructor(&theApp);
	}
#endif
	return ret;
}

////////////////////////////////////////////////////////////////////////////////

static const WCHAR s_wszDefTextFileType[] = L"*.txt;*.htm;*.html;*.css;*.xml;*.xsl;*.xslt;*.cgi;*.pl;*.php;*.sh;*.jsp;*.asp;*.ini";

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

typedef BOOL(WINAPI* FnFileIconInit)(BOOL fRestoreCache);
static FnFileIconInit s_pfnFileIconInit = reinterpret_cast<FnFileIconInit>(static_cast<INT_PTR>(-1));

EXTERN_C BOOL MyFileIconInit(BOOL fRestoreCache)
{
	if (s_pfnFileIconInit == reinterpret_cast<FnFileIconInit>(static_cast<INT_PTR>(-1)))
	{
		s_pfnFileIconInit = NULL;
		auto hShell32 = ::GetModuleHandle(_T("shell32.dll"));
		if (hShell32)
		{
			s_pfnFileIconInit = reinterpret_cast<FnFileIconInit>(::GetProcAddress(hShell32, MAKEINTRESOURCEA(660)));
		}
	}
	if (!s_pfnFileIconInit)
		return TRUE;
	return s_pfnFileIconInit(fRestoreCache);
}

////////////////////////////////////////////////////////////////////////////////

class CEmptyStream : public CUnknownImplT<IStream>
{
public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject);

	// ISequentialStream Interface
public:
	STDMETHOD(Read)(void* pv, ULONG cb, ULONG* pcbRead)
	{
		if (!pv || !pcbRead)
			return E_POINTER;
		*pcbRead = 0;
		return S_FALSE;
	}
	STDMETHOD(Write)(void const* pv, ULONG cb, ULONG* pcbWritten)
	{
		if (!pv || !pcbWritten)
			return E_POINTER;
		*pcbWritten = cb;
		return S_OK;
	}

	// IStream Interface
public:
	STDMETHOD(SetSize)(ULARGE_INTEGER libNewSize)
		{ return E_NOTIMPL; }
	STDMETHOD(CopyTo)(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten)
		{ return E_NOTIMPL; }
	STDMETHOD(Commit)(DWORD grfCommitFlags)
		{ return E_NOTIMPL; }
	STDMETHOD(Revert)()
		{ return E_NOTIMPL; }
	STDMETHOD(LockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
		{ return E_NOTIMPL; }
	STDMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
		{ return E_NOTIMPL; }
	STDMETHOD(Clone)(IStream** ppstm)
	{
		if (!ppstm)
			return E_POINTER;
		*ppstm = new CEmptyStream();
		return *ppstm != NULL ? S_OK : E_OUTOFMEMORY;
	}
	STDMETHOD(Seek)(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer)
		{ return E_NOTIMPL; }
	STDMETHOD(Stat)(STATSTG* pStatstg, DWORD grfStatFlag)
		{ return E_NOTIMPL; }
};

STDMETHODIMP CEmptyStream::QueryInterface(REFIID riid, void** ppvObject)
{ 
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IStream) ||
		IsEqualIID(riid, IID_ISequentialStream))
	{
		*ppvObject = static_cast<IStream*>(this);
		AddRef();
		return S_OK;
	}
	else
		return E_NOINTERFACE; 
}

////////////////////////////////////////////////////////////////////////////////

CMainApplication::CMainApplication()
{
	m_hImageListFileIcon = NULL;
	m_hImageListToolBar = NULL;
	m_hImageListToolBarL = NULL;
	m_hImageListAddrButtons = NULL;
	m_hImageListAddrButtonsL = NULL;
	m_bUseOFNUnicode = true;
	memset(&m_ofnW, 0, sizeof(m_ofnW));
	m_ofnW.lStructSize = OPENFILENAME_SIZE_VERSION_400W;
	m_bUseBIUnicode = true;
	memset(&m_biW, 0, sizeof(m_biW));
	m_bUsePlacement = false;
	m_pidlEasySFTP = NULL;
	m_pEasySFTPRoot = NULL;
	//m_pStreamViewStateLocal = NULL;
	//m_pStreamViewStateServer = NULL;
	m_bEmulatingRegistry = false;
	m_bIsRegisteredAsUserClass = false;

	m_bExitWithRegister = false;
	m_bIsRegisterForSystem = false;
	m_bUnregisterOperation = false;
	m_bNoRestart = false;
}

CMainApplication::~CMainApplication()
{
}

bool CMainApplication::InitInstance()
{
	if (!InitRegistryHook())
	{
		if (::MyMessageBoxW(NULL, MAKEINTRESOURCEW(IDS_FAILED_TO_INIT_EASYSFTP_IN_REGHOOK), NULL,
			MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
			m_bExitWithRegister = true;
		return false;
	}

#ifndef _WIN64
	if (m_bIsWin9x && m_bNeedEmulationMode)
	{
		if (::MyMessageBoxW(NULL, MAKEINTRESOURCEW(IDS_CANNOT_RUN_IN_EMULATION_MODE), NULL,
			MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
			m_bExitWithRegister = true;
		return false;
	}
#endif

	if (!InitSystemLibraries())
	{
		::MyMessageBoxW(NULL, MAKEINTRESOURCEW(IDS_FAILED_TO_LOAD_SYSLIBS), NULL, MB_ICONEXCLAMATION);
		return false;
	}

	int nParam = ParseCommandLine();
	if (nParam & (paramRegister | paramUnregister))
	{
		m_bExitWithRegister = true;
		m_bUnregisterOperation = ((nParam & paramUnregister) != 0);
		m_bNoRestart = true;
		return false;
	}

	if ((m_bEmulatingRegistry && !InitRegHook()) || !InitEasySFTP())
	{
		if (m_bEmulatingRegistry)
		{
			if (::MyMessageBoxW(NULL, MAKEINTRESOURCEW(IDS_FAILED_TO_INIT_EASYSFTP_IN_REGHOOK), NULL,
				MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
				m_bExitWithRegister = true;
		}
		else
		{
			::MyMessageBoxW(NULL, MAKEINTRESOURCEW(IDS_FAILED_TO_INIT_SHELLDLL), NULL, MB_ICONEXCLAMATION);
		}
		return false;
	}

	if (!InitGraphics() || !InitWindowClasses() || !InitAppData())
	{
		::MyMessageBoxW(NULL, MAKEINTRESOURCEW(IDS_FAILED_TO_INIT_APP), NULL, MB_ICONEXCLAMATION);
		return false;
	}

	LoadINISettings();

	CMainWindow* pWnd = new CMainWindow();
	if (!pWnd->CreateEx())
		return false;

	m_pMainWnd = pWnd;
	//m_pEasySFTPRoot->SetListener(pWnd);

	if (m_bUsePlacement)
		::SetWindowPlacement(pWnd->m_hWnd, &m_wpFrame);
	if (!m_bUsePlacement || m_nCmdShow != SW_SHOWNORMAL || m_wpFrame.showCmd == SW_HIDE)
		::ShowWindow(pWnd->m_hWnd, m_nCmdShow);
	::UpdateWindow(pWnd->m_hWnd);

	return true;
}

int CMainApplication::ExitInstance()
{
	SaveINISettings();

	if (m_pUnkThreadRef)
		m_pUnkThreadRef->Release();
	if (m_pEasySFTPRoot)
		m_pEasySFTPRoot->Release();
	if (m_pidlEasySFTP)
		::CoTaskMemFree(m_pidlEasySFTP);
	if (m_hImageListAddrButtonsL)
		::ImageList_Destroy(m_hImageListAddrButtonsL);
	if (m_hImageListAddrButtons)
		::ImageList_Destroy(m_hImageListAddrButtons);
	if (m_hImageListToolBarL)
		::ImageList_Destroy(m_hImageListToolBarL);
	if (m_hImageListToolBar)
		::ImageList_Destroy(m_hImageListToolBar);
	if (m_hImageListFileIcon)
		::ImageList_Destroy(m_hImageListFileIcon);
	if (m_hMenuPopup)
		::DestroyMenu(m_hMenuPopup);
	//if (m_pStreamViewStateLocal)
	//	m_pStreamViewStateLocal->Release();
	//if (m_pStreamViewStateServer)
	//	m_pStreamViewStateServer->Release();
	for (int i = 0; i < m_aLocalStreams.GetCount(); i++)
	{
		CMRUStreamData* p = m_aLocalStreams.GetItem(i);
		if (p->nSize)
			free(p->pbData);
		::CoTaskMemFree(p->pidl);
		free(p);
	}
	m_aLocalStreams.RemoveAll();
	for (int i = 0; i < m_aServerStreams.GetCount(); i++)
	{
		CMRUStreamData* p = m_aServerStreams.GetItem(i);
		if (p->nSize)
			free(p->pbData);
		::CoTaskMemFree(p->pidl);
		free(p);
	}
	m_aServerStreams.RemoveAll();

	{
		size_t dw = m_strTempPath.GetLength();
		CMyStringW str;
		if (dw != 3 || ((LPCWSTR) m_strTempPath)[1] != L':')
			str.SetString(m_strTempPath, dw - 1);
		::MyRemoveDirectoryRecursiveW(str);
	}

	if (m_bEmulatingRegistry)
		TermRegHook();
	//::WSACleanup();
	::OleUninitialize();

	if (m_pMainWnd)
	{
		OutputDebugString(_T("EasySFTP: warning: the main window instance is not released!\n"));
		delete m_pMainWnd;
	}

	if (m_bExitWithRegister)
		DoRegister();

	return CMyApplication::ExitInstance();
}

bool CMainApplication::OnIdle(long lCount)
{
	register CMainWindow* pWnd = (CMainWindow*) m_pMainWnd;
	return pWnd ? pWnd->OnIdle(lCount) : false;
}

bool CMainApplication::InitRegistryHook()
{
	m_bNeedEmulationMode = false;
	{
		HKEY hKey, hKey2;
		if (::RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("CLSID"), 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
		{
			CMyStringW str;
			str.Format(L"{%08lX-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}",
				CLSID_EasySFTPOld.Data1, (int)CLSID_EasySFTPOld.Data2, (int)CLSID_EasySFTPOld.Data3,
				(int)CLSID_EasySFTPOld.Data4[0], (int)CLSID_EasySFTPOld.Data4[1], (int)CLSID_EasySFTPOld.Data4[2],
				(int)CLSID_EasySFTPOld.Data4[3], (int)CLSID_EasySFTPOld.Data4[4], (int)CLSID_EasySFTPOld.Data4[5],
				(int)CLSID_EasySFTPOld.Data4[6], (int)CLSID_EasySFTPOld.Data4[7]);
			bool bHasCLSID = (::RegOpenKeyEx(hKey, str, 0, KEY_QUERY_VALUE, &hKey2) == ERROR_SUCCESS);
			if (bHasCLSID)
				::RegCloseKey(hKey2);
			::RegCloseKey(hKey);
			m_bNeedEmulationMode = !bHasCLSID;
		}
	}

	if (m_bNeedEmulationMode)
	{
		if (!InitRegHook())
		{
			return false;
		}
		m_bEmulatingRegistry = true;
	}
	return true;
}

bool CMainApplication::InitSystemLibraries()
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif
	//WSADATA wsa;
	if (FAILED(::OleInitialize(NULL)))
		return false;
	//if (::WSAStartup(MAKEWORD(2, 0), &wsa))
	//	return false;

	//SSL_library_init();
	//ERR_load_CRYPTO_strings();

	{
		INITCOMMONCONTROLSEX icex;
		icex.dwSize = sizeof(icex);
		icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES | ICC_USEREX_CLASSES | ICC_LINK_CLASS;
		if (!::InitCommonControlsEx(&icex))
			return false;
	}

	{
		HINSTANCE hInstShell32 = ::GetModuleHandle(_T("shell32.dll"));
		if (hInstShell32)
		{
			BOOL (STDMETHODCALLTYPE* pfnFileIconInit)(_In_ BOOL fRestoreCache);
			pfnFileIconInit = (BOOL (STDMETHODCALLTYPE*)(_In_ BOOL fRestoreCache)) ::GetProcAddress(hInstShell32, MAKEINTRESOURCEA(660));
			if (pfnFileIconInit)
			{
				pfnFileIconInit(FALSE);
			}
		}
	}

	{
		HINSTANCE hInstShlwapi = ::GetModuleHandle(_T("shlwapi.dll"));
		if (hInstShlwapi)
		{
			HRESULT hr;
			HRESULT (STDMETHODCALLTYPE* pfnSHCreateThreadRef)(__inout LONG* pcRef, __deref_out IUnknown** ppunk);
			HRESULT (STDMETHODCALLTYPE* pfnSHSetThreadRef)(__in IUnknown* punk);
			pfnSHCreateThreadRef = (HRESULT (STDMETHODCALLTYPE*)(__inout LONG*, __deref_out IUnknown**))
				::GetProcAddress(hInstShlwapi, "SHCreateThreadRef");
			pfnSHSetThreadRef = (HRESULT (STDMETHODCALLTYPE*)(__in IUnknown*))
				::GetProcAddress(hInstShlwapi, "SHSetThreadRef");
			if (pfnSHCreateThreadRef)
			{
				m_uRefThread = 0;
				hr = pfnSHCreateThreadRef((LONG*) &m_uRefThread, &m_pUnkThreadRef);
			}
			else
				hr = E_NOTIMPL;
			if (SUCCEEDED(hr))
			{
				hr = pfnSHSetThreadRef(m_pUnkThreadRef);
				if (FAILED(hr))
				{
					m_pUnkThreadRef->Release();
					m_pUnkThreadRef = NULL;
				}
			}
			else
				m_pUnkThreadRef = NULL;
		}
		else
			m_pUnkThreadRef = NULL;
	}

	m_nCFShellIDList = ::RegisterClipboardFormat(CFSTR_SHELLIDLIST);

	{
		OSVERSIONINFO vi;
		vi.dwOSVersionInfoSize = sizeof(vi);
		if (!::GetVersionExA(&vi))
		{
			m_bUseOFNUnicode = false;
#ifndef _WIN64
			m_bIsWin9x = true;
#endif
		}
		else
		{
			m_bUseOFNUnicode = (vi.dwPlatformId == VER_PLATFORM_WIN32_NT);
#ifndef _WIN64
			m_bIsWin9x = !m_bUseOFNUnicode;
#endif
		}
	}

	return true;
}

int CMainApplication::ParseCommandLine()
{
	// "<program-name>" <SP> arguments
	LPWSTR lpwCommandLine = GetCommandLineW();
	bool bInQuote = false;
	int nCurrentStatus = 0;
	// first, skip program name
	while (*lpwCommandLine && (bInQuote || (*lpwCommandLine != L' ' && *lpwCommandLine != L'\t')))
	{
		if (*lpwCommandLine == L'\"')
			bInQuote = !bInQuote;
		lpwCommandLine++;
	}

	LPWSTR lpwBefore, lpwBuffer, lpwPos;
	DWORD dwLen, dwMaxLen;
	CMyStringW strBuffer;
	dwMaxLen = MAX_PATH;
	while (true)
	{
		while (*lpwCommandLine == L' ' || *lpwCommandLine == L'\t')
			lpwCommandLine++;
		if (!*lpwCommandLine)
			break;
		lpwBefore = lpwCommandLine;
		lpwPos = lpwBuffer = strBuffer.GetBuffer(dwMaxLen);
		dwLen = 0;
		bInQuote = false;
		while (true)
		{
			WCHAR c;
			c = *lpwCommandLine;
			if (!c || (!bInQuote && (c == L' ' || c == L'\t')))
				break;
			UINT uBackSlashCount = 0;
			while (c == L'\\')
			{
				uBackSlashCount++;
				lpwCommandLine++;
				c = *lpwCommandLine;
			}
			if (c == L'\"')
			{
				if (uBackSlashCount % 2 == 0)
				{
					bInQuote = !bInQuote;
					c = 0;
				}
				uBackSlashCount /= 2;
			}
			while (uBackSlashCount--)
			{
				*lpwPos++ = L'\\';
				dwLen++;
				if (dwLen == dwMaxLen)
				{
					lpwBuffer = strBuffer.GetBuffer(dwMaxLen += MAX_PATH);
					lpwPos = lpwBuffer + dwLen;
				}
			}
			if (c)
			{
				*lpwPos++ = c;
				dwLen++;
				if (dwLen == dwMaxLen)
				{
					lpwBuffer = strBuffer.GetBuffer(dwMaxLen += MAX_PATH);
					lpwPos = lpwBuffer + dwLen;
				}
			}
			lpwCommandLine++;
		}
		strBuffer.ReleaseBuffer(dwLen);

		if (!strBuffer.IsEmpty())
			CheckCommandParameter(strBuffer, nCurrentStatus);
	}

	return nCurrentStatus;
}

void CMainApplication::CheckCommandParameter(LPCWSTR lpszParam, int& nCurrentStatus)
{
	if (*lpszParam == L'-' || *lpszParam == L'/')
	{
		lpszParam++;
		if (_wcsicmp(lpszParam, L"register") == 0 ||
			_wcsicmp(lpszParam, L"regserver") == 0)
			nCurrentStatus |= paramRegister;
		else if (_wcsicmp(lpszParam, L"unregister") == 0 ||
			_wcsicmp(lpszParam, L"unregserver") == 0)
			nCurrentStatus |= paramUnregister;
		else if (_wcsicmp(lpszParam, L"local") == 0)
		{
			nCurrentStatus &= ~paramNextIsServerPath;
			nCurrentStatus |= paramNextIsLocalPath;
		}
		else if (_wcsicmp(lpszParam, L"server") == 0)
		{
			nCurrentStatus &= ~paramNextIsLocalPath;
			nCurrentStatus |= paramNextIsServerPath;
		}
	}
	else
	{
		if (nCurrentStatus & paramNextIsLocalPath)
		{
			m_strFirstLocalPath = lpszParam;
			nCurrentStatus ^= paramNextIsLocalPath;
		}
		else if (nCurrentStatus & paramNextIsServerPath)
		{
			m_strFirstServerPath = lpszParam;
			nCurrentStatus ^= paramNextIsServerPath;
		}
	}
}

bool CMainApplication::InitEasySFTP()
{
	HRESULT hr;
	IShellFolder* pDesktop, * pFolder;
	if (FAILED(::SHGetDesktopFolder(&pDesktop)))
		return false;
	hr = pDesktop->ParseDisplayName(NULL, NULL, (LPWSTR) L"::{AD29C042-B9E3-4740-9DF6-D7DA5B8D0199}",
		NULL, (PIDLIST_RELATIVE*) &m_pidlEasySFTP, NULL);
	if (FAILED(hr))
		m_pidlEasySFTP = NULL;

	if (!m_pidlEasySFTP)
	{
		pDesktop->Release();
		::MessageBeep(MB_ICONHAND);
		return false;
	}
	hr = pDesktop->BindToObject(m_pidlEasySFTP, NULL, IID_IShellFolder, (void**) &pFolder);
	pDesktop->Release();
	if (FAILED(hr))
		return false;
	hr = pFolder->QueryInterface(IID_IEasySFTPOldRoot, (void**) &m_pEasySFTPRoot);
	pFolder->Release();
	if (FAILED(hr))
		return false;

	IEasySFTPInternal* pInternal;
	hr = m_pEasySFTPRoot->QueryInterface(IID_IEasySFTPInternal, (void**) &pInternal);
	if (SUCCEEDED(hr))
	{
		pInternal->SetEmulateRegMode(m_bEmulatingRegistry);
		pInternal->Release();
	}

	// check where to register
	{
		CMyStringW str;
		::MyStringFromGUIDW(CLSID_EasySFTPOld, str);
		str.InsertString(L"Software\\Classes\\CLSID\\", 0);
		HKEY hKey;
		if (::RegOpenKeyEx(HKEY_CURRENT_USER, str, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
		{
			m_bIsRegisteredAsUserClass = true;
			::RegCloseKey(hKey);
		}
		else
		{
			m_bIsRegisteredAsUserClass = false;
		}
	}

	return true;
}

bool CMainApplication::InitGraphics()
{
	MyFileIconInit(TRUE);

	m_hImageListFileIcon = ::ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);
	if (!m_hImageListFileIcon)
		return false;

	//m_hImageListToolBar = ::ImageList_LoadImage(m_hInstance, MAKEINTRESOURCE(IDB_TOOLBAR),
	//	16, 0, CLR_NONE, IMAGE_BITMAP, LR_LOADTRANSPARENT);
	//if (!m_hImageListToolBar)
	//	return false;
	m_hImageListToolBar = ::ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);
	if (!m_hImageListToolBar)
		return false;
	::ImageList_Add(m_hImageListToolBar, ::LoadBitmap(m_hInstance, MAKEINTRESOURCE(IDB_TOOLBAR)), NULL);
	m_hImageListToolBarL = ::ImageList_Create(32, 32, ILC_COLOR32 | ILC_MASK, 0, 0);
	if (!m_hImageListToolBarL)
		return false;
	::ImageList_Add(m_hImageListToolBarL, ::LoadBitmap(m_hInstance, MAKEINTRESOURCE(IDB_TOOLBAR_L)), NULL);
	m_hImageListAddrButtons = ::ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);
	if (!m_hImageListAddrButtons)
		return false;
	::ImageList_Add(m_hImageListAddrButtons, ::LoadBitmap(m_hInstance, MAKEINTRESOURCE(IDB_ADDRESS_BUTTONS)), NULL);
	m_hImageListAddrButtonsL = ::ImageList_Create(32, 32, ILC_COLOR32 | ILC_MASK, 0, 0);
	if (!m_hImageListAddrButtonsL)
		return false;
	::ImageList_Add(m_hImageListAddrButtonsL, ::LoadBitmap(m_hInstance, MAKEINTRESOURCE(IDB_ADDRESS_BUTTONS_L)), NULL);

	return true;
}

bool CMainApplication::InitWindowClasses()
{
	if (!::InitSplitter(m_hInstance))
		return false;

	union
	{
		WNDCLASSEXW wcex;
		WNDCLASSEX wcexA;
	};
	CMyStringW strC(s_szMainWndClass);

	wcex.cbSize = sizeof(WNDCLASSEXW);

	wcex.style			= 0;
	//wcex.style = 0;
	wcex.lpfnWndProc	= GetMyWndProc();
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= m_hInstance;
	wcex.hIcon			= LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_EASYFTP));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	//wcex.hbrBackground  = NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= strC;
	wcex.hIconSm		= (HICON) LoadImage(wcex.hInstance, MAKEINTRESOURCE(IDI_EASYFTP),
		IMAGE_ICON, 16, 16, 0);

	if (!::RegisterClassExW(&wcex))
	{
		wcexA.lpszClassName = strC;
		if (!::RegisterClassExA(&wcexA))
			return false;
	}
	strC = s_szViewParentWndClass;
	wcex.lpszClassName	= strC;
	wcex.hIcon = wcex.hIconSm = NULL;
	if (!::RegisterClassExW(&wcex))
	{
		wcexA.lpszClassName = strC;
		if (!::RegisterClassExA(&wcexA))
			return false;
	}
	return true;
}

bool CMainApplication::InitAppData()
{
	::GetDDEVariables();

	::srand((unsigned int) time(NULL));

	{
		GetModuleFileNameString(m_hInstance, m_strINIFile);
		LPWSTR lpb = m_strINIFile.GetBuffer();
		LPWSTR lp = wcsrchr(lpb, L'.');
		if (lp)
		{
			lp++;
			m_strINIFile.ReleaseBuffer((DWORD) (((DWORD_PTR) lp - (DWORD_PTR) lpb) / sizeof(WCHAR)));
		}
		else
			m_strINIFile += L'.';
		m_strINIFile += L"ini";
	}
	{
		LPCWSTR lps = m_strINIFile;
		LPCWSTR lpe = wcsrchr(lps, L'\\');
		if (lpe)
			m_strLocalViewStateFile.SetString(lps, (DWORD) ((DWORD_PTR) (lpe + 1) - (DWORD_PTR) lps) / sizeof(WCHAR));
		m_strServerViewStateFile = m_strLocalViewStateFile;
		m_strLocalViewStateFile += s_szLocalViewStateFile;
		m_strServerViewStateFile += s_szServerViewStateFile;
	}

	//{
	//	CMyStringW str(s_wszDefTextFileType);
	//	LPWSTR lpb = str.GetBuffer();
	//	LPWSTR lp = wcschr(lpb, L';');
	//	while (lp)
	//	{
	//		*lp++ = 0;
	//		m_arrDefTextFileType.Add(lpb);
	//		lpb = lp;
	//		lp = wcschr(lp, L';');
	//	}
	//}

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

	{
		::SetLastError(ERROR_SUCCESS);
		DWORD dw;
		bool bLastIsDelimiter = true;
		{
			LPWSTR lpw = m_strTempPath.GetBuffer(MAX_PATH + 1);
			dw = ::GetTempPathW(MAX_PATH, lpw);
			if (dw)
				bLastIsDelimiter = (lpw[dw - 1] == L'\\');
		}
		if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
		{
			LPSTR lp = m_strTempPath.GetBufferA(MAX_PATH + 1);
			dw = ::GetTempPathA(MAX_PATH, lp);
			if (dw)
				bLastIsDelimiter = (lp[dw - 1] == '\\');
			m_strTempPath.ReleaseBufferA();
		}
		else
			m_strTempPath.ReleaseBuffer();
		if (!bLastIsDelimiter)
			m_strTempPath += L'\\';
		m_strTempPath += L"EasySFTP";
		while (true)
		{
			if (!::MyIsExistFileW(m_strTempPath) || ::MyIsDirectoryW(m_strTempPath))
				break;
			m_strTempPath += L'_';
		}
		::MyCreateDirectoryW(m_strTempPath, NULL);
		m_strTempPath += L'\\';
	}

	m_hMenuPopup = ::LoadMenu(m_hInstance, MAKEINTRESOURCE(IDR_POPUP));

	m_strTitle.LoadString(IDS_APP_TITLE);

	return true;
}


//int CMainApplication::GetImageListIconIndex(IExtractIconA* pIcon)
//{
//	CMyStringW str, strKey;
//	int n;
//	UINT u;
//	HRESULT hr;
//	HICON hi, his;
//
//	hr = pIcon->GetIconLocation(GIL_OPENICON, str.GetBufferA(MAX_PATH), MAX_PATH, &n, &u);
//	if (FAILED(hr))
//		return -1;
//	str.ReleaseBufferA();
//	strKey.Format(L",%d", n);
//	strKey.InsertString(str, 0);
//	if (m_mapIcon.IsItemKey(strKey))
//	{
//		n = m_mapIcon.GetItem(strKey);
//		return n;
//	}
//	hr = pIcon->Extract(str, (UINT) n, &hi, &his, MAKELONG(32, 16));
//	n = ::ImageList_AddIcon(m_hImageListFileIcon, his);
//	::DestroyIcon(hi);
//	::DestroyIcon(his);
//	m_mapIcon.Add(n, strKey);
//	return n;
//}
//
//int CMainApplication::GetImageListIconIndex(IExtractIconW* pIcon)
//{
//	CMyStringW str, strKey;
//	int n;
//	UINT u;
//	HRESULT hr;
//	HICON hi, his;
//
//	hr = pIcon->GetIconLocation(GIL_OPENICON, str.GetBuffer(MAX_PATH), MAX_PATH, &n, &u);
//	if (FAILED(hr))
//		return -1;
//	str.ReleaseBuffer();
//	strKey.Format(L",%d", n);
//	strKey.InsertString(str, 0);
//	if (m_mapIcon.IsItemKey(strKey))
//	{
//		n = m_mapIcon.GetItem(strKey);
//		return n;
//	}
//	hr = pIcon->Extract(str, (UINT) n, &hi, &his, MAKELONG(32, 16));
//	if (FAILED(hr))
//		return -1;
//	n = ::ImageList_AddIcon(m_hImageListFileIcon, his);
//	::DestroyIcon(hi);
//	::DestroyIcon(his);
//	m_mapIcon.Add(n, strKey);
//	return n;
//}
//
//int CMainApplication::GetImageListIconIndex(LPCWSTR lpszFileName, DWORD dwAttributes)
//{
//	CMyStringW str, strKey;
//	int n;
//	SHFILEINFOW sfi;
//	DWORD_PTR dw;
//
//	if (dwAttributes == -1)
//	{
//		dwAttributes = ::GetFileAttributesW(lpszFileName);
//		if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
//		{
//			str = lpszFileName;
//			dwAttributes = ::GetFileAttributesA(str);
//		}
//		if (dwAttributes == -1)
//			dwAttributes = FILE_ATTRIBUTE_NORMAL;
//	}
//	dw = ::SHGetFileInfoW(lpszFileName, dwAttributes, &sfi, sizeof(sfi),
//		SHGFI_ICONLOCATION | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
//	if (!dw)
//	{
//		SHFILEINFOA sfiA;
//		str = lpszFileName;
//		dw = ::SHGetFileInfoA(str, dwAttributes, &sfiA, sizeof(sfiA),
//			SHGFI_ICONLOCATION | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
//		if (!dw)
//			return -1;
//		str = sfiA.szDisplayName;
//		n = sfiA.iIcon;
//	}
//	else
//	{
//		str = sfi.szDisplayName;
//		n = sfi.iIcon;
//	}
//	strKey.Format(L",%d", n);
//	strKey.InsertString(str, 0);
//	if (m_mapIcon.IsItemKey(strKey))
//	{
//		n = m_mapIcon.GetItem(strKey);
//		return n;
//	}
//
//	dw = ::SHGetFileInfoW(lpszFileName, dwAttributes, &sfi, sizeof(sfi),
//		SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
//	if (!dw)
//	{
//		SHFILEINFOA sfiA;
//		str = lpszFileName;
//		dw = ::SHGetFileInfoA(str, dwAttributes, &sfiA, sizeof(sfiA),
//			SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
//		if (!dw)
//			return -1;
//		sfi.hIcon = sfiA.hIcon;
//	}
//
//	n = ::ImageList_AddIcon(m_hImageListFileIcon, sfi.hIcon);
//	::DestroyIcon(sfi.hIcon);
//	m_mapIcon.Add(n, strKey);
//	return n;
//}

bool CMainApplication::FileDialog(bool bOpen, CMyStringW& rstrFileName, CMyWindow* pWndOwner)
{
	BOOL bRet;
	if (m_bUseOFNUnicode)
	{
		m_ofnW.lpstrFile = rstrFileName.GetBuffer(MAX_PATH);
		m_ofnW.nMaxFile = MAX_PATH;
		if (pWndOwner)
			m_ofnW.hwndOwner = pWndOwner->m_hWnd;
		else
			m_ofnW.hwndOwner = m_pMainWnd->m_hWnd;
		m_ofnW.lpstrFilter = m_strFilter;
	}
	else
	{
		m_ofnA.lpstrFile = rstrFileName.GetBufferA(MAX_PATH);
		m_ofnA.nMaxFile = MAX_PATH;
		if (pWndOwner)
			m_ofnA.hwndOwner = pWndOwner->m_hWnd;
		else
			m_ofnA.hwndOwner = m_pMainWnd->m_hWnd;
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

bool CMainApplication::FolderDialog(CMyStringW& rstrDirectoryName, CMyWindow* pWndOwner)
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

void CMainApplication::GetTempFile(LPCWSTR lpszFileName, CMyStringW& rstrFullPath)
{
	CMyStringW str, str2;
	LPCWSTR lp = wcsrchr(lpszFileName, L'.');
	if (!lp)
	{
		lp = lpszFileName;
		while (*lp)
			lp++;
	}
	int nCounter = 0;
	str.SetString(lpszFileName, (DWORD) (((DWORD_PTR) lp - (DWORD_PTR) lpszFileName) / sizeof(WCHAR)));
	while (true)
	{
		rstrFullPath = m_strTempPath;
		rstrFullPath += str;
		if (nCounter)
		{
			str2.Format(L"[%d]", nCounter);
			rstrFullPath += str2;
		}
		if (*lp)
			rstrFullPath += lp;
		if (!::MyIsExistFileW(rstrFullPath))
			break;
		nCounter++;
	}
}

//static bool __stdcall IsMatchItemIDListOfRegValue(HKEY hKey, LPCTSTR lpszValueName, DWORD dwLength, PCIDLIST_ABSOLUTE pidl)
//{
//	if (::GetItemIDListSize(pidl) != (SIZE_T) dwLength)
//		return false;
//
//	LPBYTE lpb = (LPBYTE) malloc((size_t) dwLength);
//	DWORD dw = dwLength;
//	bool bRet = false;
//	if (::RegQueryValueEx(hKey, lpszValueName, NULL, NULL, lpb, &dw) == ERROR_SUCCESS)
//		bRet = (dw == dwLength && memcmp(pidl, lpb, (size_t) dw) == 0);
//	free(lpb);
//	return bRet;
//}

CMainApplication::CMRUStream::CMRUStream(IStream* pStreamBase, CMRUStreamData* pData)
{
	m_uRef = 1;
	{
		ULARGE_INTEGER uli;
		uli.QuadPart = pData->nSize;
		pStreamBase->SetSize(uli);
		if (pData->nSize)
		{
			pStreamBase->Write(pData->pbData, (ULONG) pData->nSize, &uli.LowPart);
			uli.QuadPart = 0;
			pStreamBase->Seek(*((LARGE_INTEGER*) &uli), STREAM_SEEK_SET, &uli);
		}
	}
	m_pData = pData;
	m_pStream = pStreamBase;
	pStreamBase->AddRef();
}

CMainApplication::CMRUStream::~CMRUStream()
{
	STATSTG stg;
	m_pStream->Stat(&stg, STATFLAG_NONAME);
	PBYTE pb = (PBYTE)(m_pData->pbData ?
		realloc(m_pData->pbData, (size_t) stg.cbSize.QuadPart) :
		malloc((size_t) stg.cbSize.QuadPart));
	if (!pb)
	{
		if (m_pData->pbData)
		{
			free(m_pData->pbData);
			m_pData->pbData = NULL;
		}
		m_pData->nSize = 0;
	}
	else
	{
		m_pData->pbData = pb;
		m_pData->nSize = (size_t) stg.cbSize.QuadPart;
		while (stg.cbSize.QuadPart)
		{
			ULONG u = (stg.cbSize.QuadPart >= 8192 ? 8192 : (ULONG) stg.cbSize.QuadPart);
			ULONG u2 = 0;
			HRESULT hr = m_pStream->Read(pb, u, &u2);
			if (FAILED(hr) || !u2)
				break;
			pb += u2;
			stg.cbSize.QuadPart -= u2;
		}
	}
	m_pStream->Release();
}

HRESULT CMainApplication::GetViewStateStream(bool bServer, PCIDLIST_ABSOLUTE pidlCurrent, DWORD grfMode, IStream** ppStream)
{
	if (!ppStream)
		return E_POINTER;
	CMyPtrArrayT<CMRUStreamData>* pa = (bServer ? &m_aServerStreams : &m_aLocalStreams);
	CMRUStreamData* pData = NULL;
	for (int i = 0; i < pa->GetCount(); i++)
	{
		pData = pa->GetItem(i);
		if (::IsEqualIDList(pidlCurrent, pData->pidl))
			break;
		pData = NULL;
	}
	if (!pData)
	{
		if (grfMode == STGM_READ)
		{
			*ppStream = new CEmptyStream();
			return *ppStream ? S_OK : E_OUTOFMEMORY;
		}
		pData = (CMRUStreamData*) malloc(sizeof(CMRUStreamData));
		pData->pidl = (PIDLIST_ABSOLUTE) ::DuplicateItemIDList(pidlCurrent);
		pData->pbData = NULL;
		pData->nSize = 0;
		pa->Add(pData);
	}

	IStream* pRet;
	HRESULT hr = ::CreateStreamOnHGlobal(NULL, TRUE, &pRet);
	if (FAILED(hr))
		return hr;
	*ppStream = new CMRUStream(pRet, pData);
	pRet->Release();
	return *ppStream ? S_OK : E_OUTOFMEMORY;
//	HKEY hKey;
//	LONG lError;
//	lError = ::RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StreamMRU"), 0, KEY_QUERY_VALUE, &hKey);
//	if (lError != ERROR_SUCCESS)
//		return E_FAIL;
//
//	LPDWORD lpdwMRULists;
//
//	DWORD dwDA, dwCD;
//	switch (grfMode)
//	{
//		case STGM_READ:
//			dwDA = GENERIC_READ;
//			dwCD = OPEN_ALWAYS;
//			break;
//		case STGM_WRITE:
//			dwDA = GENERIC_WRITE;
//			dwCD = CREATE_ALWAYS;
//			break;
//	}
////#ifdef _EASYSFTP_USE_VIEWSTATE_STREAM
//	//if (FAILED(::CreateStreamOnHGlobal(NULL, TRUE, &m_pStreamViewStateLocal)))
//	if (FAILED(::MyOpenFileToStreamEx(m_strLocalViewStateFile, GENERIC_READ | GENERIC_WRITE,
//		FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL, ppStream)))
////#endif
//		m_pStreamViewStateLocal = new CEmptyStream();
////#ifdef _EASYSFTP_USE_VIEWSTATE_STREAM
//	//if (FAILED(::CreateStreamOnHGlobal(NULL, TRUE, &m_pStreamViewStateServer)))
//	if (FAILED(::MyOpenFileToStreamEx(m_strServerViewStateFile, GENERIC_READ | GENERIC_WRITE,
//		FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL, ppStream)))
////#endif
//		m_pStreamViewStateServer = new CEmptyStream();
}

////////////////////////////////////////////////////////////////////////////////

static void __stdcall _LoadStateData(PVOID pvSection, HANDLE hFileState, CMyPtrArrayT<CMRUStreamData>& aStreams)
{
	CMyStringW str;
	int i;
	CMRUStreamData* pData;
	ULONGLONG ull;
	DWORD dw, dwr;
	PBYTE pb;
	PIDLIST_ABSOLUTE pidl;

	i = 0;
	while (true)
	{
		str.Format(L"%d", i++);
		LPWSTR lp = MyGetProfileStringW(pvSection, str, NULL);
		if (!lp)
			break;
		dw = ::UnformatByteStringExW(lp, NULL, 0);
		if (!dw)
		{
			free(lp);
			break;
		}
		pidl = (PIDLIST_ABSOLUTE) ::CoTaskMemAlloc((SIZE_T) dw);
		::UnformatByteStringExW(lp, pidl, dw);
		free(lp);
		pData = (CMRUStreamData*) malloc(sizeof(CMRUStreamData));
		pData->pidl = pidl;
		pData->nSize = 0;
		pData->pbData = NULL;
		aStreams.Add(pData);
		if (!::ReadFile(hFileState, &ull, sizeof(ull), &dwr, NULL) || !dwr)
			break;
		pb = (PBYTE) malloc((size_t) ull);
		if (!pb)
			break;
		pData->pbData = pb;
		pData->nSize = (SIZE_T) ull;
		while (ull)
		{
			dw = (ull > 8192 ? 8192 : (DWORD) ull);
			if (!::ReadFile(hFileState, pb, dw, &dwr, NULL) || !dwr)
				break;
			pb += dwr;
			ull -= dwr;
		}
	}
}

void CMainApplication::LoadINISettings()
{
	HINIFILE hINI;
	PVOID pvSection;
	CMyStringW str;
	//int n;

	hINI = ::MyLoadINIFileW(m_strINIFile, false);

	pvSection = ::MyGetProfileSectionW(hINI, L"General");
	//if (pvSection)
	{
		m_nSplitterPos = ::MyGetProfileIntW(pvSection, L"SplitterPos", -1);
		if (::MyGetProfileBinaryW(pvSection, L"WindowPlacement", &m_wpFrame, sizeof(m_wpFrame)) > 0)
			m_bUsePlacement = true;
//#ifdef _EASYSFTP_USE_VIEWSTATE_STREAM
		//ULARGE_INTEGER uli;
		//uli.QuadPart = ::MyGetProfileBinaryW(pvSection, L"ViewState", NULL, 0);
		//if (uli.QuadPart)
		//{
		//	if (SUCCEEDED(m_pStreamViewState->SetSize(uli)))
		//	{
		//		HGLOBAL hglb;
		//		if (SUCCEEDED(::GetHGlobalFromStream(m_pStreamViewState, &hglb)))
		//		{
		//			LPVOID lpv = ::GlobalLock(hglb);
		//			::MyGetProfileBinaryW(pvSection, L"ViewState", lpv, (DWORD) uli.QuadPart);
		//			::GlobalUnlock(hglb);
		//		}
		//	}
		//}
//#endif
		::MyEndReadProfileSectionW(pvSection);
	}

	HANDLE hFileState;
	pvSection = ::MyGetProfileSectionW(hINI, L"LocalViewState");
	if (pvSection)
	{
		hFileState = ::CreateFileW(m_strLocalViewStateFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
			hFileState = ::CreateFileA(m_strLocalViewStateFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFileState != INVALID_HANDLE_VALUE && hFileState)
		{
			_LoadStateData(pvSection, hFileState, m_aLocalStreams);
			::CloseHandle(hFileState);
		}
		::MyEndReadProfileSectionW(pvSection);
	}
	pvSection = ::MyGetProfileSectionW(hINI, L"ServerViewState");
	if (pvSection)
	{
		hFileState = ::CreateFileW(m_strServerViewStateFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
			hFileState = ::CreateFileA(m_strServerViewStateFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFileState != INVALID_HANDLE_VALUE && hFileState)
		{
			_LoadStateData(pvSection, hFileState, m_aServerStreams);
			::CloseHandle(hFileState);
		}
		::MyEndReadProfileSectionW(pvSection);
	}

	::MyCloseINIFile(hINI);
}

static void __stdcall _SaveStateData(HANDLE hFileINI, HANDLE hFileState, const CMyPtrArrayT<CMRUStreamData>& aStreams)
{
	CMyStringW str;
	ULONGLONG ull;
	DWORD dw, dwr;

	for (int i = 0; i < aStreams.GetCount(); i++)
	{
		CMRUStreamData* p = aStreams.GetItem(i);
		str.Format(L"%d", i);
		::MyWriteINIValueW(hFileINI, str, p->pidl, (DWORD) ::GetItemIDListSize(p->pidl));
		ull = p->nSize;
		::WriteFile(hFileState, &ull, sizeof(ull), &dwr, NULL);
		PBYTE pb = p->pbData;
		while (ull)
		{
			dw = (ull >= 8192 ? 8192 : (DWORD) ull);
			ull -= dw;
			::WriteFile(hFileState, pb, dw, &dwr, NULL);
			pb += dw;
		}
	}
}

void CMainApplication::SaveINISettings()
{
	HANDLE hFile;
	CMyStringW str;
	hFile = ::CreateFileW(m_strINIFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
		hFile = ::CreateFileA(m_strINIFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	::MyWriteINISectionW(hFile, L"General");
	::MyWriteINIValueW(hFile, L"SplitterPos", m_nSplitterPos);
	::MyWriteINIValueW(hFile, L"WindowPlacement", &m_wpFrame, sizeof(m_wpFrame));

	HANDLE hFileState;
	hFileState = ::CreateFileW(m_strLocalViewStateFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
		hFileState = ::CreateFileA(m_strLocalViewStateFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFileState != INVALID_HANDLE_VALUE && hFileState)
	{
		::MyWriteCRLFW(hFile);
		::MyWriteINISectionW(hFile, L"LocalViewState");
		_SaveStateData(hFile, hFileState, m_aLocalStreams);
		::CloseHandle(hFileState);
	}

	hFileState = ::CreateFileW(m_strServerViewStateFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
		hFileState = ::CreateFileA(m_strServerViewStateFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFileState != INVALID_HANDLE_VALUE && hFileState)
	{
		::MyWriteCRLFW(hFile);
		::MyWriteINISectionW(hFile, L"ServerViewState");
		_SaveStateData(hFile, hFileState, m_aServerStreams);
		::CloseHandle(hFileState);
	}
//#ifdef _EASYSFTP_USE_VIEWSTATE_STREAM
	//HGLOBAL hglb;
	//if (SUCCEEDED(::GetHGlobalFromStream(m_pStreamViewState, &hglb)))
	//{
	//	SIZE_T nSize = ::GlobalSize(hglb);
	//	LPVOID lpv = ::GlobalLock(hglb);
	//	::MyWriteINIValueW(hFile, L"ViewState", lpv, (DWORD) nSize);
	//	::GlobalUnlock(hglb);
	//}
//#endif
	//if (m_aKnownFingerPrints.GetCount())
	//{
	//	::MyWriteCRLFW(hFile);
	//	::MyWriteINISectionW(hFile, L"FingerPrint");
	//	for (int n = 0; n < m_aKnownFingerPrints.GetCount(); n++)
	//	{
	//		CKnownFingerPrint* pPrint = m_aKnownFingerPrints.GetItem(n);
	//		::MyWriteINIValueW(hFile, pPrint->strHostName, pPrint->pFingerPrint, (DWORD) pPrint->nFingerPrintLen);
	//	}
	//}

	//for (int n = 0; n < m_aHostSettings.GetCount(); n++)
	//{
	//	CHostSettings* pHost = m_aHostSettings.GetItem(n);
	//	::MyWriteCRLFW(hFile);
	//	str.Format(L"Host%d", n + 1);
	//	::MyWriteINISectionW(hFile, str);
	//	::MyWriteINIValueW(hFile, L"Name", pHost->strDisplayName);
	//	::MyWriteINIValueW(hFile, L"Host", pHost->strHostName);
	//	::MyWriteINIValueW(hFile, L"SFTPMode", pHost->bSFTPMode ? 1 : 0);
	//	::MyWriteINIValueW(hFile, L"Port", pHost->nPort);
	//	::MyWriteINIValueW(hFile, L"UserName", pHost->strUserName);
	//	::MyWriteINIValueW(hFile, L"TextMode", (int) pHost->bTextMode);
	//	::MyWriteINIValueW(hFile, L"ServerCharset", (int) pHost->nServerCharset);
	//	::MyWriteINIValueW(hFile, L"InitLocalPath", pHost->strInitLocalPath);
	//	::MyWriteINIValueW(hFile, L"InitServerPath", pHost->strInitServerPath);
	//	::MyWriteINIValueW(hFile, L"TransferMode", (int) pHost->nTransferMode);
	//	str.Empty();
	//	for (int n2 = 0; n2 < pHost->arrTextFileType.GetCount(); n2++)
	//	{
	//		if (!str.IsEmpty())
	//			str += L';';
	//		str += pHost->arrTextFileType.GetItem(n2);
	//	}
	//	::MyWriteINIValueW(hFile, L"TextFileType", str);
	//	::MyWriteINIValueW(hFile, L"UseSystemTextFileType", pHost->bUseSystemTextFileType ? 1 : 0);
	//	::MyWriteINIValueW(hFile, L"AdjustRecvModifyTime", pHost->bAdjustRecvModifyTime ? 1 : 0);
	//	::MyWriteINIValueW(hFile, L"AdjustSendModifyTime", pHost->bAdjustSendModifyTime ? 1 : 0);
	//	::MyWriteINIValueW(hFile, L"ChmodCommand", pHost->strChmodCommand);
	//	//::MyWriteINIValueW(hFile, L"TouchCommand", pHost->strTouchCommand);
	//}
	::CloseHandle(hFile);
}

typedef HRESULT (STDMETHODCALLTYPE* T_SHAutoComplete)(HWND hwndEdit, DWORD dwFlags);
static T_SHAutoComplete s_pfnSHAutoComplete = NULL;
static bool s_bSHAutoCompleteReceived = false;

void CMainApplication::DoAutoComplete(HWND hWndEdit, IEnumString* pEnumString)
{
	if (!s_bSHAutoCompleteReceived)
	{
		HINSTANCE hInstSHLWAPI = ::GetModuleHandle(_T("shlwapi.dll"));
		if (!hInstSHLWAPI)
			hInstSHLWAPI = ::LoadLibrary(_T("shlwapi.dll"));
		s_pfnSHAutoComplete = (T_SHAutoComplete) ::GetProcAddress(hInstSHLWAPI, "SHAutoComplete");
		s_bSHAutoCompleteReceived = true;
	}
	if (!pEnumString)
	{
		if (!s_pfnSHAutoComplete)
			return;
		s_pfnSHAutoComplete(hWndEdit, SHACF_FILESYS_DIRS);
	}
	else
	{
		;
	}
}

bool CMainApplication::CheckExternalApplications()
{
	//if (!m_aObjectTransferring.GetCount())
	//	return true;
	if (::MyMessageBoxW(NULL, MAKEINTRESOURCEW(IDS_EXTERNAL_APP_IS_DOWNLOADING), NULL, MB_ICONEXCLAMATION | MB_YESNO) == IDNO)
		return false;
	return true;
}

void CMainApplication::DoRegister()
{
	CMyStringW strExe;
	if (!::MySearchPathStringW(L"RegESFTP.exe", strExe))
	{
		::MyMessageBoxW(NULL, MAKEINTRESOURCEW(IDS_REGESFTP_NOT_FOUND), NULL, MB_ICONEXCLAMATION);
		return;
	}

	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(sei));
	sei.cbSize = sizeof(sei);
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.lpFile = strExe;
	sei.lpParameters = m_bUnregisterOperation ? _T("/unregister") : NULL;
	sei.lpVerb = m_bIsRegisterForSystem ? _T("runas") : NULL;
	sei.nShow = SW_SHOWNORMAL;

	if (::ShellExecuteEx(&sei))
	{
		::WaitForSingleObject(sei.hProcess, INFINITE);
		::CloseHandle(sei.hProcess);
	}

#ifdef _DEBUG
	if (!::UnregisterClassW(s_szMainWndClass, m_hInstance))
	{
		strExe = s_szMainWndClass;
		::UnregisterClassA(strExe, m_hInstance);
		strExe = s_szViewParentWndClass;
		::UnregisterClassA(strExe, m_hInstance);
		::UnregisterClassA(SPLITTER_CLASSA, m_hInstance);
	}
	else
	{
		::UnregisterClassW(s_szViewParentWndClass, m_hInstance);
		::UnregisterClassW(SPLITTER_CLASSW, m_hInstance);
	}
	if (!m_bNoRestart)
		s_bRestartApp = true;
#else
	if (!m_bNoRestart)
	{
		PROCESS_INFORMATION pi;
		BOOL bRet;
		if (::GetCommandLineW())
		{
			STARTUPINFOW si;
			si.cb = sizeof(si);
			memset(&si, 0, sizeof(si));
			bRet = ::CreateProcessW(NULL, ::GetCommandLineW(), NULL, NULL, FALSE,
				0, NULL, NULL, &si, &pi);
		}
		else
		{
			STARTUPINFOA si;
			si.cb = sizeof(si);
			memset(&si, 0, sizeof(si));
			bRet = ::CreateProcessA(NULL, ::GetCommandLineA(), NULL, NULL, FALSE,
				0, NULL, NULL, &si, &pi);
		}
		if (bRet)
		{
			::CloseHandle(pi.hProcess);
			::CloseHandle(pi.hThread);
		}
	}
#endif
}
