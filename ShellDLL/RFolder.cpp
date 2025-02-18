/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 RFolder.cpp - implementations of CEasySFTPFolderRoot
 */

#include "stdafx.h"
#include "ShellDLL.h"
#include "RFolder.h"

#include "DIMalloc.h"
#include "FTPFldr.h"
#include "SFTPFldr.h"

#include "HostPage.h"
#include "CsetPage.h"
#include "TferPage.h"

static const WORD s_wRootCommands[] = {
	ID_RCOMMAND_ADD_HOST
};
#define ROOT_COMMAND_COUNT    (sizeof(s_wRootCommands) / sizeof(s_wRootCommands[0]))

// {36089763-B05F-4ed9-B7CB-D2597B21ADA7}
static const GUID GUID_RootItemColumn =
{ 0x36089763, 0xb05f, 0x4ed9, { 0xb7, 0xcb, 0xd2, 0x59, 0x7b, 0x21, 0xad, 0xa7 } };
#define PID_ROOTITEM_BASE          200
#ifdef STRING_ID_TO_MY_PID
#undef STRING_ID_TO_MY_PID
#undef MY_PID_TO_STRING_ID
#endif
#define STRING_ID_TO_MY_PID(u)     (PID_ROOTITEM_BASE + (u) - IDS_HEAD_FILE_NAME + 1)
#define MY_PID_TO_STRING_ID(u)     (IDS_HEAD_FILE_NAME - 1 + (u) - PID_ROOTITEM_BASE)
#define PID_ROOTITEM_NAME          STRING_ID_TO_MY_PID(IDS_RHEAD_NAME)
#define PID_ROOTITEM_MODE          STRING_ID_TO_MY_PID(IDS_RHEAD_MODE)
#define PID_ROOTITEM_HOST          STRING_ID_TO_MY_PID(IDS_RHEAD_HOST)
#define PID_ROOTITEM_PORT          STRING_ID_TO_MY_PID(IDS_RHEAD_PORT)
#define PID_ROOTITEM_STATUS        STRING_ID_TO_MY_PID(IDS_RHEAD_STATUS)
static const UINT s_uHeaderRootItemPIDs[] = {
	PID_ROOTITEM_NAME,
	PID_ROOTITEM_MODE,
	PID_ROOTITEM_HOST,
	PID_ROOTITEM_PORT,
	PID_ROOTITEM_STATUS
};

static HRESULT GetConnectionModeFromUrl(LPCWSTR lpsz, EasySFTPConnectionMode* pMode)
{
	if (wcsncmp(lpsz, L"sftp:", 5) == 0)
		*pMode = EasySFTPConnectionMode::SFTP;
	else if (wcsncmp(lpsz, L"ftp:", 4) == 0)
		*pMode = EasySFTPConnectionMode::FTP;
	else
		return E_INVALIDARG;
	return S_OK;
}

static int CompareConnectionMode(EasySFTPConnectionMode mode1, EasySFTPConnectionMode mode2)
{
	if (mode1 == mode2)
		return 0;
	if (mode1 == EasySFTPConnectionMode::SFTP)
		return -1;
	return 1;
}

////////////////////////////////////////////////////////////////////////////////

class CEnumRootItemIDList : public CUnknownImplT<IEnumIDList>
{
public:
	CEnumRootItemIDList(CEasySFTPFolderRoot* pRoot, SHCONTF grfFlags);
	virtual ~CEnumRootItemIDList();

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);

	STDMETHOD(Next)(ULONG celt, PITEMID_CHILD* rgelt, ULONG* pceltFetched);
	STDMETHOD(Skip)(ULONG celt);
	STDMETHOD(Reset)();
	STDMETHOD(Clone)(IEnumIDList** ppEnum);

private:
	SHCONTF m_grfFlags;
	ULONG m_uPos;
	CEasySFTPFolderRoot* m_pRoot;
};

////////////////////////////////////////////////////////////////////////////////

CEnumRootItemIDList::CEnumRootItemIDList(CEasySFTPFolderRoot* pRoot, SHCONTF grfFlags)
	: m_pRoot(pRoot)
	, m_grfFlags(grfFlags)
	, m_uPos(0)
{
	m_pRoot->AddRef();
}

CEnumRootItemIDList::~CEnumRootItemIDList()
{
	m_pRoot->Release();
}

