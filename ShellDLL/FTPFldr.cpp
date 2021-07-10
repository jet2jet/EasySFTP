
#include "StdAfx.h"
#include "ShellDLL.h"
#include "FTPFldr.h"

static void __stdcall GetFTPStatusMessage(int nStatus, LPCWSTR lpszMessage, CMyStringW& rstrMessage)
{
	CMyStringW strMsg;
	rstrMessage.Format(IDS_COMMAND_FAILED, nStatus);
	if (lpszMessage && *lpszMessage)
		strMsg = lpszMessage;
	//else if (!strMsg.LoadString(((UINT) nStatus - SSH_FX_EOF) + IDS_SFTP_EOF))
	//	strMsg.LoadString(IDS_SFTP_UNKNOWN);

	if (!strMsg.IsEmpty())
	{
		rstrMessage += L" (";
		rstrMessage += strMsg;
		rstrMessage += L')';
	}
}

////////////////////////////////////////////////////////////////////////////////

CSFTPFolderFTPDirectory::CSFTPFolderFTPDirectory(
	CDelegateMallocData* pMallocData,
	CFTPDirectoryItem* pItemMe,
	CFTPDirectoryBase* pParent,
	CFTPDirectoryRootBase* pRoot,
	LPCWSTR lpszDirectory)
	: CFTPDirectoryBase(pMallocData, pItemMe, pParent, pRoot, lpszDirectory)
{
}

CSFTPFolderFTPDirectory::CSFTPFolderFTPDirectory(CDelegateMallocData* pMallocData, CFTPDirectoryItem* pItemMe)
	: CFTPDirectoryBase(pMallocData, pItemMe)
{
}

CSFTPFolderFTPDirectory::~CSFTPFolderFTPDirectory()
{
}

STDMETHODIMP CSFTPFolderFTPDirectory::CreateInstance(CFTPDirectoryItem* pItemMe, CFTPDirectoryBase* pParent, CFTPDirectoryRootBase* pRoot, LPCWSTR lpszDirectory, CFTPDirectoryBase** ppResult)
{
	CSFTPFolderFTPDirectory* p = new CSFTPFolderFTPDirectory(m_pMallocData, pItemMe, pParent, pRoot, lpszDirectory);
	if (!p)
		return E_OUTOFMEMORY;
	*ppResult = p;
	return S_OK;
}

STDMETHODIMP_(void) CSFTPFolderFTPDirectory::UpdateItem(CFTPFileItem* pOldItem, LPCWSTR lpszNewItem, LONG lEvent)
{
	CFTPDirectoryBase::UpdateItem(pOldItem, lpszNewItem, lEvent);

	//switch (lEvent)
	//{
	//	case SHCNE_CREATE:
	//	case SHCNE_MKDIR:
	//	{
	//		register CSFTPFolderSFTP* pRoot = (CSFTPFolderSFTP*) m_pRoot;
	//		CMyStringW strFile(m_strDirectory);
	//		if (((LPCWSTR) strFile)[strFile.GetLength() - 1] != L'/')
	//			strFile += L'/';
	//		strFile += lpszNewItem;
	//		ULONG uMsgID = pRoot->m_pChannel->LStat(strFile);
	//		if (!uMsgID)
	//		{
	//			break;
	//		}
	//		CSFTPWaitAttrData* pAttr = new CSFTPWaitAttrData();
	//		if (!pAttr)
	//		{
	//			break;
	//		}
	//		pAttr->uMsgID = uMsgID;
	//		pAttr->nType = CSFTPWaitAttrData::typeNormal;
	//		pRoot->m_listWaitResponse.Add(pAttr, uMsgID);

	//		while (pAttr->uMsgID)
	//		{
	//			if (pRoot->m_pClient->m_socket.CanReceive(WAIT_RECEIVE_TIME))
	//			{
	//				pRoot->DoReceiveSocket();
	//				if (!pRoot->m_pClient || !theApp.MyPumpMessage())
	//				{
	//					pAttr->nResult = SSH_FX_NO_CONNECTION;
	//					break;
	//				}
	//			}
	//			else
	//			{
	//				pAttr->nResult = SSH_FX_NO_CONNECTION;
	//				break;
	//			}
	//		}

	//		if (pAttr->nResult != SSH_FX_OK)
	//		{
	//			delete pAttr;
	//			break;
	//		}

	//		CSFTPFileData fileData;
	//		memcpy(&fileData.attr, &pAttr->fileData, sizeof(fileData.attr));
	//		fileData.strFileName = lpszNewItem;
	//		CFTPFileItem* pItem = ParseSFTPData(((CSFTPFolderSFTP*) m_pRoot)->m_pChannel->GetServerVersion(), &fileData);
	//		delete pAttr;

	//		::EnterCriticalSection(&m_csFiles);
	//		m_aFiles.Add(pItem);
	//		::LeaveCriticalSection(&m_csFiles);
	//	}
	//	break;
	//	case SHCNE_UPDATEITEM:
	//	case SHCNE_UPDATEDIR:
	//	{
	//		register CSFTPFolderSFTP* pRoot = (CSFTPFolderSFTP*) m_pRoot;
	//		CMyStringW strFile;
	//		if (((LPCWSTR) strFile)[strFile.GetLength() - 1] != L'/')
	//			strFile += L'/';
	//		strFile += pOldItem->strFileName;
	//		ULONG uMsgID = ((CSFTPFolderSFTP*) m_pRoot)->m_pChannel->LStat(strFile);
	//		if (!uMsgID)
	//		{
	//			break;
	//		}
	//		CSFTPWaitAttrData* pAttr = new CSFTPWaitAttrData();
	//		if (!pAttr)
	//		{
	//			break;
	//		}
	//		pAttr->uMsgID = uMsgID;
	//		pAttr->nType = CSFTPWaitAttrData::typeNormal;
	//		pRoot->m_listWaitResponse.Add(pAttr, uMsgID);

	//		while (pAttr->uMsgID)
	//		{
	//			if (pRoot->m_pClient->m_socket.CanReceive(WAIT_RECEIVE_TIME))
	//			{
	//				pRoot->DoReceiveSocket();
	//				if (!pRoot->m_pClient || !theApp.MyPumpMessage())
	//				{
	//					pAttr->nResult = SSH_FX_NO_CONNECTION;
	//					break;
	//				}
	//			}
	//			else
	//			{
	//				pAttr->nResult = SSH_FX_NO_CONNECTION;
	//				break;
	//			}
	//		}

	//		if (pAttr->nResult != SSH_FX_OK)
	//		{
	//			delete pAttr;
	//			break;
	//		}

	//		ParseSFTPAttributes(((CSFTPFolderSFTP*) m_pRoot)->m_pChannel->GetServerVersion(), pOldItem, &pAttr->fileData);
	//		delete pAttr;
	//	}
	//	break;
	//}
}

////////////////////////////////////////////////////////////////////////////////

CSFTPFolderFTP::CSFTPFolderFTP(CDelegateMallocData* pMallocData, CFTPDirectoryItem* pItemMe, CEasySFTPFolderRoot* pFolderRoot)
	: CFTPDirectoryRootBase(pMallocData, pItemMe)
	, m_pFolderRoot(pFolderRoot)
{
	//m_bMyUserInfo = false;
	//m_pUser = NULL;
	//m_pClient = NULL;
	//m_pChannel = NULL;
	//m_hWndTimer = NULL;
	//m_nPort = 22;
	m_pConnection = NULL;
	//::InitializeCriticalSection(&m_csSocket);
	m_dwTransferringCount = 0;
	m_nServerCharset = scsUTF8;
	m_pFolderRoot->AddRef();
}

CSFTPFolderFTP::~CSFTPFolderFTP()
{
	Disconnect();
	m_pFolderRoot->Release();
	//::DeleteCriticalSection(&m_csSocket);
}

