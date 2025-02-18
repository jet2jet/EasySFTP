/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 Folder.cpp - implementations of CFolderBase
 */

#include "StdAfx.h"
#include "ShellDLL.h"
#include "FoldBase.h"

extern "C" SHGDNF __stdcall ConvertDisplayNameFlags(SIGDN flags)
{
	switch (flags)
	{
		case SIGDN_NORMALDISPLAY:
		default:
			return SHGDN_NORMAL;
		case SIGDN_PARENTRELATIVEPARSING:
			return SHGDN_FORPARSING | SHGDN_INFOLDER;
		case SIGDN_DESKTOPABSOLUTEPARSING:
			return SHGDN_FORPARSING | SHGDN_NORMAL;
		case SIGDN_PARENTRELATIVEEDITING:
			return SHGDN_FOREDITING | SHGDN_INFOLDER;
		case SIGDN_DESKTOPABSOLUTEEDITING:
			return SHGDN_FOREDITING | SHGDN_NORMAL;
		case SIGDN_FILESYSPATH:
		case SIGDN_URL:
			return SHGDN_NORMAL;
		case SIGDN_PARENTRELATIVEFORADDRESSBAR:
			return SHGDN_FORADDRESSBAR | SHGDN_INFOLDER;
		case SIGDN_PARENTRELATIVE:
			return SHGDN_INFOLDER;
	}
}

////////////////////////////////////////////////////////////////////////////////

CFolderBase::CFolderBase(CDelegateMallocData* pMallocData)
	: m_pMallocData(pMallocData)
{
	m_pMallocData->AddRef();
	m_pidlMe = NULL;
	m_hWndOwnerCache = NULL;
	m_pUnkSite = NULL;
	::InitializeCriticalSection(&m_csPidlMe);
}

CFolderBase::~CFolderBase()
{
	::DeleteCriticalSection(&m_csPidlMe);
	if (m_pUnkSite)
	{
		m_pUnkSite->Release();
//#ifdef _DEBUG
//		m_pUnkSite = NULL;
//#endif
	}
	if (m_pidlMe)
		::CoTaskMemFree(m_pidlMe);
	m_pMallocData->Release();
//#ifdef _DEBUG
//	m_pMallocData = NULL;
//#endif
}

STDMETHODIMP CFolderBase::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IShellFolder) ||
		IsEqualIID(riid, IID_IShellFolder2))
		*ppv = (IShellFolder2*) this;
	else if (IsEqualIID(riid, IID_IPersist) ||
		IsEqualIID(riid, IID_IPersistFolder) ||
		IsEqualIID(riid, IID_IPersistFolder2))
		*ppv = (IPersistFolder2*) this;
	else if (IsEqualIID(riid, IID_IPersistIDList))
		*ppv = (IPersistIDList*) this;
	else if (IsEqualIID(riid, IID_IObjectWithSite))
		*ppv = (IObjectWithSite*) this;
	else if (IsEqualIID(riid, IID_IShellIcon))
		*ppv = (IShellIcon*) this;
	else if (IsEqualIID(riid, IID_IShellFolderViewCB))
		*ppv = (IShellFolderViewCB*) this;
	else
	{
		// NOTE: we must set NULL to *ppv, or causes 0xC000041D exception in Windows 7 (x64)
		*ppv = NULL;
#ifdef _DEBUG
		CMyStringW str;
		str.Format(L"CFolderBase::QueryInterface: unknown interface: {%08lX-%04hX-%04hX-%02X%02X-%02X%02X%02X%02X%02X%02X}\n",
			riid.Data1, riid.Data2, riid.Data3, (UINT) riid.Data4[0], (UINT) riid.Data4[1],
			(UINT) riid.Data4[2], (UINT) riid.Data4[3], (UINT) riid.Data4[4], (UINT) riid.Data4[5],
			(UINT) riid.Data4[6], (UINT) riid.Data4[7]);
		OutputDebugString(str);
#endif
		return E_NOINTERFACE;
	}
	AddRef();
	return S_OK;
}

void CFolderBase::OnAddRef()
{
}

void CFolderBase::OnRelease()
{
}

STDMETHODIMP CFolderBase::ParseDisplayName(HWND hWnd, LPBC pbc, LPWSTR pszDisplayName,
	ULONG* pchEaten, PIDLIST_RELATIVE* ppidl, ULONG* pdwAttributes)
{
	return ParseDisplayName2(NULL, hWnd, pbc, pszDisplayName, pchEaten, ppidl, pdwAttributes);
}

