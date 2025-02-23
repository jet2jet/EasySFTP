/*
 EasySFTP - Copyright (C) 2025 Kuri-Applications

 FoldRoot.cpp - implementation of CFTPDirectoryRootBase
 */

#include "stdafx.h"
#include "ShellDLL.h"
#include "FoldRoot.h"

#include "RFolder.h"

 ///////////////////////////////////////////////////////////////////////////////

CFTPDirectoryRootBase::CFTPDirectoryRootBase(CDelegateMallocData* pMallocData, CFTPDirectoryItem* pItemMe, CEasySFTPFolderRoot* pParent)
	: CFTPDirectoryT(pMallocData, pItemMe, theApp.GetTypeInfo(IID_IEasySFTPRootDirectory))
{
	m_bIsRoot = true;
	m_pParent = pParent;
	if (pParent)
		pParent->AddRef();
	m_strDirectory = L"/";
	m_bTextMode = TEXTMODE_NO_CONVERT;
	m_nServerCharset = scsUTF8;
	m_nTransferMode = TRANSFER_MODE_AUTO;
	m_arrTextFileType.CopyArray(theApp.m_arrDefTextFileType);
	m_bUseSystemTextFileType = true;
	m_pObjectOnClipboard = NULL;
	m_bUseThumbnailPreview = true;
	m_bIsTransferCanceled = false;
	m_pTransferDialog = NULL;
}

CFTPDirectoryRootBase::~CFTPDirectoryRootBase()
{
	::EnterCriticalSection(&theApp.m_csHosts);
	for (int i = 0, c = theApp.m_aHosts.GetCount(); i < c; ++i)
	{
		auto* pHostData = theApp.m_aHosts.GetItem(i);
		if (pHostData && pHostData->pDirItem && pHostData->pDirItem->pDirectory == this)
			pHostData->pDirItem->pDirectory = NULL;
	}
	::LeaveCriticalSection(&theApp.m_csHosts);

	if (m_pTransferDialog)
		delete m_pTransferDialog;
	if (m_pParent)
		m_pParent->Release();
}

ULONG CFTPDirectoryRootBase::DetachAndRelease()
{
	if (!CFTPDirectoryBase::DetachImpl())
		return 0;
	if (m_pParent)
	{
		auto c = m_uRef;
		for (ULONG u = 0; u < c; ++u)
			m_pParent->Release();
		m_pParent = NULL;
	}
	return Release();
}

STDMETHODIMP CFTPDirectoryRootBase::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	if (IsEqualIID(riid, IID_IExtractIconA) || IsEqualIID(riid, IID_IExtractIconW))
	{
		CEasySFTPRootIcon* pIcon = new CEasySFTPRootIcon(IML_INDEX_NETDRIVE);
		HRESULT hr = pIcon->QueryInterface(riid, ppv);
		pIcon->Release();
		return hr;
	}
	else if (IsEqualIID(riid, IID_IContextMenu))
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	else if (IsEqualIID(riid, IID_IEasySFTPRootDirectory) ||
		IsEqualIID(riid, IID_IEasySFTPDirectory) ||
		IsEqualIID(riid, IID_IDispatch) ||
		IsEqualIID(riid, IID_IUnknown))
	{
		*ppv = static_cast<IEasySFTPRootDirectory*>(this);
		AddRef();
		return S_OK;
	}
	return CFTPDirectoryBase::QueryInterface(riid, ppv);
}

STDMETHODIMP_(ULONG) CFTPDirectoryRootBase::AddRef()
{
	if (m_pParent)
		m_pParent->AddRef();
	auto u = CFTPDirectoryT::AddRef();
	return u;
}

STDMETHODIMP_(ULONG) CFTPDirectoryRootBase::Release()
{
	if (m_pParent)
		m_pParent->Release();
	return CFTPDirectoryT::Release();
}

STDMETHODIMP CFTPDirectoryRootBase::GetClassInfo(ITypeInfo** ppTI)
{
	return theApp.m_pTypeLib->GetTypeInfoOfGuid(CLSID_EasySFTPRootDirectory, ppTI);
}