bool CSFTPFolderFTP::Connect(HWND hWnd, LPCWSTR lpszHostName, int nPort, CUserInfo* pUser)
{
	m_hWndOwner = hWnd;

	if (pUser)
	{
		m_pUser = pUser;
		pUser->AddRef();
		//m_nServerCharset = scsUTF8;
	}
	else
	{
		m_pUser = new CUserInfo();
		if (!DoRetryAuthentication(true))
		{
			m_pUser->Release();
			m_pUser = NULL;
			return false;
		}
		//m_nServerCharset = scsUTF8;
	}

	if (m_pConnection)
		Disconnect();

	{
		CMyStringW str(lpszHostName);
		m_strHostName = str;
	}
	m_pConnection = new CFTPConnection();
	m_pConnection->m_socket.SetCharset((ServerCharset) m_nServerCharset);

	if (!m_pConnection->Connect(nPort, lpszHostName))
	{
		delete m_pConnection;
		m_pConnection = NULL;
		m_pUser->Release();
		m_pUser = NULL;
		return false;
	}
	//m_pSocket->AsyncSelect(m_hWnd, MY_WM_SOCKETMESSAGE, FD_READ | FD_CLOSE);
	m_bLoggingIn = true;
	m_bFirstReceive = false;
	m_nServerSystemType = SVT_UNKNOWN;
	m_nYearFollows = 0;
	m_bY2KProblem = false;

	m_idTimer = theApp.RegisterTimer(KEEP_CONNECTION_TIME_SPAN,
		(PFNEASYSFTPTIMERPROC) KeepConnectionTimerProc, (LPARAM) this);

	//SetStatusText(ID_STATUS_HOST, lpszHostName);

	if (!WaitForReceive(&m_bLoggingIn))
	{
		Disconnect();
		return false;
	}
	return true;
}

STDMETHODIMP CSFTPFolderFTP::Disconnect()
{
	if (!m_pConnection)
		return S_OK;

	m_pConnection->SendCommand(L"QUIT", NULL);
	m_pConnection->Close();
	//delete m_pConnection;
	m_pConnection->Release();
	m_pConnection = NULL;

	theApp.UnregisterTimer(m_idTimer);
	m_idTimer = 0;

	m_hWndOwner = NULL;

	return S_OK;
}

void CSFTPFolderFTP::IncrementTransferCount()
{
	::InterlockedIncrement((LONG*) &m_dwTransferringCount);
}

void CSFTPFolderFTP::DecrementTransferCount()
{
	::InterlockedDecrement((LONG*) &m_dwTransferringCount);
}

STDMETHODIMP CSFTPFolderFTP::GetFTPItemUIObjectOf(HWND hWndOwner, CFTPDirectoryBase* pDirectory,
	const CMyPtrArrayT<CFTPFileItem>& aItems, CFTPDataObject** ppObject)
{
	if (!m_pConnection)
		return OLE_E_NOTRUNNING;
	CFTPDataObject* pObject = new CFTPDataObject(//(CSFTPFolderSFTPDirectory*) pDirectory,
		m_pMallocData->pMalloc,
		pDirectory->m_pidlMe,
		m_strHostName,
		m_pConnection,
		this,
		pDirectory, aItems);
	if (!pObject)
		return E_OUTOFMEMORY;
	pObject->SetTextMode(m_bTextMode);
	*ppObject = pObject;
	return S_OK;
}

STDMETHODIMP CSFTPFolderFTP::SetFTPItemNameOf(HWND hWnd, CFTPDirectoryBase* pDirectory, CFTPFileItem* pItem, LPCWSTR pszName, SHGDNF uFlags)
{
	CMyStringW strFrom(pDirectory->m_strDirectory), strTo(pDirectory->m_strDirectory);
	size_t dw = strFrom.GetLength() - 1;
	if (((LPCWSTR) strFrom)[dw] != L'/')
		strFrom += L'/';
	if (((LPCWSTR) strTo)[dw] != L'/')
		strTo += L'/';
	strFrom += pItem->strFileName;
	strTo += pszName;

	CWaitRenameData* pWait = new CWaitRenameData();
	if (!pWait)
	{
		::MyMessageBoxW(hWnd, MAKEINTRESOURCEW(IDS_COMMAND_OUTOFMEMORY), NULL, MB_ICONHAND);
		return E_OUTOFMEMORY;
	}

	pWait->nCode = 0;
	pWait->bWaiting = true;
	if (!m_pConnection->SendDoubleCommand(L"RNFR", strFrom, L"RNTO", strTo,
		pWait, pWait))
	{
		delete pWait;
		::MyMessageBoxW(hWnd, MAKEINTRESOURCEW(IDS_COMMAND_OUTOFMEMORY), NULL, MB_ICONHAND);
		return E_OUTOFMEMORY;
	}
	if (!WaitForReceive(&pWait->bWaiting))
	{
		delete pWait;
		::MyMessageBoxW(hWnd, MAKEINTRESOURCEW(IDS_COMMAND_CONNECTION_ERROR), NULL, MB_ICONHAND);
		return E_FAIL;
	}
	if (pWait->nCode >= 400)
	{
		CMyStringW strMsg;
		GetFTPStatusMessage(pWait->nCode, pWait->strMsg, strMsg);
		delete pWait;
		::MyMessageBoxW(hWnd, strMsg, NULL, MB_ICONHAND);
		return E_FAIL;
	}
	delete pWait;
	return S_OK;
}

STDMETHODIMP CSFTPFolderFTP::DoDeleteFTPItems(HWND hWndOwner, CFTPDirectoryBase* pDirectory, const CMyPtrArrayT<CFTPFileItem>& aItems)
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

		{
			HRESULT hr2;
			CFTPWaitConfirm* pData = new CFTPWaitConfirm();
			if (!pData)
				hr2 = E_OUTOFMEMORY;
			else
			{
				pData->nCode = 0;
				pData->bWaiting = true;
				if (!m_pConnection)
					hr2 = OLE_E_NOTRUNNING;
				else if (!m_pConnection->SendCommand(L"DELE", strFile, pData))
					hr2 = E_UNEXPECTED;
				else if (!WaitForReceive(&pData->bWaiting))
					hr2 = E_UNEXPECTED;
				else
				{
					hr2 = (pData->nCode < 300 ? S_OK : E_FAIL);
				}

				if (SUCCEEDED(hr2))
					pDirectory->UpdateRemoveFile(pItem->strFileName, pItem->IsDirectory());
			}
			if (FAILED(hr2))
			{
				CMyStringW str, str2;
				if (hr2 == OLE_E_NOTRUNNING)
					str2.LoadString(IDS_COMMAND_CONNECTION_ERROR);
				else if (!pData)
					str2.LoadString(IDS_COMMAND_OUTOFMEMORY);
				else if (!pData->nCode)
					str2.LoadString(IDS_COMMAND_UNKNOWN_ERROR);
				else
					GetFTPStatusMessage(pData->nCode, pData->strMsg, str2);
				str = strFile;
				str += L": ";
				str += str2;
				astrMsgs.Add(str);
				if (SUCCEEDED(hr))
					hr = hr2;
			}
			if (pData)
				delete pData;
		}
	}
	if (FAILED(hr))
		theApp.MultipleErrorMsgBox(hWndOwner, astrMsgs);
	return hr;
}

STDMETHODIMP CSFTPFolderFTP::MoveFTPItems(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszFromDir, LPCWSTR lpszFileNames)
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

		CWaitRenameData* pData = NULL;
		HRESULT hr2;

		CFTPFileItem* p = RetrieveFileItem2(strFileFrom);
		if (!p)
		{
			hr2 = E_UNEXPECTED;
		}
		else
		{
			bool bIsDir = p->IsDirectory();
			p->Release();
			pData = new CWaitRenameData();
			if (!pData)
			{
				hr2 = E_OUTOFMEMORY;
			}
			else
			{
				pData->bWaiting = true;
				pData->bSecondary = false;
				pData->nCode = 0;
				if (!m_pConnection)
					hr2 = OLE_E_NOTRUNNING;
				else if (!m_pConnection->SendDoubleCommand(L"RNFR", strFileFrom, L"RNTO", strFileTo))
					hr2 = E_UNEXPECTED;
				else if (!WaitForReceive(&pData->bWaiting))
					hr2 = E_UNEXPECTED;
				else
				{
					hr2 = (pData->nCode < 300 && pData->nCode != 202 ? S_OK : E_FAIL);
				}

				if (SUCCEEDED(hr2))
					pDirectory->UpdateMoveFile(lpszFromDir, strFile, bIsDir);
			}
		}

		if (FAILED(hr2))
		{
			CMyStringW str, str2;
			if (hr2 == OLE_E_NOTRUNNING)
				str2.LoadString(IDS_COMMAND_CONNECTION_ERROR);
			else if (hr2 == E_UNEXPECTED)
				str2.LoadString(IDS_COMMAND_UNKNOWN_ERROR);
			else if (!pData)
				str2.LoadString(IDS_COMMAND_OUTOFMEMORY);
			else if (!pData->nCode)
				str2.LoadString(IDS_COMMAND_UNKNOWN_ERROR);
			else
				GetFTPStatusMessage(pData->nCode, pData->strMsg, str2);
			str = strFile;
			str += L": ";
			str += str2;
			astrMsgs.Add(str);
			if (SUCCEEDED(hr))
				hr = hr2;
		}
		if (pData)
			delete pData;
	}
	if (FAILED(hr))
		theApp.MultipleErrorMsgBox(hWndOwner, astrMsgs);
	return hr;
}

