
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

static void __stdcall FTPItemToStatStg(CFTPFileItem* pItem, DWORD grfStatFlag, DWORD grfMode, DWORD grfStateBits, STATSTG* pStatstg)
{
	if (!(grfStatFlag & STATFLAG_NONAME))
		pStatstg->pwcsName = DuplicateCoMemString(pItem->strFileName);
	pStatstg->cbSize = pItem->uliSize;
	pStatstg->type = pItem->IsDirectory() ? STGTY_STORAGE : STGTY_STREAM;
	pStatstg->mtime = pItem->ftModifyTime;
	pStatstg->ctime = pItem->ftCreateTime;
	pStatstg->atime = {};
	pStatstg->grfLocksSupported = 0;
	pStatstg->clsid = CLSID_NULL;
	pStatstg->grfMode = grfMode;
	pStatstg->grfStateBits = grfStateBits;
}

static void FormatMfctMfmtString(CMyStringW& str, LPCWSTR lpszFileName, const FILETIME* filetime)
{
	SYSTEMTIME st = {};
	::FileTimeToSystemTime(filetime, &st);
	str.Format(L"%04hu%02hu%02hu%02hu%02hu%02hu \"%s\"", st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond, lpszFileName);
}

////////////////////////////////////////////////////////////////////////////////

CSFTPFolderFTPDirectory::CSFTPFolderFTPDirectory(
	CDelegateMallocData* pMallocData,
	CFTPDirectoryItem* pItemMe,
	CFTPDirectoryBase* pParent,
	LPCWSTR lpszDirectory)
	: CFTPDirectory(pMallocData, pItemMe, pParent, lpszDirectory)
{
}

CSFTPFolderFTPDirectory::CSFTPFolderFTPDirectory(CDelegateMallocData* pMallocData, CFTPDirectoryItem* pItemMe, ITypeInfo* pInfo)
	: CFTPDirectory(pMallocData, pItemMe, pInfo)
{
}

CSFTPFolderFTPDirectory::~CSFTPFolderFTPDirectory()
{
}

STDMETHODIMP CSFTPFolderFTPDirectory::CreateInstance(CFTPDirectoryItem* pItemMe, CFTPDirectoryBase* pParent, LPCWSTR lpszDirectory, CFTPDirectoryBase** ppResult)
{
	CSFTPFolderFTPDirectory* p = new CSFTPFolderFTPDirectory(m_pMallocData, pItemMe, pParent, lpszDirectory);
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
	//		register CSFTPFolderFTP* pRoot = (CSFTPFolderFTP*) m_pRoot;
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
	//		CFTPFileItem* pItem = ParseSFTPData(((CSFTPFolderFTP*) m_pRoot)->m_pChannel->GetServerVersion(), &fileData);
	//		delete pAttr;

	//		::EnterCriticalSection(&m_csFiles);
	//		m_aFiles.Add(pItem);
	//		::LeaveCriticalSection(&m_csFiles);
	//	}
	//	break;
	//	case SHCNE_UPDATEITEM:
	//	case SHCNE_UPDATEDIR:
	//	{
	//		register CSFTPFolderFTP* pRoot = (CSFTPFolderFTP*) m_pRoot;
	//		CMyStringW strFile;
	//		if (((LPCWSTR) strFile)[strFile.GetLength() - 1] != L'/')
	//			strFile += L'/';
	//		strFile += pOldItem->strFileName;
	//		ULONG uMsgID = ((CSFTPFolderFTP*) m_pRoot)->m_pChannel->LStat(strFile);
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

	//		ParseSFTPAttributes(((CSFTPFolderFTP*) m_pRoot)->m_pChannel->GetServerVersion(), pOldItem, &pAttr->fileData);
	//		delete pAttr;
	//	}
	//	break;
	//}
}

////////////////////////////////////////////////////////////////////////////////

CSFTPFolderFTP::CSFTPFolderFTP(CDelegateMallocData* pMallocData, CFTPDirectoryItem* pItemMe, CEasySFTPFolderRoot* pFolderRoot, bool bIsFTPS)
	: CFTPDirectoryRootBase(pMallocData, pItemMe, pFolderRoot)
{
	m_pConnection = NULL;
	m_pUser = NULL;
	::InitializeCriticalSection(&m_csSocket);
	m_dwTransferringCount = 0;
	m_nServerCharset = scsUTF8;
	m_bIsFTPS = bIsFTPS;
}

CSFTPFolderFTP::~CSFTPFolderFTP()
{
	if (m_pConnection)
		DisconnectImpl(m_pConnection);
	::DeleteCriticalSection(&m_csSocket);
}

bool CSFTPFolderFTP::Connect(HWND hWnd, LPCWSTR lpszHostName, int nPort, IEasySFTPAuthentication* pUser)
{
	CMyScopedCSLock lock(m_csSocket);
	m_hWndOwner = hWnd;

	if (m_pUser)
		m_pUser->Release();
	if (pUser)
	{
		m_pUser = pUser;
		pUser->AddRef();
		//m_nServerCharset = scsUTF8;
	}
	else
	{
		m_pUser = new CAuthentication();
		if (DoRetryAuthentication(true) <= 0)
		{
			m_pUser->Release();
			m_pUser = NULL;
			return false;
		}
		//m_nServerCharset = scsUTF8;
	}

	if (m_pConnection)
		DisconnectImpl(m_pConnection);

	{
		CMyStringW str(lpszHostName);
		m_strHostName = str;
	}
	m_pConnection = new CFTPConnection();
	m_pConnection->m_socket.SetCharset((ServerCharset)m_nServerCharset);

	if (!m_pConnection->Connect(nPort, lpszHostName))
	{
		delete m_pConnection;
		m_pConnection = NULL;
		m_pUser->Release();
		m_pUser = NULL;
		return false;
	}
	//m_pSocket->AsyncSelect(m_hWnd, MY_WM_SOCKETMESSAGE, FD_READ | FD_CLOSE);
	m_nServerSystemType = SVT_UNKNOWN;
	m_nYearFollows = 0;
	m_bY2KProblem = false;

	m_idTimer = theApp.RegisterTimer(KEEP_CONNECTION_TIME_SPAN,
		(PFNEASYSFTPTIMERPROC)KeepConnectionTimerProc, (LPARAM)this);

	//SetStatusText(ID_STATUS_HOST, lpszHostName);

	if (!WaitForReceive(&m_pConnection->m_bIsLoggingIn))
	{
		DisconnectImpl(m_pConnection);
		return false;
	}
	return true;
}