STDMETHODIMP CFTPDirectoryRootBase::DoDeleteFTPItems(HWND hWndOwner, CFTPDirectoryBase* pDirectory, const CMyPtrArrayT<CFTPFileItem>& aItems)
{
	HRESULT hr = S_OK;
	CMyStringW strFile;
	CMyStringArrayW astrMsgs;
	CFTPFileItem* pItem;
	for (int i = 0; i < aItems.GetCount(); i++)
	{
		pItem = aItems.GetItem(i);

		strFile = pDirectory->m_strDirectory;
		if (((LPCWSTR)strFile)[strFile.GetLength() - 1] != L'/')
			strFile += L'/';
		strFile += pItem->strFileName;

		if (pItem->IsDirectory())
		{
			auto hr2 = DoDeleteDirectoryRecursive(hWndOwner, astrMsgs, pItem->strFileName, pDirectory);
			if (FAILED(hr2))
			{
				if (SUCCEEDED(hr))
				{
					hr = hr2;
				}
				continue;
			}
		}
		auto hr2 = DoDeleteFileOrDirectory(hWndOwner, astrMsgs, pItem->IsDirectory(), strFile, pDirectory);
		if (SUCCEEDED(hr))
		{
			hr = hr2;
		}
	}
	if (FAILED(hr))
		theApp.MultipleErrorMsgBox(hWndOwner, astrMsgs);
	return hr;
}

STDMETHODIMP CFTPDirectoryRootBase::MoveFTPItems(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszFromDir, LPCWSTR lpszFileNames)
{
	HRESULT hr = S_OK;
	CMyStringW strFile, strFileFrom, strFileTo;
	CMyStringArrayW astrMsgs;
	while (*lpszFileNames)
	{
		strFile = lpszFileNames;
		strFileFrom = lpszFromDir;
		if (((LPCWSTR)strFileFrom)[strFileFrom.GetLength() - 1] != L'/')
			strFileFrom += L'/';
		strFileFrom += lpszFileNames;

		strFileTo = pDirectory->m_strDirectory;
		if (((LPCWSTR)strFileTo)[strFileTo.GetLength() - 1] != L'/')
			strFileTo += L'/';
		strFileTo += lpszFileNames;

		while (*lpszFileNames++);

		CMyStringW strMsg;
		auto hr2 = RenameFTPItem(strFileFrom, strFileTo, &strMsg);
		if (FAILED(hr2))
			astrMsgs.Add(strMsg);
		if (SUCCEEDED(hr))
			hr = hr2;
	}
	if (FAILED(hr))
		theApp.MultipleErrorMsgBox(hWndOwner, astrMsgs);
	return hr;
}

STDMETHODIMP CFTPDirectoryRootBase::UpdateFTPItemAttributes(HWND hWndOwner, CFTPDirectoryBase* pDirectory,
	const CMyPtrArrayT<CServerFileAttrData>& aAttrs, bool* pabResults)
{
	CMyStringArrayW astrMsgs;
	HRESULT hr = S_OK;
	for (int i = 0; i < aAttrs.GetCount(); i++)
	{
		CServerFileAttrData* pAttr = aAttrs.GetItem(i);

		CMyStringW strMsg;
		auto hr2 = UpdateFTPItemAttribute(pDirectory, pAttr, &strMsg);
		pabResults[i] = SUCCEEDED(hr2);
		if (FAILED(hr2))
		{
			if (!strMsg.IsEmpty())
				astrMsgs.Add(strMsg);
			if (SUCCEEDED(hr))
				hr = hr2;
		}
	}
	if (FAILED(hr))
		theApp.MultipleErrorMsgBox(hWndOwner, astrMsgs);
	return hr;
}

