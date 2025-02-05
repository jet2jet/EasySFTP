/*
 EasySFTP - Copyright (C) 2025 Kuri-Applications

 FoldRoot.cpp - implementation of CFTPFileStream
 */

#include "stdafx.h"
#include "ShellDLL.h"
#include "FoldRoot.h"

#include "RFolder.h"

 ///////////////////////////////////////////////////////////////////////////////

CFTPDirectoryRootBase::CFTPDirectoryRootBase(CDelegateMallocData* pMallocData, CFTPDirectoryItem* pItemMe)
	: CFTPDirectoryBase(pMallocData, pItemMe)
{
	m_bIsRoot = true;
	m_pRoot = this;
	m_strDirectory = L"/";
	m_bTextMode = TEXTMODE_NO_CONVERT;
	m_nServerCharset = scsUTF8;
	m_nTransferMode = TRANSFER_MODE_AUTO;
	m_arrTextFileType.CopyArray(theApp.m_arrDefTextFileType);
	m_bUseSystemTextFileType = true;
	m_pObjectOnClipboard = NULL;
	m_bUseThumbnailPreview = true;
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
	return CFTPDirectoryBase::QueryInterface(riid, ppv);
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

STDMETHODIMP CFTPDirectoryRootBase::GetHostInfo(VARIANT_BOOL* pbIsSFTP, int* pnPort, BSTR* pbstrHostName)
{
	int nPort;
	LPCWSTR lpw;
	lpw = GetProtocolName(nPort);
	if (pbIsSFTP)
		*pbIsSFTP = (_wcsicmp(lpw, L"sftp") == 0);
	if (pnPort)
		*pnPort = nPort;
	if (pbstrHostName)
	{
		*pbstrHostName = ::SysAllocStringLen(m_strHostName, (UINT)m_strHostName.GetLength());
		if (!*pbstrHostName)
			return E_OUTOFMEMORY;
	}
	return S_OK;
}

STDMETHODIMP CFTPDirectoryRootBase::GetTextMode(LONG* pnTextMode)
{
	if (!pnTextMode)
		return E_POINTER;
	*pnTextMode = (LONG)m_bTextMode;
	return S_OK;
}

STDMETHODIMP CFTPDirectoryRootBase::SetTextMode(LONG nTextMode)
{
	if (nTextMode >= 0x100 || nTextMode < 0)
		return E_INVALIDARG;
	m_bTextMode = (BYTE)nTextMode;
	return S_OK;
}

STDMETHODIMP CFTPDirectoryRootBase::GetTransferMode(LONG* pnTransferMode)
{
	if (!pnTransferMode)
		return E_POINTER;
	*pnTransferMode = (LONG)m_nTransferMode;
	return S_OK;
}

STDMETHODIMP CFTPDirectoryRootBase::SetTransferMode(LONG nTransferMode)
{
	if (nTransferMode >= 0x80 || nTransferMode < 0)
		return E_INVALIDARG;
	m_nTransferMode = (BYTE)nTransferMode;
	return S_OK;
}

STDMETHODIMP CFTPDirectoryRootBase::IsTextFile(LPCWSTR lpszFileName)
{
	if (m_nTransferMode == TRANSFER_MODE_TEXT)
		return S_OK;
	else if (m_nTransferMode == TRANSFER_MODE_BINARY)
		return S_FALSE;
	if (!lpszFileName)
		return E_POINTER;

	LPCWSTR lp;
	lp = wcsrchr(lpszFileName, L'\\');
	if (lp)
		lpszFileName = lp + 1;
	lp = wcsrchr(lpszFileName, L'/');
	if (lp)
		lpszFileName = lp + 1;
	for (int i = 0; i < m_arrTextFileType.GetCount(); i++)
	{
		if (::MyMatchWildcardW(lpszFileName, m_arrTextFileType.GetItem(i)))
			return S_OK;
	}
	if (m_bUseSystemTextFileType)
	{
		lp = wcsrchr(lpszFileName, L'.');
		if (lp)
		{
			CMyStringW str(lp);
			HKEY hKey;
			DWORD dw;
			bool bRet = false;
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
						bRet = true;
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
						lpszFileName = str;
						lp = wcschr(lpszFileName, L'/');
						if (lp && _wcsnicmp(lpszFileName, L"text", ((size_t)((DWORD_PTR)lp - (DWORD_PTR)lpszFileName)) / sizeof(WCHAR)) == 0)
							bRet = true;
					}
				}
				::RegCloseKey(hKey);
			}
			return bRet ? S_OK : S_FALSE;
		}
	}

	return S_FALSE;
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

HRESULT CFTPDirectoryRootBase::DoDeleteDirectoryRecursive(HWND hWndOwner, CMyStringArrayW& astrMsgs, LPCWSTR lpszName, CFTPDirectoryBase* pDirectory)
{
	CFTPDirectoryBase* pDir;
	auto hr = pDirectory->OpenNewDirectory(lpszName, &pDir);
	if (FAILED(hr))
	{
		return hr;
	}
	IEnumIDList* pIDList = NULL;
	hr = pDir->EnumObjects(hWndOwner, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FASTITEMS, &pIDList);
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
				hr = DoDeleteFileOrDirectory(hWndOwner, astrMsgs, (attr & SFGAO_FOLDER) != 0, str, NULL);
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