STDMETHODIMP CSFTPFolderFTP::UpdateFTPItemAttributes(HWND hWndOwner, CFTPDirectoryBase* pDirectory,
	CServerFilePropertyDialog* pDialog, const CMyPtrArrayT<CServerFileAttrData>& aAttrs, bool* pabResults)
{
	CMyStringArrayW astrMsgs;
	HRESULT hr = S_OK;
	CMyStringW strFile;
	for (int i = 0; i < aAttrs.GetCount(); i++)
	{
		CServerFileAttrData* pAttr = aAttrs.GetItem(i);

		strFile = pDirectory->m_strDirectory;
		if (((LPCWSTR) strFile)[strFile.GetLength() - 1] != L'/')
			strFile += L'/';
		strFile += pAttr->pItem->strFileName;

		HRESULT hr2;
		CFTPWaitConfirm* pData = new CFTPWaitConfirm();
		if (!pData)
		{
			hr2 = E_OUTOFMEMORY;
		}
		else
		{
			CMyStringW strMod;
			strMod.Format(L"%03o \"%s\"", pAttr->nUnixMode, (LPCWSTR) strFile);

			pData->bWaiting = true;
			pData->nCode = 0;
			if (!m_pConnection)
				hr2 = OLE_E_NOTRUNNING;
			else if (!m_pConnection->SendCommand(m_strChmodCommand, strMod, pData))
				hr2 = E_UNEXPECTED;
			else if (!WaitForReceive(&pData->bWaiting))
				hr2 = E_UNEXPECTED;
			else
			{
				hr2 = (pData->nCode < 300 && pData->nCode != 202 ? S_OK : E_FAIL);
			}

			if (SUCCEEDED(hr2))
				pDirectory->UpdateFileAttrs(pAttr->pItem->strFileName, pAttr->pItem->IsDirectory());
		}

		if (FAILED(hr2))
		{
			CMyStringW str, str2;
			if (hr2 == OLE_E_NOTRUNNING)
				str2.LoadString(IDS_COMMAND_CONNECTION_ERROR);
			else if (!pData)
				str2.LoadString(IDS_COMMAND_OUTOFMEMORY);
			else if (!pData->nCode)
				str2.LoadString(IDS_COMMAND_UNKNOWN_ERROR);
			else
				GetFTPStatusMessage(pData->nCode, pData->strMsg, str2);
			str = strFile;
			str += L": ";
			str += str2;
			astrMsgs.Add(str);
			if (SUCCEEDED(hr))
				hr = hr2;
		}
		if (pData)
			delete pData;
	}
	if (FAILED(hr))
		theApp.MultipleErrorMsgBox(hWndOwner, astrMsgs);
	return hr;
}

STDMETHODIMP CSFTPFolderFTP::CreateFTPDirectory(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszName)
{
	CMyStringW strFile(pDirectory->m_strDirectory);
	if (strFile.IsEmpty() || ((LPCWSTR) strFile)[strFile.GetLength() - 1] != L'/')
		strFile += L'/';
	strFile += lpszName;

	CFTPWaitConfirm* pWait = new CFTPWaitConfirm();
	if (!pWait)
	{
		::MyMessageBoxW(hWndOwner, MAKEINTRESOURCEW(IDS_COMMAND_OUTOFMEMORY), NULL, MB_ICONHAND);
		return E_OUTOFMEMORY;
	}

	pWait->nCode = 0;
	pWait->bWaiting = true;
	if (!m_pConnection->SendCommand(L"MKD", strFile, pWait))
	{
		delete pWait;
		::MyMessageBoxW(hWndOwner, MAKEINTRESOURCEW(IDS_COMMAND_OUTOFMEMORY), NULL, MB_ICONHAND);
		return E_OUTOFMEMORY;
	}
	if (!WaitForReceive(&pWait->bWaiting))
	{
		delete pWait;
		::MyMessageBoxW(hWndOwner, MAKEINTRESOURCEW(IDS_COMMAND_CONNECTION_ERROR), NULL, MB_ICONHAND);
		return E_FAIL;
	}
	if (pWait->nCode >= 400)
	{
		CMyStringW strMsg;
		GetFTPStatusMessage(pWait->nCode, pWait->strMsg, strMsg);
		delete pWait;
		::MyMessageBoxW(hWndOwner, strMsg, NULL, MB_ICONHAND);
		return E_FAIL;
	}
	delete pWait;
	return S_OK;
}

STDMETHODIMP CSFTPFolderFTP::CreateFTPItemStream(CFTPDirectoryBase* pDirectory, CFTPFileItem* pItem, IStream** ppStream)
{
	CMyStringW strFileName = pDirectory->m_strDirectory;
	if (strFileName.IsEmpty() ||
		((LPCWSTR) strFileName)[strFileName.GetLength() - 1] != L'/')
		strFileName += L'/';
	strFileName += pItem->strFileName;

	CFTPStream* pStream = new CFTPStream(strFileName, pItem->uliSize, this);
	if (!pStream)
		return E_OUTOFMEMORY;

	//CFTPFileRecvMessage* pMessage = new CFTPFileRecvMessage(strFileName);
	//CFTPWaitEstablishPassive* pEstablishWait = StartPassive(pMessage);
	//pMessage->Release();
	//if (!pEstablishWait)
	//	return E_FAIL;
	//if (!WaitForReceive(&pEstablishWait->bWaiting))
	//{
	//	delete pEstablishWait;
	//	return E_FAIL;
	//}
	//CFTPWaitPassive* pPassive = pEstablishWait->pRet;
	//delete pEstablishWait;
	//if (!WaitForReceive(&pPassive->bWaiting))
	//{
	//	delete pPassive;
	//	return E_FAIL;
	//}
	//CFTPStream* pStream = new CFTPStream(pItem->uliSize, pPassive->pPassive, m_pConnection);
	//if (!pStream)
	//{
	//	m_pConnection->ClosePassiveSocket(pPassive->pPassive);
	//	delete pPassive;
	//	return E_OUTOFMEMORY;
	//}
	//pPassive->pPassive = NULL;
	//delete pPassive;
	*ppStream = pStream;

	return S_OK;
}

