/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 LFileVw.cpp - implementations of CShellFolderFileView
 */

#include "StdAfx.h"
#include "EasySFTP.h"
#include "LFileVw.h"

#include "IDList.h"

void __stdcall _StartHook(CMyWindow* pWnd);
bool __stdcall _EndHook();

CShellFolderFileView::CShellFolderFileView()
{
	m_pBrowser = NULL;
	m_pFolder = NULL;
	m_pView = NULL;
	m_lpidlAbsoluteMe = NULL;
	m_pDirectory = NULL;
	m_fs.ViewMode = FVM_DETAILS;
	m_fs.fFlags = FWF_SHOWSELALWAYS | FWF_NOWEBVIEW;
	m_bFocused = false;
	m_bReplacing = false;
}

CShellFolderFileView::~CShellFolderFileView()
{
	ReleaseAll();
}

void CShellFolderFileView::ReleaseAll()
{
	if (m_lpidlAbsoluteMe)
	{
		::CoTaskMemFree(m_lpidlAbsoluteMe);
		m_lpidlAbsoluteMe = NULL;
	}
	if (m_pDirectory)
	{
		m_pDirectory->Release();
		m_pDirectory = NULL;
	}
	//if (m_pBrowser)
	//	m_pBrowser->Release();
	if (m_pView)
	{
		m_pView->Release();
		m_pView = NULL;
	}
	if (m_pFolder)
	{
		m_pFolder->Release();
		m_pFolder = NULL;
	}
}

HWND CShellFolderFileView::Create(IShellBrowser* pBrowser, HWND hWndParent)
{
	IShellFolder* pFolder;
	HRESULT hr;
	HWND hWnd;
	PIDLIST_ABSOLUTE lpItemID;

	hr = ::SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &lpItemID);
	if (FAILED(hr))
		return NULL;
	hr = ::SHGetDesktopFolder(&pFolder);
	if (FAILED(hr))
	{
		::CoTaskMemFree(lpItemID);
		return NULL;
	}
	hWnd = Create(lpItemID, pFolder, pBrowser, hWndParent);
	pFolder->Release();
	::CoTaskMemFree(lpItemID);
	return hWnd;
}

HWND CShellFolderFileView::Create(PCIDLIST_ABSOLUTE lpItemID, IShellFolder* pFolder, IShellBrowser* pBrowser, HWND hWndParent)
{
	IShellView* pView;
	HRESULT hr;
	HWND hWnd;
	RECT rc;

	m_bReplacing = true;
	hr = pFolder->CreateViewObject(hWndParent, IID_IShellView, (void**) &pView);
	if (FAILED(hr))
	{
		m_bReplacing = false;
		return NULL;
	}
	rc.left = rc.top = 0;
	rc.right = rc.bottom = 100;
	m_lpidlAbsoluteMe = (PIDLIST_ABSOLUTE) ::DuplicateItemIDList(lpItemID);
	pFolder->AddRef();
	hr = pView->CreateViewWindow(NULL, &m_fs, pBrowser, &rc, &hWnd);
	if (FAILED(hr))
	{
		pView->Release();
		pFolder->Release();
		m_bReplacing = false;
		return NULL;
	}
	if (!Attach(hWnd))
	{
		pView->DestroyViewWindow();
		pView->Release();
		pFolder->Release();
		m_bReplacing = false;
		return NULL;
	}
	hr = pView->UIActivate(SVUIA_ACTIVATE_NOFOCUS);
	m_pBrowser = pBrowser;
	//pBrowser->AddRef();
	m_pView = pView;
	m_pFolder = pFolder;
	if (FAILED(pFolder->QueryInterface(IID_IEasySFTPDirectory, (void**) &m_pDirectory)))
		m_pDirectory = NULL;
	m_bReplacing = false;
	return hWnd;
}

void CShellFolderFileView::OnFocusView(bool bFocus)
{
	m_bFocused = bFocus;
}

HRESULT CShellFolderFileView::ReplaceView(PCUIDLIST_RELATIVE lpItemID)
{
	IShellFolder* pFolder;
	HRESULT hr;
	PIDLIST_ABSOLUTE lp;

	lp = AppendItemIDList(m_lpidlAbsoluteMe, lpItemID);
	if (!lp)
		return E_OUTOFMEMORY;
	hr = m_pFolder->BindToObject(lpItemID, NULL, IID_IShellFolder, (void**) &pFolder);
	if (FAILED(hr))
	{
		::CoTaskMemFree(lp);
		return hr;
	}
	hr = ReplaceView(pFolder);
	pFolder->Release();
	if (SUCCEEDED(hr))
	{
		::CoTaskMemFree(m_lpidlAbsoluteMe);
		m_lpidlAbsoluteMe = lp;
	}
	else
		::CoTaskMemFree(lp);
	return hr;
}

