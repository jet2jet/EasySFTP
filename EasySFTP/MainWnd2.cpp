/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 MainWnd2.cpp - implementations of CMainWindow (implementations of interfaces)
 */

#include "StdAfx.h"
#include "EasySFTP.h"
#include "MainWnd.h"

#include "IDList.h"

#if !defined(NTDDI_WIN7) || (NTDDI_VERSION < NTDDI_WIN7)
#define INITGUID
#include <guiddef.h>
DEFINE_GUID(SID_SInPlaceBrowser, 0x1D2AE02B, 0x3655, 0x46CC, 0xB6, 0x3A, 0x28, 0x59, 0x88, 0x15, 0x3B, 0xCA);
#undef INITGUID
#include <guiddef.h>
#endif

////////////////////////////////////////////////////////////////////////////////

ULONG CMainWindow::InternalAddRef()
{
	return ++m_uRef;
}

ULONG CMainWindow::InternalRelease()
{
	ULONG u = --m_uRef;
	if (!u)
		delete this;
	return u;
}

STDMETHODIMP CMainWindow::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	//{
	//	CMyStringW str;
	//	str.Format(L"QueryInterface for {%08lX-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}\n",
	//		riid.Data1, riid.Data2, riid.Data3,
	//		riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3],
	//		riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7]);
	//	OutputDebugString(str);
	//}
	if (IsEqualIID(riid, IID_IUnknown))
	{
		*ppv = (IUnknown*) this;
		AddRef();
		return S_OK;
	}
	if (IsEqualIID(riid, IID_IEasySFTPListener))
	{
		*ppv = (IEasySFTPListener*) this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

// in MainWnd.cpp
bool __stdcall IsWindowOrChildrenFocused(HWND hWnd, HWND hWndFocus);

HRESULT CMainWindow::QueryActiveShellView(bool bServer, IShellView** ppshv)
{
	//if (IsWindowOrChildrenFocused(m_wndListViewServer, ::GetFocus()))
	if (bServer)
	{
		if (m_wndListViewServer.m_pView)
			return m_wndListViewServer.m_pView->QueryInterface(IID_IShellView, (void**) ppshv);
	}
	else
	{
		if (m_wndListViewLocal.m_pView)
			return m_wndListViewLocal.m_pView->QueryInterface(IID_IShellView, (void**) ppshv);
	}
	*ppshv = NULL;
	return S_OK;
}

void CMainWindow::OnViewWindowActive(bool bServer, IShellView* pView)
{
	if (bServer)
	{
		if (m_wndListViewLocal.m_pView)
		{
			m_wndListViewLocal.m_pView->UIActivate(SVUIA_ACTIVATE_NOFOCUS);
			m_wndListViewLocal.OnFocusView(false);
		}
		if (m_wndListViewServer.m_pView)
			m_wndListViewServer.OnFocusView(true);
	}
	else
	{
		if (m_wndListViewServer.m_pView)
		{
			m_wndListViewServer.m_pView->UIActivate(SVUIA_ACTIVATE_NOFOCUS);
			m_wndListViewServer.OnFocusView(false);
		}
		if (m_wndListViewLocal.m_pView)
			m_wndListViewLocal.OnFocusView(true);
	}
}

struct CBrowseViewData
{
	bool bServer;
	bool bFilePath;
	union
	{
		LPWSTR lpszPath;
		struct
		{
			UINT wFlags;
			PIDLIST_RELATIVE pidl;
		};
	};
};

// lParam: CBrowseViewData* (allocated by CoTaskMemAlloc)
LRESULT CMainWindow::OnBrowseView(WPARAM wParam, LPARAM lParam)
{
	CBrowseViewData* pData = (CBrowseViewData*) lParam;
	if (!pData->bServer && pData->bFilePath)
	{
		UpdateCurrentFolderAbsolute(pData->lpszPath);
		::CoTaskMemFree(pData->lpszPath);
	}
	else
	{
		switch (pData->wFlags & (SBSP_RELATIVE | SBSP_PARENT | SBSP_NAVIGATEBACK | SBSP_NAVIGATEFORWARD))
		{
			case SBSP_RELATIVE:
				if (pData->bServer)
					UpdateServerFolder(pData->pidl);
				else
					UpdateCurrentFolder(pData->pidl);
				break;
			case SBSP_ABSOLUTE:
				if (pData->bServer)
					UpdateServerFolderAbsolute((PCUIDLIST_ABSOLUTE) pData->pidl);
				else
					UpdateCurrentFolderAbsolute((PCUIDLIST_ABSOLUTE) pData->pidl);
				break;
			case SBSP_PARENT:
				if (pData->bServer)
					NavigateServerParentFolder();
				else
					NavigateParentFolder();
				break;
		}
		if (pData->pidl)
			::CoTaskMemFree(pData->pidl);
	}
	::CoTaskMemFree(pData);
	return 0;
}

STDMETHODIMP CMainWindow::ChangeLocalDirectory(LPCWSTR lpszPath)
{
	CBrowseViewData* pData = (CBrowseViewData*) ::CoTaskMemAlloc(sizeof(CBrowseViewData));
	pData->bServer = false;
	pData->bFilePath = true;
	size_t nLen = wcslen(lpszPath) + 1;
	pData->lpszPath = (LPWSTR) ::CoTaskMemAlloc(sizeof(WCHAR) * nLen);
	memcpy(pData->lpszPath, lpszPath, sizeof(WCHAR) * nLen);
	::PostMessage(m_hWnd, MY_WM_BROWSE_VIEW, 0, (LPARAM) pData);
	return S_OK;
}

LRESULT CMainWindow::OnUpdateSetMenu(WPARAM wParam, LPARAM lParam)
{
	if (!m_bWindowCreated)
	{
		::PostMessage(m_hWnd, MY_WM_UPDATESETMENU, wParam, lParam);
		return 0;
	}
	::SetMenu(m_hWnd, m_hMenuSet ? m_hMenuSet : m_hMenu);
	::DrawMenuBar(m_hWnd);
	m_bUpdateSetMenu = false;
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CMainWindow::CBrowser::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	//{
	//	CMyStringW str;
	//	str.Format(L"QueryInterface for {%08lX-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}\n",
	//		riid.Data1, riid.Data2, riid.Data3,
	//		riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3],
	//		riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7]);
	//	OutputDebugString(str);
	//}
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IOleWindow) ||
		IsEqualIID(riid, IID_IShellBrowser))
	{
		*ppv = (IShellBrowser*) this;
		AddRef();
		return S_OK;
	}