STDMETHODIMP CFolderBase::CreateViewObject(HWND hWndOwner, REFIID riid, void** ppv)
{
	SFV_CREATE sfv;

	if (!ppv)
		return E_POINTER;

	// NOTE: we must set NULL to *ppv, or causes 0xC000041D exception in Windows 7 (x64)
	*ppv = NULL;

	if (!IsEqualIID(riid, IID_IUnknown) &&
		!IsEqualIID(riid, IID_IOleWindow) &&
		!IsEqualIID(riid, IID_IShellView) &&
		!IsEqualIID(riid, IID_IShellView2) &&
		!IsEqualIID(riid, IID_IShellView3))
		return E_NOINTERFACE;

	sfv.cbSize = sizeof(sfv);
	sfv.pshf = this;
	sfv.psvOuter = NULL;
	sfv.psfvcb = this;
	IShellView* pv;
	// TODO: we should implement IShellFolder2 and IPersistFolder2
	HRESULT hr = ::SHCreateShellFolderView(&sfv, &pv);
	if (FAILED(hr))
		return hr;
	//CFolderViewWrapper* pWrapper = new CFolderViewWrapper(pv);
	//pv->Release();
	//hr = pWrapper->QueryInterface(riid, ppv);
	//pWrapper->Release();
	hr = pv->QueryInterface(riid, ppv);
	pv->Release();
	return hr;
}

STDMETHODIMP CFolderBase::MessageSFVCB(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case SFVM_GETNOTIFY:
			::EnterCriticalSection(&m_csPidlMe);
			*((PIDLIST_ABSOLUTE*) wParam) = m_pidlMe;
			::LeaveCriticalSection(&m_csPidlMe);
			*((LONG*) lParam) = SHCNE_ALLEVENTS;
			return S_OK;
		case SFVM_QUERYFSNOTIFY:
			::EnterCriticalSection(&m_csPidlMe);
			((SHChangeNotifyEntry*) lParam)->pidl = m_pidlMe;
			::LeaveCriticalSection(&m_csPidlMe);
			((SHChangeNotifyEntry*) lParam)->fRecursive = TRUE;
			return S_OK;
		case SFVM_FSNOTIFY:
		{
			PCUITEMID_CHILD* apidl = (PCUITEMID_CHILD*) wParam;
			PCUITEMID_CHILD pidl1 = apidl[0];
			PCUITEMID_CHILD pidl2 = NULL;
			if (lParam == SHCNE_RENAMEFOLDER || lParam == SHCNE_RENAMEITEM)
				pidl2 = apidl[1];
			else if (lParam == SHCNE_MKDIR || lParam == SHCNE_CREATE ||
				lParam == SHCNE_DRIVEADD || lParam == SHCNE_MEDIAINSERTED)
			{
				pidl2 = apidl[0];
				pidl1 = NULL;
			}
			UpdateItem(pidl1, pidl2, (LONG) lParam);
		}
		return S_OK;
		case SFVM_WINDOWCREATED:
			m_ahWndViews.Add((HWND) wParam);
			return S_OK;
		case 16: // SFVM_WINDOWCLOSING
		{
			int n = m_ahWndViews.FindItem((HWND) wParam);
			if (n >= 0)
				m_ahWndViews.RemoveItem(n);
		}
		return S_OK;
		case 102:
			*((ULONG*) wParam) = (ULONG) -1;
			return S_OK;
	}
#ifdef _DEBUG
	{
		CMyStringW str;
		str.Format(L"CFolderBase::MessageSFVCB: unknown message(%u): wParam = %p, lParam = %p\n",
			uMsg, wParam, lParam);
		OutputDebugString(str);
	}
#endif
	return E_NOTIMPL;
	//return S_OK;
}