HRESULT CShellFolderFileView::ReplaceViewAbsolute(PCUIDLIST_ABSOLUTE lpItemID)
{
	IShellFolder* pFolder, * pDesktop;
	HRESULT hr;
	PIDLIST_ABSOLUTE lp;

	lp = (PIDLIST_ABSOLUTE) DuplicateItemIDList(lpItemID);
	if (!lp)
		return E_OUTOFMEMORY;
	hr = ::SHGetDesktopFolder(&pDesktop);
	if (!IsRootIDList(lpItemID))
	{
		hr = pDesktop->BindToObject(lpItemID, NULL, IID_IShellFolder, (void**) &pFolder);
		pDesktop->Release();
	}
	else
		pFolder = pDesktop;
	if (FAILED(hr))
		return hr;
	hr = ReplaceView(pFolder);
	pFolder->Release();
	if (SUCCEEDED(hr))
	{
		::CoTaskMemFree(m_lpidlAbsoluteMe);
		m_lpidlAbsoluteMe = lp;
	}
	else
		::CoTaskMemFree(lp);
	return hr;
}

HRESULT CShellFolderFileView::ReplaceViewAbsolute(PCUIDLIST_ABSOLUTE lpItemID, IShellFolder* pFolder)
{
	HRESULT hr;
	PIDLIST_ABSOLUTE lp;

	lp = (PIDLIST_ABSOLUTE) DuplicateItemIDList(lpItemID);
	if (!lp)
		return E_OUTOFMEMORY;
	hr = ReplaceView(pFolder);
	if (SUCCEEDED(hr))
	{
		::CoTaskMemFree(m_lpidlAbsoluteMe);
		m_lpidlAbsoluteMe = lp;
	}
	else
		::CoTaskMemFree(lp);
	return hr;
}

HRESULT CShellFolderFileView::ReplaceView(IShellFolder* pFolder)
{
	IShellView* pView;
	HRESULT hr;
	RECT rc;
	HWND hWnd, hWndParent, hWndOld;
	bool bFocus;

	if (m_bReplacing)
	{
		::MessageBeep(0);
		return E_PENDING;
	}

	m_bReplacing = true;
	bFocus = m_bFocused;
	hWndParent = ::GetParent(m_hWnd);
	pView = NULL;
	hr = pFolder->CreateViewObject(hWndParent, IID_IShellView, (void**) &pView);
	if (FAILED(hr) || !pView)
	{
		m_bReplacing = false;
		return hr;
	}
	hWndOld = Detach();
	::GetWindowRect(hWndOld, &rc);
	::ScreenToClient(hWndParent, (LPPOINT) &rc);
	::ScreenToClient(hWndParent, ((LPPOINT) &rc) + 1);
	hr = pView->CreateViewWindow(m_pView, &m_fs, m_pBrowser, &rc, &hWnd);
	if (FAILED(hr))
	{
		pView->Release();
		Attach(hWndOld);
		m_pView->UIActivate(bFocus ? SVUIA_ACTIVATE_FOCUS : SVUIA_ACTIVATE_NOFOCUS);
		m_bReplacing = false;
		return hr;
	}
	if (!Attach(hWnd))
	{
		pView->DestroyViewWindow();
		pView->Release();
		Attach(hWndOld);
		m_pView->UIActivate(bFocus ? SVUIA_ACTIVATE_FOCUS : SVUIA_ACTIVATE_NOFOCUS);
		m_bReplacing = false;
		return hr;
	}
	hr = m_pView->UIActivate(SVUIA_DEACTIVATE);
	hr = m_pView->DestroyViewWindow();
	if (FAILED(hr))
	{
		pView->DestroyViewWindow();
		pView->Release();
		Subclass(hWndOld);
		m_pView->UIActivate(bFocus ? SVUIA_ACTIVATE_FOCUS : SVUIA_ACTIVATE_NOFOCUS);
		m_bReplacing = false;
		return hr;
	}
	hr = pView->UIActivate(bFocus ? SVUIA_ACTIVATE_FOCUS : SVUIA_ACTIVATE_NOFOCUS);
	if (bFocus)
		::SetFocus(m_hWnd);
	m_pView->Release();
	m_pView = pView;
	m_pFolder->Release();
	m_pFolder = pFolder;
	pFolder->AddRef();
	if (m_pDirectory)
		m_pDirectory->Release();
	if (FAILED(pFolder->QueryInterface(IID_IEasySFTPDirectory, (void**) &m_pDirectory)))
		m_pDirectory = NULL;
	m_bReplacing = false;
	return S_OK;
}