STDMETHODIMP CSFTPFolderFTP::WriteFTPItem(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszName, IStream* pStream,
	void* pvObject, CTransferStatus* pStatus)
{
	HRESULT hr = S_OK;
	CMyStringW strFile, strMsg;

	strFile = pDirectory->m_strDirectory;
	if (((LPCWSTR) strFile)[strFile.GetLength() - 1] != L'/')
		strFile += L'/';
	strFile += lpszName;

	IncrementTransferCount();

	CFTPFileSendMessage* pMessage = new CFTPFileSendMessage(pStream, strFile);
	if (!pMessage)
	{
		hr = E_OUTOFMEMORY;
		CMyStringW str;
		str.LoadString(IDS_COMMAND_OUTOFMEMORY);
		strMsg = strFile;
		strMsg += L": ";
		strMsg += str;
	}
	else
	{
		CFTPWaitEstablishPassive* pEstablishWait = StartPassive(pMessage);
		if (!pEstablishWait)
		{
			pMessage->Release();
			hr = E_OUTOFMEMORY;
			CMyStringW str;
			str.LoadString(IDS_COMMAND_OUTOFMEMORY);
			strMsg = strFile;
			strMsg += L": ";
			strMsg += str;
		}
		else if (!WaitForReceive(&pEstablishWait->bWaiting))
		{
			delete pEstablishWait;
			pMessage->Release();
			hr = E_FAIL;
			CMyStringW str;
			str.LoadString(IDS_COMMAND_FAILED);
			strMsg = strFile;
			strMsg += L": ";
			strMsg += str;
		}
		else
		{
			CFTPWaitPassive* pPassive = pEstablishWait->pRet;
			delete pEstablishWait;
			if (!WaitForReceive(&pPassive->bWaiting))
			{
				delete pPassive;
				pMessage->Release();
				hr = E_FAIL;
				CMyStringW str;
				str.LoadString(IDS_COMMAND_FAILED);
				strMsg = strFile;
				strMsg += L": ";
				strMsg += str;
			}
			else
			{
				while (true)
				{
					if (pPassive->pPassive->IsRemoteClosed())
						break;
					if (!pMessage->ReadyToWrite(pPassive->pPassive))
						break;
					if (!m_pConnection || !theApp.MyPumpMessage2())
					{
						pMessage->m_bCanceled = true;
						break;
					}
					pStatus->TransferInProgress(pvObject, pMessage->m_uliOffset);
				}
				CFTPWaitConfirm* pConfirm = new CFTPWaitConfirm();
				pConfirm->bWaiting = true;
				m_pConnection->ClosePassiveSocket(pPassive->pPassive, pConfirm);
				delete pPassive;
				if (!WaitForReceive(&pConfirm->bWaiting))
				{
					hr = E_FAIL;
					CMyStringW str;
					str.LoadString(IDS_COMMAND_FAILED);
					strMsg = strFile;
					strMsg += L": ";
					strMsg += str;
				}
				else if (pConfirm->nCode >= 300)
				{
					hr = E_FAIL;
					strMsg = strFile;
					strMsg += L": ";
					strMsg += pConfirm->strMsg;
				}
				else if (!pMessage->m_bFinished || pMessage->m_bCanceled)
				{
					CMyStringW str;
					hr = E_FAIL;
					strMsg = strFile;
					strMsg += L": ";
					str.LoadString(IDS_COMMAND_UNKNOWN_ERROR);
					strMsg += str;
				}
				else
					pDirectory->UpdateNewFile(lpszName, false);
				delete pConfirm;
				pMessage->Release();
			}
		}
	}

	DecrementTransferCount();
	if (FAILED(hr) && !strMsg.IsEmpty())
		::MyMessageBoxW(hWndOwner, strMsg, NULL, MB_ICONHAND);
	return hr;
}

STDMETHODIMP CSFTPFolderFTP::CreateInstance(CFTPDirectoryItem* pItemMe, CFTPDirectoryBase* pParent, CFTPDirectoryRootBase* pRoot, LPCWSTR lpszDirectory, CFTPDirectoryBase** ppResult)
{
	CSFTPFolderFTPDirectory* p = new CSFTPFolderFTPDirectory(m_pMallocData, pItemMe, pParent, pRoot, lpszDirectory);
	if (!p)
		return E_OUTOFMEMORY;
	*ppResult = p;
	return S_OK;
}

void CSFTPFolderFTP::PreShowPropertyDialog(CServerFilePropertyDialog* pDialog)
{
	pDialog->m_bChangeOwner = false;
	pDialog->m_bChangeAttr = (m_nServerSystemType == SVT_UNIX);
	pDialog->m_bSupportedName = false;
}

void CSFTPFolderFTP::PreShowServerInfoDialog(CServerInfoDialog* pDialog)
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
		const sockaddr* psa = m_pConnection->m_socket.GetConnectedAddress(&nLen);
		getnameinfo(psa, (socklen_t) nLen, str.GetBufferA(NI_MAXHOST + 1), NI_MAXHOST + 1, NULL, 0, NI_NUMERICHOST);
		str.ReleaseBufferA();
		pDialog->m_strInfo += str;
	}

	pDialog->m_strInfo += L"\r\n";
	str.LoadString(IDS_SERVER_NAME);
	pDialog->m_strInfo += str;
	pDialog->m_strInfo += L": ";
	pDialog->m_strInfo += m_strServerInfo;
	pDialog->m_strInfo += L"\r\n";
	str.LoadString(IDS_FTP_WELCOME);
	pDialog->m_strInfo += str;
	pDialog->m_strInfo += L':';
	pDialog->m_strInfo += L"\r\n";
	pDialog->m_strInfo += m_strWelcomeMessage;
}

bool CSFTPFolderFTP::ReceiveDirectory(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszDirectory, bool* pbReceived)
{
	//::EnterCriticalSection(&m_csSocket);
	if (!m_pConnection)
	{
		if (!Connect(hWndOwner, m_strHostName, m_nPort, NULL))
		{
			//::LeaveCriticalSection(&m_csSocket);
			return false;
		}
	}
	if (*pbReceived)
	{
		//::LeaveCriticalSection(&m_csSocket);
		return true;
	}

	*pbReceived = true;
	CFTPFileListingHandler* pHandler = new CFTPFileListingHandler(this, pDirectory);
	if (!pHandler)
	{
		//::LeaveCriticalSection(&m_csSocket);
		return false;
	}
	CFTPPassiveMessage* pMessage;
	if (m_pConnection->IsCommandAvailable(L"MLST"))
		pMessage = new CFTPFileMListingMessage(pHandler, lpszDirectory);
	else
		pMessage = new CFTPFileListingMessage(pHandler, lpszDirectory);
	CFTPWaitEstablishPassive* pEstablishWait = StartPassive(pMessage);
	pMessage->Release();

	// waiting for being established passive connection
	if (!WaitForReceive(&pEstablishWait->bWaiting) || !pEstablishWait->pRet)
	{
		delete pEstablishWait;
		delete pHandler;
		//::LeaveCriticalSection(&m_csSocket);
		return false;
	}
	CFTPWaitPassive* pPassiveWait = pEstablishWait->pRet;
	delete pEstablishWait;

	pPassiveWait->bWaiting = true;
	if (!WaitForReceive(&pPassiveWait->bWaiting, NULL))
	{
		delete pPassiveWait;
		delete pHandler;
		//::LeaveCriticalSection(&m_csSocket);
		return false;
	}

	pPassiveWait->bWaiting = true;
	if (!WaitForReceive(&pPassiveWait->bWaiting, pPassiveWait))
	{
		delete pPassiveWait;
		delete pHandler;
		//::LeaveCriticalSection(&m_csSocket);
		return false;
	}

	if (pPassiveWait->nWaitFlags == CFTPWaitPassive::flagError)
	{
		delete pPassiveWait;
		delete pHandler;
		return false;
	}

	// wait for 226
	if (!WaitForReceive(&pPassiveWait->bWaiting, NULL))
	{
		delete pPassiveWait;
		delete pHandler;
		//::LeaveCriticalSection(&m_csSocket);
		return false;
	}

	delete pPassiveWait;
	delete pHandler;
	return true;
}

bool CSFTPFolderFTP::ValidateDirectory(LPCWSTR lpszParentDirectory, PCUIDLIST_RELATIVE pidlChild, CMyStringW& rstrRealPath)
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
	if (m_pConnection)
	{
		CWaitMakeDirData* pData = new CWaitMakeDirData();
		pData->nWaitFlags = CWaitMakeDirData::flagWaitingForRealPath1;
		pData->strRemoteDirectory = strPath;
		pData->bRPError = false;
		m_pConnection->SendDoubleCommand(L"CWD", strPath, L"PWD", NULL,
			pData, pData);
		if (!WaitForReceive(&pData->bWaiting))
		{
			delete pData;
			return false;
		}

		rstrRealPath = pData->strRemoteDirectory;
		delete pData;
	}
	else
#endif
		rstrRealPath = strPath;
	return true;
}

CFTPFileItem* CSFTPFolderFTP::RetrieveFileItem(CFTPDirectoryBase* pDirectory, LPCWSTR lpszFileName)
{
	CMyStringW strFileName(pDirectory->m_strDirectory);
	if (strFileName.IsEmpty() ||
		((LPCWSTR) strFileName)[strFileName.GetLength() - 1] != L'/')
		strFileName += L'/';
	strFileName += lpszFileName;
	return RetrieveFileItem2(strFileName);
}

