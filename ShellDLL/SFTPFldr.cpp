
#include "StdAfx.h"
#include "ShellDLL.h"
#include "SFTPFldr.h"

#include "DragData.h"
#include "SFTPStrm.h"

static void __stdcall GetSFTPStatusMessage(int nStatus, LPCWSTR lpszMessage, CMyStringW& rstrMessage)
{
	CMyStringW strMsg;
	rstrMessage.Format(IDS_COMMAND_FAILED, nStatus);
	if (lpszMessage && *lpszMessage)
		strMsg = lpszMessage;
	else if (!strMsg.LoadString(((UINT) nStatus - SSH_FX_EOF) + IDS_SFTP_EOF))
		strMsg.LoadString(IDS_SFTP_UNKNOWN);

	if (!strMsg.IsEmpty())
	{
		rstrMessage += L" (";
		rstrMessage += strMsg;
		rstrMessage += L')';
	}
}

////////////////////////////////////////////////////////////////////////////////

CSFTPFolderSFTPDirectory::CSFTPFolderSFTPDirectory(
	CDelegateMallocData* pMallocData,
	CFTPDirectoryItem* pItemMe,
	CFTPDirectoryBase* pParent,
	CFTPDirectoryRootBase* pRoot,
	LPCWSTR lpszDirectory)
	: CFTPDirectoryBase(pMallocData, pItemMe, pParent, pRoot, lpszDirectory)
{
}

CSFTPFolderSFTPDirectory::CSFTPFolderSFTPDirectory(CDelegateMallocData* pMallocData, CFTPDirectoryItem* pItemMe)
	: CFTPDirectoryBase(pMallocData, pItemMe)
{
}

CSFTPFolderSFTPDirectory::~CSFTPFolderSFTPDirectory()
{
}

STDMETHODIMP CSFTPFolderSFTPDirectory::CreateInstance(CFTPDirectoryItem* pItemMe, CFTPDirectoryBase* pParent, CFTPDirectoryRootBase* pRoot, LPCWSTR lpszDirectory, CFTPDirectoryBase** ppResult)
{
	CSFTPFolderSFTPDirectory* p = new CSFTPFolderSFTPDirectory(m_pMallocData, pItemMe, pParent, pRoot, lpszDirectory);
	if (!p)
		return E_OUTOFMEMORY;
	*ppResult = p;
	return S_OK;
}