void CShellFolderFileView::Refresh()
{
	m_pView->Refresh();
}

bool CShellFolderFileView::SendCommandString(HWND hWndBrowser, UINT uItem, LPCWSTR lpszCommand)
{
	HRESULT hr;
	IContextMenu* pMenu;
	hr = m_pView->GetItemObject(uItem, IID_IContextMenu, (void**) &pMenu);
	if (SUCCEEDED(hr))
	{
		HMENU hMenuDummy = ::CreatePopupMenu();
		hr = pMenu->QueryContextMenu(hMenuDummy, 0, 0, 0xFFFF, CMF_CANRENAME);
		if (SUCCEEDED(hr))
		{
			CMyStringW strCommand(lpszCommand);
			CMINVOKECOMMANDINFOEX ci;
			memset(&ci, 0, sizeof(ci));
			ci.cbSize = sizeof(ci);
			ci.nShow = SW_SHOWNORMAL;
			ci.fMask = CMIC_MASK_UNICODE;
			ci.lpVerb = strCommand;
			ci.lpVerbW = lpszCommand;
			ci.hwnd = hWndBrowser;
			hr = pMenu->InvokeCommand((CMINVOKECOMMANDINFO*) &ci);
		}
		::DestroyMenu(hMenuDummy);
		pMenu->Release();
	}
	return SUCCEEDED(hr);
}

void CShellFolderFileView::DoCreateNewFolder(HWND hWndBrowser)
{
	IFolderView* pFView;
	HRESULT hr;
	int nCount;
	hr = m_pView->QueryInterface(IID_IFolderView, (void**) &pFView);
	if (SUCCEEDED(hr))
	{
		hr = pFView->ItemCount(SVGIO_ALLVIEW, &nCount);
		for (int i = 0; i < nCount; i++)
			hr = pFView->SelectItem(i, SVSI_DESELECT);
	}
	else
		pFView = NULL;
	if (SendCommandString(hWndBrowser, SVGIO_BACKGROUND, CMDSTR_NEWFOLDERW))
	{
		if (pFView)
		{
			hr = pFView->ItemCount(SVGIO_ALLVIEW, &nCount);
			hr = pFView->SelectItem(nCount - 1, SVSI_EDIT);
			pFView->Release();
		}
	}
}

void CShellFolderFileView::DoOpen(HWND hWndBrowser)
{
	int nCount;
	PIDLIST_ABSOLUTE* ppidl;
	ppidl = GetAllSelection(&nCount);
	if (ppidl)
	{
		SHELLEXECUTEINFOA sei;
		sei.cbSize = sizeof(sei);
		sei.fMask = SEE_MASK_IDLIST;
		sei.lpVerb = NULL;
		sei.lpFile = NULL;
		sei.lpParameters = NULL;
		sei.lpDirectory = NULL;
		sei.nShow = SW_SHOWNORMAL;
		sei.hwnd = hWndBrowser;
		for (int i = 0; i < nCount; i++)
		{
			sei.lpIDList = ppidl[i];
			::ShellExecuteExA(&sei);
			::CoTaskMemFree(ppidl[i]);
		}
	}
}

void CShellFolderFileView::DoCreateShortcut(HWND hWndBrowser)
{
	::SendMessage(m_hWnd, WM_COMMAND, MAKEWPARAM(28688, 0), (LPARAM) 0);
}

void CShellFolderFileView::DoCopy(HWND hWndBrowser, bool bCut)
{
	::SendMessage(m_hWnd, WM_COMMAND, MAKEWPARAM(bCut ? 28696 : 28697, 0), (LPARAM) 0);
	//SendCommandString(hWndBrowser, SVGIO_SELECTION, L"Delete");
}

void CShellFolderFileView::DoPaste(HWND hWndBrowser)
{
	::SendMessage(m_hWnd, WM_COMMAND, MAKEWPARAM(28698, 0), (LPARAM) 0);
	//SendCommandString(hWndBrowser, SVGIO_SELECTION, L"Delete");
}

void CShellFolderFileView::DoDelete(HWND hWndBrowser)
{
	::SendMessage(m_hWnd, WM_COMMAND, MAKEWPARAM(28689, 0), (LPARAM) 0);
	//SendCommandString(hWndBrowser, SVGIO_SELECTION, L"Delete");
}