STDMETHODIMP CFolderBase::ParseDisplayName2(PIDLIST_RELATIVE pidlParent,
	HWND hWnd,
	LPBC pbc,
	LPWSTR pszDisplayName,
	ULONG* pchEaten,
	PIDLIST_RELATIVE* ppidl,
	ULONG* pdwAttributes)
{
	//bool bSFTPMode;
	ULONG uEaten;
	if (!ppidl)
		return E_POINTER;
	m_hWndOwnerCache = hWnd;
	*ppidl = NULL;
	//if (!(bSFTPMode = !(memcmp(pszDisplayName, L"ftp://", sizeof(WCHAR) * 6) == 0)) ||
	//	(memcmp(pszDisplayName, L"sftp://", sizeof(WCHAR) * 7) == 0))
	//{
		LPWSTR lpw;
		CMyStringW strName;
		PIDLIST_RELATIVE pidlCurrent;

		pidlCurrent = NULL;
		uEaten = 0;
		while (true)
		{
			if (*pszDisplayName == L'/')
			{
				pszDisplayName++;
				uEaten++;
			}
			if (!*pszDisplayName)
				break;
			lpw = wcschr(pszDisplayName, L'/');
			if (lpw)
			{
				if (lpw == pszDisplayName)
					continue;
				strName.SetString(pszDisplayName, (DWORD) ((DWORD_PTR) lpw - (DWORD_PTR) pszDisplayName) / sizeof(WCHAR));
			}
			else
				strName = pszDisplayName;

			{
				pszDisplayName += strName.GetLength();
				uEaten += (ULONG) strName.GetLength();

				PITEMID_CHILD pidlChild;
				pidlChild = ::CreateDummyFileItem(m_pMallocData->pMalloc, strName, *pszDisplayName == L'/');
				if (!pidlChild)
				{
					::CoTaskMemFree(pidlCurrent);
					return E_OUTOFMEMORY;
				}
				PIDLIST_RELATIVE pidl2 = (PIDLIST_RELATIVE) ::AppendItemIDList(
					(PCUIDLIST_ABSOLUTE) (pidlCurrent ? pidlCurrent : pidlParent),
					(PCUIDLIST_RELATIVE) pidlChild);
				::CoTaskMemFree(pidlChild);
				if (pidlCurrent)
					::CoTaskMemFree(pidlCurrent);
				if (!pidl2)
					return E_OUTOFMEMORY;
				pidlCurrent = pidl2;
			}
		}
		if (pdwAttributes)
		{
			//hr = GetAttributesOf(1, (PCUITEMID_CHILD_ARRAY) &pidlCurrent, pdwAttributes);
			*pdwAttributes &= SFGAO_FOLDER;
		}
		if (pchEaten)
			*pchEaten = uEaten;
		if (!pidlCurrent)
			pidlCurrent = pidlParent ? (PIDLIST_RELATIVE) ::DuplicateItemIDList((PCUIDLIST_ABSOLUTE) pidlParent) : ::MakeNullIDList();
		*ppidl = pidlCurrent;
		return *ppidl ? S_OK : E_OUTOFMEMORY;
	//}
	//else
	//	return E_INVALIDARG;
}

STDMETHODIMP CFolderBase::GetClassID(CLSID* pClassID)
{
	if (!pClassID)
		return E_POINTER;
	*pClassID = CLSID_NULL;
	return S_OK;
}

STDMETHODIMP CFolderBase::Initialize(PCIDLIST_ABSOLUTE pidl)
{
	::EnterCriticalSection(&m_csPidlMe);
	if (m_pidlMe)
		::CoTaskMemFree(m_pidlMe);

	m_pidlMe = (PIDLIST_ABSOLUTE) ::DuplicateItemIDList(pidl);
	::LeaveCriticalSection(&m_csPidlMe);
	if (!m_pidlMe)
		return E_OUTOFMEMORY;

	return S_OK;
}

STDMETHODIMP CFolderBase::GetCurFolder(PIDLIST_ABSOLUTE* ppidl)
{
	::EnterCriticalSection(&m_csPidlMe);
	*ppidl = (PIDLIST_ABSOLUTE) ::DuplicateItemIDList(m_pidlMe);
	::LeaveCriticalSection(&m_csPidlMe);
	return m_pidlMe ? S_OK : S_FALSE;
}