#ifdef _EASYSFTP_USE_ICOMMDLGBROWSER
	else if (IsEqualIID(riid, IID_ICommDlgBrowser))
	{
		*ppv = (ICommDlgBrowser*) this;
		AddRef();
		return S_OK;
	}
#endif
	else if (IsEqualIID(riid, IID_IServiceProvider))
	{
		*ppv = (IServiceProvider*) this;
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid, IID_IInternetHostSecurityManager))
	{
		*ppv = (IInternetHostSecurityManager*) this;
		AddRef();
		return S_OK;
	}
	//else if (IsEqualIID(riid, IID_IShellBrowserService))
	//{
	//	*ppv = (IShellBrowserService*) this;
	//	AddRef();
	//	return S_OK;
	//}
	//else if (IsEqualIID(riid, IID_IOleCommandTarget))
	//{
	//	*ppv = (IOleCommandTarget*) this;
	//	AddRef();
	//	return S_OK;
	//}
	return E_NOINTERFACE;
}

STDMETHODIMP CMainWindow::CBrowser::QueryService(REFGUID guidService, REFIID riid, void FAR* FAR* ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	//{
	//	CMyStringW str;
	//	str.Format(L"QueryService for service {%08lX-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}, iid {%08lX-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}\n",
	//		guidService.Data1, guidService.Data2, guidService.Data3,
	//		guidService.Data4[0], guidService.Data4[1], guidService.Data4[2], guidService.Data4[3],
	//		guidService.Data4[4], guidService.Data4[5], guidService.Data4[6], guidService.Data4[7],
	//		riid.Data1, riid.Data2, riid.Data3,
	//		riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3],
	//		riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7]);
	//	OutputDebugString(str);
	//}
	// we need to check if guidService is SID_SShellBrowser (for XP SP2 and Vista)
	//   or SID_SInPlaceBrowser (for Windows 7)
	if (IsEqualGUID(guidService, SID_STopLevelBrowser) ||
		IsEqualGUID(guidService, SID_SShellBrowser) ||
		IsEqualGUID(guidService, SID_SInPlaceBrowser))
		return QueryInterface(riid, ppvObject);
	else if (IsEqualGUID(guidService, SID_STopWindow))
		return QueryInterface(riid, ppvObject);
	else if (IsEqualGUID(guidService, SID_SInternetHostSecurityManager))
		return QueryInterface(riid, ppvObject);
	return E_NOTIMPL;
}