STDMETHODIMP CEnumRootItemIDList::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IEnumIDList))
	{
		*ppv = this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP CEnumRootItemIDList::Next(ULONG celt, PITEMID_CHILD* rgelt, ULONG* pceltFetched)
{
	if (!rgelt)
		return E_POINTER;
	if (!celt)
		return S_OK;
	if (pceltFetched)
		*pceltFetched = 0;
	::EnterCriticalSection(&theApp.m_csHosts);
	ULONG uCount = (ULONG)theApp.m_aHosts.GetCount();
	while (m_uPos < uCount)
	{
		CHostFolderData* pData = theApp.m_aHosts.GetItem((int)(m_uPos++));
		if ((m_grfFlags & SHCONTF_FOLDERS) != 0)
		{
			*rgelt = ::CreateHostItem(m_pRoot->m_pMallocData->pMalloc,
				pData->ConnectionMode, (WORD)pData->nPort, pData->pDirItem->strName);
			if (*rgelt && pceltFetched)
				(*pceltFetched)++;
			rgelt++;
			if (!--celt)
			{
				::LeaveCriticalSection(&theApp.m_csHosts);
				return S_OK;
			}
		}
	}
	::LeaveCriticalSection(&theApp.m_csHosts);
	while (m_uPos < uCount + ROOT_COMMAND_COUNT)
	{
		ULONG uIndex = (m_uPos++) - uCount;
		if ((m_grfFlags & SHCONTF_NONFOLDERS) != 0)
		{
			*rgelt = ::CreateRootCommandItem(m_pRoot->m_pMallocData->pMalloc,
				s_wRootCommands[uIndex]);
			if (*rgelt && pceltFetched)
				(*pceltFetched)++;
			rgelt++;
			if (!--celt)
				return S_OK;
		}
	}
	return S_FALSE;
}

STDMETHODIMP CEnumRootItemIDList::Skip(ULONG celt)
{
	if (!celt)
		return S_OK;
	::EnterCriticalSection(&theApp.m_csHosts);
	if (m_uPos + celt > (ULONG)theApp.m_aHosts.GetCount() + ROOT_COMMAND_COUNT)
	{
		m_uPos = (ULONG)theApp.m_aHosts.GetCount() + ROOT_COMMAND_COUNT;
		::LeaveCriticalSection(&theApp.m_csHosts);
		return S_FALSE;
	}
	::LeaveCriticalSection(&theApp.m_csHosts);
	m_uPos += celt;
	return S_OK;
}

STDMETHODIMP CEnumRootItemIDList::Reset()
{
	m_uPos = 0;
	return S_OK;
}

STDMETHODIMP CEnumRootItemIDList::Clone(IEnumIDList** ppEnum)
{
	CEnumRootItemIDList* pEnum = new CEnumRootItemIDList(m_pRoot, m_grfFlags);
	pEnum->m_uPos = m_uPos;
	*ppEnum = pEnum;
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

CEasySFTPFolderRoot::CEasySFTPFolderRoot()
	: CFolderBase(new CDelegateMallocData())
	, CDispatchImplNoUnknownT(theApp.GetTypeInfo(IID_IEasySFTPRoot))
{
	m_pMallocData->pMalloc = new CDelegateItemIDMalloc();
	m_uRef = 1;
	//m_pListener = NULL;
	m_pidlMe = NULL;
	::EnterCriticalSection(&theApp.m_csRootRefs);
	theApp.SetReference(this);
	::LeaveCriticalSection(&theApp.m_csRootRefs);

	CMyStringW str;
	str.Format(L"CEasySFTPFolderRoot::CEasySFTPFolderRoot() for (0x%p), count = 1\n",
		(void*)this);
	OutputDebugString(str);
}

CEasySFTPFolderRoot::~CEasySFTPFolderRoot()
{
	//if (m_pListener)
	//	m_pListener->Release();
	//if (m_pidlMe)
	//	::CoTaskMemFree(m_pidlMe);
	m_pMallocData->Release();
}

STDMETHODIMP CEasySFTPFolderRoot::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	if (IsEqualIID(riid, IID_CEasySFTPFolderRoot))
	{
		*ppv = this;
		AddRef();
		return S_OK;
	}
	if (IsEqualIID(riid, IID_IDelegateFolder))
	{
		*ppv = static_cast<IDelegateFolder*>(this);
		AddRef();
		return S_OK;
	}
	if (IsEqualIID(riid, IID_IPersistPropertyBag))
	{
		*ppv = static_cast<IPersistPropertyBag*>(this);
		AddRef();
		return S_OK;
	}
	if (IsEqualIID(riid, IID_IDispatch) || IsEqualIID(riid, IID_IEasySFTPRoot))
	{
		*ppv = static_cast<IEasySFTPRoot*>(this);
		AddRef();
		return S_OK;
	}
	if (IsEqualIID(riid, IID_IProvideClassInfo))
	{
		*ppv = static_cast<IProvideClassInfo*>(this);
		AddRef();
		return S_OK;
	}
	if (IsEqualIID(riid, IID_IEasySFTPOldRoot) || IsEqualIID(riid, IID_IEasySFTPOldRoot2))
	{
		*ppv = static_cast<IEasySFTPOldRoot2*>(this);
		AddRef();
		return S_OK;
	}
	if (IsEqualIID(riid, IID_IEasySFTPInternal))
	{
		*ppv = static_cast<IEasySFTPInternal*>(this);
		AddRef();
		return S_OK;
	}
	HRESULT hr = CFolderBase::QueryInterface(riid, ppv);
	if (SUCCEEDED(hr))
		return hr;
	*ppv = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CEasySFTPFolderRoot::AddRef()
{
//#ifdef _DEBUG
//	CMyStringW str;
//	str.Format(L"CEasySFTPFolderRoot::AddRef() (%p), count = %lu\n",
//		this, (m_uRef + 1));
//	OutputDebugString(str);
//#endif
	CFolderBase::OnAddRef();
	return ::InterlockedIncrement(&m_uRef);
}

STDMETHODIMP_(ULONG) CEasySFTPFolderRoot::Release()
{
//#ifdef _DEBUG
//	CMyStringW str;
//	str.Format(L"CEasySFTPFolderRoot::Release() (%p), count = %lu\n",
//		this, (m_uRef - 1));
//	OutputDebugString(str);
//#endif
	CFolderBase::OnRelease();
	::EnterCriticalSection(&theApp.m_csRootRefs);
	auto u = ::InterlockedDecrement(&m_uRef);
	if (!u)
		theApp.SetReference(NULL);
	::LeaveCriticalSection(&theApp.m_csRootRefs);
	if (!u)
		delete this;
	return u;
}

STDMETHODIMP CEasySFTPFolderRoot::GetClassInfo(ITypeInfo** ppTI)
{
	return theApp.m_pTypeLib->GetTypeInfoOfGuid(CLSID_EasySFTPRoot, ppTI);
}

STDMETHODIMP CEasySFTPFolderRoot::ParseDisplayName(HWND hWnd, LPBC pbc, LPWSTR pszDisplayName,
	ULONG* pchEaten, PIDLIST_RELATIVE* ppidl, ULONG* pdwAttributes)
{
	ULONG uEaten;
	PIDLIST_RELATIVE pidlCurrent;
	if (!ppidl)
		return E_POINTER;
	*ppidl = NULL;
	m_hWndOwnerCache = hWnd;
	EasySFTPConnectionMode mode;
	if (SUCCEEDED(GetConnectionModeFromUrl(pszDisplayName, &mode)))
	{
		LPWSTR lpw, lpw2;
		CMyStringW strName;
		int nPort = mode == EasySFTPConnectionMode::SFTP ? 22 : 21;

		lpw = wcschr(pszDisplayName, L':');
		if (!lpw)
			return E_INVALIDARG;
		++lpw;
		if (lpw[0] != L'/' || lpw[1] != L'/')
			return E_INVALIDARG;
		lpw += 2;
		uEaten = static_cast<ULONG>(lpw - pszDisplayName);
		pszDisplayName = lpw;
		lpw = wcschr(pszDisplayName, L'/');
		lpw2 = wcschr(pszDisplayName, L':');
		if (lpw2)
		{
			CMyStringW str2;
			strName.SetString(pszDisplayName, (DWORD)((DWORD_PTR)lpw2 - (DWORD_PTR)pszDisplayName) / sizeof(WCHAR));
			lpw2++;
			if (lpw)
				str2.SetString(pszDisplayName, (DWORD)((DWORD_PTR)lpw - (DWORD_PTR)lpw2) / sizeof(WCHAR));
			else
				str2 = lpw2;
			nPort = _wtoi(str2);
		}
		else
		{
			if (lpw)
				strName.SetString(pszDisplayName, (DWORD)((DWORD_PTR)lpw - (DWORD_PTR)pszDisplayName) / sizeof(WCHAR));
			else
				strName = pszDisplayName;
		}
		if (strName.IsEmpty())
			return E_INVALIDARG;
		pszDisplayName += strName.GetLength();
		uEaten += (ULONG)strName.GetLength();

		pidlCurrent = (PIDLIST_RELATIVE) ::CreateHostItem(m_pMallocData->pMalloc, mode, (WORD)nPort, strName);
		if (!pidlCurrent)
			return E_OUTOFMEMORY;

		CFTPDirectoryRootBase* pRoot;
		ULONG u = 0;
		HRESULT hr = _BindToObject(hWnd, (PCUITEMID_CHILD)pidlCurrent, NULL, NULL, &pRoot);
		if (SUCCEEDED(hr) && pRoot->IsDirectoryReceived())
		{
			PIDLIST_RELATIVE pidlChildRelative = NULL;
			hr = pRoot->ParseDisplayName(hWnd, pbc, pszDisplayName, &u, &pidlChildRelative, pdwAttributes);
			if (SUCCEEDED(hr))
			{
				*ppidl = ::AppendItemIDList((PIDLIST_ABSOLUTE)pidlCurrent, pidlChildRelative);
				::CoTaskMemFree(pidlChildRelative);
				if (!*ppidl)
					hr = E_OUTOFMEMORY;
			}
			pRoot->Release();
		}
		else
			hr = ParseDisplayName2(pidlCurrent, hWnd, pbc, pszDisplayName, &u, ppidl, pdwAttributes);
		if (SUCCEEDED(hr) && pchEaten)
			*pchEaten = uEaten + u;
		m_pMallocData->pMalloc->Free(pidlCurrent);
		return hr;
	}
	else
	{
		CHostFolderData* pHostData;
		HRESULT hr = E_INVALIDARG;
		::EnterCriticalSection(&theApp.m_csHosts);
		for (int i = 0; i < theApp.m_aHosts.GetCount(); i++)
		{
			pHostData = theApp.m_aHosts.GetItem(i);
			if (pHostData->pDirItem->strName.Compare(pszDisplayName) == 0)
			{
				pidlCurrent = (PIDLIST_RELATIVE) ::CreateHostItem(m_pMallocData->pMalloc,
					pHostData->ConnectionMode, (WORD)pHostData->nPort, pHostData->pDirItem->strName);
				if (!pidlCurrent)
				{
					hr = E_OUTOFMEMORY;
					break;
				}
				if (pchEaten)
					*pchEaten = (ULONG)wcslen(pszDisplayName);
				*ppidl = pidlCurrent;
				hr = S_OK;
				break;
			}
		}
		::LeaveCriticalSection(&theApp.m_csHosts);
		return hr;
	}
}

STDMETHODIMP CEasySFTPFolderRoot::EnumObjects(HWND hWnd, SHCONTF grfFlags, IEnumIDList** ppenumIDList)
{
	if (!ppenumIDList)
		return E_POINTER;
	*ppenumIDList = new CEnumRootItemIDList(this, grfFlags);
	return *ppenumIDList ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CEasySFTPFolderRoot::BindToObject(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void** ppv)
{
	if (!pidl || !ppv)
		return E_POINTER;
	*ppv = NULL;

	if (IsEqualIID(riid, IID_IShellItem) || IsEqualIID(riid, IID_IShellItem2))
	{
		::EnterCriticalSection(&m_csPidlMe);
		auto pidlAbs = m_pidlMe ? ::AppendItemIDList(m_pidlMe, pidl) : reinterpret_cast<PIDLIST_ABSOLUTE>(::DuplicateItemIDList(pidl));
		::LeaveCriticalSection(&m_csPidlMe);
		if (!pidlAbs)
			return E_OUTOFMEMORY;
		IShellItem* pItem = NULL;
		auto hr = ::MyCreateShellItem(pidlAbs, &pItem);
		if (FAILED(hr))
			return hr;
		if (!pItem)
			return E_UNEXPECTED;
		hr = pItem->QueryInterface(riid, ppv);
		pItem->Release();
		return hr;
	}

	PCUIDLIST_RELATIVE pidlNext = (PCUIDLIST_RELATIVE)(((DWORD_PTR)pidl) + pidl->mkid.cb);
	CFTPDirectoryRootBase* pRet;
	HRESULT hr = _BindToObject(NULL, (PCUITEMID_CHILD)pidl, pbc, NULL, &pRet);
	if (FAILED(hr))
		return hr;
	if (pidlNext->mkid.cb)
		hr = pRet->BindToObject(pidlNext, pbc, riid, ppv);
	else
		hr = pRet->QueryInterface(riid, ppv);
	pRet->Release();
	return hr;
}

HRESULT CEasySFTPFolderRoot::_BindToObject(HWND hWndOwner, PCUITEMID_CHILD pidl, LPBC pbc, IEasySFTPAuthentication* pUser, CFTPDirectoryRootBase** ppRoot)
{
	if (!pidl)
		return E_POINTER;
	CMyStringW strHostName;
	if (!::PickupHostName(pidl, strHostName))
		return E_INVALIDARG;
	CSFTPHostItem UNALIGNED* pItem = (CSFTPHostItem UNALIGNED*) pidl;

	CFTPDirectoryRootBase* pDirectory = NULL;
	CHostFolderData* pHostData;
	bool bFound = false;
	//bool bFolderAssigned = false;
	::EnterCriticalSection(&theApp.m_csHosts);
	pHostData = theApp.FindHostFolderDataUnsafe(pItem->GetConnectionMode(), strHostName, (int)pItem->nPort);
	bFound = (pHostData != NULL);
	if (pHostData)
		pDirectory = static_cast<CFTPDirectoryRootBase*>(pHostData->pDirItem->pDirectory);
	//if (!bFolderAssigned)
	if (!pDirectory)
	{
		if (!bFound)
		{
			pHostData = new CHostFolderData();
			pHostData->ConnectionMode = pItem->GetConnectionMode();
			pHostData->pDirItem = new CFTPRootDirectoryItem();
			pHostData->pDirItem->strName = strHostName;
			pHostData->pDirItem->pDirectory = NULL;
			pHostData->nPort = pItem->nPort;
			pHostData->pSettings = NULL;
		}
		if (pItem->GetConnectionMode() == EasySFTPConnectionMode::SFTP)
		{
			CSFTPFolderSFTP* pSFTP = new CSFTPFolderSFTP(m_pMallocData, pHostData->pDirItem, this);
			if (!pSFTP)
			{
				if (!bFound)
				{
					pHostData->pDirItem->Release();
					delete pHostData;
				}
				::LeaveCriticalSection(&theApp.m_csHosts);
				return E_OUTOFMEMORY;
			}
			pSFTP->m_strHostName = strHostName;
			pSFTP->m_nPort = pItem->nPort;
			pDirectory = pSFTP;

			if (pHostData->pSettings)
				pSFTP->m_nServerCharset = pHostData->pSettings->nServerCharset;
			else
				pSFTP->m_nServerCharset = scsUTF8;
		}
		else
		{
			CSFTPFolderFTP* pFTP = new CSFTPFolderFTP(m_pMallocData, pHostData->pDirItem, this);
			if (!pFTP)
			{
				if (!bFound)
				{
					pHostData->pDirItem->Release();
					delete pHostData;
				}
				::LeaveCriticalSection(&theApp.m_csHosts);
				return E_OUTOFMEMORY;
			}
			pFTP->m_strHostName = strHostName;
			pFTP->m_nPort = pItem->nPort;
			pDirectory = pFTP;

			if (pHostData->pSettings)
			{
				pFTP->m_strChmodCommand = pHostData->pSettings->strChmodCommand;
				pFTP->m_nServerCharset = pHostData->pSettings->nServerCharset;
			}
			else
			{
				pFTP->m_strChmodCommand = DEFAULT_CHMOD_COMMAND;
				pFTP->m_nServerCharset = scsUTF8;
			}
		}

		if (pHostData->pSettings)
		{
			pDirectory->m_bTextMode = pHostData->pSettings->bTextMode;
			pDirectory->m_nServerCharset = pHostData->pSettings->nServerCharset;
			pDirectory->m_nTransferMode = pHostData->pSettings->nTransferMode;
			pDirectory->m_arrTextFileType.CopyArray(pHostData->pSettings->arrTextFileType);
			pDirectory->m_bUseSystemTextFileType = pHostData->pSettings->bUseSystemTextFileType;
			pDirectory->m_bAdjustSendModifyTime = pHostData->pSettings->bAdjustSendModifyTime;
			pDirectory->m_bAdjustRecvModifyTime = pHostData->pSettings->bAdjustRecvModifyTime;
			pDirectory->m_bUseThumbnailPreview = pHostData->pSettings->bUseThumbnailPreview;
		}
		else
		{
			pDirectory->m_bTextMode = TEXTMODE_NO_CONVERT;
			pDirectory->m_nServerCharset = scsUTF8;
			pDirectory->m_nTransferMode = TRANSFER_MODE_AUTO;
			pDirectory->m_arrTextFileType.CopyArray(theApp.m_arrDefTextFileType);
			pDirectory->m_bUseSystemTextFileType = true;
			pDirectory->m_bAdjustSendModifyTime = false;
			pDirectory->m_bAdjustRecvModifyTime = true;
			pDirectory->m_bUseThumbnailPreview = true;
		}

		IPersistFolder* pPFolder;
		HRESULT hr = pDirectory->QueryInterface(IID_IPersistFolder, (void**)&pPFolder);
		if (SUCCEEDED(hr))
		{
			PITEMID_CHILD pidlChild = (PITEMID_CHILD) ::CoTaskMemAlloc(sizeof(USHORT) + pidl->mkid.cb);
			if (!pidlChild)
			{
				pPFolder->Release();
				pDirectory->DetachAndRelease();
				if (!bFound)
				{
					pHostData->pDirItem->Release();
					delete pHostData;
				}
				::LeaveCriticalSection(&theApp.m_csHosts);
				return E_OUTOFMEMORY;
			}
			memcpy(pidlChild, pidl, (size_t)pidl->mkid.cb);
			*((USHORT UNALIGNED*)(((DWORD_PTR)pidlChild) + pidl->mkid.cb)) = 0;
			::EnterCriticalSection(&m_csPidlMe);
			PIDLIST_ABSOLUTE p = ::AppendItemIDList(m_pidlMe, pidlChild);
			::LeaveCriticalSection(&m_csPidlMe);
			::CoTaskMemFree(pidlChild);
			if (!p)
			{
				pPFolder->Release();
				pDirectory->DetachAndRelease();
				if (!bFound)
				{
					pHostData->pDirItem->Release();
					delete pHostData;
				}
				::LeaveCriticalSection(&theApp.m_csHosts);
				return E_OUTOFMEMORY;
			}
			hr = pPFolder->Initialize(p);
			pPFolder->Release();
			if (FAILED(hr))
			{
				pDirectory->DetachAndRelease();
				if (!bFound)
				{
					pHostData->pDirItem->Release();
					delete pHostData;
				}
				::LeaveCriticalSection(&theApp.m_csHosts);
				return hr;
			}
		}
		pHostData->pDirItem->pDirectory = pDirectory;
	}

	if (!bFound)
		theApp.m_aHosts.Add(pHostData);
	::LeaveCriticalSection(&theApp.m_csHosts);

	if (pUser)
	{
		if (!pDirectory->Connect(hWndOwner, strHostName, pItem->nPort, pUser))
		{
			return E_ABORT;
		}
	}

	pDirectory->AddRef();
	*ppRoot = pDirectory;
	return S_OK;
}

STDMETHODIMP CEasySFTPFolderRoot::BindToStorage(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void** ppv)
{
	if (!pidl || !ppv)
		return E_POINTER;
	if (!IsEqualIID(riid, IID_IUnknown) &&
		!IsEqualIID(riid, IID_IStorage) &&
		!IsEqualIID(riid, IID_ISequentialStream) &&
		!IsEqualIID(riid, IID_IStream))
		return E_NOINTERFACE;
	*ppv = NULL;
	PCUIDLIST_RELATIVE pidlNext = (PCUIDLIST_RELATIVE)(((DWORD_PTR)pidl) + pidl->mkid.cb);
	CFTPDirectoryRootBase* pRet;
	HRESULT hr = _BindToObject(NULL, (PCUITEMID_CHILD)pidl, pbc, NULL, &pRet);
	if (FAILED(hr))
		return hr;
	if (pidlNext->mkid.cb)
		hr = pRet->BindToStorage(pidlNext, pbc, riid, ppv);
	else
		hr = pRet->QueryInterface(riid, ppv);
	pRet->Release();
	return hr;
}

STDMETHODIMP CEasySFTPFolderRoot::CompareIDs(LPARAM lParam, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2)
{
	if (!pidl1 || !pidl2)
		return E_POINTER;

	int nColumn = (lParam & SHCIDS_COLUMNMASK);

	if (nColumn < 0 || nColumn >= sizeof(s_uHeaderRootItemPIDs) / sizeof(s_uHeaderRootItemPIDs[0]))
	{
		return E_INVALIDARG;
	}
	WORD wID1, wID2;
	CMyStringW str1, str2;

	if (!pidl1->mkid.cb)
		return pidl2->mkid.cb ? MAKE_HRESULT(0, 0, (unsigned short)(short)-1) : MAKE_HRESULT(0, 0, 0);
	else if (!pidl2->mkid.cb)
		return MAKE_HRESULT(0, 0, 1);

	if (!PickupRootCommandItemID((PCUITEMID_CHILD)pidl1, wID1))
	{
		if (!PickupHostName((PCUITEMID_CHILD)pidl1, str1))
			return E_INVALIDARG;
		wID1 = 0;
	}
	if (!PickupRootCommandItemID((PCUITEMID_CHILD)pidl2, wID2))
	{
		if (!PickupHostName((PCUITEMID_CHILD)pidl2, str2))
			return E_INVALIDARG;
		wID2 = 0;
	}

	int r;
	if (wID1 || wID2)
	{
		if (!wID1)
			r = -1;
		else if (!wID2)
			r = 1;
		else
		{
			if (wID1 < wID2)
				r = -1;
			else if (wID1 > wID2)
				r = 1;
			else
				r = 0;
		}

		return MAKE_HRESULT(0, 0, (unsigned short)(short)r);
	}
	else
	{
		CSFTPHostItem UNALIGNED* pItem1 = (CSFTPHostItem UNALIGNED*) pidl1;
		CSFTPHostItem UNALIGNED* pItem2 = (CSFTPHostItem UNALIGNED*) pidl2;

		int r1, r2, r3, r4;
		r1 = str1.Compare(str2);
		r2 = CompareConnectionMode(pItem1->GetConnectionMode(), pItem2->GetConnectionMode());
		r3 = pItem1->nPort - pItem2->nPort;
		r3 = r3 < 0 ? -1 : r3 > 0 ? 1 : 0;
		{
			CMyStringW strHost1, strHost2;
			MyGetUnalignedString(pItem1->wchHostName, strHost1);
			MyGetUnalignedString(pItem2->wchHostName, strHost2);
			CHostFolderData* pHostData1 = theApp.FindHostFolderData(pItem1->GetConnectionMode(), strHost1, (int)pItem1->nPort);
			CHostFolderData* pHostData2 = theApp.FindHostFolderData(pItem2->GetConnectionMode(), strHost2, (int)pItem2->nPort);
			bool bConnected1 = (pHostData1 != NULL && pHostData1->pDirItem->pDirectory != NULL &&
				pHostData1->pDirItem->pDirectory->IsConnected() == S_OK);
			bool bConnected2 = (pHostData2 != NULL && pHostData2->pDirItem->pDirectory != NULL &&
				pHostData2->pDirItem->pDirectory->IsConnected() == S_OK);
			if (bConnected1)
				r4 = bConnected2 ? 0 : -1;
			else
				r4 = bConnected2 ? 1 : 0;
		}

		r = 0;
		switch (s_uHeaderRootItemPIDs[nColumn])
		{
		case PID_ROOTITEM_NAME:
			r = r1;
			break;
		case PID_ROOTITEM_MODE:
			r = r2;
			break;
		case PID_ROOTITEM_PORT:
			r = r3;
			break;
		case PID_ROOTITEM_STATUS:
			r = r4;
			break;
		}
		if (r == 0)
		{
			if (r1)
				r = r1;
			else if (r2)
				r = r2;
			else if (r3)
				r = r3;
			else
				r = r4;
		}
		if (r != 0)
			return MAKE_HRESULT(0, 0, (unsigned short)(short)r);
	}

	pidl1 = (PCUIDLIST_RELATIVE)(((DWORD_PTR)pidl1) + pidl1->mkid.cb);
	pidl2 = (PCUIDLIST_RELATIVE)(((DWORD_PTR)pidl2) + pidl2->mkid.cb);

	while (true)
	{
		if (!pidl1->mkid.cb)
			return pidl2->mkid.cb ? MAKE_HRESULT(0, 0, (unsigned short)(short)-1) : MAKE_HRESULT(0, 0, 0);
		else if (!pidl2->mkid.cb)
			return MAKE_HRESULT(0, 0, 1);
		if (!PickupFileName((PCUITEMID_CHILD)pidl1, str1))
			return E_INVALIDARG;
		if (!PickupFileName((PCUITEMID_CHILD)pidl2, str2))
			return E_INVALIDARG;
		//if (nColumn == 0)
		r = str1.Compare(str2);
		if (r != 0)
			return MAKE_HRESULT(0, 0, r);
		pidl1 = (PCUIDLIST_RELATIVE)(((DWORD_PTR)pidl1) + pidl1->mkid.cb);
		pidl2 = (PCUIDLIST_RELATIVE)(((DWORD_PTR)pidl2) + pidl2->mkid.cb);
	}
}

STDMETHODIMP CEasySFTPFolderRoot::CreateViewObject(HWND hWndOwner, REFIID riid, void** ppv)
{
	if (IsEqualIID(riid, IID_IExtractIconA) || IsEqualIID(riid, IID_IExtractIconW))
	{
		CEasySFTPRootIcon* pIcon = new CEasySFTPRootIcon(IML_INDEX_EASYSFTP);
		HRESULT hr = pIcon->QueryInterface(riid, ppv);
		pIcon->Release();
		return hr;
	}
	else if (IsEqualIID(riid, IID_IContextMenu))
		return E_NOTIMPL;
	return CFolderBase::CreateViewObject(hWndOwner, riid, ppv);
}

STDMETHODIMP CEasySFTPFolderRoot::GetAttributesOf(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, SFGAOF* rgfInOut)
{
	SFGAOF attrs;
	bool bFirst = true;

	if (!rgfInOut)
		return E_POINTER;
	if (!cidl)
	{
		*rgfInOut = 0;
		return E_INVALIDARG;
	}
	if (!apidl)
	{
		*rgfInOut = 0;
		return E_POINTER;
	}

	CMyStringW strHostName;
	for (UINT u = 0; u < cidl; u++, apidl++)
	{
		SFGAOF a;
		WORD wID;
		if (::PickupRootCommandItemID(*apidl, wID))
		{
			a = 0;
		}
		else
		{
			if (!::PickupHostName(*apidl, strHostName))
				return E_INVALIDARG;
			CSFTPHostItem UNALIGNED* pItem = (CSFTPHostItem UNALIGNED*) * apidl;
			a = SFGAO_BROWSABLE | SFGAO_FOLDER | SFGAO_HASSUBFOLDER;

			CHostFolderData* pHostData = theApp.FindHostFolderData(pItem->GetConnectionMode(), strHostName, (int)pItem->nPort);
			if (pHostData)
				a |= SFGAO_CANRENAME | SFGAO_HASPROPSHEET;
		}
		if (bFirst)
			attrs = a;
		else
			attrs &= a;
	}
	*rgfInOut &= attrs;

	return S_OK;
}

STDMETHODIMP CEasySFTPFolderRoot::GetUIObjectOf(HWND hWndOwner, UINT cidl, PCUITEMID_CHILD_ARRAY apidl,
	REFIID riid, UINT* rgfReserved, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	if (!cidl)
	{
		*ppv = NULL;
		return E_INVALIDARG;
	}
	if (!apidl)
	{
		*ppv = NULL;
		return E_POINTER;
	}

	if (IsEqualIID(riid, IID_IContextMenu))
	{
		IShellBrowser* pBrowser = (hWndOwner ? (IShellBrowser*) ::SendMessage(hWndOwner, CWM_GETISHELLBROWSER, 0, 0) : NULL);
		::EnterCriticalSection(&m_csPidlMe);
		CEasySFTPRootMenu* pMenu = new CEasySFTPRootMenu(this, pBrowser, m_pidlMe, apidl, (int)cidl);
		::LeaveCriticalSection(&m_csPidlMe);
		HRESULT hr = pMenu->QueryInterface(riid, ppv);
		pMenu->Release();
		return hr;
	}
	else if (IsEqualIID(riid, IID_IExtractIconA) || IsEqualIID(riid, IID_IExtractIconW))
	{
		if (cidl != 1)
		{
			*ppv = NULL;
			return E_INVALIDARG;
		}

		WORD wID;
		int iIndex;
		if (::PickupRootCommandItemID(apidl[0], wID))
		{
			switch (wID)
			{
			case ID_RCOMMAND_ADD_HOST:
				iIndex = IML_INDEX_NEWHOST;
				break;
			default:
				return E_NOTIMPL;
			}
		}
		else
		{
			CSFTPHostItem UNALIGNED* pItem;
			CMyStringW strHost;
			if (!::PickupHostName(apidl[0], strHost))
				return E_INVALIDARG;
			pItem = (CSFTPHostItem UNALIGNED*) apidl[0];

			CHostFolderData* pHostData = theApp.FindHostFolderData(pItem->GetConnectionMode(), strHost, (int)pItem->nPort);
			if (!pHostData)
				return E_INVALIDARG;
			iIndex = IML_INDEX_NETDRIVE;
		}
		//bool bConnected = (pHostData->directory.pDirectory != NULL);
		CEasySFTPRootIcon* pIcon = new CEasySFTPRootIcon(iIndex);
		HRESULT hr = pIcon->QueryInterface(riid, ppv);
		pIcon->Release();

		return hr;
	}
	else if (IsEqualIID(riid, IID_IDataObject))
	{
		::EnterCriticalSection(&m_csPidlMe);
		auto hr = ::CIDLData_CreateFromIDArray(m_pidlMe, cidl, (PCUIDLIST_RELATIVE_ARRAY)apidl, (IDataObject**)ppv);
		::LeaveCriticalSection(&m_csPidlMe);
		return hr;
	}
	*ppv = NULL;
	return E_NOTIMPL;
}

STDMETHODIMP CEasySFTPFolderRoot::GetDisplayNameOf(PCUITEMID_CHILD pidl, SHGDNF uFlags, STRRET* pName)
{
	CMyStringW str;

	if (!pidl)
	{
		str = theApp.m_strTitle;
	}
	else
	{
		WORD wID;
		if (::PickupRootCommandItemID(pidl, wID))
		{
			if (!str.LoadString(wID))
				return E_INVALIDARG;
		}
		else
		{
			CSFTPHostItem UNALIGNED* pItem;
			CMyStringW strHost;
			if (!::PickupHostName(pidl, strHost))
				return E_INVALIDARG;
			pItem = (CSFTPHostItem UNALIGNED*) pidl;

			CHostFolderData* pHostData = theApp.FindHostFolderData(pItem->GetConnectionMode(), strHost, (int)pItem->nPort);

			switch (uFlags & 0x0FFF)
			{
			case SHGDN_NORMAL:
			{
				if (uFlags & (SHGDN_FORPARSING | SHGDN_FORADDRESSBAR))
				{
					int nDef = 0;
					CMyStringW str2;
					ConnectionModeToProtocolAndPort(pItem->GetConnectionMode(), str, nDef);
					str += L"://";
					GetHostNameForUrl(strHost, str2);
					str += str2;
					if (pItem->nPort != nDef)
					{
						str2.Format(L"%d", (int)pItem->nPort);
						str += L':';
						str += str2;
					}
					str += L'/';
					bool bLastIsNotDelimiter = false;
					while (true)
					{
						pidl = (PCUITEMID_CHILD)(((DWORD_PTR)pidl) + pidl->mkid.cb);
						if (!pidl->mkid.cb)
							break;
						if (!::PickupFileName(pidl, str2))
							return E_INVALIDARG;
						if (bLastIsNotDelimiter)
							str += L'/';
						else
							bLastIsNotDelimiter = true;
						str += str2;
					}
				}
				else //if (uFlags & SHGDN_FOREDITING)
				{
					if (pHostData && pHostData->pSettings)
						str = pHostData->pSettings->strDisplayName;
					else
						str = strHost;
				}
			}
			break;
			case SHGDN_INFOLDER:
				if (uFlags & (SHGDN_FORPARSING | SHGDN_FORADDRESSBAR))
					str = strHost;
				else
				{
					if (pHostData && pHostData->pSettings)
						str = pHostData->pSettings->strDisplayName;
					else
						str = strHost;
				}
				break;
			default:
				return E_NOTIMPL;
			}
		}
	}

	SIZE_T nSize = sizeof(WCHAR) * (str.GetLength() + 1);
	auto pStr = static_cast<LPWSTR>(::CoTaskMemAlloc(nSize));
	if (!pStr)
		return E_OUTOFMEMORY;
	pName->uType = STRRET_WSTR;
	pName->pOleStr = pStr;
	memcpy(pStr, (LPCWSTR)str, nSize);
	return S_OK;
}

STDMETHODIMP CEasySFTPFolderRoot::SetNameOf(HWND hWnd, PCUITEMID_CHILD pidl, LPCWSTR pszName, SHGDNF uFlags, PITEMID_CHILD* ppidlOut)
{
	CMyStringW str;
	WORD wID;
	if (!ppidlOut)
		return E_POINTER;
	*ppidlOut = NULL;
	if (::PickupRootCommandItemID(pidl, wID))
		return E_INVALIDARG;
	else
	{
		CSFTPHostItem UNALIGNED* pItem;
		CMyStringW strHost;
		if (!::PickupHostName(pidl, strHost))
			return E_INVALIDARG;
		pItem = (CSFTPHostItem UNALIGNED*) pidl;

		CHostFolderData* pHostData;
		bool bFound = false;
		for (int i = 0; i < theApp.m_aHosts.GetCount(); i++)
		{
			pHostData = theApp.m_aHosts.GetItem(i);
			if (pHostData->ConnectionMode == pItem->GetConnectionMode() &&
				pHostData->pDirItem->strName.Compare(strHost) == 0 &&
				pHostData->nPort == (int)pItem->nPort)
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
			return E_INVALIDARG;

		switch (uFlags & 0x0FFF)
		{
		case SHGDN_NORMAL:
			return E_INVALIDARG;
		case SHGDN_INFOLDER:
		{
			if (uFlags & (SHGDN_FORPARSING | SHGDN_FORADDRESSBAR))
				return E_INVALIDARG;
			else //if (uFlags & SHGDN_FOREDITING)
			{
				auto* pNewData = new CEasySFTPHostSetting(pHostData->pSettings);
				pNewData->strDisplayName = pszName;

				theApp.UpdateHostSettings(pHostData->pSettings, pNewData, 0);

				// set dummy ppidlOut (pidl must be same because host name is not changed)
				*ppidlOut = (PITEMID_CHILD) ::DuplicateItemIDList((PCUIDLIST_ABSOLUTE)pidl);
				NotifyUpdate(SHCNE_RENAMEFOLDER, pidl, *ppidlOut);
			}
		}
		break;
		default:
			return E_NOTIMPL;
		}
	}
	return S_OK;
}

STDMETHODIMP CEasySFTPFolderRoot::GetDefaultSearchGUID(GUID* pguid)
{
	return E_NOTIMPL;
}

STDMETHODIMP CEasySFTPFolderRoot::EnumSearches(IEnumExtraSearch** ppenum)
{
	return E_NOTIMPL;
}

STDMETHODIMP CEasySFTPFolderRoot::GetDefaultColumn(DWORD dwRes, ULONG* pSort, ULONG* pDisplay)
{
	if (!pSort || !pDisplay)
		return E_POINTER;
	*pSort = 2;
	*pDisplay = 0;
	return S_OK;
}

STDMETHODIMP CEasySFTPFolderRoot::GetDefaultColumnState(UINT iColumn, SHCOLSTATEF* pcsFlags)
{
	if (!pcsFlags)
		return E_POINTER;

	SHCOLUMNID scid;
	HRESULT hr = MapColumnToSCID(iColumn, &scid);
	if (FAILED(hr))
		return hr;

	switch (scid.pid)
	{
	case PID_ROOTITEM_NAME:
	case PID_ROOTITEM_MODE:
	case PID_ROOTITEM_HOST:
		*pcsFlags = SHCOLSTATE_TYPE_STR | SHCOLSTATE_ONBYDEFAULT;
		break;
	case PID_ROOTITEM_PORT:
		*pcsFlags = SHCOLSTATE_TYPE_INT | SHCOLSTATE_ONBYDEFAULT;
		break;
	case PID_ROOTITEM_STATUS:
		*pcsFlags = SHCOLSTATE_TYPE_STR | SHCOLSTATE_ONBYDEFAULT;
		break;
	default:
		return E_INVALIDARG;
	}

	return S_OK;
}

STDMETHODIMP CEasySFTPFolderRoot::GetDetailsEx(PCUITEMID_CHILD pidl, const SHCOLUMNID* pscid, VARIANT* pv)
{
	if (!IsEqualGUID(pscid->fmtid, GUID_RootItemColumn))
		return E_INVALIDARG;

	CMyStringW str;
	WORD wID;
	CSFTPHostItem UNALIGNED* pItem = NULL;
	CHostFolderData* pHostData;
	bool bFound = false;
	if (::PickupRootCommandItemID(pidl, wID))
	{
		str.LoadString((UINT)wID);
	}
	else
	{
		if (!::PickupHostName(pidl, str))
			return E_INVALIDARG;
		pItem = (CSFTPHostItem UNALIGNED*) pidl;

		for (int i = 0; i < theApp.m_aHosts.GetCount(); i++)
		{
			pHostData = theApp.m_aHosts.GetItem(i);
			if (pHostData->ConnectionMode == pItem->GetConnectionMode() &&
				pHostData->pDirItem->strName.Compare(str) == 0 &&
				pHostData->nPort == (int)pItem->nPort)
			{
				bFound = true;
				break;
			}
		}
	}

	switch (pscid->pid)
	{
	case PID_ROOTITEM_NAME:
		pv->vt = VT_BSTR;
		if (bFound && pHostData->pSettings)
			str = pHostData->pSettings->strDisplayName;
		pv->bstrVal = ::SysAllocStringLen(str, (UINT)str.GetLength());
		if (!pv->bstrVal)
			return E_OUTOFMEMORY;
		break;
	case PID_ROOTITEM_MODE:
		pv->vt = VT_BSTR;
		static_assert(IDS_CONNECTMODE_FTP > IDS_CONNECTMODE_SFTP, "Unexpected id");
		if (pItem)
			str.LoadString(pItem->ConnectionMode + IDS_CONNECTMODE_SFTP);
		else
			str.Empty();
		pv->bstrVal = ::SysAllocStringLen(str, (UINT)str.GetLength());
		if (!pv->bstrVal)
			return E_OUTOFMEMORY;
		break;
	case PID_ROOTITEM_HOST:
		pv->vt = VT_BSTR;
		if (!pItem)
			str.Empty();
		pv->bstrVal = ::SysAllocStringLen(str, (UINT)str.GetLength());
		if (!pv->bstrVal)
			return E_OUTOFMEMORY;
		break;
	case PID_ROOTITEM_PORT:
		if (pItem)
		{
			pv->vt = VT_UI2;
			pv->uiVal = pItem->nPort;
		}
		else
			pv->vt = VT_EMPTY;
		break;
	case PID_ROOTITEM_STATUS:
	{
		bool bConnected;
		bConnected = (bFound && pHostData->pDirItem->pDirectory != NULL &&
			pHostData->pDirItem->pDirectory->IsConnected() == S_OK);
		str.LoadString(bConnected ? IDS_CONNECTED : IDS_NOT_CONNECTED);
		pv->vt = VT_BSTR;
		pv->bstrVal = ::SysAllocStringLen(str, (UINT)str.GetLength());
		if (!pv->bstrVal)
			return E_OUTOFMEMORY;
	}
	break;
	default:
		return E_INVALIDARG;
	}

	return S_OK;
}

STDMETHODIMP CEasySFTPFolderRoot::GetDetailsOf(PCUITEMID_CHILD pidl, UINT iColumn, SHELLDETAILS* psd)
{
	if (!psd)
		return E_POINTER;

	SHCOLUMNID scid;
	HRESULT hr = MapColumnToSCID(iColumn, &scid);
	if (FAILED(hr))
		return hr;

	CMyStringW str;
	if (!pidl)
	{
		switch (scid.pid)
		{
		case PID_ROOTITEM_NAME:
			psd->fmt = LVCFMT_LEFT;
			psd->cxChar = 20;
			break;
		case PID_ROOTITEM_MODE:
			psd->fmt = LVCFMT_LEFT;
			psd->cxChar = 6;
			break;
		case PID_ROOTITEM_HOST:
			psd->fmt = LVCFMT_LEFT;
			psd->cxChar = 10;
			break;
		case PID_ROOTITEM_PORT:
			psd->fmt = LVCFMT_RIGHT;
			psd->cxChar = 4;
			break;
		case PID_ROOTITEM_STATUS:
			psd->fmt = LVCFMT_LEFT;
			psd->cxChar = 6;
			break;
		default:
			return E_INVALIDARG;
		}
		psd->str.uType = STRRET_WSTR;
		str.LoadString(MY_PID_TO_STRING_ID(scid.pid));
		psd->str.pOleStr = DuplicateCoMemString(str);
		if (!psd->str.pOleStr)
			return E_OUTOFMEMORY;
		return S_OK;
	}

	WORD wID;
	CSFTPHostItem UNALIGNED* pItem = NULL;
	CHostFolderData* pHostData;
	bool bFound = false;
	if (::PickupRootCommandItemID(pidl, wID))
	{
		str.LoadString((UINT)wID);
	}
	else
	{
		if (!::PickupHostName(pidl, str))
			return E_INVALIDARG;
		pItem = (CSFTPHostItem UNALIGNED*) pidl;

		for (int i = 0; i < theApp.m_aHosts.GetCount(); i++)
		{
			pHostData = theApp.m_aHosts.GetItem(i);
			if (pHostData->ConnectionMode == pItem->GetConnectionMode() &&
				pHostData->pDirItem->strName.Compare(str) == 0 &&
				pHostData->nPort == (int)pItem->nPort)
			{
				bFound = true;
				break;
			}
		}
	}

	psd->cxChar = 0;
	switch (scid.pid)
	{
	case PID_ROOTITEM_NAME:
		if (bFound && pHostData->pSettings)
			str = pHostData->pSettings->strDisplayName;
		psd->fmt = LVCFMT_LEFT;
		psd->str.uType = STRRET_WSTR;
		psd->str.pOleStr = DuplicateCoMemString(str);
		if (!psd->str.pOleStr)
			return E_OUTOFMEMORY;
		break;
	case PID_ROOTITEM_MODE:
		if (pItem)
			str.LoadString(pItem->ConnectionMode + IDS_CONNECTMODE_SFTP);
		else
			str.Empty();
		psd->fmt = LVCFMT_LEFT;
		psd->str.uType = STRRET_WSTR;
		psd->str.pOleStr = DuplicateCoMemString(str);
		if (!psd->str.pOleStr)
			return E_OUTOFMEMORY;
		break;
	case PID_ROOTITEM_HOST:
		if (!pItem)
			str.Empty();
		psd->fmt = LVCFMT_LEFT;
		psd->str.uType = STRRET_WSTR;
		psd->str.pOleStr = DuplicateCoMemString(str);
		if (!psd->str.pOleStr)
			return E_OUTOFMEMORY;
		break;
	case PID_ROOTITEM_PORT:
		if (pItem)
			str.Format("%u", (UINT)pItem->nPort);
		else
			str.Empty();
		psd->fmt = LVCFMT_LEFT;
		psd->str.uType = STRRET_WSTR;
		psd->str.pOleStr = DuplicateCoMemString(str);
		if (!psd->str.pOleStr)
			return E_OUTOFMEMORY;
		break;
	case PID_ROOTITEM_STATUS:
	{
		bool bConnected;
		bConnected = (bFound && pHostData->pDirItem->pDirectory != NULL &&
			pHostData->pDirItem->pDirectory->IsConnected() == S_OK);
		str.LoadString(bConnected ? IDS_CONNECTED : IDS_NOT_CONNECTED);
		psd->fmt = LVCFMT_LEFT;
		psd->str.uType = STRRET_WSTR;
		psd->str.pOleStr = DuplicateCoMemString(str);
		if (!psd->str.pOleStr)
			return E_OUTOFMEMORY;
	}
	break;
	default:
		return E_INVALIDARG;
	}

	return S_OK;
}

STDMETHODIMP CEasySFTPFolderRoot::MapColumnToSCID(UINT iColumn, SHCOLUMNID* pscid)
{
	if (!pscid)
		return E_POINTER;
	if (iColumn >= sizeof(s_uHeaderRootItemPIDs) / sizeof(s_uHeaderRootItemPIDs[0]))
		return E_FAIL;
	pscid->fmtid = GUID_RootItemColumn;
	pscid->pid = s_uHeaderRootItemPIDs[iColumn];
	return S_OK;
}

STDMETHODIMP CEasySFTPFolderRoot::GetClassID(CLSID* pClassID)
{
	if (!pClassID)
		return E_POINTER;
	*pClassID = CLSID_EasySFTPOld;
	return S_OK;
}


STDMETHODIMP CEasySFTPFolderRoot::InitNew()
{
	return S_OK;
}

STDMETHODIMP CEasySFTPFolderRoot::Load(IPropertyBag*, IErrorLog*)
{
	// do nothing
	return S_OK;
}

STDMETHODIMP CEasySFTPFolderRoot::Save(IPropertyBag*, BOOL, BOOL)
{
	// do nothing
	return S_OK;
}


STDMETHODIMP CEasySFTPFolderRoot::GetIconOf(PCUITEMID_CHILD pidl, UINT flags, int* pIconIndex)
{
	if (!pIconIndex)
		return E_POINTER;

	WORD wID;
	if (::PickupRootCommandItemID(pidl, wID))
	{
		//iIndex = 0;
		if (wID == ID_RCOMMAND_ADD_HOST)
		{
			auto i = theApp.m_iNewHostIconIndex[CMainDLL::iconIndexSysSmall];
			if (i < 0)
				i = theApp.m_iNewHostIconIndex[CMainDLL::iconIndexSmall];
			*pIconIndex = i;
			return S_OK;
		}
		return S_FALSE;
	}
	else
	{
		//CSFTPHostItem UNALIGNED* pItem;
		CMyStringW strHost;
		if (!::PickupHostName(pidl, strHost))
			return E_INVALIDARG;
		//pItem = (CSFTPHostItem UNALIGNED*) pidl;

		auto i = theApp.m_iNetDriveIconIndex[CMainDLL::iconIndexSysSmall];
		if (i < 0)
			i = theApp.m_iNetDriveIconIndex[CMainDLL::iconIndexSmall];
		*pIconIndex = i;
		return S_OK;
		//return S_FALSE;
	}
}


STDMETHODIMP CEasySFTPFolderRoot::SetItemAlloc(IMalloc* pMalloc)
{
	if (m_pMallocData->pMalloc)
		m_pMallocData->pMalloc->Release();
	m_pMallocData->pMalloc = pMalloc;
	pMalloc->AddRef();
	return S_OK;
}

STDMETHODIMP CEasySFTPFolderRoot::get_Version(BSTR* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = ::SysAllocString(VERSION_STRING_W);
	return *pRet ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CEasySFTPFolderRoot::_Connect(VARIANT_BOOL bSFTP, LONG_PTR hWnd, LONG_PTR pvReserved,
	BSTR lpszHostName, long nPort, IEasySFTPRootDirectory** ppFolder)
{
	IShellFolder* pFolder = NULL;
	auto hr = Connect(bSFTP, reinterpret_cast<HWND>(hWnd), NULL, lpszHostName, nPort, &pFolder);
	if (SUCCEEDED(hr))
	{
		if (!pFolder)
			hr = E_NOINTERFACE;
		else
		{
			hr = pFolder->QueryInterface(IID_IEasySFTPRootDirectory, reinterpret_cast<void**>(ppFolder));
			pFolder->Release();
		}
	}
	return hr;
}

STDMETHODIMP CEasySFTPFolderRoot::QuickConnectDialog(LONG_PTR hWndOwner, IEasySFTPRootDirectory** ppFolder)
{
	IShellFolder* pFolder = NULL;
	auto hr = QuickConnectDialog(reinterpret_cast<HWND>(hWndOwner), &pFolder);
	if (SUCCEEDED(hr))
	{
		if (!pFolder)
			hr = E_NOINTERFACE;
		else
		{
			hr = pFolder->QueryInterface(IID_IEasySFTPRootDirectory, reinterpret_cast<void**>(ppFolder));
			pFolder->Release();
		}
	}
	return hr;
}

STDMETHODIMP CEasySFTPFolderRoot::Connect(EasySFTPConnectionMode ConnectionMode, LONG_PTR hWnd, IEasySFTPAuthentication* pAuth,
	BSTR lpszHostName, long nPort, VARIANT_BOOL bIgnoreFingerprint, IEasySFTPRootDirectory** ppFolder)
{
	PITEMID_CHILD pidlChild = ::CreateHostItem(m_pMallocData->pMalloc,
		ConnectionMode, (WORD)nPort, lpszHostName);
	if (!pidlChild)
		return E_OUTOFMEMORY;
	CFTPDirectoryRootBase* pRoot;
	HRESULT hr = _BindToObject(reinterpret_cast<HWND>(hWnd), pidlChild, NULL, NULL, &pRoot);
	::CoTaskMemFree(pidlChild);
	if (FAILED(hr))
		return hr;


	hr = pRoot->QueryInterface(IID_IEasySFTPRootDirectory, reinterpret_cast<void**>(ppFolder));
	pRoot->Release();
	return hr;
}

//STDMETHODIMP CEasySFTPFolderRoot::SetListener(IEasySFTPListener* pListener)
//{
//	if (m_pListener)
//		m_pListener->Release();
//	m_pListener = pListener;
//	if (pListener)
//		pListener->AddRef();
//	return S_OK;
//}

STDMETHODIMP CEasySFTPFolderRoot::Connect(VARIANT_BOOL bSFTP, HWND hWnd, const void* pvReserved,
	LPCWSTR lpszHostName, int nPort, IShellFolder** ppFolder)
{
	IEasySFTPRootDirectory* p = NULL;
	BSTR bstrHost = ::SysAllocString(lpszHostName);
	if (!bstrHost)
		return E_OUTOFMEMORY;
	auto hr = Connect(bSFTP ? EasySFTPConnectionMode::SFTP : EasySFTPConnectionMode::FTP,
		reinterpret_cast<LONG_PTR>(hWnd), NULL, bstrHost, nPort, VARIANT_FALSE, &p);
	::SysFreeString(bstrHost);
	if (FAILED(hr))
		return hr;
	if (!p)
		return E_OUTOFMEMORY;

	CFTPDirectoryBase* pFolderBase = NULL;
	hr = p->QueryInterface(IID_CFTPDirectoryBase, reinterpret_cast<void**>(&pFolderBase));
	p->Release();
	if (FAILED(hr))
		return hr;
	auto* pRoot = pFolderBase->GetRoot();
	pRoot->AddRef();
	{
		CMyStringW strRealPath;
		CFTPDirectoryBase* pDirectory;
		if (!pRoot->ValidateDirectory(L".", NULL, strRealPath) || strRealPath.Compare(L".") == 0)
		{
			pDirectory = pRoot;
		}
		else
		{
			LPCWSTR lpw = strRealPath;
			if (*lpw == L'/')
				lpw++;
			hr = pRoot->OpenNewDirectory(lpw, &pDirectory);
			if (FAILED(hr))
				pDirectory = pRoot;
			else
				pRoot->Release();
		}
		*ppFolder = pDirectory;
	}
	pFolderBase->Release();
	return S_OK;
}

STDMETHODIMP CEasySFTPFolderRoot::QuickConnectDialog(HWND hWndOwner, IShellFolder** ppFolder)
{
	if (!ppFolder)
		return E_POINTER;
	CAuthentication* pUser = new CAuthentication();
	*ppFolder = NULL;
	if (!ConnectDialog(hWndOwner, pUser))
	{
		pUser->Release();
		return S_FALSE;
	}
	PITEMID_CHILD pidlChild = ::CreateHostItem(m_pMallocData->pMalloc,
		m_dlgConnect.m_ConnectionMode, (WORD)m_dlgConnect.m_nPort, m_dlgConnect.m_strHostName);
	if (!pidlChild)
	{
		pUser->Release();
		return E_OUTOFMEMORY;
	}
	CFTPDirectoryRootBase* pRoot;
	HRESULT hr = _BindToObject(hWndOwner, pidlChild, NULL, pUser, &pRoot);
	pUser->Release();
	::CoTaskMemFree(pidlChild);
	if (FAILED(hr))
		return hr;

	{
		CFTPDirectoryBase* pDirectory;
		if (FAILED(_RetrieveDirectory(pRoot, L".", &pDirectory)))
			pDirectory = pRoot;
		else
			pRoot->Release();
		*ppFolder = pDirectory;
	}

	return S_OK;
}

STDMETHODIMP CEasySFTPFolderRoot::GetDependencyLibraryInfo(BSTR* poutLibraryInfo)
{
	if (!poutLibraryInfo)
		return E_POINTER;

	CMyStringW str;

	str = L"* OpenSSL: ";
	str += OPENSSL_VERSION_TEXT;
	str += L"\r\n";

	str += L"* libssh2: ";
	str += LIBSSH2_VERSION;
	str += L"\r\n";

	str += L"* containing part of PuTTY source code\r\n";

	*poutLibraryInfo = ::SysAllocStringLen(str, static_cast<UINT>(str.GetLength()));
	return *poutLibraryInfo ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CEasySFTPFolderRoot::CreateAuthentication(EasySFTPAuthenticationMode mode, BSTR bstrUserName, IEasySFTPAuthentication** ppAuth)
{
	if (!ppAuth)
		return E_POINTER;
	CAuthentication* p = new CAuthentication();
	if (!p)
		return E_OUTOFMEMORY;
	auto hr = p->put_Type(mode);
	if (FAILED(hr))
	{
		p->Release();
		return hr;
	}
	hr = p->put_UserName(bstrUserName);
	if (FAILED(hr))
	{
		p->Release();
		return hr;
	}
	*ppAuth = p;
	return S_OK;
}

STDMETHODIMP CEasySFTPFolderRoot::get_HostSettings(IEasySFTPHostSettingList** ppList)
{
	if (!ppList)
		return E_POINTER;
	*ppList = new CEasySFTPHostSettingList(this);
	return *ppList ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CEasySFTPFolderRoot::CreateHostSetting(IEasySFTPHostSetting** ppRet)
{
	if (!ppRet)
		return E_POINTER;
	*ppRet = new CEasySFTPHostSetting();
	return *ppRet ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CEasySFTPFolderRoot::ConnectFromSetting(LONG_PTR hWnd, IEasySFTPHostSetting* pSetting, VARIANT_BOOL bIgnoreFingerprint, IEasySFTPDirectory** ppFolder)
{
	if (!ppFolder)
		return E_POINTER;
	auto hWndOwner = reinterpret_cast<HWND>(hWnd);
	BSTR bstr;
	CMyStringW strHostName, strServerFolder;
	long nPort;
	EasySFTPConnectionMode mode;
	auto hr = pSetting->get_HostName(&bstr);
	if (FAILED(hr))
		return hr;
	MyBSTRToString(bstr, strHostName);
	::SysFreeString(bstr);

	hr = pSetting->get_InitServerPath(&bstr);
	if (FAILED(hr))
		return hr;
	MyBSTRToString(bstr, strServerFolder);
	::SysFreeString(bstr);

	hr = pSetting->get_ConnectionMode(&mode);
	if (FAILED(hr))
		return hr;

	hr = pSetting->get_Port(&nPort);
	if (FAILED(hr))
		return hr;

	PITEMID_CHILD pidlNew = ::CreateHostItem(m_pMallocData->pMalloc,
		mode, (WORD)nPort, strHostName);

	CAuthentication* pUser = new CAuthentication();
	if (DoRetryAuthentication(hWndOwner, pUser, mode, NULL, true) <= 0)
	{
		pUser->Release();
		::CoTaskMemFree(pidlNew);
		return E_ABORT;
	}
	CFTPDirectoryRootBase* pRoot;
	hr = _BindToObject(hWndOwner, pidlNew, NULL, pUser, &pRoot);
	pUser->Release();
	if (FAILED(hr))
		return hr;

	CFTPDirectoryBase* pDirectory;
	hr = _RetrieveDirectory(pRoot, strServerFolder, &pDirectory);
	if (SUCCEEDED(hr))
	{
		hr = pDirectory->QueryInterface(IID_IEasySFTPDirectory, reinterpret_cast<void**>(ppFolder));
		pDirectory->Release();
	}
	pRoot->Release();

	return hr;
}

STDMETHODIMP CEasySFTPFolderRoot::SetEmulateRegMode(bool bEmulate)
{
	theApp.m_bEmulateRegMode = bEmulate;
	return S_OK;
}

STDMETHODIMP_(void) CEasySFTPFolderRoot::UpdateItem(PCUITEMID_CHILD pidlOld, PCUITEMID_CHILD pidlNew, LONG lEvent)
{
	//WORD wID;
	//CSFTPHostItem UNALIGNED* pItemOld = NULL;
	//CSFTPHostItem UNALIGNED* pItemNew = NULL;
	//CHostFolderData* pHostData;
	//bool bFound = false;
	//if (pidlOld && ::PickupRootCommandItemID(pidlOld, wID))
	//	return;
	//if (pidlNew && ::PickupRootCommandItemID(pidlNew, wID))
	//	return;

	//CMyStringW strOld, strNew;
	//if (pidlOld)
	//{
	//	if (!::PickupHostName(pidlOld, strOld))
	//		return;
	//	pItemOld = (CSFTPHostItem UNALIGNED*) pidlOld;
	//}
	//if (pidlNew)
	//{
	//	if (!::PickupHostName(pidlNew, strNew))
	//		return;
	//	pItemNew = (CSFTPHostItem UNALIGNED*) pidlNew;
	//}

	theApp.UpdateHostSettings(NULL, NULL, -1);

	//CHostFolderData* pHostData;
	//switch (lEvent)
	//{
	//	case SHCNE_MKDIR:
	//		pHostData = new CHostFolderData();
	//		pHostData->bSFTPMode = pItemNew->bSFTP;
	//		pHostData->directory.strName = strNew;
	//		pHostData->directory.pDirectory = NULL;
	//		pHostData->nPort = pItemNew->nPort;
	//		break;
	//}
}

bool CEasySFTPFolderRoot::ConnectDialog(HWND hWndOwner, IEasySFTPAuthentication* pUser)
{
	m_dlgConnect.SetDialogMode(false);
	if (m_dlgConnect.ModalDialogW(hWndOwner) == IDOK)
	{
		if (SUCCEEDED(m_dlgConnect.SetToAuthentication(pUser)))
			return true;
	}
	return false;
}

int CEasySFTPFolderRoot::DoRetryAuthentication(HWND hWndOwner, IEasySFTPAuthentication* pUser, EasySFTPConnectionMode mode, const char* pszAuthList, bool bFirstAttempt)
{
	//if (m_pListener)
	//{
	//	HRESULT hr = m_pListener->Authenticate(VARIANT_TRUE, pUser, pszAuthList, bFirstAttempt ? VARIANT_TRUE : VARIANT_FALSE);
	//	if (FAILED(hr))
	//		return -1;
	//	return hr == S_OK ? 1 : 0;
	//}

	if ((m_dlgConnect.m_ConnectionMode = mode) == EasySFTPConnectionMode::SFTP)
	{
		bool bAvailable = true;
		if (pszAuthList)
		{
			m_dlgConnect.m_bDisableAuthPassword = true;
			m_dlgConnect.m_bDisableAuthPublicKey = true;
			bAvailable = false;
			while (*pszAuthList)
			{
				if (_stricmp(pszAuthList, "password") == 0)
				{
					m_dlgConnect.m_bDisableAuthPassword = false;
					bAvailable = true;
				}
				else if (_stricmp(pszAuthList, "publickey") == 0)
				{
					m_dlgConnect.m_bDisableAuthPublicKey = false;
					bAvailable = true;
				}
				while (*pszAuthList++);
			}
		}
		else
		{
			m_dlgConnect.m_bDisableAuthPassword = false;
			m_dlgConnect.m_bDisableAuthPublicKey = false;
			bAvailable = true;
		}
		// If the authentication methods EasySFTP supports are not found, return error
		if (!bAvailable)
			return -1;
	}
	m_dlgConnect.SetDialogMode(true);
	if (!bFirstAttempt)
	{
		::MessageBeep(MB_ICONEXCLAMATION);
		m_dlgConnect.m_strMessage.LoadString(IDS_AUTH_FAILED);
	}
	if (m_dlgConnect.ModalDialogW(hWndOwner) == IDOK)
	{
		if (SUCCEEDED(m_dlgConnect.SetToAuthentication(pUser)))
			return 1;
	}
	return 0;
}

HRESULT CEasySFTPFolderRoot::OpenWithConnectDialog(HWND hWndOwner, PCUITEMID_CHILD pidl, CFTPDirectoryRootBase** ppRoot)
{
	CMyStringW strHostName;
	if (!::PickupHostName(pidl, strHostName))
		return E_INVALIDARG;
	CSFTPHostItem UNALIGNED* pItem = (CSFTPHostItem UNALIGNED*) pidl;

	CAuthentication* pUser = new CAuthentication();
	if (DoRetryAuthentication(hWndOwner, pUser, pItem->GetConnectionMode(), NULL, true) <= 0)
	{
		pUser->Release();
		return E_ABORT;
	}
	HRESULT hr = _BindToObject(hWndOwner, pidl, NULL, pUser, ppRoot);
	pUser->Release();
	return hr;
}

HRESULT CEasySFTPFolderRoot::AddHostSettings(CEasySFTPHostSetting* pSettings)
{
	theApp.UpdateHostSettings(NULL, pSettings, 1);

	PITEMID_CHILD pidlNew = ::CreateHostItem(m_pMallocData->pMalloc,
		pSettings->ConnectionMode, (WORD)pSettings->nPort, pSettings->strHostName);
	NotifyUpdate(SHCNE_MKDIR, pidlNew, NULL);
	::CoTaskMemFree(pidlNew);
	return S_OK;
}

HRESULT CEasySFTPFolderRoot::RemoveHostSettings(CEasySFTPHostSetting* pSettings)
{
	theApp.UpdateHostSettings(pSettings, NULL, 2);

	PITEMID_CHILD pidlNew = ::CreateHostItem(m_pMallocData->pMalloc,
		pSettings->ConnectionMode, (WORD)pSettings->nPort, pSettings->strHostName);
	NotifyUpdate(SHCNE_RMDIR, pidlNew, NULL);
	::CoTaskMemFree(pidlNew);
	return S_OK;
}

HRESULT CEasySFTPFolderRoot::_RetrieveDirectory(CFTPDirectoryRootBase* pRoot, LPCWSTR lpszPath, CFTPDirectoryBase** ppDirectory)
{
	CMyStringW strRealPath;
	if (!pRoot->ValidateDirectory(lpszPath, NULL, strRealPath) || strRealPath.IsEmpty() ||
		((LPCWSTR)strRealPath)[0] != L'/')
	{
		return E_FAIL;
	}
	else if (strRealPath.Compare(L"/") == 0)
	{
		*ppDirectory = pRoot;
		pRoot->AddRef();
		return S_OK;
	}
	else
	{
		LPCWSTR lpw = strRealPath;
		//if (*lpw == L'/')
		lpw++;
		return pRoot->OpenNewDirectory(lpw, ppDirectory);
	}
}

////////////////////////////////////////////////////////////////////////////////

CEasySFTPRootIcon::CEasySFTPRootIcon(int iIndex)
	: m_uRef(1)
	, m_iIndex(iIndex)
{
}

CEasySFTPRootIcon::~CEasySFTPRootIcon()
{
}

STDMETHODIMP CEasySFTPRootIcon::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;

	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IExtractIconW))
	{
		*ppv = static_cast<IExtractIconW*>(this);
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid, IID_IExtractIconA))
	{
		*ppv = static_cast<IExtractIconA*>(this);
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CEasySFTPRootIcon::AddRef()
{
	return ::InterlockedIncrement(&m_uRef);
}

STDMETHODIMP_(ULONG) CEasySFTPRootIcon::Release()
{
	ULONG u = ::InterlockedDecrement(&m_uRef);
	if (!u)
		delete this;
	return u;
}

STDMETHODIMP CEasySFTPRootIcon::GetIconLocation(UINT uFlags, LPWSTR pszIconFile, UINT cchMax, int* piIndex, UINT* pwFlags)
{
	if (!piIndex)
		return E_POINTER;
	if (cchMax > 0)
	{
		if (!pszIconFile)
			return E_POINTER;
		if (cchMax >= 3)
			cchMax = 3;
		memcpy(pszIconFile, L"#0", sizeof(WCHAR) * (cchMax - 1));
		pszIconFile[cchMax - 1] = 0;
	}
	//if (m_bConnected)
	//	*piIndex = 1;
	//else
	*piIndex = 0;
	if (pwFlags)
		*pwFlags = GIL_DONTCACHE | GIL_NOTFILENAME;
	return S_OK;
}

static HRESULT __stdcall _ExtractIconImpl(int iIndex, WORD iconSize, HICON* phIcon)
{
	if (iconSize <= 16)
		*phIcon = ::ImageList_GetIcon(theApp.m_himlIconSmall, iIndex, ILD_TRANSPARENT);
	else if (iconSize <= 32)
		*phIcon = ::ImageList_GetIcon(theApp.m_himlIconLarge, iIndex, ILD_TRANSPARENT);
	else if (iconSize <= 48)
		*phIcon = ::ImageList_GetIcon(theApp.m_himlIconExtraLarge, iIndex, ILD_TRANSPARENT);
	else if (iconSize <= 256)
		*phIcon = ::ImageList_GetIcon(theApp.m_himlIconJumbo, iIndex, ILD_TRANSPARENT);
	else
	{
		*phIcon = NULL;
		return E_NOTIMPL;
	}
	if (!*phIcon)
		return E_OUTOFMEMORY;
	return S_OK;
}

HRESULT CEasySFTPRootIcon::DoExtract(HICON* phIconLarge, HICON* phIconSmall, UINT nIconSize)
{
	if (phIconLarge)
	{
		auto hr = _ExtractIconImpl(m_iIndex, LOWORD(nIconSize), phIconLarge);
		if (FAILED(hr))
			return hr;
	}
	if (phIconSmall)
	{
		auto hr = _ExtractIconImpl(m_iIndex, HIWORD(nIconSize), phIconSmall);
		if (FAILED(hr))
		{
			if (phIconLarge)
			{
				::DestroyIcon(*phIconLarge);
				*phIconLarge = NULL;
			}
			return hr;
		}
	}
	return S_OK;
}

STDMETHODIMP CEasySFTPRootIcon::Extract(LPCWSTR pszFile, UINT nIconIndex, HICON* phIconLarge, HICON* phIconSmall, UINT nIconSize)
{
	if (!pszFile)
		return E_POINTER;
	if (wcscmp(pszFile, L"#0") != 0)
		return E_INVALIDARG;
	return DoExtract(phIconLarge, phIconSmall, nIconSize);
}

STDMETHODIMP CEasySFTPRootIcon::GetIconLocation(UINT uFlags, LPSTR pszIconFile, UINT cchMax, int* piIndex, UINT* pwFlags)
{
	if (!piIndex)
		return E_POINTER;
	if (cchMax > 0)
	{
		if (!pszIconFile)
			return E_POINTER;
		if (cchMax >= 3)
			cchMax = 3;
		memcpy(pszIconFile, "#0", sizeof(CHAR) * (cchMax - 1));
		pszIconFile[cchMax - 1] = 0;
	}
	//if (m_bConnected)
	//	*piIndex = 1;
	//else
	*piIndex = 0;
	if (pwFlags)
		*pwFlags = GIL_DONTCACHE;
	return S_OK;
}

STDMETHODIMP CEasySFTPRootIcon::Extract(LPCSTR pszFile, UINT nIconIndex, HICON* phIconLarge, HICON* phIconSmall, UINT nIconSize)
{
	if (!pszFile)
		return E_POINTER;
	if (strcmp(pszFile, "#0") != 0)
		return E_INVALIDARG;
	return DoExtract(phIconLarge, phIconSmall, nIconSize);
}

////////////////////////////////////////////////////////////////////////////////

CEasySFTPRootMenu::CEasySFTPRootMenu(CEasySFTPFolderRoot* pRoot,
	IShellBrowser* pBrowser,
	PCIDLIST_ABSOLUTE pidlMe,
	PCUITEMID_CHILD_ARRAY apidl,
	int nCount)
	: m_uRef(1)
	, m_pRoot(pRoot)
	, m_pBrowser(pBrowser)
	, m_nCount(nCount)
{
	m_pidlMe = (PIDLIST_ABSOLUTE) ::DuplicateItemIDList(pidlMe);
	m_apidl = (PIDLIST_RELATIVE*)malloc(sizeof(PIDLIST_RELATIVE) * nCount);
	m_wIDCommand = 0;
	for (int i = 0; i < nCount; i++)
	{
		WORD w;
		if (::PickupRootCommandItemID(apidl[i], w))
		{
			if (i == 0)
				m_wIDCommand = w;
		}
		//m_apidl[i] = (PIDLIST_RELATIVE) ::DuplicateItemIDList((PCUIDLIST_ABSOLUTE) apidl[i]);
		m_apidl[i] = (PIDLIST_RELATIVE)apidl[i];
	}
	if (pRoot)
		pRoot->AddRef();
	if (pBrowser)
		pBrowser->AddRef();
	m_pUnkSite = NULL;
}

CEasySFTPRootMenu::~CEasySFTPRootMenu()
{
	if (m_pUnkSite)
		m_pUnkSite->Release();
	//for (int i = 0; i < m_nCount; i++)
	//	::CoTaskMemFree(m_apidl[i]);
	free(m_apidl);
	::CoTaskMemFree(m_pidlMe);
	if (m_pBrowser)
		m_pBrowser->Release();
	if (m_pRoot)
		m_pRoot->Release();
}

STDMETHODIMP CEasySFTPRootMenu::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;

	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IContextMenu))
	{
		*ppv = static_cast<IContextMenu*>(this);
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid, IID_IObjectWithSite))
	{
		*ppv = static_cast<IObjectWithSite*>(this);
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CEasySFTPRootMenu::AddRef()
{
	return ::InterlockedIncrement(&m_uRef);
}

STDMETHODIMP_(ULONG) CEasySFTPRootMenu::Release()
{
	ULONG u = ::InterlockedDecrement(&m_uRef);
	if (!u)
		delete this;
	return u;
}

STDMETHODIMP CEasySFTPRootMenu::QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	int i, nCount;
	MENUITEMINFO mii;
	UINT uMaxID;
	CMyStringW str;

#ifdef _WIN64
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_ID | MIIM_STATE;
#else
	mii.cbSize = MENUITEMINFO_SIZE_V1;
	mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
#endif
	uMaxID = idCmdFirst;
	if (m_wIDCommand != 0)
	{
		str.LoadString((UINT)m_wIDCommand);
		mii.fType = MFT_STRING;
		mii.fState = (m_nCount == 1 ? MFS_ENABLED : MFS_GRAYED);
		if (!(uFlags & CMF_NODEFAULT))
			mii.fState |= MFS_DEFAULT;
		mii.wID = idCmdFirst;
		mii.dwTypeData = (LPTSTR)(LPCTSTR)str;
		::InsertMenuItem(hMenu, indexMenu++, TRUE, &mii);
	}
	else
	{
		HMENU h = ::GetSubMenu(theApp.m_hMenuContext, CXMENU_POPUP_HOST);
		nCount = ::GetMenuItemCount(h);
		for (i = 0; i < nCount; i++)
		{
			mii.cch = MAX_PATH;
#ifdef _UNICODE
			mii.dwTypeData = str.GetBufferW(MAX_PATH);
#else
			mii.dwTypeData = str.GetBufferA(MAX_PATH);
#endif
			::GetMenuItemInfo(h, (UINT)i, TRUE, &mii);
			if (uFlags & CMF_DEFAULTONLY)
			{
				if (mii.wID != ID_HOST_CONNECT)
					continue;
			}
			if (uFlags & CMF_DVFILE)
			{
				if (mii.wID == ID_HOST_RENAME || mii.wID == ID_HOST_DELETE ||
					mii.wID == ID_HOST_PROPERTY)
					continue;
			}
			if (!(uFlags & CMF_CANRENAME))
			{
				if (mii.wID == ID_HOST_RENAME)
					continue;
			}
			if (!(uFlags & CMF_NODEFAULT) && mii.wID == ID_HOST_CONNECT)
				mii.fState |= MFS_DEFAULT;
			mii.wID = (WORD)((UINT)mii.wID - ID_HOST_BASE + idCmdFirst);
			if (uMaxID < (UINT)mii.wID)
				uMaxID = (UINT)mii.wID;
			::InsertMenuItem(hMenu, indexMenu++, TRUE, &mii);
		}
	}
	return MAKE_HRESULT(SEVERITY_SUCCESS, 0, uMaxID - idCmdFirst + 1);
}

STDMETHODIMP CEasySFTPRootMenu::InvokeCommand(CMINVOKECOMMANDINFO* pici)
{
	CMINVOKECOMMANDINFOEX* piciex = (CMINVOKECOMMANDINFOEX*)pici;
	UINT uID;
	if (piciex->cbSize == sizeof(CMINVOKECOMMANDINFOEX) && (piciex->fMask & CMIC_MASK_UNICODE))
	{
		if (!HIWORD(piciex->lpVerbW))
		{
			if (!piciex->lpVerbW)
			{
				if (m_wIDCommand)
					uID = LOWORD(piciex->lpVerb) + m_wIDCommand;
				else
					uID = LOWORD(piciex->lpVerb) + ID_HOST_BASE;
			}
			else
			{
				if (m_wIDCommand)
					uID = LOWORD(piciex->lpVerbW) + m_wIDCommand;
				else
					uID = LOWORD(piciex->lpVerbW) + ID_HOST_BASE;
			}
		}
		else
		{
			if (_wcsicmp(piciex->lpVerbW, L"open") == 0 || _wcsicmp(piciex->lpVerbW, L"connect") == 0)
			{
				if (m_wIDCommand)
					uID = m_wIDCommand;
				else
					uID = ID_HOST_CONNECT;
			}
			else if (_wcsicmp(piciex->lpVerbW, L"rename") == 0)
				uID = ID_HOST_RENAME;
			else if (_wcsicmp(piciex->lpVerbW, L"delete") == 0)
				uID = ID_HOST_DELETE;
			else if (_wcsicmp(piciex->lpVerbW, L"properties") == 0)
				uID = ID_HOST_PROPERTY;
			else
				return E_INVALIDARG;
		}
	}
	else
	{
		if (!HIWORD(piciex->lpVerb))
		{
			if (m_wIDCommand)
				uID = LOWORD(piciex->lpVerb) + m_wIDCommand;
			else
				uID = LOWORD(piciex->lpVerb) + ID_HOST_BASE;
		}
		else
		{
			if (_stricmp(piciex->lpVerb, "open") == 0 || _stricmp(piciex->lpVerb, "connect") == 0)
			{
				if (m_wIDCommand)
					uID = m_wIDCommand;
				else
					uID = ID_HOST_CONNECT;
			}
			else if (_stricmp(piciex->lpVerb, "rename") == 0)
				uID = ID_HOST_RENAME;
			else if (_stricmp(piciex->lpVerb, "delete") == 0)
				uID = ID_HOST_DELETE;
			else if (_stricmp(piciex->lpVerb, "properties") == 0)
				uID = ID_HOST_PROPERTY;
			else
				return E_INVALIDARG;
		}
	}

	if (m_wIDCommand)
	{
		if ((UINT)m_wIDCommand != uID)
			return E_INVALIDARG;
		switch (uID)
		{
		case ID_RCOMMAND_ADD_HOST:
		{
			HWND hWnd = piciex->hwnd;
			if (!hWnd && m_pBrowser)
				m_pBrowser->GetWindow(&hWnd);
			DoAdd(hWnd);
		}
		break;
		default:
			return E_INVALIDARG;
		}
	}
	else
	{
		HWND hWnd = piciex->hwnd;
		if (!hWnd && m_pBrowser)
			m_pBrowser->GetWindow(&hWnd);
		switch (uID)
		{
		case ID_HOST_CONNECT:
		{
			IShellBrowser* pBrowser;
			IServiceProvider* pService;
			pBrowser = m_pBrowser;
			if (piciex->hwnd)
				pBrowser = (IShellBrowser*) ::SendMessage(piciex->hwnd, CWM_GETISHELLBROWSER, 0, 0);
			if (pBrowser)
			{
				if (SUCCEEDED(pBrowser->QueryInterface(IID_IServiceProvider, (void**)&pService)))
				{
					if (FAILED(pService->QueryService(SID_SInPlaceBrowser, IID_IShellBrowser, (void**)&pBrowser)))
					{
						if (FAILED(pService->QueryService(SID_SShellBrowser, IID_IShellBrowser, (void**)&pBrowser)))
						{
							if (FAILED(pService->QueryService(SID_STopLevelBrowser, IID_IShellBrowser, (void**)&pBrowser)))
							{
								pBrowser = m_pBrowser;
								if (pBrowser)
									pBrowser->AddRef();
							}
						}
					}
					pService->Release();
				}
				else
					pBrowser->AddRef();
			}
			if (!pBrowser)
			{
				SHELLEXECUTEINFO sei;
				memset(&sei, 0, sizeof(sei));
				sei.cbSize = sizeof(sei);
				sei.fMask = SEE_MASK_IDLIST;
				sei.lpVerb = _T("open");
				for (int i = 0; i < m_nCount; i++)
				{
					sei.lpIDList = ::AppendItemIDList(m_pidlMe, m_apidl[i]);
					::ShellExecuteEx(&sei);
					::CoTaskMemFree(sei.lpIDList);
				}
			}
			else
			{
				UINT wFlags = SBSP_DEFMODE | SBSP_ABSOLUTE;
				if (piciex->fMask & CMIC_MASK_HOTKEY)
				{
					if (piciex->dwHotKey & MK_CONTROL)
						wFlags |= SBSP_NEWBROWSER;
					else
						wFlags |= SBSP_DEFBROWSER;
				}
				else
					wFlags |= SBSP_DEFBROWSER;
				LPCWSTR lpszLocal = NULL;
				PIDLIST_ABSOLUTE pidlTo = RetrieveDefaultDirectory(piciex->hwnd, m_apidl[0], &lpszLocal);
				if (pidlTo)
				{
					if (SUCCEEDED(pBrowser->BrowseObject(pidlTo, wFlags)) && lpszLocal)
					{
						IEasySFTPListener* pListener;
						if (SUCCEEDED(pBrowser->QueryInterface(IID_IEasySFTPListener, (void**)&pListener)))
						{
							pListener->ChangeLocalDirectory(lpszLocal);
							pListener->Release();
						}
					}
					::CoTaskMemFree(pidlTo);
				}
				pBrowser->Release();
			}
		}
		break;
		case ID_HOST_DELETE:
			DoDelete(hWnd);
			break;
		case ID_HOST_RENAME:
			//return E_NOTIMPL;
			break;
		case ID_HOST_PROPERTY:
			DoProperty(hWnd);
			break;
		default:
			return E_INVALIDARG;
		}
	}
	return S_OK;
}

STDMETHODIMP CEasySFTPRootMenu::GetCommandString(UINT_PTR idCmd, UINT uType, UINT* pReserved, LPSTR pszName, UINT cchMax)
{
	idCmd += ID_HOST_BASE;

	bool bUnicode = false;
	if (uType & GCS_UNICODE)
	{
		bUnicode = true;
		uType &= ~GCS_UNICODE;
	}
	CMyStringW str;
	switch (idCmd)
	{
	case ID_HOST_CONNECT:
		str = m_wIDCommand ? L"open" : L"connect";
		break;
	case ID_HOST_DELETE:
		str = L"delete";
		break;
	case ID_HOST_RENAME:
		str = L"rename";
		break;
	case ID_HOST_PROPERTY:
		str = L"properties";
		break;
	default:
		return E_NOTIMPL;
	}
	if (uType == GCS_VALIDATEA)
		return S_OK;
	else if (uType == GCS_HELPTEXTA)
	{
		if (!str.LoadString((UINT)idCmd))
			str.Empty();
	}
	UINT dw = (UINT)(bUnicode ? str.GetLength() : str.GetLengthA());
	if (cchMax > dw + 1)
		cchMax = dw + 1;
	if (bUnicode)
	{
		memcpy(pszName, (LPCWSTR)str, sizeof(WCHAR) * (cchMax - 1));
		((LPWSTR)pszName)[cchMax - 1] = 0;
	}
	else
	{
		memcpy(pszName, (LPCSTR)str, sizeof(CHAR) * (cchMax - 1));
		((LPSTR)pszName)[cchMax - 1] = 0;
	}
	return S_OK;
}

STDMETHODIMP CEasySFTPRootMenu::SetSite(IUnknown* pUnkSite)
{
	if (m_pUnkSite)
		m_pUnkSite->Release();
	m_pUnkSite = pUnkSite;
	if (pUnkSite)
	{
		pUnkSite->AddRef();

		IShellBrowser* pBrowser;
		HRESULT hr = pUnkSite->QueryInterface(IID_IShellBrowser, (void**)&pBrowser);
		if (SUCCEEDED(hr))
		{
			if (m_pBrowser)
				m_pBrowser->Release();
			m_pBrowser = pBrowser;
		}
	}
	return S_OK;
}

STDMETHODIMP CEasySFTPRootMenu::GetSite(REFIID riid, void** ppvSite)
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

void CEasySFTPRootMenu::DoAdd(HWND hWndOwner)
{
	// ページが初期化されなかった場合は true のままになる
	bool br1 = true;
	bool br2 = true;
	bool br3 = true;
	CEasySFTPHostSetting* pSettings = new CEasySFTPHostSetting();

	pSettings->ConnectionMode = EasySFTPConnectionMode::FTP;
	pSettings->nPort = 21;
	pSettings->bTextMode = 0;
	pSettings->nServerCharset = scsUTF8;
	pSettings->nTransferMode = TRANSFER_MODE_AUTO;
	pSettings->arrTextFileType.CopyArray(theApp.m_arrDefTextFileType);
	pSettings->bAdjustRecvModifyTime = pSettings->bAdjustSendModifyTime = false;
	pSettings->strChmodCommand = DEFAULT_CHMOD_COMMAND;

	CMyStringW strTitle(MAKEINTRESOURCEW(IDS_ADDHOST_PROP));
	CMyPropertySheet sht;
	sht.AddPage(new CHostGeneralSettingPage(pSettings, &br1));
	sht.AddPage(new CHostCharsetPage(pSettings, &br2));
	sht.AddPage(new CHostTransferPage(pSettings, &br3));
	sht.m_dwPSHFlags = PSH_NOAPPLYNOW;
	if (sht.Create(true, strTitle, hWndOwner) != (HWND)-1 && br1 && br2 && br3)
	{
		m_pRoot->AddHostSettings(pSettings);
	}
	pSettings->Release();
}

void CEasySFTPRootMenu::DoProperty(HWND hWndOwner)
{
	if (m_nCount != 1)
		return;

	CSFTPHostItem UNALIGNED* pItem;
	CMyStringW strHost;
	if (!::PickupHostName((PCUITEMID_CHILD)m_apidl[0], strHost))
		//return E_INVALIDARG;
		return;
	pItem = (CSFTPHostItem UNALIGNED*) m_apidl[0];

	CHostFolderData* pHostData = theApp.FindHostFolderData(pItem->GetConnectionMode(), strHost, (int)pItem->nPort);
	if (!pHostData || !pHostData->pSettings)
		return;

	// ページが初期化されなかった場合は true のままになる
	bool br1 = true;
	bool br2 = true;
	bool br3 = true;
	CEasySFTPHostSetting* pOldSettings = new CEasySFTPHostSetting(pHostData->pSettings);
	CEasySFTPHostSetting* pSettings = new CEasySFTPHostSetting(pHostData->pSettings);
	CMyStringW strTitle(MAKEINTRESOURCEW(IDS_CHANGEHOST_PROP));
	CMyPropertySheet sht;
	CHostGeneralSettingPage* pHGSPage = new CHostGeneralSettingPage(pSettings, &br1);
	pHGSPage->m_bNoModeChange = (pHostData->pDirItem->pDirectory != NULL &&
		pHostData->pDirItem->pDirectory->IsConnected() == S_OK);
	sht.AddPage(pHGSPage);
	sht.AddPage(new CHostCharsetPage(pSettings, &br2));
	sht.AddPage(new CHostTransferPage(pSettings, &br3));
	sht.m_dwPSHFlags = PSH_NOAPPLYNOW;
	if (sht.Create(true, strTitle, hWndOwner) != (HWND)-1 && br1 && br2 && br3)
	{
		pHostData->nPort = pSettings->nPort;
		pHostData->pDirItem->strName = pSettings->strHostName;
		if (pHostData->ConnectionMode != pSettings->ConnectionMode)
		{
			pHostData->ConnectionMode = pSettings->ConnectionMode;
			if (pHostData->pDirItem->pDirectory)
			{
				pHostData->pDirItem->pDirectory->Release();
				pHostData->pDirItem->pDirectory = NULL;
				if (pHostData->ConnectionMode == EasySFTPConnectionMode::SFTP)
				{
					CSFTPFolderSFTP* pSFTP = new CSFTPFolderSFTP(m_pRoot->m_pMallocData,
						pHostData->pDirItem, m_pRoot);
					if (pSFTP)
					{
						pSFTP->m_strHostName = pSettings->strHostName;
						pSFTP->m_nPort = pSettings->nPort;
						pSFTP->m_bTextMode = pSettings->bTextMode;
						pSFTP->m_nServerCharset = pSettings->nServerCharset;
						pSFTP->m_nTransferMode = pSettings->nTransferMode;
						pSFTP->m_arrTextFileType.CopyArray(pSettings->arrTextFileType);
						pSFTP->m_bUseSystemTextFileType = pSettings->bUseSystemTextFileType;
						pSFTP->m_bAdjustSendModifyTime = pSettings->bAdjustSendModifyTime;
						pSFTP->m_bAdjustRecvModifyTime = pSettings->bAdjustRecvModifyTime;
						pSFTP->m_bUseThumbnailPreview = pSettings->bUseThumbnailPreview;
					}
					pHostData->pDirItem->pDirectory = pSFTP;
				}
				else
				{
					CSFTPFolderFTP* pFTP = new CSFTPFolderFTP(m_pRoot->m_pMallocData,
						pHostData->pDirItem, m_pRoot);
					if (pFTP)
					{
						pFTP->m_strHostName = pSettings->strHostName;
						pFTP->m_nPort = pSettings->nPort;
						pFTP->m_strChmodCommand = pSettings->strChmodCommand;
						pFTP->m_bTextMode = pSettings->bTextMode;
						pFTP->m_nServerCharset = pSettings->nServerCharset;
						pFTP->m_nTransferMode = pSettings->nTransferMode;
						pFTP->m_arrTextFileType.CopyArray(pSettings->arrTextFileType);
						pFTP->m_bUseSystemTextFileType = pSettings->bUseSystemTextFileType;
						pFTP->m_bAdjustSendModifyTime = pSettings->bAdjustSendModifyTime;
						pFTP->m_bAdjustRecvModifyTime = pSettings->bAdjustRecvModifyTime;
						pFTP->m_bUseThumbnailPreview = pSettings->bUseThumbnailPreview;
					}
					pHostData->pDirItem->pDirectory = pFTP;
				}
			}
		}
		else if (pHostData->pDirItem->pDirectory)
		{
			CFTPDirectoryRootBase* pRoot = static_cast<CFTPDirectoryRootBase*>(pHostData->pDirItem->pDirectory);
			pRoot->SetChmodCommand(pSettings->strChmodCommand);
			pRoot->m_bTextMode = pSettings->bTextMode;
			pRoot->m_nServerCharset = pSettings->nServerCharset;
			pRoot->m_nTransferMode = pSettings->nTransferMode;
			pRoot->m_arrTextFileType.CopyArray(pSettings->arrTextFileType);
			pRoot->m_bUseSystemTextFileType = pSettings->bUseSystemTextFileType;
			pRoot->m_bAdjustSendModifyTime = pSettings->bAdjustSendModifyTime;
			pRoot->m_bAdjustRecvModifyTime = pSettings->bAdjustRecvModifyTime;
			pRoot->m_bUseThumbnailPreview = pSettings->bUseThumbnailPreview;
		}

		theApp.UpdateHostSettings(pOldSettings, pSettings, 0);

		PITEMID_CHILD pidlNew = ::CreateHostItem(m_pRoot->m_pMallocData->pMalloc,
			pSettings->ConnectionMode, (WORD)pSettings->nPort, pSettings->strHostName);
		m_pRoot->NotifyUpdate(SHCNE_RENAMEFOLDER, (PCUITEMID_CHILD)m_apidl[0], pidlNew);
		::CoTaskMemFree(pidlNew);
		//PITEMID_CHILD arr[2];
		//arr[0] = (PITEMID_CHILD) m_apidl[0];
		//arr[1] = ::CreateHostItem(m_pRoot->m_pMallocData->pMalloc,
		//	pSettings->bSFTPMode, (WORD) pSettings->nPort, pSettings->strHostName);
		//LRESULT lr = ::SHShellFolderView_Message(hWndOwner, SFVM_UPDATEOBJECT, (LPARAM) arr);
		//if (lr == (LRESULT) -1)
		//	::CoTaskMemFree(arr[1]);
		//else
		//{
		//	//::CoTaskMemFree(m_apidl[0]);
		//	//m_apidl[0] = (PIDLIST_RELATIVE) ::DuplicateItemIDList((PCUIDLIST_ABSOLUTE) arr[1]);
		//	m_apidl[0] = arr[1];
		//}
	}
	pOldSettings->Release();
	pSettings->Release();
}

void CEasySFTPRootMenu::DoDelete(HWND hWndOwner)
{
	CSFTPHostItem UNALIGNED* pItem;
	CMyStringW str;
	CMySimpleArray<PCUITEMID_CHILD> apidl;

	pItem = NULL;
	for (int i = 0; i < m_nCount; i++)
	{
		if (::PickupHostName((PCUITEMID_CHILD)m_apidl[i], str))
		{
			pItem = (CSFTPHostItem UNALIGNED*) m_apidl[i];
			CHostFolderData* pHostData = theApp.FindHostFolderData(pItem->GetConnectionMode(), str, (int)pItem->nPort);
			if (pHostData && pHostData->pSettings)
			{
				if (apidl.Add((PCUITEMID_CHILD)m_apidl[i]) != 0)
					pItem = NULL;
				else
					pItem = (CSFTPHostItem UNALIGNED*) m_apidl[i];
			}
		}
	}

	if (!apidl.GetCount())
		return;

	if (pItem)
	{
		CHostFolderData* pHostData = theApp.FindHostFolderData(pItem->GetConnectionMode(), str, (int)pItem->nPort);
		if (!pHostData || !pHostData->pSettings)
		{
			CMyStringW str2(str);
			str.Format(IDS_DELETE_HOST, (LPCWSTR)str2);
		}
		else
			str.Format(IDS_DELETE_HOST, (LPCWSTR)pHostData->pSettings->strDisplayName);
	}
	else
		str.Format(IDS_DELETE_MULTIPLE, apidl.GetCount());

	if (::MyMessageBoxW(hWndOwner, str, NULL, MB_ICONQUESTION | MB_YESNO) != IDYES)
		return;

	{
		CMyPtrArrayT<CEasySFTPHostSetting> aSettings;
		for (int i = 0, i2 = 0; i < m_nCount; i++)
		{
			if (::PickupHostName((PCUITEMID_CHILD)m_apidl[i], str))
			{
				pItem = (CSFTPHostItem UNALIGNED*) m_apidl[i];
				CHostFolderData* pHostData = theApp.FindHostFolderData(pItem->GetConnectionMode(), str, (int)pItem->nPort);
				if (pHostData && pHostData->pSettings)
				{
					aSettings.Add(pHostData->pSettings);
					pHostData->pSettings->AddRef();
				}
			}
		}
		theApp.UpdateHostSettings(aSettings, {}, 2);
		for (int i = 0; i < aSettings.GetCount(); ++i)
			aSettings[i]->Release();
	}

	for (int i = 0; i < apidl.GetCount(); i++)
		m_pRoot->NotifyUpdate(SHCNE_RMDIR, apidl[i], NULL);
}

PIDLIST_ABSOLUTE CEasySFTPRootMenu::RetrieveDefaultDirectory(HWND hWndOwner, PCUIDLIST_RELATIVE pidl, LPCWSTR* lplpszLocalDirectory)
{
	CFTPDirectoryRootBase* pRoot = NULL;
	HRESULT hr;
	CMyStringW strHost;
	if (lplpszLocalDirectory)
		*lplpszLocalDirectory = NULL;
	if (!::PickupHostName((PCUITEMID_CHILD)pidl, strHost))
		return ::AppendItemIDList(m_pidlMe, pidl);
	hr = m_pRoot->_BindToObject(hWndOwner, (PCUITEMID_CHILD)pidl, NULL, NULL, &pRoot);
	if (SUCCEEDED(hr))
	{
		if (pRoot->IsConnected() != S_OK)
		{
			pRoot->Release();
			pRoot = NULL;
		}
	}
	if (!pRoot)
	{
		hr = m_pRoot->OpenWithConnectDialog(hWndOwner, (PCUITEMID_CHILD)pidl, &pRoot);
		if (FAILED(hr))
			return hr == E_ABORT ? NULL : ::AppendItemIDList(m_pidlMe, pidl);
	}
	CSFTPHostItem UNALIGNED* pItem = (CSFTPHostItem UNALIGNED*) pidl;
	CHostFolderData* pData;
	LPCWSTR lpszPath;
	pData = theApp.FindHostFolderData(pItem->GetConnectionMode(), strHost, (int)pItem->nPort);
	if (!pData || !pData->pSettings)
		lpszPath = L".";
	else
	{
		if (pData->pSettings->strInitServerPath.IsEmpty())
			lpszPath = L".";
		else
			lpszPath = pData->pSettings->strInitServerPath;
		if (lplpszLocalDirectory && !pData->pSettings->strInitLocalPath.IsEmpty())
			*lplpszLocalDirectory = pData->pSettings->strInitLocalPath;
	}
	CFTPDirectoryBase* pDirectory;
	PIDLIST_ABSOLUTE pidlAbs;
	hr = m_pRoot->_RetrieveDirectory(pRoot, lpszPath, &pDirectory);
	if (SUCCEEDED(hr))
	{
		if (FAILED(pDirectory->GetCurFolder(&pidlAbs)))
			pidlAbs = NULL;
		pDirectory->Release();
	}
	else
	{
		if (FAILED(pRoot->GetCurFolder(&pidlAbs)))
			pidlAbs = NULL;
	}
	pRoot->Release();
	return pidlAbs ? pidlAbs : ::AppendItemIDList(m_pidlMe, pidl);
}