CFTPFileItem* CSFTPFolderFTP::RetrieveFileItem2(LPCWSTR lpszFullPathName)
{
	bool bMLST = false;
	CFTPWaitAttrData* pData = new CFTPWaitAttrData();
	pData->strFileName = lpszFullPathName;
	pData->bWaiting = true;
	if (m_pConnection->IsCommandAvailable(L"MLST"))
	{
		bMLST = true;
		if (!m_pConnection->SendCommand(L"MLST", pData->strFileName, pData))
		{
			delete pData;
			return NULL;
		}
	}
	else
	{
		//if (pData->bIsDir)
		//	str += L"/?";
		if (!m_pConnection->SendCommand(L"STAT", pData->strFileName, pData))
		{
			delete pData;
			return NULL;
		}
	}
	if (!WaitForReceive(&pData->bWaiting))
	{
		delete pData;
		return NULL;
	}
	CFTPFileItem* pItem;
	if (bMLST)
		pItem = ParseMLSxData(pData->strResult);
	else
	{
		switch (m_nServerSystemType)
		{
			case SVT_UNKNOWN:
				pItem = ParseUnixFileList(pData->strResult);
				if (!pItem)
				{
					pItem = ParseDOSFileList(pData->strResult, &m_nYearFollows, &m_bY2KProblem);
				}
				break;
			case SVT_UNIX:
				pItem = ParseUnixFileList(pData->strResult);
				break;
			case SVT_DOS:
			case SVT_WINDOWS:
				pItem = ParseDOSFileList(pData->strResult, &m_nYearFollows, &m_bY2KProblem);
				break;
			default:
				pItem = NULL;
		}
	}
	delete pData;

	return pItem;
}

////////////////////////////////////////////////////////////////////////////////

//void CSFTPFolderFTP::SendCommand(LPCWSTR lpszType, LPCWSTR lpszParam)
//{
//	if (!m_pSocket)
//		return;
//	CMyStringW str(lpszType);
//	if (lpszParam)
//	{
//		str += L' ';
//		str += lpszParam;
//	}
//	m_pSocket->SendCommand(str);
//#ifdef _DEBUG
//	{
//		str.InsertString(L"> ", 0);
//		str += L'\n';
//		OutputDebugString(str);
//	}
//#endif
//}
//
//void CSFTPFolderFTP::SecureSendCommand(LPCWSTR lpszType, const _SecureStringW& strParam)
//{
//	if (!m_pSocket)
//		return;
//	_SecureStringW str(lpszType);
//	if (!strParam.IsEmpty())
//	{
//		str += L' ';
//		str += strParam;
//	}
//	m_pSocket->SecureSendCommand(str);
//#ifdef _DEBUG
//	{
//		CMyStringW str2(L"> ");
//		str2 += lpszType;
//		str2 += L" ****\n";
//		OutputDebugString(str2);
//	}
//#endif
//}
//
//bool CSFTPFolderFTP::ReceiveMessage(CMyStringW& rstrMessage, int* pnCode)
//{
//	if (!m_pSocket)
//		return false;
//	if (!m_pSocket->ReceiveMessage(rstrMessage, pnCode))
//		return false;
//	OutputDebugFTPMessage(*pnCode, rstrMessage);
//	//if (*pnCode != 200 && *pnCode != 226)
//	//{
//	//	CMyStringW str;
//	//	str.Format(L"%d %s", *pnCode, (LPCWSTR) rstrMessage);
//	//	SetStatusText(str);
//	//}
//	return true;
//}

void CSFTPFolderFTP::ShowFTPErrorMessage(int code, LPCWSTR lpszMessage)
{
	;
}

void CALLBACK CSFTPFolderFTP::KeepConnectionTimerProc(UINT_PTR idEvent, LPARAM lParam)
{
	CSFTPFolderFTP* pThis = (CSFTPFolderFTP*) lParam;

	// send 'ignore'/no-op message to keep connection
	if (pThis->m_pConnection)
		pThis->m_pConnection->SendCommand(L"NOOP", NULL);
}

void CSFTPFolderFTP::OnFTPSocketReceive()
{
	//::EnterCriticalSection(&m_csSocket);
	_OnFTPSocketReceiveThreadUnsafe();
	//::LeaveCriticalSection(&m_csSocket);
}

// in FileList.cpp
extern "C" bool __stdcall _ParseToFileTime(LPCWSTR lpszString, LPCWSTR lpszStop, FILETIME* pftTime);