STDMETHODIMP_(ULONG) CMainWindow::CBrowser::AddRef()
{
	return This()->InternalAddRef();
}

STDMETHODIMP_(ULONG) CMainWindow::CBrowser::Release()
{
	return This()->InternalRelease();
}

STDMETHODIMP CMainWindow::CBrowser::GetWindow(HWND* phWnd)
{
	if (!phWnd)
		return E_POINTER;
	//*phWnd = This()->m_hWnd;
	*phWnd = m_hWnd;
	return S_OK;
}

STDMETHODIMP CMainWindow::CBrowser::ContextSensitiveHelp(BOOL fEnterMode)
{
	return S_OK;
}

static void __stdcall DuplicateMenu(HMENU hMenuFrom, HMENU hMenuTo, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
	int i, nCount;
	MENUITEMINFO mii;
	CMyStringW str;

#ifdef _WIN64
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_ID | MIIM_STATE | MIIM_SUBMENU;
#else
	mii.cbSize = MENUITEMINFO_SIZE_V1;
	mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE | MIIM_SUBMENU;
#endif

	nCount = ::GetMenuItemCount(hMenuFrom);
	for (i = 0; i < nCount; i++)
	{
		mii.cch = MAX_PATH;
#ifdef _UNICODE
		mii.dwTypeData = str.GetBufferW(MAX_PATH);
#else
		mii.dwTypeData = str.GetBufferA(MAX_PATH);
#endif
		::GetMenuItemInfo(hMenuFrom, (UINT) i, TRUE, &mii);
		if (mii.hSubMenu)
		{
			HMENU hs = ::CreatePopupMenu();
			DuplicateMenu(mii.hSubMenu, hs, NULL);
			mii.hSubMenu = hs;
		}
		if (lpMenuWidths)
		{
			switch (mii.wID)
			{
				case FCIDM_MENU_FILE:
					lpMenuWidths->width[0] = ::GetMenuItemCount(mii.hSubMenu);
					break;
				case FCIDM_MENU_VIEW:
					lpMenuWidths->width[2] = ::GetMenuItemCount(mii.hSubMenu);
					break;
			}
		}
		::InsertMenuItem(hMenuTo, (UINT) i, TRUE, &mii);
	}
}

STDMETHODIMP CMainWindow::CBrowser::InsertMenusSB(HMENU hMenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
	lpMenuWidths->width[0] = 0;
	lpMenuWidths->width[1] = 0;
	lpMenuWidths->width[2] = 0;
	lpMenuWidths->width[3] = 0;
	lpMenuWidths->width[4] = 0;
	lpMenuWidths->width[5] = 0;
	DuplicateMenu(This()->m_hMenu, hMenuShared, lpMenuWidths);
	{
		CMyStringW str;
		str.Format(L"InsertMenusSB: this = %p, hMenu = %p\n", this, hMenuShared);
		OutputDebugString(str);
	}
	return S_OK;
}