STDMETHODIMP CFTPDirectoryRootBase::get_HostName(BSTR* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = MyStringToBSTR(m_strHostName);
	return *pRet ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CFTPDirectoryRootBase::get_Port(long* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = m_nPort;
	return S_OK;
}

STDMETHODIMP CFTPDirectoryRootBase::get_ConnectionMode(EasySFTPConnectionMode* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = GetProtocol();
	return S_OK;
}

STDMETHODIMP CFTPDirectoryRootBase::get_TextMode(EasySFTPTextMode* pnTextMode)
{
	if (!pnTextMode)
		return E_POINTER;
	*pnTextMode = static_cast<EasySFTPTextMode>(m_bTextMode);
	return S_OK;
}

STDMETHODIMP CFTPDirectoryRootBase::put_TextMode(EasySFTPTextMode nTextMode)
{
	auto n = static_cast<int>(nTextMode);
	if (n >= 0x100 || n < 0)
		return E_INVALIDARG;
	m_bTextMode = static_cast<BYTE>(n);
	return S_OK;
}

STDMETHODIMP CFTPDirectoryRootBase::get_TransferMode(EasySFTPTransferMode* pnTransferMode)
{
	if (!pnTransferMode)
		return E_POINTER;
	*pnTransferMode = static_cast<EasySFTPTransferMode>(m_nTransferMode);
	return S_OK;
}

STDMETHODIMP CFTPDirectoryRootBase::put_TransferMode(EasySFTPTransferMode nTransferMode)
{
	auto n = static_cast<int>(nTransferMode);
	if (n >= 0x80 || n < 0)
		return E_INVALIDARG;
	m_nTransferMode = static_cast<BYTE>(n);
	return S_OK;
}

STDMETHODIMP CFTPDirectoryRootBase::IsTextFile(BSTR lpszFileName, VARIANT_BOOL* pbRet)
{
	if (!pbRet)
		return E_POINTER;
	if (m_nTransferMode == TRANSFER_MODE_TEXT)
	{
		*pbRet = VARIANT_TRUE;
		return S_OK;
	}
	else if (m_nTransferMode == TRANSFER_MODE_BINARY)
	{
		*pbRet = VARIANT_FALSE;
		return S_OK;
	}
	if (!lpszFileName)
		return E_POINTER;

	CMyStringW strFile;
	MyBSTRToString(lpszFileName, strFile);
	LPCWSTR lp, lpFile;
	lpFile = strFile;
	lp = wcsrchr(lpFile, L'\\');
	if (lp)
		lpFile = lp + 1;
	lp = wcsrchr(lpFile, L'/');
	if (lp)
		lpFile = lp + 1;
	for (int i = 0; i < m_arrTextFileType.GetCount(); i++)
	{
		if (::MyMatchWildcardW(lpFile, m_arrTextFileType.GetItem(i)))
			return S_OK;
	}
	if (m_bUseSystemTextFileType)
	{
		lp = wcsrchr(lpFile, L'.');
		if (lp)
		{
			CMyStringW str(lp);
			HKEY hKey;
			DWORD dw;
			VARIANT_BOOL bRet = VARIANT_FALSE;
			if (::RegOpenKeyEx(HKEY_CLASSES_ROOT, str, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
			{
				dw = MAX_PATH;
				if (::RegQueryValueEx(hKey, _T("PerceivedType"), NULL, NULL,
#ifdef _UNICODE
				(LPBYTE) str.GetBuffer(MAX_PATH),
#else
					(LPBYTE)str.GetBufferA(MAX_PATH),
#endif
					& dw) == ERROR_SUCCESS)
				{
#ifdef _UNICODE
					str.ReleaseBuffer();
#else
					str.ReleaseBufferA();
#endif
					if (str.Compare(L"text", true) == 0)
						bRet = VARIANT_TRUE;
				}
				if (!bRet)
				{
					if (::RegQueryValueEx(hKey, _T("Content Type"), NULL, NULL,
#ifdef _UNICODE
					(LPBYTE) str.GetBuffer(MAX_PATH),
#else
						(LPBYTE)str.GetBufferA(MAX_PATH),
#endif
						& dw) == ERROR_SUCCESS)
					{
#ifdef _UNICODE
						str.ReleaseBuffer();
#else
						str.ReleaseBufferA();
#endif
						lpFile = str;
						lp = wcschr(lpFile, L'/');
						if (lp && _wcsnicmp(lpFile, L"text", ((size_t)((DWORD_PTR)lp - (DWORD_PTR)lpFile)) / sizeof(WCHAR)) == 0)
							bRet = VARIANT_TRUE;
					}
				}
				::RegCloseKey(hKey);
			}
			*pbRet = bRet;
			return S_OK;
		}
	}

	*pbRet = VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CFTPDirectoryRootBase::GetHostInfo(VARIANT_BOOL* pbIsSFTP, int* pnPort, BSTR* pbstrHostName)
{
	auto mode = GetProtocol();
	if (pbIsSFTP)
		*pbIsSFTP = (mode == EasySFTPConnectionMode::SFTP);
	if (pnPort)
		*pnPort = m_nPort;
	if (pbstrHostName)
	{
		*pbstrHostName = MyStringToBSTR(m_strHostName);
		if (!*pbstrHostName)
			return E_OUTOFMEMORY;
	}
	return S_OK;
}

//void CFTPDirectoryRootBase::BeforeClipboardOperation(IDataObject* pObjectNew)
//{
//}
//
//void CFTPDirectoryRootBase::AfterClipboardOperation(IDataObject* pObjectNew)
//{
//}

void CFTPDirectoryRootBase::ShowServerInfoDialog(HWND hWndOwner)
{
	CServerInfoDialog dlg;
	PreShowServerInfoDialog(&dlg);
	dlg.ModalDialogW(hWndOwner);
}

void CFTPDirectoryRootBase::OpenTransferDialogImpl(HWND hWndOwner)
{
	if (!m_pTransferDialog)
	{
		m_bIsTransferCanceled = false;
		m_pTransferDialog = new CTransferDialog(this);
		m_pTransferDialog->CreateW(hWndOwner);
	}
}

void CFTPDirectoryRootBase::CloseTransferDialogImpl()
{
	if (m_pTransferDialog)
	{
		m_bIsTransferCanceled = false;
		m_pTransferDialog->DestroyWindow();
		delete m_pTransferDialog;
		m_pTransferDialog = NULL;
	}
}

void CFTPDirectoryRootBase::TransferCanceled(void* pvTransfer)
{
	m_bIsTransferCanceled = true;
}

void CFTPDirectoryRootBase::TransferInProgress(void* pvObject, ULONGLONG uliPosition)
{
	if (m_pTransferDialog)
		m_pTransferDialog->UpdateTransferItem(static_cast<CTransferDialog::CTransferItem*>(pvObject), uliPosition);
}

bool CFTPDirectoryRootBase::TransferIsCanceled(void* pvObject)
{
	return m_bIsTransferCanceled;
}

void CFTPDirectoryRootBase::OnDisconnect()
{
	if (m_uRef > 0)
	{
		AddRef();
		// don't detach my parent (CFTPDirectoryRootBase::m_pParent)
		CFTPDirectoryBase::DetachAndRelease();
	}
}

HRESULT CFTPDirectoryRootBase::DoDeleteDirectoryRecursive(HWND hWndOwner, CMyStringArrayW& astrMsgs, LPCWSTR lpszName, CFTPDirectoryBase* pDirectory)
{
	CFTPDirectoryBase* pDir;
	auto hr = pDirectory->OpenNewDirectory(lpszName, &pDir);
	if (FAILED(hr))
	{
		return hr;
	}
	IEnumIDList* pIDList = NULL;
	hr = pDir->EnumObjects(hWndOwner, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN, &pIDList);
	if (FAILED(hr) || !pIDList)
	{
		pDir->Release();
		return hr;
	}
	while (true)
	{
		PITEMID_CHILD id;
		ULONG u = 0;
		hr = pIDList->Next(1, &id, &u);
		if (FAILED(hr))
		{
			break;
		}
		if (hr == S_FALSE || u == 0)
		{
			hr = S_OK;
			break;
		}
		SFGAOF attr = SFGAO_FOLDER;
		CMyStringW str;
		hr = pDir->GetAttributesOf(1, &id, &attr);
		if (SUCCEEDED(hr))
		{
			STRRET strret;
			strret.uType = STRRET_WSTR;
			hr = pDir->GetDisplayNameOf(id, SHGDN_FORPARSING | SHGDN_INFOLDER, &strret);
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
					str = (LPCSTR)(((LPCBYTE)id) + strret.uOffset);
					break;
				}
			}
		}
		::CoTaskMemFree(id);

		if (!str.IsEmpty())
		{
			str.InsertChar(L'/', 0);
			str.InsertString(lpszName, 0);
			if ((attr & SFGAO_FOLDER) != 0)
			{
				hr = DoDeleteDirectoryRecursive(hWndOwner, astrMsgs, str, pDirectory);
			}
			if (SUCCEEDED(hr))
			{
				str.InsertChar(L'/', 0);
				str.InsertString(pDirectory->m_strDirectory, 0);
				hr = DoDeleteFileOrDirectory(hWndOwner, astrMsgs, (attr & SFGAO_FOLDER) != 0, str, pDirectory);
			}
		}
		if (FAILED(hr))
		{
			break;
		}
	}
	pIDList->Release();
	pDir->Release();
	return hr;
}

HRESULT CFTPDirectoryRootBase::IsDirectoryExists(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszDirectory)
{
	CMyStringW strPartName;
	pDirectory->AddRef();
	while (true)
	{
		auto* p = wcschr(lpszDirectory, L'/');
		if (!p)
		{
			strPartName = lpszDirectory;
			lpszDirectory = NULL;
		}
		else
		{
			strPartName.SetString(lpszDirectory, p - lpszDirectory);
			lpszDirectory = p + 1;
		}

		if (!pDirectory->m_bDirReceived)
		{
			if (!ReceiveDirectory(hWndOwner, pDirectory, pDirectory->m_strDirectory, &pDirectory->m_bDirReceived))
			{
				pDirectory->Release();
				return E_FAIL;
			}
		}

		auto found = false;
		auto isDir = false;
		::EnterCriticalSection(&pDirectory->m_csFiles);
		for (int i = 0; i < pDirectory->m_aFiles.GetCount(); ++i)
		{
			auto* p = pDirectory->m_aFiles.GetItem(i);
			if (p->strFileName.Compare(strPartName) == 0)
			{
				found = true;
				isDir = p->IsDirectory();
				break;
			}
		}
		::LeaveCriticalSection(&pDirectory->m_csFiles);

		if (!found || !isDir)
		{
			pDirectory->Release();
			return S_FALSE;
		}

		if (!lpszDirectory || !*lpszDirectory)
			break;

		{
			CFTPDirectoryBase* pChildDir;
			auto hr = pDirectory->OpenNewDirectory(strPartName, &pChildDir);
			pDirectory->Release();
			if (FAILED(hr))
			{
				return hr;
			}
			pDirectory = pChildDir;
		}
	}
	pDirectory->Release();
	return S_OK;
}