void CSFTPFolderFTP::_OnFTPSocketReceiveThreadUnsafe()
{
	int code;
	CMyStringW str, strCommand;
	CWaitResponseData* pWait;
	if (!m_pConnection->ReceiveMessage(code, str, &pWait, &strCommand))
	{
		Disconnect();
		return;
	}

	if (strCommand.Compare(L"NOOP", true) == 0)
		return;

	//m_bFirstReceive = true;
	if (m_bLoggingIn)
	{
		switch (code)
		{
			// response for NOOP, etc.
			case 200:
				break;
			// response for FEAT
			case 211:
				if (pWait && pWait->nWaitType == CWaitResponseData::WRD_GETFEATURE)
				{
					m_bLoggingIn = false;
					m_pConnection->InitAvaliableCommands(str);
				}
				break;
			// initial message
			case 220:
				m_strServerInfo = str;
				m_pConnection->SendCommand(L"USER", m_pUser->strName);
				//SetStatusText(MAKEINTRESOURCEW(IDS_AUTHENTICATING));
				break;
			// response for SYST
			case 215:
				ConvertToLowerLenW(str.GetBuffer(), (SIZE_T) str.GetLength());
				if (wcsstr(str, L"unix"))
					m_nServerSystemType = SVT_UNIX;
				else if (wcsstr(str, L"dos"))
					m_nServerSystemType = SVT_DOS;
				else if (wcsstr(str, L"windows"))
					m_nServerSystemType = SVT_WINDOWS;
				//else
				//	m_nServerSystemType = SVT_UNKNOWN;
				m_pConnection->SendCommand(L"FEAT", NULL,
					new CWaitFeatureData());
				////UpdateServerFolderAbsolute(L"/");
				//UpdateServerFolderAbsolute(L".");
				break;
			// response for USER
			case 331:
				m_pConnection->SecureSendCommand(L"PASS", m_pUser->strPassword);
				break;
			case 230:
				//SetStatusText(MAKEINTRESOURCEW(IDS_CONNECTED));
				{
					LPWSTR lpw = str.GetBuffer();
					LPWSTR lpw2R = wcsrchr(lpw, L'\r');
					LPWSTR lpw2L = wcsrchr(lpw, L'\n');
					if (lpw2R || lpw2L)
					{
						if (lpw2R)
							*lpw2R = 0;
						else
							*lpw2L = 0;
						str.ReleaseBuffer();
					}
					else
						str.Empty();
					m_strWelcomeMessage = str;
				}
				m_pConnection->SendCommand(L"SYST", NULL);
				m_pUser->Release();
				m_pUser = NULL;
				break;
			case 530:
				switch (DoRetryAuthentication(false))
				{
					case 1:
						m_pConnection->SendCommand(L"USER", m_pUser->strName);
						break;
					case 0:
						Disconnect();
						break;
					case -1:
						Disconnect();
						::MyMessageBoxW(m_hWndOwner, MAKEINTRESOURCEW(IDS_FAILED_TO_CONNECT), NULL, MB_ICONEXCLAMATION);
						break;
				}
				break;
			case 500:
			default:
				if (pWait && pWait->nWaitType == CWaitResponseData::WRD_GETFEATURE)
				{
					m_bLoggingIn = false;
					m_pConnection->InitAvaliableCommands(NULL);
					delete ((CWaitFeatureData*) pWait);
				}
				else
					Disconnect();
				break;
		}
		if (pWait)
			delete pWait;
		return;
	}

	switch (code)
	{
		case 125:
		case 150:
		{
			if (pWait)
			{
				CFTPWaitPassive* pw = (CFTPWaitPassive*) pWait;
				if (pw->nWaitFlags == CFTPWaitPassive::flagError)
				{
					m_pConnection->ClosePassiveSocket(pw->pPassive);
					delete pw->pPassive;
					pw->pPassive = NULL;
				}
				else
				{
					CFTPPassiveMessage* pMessage = pw->pMessage;
					CTextSocket* pPassive = pw->pPassive;
					if (pMessage->ConnectionEstablished(pPassive))
					{
						//m_aWaitPassives.Add(pw);
						pw->nWaitFlags = CFTPWaitPassive::flagWaitingForPassiveDone;
					}
					else
					{
						pw->nWaitFlags = CFTPWaitPassive::flagError;
					}
				}
				pw->bWaiting = false;
			}
		}
		break;
		// response for NOOP, SITE, etc.
		case 200:
		{
			if (pWait)
			{
				switch (pWait->nWaitType)
				{
					//case CWaitResponseData::WRD_MAKEDIR:
					//{
					//	CWaitMakeDirData* pData = (CWaitMakeDirData*) pWait;
					//	switch (pData->nWaitFlags)
					//	{
					//		case CWaitMakeDirData::flagWaitingForFileListings:
					//		{
					//			m_pConnection->SendCommand(L"PWD", NULL, pWait);
					//			//m_aWaitResponse.Enqueue(pWait);
					//		}
					//		break;
					//		case CWaitMakeDirData::flagWaitingForRealPath:
					//		{
					//			m_pConnection->SendCommand(L"PWD", NULL, pWait);
					//			//m_aWaitResponse.Enqueue(pWait);
					//		}
					//		break;
					//	}
					//}
					//break;
					//case CWaitResponseData::WRD_GETFILEINFO:
					//{
					//	CWaitFileInfoData* pData = (CWaitFileInfoData*) pWait;
					//	if (pData->nInfoType == CWaitFileInfoData::fileInfoSize)
					//		m_pConnection->SendCommand(L"SIZE", pData->strFileName, pData);
					//	else
					//	{
					//		pData->bSucceeded = false;
					//		pData->bWaiting = false;
					//	}
					//}
					//break;
					case CWaitResponseData::WRD_PASSIVEMSG:
						// usually using SendCommandWithType; ignore it
						break;
					case CWaitResponseData::WRD_CONFIRM:
					{
						CFTPWaitConfirm* pData = (CFTPWaitConfirm*) pWait;
						pData->nCode = code;
						pData->strMsg = str;
						pData->bWaiting = false;
					}
					break;
				}
			}
		}
		break;
		// 'Command not implemented, superfluous at this site'
		case 202:
		{
			if (pWait)
			{
				switch (pWait->nWaitType)
				{
					case CWaitResponseData::WRD_CONFIRM:
					{
						CFTPWaitConfirm* pData = (CFTPWaitConfirm*) pWait;
						pData->nCode = code;
						pData->strMsg = str;
						pData->bWaiting = false;
					}
					break;
				}
			}
		}
		break;
		case 211:
		case 212:
		case 213:
		{
			// response for STAT, SIZE, MDTM, etc.
			if (pWait)
			{
				switch (pWait->nWaitType)
				{
					case CWaitResponseData::WRD_FTPWAITATTR:
					{
						CFTPWaitAttrData* pData = (CFTPWaitAttrData*) pWait;
						LPCWSTR lpLine = str;
						LPCWSTR lp = wcsrchr(lpLine, L'\n');
						if (lp)
							str.ReleaseBuffer((DWORD) ((DWORD_PTR) lp - (DWORD_PTR) lpLine));
						lpLine = str;
						lp = wcschr(lpLine, L'\n');
						if (lp)
							str.DeleteString(0, (DWORD) ((DWORD_PTR) lp - (DWORD_PTR) lpLine));

						//AddFileDataIfNeed(pData->bIsDir ? L"." : pData->strFileName, str);
						//delete pData;
						pData->strResult = str;
						pData->bWaiting = false;
					}
					break;
					case CWaitResponseData::WRD_GETFILEINFO:
					{
						CWaitFileInfoData* pData = (CWaitFileInfoData*) pWait;
						switch (pData->nInfoType)
						{
							case CWaitFileInfoData::fileInfoSize:
							{
								LPCWSTR lp = NULL;
								pData->uliSize.QuadPart = _wcstoi64(str, (wchar_t**) &lp, 10);
								pData->bSucceeded = (lp && !*lp);
								pData->bWaiting = false;
							}
							break;
							case CWaitFileInfoData::fileInfoMDTM:
								pData->bSucceeded = _ParseToFileTime(str, NULL, &pData->ftModifiedTime);
								pData->bWaiting = false;
								break;
						}
					}
					break;
					case CWaitResponseData::WRD_CONFIRM:
					{
						CFTPWaitConfirm* pData = (CFTPWaitConfirm*) pWait;
						pData->nCode = code;
						pData->strMsg = str;
						pData->bWaiting = false;
					}
					break;
				}
			}
		}
		break;
		case 226:
		case 451:   // transfer aborted
		{
			// response for finishing passive
			if (pWait)
			{
				if (pWait->nWaitType == CWaitResponseData::WRD_PASSIVEMSG)
				{
					CFTPWaitPassive* pData = (CFTPWaitPassive*) pWait;
					pData->bWaiting = false;
					pData->nWaitFlags = (code >= 400 ? CFTPWaitPassive::flagError : CFTPWaitPassive::flagFinished);
				}
				else if (pWait->nWaitType == CWaitResponseData::WRD_CONFIRM)
				{
					CFTPWaitConfirm* pData = (CFTPWaitConfirm*) pWait;
					pData->bWaiting = false;
					pData->nCode = code;
					pData->strMsg = str;
				}
			}
		}
		break;
		case 227:
		{
			if (pWait->nWaitType == CWaitResponseData::WRD_PASSIVE)
			{
				CTextSocket* pRet = NULL;
				CMyStringW strIP, strTemp;
				int port;
				LPCWSTR lp, lp2, lp3;
				register int iTemp;
				// "227 Entering Passive Mode (n1,n2,n3,n4,p1,p2)"
				lp = wcschr(str, L'(');
				if (lp)
				{
					lp++;
					lp2 = wcschr(lp, L')');
					if (lp2)
					{
						bool bSucceeded = true;
						str.SetString(lp, ((DWORD) ((LPCBYTE) lp2 - (LPCBYTE) lp)) / sizeof(WCHAR));
						lp2 = lp = str;
						code = 6;
						port = 0;
						while (true)
						{
							lp2 = wcschr(lp, L',');
							if (code > 1)
							{
								if (!lp2 || lp == lp2)
									break;
							}
							iTemp = (int) wcstol(lp, (wchar_t**) &lp3, 10);
							if ((code == 1 && *lp3) || (code > 1 && lp == lp3))
							{
								//return NULL;
								bSucceeded = false;
								break;
							}
							if (code >= 3)
							{
								strTemp.Format(L"%d", iTemp);
								strIP += strTemp;
								if (code < 5)
									strIP += L'.';
							}
							else if (code == 2)
								port = iTemp;
							else
								port |= iTemp << 8;
							code--;
							if (!code)
							{
								if (lp2)
									bSucceeded = false;
								//	return NULL;
								break;
							}
							lp = lp2 + 1;
						}
						if (!lp)
						{
							pRet = new CTextSocket();
							if (!pRet->Connect(port, strIP, AF_INET, SOCK_STREAM))
							{
								delete pRet;
								pRet = NULL;
							}
						}
					}
				}

				CFTPWaitEstablishPassive* pData = (CFTPWaitEstablishPassive*) pWait;
				if (pRet)
				{
					pData->pRet = PassiveStarted(pData, pRet);
					if (!pData->pRet)
						delete pRet;
				}
				else
				{
					pData->bWaiting = false;
					pData->pRet = NULL;
				}
			}
		}
		break;
		case 229:
		{
			if (pWait->nWaitType == CWaitResponseData::WRD_PASSIVE)
			{
				CTextSocket* pRet = NULL;
				// "229 Entering Extended Passive Mode (|||<port>|)"
				// ('|' is a delimiter which may be replaced by any other chars)
				LPCWSTR lp = wcschr(str, L'(');
				if (lp)
				{
					lp++;
					LPCWSTR lp2 = wcschr(lp, L')');
					if (lp2)
					{
						str.SetString(lp, ((DWORD) ((LPCBYTE) lp2 - (LPCBYTE) lp)) / sizeof(WCHAR));
						lp = str;
						code = 5;
						WCHAR chDelimiter = *lp;
						LPCWSTR lp3;
						int port = 0;
						while (code)
						{
							lp2 = wcschr(lp, chDelimiter);
							if (!lp2)
							{
								//if (code > 1)
								//	code = -1;
								break;
							}
							if (code == 2)
							{
								if (lp == lp2)
									break;
								port = (int) wcstol(lp, (wchar_t**) &lp3, 10);
								if (lp3 != lp2)
									break;
							}
							else if (lp != lp2)
								break;
							code--;
							lp = lp2 + 1;
						}
						if (!*lp && port)
						{
							pRet = new CTextSocket();
							if (!pRet->Connect(m_pConnection->m_socket.GetThisAddrInfo(), port))
							{
								delete pRet;
								pRet = NULL;
							}
						}
					}
				}

				CFTPWaitEstablishPassive* pData = (CFTPWaitEstablishPassive*) pWait;
				if (pRet)
				{
					pData->pRet = PassiveStarted(pData, pRet);
					if (!pData->pRet)
						delete pRet;
				}
				else
				{
					pData->bWaiting = false;
					pData->pRet = NULL;
				}
			}
		}
		break;
		// response for file operations (succeeded)
		case 250:
			if (pWait)
			{
				switch (pWait->nWaitType)
				{
					case CWaitResponseData::WRD_FTPWAITATTR:
					{
						CFTPWaitAttrData* pData = (CFTPWaitAttrData*) pWait;
						CMyStringW strAttr;
						LPCWSTR lp = wcschr(str, L'\n');
						if (lp)
						{
							lp++;
							while (*lp == L' ')
								lp++;
							LPCWSTR lp2 = wcschr(lp, L'\n');
							if (lp2)
							{
								strAttr.SetString(lp, (DWORD) (((DWORD_PTR) lp2 - (DWORD_PTR) lp) / sizeof(WCHAR)));
								//m_wndListViewServer.AddMFileDataIfNeed(strAttr);
								pData->strResult = strAttr;
							}
						}
						//delete pData;
						pData->bWaiting = false;
						//SetStatusText(MAKEINTRESOURCEW(IDS_COMMAND_OK));
					}
					break;
					case CWaitResponseData::WRD_RENAME:
					{
						CWaitRenameData* pData = (CWaitRenameData*) pWait;
						pData->bWaiting = false;
						pData->nCode = code;
						pData->strMsg = str;
					}
					break;
					case CWaitResponseData::WRD_MAKEDIR:
					{
						CWaitMakeDirData* pData = (CWaitMakeDirData*) pWait;
						switch (pData->nWaitFlags)
						{
							case CWaitMakeDirData::flagWaitingForRealPath1:
								pData->nWaitFlags = CWaitMakeDirData::flagWaitingForRealPath2;
								//m_aWaitResponse.Enqueue(pData);
								break;
							default:
								//delete pData;
								pData->nWaitFlags = CWaitMakeDirData::flagError;
								pData->bWaiting = false;
								break;
						}
					}
					break;
					case CWaitResponseData::WRD_CONFIRM:
					{
						CFTPWaitConfirm* pData = (CFTPWaitConfirm*) pWait;
						pData->nCode = code;
						pData->strMsg = str;
						pData->bWaiting = false;
					}
					break;
				}
			}
			break;
		// resonse for PWD, CWD, and MKD
		case 257:
		{
			if (pWait)
			{
				switch (pWait->nWaitType)
				{
					case CWaitResponseData::WRD_MAKEDIR:
					{
						CWaitMakeDirData* pDir = (CWaitMakeDirData*) pWait;
						switch (pDir->nWaitFlags)
						{
							case CWaitMakeDirData::flagWaitingForRealPath2:
							{
								if (pDir->bRPError)
									pDir->nWaitFlags = CWaitMakeDirData::flagError;
								else
								{
									LPCWSTR lpw = str;
									LPCWSTR lpw2;
									while (*lpw == L' ')
										lpw++;
									if (*lpw == '\"')
									{
										lpw2 = ++lpw;
										while (*lpw2)
										{
											if (*lpw2 == L'\"')
												break;
											lpw2++;
										}
										pDir->strRemoteDirectory.SetString(lpw, ((DWORD) ((LPCBYTE) lpw2 - (LPCBYTE) lpw)) / sizeof(WCHAR));
									}
									pDir->nWaitFlags = CWaitMakeDirData::flagFinished;
								}
								pDir->bWaiting = false;
							}
							break;
							//case CWaitMakeDirData::flagWaitingForMKDSend:
							//	if (pDir->pSendFile)
							//		DoSendConfirm(pDir->pSendFile, true);
							//	else
							//		DoSendFolder(pDir->strLocalDirectory, pDir->strRemoteDirectory);
							//	break;
							//case CWaitMakeDirData::flagWaitingForMKDNoSend:
							//	if (pDir->pvData)
							//	{
							//		CFTPFileItem* pItem = m_wndListViewServer.EndCreateLabelEdit(&pDir->strRemoteDirectory);
							//		if (pItem)
							//		{
							//			// 改めてサーバーから属性情報を得る
							//			CFTPWaitAttrData* pAttr = new CFTPWaitAttrData();
							//			pAttr->bIsDir = (pItem->type == fitypeDir);
							//			pAttr->strFileName = pDir->strRemoteDirectory;
							//			if (m_pConnection->IsCommandAvailable(L"MLST"))
							//			{
							//				SendCommand(L"MLST", pDir->strRemoteDirectory);
							//			}
							//			else
							//			{
							//				//if (pData->bIsDir)
							//				//	str += L"/?";
							//				SendCommand(L"STAT", pDir->strRemoteDirectory);
							//			}
							//			m_aWaitResponse.Enqueue(pAttr);
							//		}
							//	}
							//	break;
						}
						//delete pDir;
						break;
					}
					break;
					case CWaitResponseData::WRD_CONFIRM:
					{
						CFTPWaitConfirm* pData = (CFTPWaitConfirm*) pWait;
						pData->nCode = code;
						pData->strMsg = str;
						pData->bWaiting = false;
					}
					break;
				}
			}
		}
		break;
		case 350:
		{
			if (pWait)
			{
				if (pWait->nWaitType == CWaitResponseData::WRD_RENAME)
				{
					CWaitRenameData* pData = (CWaitRenameData*) pWait;
					//if (pData->pItem)
					//	m_wndListViewServer.RefreshFileItem(pData->iItem);
					//delete pData;
					pData->bSecondary = true;
				}
				else if (pWait->nWaitType == CWaitResponseData::WRD_PASSIVEMSG)
				{
					// do nothing (response to REST)
				}
			}
		}
		break;
		case 450: // Requested file action not taken. File unavailable (e.g., file busy).
		{
			if (pWait)
			{
				switch (pWait->nWaitType)
				{
					case CWaitResponseData::WRD_MAKEDIR:
					{
						CWaitMakeDirData* pData = (CWaitMakeDirData*) pWait;
						switch (pData->nWaitFlags)
						{
							case CWaitMakeDirData::flagWaitingForRealPath1:
								pData->nWaitFlags = CWaitMakeDirData::flagWaitingForRealPath2;
								pData->bRPError = true;
								break;
							case CWaitMakeDirData::flagWaitingForRealPath2:
								pData->nWaitFlags = CWaitMakeDirData::flagError;
								pData->bRPError = true;
								pData->bWaiting = false;
								break;
						}
					}
					break;
					case CWaitResponseData::WRD_RENAME:
					{
						CWaitRenameData* pData = (CWaitRenameData*) pWait;
						//if (pData->pItem)
						//	m_wndListViewServer.RefreshFileItem(pData->iItem);
						//delete pData;
						if (pData->bSecondary)
							pData->bWaiting = false;
						if (!pData->nCode)
						{
							pData->nCode = code;
							pData->strMsg = str;
							pData->bSecondary = true;
						}
						pWait = NULL;
						str.Empty();
					}
					break;
					case CWaitResponseData::WRD_FTPWAITATTR:
					{
						CFTPWaitAttrData* pData = (CFTPWaitAttrData*) pWait;
						pData->strResult.Empty();
						//pData->bWaiting = false;
						str.Empty();
					}
					break;
					case CWaitResponseData::WRD_CONFIRM:
					{
						CFTPWaitConfirm* pData = (CFTPWaitConfirm*) pWait;
						pData->nCode = code;
						pData->strMsg = str;
						//pData->bWaiting = false;
						str.Empty();
					}
					break;
				}
				if (pWait)
					pWait->bWaiting = false;
			}
			if (!str.IsEmpty())
			{
				//::MessageBeep(MB_ICONEXCLAMATION);
				//SetStatusText(str);
				ShowFTPErrorMessage(code, str);
			}
		}
		break;

		case 421: // Service not available, closing control connection.
		default:
			if (code == 421 || code >= 500)
			{
				if (pWait)
				{
					switch (pWait->nWaitType)
					{
						case CWaitResponseData::WRD_PASSIVE:
						{
							CFTPWaitEstablishPassive* pData = (CFTPWaitEstablishPassive*) pWait;
							// 'EPSV' is not supported, so we use 'PASV'
							m_pConnection->SendCommand(L"PASV", NULL, pData);
						}
						break;
						case CWaitResponseData::WRD_PASSIVEMSG:
						{
							CFTPWaitPassive* pData = (CFTPWaitPassive*) pWait;
							pData->nWaitFlags = CFTPWaitPassive::flagError;
						}
						break;
						case CWaitResponseData::WRD_FTPWAITATTR:
						{
							CFTPWaitAttrData* pData = (CFTPWaitAttrData*) pWait;
							//delete pData;
							pData->strResult.Empty();
							pData->bWaiting = false;
							str.Empty();
						}
						break;
						case CWaitResponseData::WRD_RENAME:
						{
							CWaitRenameData* pData = (CWaitRenameData*) pWait;
							//if (pData->pItem)
							//	m_wndListViewServer.RefreshFileItem(pData->iItem);
							//delete pData;
							if (pData->bSecondary)
								pData->bWaiting = false;
							if (!pData->nCode)
							{
								pData->nCode = code;
								pData->strMsg = str;
								pData->bSecondary = true;
							}
							str.Empty();
						}
						break;
						case CWaitResponseData::WRD_MAKEDIR:
						{
							CWaitMakeDirData* pDir = (CWaitMakeDirData*) pWait;
							if (pDir->nWaitFlags == CWaitMakeDirData::flagWaitingForRealPath1)
							{
								pDir->nWaitFlags = CWaitMakeDirData::flagWaitingForRealPath2;
								pDir->bRPError = true;
								pWait = NULL;
							}
							else
								pDir->nWaitFlags = CWaitMakeDirData::flagError;
						}
						break;
						case CWaitResponseData::WRD_CONFIRM:
						{
							CFTPWaitConfirm* pData = (CFTPWaitConfirm*) pWait;
							pData->nCode = code;
							pData->strMsg = str;
							//pData->bWaiting = false;
							str.Empty();
						}
						break;
					}
					if (pWait)
						pWait->bWaiting = false;
				}
				if (!str.IsEmpty())
				{
					//::MessageBeep(MB_ICONEXCLAMATION);
					//SetStatusText(str);
					ShowFTPErrorMessage(code, str);
				}
			}
			break;
	}
}