STDMETHODIMP CMainWindow::CBrowser::SetMenuSB(HMENU hMenuShared, HOLEMENU hOleMenuRes, HWND hWndActiveObject)
{
	CMainWindow* pThis = This();
	if (!pThis->m_bUpdateSetMenu)
	{
		pThis->m_bUpdateSetMenu = true;
		::PostMessage(pThis->m_hWnd, MY_WM_UPDATESETMENU, 0, 0);
		pThis->m_hMenuSet = hMenuShared;
		pThis->m_hWndViewForMenu = hWndActiveObject;
	}
	else if (/*!pThis->m_hMenuSet &&*/ hMenuShared)
	{
		pThis->m_hMenuSet = hMenuShared;
		pThis->m_hWndViewForMenu = hWndActiveObject;
	}
	{
		CMyStringW str;
		str.Format(L"SetMenuSB: this = %p, hMenu = %p\n", this, hMenuShared);
		OutputDebugString(str);
	}
	return S_OK;
}

STDMETHODIMP CMainWindow::CBrowser::RemoveMenusSB(HMENU hMenuShared)
{
	int i, nCount;

	nCount = ::GetMenuItemCount(hMenuShared);

	for (i = 0; i < nCount; i++)
		::DeleteMenu(hMenuShared, i, MF_BYPOSITION);
	{
		CMyStringW str;
		str.Format(L"RemoveMenusSB: this = %p, hMenu = %p\n", this, hMenuShared);
		OutputDebugString(str);
	}

	return S_OK;
}

STDMETHODIMP CMainWindow::CBrowser::SetStatusTextSB(LPCWSTR pszStatusText)
{
	This()->SetStatusText(pszStatusText);
	return S_OK;
}

STDMETHODIMP CMainWindow::CBrowser::EnableModelessSB(BOOL fEnable)
{
	::EnableWindow(This()->m_hWnd, fEnable);
	return S_OK;
}

STDMETHODIMP CMainWindow::CBrowser::TranslateAcceleratorSB(LPMSG lpMsg, WORD wID)
{
	CMainWindow* pThis = This();
	if (!::TranslateAccelerator(pThis->m_hWnd, pThis->m_hAccel, lpMsg))
		return S_FALSE;
	return S_OK;
}

STDMETHODIMP CMainWindow::CBrowser::GetControlWindow(UINT uID, HWND* phWnd)
{
	if (!phWnd)
		return E_POINTER;
	if (uID == FCW_STATUS)
	{
		*phWnd = This()->m_wndStatusBar;
		return S_OK;
	}
	//if (uID == FCW_TREE)
	//{
	//	// dummy
	//	*phWnd = m_wndSplitter;
	//	return S_OK;
	//}
	*phWnd = NULL;
	return S_FALSE;
}

STDMETHODIMP CMainWindow::CBrowser::SendControlMsg(UINT uID, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pRet)
{
	LRESULT lr;
	//if (uID == FCW_TOOLBAR)
	//	lr = ::SendMessage(This()->m_wndToolBar, uMsg, wParam, lParam);
	//else
	if (uID == FCW_STATUS && (uMsg == SB_SETTEXTA || uMsg == SB_SETTEXTW))
		lr = ::SendMessage(This()->m_wndStatusBar, uMsg, ((wParam & 0xFF00) | 0), lParam);
	else
		return E_NOTIMPL;
	if (pRet)
		*pRet = lr;
	return S_OK;
}

STDMETHODIMP CMainWindow::CBrowser::SetToolbarItems(LPTBBUTTONSB lpButtons, UINT nButtons, UINT uFlags)
{
	return E_NOTIMPL;
}

STDMETHODIMP CMainWindow::CBrowser::GetSecurityId(BYTE* pbSecurityId, DWORD* pcbSecurityId, DWORD_PTR dwReserved)
{
	return INET_E_DEFAULT_ACTION;
}