STDMETHODIMP_(void) CSFTPFolderSFTPDirectory::UpdateItem(CFTPFileItem* pOldItem, LPCWSTR lpszNewItem, LONG lEvent)
{
	CFTPDirectoryBase::UpdateItem(pOldItem, lpszNewItem, lEvent);

	switch (lEvent)
	{
		case SHCNE_CREATE:
		case SHCNE_MKDIR:
		{
			register CSFTPFolderSFTP* pRoot = (CSFTPFolderSFTP*) m_pRoot;
			CMyStringW strFile(m_strDirectory);
			if (((LPCWSTR) strFile)[strFile.GetLength() - 1] != L'/')
				strFile += L'/';
			strFile += lpszNewItem;
			ULONG uMsgID = pRoot->m_pChannel->LStat(strFile);
			if (!uMsgID)
			{
				break;
			}
			CSFTPWaitAttrData* pAttr = new CSFTPWaitAttrData();
			if (!pAttr)
			{
				break;
			}
			pAttr->uMsgID = uMsgID;
			pAttr->nType = CSFTPWaitAttrData::typeNormal;
			pRoot->m_listWaitResponse.Add(pAttr, uMsgID);

			while (pAttr->uMsgID)
			{
				if (pRoot->m_pClient->m_socket.CanReceive(WAIT_RECEIVE_TIME))
				{
					pRoot->DoReceiveSocket();
					if (!pRoot->m_pClient || !theApp.MyPumpMessage())
					{
						pAttr->nResult = SSH_FX_NO_CONNECTION;
						break;
					}
				}
				else
				{
					pAttr->nResult = SSH_FX_NO_CONNECTION;
					break;
				}
			}

			if (pAttr->nResult != SSH_FX_OK)
			{
				delete pAttr;
				break;
			}

			CSFTPFileData fileData;
			memcpy(&fileData.attr, &pAttr->fileData, sizeof(fileData.attr));
			fileData.strFileName = lpszNewItem;
			CFTPFileItem* pItem = ParseSFTPData(((CSFTPFolderSFTP*) m_pRoot)->m_pChannel->GetServerVersion(), &fileData);
			delete pAttr;

			::EnterCriticalSection(&m_csFiles);
			m_aFiles.Add(pItem);
			::LeaveCriticalSection(&m_csFiles);
		}
		break;
		case SHCNE_UPDATEITEM:
		case SHCNE_UPDATEDIR:
		{
			register CSFTPFolderSFTP* pRoot = (CSFTPFolderSFTP*) m_pRoot;
			CMyStringW strFile;
			if (((LPCWSTR) strFile)[strFile.GetLength() - 1] != L'/')
				strFile += L'/';
			strFile += pOldItem->strFileName;
			ULONG uMsgID = ((CSFTPFolderSFTP*) m_pRoot)->m_pChannel->LStat(strFile);
			if (!uMsgID)
			{
				break;
			}
			CSFTPWaitAttrData* pAttr = new CSFTPWaitAttrData();
			if (!pAttr)
			{
				break;
			}
			pAttr->uMsgID = uMsgID;
			pAttr->nType = CSFTPWaitAttrData::typeNormal;
			pRoot->m_listWaitResponse.Add(pAttr, uMsgID);

			while (pAttr->uMsgID)
			{
				if (pRoot->m_pClient->m_socket.CanReceive(WAIT_RECEIVE_TIME))
				{
					pRoot->DoReceiveSocket();
					if (!pRoot->m_pClient || !theApp.MyPumpMessage())
					{
						pAttr->nResult = SSH_FX_NO_CONNECTION;
						break;
					}
				}
				else
				{
					pAttr->nResult = SSH_FX_NO_CONNECTION;
					break;
				}
			}

			if (pAttr->nResult != SSH_FX_OK)
			{
				delete pAttr;
				break;
			}

			ParseSFTPAttributes(((CSFTPFolderSFTP*) m_pRoot)->m_pChannel->GetServerVersion(), pOldItem, &pAttr->fileData);
			delete pAttr;
		}
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////

CSFTPFolderSFTP::CSFTPFolderSFTP(CDelegateMallocData* pMallocData, CFTPDirectoryItem* pItemMe, CEasySFTPFolderRoot* pFolderRoot)
	: CFTPDirectoryRootBase(pMallocData, pItemMe)
	, m_pFolderRoot(pFolderRoot)
{
	m_pUser = NULL;
	m_pClient = NULL;
	m_pChannel = NULL;
	m_idTimer = 0;
	m_nPort = 22;
	m_nServerCharset = scsUTF8;
	::InitializeCriticalSection(&m_csSocket);
	::InitializeCriticalSection(&m_csReceive);
	m_pFolderRoot->AddRef();
	m_hWndOwnerCache = pFolderRoot->GetHwndOwnerCache();
	m_dwTransferringCount = 0;
	//::InitializeCriticalSection(&m_csTransferringCount);
}

CSFTPFolderSFTP::~CSFTPFolderSFTP()
{
	Disconnect();
	m_pFolderRoot->Release();
#ifdef _DEBUG
	m_pFolderRoot = NULL;
#endif
	//::DeleteCriticalSection(&m_csTransferringCount);
	::DeleteCriticalSection(&m_csReceive);
	::DeleteCriticalSection(&m_csSocket);
}

//STDMETHODIMP CSFTPFolderSFTP::GetUIObjectOf(HWND hWndOwner, UINT cidl, PCUITEMID_CHILD_ARRAY apidl,
//	REFIID riid, UINT* rgfReserved, void** ppv)
//{
//	if (!ppv)
//		return E_POINTER;
//	if (!cidl)
//	{
//		*ppv = NULL;
//		return E_INVALIDARG;
//	}
//	if (!apidl)
//	{
//		*ppv = NULL;
//		return E_POINTER;
//	}
//
//	return CFTPDirectoryBase::GetUIObjectOf(hWndOwner, cidl, apidl, riid, rgfReserved, ppv);
//}

STDMETHODIMP CSFTPFolderSFTP::GetFTPItemUIObjectOf(HWND hWndOwner, CFTPDirectoryBase* pDirectory,
	const CMyPtrArrayT<CFTPFileItem>& aItems, CFTPDataObject** ppObject)
{
	if (!m_pChannel)
		return OLE_E_NOTRUNNING;
	CFTPDataObject* pObject = new CFTPDataObject(//(CSFTPFolderSFTPDirectory*) pDirectory,
		m_pMallocData->pMalloc,
		pDirectory->m_pidlMe,
		m_strHostName,
		m_pClient, m_pChannel,
		pDirectory, aItems);
	if (!pObject)
		return E_OUTOFMEMORY;
	pObject->SetTextMode(m_bTextMode);
	*ppObject = pObject;
	return S_OK;
}

STDMETHODIMP CSFTPFolderSFTP::SetFTPItemNameOf(HWND hWnd, CFTPDirectoryBase* pDirectory, CFTPFileItem* pItem, LPCWSTR pszName, SHGDNF uFlags)
{
	CMyStringW strFrom(pDirectory->m_strDirectory), strTo(pDirectory->m_strDirectory);
	size_t dw = strFrom.GetLength() - 1;
	if (((LPCWSTR) strFrom)[dw] != L'/')
		strFrom += L'/';
	if (((LPCWSTR) strTo)[dw] != L'/')
		strTo += L'/';
	strFrom += pItem->strFileName;
	strTo += pszName;

	ULONG uMsgID = m_pChannel->Rename(strFrom, strTo);
	if (!uMsgID)
	{
		::MyMessageBoxW(hWnd, MAKEINTRESOURCEW(IDS_COMMAND_OUTOFMEMORY), NULL, MB_ICONHAND);
		return E_OUTOFMEMORY;
	}

	CSFTPWaitConfirm* pData = new CSFTPWaitConfirm();
	HRESULT hr;
	if (!pData)
	{
		hr = E_OUTOFMEMORY;
		::MyMessageBoxW(hWnd, MAKEINTRESOURCEW(IDS_COMMAND_OUTOFMEMORY), NULL, MB_ICONHAND);
	}
	else
	{
		pData->uMsgID = uMsgID;
		m_listWaitResponse.Add(pData, uMsgID);

		while (pData->uMsgID)
		{
			if (m_pClient->m_socket.CanReceive(WAIT_RECEIVE_TIME))
			{
				DoReceiveSocket();
				if (!m_pClient || !theApp.MyPumpMessage())
				{
					delete pData;
					//pDirectory->Release();
					return E_FAIL;
				}
			}
			else
			{
				pData->nResult = SSH_FX_NO_CONNECTION;
				break;
			}
		}

		switch (pData->nResult)
		{
			case SSH_FX_OK:
				hr = S_OK;
				break;
			//case SSH_FX_FILE_ALREADY_EXISTS:
			//	hr = E_FAIL;
			//	break;
			case SSH_FX_PERMISSION_DENIED:
				hr = E_ACCESSDENIED;
				break;
			default:
				hr = E_FAIL;
				break;
		}
		if (FAILED(hr))
		{
			CMyStringW str;
			GetSFTPStatusMessage(pData->nResult, pData->strMessage, str);
			::MyMessageBoxW(hWnd, str, NULL, MB_ICONHAND);
		}
		delete pData;
	}
	return hr;
}

STDMETHODIMP CSFTPFolderSFTP::DoDeleteFTPItems(HWND hWndOwner, CFTPDirectoryBase* pDirectory, const CMyPtrArrayT<CFTPFileItem>& aItems)
{
	HRESULT hr = S_OK;
	CMyStringW strFile;
	CMyStringArrayW astrMsgs;
	CFTPFileItem* pItem;
	for (int i = 0; i < aItems.GetCount(); i++)
	{
		pItem = aItems.GetItem(i);

		strFile = pDirectory->m_strDirectory;
		if (((LPCWSTR) strFile)[strFile.GetLength() - 1] != L'/')
			strFile += L'/';
		strFile += pItem->strFileName;

		ULONG uMsgID = pItem->IsDirectory() ? m_pChannel->RemoveRemoteDirectory(strFile) : m_pChannel->Remove(strFile);
		if (!uMsgID)
		{
			CMyStringW str, str2;
			str2.LoadString(IDS_COMMAND_UNKNOWN_ERROR);
			str = strFile;
			str += L": ";
			str += str2;
			astrMsgs.Add(str);
			if (SUCCEEDED(hr))
				hr = E_OUTOFMEMORY;
		}
		else
		{
			CSFTPWaitConfirm* pData = new CSFTPWaitConfirm();
			if (!pData)
				hr = E_OUTOFMEMORY;
			else
			{
				pData->uMsgID = uMsgID;
				m_listWaitResponse.Add(pData, uMsgID);

				while (pData->uMsgID)
				{
					if (m_pClient->m_socket.CanReceive(WAIT_RECEIVE_TIME))
					{
						DoReceiveSocket();
						if (!m_pClient || !theApp.MyPumpMessage())
						{
							delete pData;
							//pDirectory->Release();
							return E_FAIL;
						}
					}
					else
					{
						pData->nResult = SSH_FX_NO_CONNECTION;
						break;
					}
				}

				HRESULT hr2;
				switch (pData->nResult)
				{
					case SSH_FX_OK:
						hr2 = S_OK;
						break;
					//case SSH_FX_FILE_ALREADY_EXISTS:
					//	hr = E_FAIL;
					//	break;
					case SSH_FX_PERMISSION_DENIED:
						hr2 = E_ACCESSDENIED;
						break;
					default:
						hr2 = E_FAIL;
						break;
				}
				if (FAILED(hr2))
				{
					CMyStringW str, str2;
					GetSFTPStatusMessage(pData->nResult, pData->strMessage, str2);
					str = strFile;
					str += L": ";
					str += str2;
					astrMsgs.Add(str);
					if (SUCCEEDED(hr))
						hr = hr2;
				}
				else
				{
					pDirectory->UpdateRemoveFile(pItem->strFileName, pItem->IsDirectory());
				}
				delete pData;
			}
		}
	}
	if (FAILED(hr))
		theApp.MultipleErrorMsgBox(hWndOwner, astrMsgs);
	return hr;
}

STDMETHODIMP CSFTPFolderSFTP::MoveFTPItems(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszFromDir, LPCWSTR lpszFileNames)
{
	HRESULT hr = S_OK;
	CMyStringW strFile, strFileFrom, strFileTo;
	CMyStringArrayW astrMsgs;
	while (*lpszFileNames)
	{
		strFile = lpszFileNames;
		strFileFrom = lpszFromDir;
		if (((LPCWSTR) strFileFrom)[strFileFrom.GetLength() - 1] != L'/')
			strFileFrom += L'/';
		strFileFrom += lpszFileNames;

		strFileTo = pDirectory->m_strDirectory;
		if (((LPCWSTR) strFileTo)[strFileTo.GetLength() - 1] != L'/')
			strFileTo += L'/';
		strFileTo += lpszFileNames;

		while (*lpszFileNames++);

		ULONG uMsgID;
		uMsgID = m_pChannel->Stat(strFileFrom);
		if (!uMsgID)
		{
			CMyStringW str, str2;
			str2.LoadString(IDS_COMMAND_UNKNOWN_ERROR);
			str = strFileFrom;
			str += L": ";
			str += str2;
			astrMsgs.Add(str);
			if (SUCCEEDED(hr))
				hr = E_OUTOFMEMORY;
			continue;
		}

		CSFTPWaitAttrData* pAttr = new CSFTPWaitAttrData();
		if (!pAttr)
		{
			CMyStringW str, str2;
			str2.LoadString(IDS_COMMAND_OUTOFMEMORY);
			str = strFileFrom;
			str += L": ";
			str += str2;
			astrMsgs.Add(str);
			hr = E_OUTOFMEMORY;
			continue;
		}
		pAttr->uMsgID = uMsgID;
		pAttr->nType = CSFTPWaitAttrData::typeNormal;
		m_listWaitResponse.Add(pAttr, uMsgID);

		while (pAttr->uMsgID)
		{
			if (m_pClient->m_socket.CanReceive(WAIT_RECEIVE_TIME))
			{
				DoReceiveSocket();
				if (!m_pClient || !theApp.MyPumpMessage())
				{
					delete pAttr;
					//pDirectory->Release();
					return E_FAIL;
				}
			}
			else
			{
				pAttr->nResult = SSH_FX_NO_CONNECTION;
				break;
			}
		}

		HRESULT hr2;
		switch (pAttr->nResult)
		{
			case SSH_FX_OK:
				hr2 = S_OK;
				break;
			//case SSH_FX_FILE_ALREADY_EXISTS:
			//	hr = E_FAIL;
			//	break;
			case SSH_FX_PERMISSION_DENIED:
				hr2 = E_ACCESSDENIED;
				break;
			default:
				hr2 = E_FAIL;
				break;
		}
		if (FAILED(hr2))
		{
			CMyStringW str, str2;
			GetSFTPStatusMessage(pAttr->nResult, pAttr->strMessage, str2);
			str = strFileFrom;
			str += L": ";
			str += str2;
			astrMsgs.Add(str);
			delete pAttr;
			if (SUCCEEDED(hr))
				hr = hr2;
			continue;
		}
		bool bIsDir = (pAttr->fileData.bFileType == SSH_FILEXFER_TYPE_DIRECTORY);
		delete pAttr;

		uMsgID = m_pChannel->Rename(strFileFrom, strFileTo, SSH_FXF_RENAME_OVERWRITE);
		if (!uMsgID)
		{
			CMyStringW str, str2;
			str2.LoadString(IDS_COMMAND_UNKNOWN_ERROR);
			str = strFileFrom;
			str += L": ";
			str += str2;
			astrMsgs.Add(str);
			if (SUCCEEDED(hr))
				hr = E_OUTOFMEMORY;
			continue;
		}

		CSFTPWaitConfirm* pData = new CSFTPWaitConfirm();
		if (!pData)
		{
			hr = E_OUTOFMEMORY;
			continue;
		}
		pData->uMsgID = uMsgID;
		m_listWaitResponse.Add(pData, uMsgID);

		while (pData->uMsgID)
		{
			if (m_pClient->m_socket.CanReceive(WAIT_RECEIVE_TIME))
			{
				DoReceiveSocket();
				if (!m_pClient || !theApp.MyPumpMessage())
				{
					delete pData;
					//pDirectory->Release();
					::MyMessageBoxW(hWndOwner, MAKEINTRESOURCEW(IDS_SFTP_CONNECTION_LOST), NULL, MB_ICONEXCLAMATION);
					return E_FAIL;
				}
			}
			else
			{
				pData->nResult = SSH_FX_NO_CONNECTION;
				break;
			}
		}

		switch (pData->nResult)
		{
			case SSH_FX_OK:
				hr2 = S_OK;
				break;
			//case SSH_FX_FILE_ALREADY_EXISTS:
			//	hr = E_FAIL;
			//	break;
			case SSH_FX_PERMISSION_DENIED:
				hr2 = E_ACCESSDENIED;
				break;
			default:
				hr2 = E_FAIL;
				break;
		}
		if (FAILED(hr2))
		{
			CMyStringW str, str2;
			GetSFTPStatusMessage(pData->nResult, pData->strMessage, str2);
			str = strFileFrom;
			str += L": ";
			str += str2;
			astrMsgs.Add(str);
			delete pData;
			if (SUCCEEDED(hr))
				hr = hr2;
		}
		else
		{
			delete pData;
			pDirectory->UpdateMoveFile(lpszFromDir, strFile, bIsDir);
			//NotifyUpdate(bIsDir ? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM, strFileFrom, strFileTo);
		}
	}
	if (FAILED(hr))
		theApp.MultipleErrorMsgBox(hWndOwner, astrMsgs);
	return hr;
}

STDMETHODIMP CSFTPFolderSFTP::UpdateFTPItemAttributes(HWND hWndOwner, CFTPDirectoryBase* pDirectory,
	CServerFilePropertyDialog* pDialog, const CMyPtrArrayT<CServerFileAttrData>& aAttrs, bool* pabResults)
{
	CMyStringArrayW astrMsgs;
	HRESULT hr = S_OK;
	CMyStringW strFile;
	for (int i = 0; i < aAttrs.GetCount(); i++)
	{
		CServerFileAttrData* pAttr = aAttrs.GetItem(i);

		CSFTPFileAttribute attr;
		strFile = pDirectory->m_strDirectory;
		if (((LPCWSTR) strFile)[strFile.GetLength() - 1] != L'/')
			strFile += L'/';
		strFile += pAttr->pItem->strFileName;

		attr.dwMask = 0;
		if (pDialog->m_bChangeOwner)
		{
			if (pDialog->m_bSupportedName)
			{
				attr.dwMask = SSH_FILEXFER_ATTR_OWNERGROUP;
				attr.strOwner = pAttr->strOwner;
				attr.strGroup = pAttr->strGroup;
			}
			else
			{
				attr.dwMask = SSH_FILEXFER_ATTR_UIDGID;
				attr.uUserID = pAttr->uUID;
				attr.uGroupID = pAttr->uGID;
			}
		}
		if (pDialog->m_bChangeAttr)
		{
			attr.dwMask |= SSH_FILEXFER_ATTR_PERMISSIONS;
			attr.dwPermissions = (DWORD) pAttr->nUnixMode;
		}

		ULONG uMsgID = m_pChannel->SetStat(strFile, attr);

		if (!uMsgID)
		{
			CMyStringW str, str2;
			str2.LoadString(IDS_COMMAND_UNKNOWN_ERROR);
			str = strFile;
			str += L": ";
			str += str2;
			astrMsgs.Add(str);
			if (SUCCEEDED(hr))
				hr = E_OUTOFMEMORY;
			continue;
		}

		CSFTPWaitSetStat* pWait = new CSFTPWaitSetStat();
		if (!pWait)
		{
			CMyStringW str, str2;
			str2.LoadString(IDS_COMMAND_OUTOFMEMORY);
			str = strFile;
			str += L": ";
			str += str2;
			astrMsgs.Add(str);
			hr = E_OUTOFMEMORY;
			continue;
		}
		pWait->uMsgID = uMsgID;
		//pWait->pbResult = &pabResults[i];
		m_listWaitResponse.Add(pWait, uMsgID);

		while (pWait->uMsgID)
		{
			if (m_pClient->m_socket.CanReceive(WAIT_RECEIVE_TIME))
			{
				DoReceiveSocket();
				if (!m_pClient || !theApp.MyPumpMessage())
				{
					delete pWait;
					//pDirectory->Release();
					::MyMessageBoxW(hWndOwner, MAKEINTRESOURCEW(IDS_SFTP_CONNECTION_LOST), NULL, MB_ICONEXCLAMATION);
					return E_ABORT;
				}
			}
			else
			{
				pWait->nResult = SSH_FX_NO_CONNECTION;
				break;
			}
		}

		HRESULT hr2;
		switch (pWait->nResult)
		{
			case SSH_FX_OK:
				hr2 = S_OK;
				break;
			//case SSH_FX_FILE_ALREADY_EXISTS:
			//	hr = E_FAIL;
			//	break;
			case SSH_FX_PERMISSION_DENIED:
				hr2 = E_ACCESSDENIED;
				break;
			default:
				hr2 = E_FAIL;
				break;
		}
		if (FAILED(hr2))
		{
			CMyStringW str, str2;
			GetSFTPStatusMessage(pWait->nResult, pWait->strMessage, str2);
			str = strFile;
			str += L": ";
			str += str2;
			astrMsgs.Add(str);
			delete pWait;
			if (SUCCEEDED(hr))
				hr = hr2;
			pabResults[i] = false;
		}
		else
		{
			delete pWait;
			pabResults[i] = true;
			//NotifyUpdate(pAttr->pItem->IsDirectory() ? SHCNE_UPDATEDIR : SHCNE_UPDATEITEM, strFile, NULL);
			pDirectory->UpdateFileAttrs(pAttr->pItem->strFileName, pAttr->pItem->IsDirectory());
		}
	}
	if (FAILED(hr))
		theApp.MultipleErrorMsgBox(hWndOwner, astrMsgs);
	return hr;
}

STDMETHODIMP CSFTPFolderSFTP::CreateFTPDirectory(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszName)
{
	HRESULT hr = S_OK;
	CMyStringW strFile, strMsg;

	strFile = pDirectory->m_strDirectory;
	if (((LPCWSTR) strFile)[strFile.GetLength() - 1] != L'/')
		strFile += L'/';
	strFile += lpszName;

	ULONG uMsgID = m_pChannel->CreateRemoteDirectory(strFile);
	if (!uMsgID)
	{
		CMyStringW str2;
		str2.LoadString(IDS_COMMAND_UNKNOWN_ERROR);
		strMsg = strFile;
		strMsg += L": ";
		strMsg += str2;
		if (SUCCEEDED(hr))
			hr = E_OUTOFMEMORY;
	}
	else
	{
		CSFTPWaitConfirm* pData = new CSFTPWaitConfirm();
		if (!pData)
			hr = E_OUTOFMEMORY;
		else
		{
			pData->uMsgID = uMsgID;
			m_listWaitResponse.Add(pData, uMsgID);

			while (pData->uMsgID)
			{
				if (m_pClient->m_socket.CanReceive(WAIT_RECEIVE_TIME))
				{
					DoReceiveSocket();
					if (!m_pClient || !theApp.MyPumpMessage())
					{
						delete pData;
						//pDirectory->Release();
						return E_FAIL;
					}
				}
				else
				{
					pData->nResult = SSH_FX_NO_CONNECTION;
					break;
				}
			}

			HRESULT hr2;
			switch (pData->nResult)
			{
				case SSH_FX_OK:
					hr2 = S_OK;
					break;
				//case SSH_FX_FILE_ALREADY_EXISTS:
				//	hr = E_FAIL;
				//	break;
				case SSH_FX_PERMISSION_DENIED:
					hr2 = E_ACCESSDENIED;
					break;
				default:
					hr2 = E_FAIL;
					break;
			}
			if (FAILED(hr2))
			{
				CMyStringW str2;
				GetSFTPStatusMessage(pData->nResult, pData->strMessage, str2);
				strMsg = strFile;
				strMsg += L": ";
				strMsg += str2;
				if (SUCCEEDED(hr))
					hr = hr2;
			}
			else
				pDirectory->UpdateNewFile(lpszName, true);
				//pDirectory->NotifyUpdate(SHCNE_MKDIR, lpszName, NULL);
			delete pData;
		}
	}
	if (FAILED(hr) && !strMsg.IsEmpty())
		::MyMessageBoxW(hWndOwner, strMsg, NULL, MB_ICONHAND);
	return hr;
}

STDMETHODIMP CSFTPFolderSFTP::CreateShortcut(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszName, LPCWSTR lpszLinkTo, bool bHardLink)
{
	HRESULT hr = S_OK;
	CMyStringW strFile, strMsg;

	strFile = pDirectory->m_strDirectory;
	if (((LPCWSTR) strFile)[strFile.GetLength() - 1] != L'/')
		strFile += L'/';
	strFile += lpszName;

	ULONG uMsgID;
	if (!bHardLink)
		uMsgID = m_pChannel->SymLink(strFile, lpszLinkTo);
	else
		uMsgID = m_pChannel->Link(strFile, lpszLinkTo, false);
	if (!uMsgID)
	{
		CMyStringW str2;
		str2.LoadString(IDS_COMMAND_UNKNOWN_ERROR);
		strMsg = strFile;
		strMsg += L": ";
		strMsg += str2;
		if (SUCCEEDED(hr))
			hr = E_OUTOFMEMORY;
	}
	else
	{
		CSFTPWaitConfirm* pData = new CSFTPWaitConfirm();
		if (!pData)
			hr = E_OUTOFMEMORY;
		else
		{
			pData->uMsgID = uMsgID;
			m_listWaitResponse.Add(pData, uMsgID);

			while (pData->uMsgID)
			{
				if (m_pClient->m_socket.CanReceive(WAIT_RECEIVE_TIME))
				{
					DoReceiveSocket();
					if (!m_pClient || !theApp.MyPumpMessage())
					{
						delete pData;
						//pDirectory->Release();
						return E_FAIL;
					}
				}
				else
				{
					pData->nResult = SSH_FX_NO_CONNECTION;
					break;
				}
			}

			HRESULT hr2;
			switch (pData->nResult)
			{
				case SSH_FX_OK:
					hr2 = S_OK;
					break;
				//case SSH_FX_FILE_ALREADY_EXISTS:
				//	hr = E_FAIL;
				//	break;
				case SSH_FX_PERMISSION_DENIED:
					hr2 = E_ACCESSDENIED;
					break;
				default:
					hr2 = E_FAIL;
					break;
			}
			if (FAILED(hr2))
			{
				CMyStringW str2;
				GetSFTPStatusMessage(pData->nResult, pData->strMessage, str2);
				strMsg = strFile;
				strMsg += L": ";
				strMsg += str2;
				if (SUCCEEDED(hr))
					hr = hr2;
			}
			else
				pDirectory->UpdateNewFile(lpszName, false);
				//pDirectory->NotifyUpdate(SHCNE_CREATE, lpszName, NULL);
			delete pData;
		}
	}
	if (FAILED(hr) && !strMsg.IsEmpty())
		::MyMessageBoxW(hWndOwner, strMsg, NULL, MB_ICONHAND);
	return hr;
}

STDMETHODIMP CSFTPFolderSFTP::CreateFTPItemStream(CFTPDirectoryBase* pDirectory, CFTPFileItem* pItem, IStream** ppStream)
{
	CSFTPStream* pStream = new CSFTPStream(&m_xStreamCounter,
		m_pClient,
		m_pChannel,
		pDirectory->m_strDirectory);
	if (!pStream)
		return E_OUTOFMEMORY;
	pStream->m_uliDataSize.QuadPart = pItem->uliSize.QuadPart;
	if (!pStream->TryOpenFile(pItem))
	{
		pStream->Release();
		return E_FAIL;
	}
	*ppStream = pStream;
	return S_OK;
}

#define BUFFER_SIZE      32768

STDMETHODIMP CSFTPFolderSFTP::WriteFTPItem(
	HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszName, IStream* pStream,
	void* pvObject, CTransferStatus* pStatus)
{
	HRESULT hr = S_OK;
	CMyStringW strFile, strMsg;

	strFile = pDirectory->m_strDirectory;
	if (((LPCWSTR) strFile)[strFile.GetLength() - 1] != L'/')
		strFile += L'/';
	strFile += lpszName;

	IncrementTransferCount();

	ULONG uMsgID = m_pChannel->OpenFile(strFile, SSH_FXF_CREAT | SSH_FXF_WRITE | SSH_FXF_TRUNC);
	if (!uMsgID)
	{
		CMyStringW str2;
		str2.LoadString(IDS_COMMAND_UNKNOWN_ERROR);
		strMsg = strFile;
		strMsg += L": ";
		strMsg += str2;
		if (SUCCEEDED(hr))
			hr = E_OUTOFMEMORY;
	}
	else
	{
		CSFTPWaitFileHandle* pHandleData = new CSFTPWaitFileHandle();
		if (!pHandleData)
		{
			hr = E_OUTOFMEMORY;
		}
		else
		{
			pHandleData->uMsgID = uMsgID;
			m_listWaitResponse.Add(pHandleData, uMsgID);

			while (pHandleData->uMsgID)
			{
				if (m_pClient->m_socket.CanReceive(WAIT_RECEIVE_TIME))
				{
					DoReceiveSocket();
					if (!m_pClient || !theApp.MyPumpMessage())
					{
						delete pHandleData;
						//pDirectory->Release();
						return E_FAIL;
					}
				}
				else
				{
					pHandleData->nResult = SSH_FX_NO_CONNECTION;
					break;
				}
			}

			HRESULT hr2;
			switch (pHandleData->nResult)
			{
				case SSH_FX_OK:
					hr2 = S_OK;
					break;
				//case SSH_FX_FILE_ALREADY_EXISTS:
				//	hr = E_FAIL;
				//	break;
				case SSH_FX_PERMISSION_DENIED:
					hr2 = E_ACCESSDENIED;
					break;
				default:
					hr2 = E_FAIL;
					break;
			}
			if (FAILED(hr2))
			{
				CMyStringW str2;
				GetSFTPStatusMessage(pHandleData->nResult, pHandleData->strMessage, str2);
				strMsg = strFile;
				strMsg += L": ";
				strMsg += str2;
				if (SUCCEEDED(hr))
					hr = hr2;
				delete pHandleData;
			}
			else
			{
				HSFTPHANDLE h = pHandleData->hSFTPHandle;
				delete pHandleData;

				void* pv = malloc(BUFFER_SIZE);
				if (!pv)
				{
					hr = E_OUTOFMEMORY;
				}
				else
				{
					ULONGLONG uliOffset;
					ULONG nLen;
					CSFTPWaitConfirm* pData = new CSFTPWaitConfirm();
					if (!pData)
					{
						hr = E_OUTOFMEMORY;
					}
					else
					{
						uliOffset = 0;
						while (true)
						{
							while (theApp.CheckQueueMessage())
							{
								if (!theApp.MyPumpMessage2())
								{
									hr = E_UNEXPECTED;
									break;
								}
							}
							if (FAILED(hr))
								break;
							if (pStatus->TransferIsCanceled(pvObject))
							{
								hr = S_FALSE;
								break;
							}
							hr = pStream->Read(pv, BUFFER_SIZE, &nLen);
							if (FAILED(hr))
								break;
							if (!nLen)
							{
								hr = S_OK;
								break;
							}
							if (pStatus->TransferIsCanceled(pvObject))
							{
								hr = S_FALSE;
								break;
							}
							uMsgID = m_pChannel->WriteFile(h, uliOffset, pv, (size_t) nLen);

							if (!uMsgID)
							{
								CMyStringW str2;
								str2.LoadString(IDS_COMMAND_UNKNOWN_ERROR);
								strMsg = strFile;
								strMsg += L": ";
								strMsg += str2;
								if (SUCCEEDED(hr))
									hr = E_OUTOFMEMORY;
								break;
							}

							pData->uMsgID = uMsgID;
							m_listWaitResponse.Add(pData, uMsgID);

							hr2 = S_OK;
							while (pData->uMsgID)
							{
								if (m_pClient->m_socket.CanReceive(WAIT_RECEIVE_TIME))
								{
									DoReceiveSocket();
									if (!m_pClient || !theApp.MyPumpMessage())
									{
										//pDirectory->Release();
										hr2 = E_FAIL;
										break;
									}
								}
								else
								{
									pData->nResult = SSH_FX_NO_CONNECTION;
									break;
								}
							}
							if (FAILED(hr2))
								break;

							switch (pData->nResult)
							{
								case SSH_FX_OK:
									hr2 = S_OK;
									break;
								//case SSH_FX_FILE_ALREADY_EXISTS:
								//	hr = E_FAIL;
								//	break;
								case SSH_FX_PERMISSION_DENIED:
									hr2 = E_ACCESSDENIED;
									break;
								default:
									hr2 = E_FAIL;
									break;
							}
							if (FAILED(hr2))
							{
								CMyStringW str2;
								GetSFTPStatusMessage(pData->nResult, pData->strMessage, str2);
								strMsg = strFile;
								strMsg += L": ";
								strMsg += str2;
								if (SUCCEEDED(hr))
									hr = hr2;
								break;
							}

							if (!m_pClient || !theApp.MyPumpMessage2())
							{
								//pDirectory->Release();
								hr = E_FAIL;
								break;
							}

							uliOffset += nLen;
							pStatus->TransferInProgress(pvObject, uliOffset);
						}
						delete pData;
					}
					free(pv);
				}
				m_pChannel->CloseHandle(h);
				if (SUCCEEDED(hr))
					pDirectory->UpdateNewFile(lpszName, false);
					//pDirectory->NotifyUpdate(SHCNE_CREATE, lpszName, NULL);
			}
		}
	}
	DecrementTransferCount();
	if (FAILED(hr) && !strMsg.IsEmpty())
		::MyMessageBoxW(hWndOwner, strMsg, NULL, MB_ICONHAND);
	return hr;
}

STDMETHODIMP CSFTPFolderSFTP::CreateInstance(CFTPDirectoryItem* pItemMe, CFTPDirectoryBase* pParent, CFTPDirectoryRootBase* pRoot, LPCWSTR lpszDirectory, CFTPDirectoryBase** ppResult)
{
	CSFTPFolderSFTPDirectory* p = new CSFTPFolderSFTPDirectory(m_pMallocData, pItemMe, pParent, pRoot, lpszDirectory);
	if (!p)
		return E_OUTOFMEMORY;
	*ppResult = p;
	return S_OK;
}

bool CSFTPFolderSFTP::Connect(HWND hWnd, LPCWSTR lpszHostName, int nPort, CUserInfo* pUser)
{
	m_hWndOwner = hWnd;
	if (pUser)
	{
		//const CUserInfo* pBase = (const CUserInfo*) pUser;
		//m_bMyUserInfo = false;
		m_pUser = pUser;
		pUser->AddRef();
		//m_pUser = new CUserInfo();
		//m_pUser->strName = pBase->strName;
		//m_pUser->strPassword = pBase->strPassword;
		//m_pUser->strNewPassword = pBase->strNewPassword;
		//m_pUser->nAuthType = pBase->nAuthType;
		//m_pUser->keyData = pBase->keyData;
		//m_pUser->keyType = pBase->keyType;
		m_bFirstAuthenticate = false;
		//m_nServerCharset = scsUTF8;
	}
	else
	{
		m_pUser = new CUserInfo();
		//// set this flag to show password dialog when connected
		//m_bFirstAuthenticate = true;
		if (!DoRetryAuthentication(NULL, true))
		{
			m_pUser->Release();
			m_pUser = NULL;
			return false;
		}
		m_bFirstAuthenticate = false;
		//m_nServerCharset = scsUTF8;
	}
	{
		CMyStringW str(lpszHostName);
		m_strHostName = str;
	}
	m_nPort = nPort;

	m_pClient = new CSSH2Client();

	const addrinfo* pai = m_pClient->m_socket.TryConnect(nPort, m_strHostName);
	if (!pai)
	{
		m_pClient->Release();
		m_pClient = NULL;
		if (m_pUser)
		{
			m_pUser->Release();
			m_pUser = NULL;
		}

		CMyStringW str;
		str.Format(IDS_UNKNOWN_HOST, lpszHostName);
		::MyMessageBoxW(hWnd, str, NULL, MB_ICONEXCLAMATION);
		return false;
	}
	//SetStatusText(MAKEINTRESOURCEW(IDS_CONNECTING));
	if (!m_pClient->m_socket.CMySocket::Connect(pai))
	{
		m_pClient->Release();
		m_pClient = NULL;
		if (m_pUser)
		{
			m_pUser->Release();
			m_pUser = NULL;
		}

		::MyMessageBoxW(hWnd, MAKEINTRESOURCEW(IDS_FAILED_TO_CONNECT), NULL, MB_ICONEXCLAMATION);
		return false;
	}

	//pSocket->AsyncSelect(m_hWnd, MY_WM_SOCKETMESSAGE, FD_READ | FD_CLOSE);
	m_bLoggedIn = false;
	m_bFirstReceive = true;
	m_bFirstFollowKex = false;
	m_bAuthenticated = false;
	//m_bFirstAuthenticate = false;

	m_idTimer = theApp.RegisterTimer(KEEP_CONNECTION_TIME_SPAN,
		(PFNEASYSFTPTIMERPROC) KeepConnectionTimerProc, (LPARAM) this);

	while (!m_bLoggedIn)
	{
		if (m_pClient->m_socket.CanReceive(WAIT_RECEIVE_TIME))
		{
			DoReceiveSocket();
			if (!m_pClient || !theApp.MyPumpMessage())
			{
				Disconnect();
				return false;
			}
		}
		else if (!m_bAuthenticated)
		{
			Disconnect();
			return false;
		}
	}
	return true;
}

//static void __stdcall _DumpDirectories(CFTPDirectoryItem* pItem, size_t nSpaceCount)
//{
//	CMyStringW strDump, strSpace(L' ', nSpaceCount);
//	strDump.Format(L"%spItem = %p\n%spDirectory = %p\n",
//		(LPCWSTR) strSpace, pItem,
//		(LPCWSTR) strSpace, pItem->pDirectory);
//	OutputDebugString(strDump);
//	if (pItem->pDirectory)
//	{
//		strDump.Format(L"%sFiles:\n", (LPCWSTR) strSpace);
//		OutputDebugString(strDump);
//		for (int i = 0; i < pItem->pDirectory->m_aFiles.GetCount(); i++)
//		{
//			CFTPFileItem* pFile = pItem->pDirectory->m_aFiles.GetItem(i);
//			strDump.Format(L"%spFile = %p (%s)\n",
//				(LPCWSTR) strSpace, pFile, (LPCWSTR) pFile->strFileName);
//			OutputDebugString(strDump);
//		}
//		strDump.Format(L"%ssub directories:\n", (LPCWSTR) strSpace);
//		OutputDebugString(strDump);
//		for (int i = 0; i < pItem->pDirectory->m_aDirectories.GetCount(); i++)
//		{
//			CFTPDirectoryItem* pItem2 = pItem->pDirectory->m_aDirectories.GetItem(i);
//			_DumpDirectories(pItem2, nSpaceCount + 2);
//		}
//	}
//}

STDMETHODIMP CSFTPFolderSFTP::Disconnect()
{
	if (!m_pClient)
		return S_OK;

	::EnterCriticalSection(&m_csSocket);
	theApp.UnregisterTimer(m_idTimer);
	m_idTimer = 0;

//#ifdef _DEBUG
//	{
//		CMyStringW strDump;
//		strDump.Format(L"Disconnect():\nm_pClient = %p\nm_pChannel = %p\n", m_pClient, m_pChannel);
//		OutputDebugString(strDump);
//		OutputDebugString(_T("directories:\n"));
//		for (int i = 0; i < m_aDirectories.GetCount(); i++)
//		{
//			CFTPDirectoryItem* pItem = m_aDirectories.GetItem(i);
//			_DumpDirectories(pItem, 2);
//		}
//		OutputDebugString(strDump);
//	}
//#endif

	if (m_pClient->m_socket.IsConnected())
	{
		if (m_pChannel && !m_pClient->m_socket.IsRemoteClosed())
			m_pChannel->CloseChannel();
		//m_pClient->m_socket.CTextSocket::Close();
		m_pClient->m_socket.Close();
	}
	//_INDEX ind = m_listSFTPSendFile.GetFirstItemPosition();
	//while (ind)
	//{
	//	CSFTPSendFileData* pData = m_listSFTPSendFile.GetNextItem(ind);
	//	if (pData->hFile)
	//	{
	//		m_pChannel->CloseHandle(pData->hFile);
	//		pData->pStream->Release();
	//	}
	//	delete pData;
	//}
	//m_listSFTPSendFile.RemoveAll();
	if (m_pChannel)
	{
		//if (m_hDirFile)
		//	m_pChannel->CloseHandle(m_hDirFile);
		m_pChannel->Release();
		m_pChannel = NULL;
	}
	m_pClient->Release();
	m_pClient = NULL;
	::LeaveCriticalSection(&m_csSocket);

	RemoveAllFiles();

	if (m_pUser)
	{
		m_pUser->Release();
		m_pUser = NULL;
	}

	//if (m_bFirstMessageReceived)
	//	m_bFirstMessageReceived = false;
	//else
	//	::MyMessageBoxW(m_hWndOwner, MAKEINTRESOURCEW(IDS_FAILED_TO_CONNECT), NULL, MB_ICONEXCLAMATION);

	m_hWndOwner = NULL;

	//m_strHostName.Empty();
	return S_OK;
}

void CSFTPFolderSFTP::IncrementTransferCount()
{
	::InterlockedIncrement((LONG*) &m_dwTransferringCount);
}

void CSFTPFolderSFTP::DecrementTransferCount()
{
	::InterlockedDecrement((LONG*) &m_dwTransferringCount);
}

void CSFTPFolderSFTP::PreShowPropertyDialog(CServerFilePropertyDialog* pDialog)
{
	pDialog->m_bChangeOwner = true;
	pDialog->m_bChangeAttr = true;
	pDialog->m_bSupportedName = (m_pChannel->GetServerVersion() >= 4);
}

bool CSFTPFolderSFTP::PreShowCreateShortcutDialog(CLinkDialog* pDialog)
{
	if (pDialog)
		pDialog->m_bAllowHardLink = (m_pChannel->GetServerVersion() >= 6);
	return true;
}

void CSFTPFolderSFTP::PreShowServerInfoDialog(CServerInfoDialog* pDialog)
{
	CMyStringW str;
	str.LoadString(IDS_SERVER_HOST);
	pDialog->m_strInfo = str;
	pDialog->m_strInfo += L": ";
	pDialog->m_strInfo += m_strHostName;
	pDialog->m_strInfo += L"\r\n";
	str.LoadString(IDS_SERVER_ADDRESS);
	pDialog->m_strInfo += str;
	pDialog->m_strInfo += L": ";

	{
		size_t nLen;
		const sockaddr* psa = m_pClient->m_socket.GetConnectedAddress(&nLen);
		getnameinfo(psa, (socklen_t) nLen, str.GetBufferA(NI_MAXHOST + 1), NI_MAXHOST + 1, NULL, 0, NI_NUMERICHOST);
		str.ReleaseBufferA();
		pDialog->m_strInfo += str;
	}

	pDialog->m_strInfo += L"\r\n";
	if (m_pClient)
	{
		str.LoadString(IDS_SSH_VERSION);
		pDialog->m_strInfo += str;
		pDialog->m_strInfo += L": ";
		pDialog->m_strInfo += m_pClient->m_strServerName;
		pDialog->m_strInfo += L"\r\n";
		if (m_pClient->m_pKex)
		{
			str.LoadString(IDS_SSH_KEY_TYPE);
			pDialog->m_strInfo += str;
			pDialog->m_strInfo += L": ";
			pDialog->m_strInfo += KeyTypeToName((KeyType) m_pClient->m_pKex->m_nKeyType);
			pDialog->m_strInfo += L"\r\n";
			str.LoadString(IDS_SSH_KEX);
			pDialog->m_strInfo += str;
			pDialog->m_strInfo += L": ";
			pDialog->m_strInfo += m_pClient->m_pKex->m_pKexClient->GetKexTypeName();
			pDialog->m_strInfo += L"\r\n";
		}
	}
	if (m_pChannel)
	{
		str.LoadString(IDS_SFTP_VERSION);
		pDialog->m_strInfo += str;
		pDialog->m_strInfo += L": ";
		str.Format(L"%lu", m_pChannel->GetServerVersion());
		pDialog->m_strInfo += str;
		pDialog->m_strInfo += L"\r\n";
	}
}

bool CSFTPFolderSFTP::ReceiveDirectory(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszDirectory, bool* pbReceived)
{
	::EnterCriticalSection(&m_csSocket);
	if (m_pClient && m_pClient->m_socket.IsRemoteClosed())
	{
		::LeaveCriticalSection(&m_csSocket);
		Disconnect();
		::EnterCriticalSection(&m_csSocket);
	}
	if (!m_pClient)
	{
		if (!Connect(hWndOwner, m_strHostName, m_nPort, NULL))
		{
			::LeaveCriticalSection(&m_csSocket);
			return false;
		}
	}
	if (*pbReceived)
	{
		::LeaveCriticalSection(&m_csSocket);
		return true;
	}
	CSFTPWaitDirectoryData* pData = new CSFTPWaitDirectoryData();
	if (!pData)
	{
		::LeaveCriticalSection(&m_csSocket);
		return false;
	}
	ULONG uMsg = m_pChannel->OpenDirectory(lpszDirectory);
	if (!uMsg)
	{
		delete pData;
		::LeaveCriticalSection(&m_csSocket);
		return false;
	}
	pData->uMsgID = uMsg;
	pData->nStep = CSFTPWaitDirectoryData::stepRetrieveHandle;
	pData->hSFTPHandle = NULL;
	pData->bResult = false;
	pData->dwReadLinkCount = 0;
	pData->pDirectory = (CSFTPFolderSFTPDirectory*) pDirectory;
	m_listWaitResponse.Add(pData, uMsg);
	pDirectory->AddRef();
	while (pData->nStep)
	{
		if (m_pClient->m_socket.CanReceive(WAIT_RECEIVE_TIME))
		{
			DoReceiveSocket();
			if (!m_pClient || !theApp.MyPumpMessage())
			{
				if (m_pChannel && pData->hSFTPHandle)
					m_pChannel->CloseHandle(pData->hSFTPHandle);
				delete pData;
				pDirectory->Release();
				::LeaveCriticalSection(&m_csSocket);
				return false;
			}
		}
		else
		{
			pData->bResult = false;
			break;
		}
	}
	bool ret = pData->bResult;
	if (m_pChannel && pData->hSFTPHandle)
		m_pChannel->CloseHandle(pData->hSFTPHandle);
	if (pData->uMsgID)
		m_listWaitResponse.Remove(pData->uMsgID);
	delete pData;
	pDirectory->Release();
	*pbReceived = true;
	::LeaveCriticalSection(&m_csSocket);
	return ret;
}

bool CSFTPFolderSFTP::ValidateDirectory(LPCWSTR lpszParentDirectory, PCUIDLIST_RELATIVE pidlChild,
	CMyStringW& rstrRealPath)
{
	CMyStringW strPath = lpszParentDirectory;
	bool bLastIsNotDelimiter, bFirst;
	bLastIsNotDelimiter = (strPath.IsEmpty() || ((LPCWSTR) strPath)[strPath.GetLength() - 1] != L'/');
	CMyStringW str;
	bFirst = true;
	while (pidlChild && pidlChild->mkid.cb)
	{
		if (!PickupFileName((PUITEMID_CHILD) pidlChild, str))
			return false;
		if (bLastIsNotDelimiter)
			strPath += L'/';
		strPath += str;
		bLastIsNotDelimiter = true;
		pidlChild = (PCUIDLIST_RELATIVE) (((DWORD_PTR) pidlChild) + pidlChild->mkid.cb);
	}

	// validate only if connected
#if 1 || defined(_FORCE_VALIDATE)
	if (m_pClient)
	{
		::EnterCriticalSection(&m_csSocket);
		if (!m_pClient)
		{
			::LeaveCriticalSection(&m_csSocket);
			return false;
		}
		ULONG uMsg = m_pChannel->RealPath(strPath);
		if (!uMsg)
		{
			::LeaveCriticalSection(&m_csSocket);
			return false;
		}

		CSFTPWaitAttrData* pAttr = new CSFTPWaitAttrData();
		pAttr->uMsgID = uMsg;
		pAttr->nType = CSFTPWaitAttrData::typeRealPath;
		pAttr->strFileName = strPath;
		m_listWaitResponse.Add(pAttr, uMsg);
		while (pAttr->nType != CSFTPWaitAttrData::typeEnd)
		{
			if (m_pClient->m_socket.CanReceive(WAIT_RECEIVE_TIME))
			{
				DoReceiveSocket();
				if (!m_pClient || !theApp.MyPumpMessage())
				{
					delete pAttr;
					::LeaveCriticalSection(&m_csSocket);
					return false;
				}
			}
			else
			{
				if (pAttr->uMsgID)
					m_listWaitResponse.Remove(pAttr->uMsgID);
				delete pAttr;
				::LeaveCriticalSection(&m_csSocket);
				return false;
			}
		}
		::LeaveCriticalSection(&m_csSocket);
		if (pAttr->strFileName.IsEmpty())
		{
			delete pAttr;
			return false;
		}
		// TODO: validate if it is 'real' directory
		rstrRealPath = pAttr->strFileName;
		delete pAttr;
	}
	else
#endif
		rstrRealPath = strPath;
	return true;
}

CFTPFileItem* CSFTPFolderSFTP::RetrieveFileItem(CFTPDirectoryBase* pDirectory, LPCWSTR lpszFileName)
{
	CMyStringW strPath = pDirectory->m_strDirectory;
	if (strPath.IsEmpty() || ((LPCWSTR) strPath)[strPath.GetLength() - 1] != L'/')
		strPath += L'/';
	strPath += lpszFileName;

	::EnterCriticalSection(&m_csSocket);
	ULONG uMsg = m_pChannel->LStat(strPath);
	if (!uMsg)
	{
		::LeaveCriticalSection(&m_csSocket);
		return NULL;
	}

	CSFTPWaitAttrData* pAttr = new CSFTPWaitAttrData();
	pAttr->uMsgID = uMsg;
	pAttr->nType = CSFTPWaitAttrData::typeNormal;
	pAttr->strFileName = strPath;
	pAttr->nResult = SSH_FX_OK;
	m_listWaitResponse.Add(pAttr, uMsg);
	while (pAttr->uMsgID)
	{
		if (m_pClient->m_socket.CanReceive(WAIT_RECEIVE_TIME))
		{
			DoReceiveSocket();
			if (!m_pClient || !theApp.MyPumpMessage())
			{
				delete pAttr;
				::LeaveCriticalSection(&m_csSocket);
				return false;
			}
		}
		else
		{
			if (pAttr->uMsgID)
				m_listWaitResponse.Remove(pAttr->uMsgID);
			delete pAttr;
			::LeaveCriticalSection(&m_csSocket);
			return false;
		}
	}
	::LeaveCriticalSection(&m_csSocket);
	if (pAttr->nResult != SSH_FX_OK)
	{
		delete pAttr;
		return false;
	}
	CFTPFileItem* pItem;
	{
		CSFTPFileData data;
		data.strFileName = lpszFileName;
		memcpy(&data.attr, &pAttr->fileData, sizeof(data.attr));
		pItem = ParseSFTPData(m_pChannel->GetServerVersion(), &data);
		if (pAttr->fileData.aExAttrs)
			free(pAttr->fileData.aExAttrs);
	}
	delete pAttr;
	return pItem;
}

////////////////////////////////////////////////////////////////////////////////

void CALLBACK CSFTPFolderSFTP::KeepConnectionTimerProc(UINT_PTR idEvent, LPARAM lParam)
{
	CSFTPFolderSFTP* pThis = (CSFTPFolderSFTP*) lParam;

	// send 'ignore'/no-op message to keep connection
	pThis->m_pClient->m_socket.SendPacket(SSH2_MSG_IGNORE, NULL, 0);
}

void CSFTPFolderSFTP::DoReceiveSocket()
{
	while (true)
	{
		OnSFTPSocketReceive();
		if (!m_pClient || !m_pClient->m_socket.HasReceivedData())
			break;
	}
}

void CSFTPFolderSFTP::DoNextReadDirectory(CSFTPWaitDirectoryData* pData)
{
	pData->uMsgID = m_pChannel->ReadDirectory(pData->hSFTPHandle);
	if (!pData->uMsgID)
	{
		pData->nStep = CSFTPWaitDirectoryData::stepFinished;
		m_pChannel->CloseHandle(pData->hSFTPHandle);
		pData->hSFTPHandle = NULL;
		pData->bResult = false;
	}
	else
		m_listWaitResponse.Add(pData, pData->uMsgID);
}

#define SSH_AUTH_SERVICE "ssh-userauth"

void CSFTPFolderSFTP::OnSFTPSocketReceive()
{
	::EnterCriticalSection(&m_csReceive);
	UINT u = _OnSFTPSocketReceiveThreadUnsafe();
	::LeaveCriticalSection(&m_csReceive);
	if (u)
	{
		Disconnect();
		if (u != (UINT) -1)
			::MyMessageBoxW(m_hWndOwner, MAKEINTRESOURCEW(u), NULL, MB_ICONEXCLAMATION);
	}
}

UINT CSFTPFolderSFTP::_OnSFTPSocketReceiveThreadUnsafe()
{
	if (m_bFirstReceive)
	{
		m_bFirstReceive = false;
		m_bFirstFollowKex = false;
		//m_bFirstMessageReceived = true;
		if (!m_pClient->OnFirstReceive())
			return IDS_FAILED_TO_CONNECT;
	}

	BYTE bType;
	size_t nLen;
	void* pv = m_pClient->m_socket.ReceivePacket(bType, nLen);
	if (!pv)
	{
		return (UINT) -1;
	}

	if (m_bFirstFollowKex)
	{
		m_bFirstFollowKex = false;
		if (m_pClient->OnKeyExchangeInit(pv, nLen, true) < 1)
		{
			free(pv);
			return IDS_FAILED_TO_CONNECT;
		}
	}
	UINT ret = 0;
	switch (bType)
	{
		case SSH2_MSG_KEXINIT:
		{
			int r = m_pClient->OnKeyExchangeInit(pv, nLen);
			if (r < 0)
			{
				ret = IDS_FAILED_TO_CONNECT;
			}
			else if (r == 0)
				m_bFirstFollowKex = true;
		}
		break;
		case SSH2_MSG_KEXDH_REPLY:
#if (SSH2_MSG_KEX_DH_GEX_GROUP != SSH2_MSG_KEXDH_REPLY)
		case SSH2_MSG_KEX_DH_GEX_GROUP:
#endif
		case SSH2_MSG_KEX_DH_GEX_REPLY:
		{
			int r = m_pClient->OnReceiveKexMessages(this, bType, pv, nLen);
			if (r < 0)
			{
				ret = IDS_FAILED_TO_CONNECT;
			}
		}
		break;
		case SSH2_MSG_NEWKEYS:
			m_pClient->UpdateServerReceiveKeyData();
			m_pClient->m_socket.SendPacketString(SSH2_MSG_SERVICE_REQUEST, SSH_AUTH_SERVICE);
			break;
		case SSH2_MSG_SERVICE_ACCEPT:
			m_pUser->pvSessionID = m_pClient->m_pKex->m_pvSessionID;
			m_pUser->nSessionIDLen = m_pClient->m_pKex->m_nSessionIDLen;
			m_pUser->bSecondary = false;
			// m_bFirstAuthenticate が true のときは none 認証を行い、
			// 実際に可能な認証方法を得る
			m_pClient->Authenticate(SSH_AUTH_SERVICE,
				m_bFirstAuthenticate ? AUTHTYPE_NONE : m_pUser->nAuthType,
				pv, nLen, m_pUser);
			break;
		case SSH2_MSG_USERAUTH_INFO_REQUEST:
#if (SSH2_MSG_USERAUTH_PK_OK != SSH2_MSG_USERAUTH_INFO_REQUEST)
		case SSH2_MSG_USERAUTH_PK_OK:
#endif
			if (m_pUser->nAuthType == AUTHTYPE_PAGEANT)
			{
				m_pUser->bSecondary = true;
				m_pClient->DoAuthenticate(m_pUser->nAuthType, m_pUser);
			}
			break;
		case SSH2_MSG_USERAUTH_FAILURE:
		{
			CExBuffer buf;
			ULONG uSize;
			BYTE bPartial;
			void* pv2;
			if (!buf.SetDataToBuffer(pv, nLen) ||
				!buf.GetAndSkipCE(uSize) ||
				!(pv2 = buf.GetCurrentBufferPermanentAndSkip((size_t) uSize)) ||
				!buf.GetAndSkip(bPartial) ||
				!buf.IsEmpty())
			{
				ret = IDS_FAILED_TO_CONNECT;
			}
			else
			{
				if (!m_pClient->CanRetryAuthenticate())
				{
					m_pClient->EndAuthenticate();
					// "type1,type2,type3" を "type1\0type2\0type3\0" にする
					char* psz = (char*) malloc(sizeof(char) * 2 + (size_t) uSize);
					memcpy(psz, pv2, (size_t) uSize);
					*((char*) (((BYTE*) psz) + uSize)) = 0;
					*(((char*) (((BYTE*) psz) + uSize)) + 1) = 0;
					char* psz2 = psz;
					while (*psz2)
					{
						if (*psz2 == ',')
							*psz2 = 0;
						psz2++;
					}
					if (DoRetryAuthentication(psz, m_bFirstAuthenticate))
					{
						m_bFirstAuthenticate = false;
						m_pUser->bSecondary = false;
						m_pClient->DoAuthenticate(m_pUser->nAuthType, m_pUser);
					}
					else
					{
						ret = (UINT) -1;
					}
					free(psz);
				}
				else
				{
					m_bFirstAuthenticate = false;
					m_pUser->bSecondary = false;
					m_pClient->DoAuthenticate(m_pUser->nAuthType, m_pUser);
				}
			}
		}
		break;
		case SSH2_MSG_USERAUTH_BANNER:
			m_pClient->EndAuthenticate();
			ret = IDS_FAILED_TO_CONNECT_BANNED;
			break;
		case SSH2_MSG_USERAUTH_SUCCESS:
			m_pClient->EndAuthenticate();
			//SetStatusText(MAKEINTRESOURCEW(IDS_CONNECTED));
			if (m_pUser)
			{
				m_pUser->Release();
				m_pUser = NULL;
			}
			m_bAuthenticated = true;
			m_pChannel = new CSFTPChannel(this);
			m_pChannel->OpenChannel();
			break;
		default:
			CSSH2Channel::ProcessChannelMsg(this, bType, pv, nLen);
			if (m_pChannel && !m_pChannel->FlushAllMessages())
			{
#ifdef _DEBUG
				OutputDebugString(_T("\tFlushAllMessages() returned false\n"));
#endif
			}
			break;
	}

	free(pv);
	return ret;
}

////////////////////////////////////////////////////////////////////////////////

static void __stdcall FingerPrintToString(const BYTE* pFingerPrint, size_t nLen, CMyStringW& string)
{
	WCHAR szBuffer[3];
	string.Empty();
	while (nLen--)
	{
		_snwprintf_s(szBuffer, 3, L"%02x", (UINT) *pFingerPrint++);
		szBuffer[2] = 0;
		string += szBuffer;
		if (nLen)
			string += L':';
	}
}

static void __stdcall _MergeFingerPrint(const CMyStringW& strHostName, const BYTE* pFingerPrint, size_t nLen)
{
	CGeneralSettings settings;
	CMyPtrArrayT<CHostSettings> aHostSettings;
	CMyPtrArrayT<CKnownFingerPrint> aKnownFingerPrints;
	CKnownFingerPrint* pPrint = NULL;

	theApp.LoadINISettings(&settings, &aKnownFingerPrints, &aHostSettings);

	for (int n = 0; n < aKnownFingerPrints.GetCount(); n++)
	{
		pPrint = aKnownFingerPrints.GetItem(n);
		if (strHostName.Compare(pPrint->strHostName, true) == 0)
		{
			if (pPrint->nFingerPrintLen != nLen)
			{
				BYTE* pb = (BYTE*) realloc(pPrint->pFingerPrint, nLen);
				if (!pb)
					break;
				pPrint->pFingerPrint = pb;
			}
			memcpy(pPrint->pFingerPrint, pFingerPrint, nLen);
			break;
		}
		pPrint = NULL;
	}
	if (!pPrint)
	{
		pPrint = new CKnownFingerPrint();
		pPrint->pFingerPrint = (BYTE*) malloc(nLen);
		if (!pPrint->pFingerPrint)
		{
			delete pPrint;
			theApp.EmptyHostSettings(aHostSettings);
			theApp.EmptyKnownFingerPrints(aKnownFingerPrints);
			return;
		}
		pPrint->nFingerPrintLen = nLen;
		pPrint->strHostName = strHostName;
		memcpy(pPrint->pFingerPrint, pFingerPrint, nLen);
		aKnownFingerPrints.Add(pPrint);
	}

	theApp.SaveINISettings(&settings, &aKnownFingerPrints, &aHostSettings);
	theApp.EmptyHostSettings(aHostSettings);
	theApp.EmptyKnownFingerPrints(aKnownFingerPrints);
}

bool __stdcall CSFTPFolderSFTP::CheckFingerPrint(const BYTE* pFingerPrint, size_t nLen)
{
	CMyPtrArrayT<CKnownFingerPrint> aKnownFingerPrints;
	CKnownFingerPrint* pPrint;

	theApp.LoadINISettings(NULL, &aKnownFingerPrints, NULL);

	for (int n = 0; n < aKnownFingerPrints.GetCount(); n++)
	{
		pPrint = aKnownFingerPrints.GetItem(n);
		if (m_strHostName.Compare(pPrint->strHostName, true) == 0)
		{
			if (pPrint->nFingerPrintLen != nLen ||
				memcmp(pPrint->pFingerPrint, pFingerPrint, nLen) != 0)
			{
				CMyStringW strPrint;
				CMyStringW strPrint2;
				CMyStringW str;
				FingerPrintToString(pFingerPrint, nLen, strPrint);
				FingerPrintToString(pPrint->pFingerPrint, pPrint->nFingerPrintLen, strPrint2);
				str.Format(IDS_FINGER_PRINT_MISMATCH, (LPCWSTR) m_strHostName,
					(LPCWSTR) strPrint2, (LPCWSTR) strPrint);
				switch (::MyMessageBoxW(m_hWndOwner, str, NULL, MB_ICONHAND | MB_YESNOCANCEL))
				{
					case IDCANCEL:
					default:
						theApp.EmptyKnownFingerPrints(aKnownFingerPrints);
						return false;
					case IDYES:
						_MergeFingerPrint(m_strHostName, pFingerPrint, nLen);
					case IDNO:
						theApp.EmptyKnownFingerPrints(aKnownFingerPrints);
						break;
				}
			}
			else
				theApp.EmptyKnownFingerPrints(aKnownFingerPrints);
			return true;
		}
	}

	theApp.EmptyKnownFingerPrints(aKnownFingerPrints);

	CMyStringW strPrint;
	CMyStringW str;
	FingerPrintToString(pFingerPrint, nLen, strPrint);
	str.Format(IDS_NEW_FINGER_PRINT, (LPCWSTR) m_strHostName,
		(LPCWSTR) strPrint);
	switch (::MyMessageBoxW(m_hWndOwner, str, NULL, MB_ICONEXCLAMATION | MB_YESNOCANCEL))
	{
		case IDCANCEL:
		default:
			return false;
		case IDYES:
			_MergeFingerPrint(m_strHostName, pFingerPrint, nLen);
		case IDNO:
			break;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////

void CSFTPFolderSFTP::ChannelOpenFailure(CSSH2Channel* pChannel, int nReason, const CMyStringW& strMessage)
{
	if (pChannel != (CSSH2Channel*) m_pChannel)
		return;
	Disconnect();
	//::MyMessageBoxW(m_hWndOwner, MAKEINTRESOURCEW(IDS_FAILED_TO_CONNECT), NULL, MB_ICONEXCLAMATION);
}

void CSFTPFolderSFTP::ChannelOpened(CSSH2Channel* pChannel)
{
	if (pChannel != (CSSH2Channel*) m_pChannel)
		return;

	//m_pClient->NoMoreSessions();
	m_pChannel->SendChannelRequest("subsystem", true, "sftp", 4);
}

void CSFTPFolderSFTP::ChannelClosed(CSSH2Channel* pChannel)
{
	if (pChannel != (CSSH2Channel*) m_pChannel)
		return;
	Disconnect();
}

void CSFTPFolderSFTP::ChannelExitStatus(CSSH2Channel* pChannel, int nExitCode)
{
	if (pChannel != (CSSH2Channel*) m_pChannel)
		return;
}

void CSFTPFolderSFTP::ChannelConfirm(CSSH2Channel* pChannel, bool bSucceeded)
{
	if (pChannel != (CSSH2Channel*) m_pChannel)
		return;
	if (!bSucceeded)
	{
		Disconnect();
		//::MyMessageBoxW(m_hWndOwner, MAKEINTRESOURCEW(IDS_FAILED_TO_CONNECT), NULL, MB_ICONEXCLAMATION);
		return;
	}
	m_pChannel->InitSFTP();
}

void CSFTPFolderSFTP::SFTPOpened(CSFTPChannel* pChannel)
{
	if (pChannel != m_pChannel)
		return;
	m_bLoggedIn = true;
	pChannel->SetCharset((ServerCharset) m_nServerCharset);
	////UpdateServerFolderAbsolute(L"/");
	//UpdateServerFolderAbsolute(L".");
}

void CSFTPFolderSFTP::SFTPConfirm(CSFTPChannel* pChannel, CSFTPMessage* pMsg, int nStatus, const CMyStringW& strMessage)
{
	if (pChannel != m_pChannel)
		return;
	ULONG uMsgID = pMsg->uMsgID;
	if (m_listWaitResponse.IsItemKey(uMsgID))
	{
		CWaitResponseData* p = m_listWaitResponse.GetItem(uMsgID);
		switch (p->nWaitType)
		{
			case CWaitResponseData::WRD_FTPWAITATTR:
			{
				CSFTPWaitAttrData* pData = (CSFTPWaitAttrData*) p;
				m_listWaitResponse.Remove(uMsgID);
				switch (pData->nType)
				{
					case CSFTPWaitAttrData::typeNormal:
						pData->pItem = NULL;
						pData->nType = CSFTPWaitAttrData::typeEnd;
						pData->fileData.aExAttrs = NULL;
						break;
					case CSFTPWaitAttrData::typeRealPath:
						pData->strFileName.Empty();
						pData->nType = CSFTPWaitAttrData::typeEnd;
						return;
					case CSFTPWaitAttrData::typeRetrieveFile:
						break;
					case CSFTPWaitAttrData::typeReadLink:
						if (pData->pWaitDir)
						{
							CSFTPWaitDirectoryData* pWaitDir = pData->pWaitDir;
							pWaitDir->dwReadLinkCount--;
							if (!pWaitDir->dwReadLinkCount)
								DoNextReadDirectory(pWaitDir);
						}
						break;
				}
				//delete pData;
				pData->uMsgID = 0;
				pData->nResult = nStatus;
				pData->strMessage = strMessage;
				//goto DoDefault;
			}
			break;
			case CWaitResponseData::WRD_DIRECTORY:
			{
				CSFTPWaitDirectoryData* pData = (CSFTPWaitDirectoryData*) p;
				pData->uMsgID = 0;
				m_listWaitResponse.Remove(uMsgID);
				if (nStatus == SSH_FX_OK && pData->nStep == CSFTPWaitDirectoryData::stepRetrieveFiles)
				{
					pData->nStep = CSFTPWaitDirectoryData::stepFinished;
					pData->hSFTPHandle = NULL;
					pData->bResult = true;
				}
				else if (nStatus == SSH_FX_EOF && pMsg->bSentMsg == SSH_FXP_READDIR)
				{
					pData->uMsgID = uMsgID = pChannel->CloseHandle(pData->hSFTPHandle);
					pData->hSFTPHandle = NULL;
					if (!uMsgID)
					{
						pData->nStep = CSFTPWaitDirectoryData::stepFinished;
						pData->bResult = false;
					}
					else
						m_listWaitResponse.Add(pData, uMsgID);
				}
				else if (nStatus != SSH_FX_OK)
				{
					pData->nStep = CSFTPWaitDirectoryData::stepFinished;
					if (pData->hSFTPHandle)
						pChannel->CloseHandle(pData->hSFTPHandle);
					pData->hSFTPHandle = NULL;
					pData->bResult = false;
				}
			}
			break;
			case CWaitResponseData::WRD_CONFIRM:
			case CWaitResponseData::WRD_FILEHANDLE:
			{
				CSFTPWaitConfirm* pData = (CSFTPWaitConfirm*) p;
				pData->uMsgID = 0;
				pData->nResult = nStatus;
				pData->strMessage = strMessage;
				m_listWaitResponse.Remove(uMsgID);
			}
			break;
			case CWaitResponseData::WRD_SETSTAT:
			{
				CSFTPWaitSetStat* pData = (CSFTPWaitSetStat*) p;
				pData->uMsgID = 0;
				//*(pData->pbResult) = (nStatus == SSH_FX_OK);
				pData->nResult = nStatus;
				pData->strMessage = strMessage;
				m_listWaitResponse.Remove(uMsgID);
			}
			break;
		}
	}
}

void CSFTPFolderSFTP::SFTPFileHandle(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
	HSFTPHANDLE hSFTP)
{
	if (pChannel != m_pChannel)
		return;
	ULONG uMsgID = pMsg->uMsgID;
	if (m_listWaitResponse.IsItemKey(uMsgID))
	{
		CWaitResponseData* p = m_listWaitResponse.GetItem(pMsg->uMsgID);
		switch (p->nWaitType)
		{
			case CWaitResponseData::WRD_DIRECTORY:
			{
				CSFTPWaitDirectoryData* pData = (CSFTPWaitDirectoryData*) p;
				pData->nStep = CSFTPWaitDirectoryData::stepRetrieveFiles;
				pData->hSFTPHandle = hSFTP;
				m_listWaitResponse.Remove(uMsgID);
				pData->uMsgID = uMsgID = m_pChannel->ReadDirectory(hSFTP);
				if (!uMsgID)
				{
					pData->nStep = CSFTPWaitDirectoryData::stepFinished;
					pData->hSFTPHandle = NULL;
					pChannel->CloseHandle(hSFTP);
					pData->bResult = false;
				}
				else
					m_listWaitResponse.Add(pData, uMsgID);
			}
			break;
			case CWaitResponseData::WRD_FILEHANDLE:
			{
				CSFTPWaitFileHandle* pData = (CSFTPWaitFileHandle*) p;
				pData->uMsgID = 0;
				pData->hSFTPHandle = hSFTP;
				pData->nResult = SSH_FX_OK;
				m_listWaitResponse.Remove(uMsgID);
			}
			break;
		}
	}
	//if (m_listSFTPSendFile.IsItemKey(uMsgID))
	//{
	//	::EnterCriticalSection(&m_csListSendFile);
	//	_INDEX ind = m_listSFTPSendFile.PositionFromKey(uMsgID);
	//	CSFTPSendFileData* pData = m_listSFTPSendFile.GetItem(ind);
	//	m_listSFTPSendFile.Remove(ind);
	//	::LeaveCriticalSection(&m_csListSendFile);
	//	pData->hFile = hSFTP;
	//	pData->uliOffset = 0;
	//	if (pData->bForSend)
	//		_DoSendSFTPFile2(pData);
	//	else
	//		_DoRecvSFTPFile2(pData, NULL, 0);
	//}
	//if (m_listWaitForAttribute.IsItemKey(pMsg->uMsgID))
	//{
	//	CSFTPWaitAttrData* pData = m_listWaitForAttribute.GetItem(pMsg->uMsgID);
	//	m_listWaitForAttribute.Remove(pMsg->uMsgID);
	//	if (pData->nType == 2)
	//	{
	//		pData->hDirHandle = hSFTP;
	//		ULONG u = pChannel->ReadDirectory(hSFTP);
	//		if (!u)
	//		{
	//			delete pData->pDirData;
	//			delete pData;
	//			pChannel->CloseHandle(hSFTP);
	//			::MessageBeep(MB_ICONEXCLAMATION);
	//			SetStatusText(MAKEINTRESOURCEW(IDS_DIRCHANGE_FAILED));
	//		}
	//		else
	//			m_listWaitForAttribute.Add(pData, u);
	//	}
	//}
	//else if (uMsgID == m_uDirMsg)
	//{
	//	m_hDirFile = hSFTP;
	//	m_wndListViewServer.DeleteAllItems();
	//	m_uDirMsg = pChannel->ReadDirectory(hSFTP);
	//}
}

//static void __stdcall _ParseSFTPFileName(CSFTPFolderSFTPDirectory* pThis, const CSFTPFileData* aFiles, int nCount, CRecvDirectoryData* pData)
static void __stdcall _ParseSFTPFileName(CSFTPFolderSFTPDirectory* pThis, const CSFTPFileData* aFiles, int nCount)
{
	while (nCount--)
	{
		CFTPFileItem* pItem;
		pItem = ParseSFTPData(((CSFTPFolderSFTP*) pThis->m_pRoot)->m_pChannel->GetServerVersion(), aFiles++);
		if (pItem)
		{
			if (pItem->type != fitypeCurDir && pItem->type != fitypeParentDir &&
				pItem->strFileName.Compare(L".") && pItem->strFileName.Compare(L".."))
			{
				//if (pData)
				//	pData->aItems.Add(pItem);
				//else
				{
					if (pItem->IsDirectory())
					{
						CFTPDirectoryItem* p = new CFTPDirectoryItem();
						p->strName = pItem->strFileName;
						p->pDirectory = NULL;
						pThis->m_aDirectories.Add(p);
					}
					pThis->m_aFiles.Add(pItem);
				}
			}
			else
				pItem->Release();
		}
	}
}

void CSFTPFolderSFTP::SFTPReceiveFileName(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
	const CSFTPFileData* aFiles, int nCount)
{
	if (pChannel != m_pChannel)
		return;
	ULONG uMsgID = pMsg->uMsgID;
	if (m_listWaitResponse.IsItemKey(uMsgID))
	{
		CWaitResponseData* p = m_listWaitResponse.GetItem(pMsg->uMsgID);
		switch (p->nWaitType)
		{
			case CWaitResponseData::WRD_FTPWAITATTR:
			{
				CSFTPWaitAttrData* pData = (CSFTPWaitAttrData*) p;
				m_listWaitResponse.Remove(uMsgID);
				pData->uMsgID = 0;
				switch (pData->nType)
				{
					case CSFTPWaitAttrData::typeRealPath:
						pData->strFileName = aFiles[0].strFileName;
						pData->nType = CSFTPWaitAttrData::typeEnd;
						// no delete pData
						//return;
						break;
				//else if (pData->nType == CSFTPWaitAttrData::typeRetrieveFile)
				//{
				//	if (pData->hDirHandle)
				//	{
				//		_ParseSFTPFileName(this, aFiles, nCount, pData->pDirData);
				//		pChannel->CloseHandle(pData->hDirHandle);
				//		DoRecvDirectory(pData->pDirData);
				//		delete pData;
				//	}
				//	else
				//	{
				//		ULONG u = pChannel->OpenDirectory(aFiles[0].strFileName);
				//		if (!u)
				//		{
				//			delete pData->pDirData;
				//			delete pData;
				//			::MessageBeep(MB_ICONEXCLAMATION);
				//			SetStatusText(MAKEINTRESOURCEW(IDS_DIRCHANGE_FAILED));
				//		}
				//		else
				//			m_listWaitForAttribute.Add(pData, u);
				//	}
				//	return;
				//}
					case CSFTPWaitAttrData::typeReadLink:
					{
						LPCWSTR lpFile = aFiles[0].strFileName;
						LPCWSTR lp = wcsrchr(lpFile, L'/');
						if (lp)
							lp++;
						else
							lp = lpFile;
						pData->strFileName.Empty();
						if (*lpFile != L'/')
						{
							pData->strFileName = pData->strRemoteDirectory;
							if (((LPCWSTR) pData->strFileName)[pData->strFileName.GetLength() - 1] != L'/')
								pData->strFileName += L'/';
						}
						pData->strFileName += lpFile;
						pData->uMsgID = m_pChannel->LStat(pData->strFileName);
						if (pData->uMsgID)
						{
							m_listWaitResponse.Add(pData, pData->uMsgID);
						}
						else
						{
							if (pData->pWaitDir)
							{
								pData->pWaitDir->dwReadLinkCount--;
								if (!pData->pWaitDir->dwReadLinkCount)
									DoNextReadDirectory(pData->pWaitDir);
							}
							delete pData;
						}
					}
					break;
				}
				//else if (!pData->pItem)
				//{
				//	LPCWSTR lpFile = aFiles[0].strFileName;
				//	LPCWSTR lp = wcsrchr(lpFile, L'/');
				//	if (lp)
				//		lp++;
				//	else
				//		lp = lpFile;
				//	if (m_wndListViewServer.FindFileItem(lp, &pData->pItem) < 0)
				//		pData->pItem = m_wndListViewServer.AddSFTPFileData(m_pChannel->GetServerVersion(), lp, &aFiles[0].attr);
				//	//else
				//	//	m_wndListViewServer.UpdateSFTPFileAttributes(m_pChannel->GetServerVersion(), pData->pItem, &aFiles[0].attr);
				//}
				////else
				////	m_wndListViewServer.UpdateSFTPFileAttributes(m_pChannel->GetServerVersion(), pData->pItem, &aFiles[0].attr);
				////delete pData;
				//uMsgID = m_pChannel->Stat(pData->nType == CSFTPWaitAttrData::typeReadLink ? pData->strFileName : aFiles[0].strFileName);
				//if (uMsgID != 0)
				//	m_listWaitForAttribute.Add(pData, uMsgID);
				//else
				//	delete pData;
			}
			break;
			case CWaitResponseData::WRD_DIRECTORY:
			{
				CSFTPWaitDirectoryData* pData = (CSFTPWaitDirectoryData*) p;

				::EnterCriticalSection(&pData->pDirectory->m_csFiles);
				_ParseSFTPFileName(pData->pDirectory, aFiles, nCount);
				m_listWaitResponse.Remove(uMsgID);

				for (int i = 0; i < pData->pDirectory->m_aFiles.GetCount(); i++)
				{
					CFTPFileItem* pItem = pData->pDirectory->m_aFiles.GetItem(i);
					if (pItem->IsShortcut())
					{
						CSFTPWaitAttrData* pAttr = new CSFTPWaitAttrData();
						pAttr->nType = CSFTPWaitAttrData::typeReadLink;
						pAttr->strRemoteDirectory = pData->pDirectory->m_strDirectory;
						pAttr->pLinkFrom = pItem;
						CMyStringW str(pData->pDirectory->m_strDirectory);
						if (str.IsEmpty() || ((LPCWSTR) str)[str.GetLength() - 1] != L'/')
							str += L'/';
						str += pItem->strFileName;
						pAttr->uMsgID = pChannel->ReadLink(str);
						if (!pAttr->uMsgID)
							delete pAttr;
						else
						{
							pAttr->pWaitDir = pData;
							pData->dwReadLinkCount++;
							m_listWaitResponse.Add(pAttr, pAttr->uMsgID);
						}
					}
				}
				if (!pData->dwReadLinkCount)
				{
					DoNextReadDirectory(pData);
				}

				::LeaveCriticalSection(&pData->pDirectory->m_csFiles);
			}
			break;
		}
	}
	//else if (pMsg->bSentMsg == SSH_FXP_REALPATH)
	//{
	//	//m_bInVerifyDirectory = false;
	//	m_strReceivingDirPath = aFiles[0].strFileName;
	//	m_uDirMsg = pChannel->OpenDirectory(m_strReceivingDirPath);
	//}
}

void CSFTPFolderSFTP::SFTPReceiveData(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
	const void* pvData, size_t nLen, const bool* pbEOF)
{
	if (pChannel != m_pChannel)
		return;
	ULONG uMsgID = pMsg->uMsgID;
	//if (m_listSFTPSendFile.IsItemKey(uMsgID))
	//{
	//	::EnterCriticalSection(&m_csListSendFile);
	//	_INDEX ind = m_listSFTPSendFile.PositionFromKey(uMsgID);
	//	CSFTPSendFileData* pData = m_listSFTPSendFile.GetItem(ind);
	//	m_listSFTPSendFile.Remove(ind);
	//	::LeaveCriticalSection(&m_csListSendFile);
	//	if (pData->bForSend)
	//		_EndSFTPSendFile(pData, S_OK, SSH_FX_BAD_MESSAGE, NULL);
	//	else
	//		_DoRecvSFTPFile2(pData, pvData, nLen);
	//}
}

void CSFTPFolderSFTP::SFTPReceiveAttributes(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
	const CSFTPFileAttribute& attrs)
{
	if (pChannel != m_pChannel)
		return;
	if (m_listWaitResponse.IsItemKey(pMsg->uMsgID))
	{
		CWaitResponseData* pWait = m_listWaitResponse.GetItem(pMsg->uMsgID);
		m_listWaitResponse.Remove(pWait->uMsgID);
		pWait->uMsgID = 0;
		switch (pWait->nWaitType)
		{
			case CWaitResponseData::WRD_FTPWAITATTR:
			{
				CSFTPWaitAttrData* pData = (CSFTPWaitAttrData*) pWait;
				pData->nResult = SSH_FX_OK;
				switch (pData->nType)
				{
					case CSFTPWaitAttrData::typeNormal:
						memcpy(&pData->fileData, &attrs, sizeof(CSFTPFileAttribute));
						if (attrs.dwMask & SSH_FILEXFER_ATTR_EXTENDED)
						{
							pData->fileData.aExAttrs = (CSFTPFileAttributeExtendedData*) malloc(sizeof(CSFTPFileAttributeExtendedData) * attrs.nExAttrCount);
							memcpy(pData->fileData.aExAttrs, attrs.aExAttrs, sizeof(CSFTPFileAttributeExtendedData) * attrs.nExAttrCount);
						}
						else
							pData->fileData.aExAttrs = NULL;
						break;
					case CSFTPWaitAttrData::typeReadLink:
					{
						if (pData->pWaitDir)
						{
							CSFTPWaitDirectoryData* pWaitDir = pData->pWaitDir;
							CSFTPFileData file;
							file.strFileName = pData->strFileName;
							memcpy(&file.attr, &attrs, sizeof(CSFTPFileAttribute));
							CFTPFileItem* pItem = ParseSFTPData(pChannel->GetServerVersion(), &file);
							pData->pLinkFrom->pTargetFile = pItem;
							if ((pItem->nUnixMode & S_IFLNK) == S_IFLNK)
							{
								pData->pLinkFrom = pItem;
								ULONG u = m_pChannel->ReadLink(pData->strFileName);
								if (u != 0)
								{
									pData->uMsgID = u;
									m_listWaitResponse.Add(pData, u);
								}
								else
									delete pData;
							}
							else
							{
								pWaitDir->dwReadLinkCount--;
								if (!pWaitDir->dwReadLinkCount)
								{
									DoNextReadDirectory(pWaitDir);
								}
								delete pData;
							}
						}
						else
							delete pData;
						//m_wndListViewServer.UpdateSFTPFileAttributes(m_pChannel->GetServerVersion(), pData->pItem, &attrs);
						//if ((pData->pItem->nUnixMode & S_IFLNK) == S_IFLNK)
						//{
						//	ULONG u = m_pChannel->ReadLink(pData->strFileName);
						//	if (u != 0)
						//		m_listWaitForAttribute.Add(pData, u);
						//	else
						//		delete pData;
						//}
						//else
						//{
						//	m_wndListViewServer.RefreshFileItem(pData->pLinkFrom);
						//	delete pData;
						//}
						//pData->uMsgID = 0;
					}
					break;
					default:
					{
						////if (!pData->pItem)
						////{
						////	memcpy(&pData->fileData.attr, &attrs, sizeof(CSFTPFileAttribute));
						////	m_wndListViewServer.AddSFTPFileData(m_pChannel->GetServerVersion(), pData->fileData);
						////}
						////else
						//	m_wndListViewServer.UpdateSFTPFileAttributes(m_pChannel->GetServerVersion(), pData->pItem, &attrs);
						//delete pData;
					}
					break;
				}
			}
			break;
		}
	}
}

void CSFTPFolderSFTP::SFTPReceiveStatVFS(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
	const struct sftp_statvfs& statvfs)
{
	if (pChannel != m_pChannel)
		return;
}