void CShellFolderFileView::DoRename(HWND hWndBrowser)
{
	::SendMessage(m_hWnd, WM_COMMAND, MAKEWPARAM(28690, 0), (LPARAM) 0);
	//HRESULT hr;
	//IDataObject* pObject;
	//hr = m_pView->GetItemObject(SVGIO_SELECTION, IID_IDataObject, (void**) &pObject);
	//if (SUCCEEDED(hr))
	//{
	//	FORMATETC fmt;
	//	STGMEDIUM stg;
	//	fmt.cfFormat = theApp.m_nCFShellIDList;
	//	fmt.dwAspect = DVASPECT_CONTENT;
	//	fmt.tymed = TYMED_HGLOBAL;
	//	fmt.lindex = -1;
	//	fmt.ptd = NULL;
	//	hr = pObject->GetData(&fmt, &stg);
	//	if (SUCCEEDED(hr))
	//	{
	//		if (stg.tymed == TYMED_HGLOBAL)
	//		{
	//			LPIDA lp = (LPIDA) ::GlobalLock(stg.hGlobal);
	//			if (lp->cidl > 0)
	//			{
	//				PITEMID_CHILD pChildFirst = (PITEMID_CHILD) (((LPBYTE) lp) + lp->aoffset[1]);
	//				m_pView->SelectItem(pChildFirst, SVSI_EDIT);
	//			}
	//			::GlobalUnlock(stg.hGlobal);
	//		}
	//		::ReleaseStgMedium(&stg);
	//	}
	//	pObject->Release();
	//}
}

void CShellFolderFileView::DoProperty(HWND hWndBrowser)
{
	::SendMessage(m_hWnd, WM_COMMAND, MAKEWPARAM(28691, 0), (LPARAM) 0);
}

void CShellFolderFileView::DoSelectAll(HWND hWndBrowser)
{
	::SendMessage(m_hWnd, WM_COMMAND, MAKEWPARAM(28705, 0), (LPARAM) 0);
}

void CShellFolderFileView::DoInvertSelection(HWND hWndBrowser)
{
	::SendMessage(m_hWnd, WM_COMMAND, MAKEWPARAM(28706, 0), (LPARAM) 0);
}

int CShellFolderFileView::GetSelectionCount()
{
	IDataObject* pData;
	int nRet;
	FORMATETC fmt;
	STGMEDIUM stg;
	if (FAILED(m_pView->GetItemObject(SVGIO_SELECTION, IID_IDataObject, (void**) &pData)))
		return 0;
	fmt.tymed = TYMED_HGLOBAL;
	fmt.cfFormat = theApp.m_nCFShellIDList;
	fmt.lindex = -1;
	fmt.dwAspect = DVASPECT_CONTENT;
	fmt.ptd = NULL;
	if (SUCCEEDED(pData->GetData(&fmt, &stg)))
	{
		if (stg.tymed == TYMED_HGLOBAL)
		{
			CIDA* p = (CIDA*) ::GlobalLock(stg.hGlobal);
			nRet = p->cidl;
			::GlobalUnlock(stg.hGlobal);
		}
		else
			nRet = 0;
		::ReleaseStgMedium(&stg);
	}
	else
	{
		fmt.cfFormat = CF_HDROP;
		if (SUCCEEDED(pData->GetData(&fmt, &stg)))
		{
			if (stg.tymed == TYMED_HGLOBAL)
				nRet = (int) ::DragQueryFileA((HDROP) stg.hGlobal, (UINT) 0, NULL, 0);
			else
				nRet = 0;
			::ReleaseStgMedium(&stg);
		}
		else
			nRet = 0;
	}
	pData->Release();
	return nRet;
}

PIDLIST_ABSOLUTE CShellFolderFileView::GetSelectedItem(int iIndex)
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

	hr = m_pView->GetWindow(&hWnd);
	if (FAILED(hr))
		hWnd = NULL;

	hr = m_pView->GetItemObject(SVGIO_SELECTION, IID_IDataObject, (void**) &pObject);
	if (FAILED(hr))
		return NULL;
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
			pidl = (PIDLIST_RELATIVE) (((LPBYTE) lp) + lp->aoffset[iIndex + 1]);
			pidl2 = AppendItemIDList(pidl2, pidl);
		}
		else
			pidl2 = NULL;
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
			return NULL;
		}
		nCount = (int) ::DragQueryFileA((HDROP) stg.hGlobal, (UINT) iIndex, NULL, 0);
		if (nCount)
		{
			if (!::DragQueryFileW((HDROP) stg.hGlobal, (UINT) iIndex, str.GetBuffer(MAX_PATH), MAX_PATH))
			{
				::DragQueryFileA((HDROP) stg.hGlobal, (UINT) iIndex, str.GetBufferA(MAX_PATH), MAX_PATH);
				str.ReleaseBufferA();
			}
			else
				str.ReleaseBuffer();
		}
		else
			pidl2 = NULL;
		::ReleaseStgMedium(&stg);
		pObject->Release();
		if (nCount)
		{
			if (str.IsEmpty())
				return NULL;
			hr = ::SHGetDesktopFolder(&pDesktopFolder);
			if (FAILED(hr))
				return NULL;
			hr = pDesktopFolder->ParseDisplayName(hWnd, NULL,
				(LPWSTR)(LPCWSTR) str, NULL, &pidl, NULL);
			pDesktopFolder->Release();
			if (FAILED(hr))
				return NULL;
			// デスクトップのRelative == Absolute
			pidl2 = (PIDLIST_ABSOLUTE) pidl;
		}
	}
	return pidl2;
}