// to show popup menu or do navigation for My Documents or Briefcases,
// we need to return URLPOLICY_ALLOW in this method
STDMETHODIMP CMainWindow::CBrowser::ProcessUrlAction(DWORD dwAction, BYTE* pPolicy, DWORD cbPolicy, BYTE* pContext, DWORD cbContext, DWORD dwFlags, DWORD dwReserved)
{
	if (!pPolicy)
		return E_POINTER;
	if (dwAction == URLACTION_SHELL_EXECUTE_HIGHRISK ||
		dwAction == URLACTION_SHELL_EXECUTE_MODRISK ||
		dwAction == URLACTION_SHELL_EXECUTE_LOWRISK ||
		dwAction == URLACTION_SHELL_SECURE_DRAGSOURCE)
	{
		*pPolicy = URLPOLICY_ALLOW;
		return S_OK;
	}
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CMainWindow::CBrowser::QueryCustomPolicy(REFGUID guidKey, BYTE** ppPolicy, DWORD* pcbPolicy, BYTE* pContext, DWORD cbContext, DWORD dwReserved)
{
	return INET_E_DEFAULT_ACTION;
}

//STDMETHODIMP CMainWindow::CBrowser::QueryStatus(const GUID* pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT* pCmdText)
//{
//	if (IsEqualGUID(*pguidCmdGroup, CGID_Explorer))
//	{
//		return E_FAIL;
//	}
//	return OLECMDERR_E_UNKNOWNGROUP;
//}
//
//STDMETHODIMP CMainWindow::CBrowser::Exec(const GUID* pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANT* pvaIn, VARIANT *pvaOut)
//{
//	if (IsEqualGUID(*pguidCmdGroup, CGID_Explorer))
//	{
//		switch (nCmdID)
//		{
//			case OLECMDID_PREREFRESH:
//				return S_OK;
//			default:
//				return E_FAIL;
//		}
//	}
//	return OLECMDERR_E_UNKNOWNGROUP;
//}

LRESULT CMainWindow::CBrowser::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == CWM_GETISHELLBROWSER)
		return (LRESULT)(IShellBrowser*) this;
	return CMyWindow::WindowProc(message, wParam, lParam);
}


STDMETHODIMP CMainWindow::CBrowserLocal::BrowseObject(PCUIDLIST_RELATIVE pidl, UINT wFlags)
{
	if ((wFlags & (SBSP_OPENMODE | SBSP_EXPLOREMODE | SBSP_HELPMODE)) == SBSP_OPENMODE ||
		(wFlags & (SBSP_OPENMODE | SBSP_EXPLOREMODE | SBSP_HELPMODE)) == SBSP_DEFMODE)
	{
		if ((wFlags & (SBSP_SAMEBROWSER | SBSP_NEWBROWSER)) == SBSP_DEFBROWSER ||
			(wFlags & (SBSP_SAMEBROWSER | SBSP_NEWBROWSER)) == SBSP_SAMEBROWSER)
		{
			CBrowseViewData* pData = (CBrowseViewData*) ::CoTaskMemAlloc(sizeof(CBrowseViewData));
			pData->bServer = false;
			pData->bFilePath = false;
			pData->pidl = (PIDLIST_RELATIVE) ::DuplicateItemIDList(pidl);
			pData->wFlags = wFlags;
			::PostMessage(This()->m_hWnd, MY_WM_BROWSE_VIEW, 0, (LPARAM) pData);
			return S_OK;
		}
	}
	return E_NOTIMPL;
}

STDMETHODIMP CMainWindow::CBrowserLocal::GetViewStateStream(DWORD grfMode, IStream** ppStrm)
{
	//*ppStrm = theApp.m_pStreamViewStateLocal;
	//theApp.m_pStreamViewStateLocal->AddRef();
	//return S_OK;
	return theApp.GetViewStateStream(false, This()->m_wndListViewLocal.m_lpidlAbsoluteMe, grfMode, ppStrm);
}

STDMETHODIMP CMainWindow::CBrowserLocal::QueryActiveShellView(IShellView** ppshv)
{
	CMainWindow* pThis = This();
	//if (pThis->m_wndListViewServer.m_pView)
	//	return pThis->m_wndListViewServer.m_pView->QueryInterface(IID_IShellView, (void**) ppshv);
	//*ppshv = NULL;
	return pThis->QueryActiveShellView(false, ppshv);
}