STDMETHODIMP CSFTPFolderFTP::get_IsUnixServer(VARIANT_BOOL* pbRet)
{
	if (!pbRet)
		return E_POINTER;
	*pbRet = m_nServerSystemType == SVT_UNIX ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CSFTPFolderFTP::Disconnect()
{
	if (!m_pConnection)
		return S_OK;

	DisconnectImpl(m_pConnection);

	return S_OK;
}

STDMETHODIMP CSFTPFolderFTP::OpenFile(CFTPDirectoryBase* pDirectory, LPCWSTR lpszName, DWORD grfMode, HANDLE* phFile, IEasySFTPFile** ppFile)
{
	if (grfMode & (STGM_READWRITE | STGM_SHARE_DENY_READ | STGM_SHARE_DENY_WRITE | STGM_SHARE_EXCLUSIVE | STGM_PRIORITY | STGM_CONVERT | STGM_TRANSACTED | STGM_NOSCRATCH | STGM_NOSNAPSHOT | STGM_SIMPLE | STGM_DIRECT_SWMR | STGM_DELETEONRELEASE))
	{
		return STG_E_INVALIDFLAG;
	}
	if ((grfMode & STGM_WRITE) && !(grfMode & STGM_CREATE))
	{
		return STG_E_INVALIDFLAG;
	}

	auto* data = new CFTPHandleData();
	if (!data)
	{
		return E_OUTOFMEMORY;
	}

	HRESULT hr = S_OK;
	CMyStringW strFile;

	strFile = pDirectory->m_strDirectory;
	if (((LPCWSTR)strFile)[strFile.GetLength() - 1] != L'/')
		strFile += L'/';
	strFile += lpszName;

	CFTPFileItem* pItem = NULL;

	// for reading, retrieve file first
	if (!(grfMode & STGM_WRITE))
	{
		pItem = RetrieveFileItem(pDirectory, lpszName);
		if (!pItem)
		{
			delete data;
			return STG_E_FILENOTFOUND;
		}
	}

	CFTPPassiveMessage* pMessage;
	if (grfMode & STGM_WRITE)
		pMessage = new CFTPWriteFileMessage(strFile);
	else
		pMessage = new CFTPFileRecvMessage(strFile, 0);
	if (!pMessage)
	{
		delete data;
		return E_OUTOFMEMORY;
	}

	data->pMessage = pMessage;

	IncrementTransferCount();

	{
		CMyScopedCSLock lock(m_csSocket);

		CFTPWaitEstablishPassive* pEstablishWait = StartPassive(pMessage);
		pMessage->Release();
		if (!pEstablishWait)
		{
			delete data;
			DecrementTransferCount();
			return E_OUTOFMEMORY;
		}
		else if (!WaitForReceiveEstablishPassive(&pEstablishWait->bWaiting, pEstablishWait) || !pEstablishWait->pRet)
		{
			delete pEstablishWait;
			delete data;
			DecrementTransferCount();
			return E_FAIL;
		}
		data->pPassive = pEstablishWait->pRet;
		delete pEstablishWait;
	}

	// for writing, retrieve file second
	if (grfMode & STGM_WRITE)
	{
		pItem = RetrieveFileItem(pDirectory, lpszName);
		if (!pItem)
		{
			delete data->pPassive;
			pMessage->Release();
			delete data;
			DecrementTransferCount();
			return STG_E_FILENOTFOUND;
		}
	}
	data->pDirectory = pDirectory;
	pDirectory->AddRef();
	data->pItem = pItem;
	data->grfMode = grfMode;
	data->offset = 0;
	data->dwRefCount = 1;

	FTPItemToStatStg(pItem, STATFLAG_NONAME, grfMode, 0, &data->statstg);

	*phFile = static_cast<HANDLE>(data);

	if (ppFile)
	{
		*ppFile = pItem;
		pItem->AddRef();
	}
	return hr;
}

STDMETHODIMP CSFTPFolderFTP::ReadFile(HANDLE hFile, void* outBuffer, DWORD dwSize, DWORD* pdwRead)
{
	if (pdwRead)
		*pdwRead = 0;
	if (!hFile || !outBuffer)
		return E_POINTER;
	auto data = static_cast<CFTPHandleData*>(hFile);
	if (data->grfMode & STGM_WRITE)
		return STG_E_ACCESSDENIED;

	if (!data->pPassive)
	{
		// already closed
		return S_FALSE;
	}

	auto* pSocket = data->pPassive->pPassive;

	ULONG read = 0;
	DWORD dwStartTick = GetTickCount();
	while (dwSize)
	{
		if (pSocket->IsRemoteClosed())
		{
			delete data->pPassive;
			data->pPassive = NULL;
			break;
		}
		if (data->pPassive->pConnection->m_socket.CanReceive(0))
		{
			DoReceiveSocketPassiveControl(data->pPassive->pConnection);
			if (data->pPassive->nWaitFlags == CFTPWaitPassive::WaitFlags::Finished || data->pPassive->nWaitFlags == CFTPWaitPassive::WaitFlags::Error)
			{
				delete data->pPassive;
				data->pPassive = NULL;
				break;
			}
		}
		if (!pSocket->CanReceive(0))
		{
			::Sleep(1);
			if (GetTickCount() - dwStartTick >= WAIT_RECEIVE_TIME)
				break;
			continue;
		}
		bool needMoreData = false;
		int ret = pSocket->Recv(outBuffer, (SIZE_T)dwSize, 0, &needMoreData);
		if (needMoreData)
			continue;
		if (ret < 0)
			return E_FAIL;
		if (!ret)
			break;
		data->offset += (ULONG)ret;
		read += (ULONG)ret;
		outBuffer = ((LPBYTE)outBuffer) + (ULONG)ret;
		dwSize -= (ULONG)ret;
	}
	if (pdwRead)
		*pdwRead = read;
	return read ? S_OK : S_FALSE;
}

STDMETHODIMP CSFTPFolderFTP::WriteFile(HANDLE hFile, const void* inBuffer, DWORD dwSize, DWORD* pdwWritten)
{
	if (pdwWritten)
		*pdwWritten = 0;
	if (!hFile || !inBuffer)
		return E_POINTER;
	auto data = static_cast<CFTPHandleData*>(hFile);
	if (!(data->grfMode & STGM_WRITE))
		return STG_E_ACCESSDENIED;
	if (!data->pPassive)
		return E_ABORT;

	CMyScopedCSLock lock(m_csSocket);

	auto pMessage = static_cast<CFTPWriteFileMessage*>(data->pMessage);
	pMessage->m_pvBuffer = inBuffer;
	pMessage->m_dwSize = dwSize;

	if (!pMessage->ReadyToWrite(data->pPassive->pPassive))
		return E_ABORT;

	data->offset += dwSize;
	if (pdwWritten)
		*pdwWritten = dwSize;
	return S_OK;
}

STDMETHODIMP CSFTPFolderFTP::SeekFile(HANDLE hFile, LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer)
{
	auto data = static_cast<CFTPHandleData*>(hFile);

	if (data->grfMode & STGM_WRITE)
		return E_NOTIMPL;

	if (dwOrigin == STREAM_SEEK_END || (dwOrigin == STREAM_SEEK_CUR && liDistanceToMove.QuadPart < 0) ||
		dwOrigin == STREAM_SEEK_SET && (ULONGLONG)liDistanceToMove.QuadPart < data->offset)
	{
		if (!m_pConnection->IsCommandAvailable(L"REST"))
			return E_NOTIMPL;

		// reopen

		ULONGLONG ulTo;
		switch (dwOrigin)
		{
		case STREAM_SEEK_SET:
			ulTo = (ULONGLONG)liDistanceToMove.QuadPart;
			break;
		case STREAM_SEEK_CUR:
			ulTo = data->offset + liDistanceToMove.QuadPart;
			break;
		case STREAM_SEEK_END:
			ulTo = data->offset + liDistanceToMove.QuadPart;
			break;
		default:
			return STG_E_INVALIDFUNCTION;
		}

		CMyScopedCSLock lock(m_csSocket);

		if (data->pPassive)
		{
			delete data->pPassive;
			data->pPassive = NULL;
		}

		CMyStringW strFile;
		strFile = data->pDirectory->m_strDirectory;
		if (((LPCWSTR)strFile)[strFile.GetLength() - 1] != L'/')
			strFile += L'/';
		strFile += data->pItem->strFileName;

		CFTPFileRecvMessage* pMessage = new CFTPFileRecvMessage(strFile, ulTo);
		if (!pMessage)
		{
			return E_OUTOFMEMORY;
		}
		CFTPWaitEstablishPassive* pEstablishWait = StartPassive(pMessage);
		if (!pEstablishWait)
		{
			pMessage->Release();
			return E_OUTOFMEMORY;
		}
		else if (!WaitForReceiveEstablishPassive(&pEstablishWait->bWaiting, pEstablishWait) || !pEstablishWait->pRet)
		{
			delete pEstablishWait;
			pMessage->Release();
			return E_FAIL;
		}
		data->pPassive = pEstablishWait->pRet;
		delete pEstablishWait;
		data->offset = ulTo;
		return S_OK;
	}
	ULONGLONG ulTo = (dwOrigin == STREAM_SEEK_SET ? (ULONGLONG)liDistanceToMove.QuadPart :
		data->offset + liDistanceToMove.QuadPart);
	void* pb = malloc(8192);
	while (data->offset < ulTo)
	{
		ULONG ul = 8192;
		if (ulTo - data->offset < 8192)
			ul = (ULONG)(ulTo - data->offset);
		HRESULT hr = ReadFile(hFile, pb, ul, &ul);
		if (FAILED(hr))
		{
			free(pb);
			return hr;
		}
	}
	free(pb);
	if (lpNewFilePointer)
		lpNewFilePointer->QuadPart = data->offset;
	return S_OK;
}

STDMETHODIMP CSFTPFolderFTP::StatFile(HANDLE hFile, STATSTG* pStatstg, DWORD grfStatFlag)
{
	if (!hFile || !pStatstg)
		return E_POINTER;
	auto data = static_cast<CFTPHandleData*>(hFile);
	*pStatstg = data->statstg;
	if (!(grfStatFlag & STATFLAG_NONAME))
		pStatstg->pwcsName = DuplicateCoMemString(data->pItem->strFileName);
	return S_OK;
}

STDMETHODIMP CSFTPFolderFTP::CloseFile(HANDLE hFile)
{
	auto data = static_cast<CFTPHandleData*>(hFile);
	if (!--data->dwRefCount)
	{
		if (data->pPassive)
		{
			data->pPassive->pPassive->Close();
			data->pPassive->bWaiting = true;
			if (data->pPassive->nWaitFlags != CFTPWaitPassive::WaitFlags::Finished && data->pPassive->nWaitFlags != CFTPWaitPassive::WaitFlags::Error)
			{
				data->pPassive->nWaitFlags = CFTPWaitPassive::WaitFlags::WaitingForPassiveDone;
				WaitForReceivePassive(&data->pPassive->bWaiting, data->pPassive, 5000);
			}
			delete data->pPassive;
		}
		if (data->pMessage)
			data->pMessage->Release();
		delete data;
		DecrementTransferCount();
	}
	return S_OK;
}

STDMETHODIMP CSFTPFolderFTP::DuplicateFile(HANDLE hFile, HANDLE* phFile)
{
	auto data = static_cast<CFTPHandleData*>(hFile);
	++data->dwRefCount;
	*phFile = hFile;
	return S_OK;
}

STDMETHODIMP CSFTPFolderFTP::LockRegion(HANDLE hFile, ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	return STG_E_INVALIDFUNCTION;
}

STDMETHODIMP CSFTPFolderFTP::UnlockRegion(HANDLE hFile, ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD /*dwLockType*/)
{
	return STG_E_INVALIDFUNCTION;
}

STDMETHODIMP CSFTPFolderFTP::StatDirectory(CFTPDirectoryBase* pDirectory, DWORD grfMode, STATSTG* pStatstg, DWORD grfStatFlag)
{
	if (!pStatstg)
		return E_POINTER;
	if (pDirectory->m_bIsRoot)
	{
		*pStatstg = {};
		pStatstg->grfMode = m_grfMode;
		pStatstg->grfStateBits = m_grfStateBits;
	}
	else
	{
		auto pName = wcsrchr(m_strDirectory, L'/');
		if (!pName)
			pName = m_strDirectory;
		else
			++pName;
		auto* pParent = pDirectory->GetParent();
		if (!pParent)
			return STG_E_PATHNOTFOUND;
		auto pItem = pParent->GetFileItem(pName);
		if (!pItem)
			return STG_E_PATHNOTFOUND;
		FTPItemToStatStg(pItem, grfStatFlag, grfMode, pDirectory->m_grfStateBits, pStatstg);
		pStatstg->clsid = pDirectory->m_clsidThis;
	}
	return S_OK;
}

STDMETHODIMP CSFTPFolderFTP::SetFileTime(LPCWSTR lpszFileName, const FILETIME* pctime, const FILETIME* patime, const FILETIME* pmtime)
{
	if (!pctime && !pmtime)
		return S_OK;

	HRESULT hr = S_OK;
	bool isSucceededAny = false;

	{
		CMyScopedCSLock lock(m_csSocket);
		if (pctime && m_pConnection->IsCommandAvailable(L"MFCT"))
		{
			HRESULT hr2;
			CFTPWaitConfirm* pData = new CFTPWaitConfirm();
			if (!pData)
			{
				hr2 = E_OUTOFMEMORY;
			}
			else
			{
				CMyStringW strParam;
				FormatMfctMfmtString(strParam, lpszFileName, pctime);

				pData->bWaiting = true;
				pData->nCode = 0;
				if (!m_pConnection)
					hr2 = OLE_E_NOTRUNNING;
				else if (!m_pConnection->SendCommand(L"MFCT", strParam, pData))
					hr2 = E_UNEXPECTED;
				else if (!WaitForReceive(&pData->bWaiting))
					hr2 = E_UNEXPECTED;
				else
				{
					hr2 = (pData->nCode < 300 && pData->nCode != 202 ? S_OK : E_FAIL);
				}

				if (SUCCEEDED(hr2))
					isSucceededAny = true;
				delete pData;
			}
			hr = hr2;
		}
		if (pmtime && m_pConnection->IsCommandAvailable(L"MFMT"))
		{
			HRESULT hr2;
			CFTPWaitConfirm* pData = new CFTPWaitConfirm();
			if (!pData)
			{
				hr2 = E_OUTOFMEMORY;
			}
			else
			{
				CMyStringW strParam;
				FormatMfctMfmtString(strParam, lpszFileName, pctime);

				pData->bWaiting = true;
				pData->nCode = 0;
				if (!m_pConnection)
					hr2 = OLE_E_NOTRUNNING;
				else if (!m_pConnection->SendCommand(L"MFMT", strParam, pData))
					hr2 = E_UNEXPECTED;
				else if (!WaitForReceive(&pData->bWaiting))
					hr2 = E_UNEXPECTED;
				else
				{
					hr2 = (pData->nCode < 300 && pData->nCode != 202 ? S_OK : E_FAIL);
				}

				if (SUCCEEDED(hr2))
					isSucceededAny = true;
				delete pData;
			}
			if (SUCCEEDED(hr))
				hr = hr2;
		}
	}
	if (isSucceededAny)
	{
		CFTPDirectoryBase* pDirectory = NULL;
		auto* pItem = GetFileItem(lpszFileName, &pDirectory);
		if (pItem && pDirectory)
		{
			pDirectory->UpdateFileAttrs(lpszFileName, pItem->IsDirectory());
			pDirectory->Release();
		}
	}
	return hr;
}

void CSFTPFolderFTP::IncrementTransferCount()
{
	::InterlockedIncrement((LONG*)&m_dwTransferringCount);
}

void CSFTPFolderFTP::DecrementTransferCount()
{
	::InterlockedDecrement((LONG*)&m_dwTransferringCount);
}

STDMETHODIMP CSFTPFolderFTP::GetFTPItemUIObjectOf(HWND hWndOwner, CFTPDirectoryBase* pDirectory,
	const CMyPtrArrayT<CFTPFileItem>& aItems, CFTPDataObject** ppObject)
{
	if (!m_pConnection)
		return OLE_E_NOTRUNNING;
	::EnterCriticalSection(&pDirectory->m_csPidlMe);
	CFTPDataObject* pObject = new CFTPDataObject(//(CSFTPFolderFTPDirectory*) pDirectory,
		m_pMallocData->pMalloc,
		pDirectory->m_pidlMe,
		m_strHostName,
		m_pConnection,
		this,
		pDirectory, aItems);
	::LeaveCriticalSection(&pDirectory->m_csPidlMe);
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
	if (((LPCWSTR)strFrom)[dw] != L'/')
		strFrom += L'/';
	if (((LPCWSTR)strTo)[dw] != L'/')
		strTo += L'/';
	strFrom += pItem->strFileName;
	strTo += pszName;

	CWaitRenameData* pWait = new CWaitRenameData();
	if (!pWait)
	{
		::MyMessageBoxW(hWnd, MAKEINTRESOURCEW(IDS_COMMAND_OUTOFMEMORY), NULL, MB_ICONHAND);
		return E_OUTOFMEMORY;
	}

	CMyScopedCSLock lock(m_csSocket);

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

STDMETHODIMP CSFTPFolderFTP::RenameFTPItem(LPCWSTR lpszSrcFileName, LPCWSTR lpszNewFileName, CMyStringW* pstrMsg)
{
	CWaitRenameData* pData = NULL;
	HRESULT hr;

	CFTPFileItem* p = RetrieveFileItem2(NULL, lpszSrcFileName);
	if (!p)
	{
		hr = E_UNEXPECTED;
	}
	else
	{
		bool bIsDir = p->IsDirectory();
		p->Release();
		pData = new CWaitRenameData();
		if (!pData)
		{
			hr = E_OUTOFMEMORY;
		}
		else
		{
			CMyScopedCSLock lock(m_csSocket);

			pData->bWaiting = true;
			pData->bSecondary = false;
			pData->nCode = 0;
			if (!m_pConnection)
				hr = OLE_E_NOTRUNNING;
			else if (!m_pConnection->SendDoubleCommand(L"RNFR", lpszSrcFileName, L"RNTO", lpszNewFileName, NULL, pData))
				hr = E_UNEXPECTED;
			else if (!WaitForReceive(&pData->bWaiting))
				hr = E_UNEXPECTED;
			else
			{
				hr = (pData->nCode < 300 && pData->nCode != 202 ? S_OK : E_FAIL);
			}

			if (SUCCEEDED(hr))
			{
				CMyStringW strOldDirectory = lpszSrcFileName;
				{
					auto p = wcsrchr(strOldDirectory.GetBuffer(), L'/');
					if (p && p != strOldDirectory.operator LPCWSTR())
					{
						*p = 0;
						strOldDirectory.ReleaseBuffer();
					}
				}
				{
					auto p = wcsrchr(lpszSrcFileName, L'/');
					if (p)
						lpszSrcFileName = p + 1;
				}
				CMyStringW strNewDirectory = lpszNewFileName;
				{
					auto p = wcsrchr(strNewDirectory.GetBuffer(), L'/');
					if (p && p != strNewDirectory.operator LPCWSTR())
					{
						*p = 0;
						strNewDirectory.ReleaseBuffer();
					}
				}
				{
					auto p = wcsrchr(lpszNewFileName, L'/');
					if (p)
						lpszNewFileName = p + 1;
				}
				CFTPDirectoryBase* pDirectory;
				// strNewDirectory must be absolute path so convert to relative path from 'root'
				if (SUCCEEDED(OpenNewDirectory(strNewDirectory.operator LPCWSTR() + 1, &pDirectory)))
				{
					pDirectory->UpdateMoveFile(strOldDirectory, lpszSrcFileName, bIsDir, lpszNewFileName);
					pDirectory->Release();
				}
			}
		}
	}

	if (FAILED(hr) && pstrMsg)
	{
		CMyStringW str, str2;
		if (hr == OLE_E_NOTRUNNING)
			str2.LoadString(IDS_COMMAND_CONNECTION_ERROR);
		else if (hr == E_UNEXPECTED)
			str2.LoadString(IDS_COMMAND_UNKNOWN_ERROR);
		else if (!pData)
			str2.LoadString(IDS_COMMAND_OUTOFMEMORY);
		else if (!pData->nCode)
			str2.LoadString(IDS_COMMAND_UNKNOWN_ERROR);
		else
			GetFTPStatusMessage(pData->nCode, pData->strMsg, str2);
		str = lpszSrcFileName;
		str += L": ";
		str += str2;
		*pstrMsg = str;
	}
	if (pData)
		delete pData;
	return hr;
}

STDMETHODIMP CSFTPFolderFTP::UpdateFTPItemAttribute(CFTPDirectoryBase* pDirectory, const CServerFileAttrData* pAttr, CMyStringW* pstrMsg)
{
	if (!(pAttr->wMask & ServerFileAttrDataMask::Attribute))
		return S_OK;
	CMyStringW strFile = pDirectory->m_strDirectory;
	if (((LPCWSTR)strFile)[strFile.GetLength() - 1] != L'/')
		strFile += L'/';
	strFile += pAttr->pItem->strFileName;

	HRESULT hr;
	CFTPWaitConfirm* pData = new CFTPWaitConfirm();
	if (!pData)
	{
		hr = E_OUTOFMEMORY;
	}
	else
	{
		CMyScopedCSLock lock(m_csSocket);

		CMyStringW strMod;
		strMod.Format(L"%03o \"%s\"", pAttr->nUnixMode, (LPCWSTR)strFile);

		pData->bWaiting = true;
		pData->nCode = 0;
		if (!m_pConnection)
			hr = OLE_E_NOTRUNNING;
		else if (!m_pConnection->SendCommand(m_strChmodCommand, strMod, pData))
			hr = E_UNEXPECTED;
		else if (!WaitForReceive(&pData->bWaiting))
			hr = E_UNEXPECTED;
		else
		{
			hr = (pData->nCode < 300 && pData->nCode != 202 ? S_OK : E_FAIL);
		}

		if (SUCCEEDED(hr))
			pDirectory->UpdateFileAttrs(pAttr->pItem->strFileName, pAttr->pItem->IsDirectory());
	}
	if (FAILED(hr) && pstrMsg)
	{
		CMyStringW str, str2;
		if (hr == OLE_E_NOTRUNNING)
			str2.LoadString(IDS_COMMAND_CONNECTION_ERROR);
		else if (hr == E_OUTOFMEMORY)
			str2.LoadString(IDS_COMMAND_OUTOFMEMORY);
		else if (!pData->nCode)
			str2.LoadString(IDS_COMMAND_UNKNOWN_ERROR);
		else
			GetFTPStatusMessage(pData->nCode, pData->strMsg, str2);
		str = strFile;
		str += L": ";
		str += str2;
		*pstrMsg = str;
	}
	if (pData)
		delete pData;
	return hr;
}

STDMETHODIMP CSFTPFolderFTP::CreateFTPDirectory(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszName)
{
	{
		auto hr = IsDirectoryExists(hWndOwner, pDirectory, lpszName);
		if (FAILED(hr))
			return hr;
		if (hr == S_OK)
			return S_OK;
	}

	CMyStringW strFile(pDirectory->m_strDirectory);
	if (strFile.IsEmpty() || ((LPCWSTR)strFile)[strFile.GetLength() - 1] != L'/')
		strFile += L'/';
	strFile += lpszName;

	CFTPWaitConfirm* pWait = new CFTPWaitConfirm();
	if (!pWait)
	{
		::MyMessageBoxW(hWndOwner, MAKEINTRESOURCEW(IDS_COMMAND_OUTOFMEMORY), NULL, MB_ICONHAND);
		return E_OUTOFMEMORY;
	}

	CMyScopedCSLock lock(m_csSocket);

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
	pDirectory->UpdateNewFile(lpszName, true);
	return S_OK;
}

STDMETHODIMP CSFTPFolderFTP::CreateFTPItemStream(CFTPDirectoryBase* pDirectory, CFTPFileItem* pItem, IStream** ppStream)
{
	CMyStringW strFileName = pDirectory->m_strDirectory;
	if (strFileName.IsEmpty() ||
		((LPCWSTR)strFileName)[strFileName.GetLength() - 1] != L'/')
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
	//if (!WaitForReceiveEstablishPassive(&pEstablishWait->bWaiting, pEstablishWait) || !pEstablishWait->pRet)
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
	if (((LPCWSTR)strFile)[strFile.GetLength() - 1] != L'/')
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
		::EnterCriticalSection(&m_csSocket);
		CFTPWaitEstablishPassive* pEstablishWait = StartPassive(pMessage);
		if (!pEstablishWait)
		{
			::LeaveCriticalSection(&m_csSocket);
			pMessage->Release();
			hr = E_OUTOFMEMORY;
			CMyStringW str;
			str.LoadString(IDS_COMMAND_OUTOFMEMORY);
			strMsg = strFile;
			strMsg += L": ";
			strMsg += str;
		}
		else if (!WaitForReceiveEstablishPassive(&pEstablishWait->bWaiting, pEstablishWait) || !pEstablishWait->pRet)
		{
			::LeaveCriticalSection(&m_csSocket);
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
			::LeaveCriticalSection(&m_csSocket);
			CFTPWaitPassive* pPassive = pEstablishWait->pRet;
			delete pEstablishWait;
			if (!WaitForReceivePassive(&pPassive->bWaiting, pPassive) || pPassive->nWaitFlags == CFTPWaitPassive::WaitFlags::Error)
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
				if (pMessage->m_bCanceled)
				{
					delete pPassive;
				}
				else
				{
					pPassive->pPassive->Close();
					pPassive->nWaitFlags = CFTPWaitPassive::WaitFlags::WaitingForPassiveDone;
					if (!WaitForReceivePassive(&pPassive->bWaiting, pPassive, 5000))
					{
						hr = E_FAIL;
						CMyStringW str;
						str.LoadString(IDS_COMMAND_FAILED);
						strMsg = strFile;
						strMsg += L": ";
						strMsg += str;
					}
					else
						pDirectory->UpdateNewFile(lpszName, false);
					delete pPassive;
				}
				pMessage->Release();
			}
		}
	}

	DecrementTransferCount();
	if (FAILED(hr) && !strMsg.IsEmpty())
		::MyMessageBoxW(hWndOwner, strMsg, NULL, MB_ICONHAND);
	return hr;
}