STDMETHODIMP CFolderBase::GetIDList(PIDLIST_ABSOLUTE* ppidl)
{
	if (!ppidl)
		return E_POINTER;
	::EnterCriticalSection(&m_csPidlMe);
	*ppidl = (PIDLIST_ABSOLUTE) ::DuplicateItemIDList(m_pidlMe);
	::LeaveCriticalSection(&m_csPidlMe);
	return *ppidl ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CFolderBase::SetIDList(PCIDLIST_ABSOLUTE pidl)
{
	return E_ACCESSDENIED;
}

STDMETHODIMP CFolderBase::SetSite(IUnknown* pUnkSite)
{
	if (m_pUnkSite)
		m_pUnkSite->Release();
	m_pUnkSite = pUnkSite;
	if (pUnkSite)
		pUnkSite->AddRef();
	return S_OK;
}

STDMETHODIMP CFolderBase::GetSite(REFIID riid, void** ppvSite)
{
	if (!ppvSite)
		return E_POINTER;
	if (!m_pUnkSite)
	{
		*ppvSite = NULL;
		return E_FAIL;
	}
	return m_pUnkSite->QueryInterface(riid, ppvSite);
}

void CFolderBase::NotifyUpdate(LONG wEventId, PCUITEMID_CHILD pidlChild1, PCUITEMID_CHILD pidlChild2)
{
	::EnterCriticalSection(&m_csPidlMe);
	PIDLIST_ABSOLUTE pidl1 = pidlChild1 ? ::AppendItemIDList(m_pidlMe, pidlChild1) : NULL;
	PIDLIST_ABSOLUTE pidl2 = pidlChild2 ? ::AppendItemIDList(m_pidlMe, pidlChild2) : NULL;
	::LeaveCriticalSection(&m_csPidlMe);
	theApp.MyChangeNotify(wEventId, SHCNF_IDLIST | SHCNF_NOTIFYRECURSIVE | SHCNF_FLUSHNOWAIT, pidl1, pidl2);
	if (pidl1)
		::CoTaskMemFree(pidl1);
	if (pidl2)
		::CoTaskMemFree(pidl2);

	// In registry-emulating mode, the view cannot receive the change notify.
	// So we will update our own views manually.
	if (theApp.m_bEmulateRegMode)
		DefViewNotifyUpdate(wEventId, pidlChild1, pidlChild2);
}

void CFolderBase::DefViewNotifyUpdate(LONG wEventId, PCUITEMID_CHILD pidlChild1, PCUITEMID_CHILD pidlChild2)
{
	switch (wEventId)
	{
		case SHCNE_CREATE:
		case SHCNE_MKDIR:
			for (int n = 0; n < m_ahWndViews.GetCount(); n++)
			{
				HWND hWndBrowser = ::GetParent(m_ahWndViews.GetItem(n));
				PITEMID_CHILD pidl = (PITEMID_CHILD) ::DuplicateItemIDList(pidlChild1);
				// SFVM_ADDOBJECT calls ILFree() (or CoTaskMemFree()) to lParam
				SHShellFolderView_Message(hWndBrowser, SFVM_ADDOBJECT,
					(LPARAM) pidl);
			}
			break;
		case SHCNE_RENAMEFOLDER:
		case SHCNE_RENAMEITEM:
		{
			for (int n = 0; n < m_ahWndViews.GetCount(); n++)
			{
				// SFVM_UPDATEOBJECT calls ILFree() (or CoTaskMemFree()) to lParam[1]
				PCUITEMID_CHILD pidlChildren[2] = {
					pidlChild1,
					(PITEMID_CHILD) ::DuplicateItemIDList(pidlChild2)
				};
				HWND hWndBrowser = ::GetParent(m_ahWndViews.GetItem(n));
				SHShellFolderView_Message(hWndBrowser, SFVM_UPDATEOBJECT, (LPARAM) pidlChildren);
			}
		}
		break;
		case SHCNE_UPDATEDIR:
		case SHCNE_UPDATEITEM:
		{
			for (int n = 0; n < m_ahWndViews.GetCount(); n++)
			{
				// SFVM_UPDATEOBJECT calls ILFree() (or CoTaskMemFree()) to lParam[1]
				PCUITEMID_CHILD pidlChildren[2] = {
					pidlChild1,
					(PITEMID_CHILD) ::DuplicateItemIDList(pidlChild2)
				};
				HWND hWndBrowser = ::GetParent(m_ahWndViews.GetItem(n));
				SHShellFolderView_Message(hWndBrowser, SFVM_UPDATEOBJECT, (LPARAM) pidlChildren);
			}
		}
		break;
		case SHCNE_DELETE:
		case SHCNE_RMDIR:
			for (int n = 0; n < m_ahWndViews.GetCount(); n++)
			{
				HWND hWndBrowser = ::GetParent(m_ahWndViews.GetItem(n));
				SHShellFolderView_Message(hWndBrowser, SFVM_REMOVEOBJECT,
					(LPARAM) pidlChild1);
			}
			break;
	}
}

IShellBrowser* CFolderBase::GetShellBrowser(HWND hWndOwner)
{
	IShellBrowser* pRet = NULL;
	if (hWndOwner)
	{
		pRet = (IShellBrowser*) ::SendMessage(hWndOwner, CWM_GETISHELLBROWSER, 0, 0);
		if (pRet)
			pRet->AddRef();
	}
	if (!pRet)
	{
		if (m_pUnkSite)
		{
			if (FAILED(m_pUnkSite->QueryInterface(IID_IShellBrowser, (void**) &pRet)))
				pRet = NULL;
		}
	}
	return pRet;
}