STDMETHODIMP CMainWindow::CBrowserLocal::OnViewWindowActive(IShellView* pshv)
{
	//::SetActiveWindow(This()->m_hWnd);
	This()->OnViewWindowActive(false, pshv);
	return S_OK;
}

STDMETHODIMP CMainWindow::CBrowserServer::BrowseObject(PCUIDLIST_RELATIVE pidl, UINT wFlags)
{
	if ((wFlags & (SBSP_OPENMODE | SBSP_EXPLOREMODE | SBSP_HELPMODE)) == SBSP_OPENMODE ||
		(wFlags & (SBSP_OPENMODE | SBSP_EXPLOREMODE | SBSP_HELPMODE)) == SBSP_DEFMODE)
	{
		if ((wFlags & (SBSP_SAMEBROWSER | SBSP_NEWBROWSER)) == SBSP_DEFBROWSER ||
			(wFlags & (SBSP_SAMEBROWSER | SBSP_NEWBROWSER)) == SBSP_SAMEBROWSER)
		{
			CBrowseViewData* pData = (CBrowseViewData*) ::CoTaskMemAlloc(sizeof(CBrowseViewData));
			pData->bServer = true;
			pData->bFilePath = false;
			pData->pidl = (PIDLIST_RELATIVE) ::DuplicateItemIDList(pidl);
			pData->wFlags = wFlags;
			::PostMessage(This()->m_hWnd, MY_WM_BROWSE_VIEW, 0, (LPARAM) pData);
			return S_OK;
		}
	}
	return E_NOTIMPL;
}

STDMETHODIMP CMainWindow::CBrowserServer::GetViewStateStream(DWORD grfMode, IStream** ppStrm)
{
	//*ppStrm = theApp.m_pStreamViewStateServer;
	//theApp.m_pStreamViewStateServer->AddRef();
	//return S_OK;
	return theApp.GetViewStateStream(true, This()->m_wndListViewServer.m_lpidlAbsoluteMe, grfMode, ppStrm);
}

STDMETHODIMP CMainWindow::CBrowserServer::QueryActiveShellView(IShellView** ppshv)
{
	CMainWindow* pThis = This();
	//if (pThis->m_wndListViewServer.m_pView)
	//	return pThis->m_wndListViewServer.m_pView->QueryInterface(IID_IShellView, (void**) ppshv);
	//*ppshv = NULL;
	//return S_OK;
	return pThis->QueryActiveShellView(true, ppshv);
}

STDMETHODIMP CMainWindow::CBrowserServer::OnViewWindowActive(IShellView* pshv)
{
	//::SetActiveWindow(This()->m_hWnd);
	This()->OnViewWindowActive(true, pshv);
	return S_OK;
}

#ifdef _EASYSFTP_USE_ICOMMDLGBROWSER