void CSFTPFolderFTP::DoReceiveSocket()
{
	//while (true)
	{
		OnFTPSocketReceive();
		//if (!m_pSocket || !m_pSocket->CanReceive())
		//	break;
		::Sleep(0);
	}
}

CFTPWaitEstablishPassive* CSFTPFolderFTP::StartPassive(CFTPPassiveMessage* pMessage)
{
	CFTPWaitEstablishPassive* pWait = new CFTPWaitEstablishPassive(pMessage);
	pWait->bWaiting = true;
	if (!m_pConnection->SendCommand(L"EPSV", NULL, pWait))
	{
		delete pWait;
		return NULL;
	}
	//m_aWaitResponse.Add(pWait);
	return pWait;
}

CFTPWaitPassive* CSFTPFolderFTP::PassiveStarted(CFTPWaitEstablishPassive* pWait, CTextSocket* pSocket)
{
	CFTPPassiveMessage* pMessage = pWait->pMessage;
	CFTPWaitPassive* pData = new CFTPWaitPassive(
		CFTPWaitPassive::flagWaitingForEstablish,
		pMessage,
		pSocket);
	pData->bWaiting = true;
	pWait->bWaiting = false;
	if (pMessage->SendPassive(m_pConnection, pData))
	{
		//m_aDataSockets.Add(pPassive);
		return pData;
	}
	else
	{
		pMessage->Release();
		delete pData;
		return NULL;
	}
}