PIDLIST_ABSOLUTE* CShellFolderFileView::GetAllSelection(int* pnCount)
{
	CMyStringW str;
	FORMATETC fmt;
	STGMEDIUM stg;
	IDataObject* pObject;
	HRESULT hr;
	PIDLIST_RELATIVE pidl;
	PIDLIST_ABSOLUTE pidl2;
	PIDLIST_ABSOLUTE* ppRet;
	HWND hWnd;
	IShellFolder* pDesktopFolder;

	hr = m_pView->GetWindow(&hWnd);
	if (FAILED(hr))
		hWnd = NULL;

	hr = m_pView->GetItemObject(SVGIO_SELECTION, IID_IDataObject, (void**) &pObject);
	if (FAILED(hr))
	{
		if (pnCount)
			*pnCount = 0;
		return NULL;
	}
	fmt.cfFormat = theApp.m_nCFShellIDList;
	fmt.dwAspect = DVASPECT_CONTENT;
	fmt.lindex = -1;
	fmt.ptd = NULL;
	fmt.tymed = TYMED_HGLOBAL;
	hr = pObject->GetData(&fmt, &stg);
	if (SUCCEEDED(hr))
	{
		LPIDA lp = (LPIDA) ::GlobalLock(stg.hGlobal);
		if (pnCount)
			*pnCount = lp->cidl;
		if (lp->cidl)
		{
			pidl2 = (PIDLIST_ABSOLUTE) (((LPBYTE) lp) + lp->aoffset[0]);
			ppRet = (PIDLIST_ABSOLUTE*) malloc(sizeof(PIDLIST_ABSOLUTE) * lp->cidl);
			for (UINT i = 0; i < lp->cidl; i++)
			{
				pidl = (PIDLIST_RELATIVE) (((LPBYTE) lp) + lp->aoffset[i + 1]);
				ppRet[i] = AppendItemIDList(pidl2, pidl);
			}
		}
		else
			ppRet = NULL;
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
			if (pnCount)
				*pnCount = 0;
			return NULL;
		}
		nCount = (int) ::DragQueryFileA((HDROP) stg.hGlobal, (UINT) 0, NULL, 0);
		if (nCount)
		{
			hr = ::SHGetDesktopFolder(&pDesktopFolder);
			if (SUCCEEDED(hr))
			{
				ppRet = (PIDLIST_ABSOLUTE*) malloc(sizeof(PIDLIST_ABSOLUTE) * nCount);
				for (int i = 0; i < nCount; i++)
				{
					if (!::DragQueryFileW((HDROP) stg.hGlobal, (UINT) i, str.GetBuffer(MAX_PATH), MAX_PATH))
					{
						::DragQueryFileA((HDROP) stg.hGlobal, (UINT) i, str.GetBufferA(MAX_PATH), MAX_PATH);
						str.ReleaseBufferA();
					}
					else
						str.ReleaseBuffer();
					if (str.IsEmpty())
					{
						i--;
						nCount--;
						continue;
					}
					hr = pDesktopFolder->ParseDisplayName(hWnd, NULL,
						(LPWSTR)(LPCWSTR) str, NULL, &pidl, NULL);
					if (SUCCEEDED(hr))
					{
						// デスクトップのRelative == Absolute
						ppRet[i] = (PIDLIST_ABSOLUTE) pidl;
					}
					else
					{
						i--;
						nCount--;
					}
				}
				pDesktopFolder->Release();
				if (!nCount)
				{
					free(ppRet);
					ppRet = NULL;
				}
			}
		}
		else
			ppRet = NULL;
		if (pnCount)
			*pnCount = nCount;
		::ReleaseStgMedium(&stg);
		pObject->Release();
	}
	return ppRet;
}