static HRESULT __stdcall GetSelectedIDList(IShellView* ppshv, int nIndex, int* pnCount, PIDLIST_ABSOLUTE* ppidlRet)
{
	CMyStringW str;
	FORMATETC fmt;
	STGMEDIUM stg;
	IDataObject* pObject;
	HRESULT hr;
	PIDLIST_RELATIVE pidl;
	PIDLIST_ABSOLUTE pidl2;
	HWND hWnd;
	IShellFolder* pDesktopFolder;

	hr = ppshv->GetWindow(&hWnd);
	if (FAILED(hr))
		hWnd = NULL;

	hr = ppshv->GetItemObject(SVGIO_SELECTION, IID_IDataObject, (void**) &pObject);
	if (FAILED(hr))
		return hr;
	fmt.cfFormat = theApp.m_nCFShellIDList;
	fmt.dwAspect = DVASPECT_CONTENT;
	fmt.lindex = -1;
	fmt.ptd = NULL;
	fmt.tymed = TYMED_HGLOBAL;
	hr = pObject->GetData(&fmt, &stg);
	if (SUCCEEDED(hr))
	{
		LPIDA lp = (LPIDA) ::GlobalLock(stg.hGlobal);
		if (lp->cidl)
		{
			pidl2 = (PIDLIST_ABSOLUTE) (((LPBYTE) lp) + lp->aoffset[0]);
			pidl = (PIDLIST_RELATIVE) (((LPBYTE) lp) + lp->aoffset[nIndex + 1]);
			if (ppidlRet)
				pidl2 = AppendItemIDList(pidl2, pidl);
		}
		else
			pidl2 = NULL;
		if (pnCount)
			*pnCount = lp->cidl;
		::GlobalUnlock(stg.hGlobal);
		::ReleaseStgMedium(&stg);
		pObject->Release();
	}
	else
	{
		int nCount;
		fmt.cfFormat = CF_HDROP;
		hr = pObject->GetData(&fmt, &stg);
		if (FAILED(hr))
		{
			pObject->Release();
			return hr;
		}
		nCount = (int) ::DragQueryFileA((HDROP) stg.hGlobal, (UINT) nIndex, NULL, 0);
		if (nCount)
		{
			if (!::DragQueryFileW((HDROP) stg.hGlobal, (UINT) nIndex, str.GetBuffer(MAX_PATH), MAX_PATH))
			{
				::DragQueryFileA((HDROP) stg.hGlobal, (UINT) nIndex, str.GetBufferA(MAX_PATH), MAX_PATH);
				str.ReleaseBufferA();
			}
			else
				str.ReleaseBuffer();
		}
		else
			pidl2 = NULL;
		if (pnCount)
			*pnCount = nCount;
		::ReleaseStgMedium(&stg);
		pObject->Release();
		if (ppidlRet && nCount)
		{
			if (str.IsEmpty())
				return E_FAIL;
			hr = ::SHGetDesktopFolder(&pDesktopFolder);
			if (FAILED(hr))
				return hr;
			hr = pDesktopFolder->ParseDisplayName(hWnd, NULL,
				(LPWSTR)(LPCWSTR) str, NULL, &pidl, NULL);
			pDesktopFolder->Release();
			if (FAILED(hr))
				return hr;
			// デスクトップのRelative == Absolute
			pidl2 = (PIDLIST_ABSOLUTE) pidl;
		}
	}
	if (ppidlRet)
		*ppidlRet = pidl2;
	return S_OK;
}

STDMETHODIMP CMainWindow::CBrowserLocal::OnDefaultCommand(IShellView* ppshv)
{
	HRESULT hr;
	PIDLIST_ABSOLUTE pidl;
	int nc;

	hr = GetSelectedIDList(ppshv, 0, &nc, NULL);
	if (FAILED(hr))
		return hr;
	if (nc != 1)
		return S_OK;
	hr = GetSelectedIDList(ppshv, 0, NULL, &pidl);
	if (FAILED(hr))
		return hr;
	This()->UpdateCurrentFolderAbsolute(pidl);
	::CoTaskMemFree(pidl);
	return S_OK;
}

STDMETHODIMP CMainWindow::CBrowserLocal::OnStateChange(IShellView* ppshv, ULONG uChange)
{
	CMainWindow* pThis = This();
	switch (uChange)
	{
		case CDBOSC_SETFOCUS:
			pThis->m_wndListViewLocal.OnFocusView(true);
			break;
		case CDBOSC_KILLFOCUS:
			pThis->m_wndListViewLocal.OnFocusView(false);
			break;
		//case CDBOSC_SELCHANGE:
		//{
		//	HRESULT hr;
		//	PIDLIST_ABSOLUTE pidl;
		//	int nc;
		//	CMyStringW str;

		//	hr = GetSelectedIDList(ppshv, 0, &nc, &pidl);
		//	if (FAILED(hr))
		//		return hr;
		//	if (!nc)
		//		break;
		//	if (!::SHGetPathFromIDListW(pidl, str.GetBuffer(MAX_PATH)))
		//	{
		//		::SHGetPathFromIDListA(pidl, str.GetBufferA(MAX_PATH));
		//		str.ReleaseBufferA();
		//	}
		//	else
		//		str.ReleaseBuffer();
		//	::CoTaskMemFree(pidl);
		//	pThis->SetWindowTextW(str);
		//}
		//break;
	}
	return S_OK;
}