void CSFTPFolderFTP::DoReceivePassive(CFTPWaitPassive* pPassive)
{
	//while (true)
	{
		m_pConnection->ReceivePassive(pPassive);
		//if (!pPassive->pMessage->OnReceive(pPassive->pPassive))
		//{
		//	pPassive->bWaiting = false;
		//	pPassive->nWaitFlags = CFTPWaitPassive::flagError;
		//	pPassive->pMessage->EndReceive(NULL);
		//	//break;
		//	return;
		//}
		//if (pPassive->pPassive->IsRemoteClosed())
		//{
		//	pPassive->bWaiting = false;
		//	pPassive->nWaitFlags = CFTPWaitPassive::flagFinished;
		//	pPassive->pMessage->EndReceive(NULL);
		//	//break;
		//	return;
		//}
		//if (!pPassive->pPassive || !pPassive->pPassive->CanReceive())
		//	break;
		//::Sleep(0);
	}
}

bool CSFTPFolderFTP::WaitForReceive(bool* pbWaiting, CFTPWaitPassive* pPassive)
{
	while (*pbWaiting)
	{
		if (pPassive)
		{
			if (!pPassive->pPassive)
				return false;
			//if (pPassive->pPassive->CanReceive(WAIT_RECEIVE_TIME))
			{
				DoReceivePassive(pPassive);
				if (!pPassive || !theApp.MyPumpMessage())
					return false;
				if (pPassive->nWaitFlags == CFTPWaitPassive::flagFinished || pPassive->nWaitFlags == CFTPWaitPassive::flagError)
					break;
			}
		}
		else
		{
			if (m_pConnection)
			{
				if (m_pConnection->m_socket.IsRemoteClosed())
				{
					Disconnect();
					return false;
				}
				if (m_pConnection->m_socket.CanReceive(WAIT_RECEIVE_TIME))
				{
					DoReceiveSocket();
					if (!m_pConnection || !theApp.MyPumpMessage())
						return false;
				}
			}
			else
				return false;
		}
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////

CSFTPFolderFTP::CFTPFileListingHandler::CFTPFileListingHandler(CSFTPFolderFTP* pRoot, CFTPDirectoryBase* pDirectory)
	: m_pRoot(pRoot)
	, m_pDirectory(pDirectory)
{
	pRoot->AddRef();
	pDirectory->AddRef();
}

CSFTPFolderFTP::CFTPFileListingHandler::~CFTPFileListingHandler()
{
	m_pDirectory->Release();
	m_pRoot->Release();
}

bool CSFTPFolderFTP::CFTPFileListingHandler::ReceiveFileListing(CTextSocket* pPassive, bool bMListing)
{
	CFTPFileItem* pItem;
	CMyStringW str2;

	if (!pPassive->ReceiveLine(str2))
		return false;

	if (bMListing)
		pItem = ParseMLSxData(str2);
	else
	{
		switch (m_pRoot->m_nServerSystemType)
		{
			case SVT_UNKNOWN:
				pItem = ParseUnixFileList(str2);
				if (!pItem)
				{
					pItem = ParseDOSFileList(str2, &m_pRoot->m_nYearFollows, &m_pRoot->m_bY2KProblem);
					if (!pItem)
						return true;
				}
				break;
			case SVT_UNIX:
				pItem = ParseUnixFileList(str2);
				break;
			case SVT_DOS:
			case SVT_WINDOWS:
				pItem = ParseDOSFileList(str2, &m_pRoot->m_nYearFollows, &m_pRoot->m_bY2KProblem);
				break;
			default:
				pItem = NULL;
		}
	}

	if (!pItem)
		return true;
	else if (pItem->type == fitypeCurDir)
		pItem->type = fitypeDir;
	else if (/*pItem->type == fitypeCurDir || */pItem->type == fitypeParentDir ||
		pItem->strFileName.Compare(L".") == 0 || pItem->strFileName.Compare(L"..") == 0)
	{
		pItem->Release();
		return true;
	}

	::EnterCriticalSection(&m_pDirectory->m_csFiles);
	m_pDirectory->m_aFiles.Add(pItem);
	::LeaveCriticalSection(&m_pDirectory->m_csFiles);
	return true;
}

void CSFTPFolderFTP::CFTPFileListingHandler::FinishFileListing()
{
}