STDMETHODIMP CSFTPFolderFTP::CreateInstance(CFTPDirectoryItem* pItemMe, CFTPDirectoryBase* pParent, LPCWSTR lpszDirectory, CFTPDirectoryBase** ppResult)
{
	CSFTPFolderFTPDirectory* p = new CSFTPFolderFTPDirectory(m_pMallocData, pItemMe, pParent, lpszDirectory);
	if (!p)
		return E_OUTOFMEMORY;
	*ppResult = p;
	return S_OK;
}

void CSFTPFolderFTP::PreShowPropertyDialog(CServerFilePropertyDialog* pDialog)
{
	pDialog->m_bChangeOwner = false;
	pDialog->m_bChangeAttr = m_nServerSystemType == SVT_UNIX;
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
		getnameinfo(psa, (socklen_t)nLen, str.GetBufferA(NI_MAXHOST + 1), NI_MAXHOST + 1, NULL, 0, NI_NUMERICHOST);
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
	if (!m_pConnection)
	{
		if (!Connect(hWndOwner, m_strHostName, m_nPort, NULL))
		{
			return false;
		}
	}
	if (*pbReceived)
	{
		return true;
	}

	CMyScopedCSLock lock(m_csSocket);
	*pbReceived = true;
	CFTPFileListingHandler* pHandler = new CFTPFileListingHandler(this, pDirectory);
	if (!pHandler)
	{
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
	if (!WaitForReceiveEstablishPassive(&pEstablishWait->bWaiting, pEstablishWait) || !pEstablishWait->pRet)
	{
		delete pEstablishWait;
		delete pHandler;
		return false;
	}
	CFTPWaitPassive* pPassiveWait = pEstablishWait->pRet;
	delete pEstablishWait;

	pPassiveWait->bWaiting = true;
	if (!WaitForReceivePassive(&pPassiveWait->bWaiting, pPassiveWait) || pPassiveWait->nWaitFlags == CFTPWaitPassive::WaitFlags::Error)
	{
		delete pPassiveWait;
		delete pHandler;
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
	bLastIsNotDelimiter = (strPath.IsEmpty() || ((LPCWSTR)strPath)[strPath.GetLength() - 1] != L'/');
	CMyStringW str;
	bFirst = true;
	while (pidlChild && pidlChild->mkid.cb)
	{
		if (!PickupFileName((PUITEMID_CHILD)pidlChild, str))
			return false;
		if (bLastIsNotDelimiter)
			strPath += L'/';
		strPath += str;
		bLastIsNotDelimiter = true;
		pidlChild = (PCUIDLIST_RELATIVE)(((DWORD_PTR)pidlChild) + pidlChild->mkid.cb);
	}

	// validate only if connected
#if 1 || defined(_FORCE_VALIDATE)
	if (m_pConnection)
	{
		CMyScopedCSLock lock(m_csSocket);

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
		((LPCWSTR)strFileName)[strFileName.GetLength() - 1] != L'/')
		strFileName += L'/';
	strFileName += lpszFileName;
	return RetrieveFileItem2(pDirectory, strFileName);
}

CFTPFileItem* CSFTPFolderFTP::RetrieveFileItem2(CFTPDirectoryBase* pDirectory, LPCWSTR lpszFullPathName)
{
	CMyScopedCSLock lock(m_csSocket);
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
	if (pData->strResult.IsEmpty())
	{
		delete pData;
		return NULL;
	}
	CFTPFileItem* pItem;
	if (bMLST)
		pItem = ParseMLSxData(pDirectory, pData->strResult);
	else
	{
		switch (m_nServerSystemType)
		{
		case SVT_UNKNOWN:
			pItem = ParseUnixFileList(pDirectory, pData->strResult);
			if (!pItem)
			{
				pItem = ParseDOSFileList(pDirectory, pData->strResult, &m_nYearFollows, &m_bY2KProblem);
			}
			break;
		case SVT_UNIX:
			pItem = ParseUnixFileList(pDirectory, pData->strResult);
			break;
		case SVT_DOS:
		case SVT_WINDOWS:
			pItem = ParseDOSFileList(pDirectory, pData->strResult, &m_nYearFollows, &m_bY2KProblem);
			break;
		default:
			pItem = NULL;
		}
	}
	delete pData;

	return pItem;
}

HRESULT CSFTPFolderFTP::DoDeleteFileOrDirectory(HWND hWndOwner, CMyStringArrayW& astrMsgs, bool bIsDirectory, LPCWSTR lpszFile, CFTPDirectoryBase* pDirectory)
{
	HRESULT hr;
	CFTPWaitConfirm* pData = new CFTPWaitConfirm();
	if (!pData)
		hr = E_OUTOFMEMORY;
	else
	{
		CMyScopedCSLock lock(m_csSocket);
		pData->nCode = 0;
		pData->bWaiting = true;
		if (!m_pConnection)
			hr = OLE_E_NOTRUNNING;
		else if (!m_pConnection->SendCommand(bIsDirectory ? L"RMD" : L"DELE", lpszFile, pData))
			hr = E_UNEXPECTED;
		else if (!WaitForReceive(&pData->bWaiting))
			hr = E_UNEXPECTED;
		else
		{
			hr = (pData->nCode < 300 ? S_OK : E_FAIL);
		}

		if (SUCCEEDED(hr) && pDirectory != NULL)
		{
			auto pName = wcsrchr(lpszFile, L'/');
			pName = pName != NULL ? pName + 1 : lpszFile;
			pDirectory->UpdateRemoveFile(pName, bIsDirectory);
		}
	}
	if (FAILED(hr))
	{
		CMyStringW str, str2;
		if (hr == OLE_E_NOTRUNNING)
			str2.LoadString(IDS_COMMAND_CONNECTION_ERROR);
		else if (!pData)
			str2.LoadString(IDS_COMMAND_OUTOFMEMORY);
		else if (!pData->nCode)
			str2.LoadString(IDS_COMMAND_UNKNOWN_ERROR);
		else
			GetFTPStatusMessage(pData->nCode, pData->strMsg, str2);
		str = lpszFile;
		str += L": ";
		str += str2;
		astrMsgs.Add(str);
	}
	if (pData)
		delete pData;
	return hr;
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
	CSFTPFolderFTP* pThis = (CSFTPFolderFTP*)lParam;

	// send 'ignore'/no-op message to keep connection
	CMyScopedCSLock lock(pThis->m_csSocket);
	if (pThis->m_pConnection)
		pThis->m_pConnection->SendCommand(L"NOOP", NULL);
}

// in FileList.cpp
extern "C" bool __stdcall _ParseToFileTime(LPCWSTR lpszString, LPCWSTR lpszStop, FILETIME * pftTime);

CSFTPFolderFTP::ProcessLoginResult CSFTPFolderFTP::DoProcessForLogin(CFTPConnection* pConnection, int code, CMyStringW& strMsg, const CMyStringW& strCommand, CWaitResponseData* pWait)
{
	auto resultFTPS = pConnection->OnFirstFTPSHandshake(code);
	switch (resultFTPS)
	{
	case CFTPConnection::FTPSHandshakeResult::Success:
		StartAuth(pConnection);
		return ProcessLoginResult::InProgress;
	case CFTPConnection::FTPSHandshakeResult::Failure:
		return ProcessLoginResult::Failure;
	case CFTPConnection::FTPSHandshakeResult::InProgress:
		return ProcessLoginResult::InProgress;
	}
	ProcessLoginResult result = ProcessLoginResult::InProgress;
	switch (code)
	{
		// response for NOOP, etc.
	case 200:
		break;
		// response for FEAT
	case 211:
		if (pWait && pWait->nWaitType == CWaitResponseData::WRD_GETFEATURE)
		{
			_ASSERT(pConnection == m_pConnection);
			result = ProcessLoginResult::Finish;
			pConnection->InitAvaliableCommands(strMsg);
		}
		break;
		// initial message
	case 220:
		if (pConnection == m_pConnection)
			m_strServerInfo = strMsg;
		if (m_bIsFTPS)
		{
			pConnection->StartFTPSHandshake();
		}
		else
		{
			StartAuth(pConnection);
		}
		//SetStatusText(MAKEINTRESOURCEW(IDS_AUTHENTICATING));
		break;
		// response for SYST
	case 215:
		_ASSERT(pConnection == m_pConnection);
		ConvertToLowerLenW(strMsg.GetBuffer(), (SIZE_T)strMsg.GetLength());
		if (wcsstr(strMsg, L"unix"))
			m_nServerSystemType = SVT_UNIX;
		else if (wcsstr(strMsg, L"dos"))
			m_nServerSystemType = SVT_DOS;
		else if (wcsstr(strMsg, L"windows"))
			m_nServerSystemType = SVT_WINDOWS;
		//else
		//	m_nServerSystemType = SVT_UNKNOWN;
		pConnection->SendCommand(L"FEAT", NULL, new CWaitFeatureData());
		break;
		// response for USER
	case 331:
	{
		BSTR bstrPass;
		if (FAILED(m_pUser->get_Password(&bstrPass)))
			bstrPass = ::SysAllocString(L"");
		pConnection->SecureSendCommand(L"PASS", bstrPass);
		_SecureStringW::SecureEmptyBStr(bstrPass);
		::SysFreeString(bstrPass);
	}
	break;
	case 230:
		//SetStatusText(MAKEINTRESOURCEW(IDS_CONNECTED));
		if (pConnection == m_pConnection)
		{
			LPWSTR lpw = strMsg.GetBuffer();
			LPWSTR lpw2R = wcsrchr(lpw, L'\r');
			LPWSTR lpw2L = wcsrchr(lpw, L'\n');
			if (lpw2R || lpw2L)
			{
				if (lpw2R)
					*lpw2R = 0;
				else
					*lpw2L = 0;
				strMsg.ReleaseBuffer();
			}
			else
				strMsg.Empty();
			m_strWelcomeMessage = strMsg;
			pConnection->SendCommand(L"SYST", NULL);
		}
		else
			result = ProcessLoginResult::Finish;
		break;
	case 530:
		switch (DoRetryAuthentication(false))
		{
		case 1:
		{
			BSTR bstrUser;
			if (FAILED(m_pUser->get_UserName(&bstrUser)))
				bstrUser = ::SysAllocString(L"");
			pConnection->SendCommand(L"USER", bstrUser);
			::SysFreeString(bstrUser);
		}
		break;
		case 0:
			result = ProcessLoginResult::Cancel;
			break;
		case -1:
			result = ProcessLoginResult::Failure;
			break;
		}
		break;
	case 500:
	default:
		if (pWait && pWait->nWaitType == CWaitResponseData::WRD_GETFEATURE)
		{
			result = ProcessLoginResult::Finish;
			pConnection->InitAvaliableCommands(NULL);
			delete ((CWaitFeatureData*)pWait);
			pWait = NULL;
		}
		else
		{
			result = ProcessLoginResult::Failure;
		}
		break;
	}
	if (pWait)
		delete pWait;
	return result;
}

bool CSFTPFolderFTP::DisconnectImpl(CFTPConnection* pConnection)
{
	pConnection->SendCommand(L"QUIT", NULL);
	pConnection->Close();
	//delete pConnection;
	pConnection->Release();

	if (pConnection != m_pConnection)
		return false;

	m_pConnection = NULL;

	theApp.UnregisterTimer(m_idTimer);
	m_idTimer = 0;

	if (m_pUser)
	{
		m_pUser->Release();
		m_pUser = NULL;
	}

	m_hWndOwner = NULL;

	OnDisconnect();
	return true;
}

bool CSFTPFolderFTP::DoReceiveSocketCommon(CFTPConnection* pConnection, int& code, CMyStringW& strMsg, CMyStringW& strCommand, CWaitResponseData*& pWait,
	void (*pfnOnLoginFinished)(CFTPConnection* pConnection, void* pParam), void* pParam)
{
	{
		auto resultFTPS = pConnection->ProcessFTPSHandshake();
		switch (resultFTPS)
		{
		case CFTPConnection::FTPSHandshakeResult::Success:
			StartAuth(pConnection);
			return true;
		case CFTPConnection::FTPSHandshakeResult::Failure:
			if (DisconnectImpl(pConnection))
				::MyMessageBoxW(m_hWndOwner, MAKEINTRESOURCEW(IDS_FAILED_TO_CONNECT), NULL, MB_ICONEXCLAMATION);
			return true;
		case CFTPConnection::FTPSHandshakeResult::InProgress:
			return true;
		}
	}

	if (!pConnection->ReceiveMessage(code, strMsg, &pWait, &strCommand))
	{
		DisconnectImpl(pConnection);
		return true;
	}

	if (strCommand.Compare(L"NOOP", true) == 0)
		return true;

	if (pConnection->m_bIsLoggingIn)
	{
		auto r = DoProcessForLogin(pConnection, code, strMsg, strCommand, pWait);
		switch (r)
		{
		case ProcessLoginResult::Finish:
			pConnection->m_bIsLoggingIn = false;
			if (pfnOnLoginFinished)
				pfnOnLoginFinished(pConnection, pParam);
			break;
		case ProcessLoginResult::Cancel:
			DisconnectImpl(pConnection);
			break;
		case ProcessLoginResult::Failure:
			if (DisconnectImpl(pConnection))
				::MyMessageBoxW(m_hWndOwner, MAKEINTRESOURCEW(IDS_FAILED_TO_CONNECT), NULL, MB_ICONEXCLAMATION);
			break;
		}
		return true;
	}
	return false;
}

void CSFTPFolderFTP::DoReceiveSocket()
{
	int code;
	CMyStringW str, strCommand;
	CWaitResponseData* pWait;
	if (DoReceiveSocketCommon(m_pConnection, code, str, strCommand, pWait))
		return;

	switch (code)
	{
		// response for NOOP, SITE, etc.
	case 200:
	{
		if (pWait)
		{
			switch (pWait->nWaitType)
			{
			case CWaitResponseData::WRD_CONFIRM:
			{
				CFTPWaitConfirm* pData = (CFTPWaitConfirm*)pWait;
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
				CFTPWaitConfirm* pData = (CFTPWaitConfirm*)pWait;
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
				CFTPWaitAttrData* pData = (CFTPWaitAttrData*)pWait;
				LPCWSTR lpLine = str;
				LPCWSTR lp = wcsrchr(lpLine, L'\n');
				if (lp)
					str.ReleaseBuffer((DWORD)((DWORD_PTR)lp - (DWORD_PTR)lpLine));
				lpLine = str;
				lp = wcschr(lpLine, L'\n');
				if (lp)
					str.DeleteString(0, (DWORD)((DWORD_PTR)lp - (DWORD_PTR)lpLine));

				//AddFileDataIfNeed(pData->bIsDir ? L"." : pData->strFileName, str);
				//delete pData;
				pData->strResult = str;
				pData->bWaiting = false;
			}
			break;
			case CWaitResponseData::WRD_GETFILEINFO:
			{
				CWaitFileInfoData* pData = (CWaitFileInfoData*)pWait;
				switch (pData->nInfoType)
				{
				case CWaitFileInfoData::fileInfoSize:
				{
					LPCWSTR lp = NULL;
					pData->uliSize.QuadPart = _wcstoi64(str, (wchar_t**)&lp, 10);
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
				CFTPWaitConfirm* pData = (CFTPWaitConfirm*)pWait;
				pData->nCode = code;
				pData->strMsg = str;
				pData->bWaiting = false;
			}
			break;
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
				CFTPWaitAttrData* pData = (CFTPWaitAttrData*)pWait;
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
						strAttr.SetString(lp, (DWORD)(((DWORD_PTR)lp2 - (DWORD_PTR)lp) / sizeof(WCHAR)));
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
				CWaitRenameData* pData = (CWaitRenameData*)pWait;
				pData->bWaiting = false;
				pData->nCode = code;
				pData->strMsg = str;
			}
			break;
			case CWaitResponseData::WRD_MAKEDIR:
			{
				CWaitMakeDirData* pData = (CWaitMakeDirData*)pWait;
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
				CFTPWaitConfirm* pData = (CFTPWaitConfirm*)pWait;
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
				CWaitMakeDirData* pDir = (CWaitMakeDirData*)pWait;
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
							pDir->strRemoteDirectory.SetString(lpw, ((DWORD)((LPCBYTE)lpw2 - (LPCBYTE)lpw)) / sizeof(WCHAR));
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
				CFTPWaitConfirm* pData = (CFTPWaitConfirm*)pWait;
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
				CWaitRenameData* pData = (CWaitRenameData*)pWait;
				//if (pData->pItem)
				//	m_wndListViewServer.RefreshFileItem(pData->iItem);
				//delete pData;
				pData->bSecondary = true;
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
				CWaitMakeDirData* pData = (CWaitMakeDirData*)pWait;
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
				CWaitRenameData* pData = (CWaitRenameData*)pWait;
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
				CFTPWaitAttrData* pData = (CFTPWaitAttrData*)pWait;
				pData->strResult.Empty();
				//pData->bWaiting = false;
				str.Empty();
			}
			break;
			case CWaitResponseData::WRD_CONFIRM:
			{
				CFTPWaitConfirm* pData = (CFTPWaitConfirm*)pWait;
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
				case CWaitResponseData::WRD_FTPWAITATTR:
				{
					CFTPWaitAttrData* pData = (CFTPWaitAttrData*)pWait;
					//delete pData;
					pData->strResult.Empty();
					pData->bWaiting = false;
					str.Empty();
				}
				break;
				case CWaitResponseData::WRD_RENAME:
				{
					CWaitRenameData* pData = (CWaitRenameData*)pWait;
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
					CWaitMakeDirData* pDir = (CWaitMakeDirData*)pWait;
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
					CFTPWaitConfirm* pData = (CFTPWaitConfirm*)pWait;
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

void CSFTPFolderFTP::DoReceiveSocketPassiveControl(CFTPConnection* pConnection, CFTPWaitEstablishPassive* pEstablish)
{
	int code;
	CMyStringW str, strCommand;
	CWaitResponseData* pWait;
	if (DoReceiveSocketCommon(pConnection, code, str, strCommand, pWait,
		[](CFTPConnection* pConnection, void* pParam) {
			if (pParam)
			{
				auto* pEstablish = static_cast<CFTPWaitEstablishPassive*>(pParam);
				pConnection->SendCommand(L"EPSV", NULL, pEstablish);
			}
		}, pEstablish))
	{
		return;
	}
	switch (code)
	{
		case 125:
		case 150:
		{
			if (pWait)
			{
				CFTPWaitPassive* pw = (CFTPWaitPassive*)pWait;
				pw->nWaitType = CWaitResponseData::WRD_ENDPASSIVE;
				if (pw->nWaitFlags == CFTPWaitPassive::WaitFlags::Error)
				{
					delete pw->pPassive;
					pw->pPassive = NULL;
				}
				else
				{
					if (m_bIsFTPS)
					{
						auto r = pw->pPassive->StartHandshake();
						if (r == CFTPSocket::HandshakeResult::Error)
						{
							pw->nWaitFlags = CFTPWaitPassive::WaitFlags::Error;
							delete pw->pPassive;
							pw->pPassive = NULL;
						}
						else if (r == CFTPSocket::HandshakeResult::Waiting)
						{
							pw->nWaitFlags = CFTPWaitPassive::WaitFlags::WaitingForHandshake;
							break;
						}
					}
					CFTPPassiveMessage* pMessage = pw->pMessage;
					pw->nWaitFlags = CFTPWaitPassive::WaitFlags::WaitingForPassiveDone;
					if (pMessage->ConnectionEstablished(pw))
					{
						// do nothing
					}
					else
					{
						pw->nWaitFlags = CFTPWaitPassive::WaitFlags::Error;
					}
				}
				pw->bWaiting = false;
				pConnection->WaitFinishPassive(pw);
			}
		}
		break;
		// response for NOOP, SITE, etc.
		case 200:
			// usually using SendCommandWithType; ignore it
			break;
			// 'Command not implemented, superfluous at this site'
		case 202:
			break;
		case 226:
		case 451:   // transfer aborted
		{
			// response for finishing passive
			if (pWait)
			{
				if (pWait->nWaitType == CWaitResponseData::WRD_STARTPASSIVE || pWait->nWaitType == CWaitResponseData::WRD_ENDPASSIVE)
				{
					CFTPWaitPassive* pData = (CFTPWaitPassive*)pWait;
					pData->bWaiting = false;
					pData->nWaitFlags = (code >= 400 ? CFTPWaitPassive::WaitFlags::Error : CFTPWaitPassive::WaitFlags::Finished);
					pData->nCode = code;
					pData->strMsg = str;
				}
			}
		}
		break;
		case 227:
		{
			if (pWait->nWaitType == CWaitResponseData::WRD_ESTABLISHPASSIVE)
			{
				CFTPSocket* pRet = NULL;
				CMyStringW strIP, strTemp;
				int port;
				LPCWSTR lp, lp2, lp3;
				int iTemp;
				bool bIsInHandshake = false;
				// "227 Entering Passive Mode (n1,n2,n3,n4,p1,p2)"
				lp = wcschr(str, L'(');
				if (lp)
				{
					lp++;
					lp2 = wcschr(lp, L')');
					if (lp2)
					{
						bool bSucceeded = true;
						str.SetString(lp, ((DWORD)((LPCBYTE)lp2 - (LPCBYTE)lp)) / sizeof(WCHAR));
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
							iTemp = (int)wcstol(lp, (wchar_t**)&lp3, 10);
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
							pRet = new CFTPSocket();
							if (!pRet->Connect(port, strIP, AF_INET, SOCK_STREAM))
							{
								delete pRet;
								pRet = NULL;
							}
							pRet->IoControl(FIONBIO, 1);
						}
					}
				}

				CFTPWaitEstablishPassive* pData = (CFTPWaitEstablishPassive*)pWait;
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
			if (pWait->nWaitType == CWaitResponseData::WRD_ESTABLISHPASSIVE)
			{
				CFTPSocket* pRet = NULL;
				// "229 Entering Extended Passive Mode (|||<port>|)"
				// ('|' is a delimiter which may be replaced by any other chars)
				LPCWSTR lp = wcschr(str, L'(');
				if (lp)
				{
					lp++;
					LPCWSTR lp2 = wcschr(lp, L')');
					if (lp2)
					{
						str.SetString(lp, ((DWORD)((LPCBYTE)lp2 - (LPCBYTE)lp)) / sizeof(WCHAR));
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
								port = (int)wcstol(lp, (wchar_t**)&lp3, 10);
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
							pRet = new CFTPSocket();
							if (!pRet->Connect(pConnection->m_socket.GetThisAddrInfo(), port))
							{
								delete pRet;
								pRet = NULL;
							}
							pRet->IoControl(FIONBIO, 1);
						}
					}
				}

				CFTPWaitEstablishPassive* pData = (CFTPWaitEstablishPassive*)pWait;
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
		case 350:
			// do nothing (response to REST)
			break;
		case 421: // Service not available, closing control connection.
		case 450: // Requested file action not taken. File unavailable (e.g., file busy).
		default:
			if (code == 421 || code == 450 || code >= 500)
			{
				if (pWait)
				{
					switch (pWait->nWaitType)
					{
					case CWaitResponseData::WRD_ESTABLISHPASSIVE:
					{
						CFTPWaitEstablishPassive* pData = (CFTPWaitEstablishPassive*)pWait;
						// 'EPSV' is not supported, so we use 'PASV'
						pConnection->SendCommand(L"PASV", NULL, pData);
					}
					break;
					case CWaitResponseData::WRD_STARTPASSIVE:
					case CWaitResponseData::WRD_ENDPASSIVE:
					{
						CFTPWaitPassive* pData = (CFTPWaitPassive*)pWait;
						pData->nWaitFlags = CFTPWaitPassive::WaitFlags::Error;
						pData->nCode = code;
						pData->strMsg = str;
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

void CSFTPFolderFTP::StartAuth(CFTPConnection* pConnection)
{
	BSTR bstrUser;
	if (FAILED(m_pUser->get_UserName(&bstrUser)))
		bstrUser = ::SysAllocString(L"");
	pConnection->SendCommand(L"USER", bstrUser);
	::SysFreeString(bstrUser);
}

CFTPWaitEstablishPassive* CSFTPFolderFTP::StartPassive(CFTPPassiveMessage* pMessage)
{
	auto* pConnection = new CFTPConnection();
	if (!pConnection->Connect(m_nPort, m_strHostName))
	{
		delete pConnection;
		return NULL;
	}
	pConnection->CopyAvailableCommands(m_pConnection);
	CFTPWaitEstablishPassive* pWait = new CFTPWaitEstablishPassive(pMessage, pConnection);
	//m_aWaitResponse.Add(pWait);
	return pWait;
}

CFTPWaitPassive* CSFTPFolderFTP::PassiveStarted(CFTPWaitEstablishPassive* pWait, CFTPSocket* pSocket)
{
	CFTPPassiveMessage* pMessage = pWait->pMessage;
	CFTPWaitPassive* pData = new CFTPWaitPassive(
		CFTPWaitPassive::WaitFlags::WaitingForEstablish,
		pMessage,
		pWait->pConnection,
		pSocket);
	pData->bWaiting = true;
	pWait->bWaiting = false;
	pWait->pConnection = NULL;
	if (pMessage->SendPassive(pData->pConnection, pData))
	{
		//m_aDataSockets.Add(pPassive);
		return pData;
	}
	else
	{
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
		//	pPassive->nWaitFlags = CFTPWaitPassive::WaitFlags::Error;
		//	pPassive->pMessage->EndReceive(NULL);
		//	//break;
		//	return;
		//}
		//if (pPassive->pPassive->IsRemoteClosed())
		//{
		//	pPassive->bWaiting = false;
		//	pPassive->nWaitFlags = CFTPWaitPassive::WaitFlags::Finished;
		//	pPassive->pMessage->EndReceive(NULL);
		//	//break;
		//	return;
		//}
		//if (!pPassive->pPassive || !pPassive->pPassive->CanReceive())
		//	break;
		//::Sleep(0);
	}
}

bool CSFTPFolderFTP::WaitForReceive(bool* pbWaiting)
{
	while (*pbWaiting)
	{
		if (m_pConnection)
		{
			if (m_pConnection->m_socket.IsRemoteClosed())
			{
				DisconnectImpl(m_pConnection);
				return false;
			}
			if (m_pConnection->m_socket.CanReceive())
			{
				DoReceiveSocket();
			}
			if (!m_pConnection || !theApp.MyPumpMessage())
				return false;
		}
		else
			return false;
		::Sleep(1);
	}
	return true;
}

bool CSFTPFolderFTP::WaitForReceiveEstablishPassive(bool* pbWaiting, CFTPWaitEstablishPassive* pPassive)
{
	while (*pbWaiting)
	{
		if (!pPassive->pConnection)
			return false;
		if (pPassive->pConnection->m_socket.IsRemoteClosed())
		{
			DisconnectImpl(pPassive->pConnection);
			return false;
		}
		if (pPassive->pConnection->m_socket.CanReceive())
		{
			DoReceiveSocketPassiveControl(pPassive->pConnection, pPassive);
		}
		if (!theApp.MyPumpMessage())
			return false;
		::Sleep(1);
	}
	return true;
}

bool CSFTPFolderFTP::WaitForReceivePassive(bool* pbWaiting, CFTPWaitPassive* pPassive, DWORD dwTimeoutMilliseconds)
{
	auto dwEnd = GetTickCount() + dwTimeoutMilliseconds;
	while (*pbWaiting)
	{
		if (!pPassive->pPassive)
			return false;
		if (pPassive->nWaitFlags == CFTPWaitPassive::WaitFlags::WaitingForHandshake)
		{
			if (pPassive->pPassive->IsRemoteClosed())
			{
				return false;
			}
			if (pPassive->pPassive->CanReceive())
			{
				auto r = pPassive->pPassive->ContinueHandshake();
				if (r == CFTPSocket::HandshakeResult::Success)
				{
					CFTPPassiveMessage* pMessage = pPassive->pMessage;
					if (pMessage->ConnectionEstablished(pPassive))
					{
						// do nothing
					}
					else
					{
						pPassive->nWaitFlags = CFTPWaitPassive::WaitFlags::Error;
					}
					pPassive->pConnection->WaitFinishPassive(pPassive);
				}
				else if (r == CFTPSocket::HandshakeResult::Error)
				{
					return false;
				}
				else
				{
					::Sleep(1);
					continue;
				}
			}
		}
		else if (pPassive->nWaitFlags == CFTPWaitPassive::WaitFlags::ReceivingData)
		{
			DoReceivePassive(pPassive);
		}
		if (pPassive->pConnection->m_socket.IsRemoteClosed())
		{
			DisconnectImpl(pPassive->pConnection);
			return false;
		}
		if (pPassive->pConnection->m_socket.CanReceive())
		{
			DoReceiveSocketPassiveControl(pPassive->pConnection);
		}
		if (!pPassive->pConnection || !theApp.MyPumpMessage())
			return false;
		// SendingData needs to call Send outside this method
		if (pPassive->nWaitFlags == CFTPWaitPassive::WaitFlags::SendingData ||
			pPassive->nWaitFlags == CFTPWaitPassive::WaitFlags::ReceivingDataExternal ||
			pPassive->nWaitFlags == CFTPWaitPassive::WaitFlags::Finished ||
			pPassive->nWaitFlags == CFTPWaitPassive::WaitFlags::Error)
			break;
		::Sleep(1);
		if (dwTimeoutMilliseconds != INFINITE && static_cast<long>(GetTickCount()) - static_cast<long>(dwEnd) >= 0)
			return false;
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

bool CSFTPFolderFTP::CFTPFileListingHandler::ReceiveFileListing(CFTPSocket* pPassive, bool bMListing)
{
	CFTPFileItem* pItem;
	CMyStringW str2;

	if (!pPassive->ReceiveLine(str2, []() { return theApp.MyPumpMessage2(); }))
		return false;

	if (bMListing)
		pItem = ParseMLSxData(m_pDirectory, str2);
	else
	{
		switch (m_pRoot->m_nServerSystemType)
		{
		case SVT_UNKNOWN:
			pItem = ParseUnixFileList(m_pDirectory, str2);
			if (!pItem)
			{
				pItem = ParseDOSFileList(m_pDirectory, str2, &m_pRoot->m_nYearFollows, &m_pRoot->m_bY2KProblem);
				if (!pItem)
					return true;
			}
			break;
		case SVT_UNIX:
			pItem = ParseUnixFileList(m_pDirectory, str2);
			break;
		case SVT_DOS:
		case SVT_WINDOWS:
			pItem = ParseDOSFileList(m_pDirectory, str2, &m_pRoot->m_nYearFollows, &m_pRoot->m_bY2KProblem);
			break;
		default:
			pItem = NULL;
		}
	}

	if (!pItem)
		return true;
	if (pItem->type == fitypeCurDir)
		pItem->type = fitypeDir;
	if (/*pItem->type == fitypeCurDir || */pItem->type == fitypeParentDir ||
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