STDMETHODIMP CMainWindow::CBrowserLocal::IncludeObject(IShellView* ppshv, PCUITEMID_CHILD pidl)
{
	PIDLIST_ABSOLUTE lp;
	CMainWindow* pThis = This();

	lp = AppendItemIDList(pThis->m_wndListViewLocal.m_lpidlAbsoluteMeCur, pidl);
	if (IsMyComputerIDList(lp))
	{
		::CoTaskMemFree(lp);
		return S_OK;
	}
	::CoTaskMemFree(lp);
	//if (pThis->m_wndListViewLocal.m_pView == ppshv)
	{
		SFGAOF rgf;
		HRESULT hr;
		hr = pThis->m_wndListViewLocal.m_pFolderCur->GetAttributesOf(1, &pidl, &rgf);
		//if (rgf & (SFGAO_FILESYSTEM))
			return S_OK;
		//else
		//	return S_FALSE;
	}
	return S_OK;
}

STDMETHODIMP CMainWindow::CBrowserServer::OnDefaultCommand(IShellView* ppshv)
{
	HRESULT hr;
	PIDLIST_ABSOLUTE pidl;
	int nc;

	hr = GetSelectedIDList(ppshv, 0, &nc, NULL);
	if (FAILED(hr))
		return hr;
	if (nc != 1)
		return S_OK;
	hr = GetSelectedIDList(ppshv, 0, NULL, &pidl);
	if (FAILED(hr))
		return hr;
	This()->UpdateServerFolderAbsolute(pidl);
	::CoTaskMemFree(pidl);
	return S_OK;
}

STDMETHODIMP CMainWindow::CBrowserServer::OnStateChange(IShellView* ppshv, ULONG uChange)
{
	CMainWindow* pThis = This();
	switch (uChange)
	{
		case CDBOSC_SETFOCUS:
			pThis->m_wndListViewServer.OnFocusView(true);
			break;
		case CDBOSC_KILLFOCUS:
			pThis->m_wndListViewServer.OnFocusView(false);
			break;
		//case CDBOSC_SELCHANGE:
		//{
		//	HRESULT hr;
		//	PIDLIST_ABSOLUTE pidl;
		//	int nc;
		//	CMyStringW str;

		//	hr = GetSelectedIDList(ppshv, 0, &nc, &pidl);
		//	if (FAILED(hr))
		//		return hr;
		//	if (!nc)
		//		break;
		//	if (!::SHGetPathFromIDListW(pidl, str.GetBuffer(MAX_PATH)))
		//	{
		//		::SHGetPathFromIDListA(pidl, str.GetBufferA(MAX_PATH));
		//		str.ReleaseBufferA();
		//	}
		//	else
		//		str.ReleaseBuffer();
		//	::CoTaskMemFree(pidl);
		//	pThis->SetWindowTextW(str);
		//}
		//break;
	}
	return S_OK;
}

STDMETHODIMP CMainWindow::CBrowserServer::IncludeObject(IShellView* ppshv, PCUITEMID_CHILD pidl)
{
	PIDLIST_ABSOLUTE lp;
	CMainWindow* pThis = This();

	lp = AppendItemIDList(pThis->m_wndListViewServer.m_lpidlAbsoluteMeCur, pidl);
	if (IsMyComputerIDList(lp))
	{
		::CoTaskMemFree(lp);
		return S_OK;
	}
	::CoTaskMemFree(lp);
	//if (pThis->m_wndListViewServer.m_pView == ppshv)
	{
		SFGAOF rgf;
		HRESULT hr;
		hr = pThis->m_wndListViewServer.m_pFolderCur->GetAttributesOf(1, &pidl, &rgf);
		//if (rgf & (SFGAO_FILESYSTEM))
			return S_OK;
		//else
		//	return S_FALSE;
	}
	return S_OK;
}

#endif
