
#include "StdAfx.h"
#include "ShellDLL.h"
#include "SFTPFldr.h"

#include "DragData.h"

static HRESULT __stdcall SFTPResultToHResult(int nStatus)
{
	switch (nStatus)
	{
	case SSH_FX_OK:
		return S_OK;
	case SSH_FX_EOF:
		return S_FALSE;
	case SSH_FX_NO_SUCH_FILE:
		return STG_E_FILENOTFOUND;
	case SSH_FX_PERMISSION_DENIED:
		return STG_E_ACCESSDENIED;
	case SSH_FX_FAILURE:
		return E_FAIL;
	case SSH_FX_BAD_MESSAGE:
		return E_FAIL;
	case SSH_FX_NO_CONNECTION:
	case SSH_FX_CONNECTION_LOST:
		return E_ABORT;
	case SSH_FX_OP_UNSUPPORTED:
		return E_NOTIMPL;
	case SSH_FX_INVALID_HANDLE:
		return STG_E_INVALIDPOINTER;
	case SSH_FX_NO_SUCH_PATH:
		return STG_E_PATHNOTFOUND;
	case SSH_FX_FILE_ALREADY_EXISTS:
		return STG_E_FILEALREADYEXISTS;
	case SSH_FX_WRITE_PROTECT:
		return STG_E_ACCESSDENIED;
	case SSH_FX_NO_MEDIA:
		return STG_E_PATHNOTFOUND;
	case SSH_FX_NO_SPACE_ON_FILESYSTEM:
	case SSH_FX_QUOTA_EXCEEDED:
		return STG_E_MEDIUMFULL;
	case SSH_FX_UNKNOWN_PRINCIPAL:
		return E_INVALIDARG;
	case SSH_FX_LOCK_CONFLICT:
		return STG_E_LOCKVIOLATION;
	case SSH_FX_DIR_NOT_EMPTY:
		return STG_E_ACCESSDENIED;
	case SSH_FX_NOT_A_DIRECTORY:
		return STG_E_FILENOTFOUND;
	case SSH_FX_INVALID_FILENAME:
		return STG_E_INVALIDNAME;
	case SSH_FX_LINK_LOOP:
		return STG_E_INVALIDPARAMETER;
	case SSH_FX_CANNOT_DELETE:
		return STG_E_ACCESSDENIED;
	case SSH_FX_INVALID_PARAMETER:
		return STG_E_INVALIDPARAMETER;
	case SSH_FX_FILE_IS_A_DIRECTORY:
		return STG_E_INVALIDFUNCTION;
	case SSH_FX_BYTE_RANGE_LOCK_CONFLICT:
	case SSH_FX_BYTE_RANGE_LOCK_REFUSED:
		return STG_E_LOCKVIOLATION;
	case SSH_FX_DELETE_PENDING:
		return STG_E_ACCESSDENIED;
	case SSH_FX_FILE_CORRUPT:
		return STG_E_DOCFILECORRUPT;
	case SSH_FX_OWNER_INVALID:
	case SSH_FX_GROUP_INVALID:
		return STG_E_INVALIDPARAMETER;
	case SSH_FX_NO_MATCHING_BYTE_RANGE_LOCK:
		return STG_E_INVALIDPARAMETER;
	default:
		return E_FAIL;
	}
}

static void __stdcall AttributesToStatstg(const CSFTPFileAttribute& attr, DWORD grfStatFlag, ULONG uServerVersion, const CMyStringW& strName, DWORD grfMode, DWORD grfStateBits, STATSTG* pStatstg)
{
	if (!(grfStatFlag & STATFLAG_NONAME))
		pStatstg->pwcsName = DuplicateCoMemString(strName);
	if (attr.dwMask & SSH_FILEXFER_ATTR_SIZE)
		pStatstg->cbSize = attr.uliSize;
	else
		pStatstg->cbSize.QuadPart = 0;
	pStatstg->type = attr.bFileType == SSH_FILEXFER_TYPE_DIRECTORY ? STGTY_STORAGE : STGTY_STREAM;
	if (uServerVersion >= 4)
	{
		if (attr.dwMask & SSH_FILEXFER_ATTR_MODIFYTIME)
			Time64AndNanoToFileTime(attr.dwModifiedTime,
				attr.dwModifiedTimeNano,
				&pStatstg->mtime);
		else
			memset(&pStatstg->mtime, 0, sizeof(pStatstg->mtime));
		if (attr.dwMask & SSH_FILEXFER_ATTR_CREATETIME)
			Time64AndNanoToFileTime(attr.dwCreateTime,
				attr.dwCreateTimeNano,
				&pStatstg->ctime);
		else
			memset(&pStatstg->ctime, 0, sizeof(pStatstg->ctime));
		if (attr.dwMask & SSH_FILEXFER_ATTR_ACCESSTIME)
			Time64AndNanoToFileTime(attr.dwAccessTime,
				attr.dwAccessTimeNano,
				&pStatstg->atime);
		else
			memset(&pStatstg->atime, 0, sizeof(pStatstg->atime));
	}
	else
	{
		if (attr.dwMask & SSH_FILEXFER_ATTR_ACMODTIME)
		{
			TimetToFileTime((time_t)attr.dwModifiedTime, &pStatstg->mtime);
			TimetToFileTime((time_t)attr.dwAccessTime, &pStatstg->atime);
		}
		else
		{
			memset(&pStatstg->mtime, 0, sizeof(pStatstg->mtime));
			memset(&pStatstg->atime, 0, sizeof(pStatstg->atime));
		}
		memset(&pStatstg->ctime, 0, sizeof(pStatstg->ctime));
	}
	if (uServerVersion >= 6)
		pStatstg->grfLocksSupported = LOCK_WRITE | LOCK_EXCLUSIVE;
	else
		pStatstg->grfLocksSupported = 0;
	pStatstg->clsid = CLSID_NULL;
	pStatstg->grfMode = grfMode;
	pStatstg->grfStateBits = grfStateBits;
}

static void __stdcall GetSFTPStatusMessage(int nStatus, LPCWSTR lpszMessage, CMyStringW& rstrMessage)
{
	CMyStringW strMsg;
	rstrMessage.Format(IDS_COMMAND_FAILED, nStatus);
	if (lpszMessage && *lpszMessage)
		strMsg = lpszMessage;
	else if (!strMsg.LoadString(((UINT)nStatus - SSH_FX_EOF) + IDS_SFTP_EOF))
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
	LPCWSTR lpszDirectory)
	: CFTPDirectory(pMallocData, pItemMe, pParent, lpszDirectory)
{
}

CSFTPFolderSFTPDirectory::CSFTPFolderSFTPDirectory(CDelegateMallocData* pMallocData, CFTPDirectoryItem* pItemMe, ITypeInfo* pInfo)
	: CFTPDirectory(pMallocData, pItemMe, pInfo)
{
}

CSFTPFolderSFTPDirectory::~CSFTPFolderSFTPDirectory()
{
}

STDMETHODIMP CSFTPFolderSFTPDirectory::CreateInstance(CFTPDirectoryItem* pItemMe, CFTPDirectoryBase* pParent, LPCWSTR lpszDirectory, CFTPDirectoryBase** ppResult)
{
	CSFTPFolderSFTPDirectory* p = new CSFTPFolderSFTPDirectory(m_pMallocData, pItemMe, pParent, lpszDirectory);
	if (!p)
		return E_OUTOFMEMORY;
	*ppResult = p;
	return S_OK;
}

STDMETHODIMP_(void) CSFTPFolderSFTPDirectory::UpdateItem(CFTPFileItem* pOldItem, LPCWSTR lpszNewItem, LONG lEvent)
{
	CFTPDirectoryBase::UpdateItem(pOldItem, lpszNewItem, lEvent);

	//register CSFTPFolderSFTP* pRoot = static_cast<CSFTPFolderSFTP*>(m_pRoot);
	//switch (lEvent)
	//{
	//case SHCNE_CREATE:
	//case SHCNE_MKDIR:
	//{
	//	CMyStringW strFile(m_strDirectory);
	//	if (((LPCWSTR)strFile)[strFile.GetLength() - 1] != L'/')
	//		strFile += L'/';
	//	strFile += lpszNewItem;
	//	ULONG uMsgID = pRoot->m_pChannel->LStat(strFile);
	//	if (!uMsgID)
	//	{
	//		break;
	//	}
	//	CSFTPWaitAttrData* pAttr = new CSFTPWaitAttrData();
	//	if (!pAttr)
	//	{
	//		break;
	//	}
	//	pAttr->uMsgID = uMsgID;
	//	pAttr->nType = CSFTPWaitAttrData::typeNormal;
	//	pRoot->m_listWaitResponse.Add(pAttr, uMsgID);

	//	while (pAttr->uMsgID)
	//	{
	//		auto hr = pRoot->PumpSocketAndMessage();
	//		if (FAILED(hr) || hr == S_FALSE)
	//		{
	//			pAttr->nResult = SSH_FX_NO_CONNECTION;
	//			break;
	//		}
	//	}

	//	if (pAttr->nResult != SSH_FX_OK)
	//	{
	//		delete pAttr;
	//		break;
	//	}

	//	CSFTPFileData fileData;
	//	memcpy(&fileData.attr, &pAttr->fileData, sizeof(fileData.attr));
	//	fileData.strFileName = lpszNewItem;
	//	CFTPFileItem* pItem = ParseSFTPData(this, pRoot->m_pChannel->GetServerVersion(), &fileData);
	//	delete pAttr;

	//	::EnterCriticalSection(&m_csFiles);
	//	m_aFiles.Add(pItem);
	//	::LeaveCriticalSection(&m_csFiles);
	//}
	//break;
	//case SHCNE_UPDATEITEM:
	//case SHCNE_UPDATEDIR:
	//{
	//	CMyStringW strFile;
	//	if (((LPCWSTR)strFile)[strFile.GetLength() - 1] != L'/')
	//		strFile += L'/';
	//	strFile += pOldItem->strFileName;
	//	ULONG uMsgID = pRoot->m_pChannel->LStat(strFile);
	//	if (!uMsgID)
	//	{
	//		break;
	//	}
	//	CSFTPWaitAttrData* pAttr = new CSFTPWaitAttrData();
	//	if (!pAttr)
	//	{
	//		break;
	//	}
	//	pAttr->uMsgID = uMsgID;
	//	pAttr->nType = CSFTPWaitAttrData::typeNormal;
	//	pRoot->m_listWaitResponse.Add(pAttr, uMsgID);

	//	while (pAttr->uMsgID)
	//	{
	//		auto hr = pRoot->PumpSocketAndMessage();
	//		if (FAILED(hr) || hr == S_FALSE)
	//		{
	//			pAttr->nResult = SSH_FX_NO_CONNECTION;
	//			break;
	//		}
	//	}

	//	if (pAttr->nResult != SSH_FX_OK)
	//	{
	//		delete pAttr;
	//		break;
	//	}

	//	ParseSFTPAttributes(pRoot->m_pChannel->GetServerVersion(), pOldItem, &pAttr->fileData);
	//	delete pAttr;
	//}
	//break;
	//}
}

////////////////////////////////////////////////////////////////////////////////

CSFTPFolderSFTP::CSFTPFolderSFTP(CDelegateMallocData* pMallocData, CFTPDirectoryItem* pItemMe, CEasySFTPFolderRoot* pFolderRoot)
	: CFTPDirectoryRootBase(pMallocData, pItemMe, pFolderRoot)
{
	m_pUser = NULL;
	m_pClient = NULL;
	m_pChannel = NULL;
	m_idTimer = 0;
	m_nPort = 22;
	m_nServerCharset = scsUTF8;
	::InitializeCriticalSection(&m_csSocket);
	::InitializeCriticalSection(&m_csReceive);
	m_bNextLoop = false;
	m_hWndOwnerCache = m_pParent->GetHwndOwnerCache();
	m_dwTransferringCount = 0;
	//::InitializeCriticalSection(&m_csTransferringCount);
}

CSFTPFolderSFTP::~CSFTPFolderSFTP()
{
	Disconnect();
	//::DeleteCriticalSection(&m_csTransferringCount);
	::DeleteCriticalSection(&m_csReceive);
	::DeleteCriticalSection(&m_csSocket);
}

STDMETHODIMP CSFTPFolderSFTP::GetFTPItemUIObjectOf(HWND hWndOwner, CFTPDirectoryBase* pDirectory,
	const CMyPtrArrayT<CFTPFileItem>& aItems, CFTPDataObject** ppObject)
{
	if (!m_pChannel)
		return OLE_E_NOTRUNNING;
	::EnterCriticalSection(&pDirectory->m_csPidlMe);
	CFTPDataObject* pObject = new CFTPDataObject(
		m_pMallocData->pMalloc,
		pDirectory->m_pidlMe,
		m_strHostName,
		this, m_pChannel,
		pDirectory, aItems);
	::LeaveCriticalSection(&pDirectory->m_csPidlMe);
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
	if (((LPCWSTR)strFrom)[dw] != L'/')
		strFrom += L'/';
	if (((LPCWSTR)strTo)[dw] != L'/')
		strTo += L'/';
	strFrom += pItem->strFileName;
	strTo += pszName;

	ULONG uMsgID = m_pChannel->Rename(strFrom, strTo);
	if (!uMsgID)
	{
		if (hWnd)
			::MyMessageBoxW(hWnd, MAKEINTRESOURCEW(IDS_COMMAND_OUTOFMEMORY), NULL, MB_ICONHAND);
		return E_OUTOFMEMORY;
	}

	CSFTPWaitConfirm* pData = new CSFTPWaitConfirm();
	HRESULT hr;
	if (!pData)
	{
		hr = E_OUTOFMEMORY;
		if (hWnd)
			::MyMessageBoxW(hWnd, MAKEINTRESOURCEW(IDS_COMMAND_OUTOFMEMORY), NULL, MB_ICONHAND);
	}
	else
	{
		pData->uMsgID = uMsgID;
		m_listWaitResponse.Add(pData, uMsgID);

		while (pData->uMsgID)
		{
			auto hr = PumpSocketAndMessage();
			if (FAILED(hr))
			{
				delete pData;
				return hr;
			}
			if (hr == S_FALSE)
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
		if (FAILED(hr) && hWnd)
		{
			CMyStringW str;
			GetSFTPStatusMessage(pData->nResult, pData->strMessage, str);
			::MyMessageBoxW(hWnd, str, NULL, MB_ICONHAND);
		}
		delete pData;
	}
	return hr;
}

STDMETHODIMP CSFTPFolderSFTP::RenameFTPItem(LPCWSTR lpszSrcFileName, LPCWSTR lpszNewFileName, CMyStringW* pstrMsg)
{
	HRESULT hr = S_OK;
	CMyStringW strMsg;

	ULONG uMsgID;
	uMsgID = m_pChannel->Stat(lpszSrcFileName);
	if (!uMsgID)
	{
		CMyStringW str, str2;
		str2.LoadString(IDS_COMMAND_UNKNOWN_ERROR);
		str = lpszSrcFileName;
		str += L": ";
		str += str2;
		strMsg = str;
		if (SUCCEEDED(hr))
			hr = E_OUTOFMEMORY;
	}
	if (SUCCEEDED(hr))
	{
		CSFTPWaitAttrData* pAttr = new CSFTPWaitAttrData();
		if (!pAttr)
		{
			CMyStringW str, str2;
			str2.LoadString(IDS_COMMAND_OUTOFMEMORY);
			str = lpszSrcFileName;
			str += L": ";
			str += str2;
			strMsg = str;
			hr = E_OUTOFMEMORY;
		}
		else
		{
			pAttr->uMsgID = uMsgID;
			pAttr->nType = CSFTPWaitAttrData::typeNormal;
			m_listWaitResponse.Add(pAttr, uMsgID);

			while (pAttr->uMsgID)
			{
				auto hr = PumpSocketAndMessage();
				if (FAILED(hr))
				{
					delete pAttr;
					//pDirectory->Release();
					return hr;
				}
				if (hr == S_FALSE)
				{
					pAttr->nResult = SSH_FX_NO_CONNECTION;
					break;
				}
			}

			hr = SFTPResultToHResult(pAttr->nResult);
		}
		if (FAILED(hr))
		{
			CMyStringW str, str2;
			GetSFTPStatusMessage(pAttr->nResult, pAttr->strMessage, str2);
			str = lpszSrcFileName;
			str += L": ";
			str += str2;
			strMsg = str;
			delete pAttr;
		}
		else
		{
			bool bIsDir = (pAttr->fileData.bFileType == SSH_FILEXFER_TYPE_DIRECTORY);
			delete pAttr;

			uMsgID = m_pChannel->Rename(lpszSrcFileName, lpszNewFileName, SSH_FXF_RENAME_OVERWRITE);
			if (!uMsgID)
			{
				CMyStringW str, str2;
				str2.LoadString(IDS_COMMAND_UNKNOWN_ERROR);
				str = lpszSrcFileName;
				str += L": ";
				str += str2;
				strMsg = str;
				if (SUCCEEDED(hr))
					hr = E_OUTOFMEMORY;
			}
			if (SUCCEEDED(hr))
			{
				CSFTPWaitConfirm* pData = new CSFTPWaitConfirm();
				if (!pData)
				{
					hr = E_OUTOFMEMORY;
				}
				else
				{
					pData->uMsgID = uMsgID;
					m_listWaitResponse.Add(pData, uMsgID);

					while (pData->uMsgID)
					{
						auto hr = PumpSocketAndMessage();
						if (FAILED(hr))
						{
							delete pData;
							//pDirectory->Release();
							if (pstrMsg)
								pstrMsg->LoadString(IDS_SFTP_CONNECTION_LOST);
							return hr;
						}
						if (hr == S_FALSE)
						{
							pData->nResult = SSH_FX_NO_CONNECTION;
							break;
						}
					}

					hr = SFTPResultToHResult(pData->nResult);
					if (FAILED(hr))
					{
						CMyStringW str, str2;
						GetSFTPStatusMessage(pData->nResult, pData->strMessage, str2);
						str = lpszSrcFileName;
						str += L": ";
						str += str2;
						strMsg = str;
						delete pData;
					}
					else
					{
						delete pData;

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
						//NotifyUpdate(bIsDir ? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM, strFileFrom, strFileTo);
					}
				}
			}
		}
	}
	if (pstrMsg)
	{
		if (FAILED(hr))
			*pstrMsg = strMsg;
		else
			pstrMsg->Empty();
	}
	return hr;
}

STDMETHODIMP CSFTPFolderSFTP::UpdateFTPItemAttribute(CFTPDirectoryBase* pDirectory, const CServerFileAttrData* pAttr, CMyStringW* pstrMsg)
{
	CSFTPFileAttribute attr;
	CMyStringW strFile = pDirectory->m_strDirectory;
	if ((strFile.operator LPCWSTR())[strFile.GetLength() - 1] != L'/')
		strFile += L'/';
	strFile += pAttr->pItem->strFileName;

	attr.dwMask = 0;
	if (pAttr->wMask & ServerFileAttrDataMask::OwnerGroupName)
	{
		attr.dwMask = SSH_FILEXFER_ATTR_OWNERGROUP;
		attr.strOwner = pAttr->strOwner;
		attr.strGroup = pAttr->strGroup;
	}
	else if (pAttr->wMask & ServerFileAttrDataMask::OwnerGroupID)
	{
		attr.dwMask = SSH_FILEXFER_ATTR_UIDGID;
		attr.uUserID = pAttr->uUID;
		attr.uGroupID = pAttr->uGID;
	}
	if (pAttr->wMask & ServerFileAttrDataMask::Attribute)
	{
		attr.dwMask |= SSH_FILEXFER_ATTR_PERMISSIONS;
		attr.dwPermissions = (DWORD)pAttr->nUnixMode;
	}

	ULONG uMsgID = m_pChannel->SetStat(strFile, attr);

	if (!uMsgID)
	{
		if (pstrMsg)
		{
			CMyStringW str, str2;
			str2.LoadString(IDS_COMMAND_UNKNOWN_ERROR);
			str = strFile;
			str += L": ";
			str += str2;
			*pstrMsg = str;
		}
		return E_OUTOFMEMORY;
	}

	CSFTPWaitSetStat* pWait = new CSFTPWaitSetStat();
	if (!pWait)
	{
		if (pstrMsg)
		{
			CMyStringW str, str2;
			str2.LoadString(IDS_COMMAND_OUTOFMEMORY);
			str = strFile;
			str += L": ";
			str += str2;
			*pstrMsg = str;
		}
		return E_OUTOFMEMORY;
	}
	pWait->uMsgID = uMsgID;
	//pWait->pbResult = &pabResults[i];
	m_listWaitResponse.Add(pWait, uMsgID);

	while (pWait->uMsgID)
	{
		auto hr = PumpSocketAndMessage();
		if (FAILED(hr))
		{
			delete pWait;
			//pDirectory->Release();
			return hr;
		}
		if (hr == S_FALSE)
		{
			pWait->nResult = SSH_FX_NO_CONNECTION;
			break;
		}
	}

	HRESULT hr2 = SFTPResultToHResult(pWait->nResult);
	if (FAILED(hr2))
	{
		if (pstrMsg)
		{
			CMyStringW str, str2;
			GetSFTPStatusMessage(pWait->nResult, pWait->strMessage, str2);
			str = strFile;
			str += L": ";
			str += str2;
			*pstrMsg = str;
		}
	}
	else
	{
		//NotifyUpdate(pAttr->pItem->IsDirectory() ? SHCNE_UPDATEDIR : SHCNE_UPDATEITEM, strFile, NULL);
		pDirectory->UpdateFileAttrs(pAttr->pItem->strFileName, pAttr->pItem->IsDirectory());
	}
	delete pWait;
	return hr2;
}

STDMETHODIMP CSFTPFolderSFTP::CreateFTPDirectory(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszName)
{
	{
		auto hr = IsDirectoryExists(hWndOwner, pDirectory, lpszName);
		if (FAILED(hr))
			return hr;
		if (hr == S_OK)
			return S_OK;
	}

	HRESULT hr = S_OK;
	CMyStringW strFile, strMsg;

	strFile = pDirectory->m_strDirectory;
	if (((LPCWSTR)strFile)[strFile.GetLength() - 1] != L'/')
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
				auto hr = PumpSocketAndMessage();
				if (FAILED(hr))
				{
					delete pData;
					//pDirectory->Release();
					return hr;
				}
				if (hr == S_FALSE)
				{
					pData->nResult = SSH_FX_NO_CONNECTION;
					break;
				}
			}

			HRESULT hr2 = SFTPResultToHResult(pData->nResult);
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
	if (FAILED(hr) && !strMsg.IsEmpty() && hWndOwner)
		::MyMessageBoxW(hWndOwner, strMsg, NULL, MB_ICONHAND);
	return hr;
}

STDMETHODIMP CSFTPFolderSFTP::CreateShortcut(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszName, LPCWSTR lpszLinkTo, bool bHardLink)
{
	HRESULT hr = S_OK;
	CMyStringW strFile, strMsg;

	strFile = pDirectory->m_strDirectory;
	if (((LPCWSTR)strFile)[strFile.GetLength() - 1] != L'/')
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
				auto hr = PumpSocketAndMessage();
				if (FAILED(hr))
				{
					delete pData;
					//pDirectory->Release();
					return hr;
				}
				if (hr == S_FALSE)
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
	if (FAILED(hr) && !strMsg.IsEmpty() && hWndOwner)
		::MyMessageBoxW(hWndOwner, strMsg, NULL, MB_ICONHAND);
	return hr;
}

STDMETHODIMP CSFTPFolderSFTP::CreateFTPItemStream(CFTPDirectoryBase* pDirectory, CFTPFileItem* pItem, IStream** ppStream)
{
	CSFTPStream* pStream = new CSFTPStream(&m_xStreamCounter,
		this,
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
	if (((LPCWSTR)strFile)[strFile.GetLength() - 1] != L'/')
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
				auto hr = PumpSocketAndMessage();
				if (FAILED(hr))
				{
					delete pHandleData;
					//pDirectory->Release();
					return hr;
				}
				if (hr == S_FALSE)
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
							uMsgID = m_pChannel->WriteFile(h, uliOffset, pv, (size_t)nLen);

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
								hr2 = PumpSocketAndMessage();
								if (FAILED(hr2))
								{
									//pDirectory->Release();
									break;
								}
								if (hr2 == S_FALSE)
								{
									hr2 = S_OK;
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
	if (FAILED(hr) && !strMsg.IsEmpty() && hWndOwner)
		::MyMessageBoxW(hWndOwner, strMsg, NULL, MB_ICONHAND);
	return hr;
}

STDMETHODIMP CSFTPFolderSFTP::CreateInstance(CFTPDirectoryItem* pItemMe, CFTPDirectoryBase* pParent, LPCWSTR lpszDirectory, CFTPDirectoryBase** ppResult)
{
	CSFTPFolderSFTPDirectory* p = new CSFTPFolderSFTPDirectory(m_pMallocData, pItemMe, pParent, lpszDirectory);
	if (!p)
		return E_OUTOFMEMORY;
	*ppResult = p;
	return S_OK;
}

bool CSFTPFolderSFTP::Connect(HWND hWnd, LPCWSTR lpszHostName, int nPort, IEasySFTPAuthentication* pUser)
{
	IEasySFTPAuthentication* pMyUser;
	if (pUser)
	{
		m_pUser = pMyUser = pUser;
		pUser->AddRef();
		m_bFirstAuthenticate = false;
	}
	else
	{
		m_pUser = pMyUser = new CAuthentication();
		//// set this flag to show password dialog when connected
		m_bFirstAuthenticate = true;
	}
	{
		CMyStringW str(lpszHostName);
		m_strHostName = str;
	}
	m_nPort = nPort;

	while (true)
	{
		m_bReconnect = false;
		m_hWndOwner = hWnd;

		m_pClient = new CSSH2Client();

		const addrinfo* pai = m_pClient->m_socket.TryConnect(nPort, m_strHostName);
		if (!pai)
		{
			m_pClient->Release();
			m_pClient = NULL;
			pMyUser->Release();
			if (m_pUser == pMyUser)
			{
				m_pUser = NULL;
			}

			if (hWnd)
			{
				CMyStringW str;
				str.Format(IDS_UNKNOWN_HOST, lpszHostName);
				::MyMessageBoxW(hWnd, str, NULL, MB_ICONEXCLAMATION);
			}
			return false;
		}
		//SetStatusText(MAKEINTRESOURCEW(IDS_CONNECTING));
		if (!m_pClient->m_socket.CMySocket::Connect(pai))
		{
			m_pClient->Release();
			m_pClient = NULL;
			pMyUser->Release();
			if (m_pUser == pMyUser)
			{
				m_pUser = NULL;
			}

			if (hWnd)
				::MyMessageBoxW(hWnd, MAKEINTRESOURCEW(IDS_FAILED_TO_CONNECT), NULL, MB_ICONEXCLAMATION);
			return false;
		}

		//pSocket->AsyncSelect(m_hWnd, MY_WM_SOCKETMESSAGE, FD_READ | FD_CLOSE);
		m_phase = Phase::First;
		//m_bFirstAuthenticate = false;

		m_idTimer = theApp.RegisterTimer(KEEP_CONNECTION_TIME_SPAN,
			(PFNEASYSFTPTIMERPROC)KeepConnectionTimerProc, (LPARAM)this);

		while (m_phase < Phase::LoggedIn)
		{
			auto hr = PumpSocketAndMessage();
			if (FAILED(hr))
			{
				Disconnect();
				return false;
			}
			if (hr == S_FALSE)
			{
				Disconnect();
				return false;
			}
			if (m_bReconnect)
			{
				// Disconnect will release m_pUser so keep temporally
				auto* pUserTemp = m_pUser;
				pUserTemp->AddRef();
				Disconnect();
				m_pUser = pUserTemp;
				break;
			}
		}
		if (m_bReconnect)
		{
			continue;
		}
		break;
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

STDMETHODIMP CSFTPFolderSFTP::get_IsUnixServer(VARIANT_BOOL* pbRet)
{
	if (!pbRet)
		return E_POINTER;
	*pbRet = VARIANT_TRUE;
	return S_OK;
}

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

	OnDisconnect();

	//m_strHostName.Empty();
	return S_OK;
}

STDMETHODIMP CSFTPFolderSFTP::OpenFile(CFTPDirectoryBase* pDirectory, LPCWSTR lpszName, DWORD grfMode, HANDLE* phFile, IEasySFTPFile** ppFile)
{
	if (grfMode & (STGM_SHARE_DENY_READ | STGM_SHARE_DENY_WRITE | STGM_SHARE_EXCLUSIVE | STGM_PRIORITY | STGM_CONVERT | STGM_TRANSACTED | STGM_NOSCRATCH | STGM_NOSNAPSHOT | STGM_SIMPLE | STGM_DIRECT_SWMR | STGM_DELETEONRELEASE))
	{
		return STG_E_INVALIDFLAG;
	}

	auto* data = new CSFTPHandleData();
	if (!data)
	{
		return E_OUTOFMEMORY;
	}

	int nMode = 0;
	if (grfMode & STGM_WRITE)
	{
		if (grfMode & STGM_READWRITE)
		{
			delete data;
			return STG_E_INVALIDFUNCTION;
		}
		nMode |= SSH_FXF_WRITE;
	}
	else if (grfMode & STGM_READWRITE)
		nMode |= SSH_FXF_READ | SSH_FXF_WRITE;
	else
		nMode |= SSH_FXF_READ;
	if (grfMode & STGM_CREATE)
	{
		nMode |= SSH_FXF_CREAT | SSH_FXF_TRUNC;
	}
	else
		nMode |= SSH_FXF_EXCL;

	HRESULT hr = S_OK;
	CMyStringW strFile, strMsg;

	strFile = pDirectory->m_strDirectory;
	if (((LPCWSTR)strFile)[strFile.GetLength() - 1] != L'/')
		strFile += L'/';
	strFile += lpszName;

	{
		auto pName = wcsrchr(strFile, L'/');
		if (pName)
			data->strName = pName;
		else
			data->strName = strFile;
	}
	data->grfMode = grfMode;
	data->offset = 0;
	data->dwRefCount = 1;
	data->bIsEOF = false;
	memset(&data->statstg, 0, sizeof(data->statstg));

	IncrementTransferCount();

	ULONG uMsgID = m_pChannel->OpenFile(strFile, nMode);
	if (!uMsgID)
	{
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
				auto hr = PumpSocketAndMessage();
				if (FAILED(hr))
				{
					delete pHandleData;
					//pDirectory->Release();
					return hr;
				}
				if (hr == S_FALSE)
				{
					pHandleData->nResult = SSH_FX_NO_CONNECTION;
					break;
				}
			}

			hr = SFTPResultToHResult(pHandleData->nResult);
			if (SUCCEEDED(hr))
			{
				HSFTPHANDLE h = pHandleData->hSFTPHandle;
				delete pHandleData;
				data->hSFTP = h;

				hr = _StatFile(static_cast<HANDLE>(data), &data->statstg, STATFLAG_NONAME);
				if (SUCCEEDED(hr))
				{
					*phFile = static_cast<HANDLE>(data);

					if (ppFile)
					{
						auto* pItem = GetFileItem(strFile, NULL);
						*ppFile = pItem;
						if (pItem)
							pItem->AddRef();
					}
				}
			}
		}
	}
	if (FAILED(hr))
	{
		delete data;
		DecrementTransferCount();
	}
	return hr;
}

STDMETHODIMP CSFTPFolderSFTP::ReadFile(HANDLE hFile, void* outBuffer, DWORD dwSize, DWORD* pdwRead)
{
	if (pdwRead)
		*pdwRead = 0;
	if (!hFile || !outBuffer)
		return E_POINTER;
	auto data = static_cast<CSFTPHandleData*>(hFile);
	if (data->bIsEOF)
		return S_FALSE;

	::EnterCriticalSection(&m_csSocket);
	ULONG uMsg = m_pChannel->ReadFile(data->hSFTP, data->offset, dwSize);
	if (!uMsg)
	{
		::LeaveCriticalSection(&m_csSocket);
		return E_INVALIDARG;
	}

	CSFTPWaitReadData* pData = new CSFTPWaitReadData();
	pData->uMsgID = uMsg;
	pData->nResult = SSH_FX_OK;
	pData->outBuffer = outBuffer;
	pData->bufferCapacity = dwSize;
	pData->readBytes = 0;
	m_listWaitResponse.Add(pData, uMsg);
	while (pData->uMsgID)
	{
		auto hr = PumpSocketAndMessage();
		if (FAILED(hr))
		{
			delete pData;
			::LeaveCriticalSection(&m_csSocket);
			return hr;
		}
		if (hr == S_FALSE)
		{
			if (pData->uMsgID)
				m_listWaitResponse.Remove(pData->uMsgID);
			delete pData;
			::LeaveCriticalSection(&m_csSocket);
			return E_ABORT;
		}
	}
	::LeaveCriticalSection(&m_csSocket);
	data->offset += pData->readBytes;
	if (pData->nResult != SSH_FX_OK)
	{
		auto hr = SFTPResultToHResult(pData->nResult);
		delete pData;
		return hr;
	}
	if (pData->bIsEOF)
		data->bIsEOF = true;
	if (pdwRead)
		*pdwRead = pData->readBytes;
	auto hr = pData->bufferCapacity > pData->readBytes ? S_FALSE : S_OK;
	delete pData;
	return hr;
}

STDMETHODIMP CSFTPFolderSFTP::WriteFile(HANDLE hFile, const void* inBuffer, DWORD dwSize, DWORD* pdwWritten)
{
	if (pdwWritten)
		*pdwWritten = 0;
	if (!hFile || !inBuffer)
		return E_POINTER;
	auto data = static_cast<CSFTPHandleData*>(hFile);

	::EnterCriticalSection(&m_csSocket);
	ULONG uMsg = m_pChannel->WriteFile(data->hSFTP, data->offset, inBuffer, dwSize);
	if (!uMsg)
	{
		::LeaveCriticalSection(&m_csSocket);
		return E_INVALIDARG;
	}

	CSFTPWaitConfirm* pData = new CSFTPWaitConfirm();
	m_listWaitResponse.Add(pData, uMsg);
	while (pData->uMsgID)
	{
		auto hr = PumpSocketAndMessage();
		if (FAILED(hr))
		{
			delete pData;
			::LeaveCriticalSection(&m_csSocket);
			return hr;
		}
		if (hr == S_FALSE)
		{
			if (pData->uMsgID)
				m_listWaitResponse.Remove(pData->uMsgID);
			delete pData;
			::LeaveCriticalSection(&m_csSocket);
			return E_ABORT;
		}
	}
	::LeaveCriticalSection(&m_csSocket);
	if (pData->nResult != SSH_FX_OK)
	{
		auto hr = SFTPResultToHResult(pData->nResult);
		delete pData;
		return hr;
	}
	data->offset += dwSize;
	if (pdwWritten)
		*pdwWritten = dwSize;
	delete pData;
	return S_OK;
}

STDMETHODIMP CSFTPFolderSFTP::SeekFile(HANDLE hFile, LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer)
{
	auto data = static_cast<CSFTPHandleData*>(hFile);

	switch (dwOrigin)
	{
	case STREAM_SEEK_SET:
		if (liDistanceToMove.QuadPart >= 0)
			data->offset = static_cast<ULONGLONG>(liDistanceToMove.QuadPart);
		break;
	case STREAM_SEEK_END:
	{
		auto l = static_cast<LONGLONG>(data->statstg.cbSize.QuadPart) + liDistanceToMove.QuadPart;
		if (l < 0)
			l = 0;
		data->offset = static_cast<ULONGLONG>(l);
		break;
	}
	case STREAM_SEEK_CUR:
	{
		auto l = static_cast<LONGLONG>(data->offset) + liDistanceToMove.QuadPart;
		if (l < 0)
			l = 0;
		data->offset = static_cast<ULONGLONG>(l);
		break;
	}
	}
	if (data->offset >= data->statstg.cbSize.QuadPart)
	{
		data->offset = data->statstg.cbSize.QuadPart;
		data->bIsEOF = true;
	}
	else
		data->bIsEOF = false;
	if (lpNewFilePointer)
		lpNewFilePointer->QuadPart = data->offset;
	return S_OK;
}

STDMETHODIMP CSFTPFolderSFTP::StatFile(HANDLE hFile, STATSTG* pStatstg, DWORD grfStatFlag)
{
	if (!hFile || !pStatstg)
		return E_POINTER;
	auto data = static_cast<CSFTPHandleData*>(hFile);
	*pStatstg = data->statstg;
	if (!(grfStatFlag & STATFLAG_NONAME))
		pStatstg->pwcsName = DuplicateCoMemString(data->strName);
	return S_OK;
}

STDMETHODIMP CSFTPFolderSFTP::CloseFile(HANDLE hFile)
{
	auto data = static_cast<CSFTPHandleData*>(hFile);
	if (!--data->dwRefCount)
	{
		if (!m_pChannel->CloseHandle(data->hSFTP))
			return E_INVALIDARG;
		delete data;
		DecrementTransferCount();
	}
	return S_OK;
}

STDMETHODIMP CSFTPFolderSFTP::DuplicateFile(HANDLE hFile, HANDLE* phFile)
{
	auto data = static_cast<CSFTPHandleData*>(hFile);
	++data->dwRefCount;
	*phFile = hFile;
	return S_OK;
}

STDMETHODIMP CSFTPFolderSFTP::LockRegion(HANDLE hFile, ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	if (!hFile)
		return E_POINTER;
	if (m_pChannel->GetServerVersion() < 6)
	{
		return STG_E_INVALIDFUNCTION;
	}
	auto data = static_cast<CSFTPHandleData*>(hFile);
	DWORD lock = 0;
	if (dwLockType == LOCK_WRITE)
		lock = SSH_FXF_BLOCK_WRITE | SSH_FXF_BLOCK_DELETE;
	else if (dwLockType == LOCK_EXCLUSIVE)
		lock = SSH_FXF_BLOCK_READ | SSH_FXF_BLOCK_WRITE | SSH_FXF_BLOCK_DELETE;

	::EnterCriticalSection(&m_csSocket);
	ULONG uMsg = m_pChannel->Block(data->hSFTP, libOffset.QuadPart, cb.QuadPart, lock);
	if (!uMsg)
	{
		::LeaveCriticalSection(&m_csSocket);
		return E_INVALIDARG;
	}

	CSFTPWaitConfirm* pData = new CSFTPWaitConfirm();
	m_listWaitResponse.Add(pData, uMsg);
	while (pData->uMsgID)
	{
		auto hr = PumpSocketAndMessage();
		if (FAILED(hr))
		{
			delete pData;
			::LeaveCriticalSection(&m_csSocket);
			return hr;
		}
		if (hr == S_FALSE)
		{
			if (pData->uMsgID)
				m_listWaitResponse.Remove(pData->uMsgID);
			delete pData;
			::LeaveCriticalSection(&m_csSocket);
			return E_ABORT;
		}
	}
	::LeaveCriticalSection(&m_csSocket);
	if (pData->nResult != SSH_FX_OK)
	{
		auto hr = SFTPResultToHResult(pData->nResult);
		delete pData;
		return hr;
	}
	delete pData;
	return S_OK;
}

STDMETHODIMP CSFTPFolderSFTP::UnlockRegion(HANDLE hFile, ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD /*dwLockType*/)
{
	if (!hFile)
		return E_POINTER;
	if (m_pChannel->GetServerVersion() < 6)
	{
		return STG_E_INVALIDFUNCTION;
	}
	auto data = static_cast<CSFTPHandleData*>(hFile);

	::EnterCriticalSection(&m_csSocket);
	ULONG uMsg = m_pChannel->Unblock(data->hSFTP, libOffset.QuadPart, cb.QuadPart);
	if (!uMsg)
	{
		::LeaveCriticalSection(&m_csSocket);
		return E_INVALIDARG;
	}

	CSFTPWaitConfirm* pData = new CSFTPWaitConfirm();
	m_listWaitResponse.Add(pData, uMsg);
	while (pData->uMsgID)
	{
		auto hr = PumpSocketAndMessage();
		if (FAILED(hr))
		{
			delete pData;
			::LeaveCriticalSection(&m_csSocket);
			return hr;
		}
		if (hr == S_FALSE)
		{
			if (pData->uMsgID)
				m_listWaitResponse.Remove(pData->uMsgID);
			delete pData;
			::LeaveCriticalSection(&m_csSocket);
			return E_ABORT;
		}
	}
	::LeaveCriticalSection(&m_csSocket);
	if (pData->nResult != SSH_FX_OK)
	{
		auto hr = SFTPResultToHResult(pData->nResult);
		delete pData;
		return hr;
	}
	delete pData;
	return S_OK;
}

STDMETHODIMP CSFTPFolderSFTP::StatDirectory(CFTPDirectoryBase* pDirectory, DWORD grfMode, STATSTG* pStatstg, DWORD grfStatFlag)
{
	if (!pStatstg)
		return E_POINTER;
	::EnterCriticalSection(&m_csSocket);
	ULONG uMsg = m_pChannel->Stat(pDirectory->m_strDirectory);
	if (!uMsg)
	{
		::LeaveCriticalSection(&m_csSocket);
		return E_INVALIDARG;
	}

	CSFTPWaitAttrData* pAttr = new CSFTPWaitAttrData();
	pAttr->uMsgID = uMsg;
	pAttr->nType = CSFTPWaitAttrData::typeNormal;
	pAttr->nResult = SSH_FX_OK;
	m_listWaitResponse.Add(pAttr, uMsg);
	while (pAttr->uMsgID)
	{
		auto hr = PumpSocketAndMessage();
		if (FAILED(hr))
		{
			delete pAttr;
			::LeaveCriticalSection(&m_csSocket);
			return hr;
		}
		if (hr == S_FALSE)
		{
			if (pAttr->uMsgID)
				m_listWaitResponse.Remove(pAttr->uMsgID);
			delete pAttr;
			::LeaveCriticalSection(&m_csSocket);
			return E_ABORT;
		}
	}
	::LeaveCriticalSection(&m_csSocket);
	if (pAttr->nResult != SSH_FX_OK)
	{
		auto hr = SFTPResultToHResult(pAttr->nResult);
		delete pAttr;
		return hr;
	}

	auto uServerVersion = m_pChannel->GetServerVersion();
	CMyStringW strName;
	if (pDirectory->m_strDirectory == L"/")
	{
		strName = pDirectory->m_strDirectory;
	}
	else
	{
		auto pName = wcsrchr(pDirectory->m_strDirectory, L'/');
		if (pName)
			strName = pName + 1;
		else
			strName = pDirectory->m_strDirectory;
	}

	AttributesToStatstg(pAttr->fileData, grfStatFlag, uServerVersion, strName, grfMode, pDirectory->m_grfStateBits, pStatstg);
	pStatstg->clsid = pDirectory->m_clsidThis;
	return S_OK;
}

STDMETHODIMP CSFTPFolderSFTP::SetFileTime(LPCWSTR lpszFileName, const FILETIME* pctime, const FILETIME* patime, const FILETIME* pmtime)
{
	if (!pctime && !patime && !pmtime)
		return S_OK;

	CSFTPFileAttribute attr;

	attr.dwMask = 0;
	if (m_pChannel->GetServerVersion() >= 4)
	{
		if (pctime)
		{
			attr.dwMask |= SSH_FILEXFER_ATTR_CREATETIME;
			FileTimeToTime64AndNano(&attr.dwCreateTime, &attr.dwCreateTimeNano, pctime);
		}
		if (pmtime)
		{
			attr.dwMask |= SSH_FILEXFER_ATTR_MODIFYTIME;
			FileTimeToTime64AndNano(&attr.dwModifiedTime, &attr.dwModifiedTimeNano, pmtime);
		}
		if (patime)
		{
			attr.dwMask |= SSH_FILEXFER_ATTR_ACCESSTIME;
			FileTimeToTime64AndNano(&attr.dwAccessTime, &attr.dwAccessTimeNano, patime);
		}
	}
	else
	{
		if (!patime && !pmtime)
			return S_OK;
		attr.dwMask = SSH_FILEXFER_ATTR_ACMODTIME;
		if (patime)
			FileTimeToTimet(reinterpret_cast<time_t*>(&attr.dwAccessTime), patime);
		if (pmtime)
			FileTimeToTimet(reinterpret_cast<time_t*>(&attr.dwModifiedTime), pmtime);
	}

	ULONG uMsgID = m_pChannel->SetStat(lpszFileName, attr);

	if (!uMsgID)
	{
		return E_OUTOFMEMORY;
	}

	CSFTPWaitSetStat* pWait = new CSFTPWaitSetStat();
	if (!pWait)
	{
		return E_OUTOFMEMORY;
	}
	pWait->uMsgID = uMsgID;
	//pWait->pbResult = &pabResults[i];
	m_listWaitResponse.Add(pWait, uMsgID);

	while (pWait->uMsgID)
	{
		auto hr = PumpSocketAndMessage();
		if (FAILED(hr))
		{
			delete pWait;
			return hr;
		}
		if (hr == S_FALSE)
		{
			pWait->nResult = SSH_FX_NO_CONNECTION;
			break;
		}
	}

	HRESULT hr = SFTPResultToHResult(pWait->nResult);
	if (FAILED(hr))
	{
		delete pWait;
		return hr;
	}
	else
	{
		CFTPDirectoryBase* pDirectory = NULL;
		auto* pItem = GetFileItem(lpszFileName, &pDirectory);
		if (pItem && pDirectory)
		{
			pDirectory->UpdateFileAttrs(pItem->strFileName, pItem->IsDirectory());
			pDirectory->Release();
		}
		return S_OK;
	}
}

void CSFTPFolderSFTP::IncrementTransferCount()
{
	::InterlockedIncrement((LONG*)&m_dwTransferringCount);
}

void CSFTPFolderSFTP::DecrementTransferCount()
{
	::InterlockedDecrement((LONG*)&m_dwTransferringCount);
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
		getnameinfo(psa, (socklen_t)nLen, str.GetBufferA(NI_MAXHOST + 1), NI_MAXHOST + 1, NULL, 0, NI_NUMERICHOST);
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
		if (m_phase >= Phase::Authenticating)
		{
			str.LoadString(IDS_SSH_KEY_TYPE);
			pDialog->m_strInfo += str;
			pDialog->m_strInfo += L": ";
			size_t keyLen = 0;
			int nType = 0;
			libssh2_session_hostkey(m_pClient->GetSession(), &keyLen, &nType);
			switch (nType)
			{
			case LIBSSH2_HOSTKEY_TYPE_RSA:
				pDialog->m_strInfo += L"rsa";
				break;
			case LIBSSH2_HOSTKEY_TYPE_DSS:
				pDialog->m_strInfo += L"dss";
				break;
			case LIBSSH2_HOSTKEY_TYPE_ECDSA_256:
				pDialog->m_strInfo += L"ecdsa-256";
				break;
			case LIBSSH2_HOSTKEY_TYPE_ECDSA_384:
				pDialog->m_strInfo += L"ecdsa-384";
				break;
			case LIBSSH2_HOSTKEY_TYPE_ECDSA_521:
				pDialog->m_strInfo += L"ecdsa-521";
				break;
			case LIBSSH2_HOSTKEY_TYPE_ED25519:
				pDialog->m_strInfo += L"ed25519";
				break;
			default:
				pDialog->m_strInfo += L"unknown";
				break;
			}
			pDialog->m_strInfo += L"\r\n";
			str.LoadString(IDS_SSH_KEX);
			pDialog->m_strInfo += str;
			pDialog->m_strInfo += L": ";
			pDialog->m_strInfo += libssh2_session_methods(m_pClient->GetSession(), LIBSSH2_METHOD_KEX);
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
	pData->pDirectory = pDirectory;
	m_listWaitResponse.Add(pData, uMsg);
	pDirectory->AddRef();
	while (pData->nStep)
	{
		auto hr = PumpSocketAndMessage();
		if (FAILED(hr))
		{
			if (m_pChannel && pData->hSFTPHandle)
				m_pChannel->CloseHandle(pData->hSFTPHandle);
			delete pData;
			pDirectory->Release();
			::LeaveCriticalSection(&m_csSocket);
			return false;
		}
		if (hr == S_FALSE)
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
	if (m_pClient)
	{
		::EnterCriticalSection(&m_csSocket);
		if (!m_pChannel)
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
			auto hr = PumpSocketAndMessage();
			if (FAILED(hr))
			{
				delete pAttr;
				::LeaveCriticalSection(&m_csSocket);
				return false;
			}
			if (hr == S_FALSE)
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
	if (strPath.IsEmpty() || ((LPCWSTR)strPath)[strPath.GetLength() - 1] != L'/')
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
		auto hr = PumpSocketAndMessage();
		if (FAILED(hr))
		{
			delete pAttr;
			::LeaveCriticalSection(&m_csSocket);
			return false;
		}
		if (hr == S_FALSE)
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
		pItem = ParseSFTPData(pDirectory, m_pChannel->GetServerVersion(), &data);
		if (pAttr->fileData.aExAttrs)
			free(pAttr->fileData.aExAttrs);
	}
	delete pAttr;
	return pItem;
}

////////////////////////////////////////////////////////////////////////////////

void CALLBACK CSFTPFolderSFTP::KeepConnectionTimerProc(UINT_PTR idEvent, LPARAM lParam)
{
	CSFTPFolderSFTP* pThis = (CSFTPFolderSFTP*)lParam;

	pThis->m_pClient->SendKeepAlive();
	// send 'ignore'/no-op message to keep connection
	// pThis->m_pClient->m_socket.SendPacket(SSH2_MSG_IGNORE, NULL, 0);
}

HRESULT CSFTPFolderSFTP::PumpSocketAndMessage(DWORD dwWaitTime)
{
	if (dwWaitTime == 0xFFFFFFFF)
		dwWaitTime = WAIT_RECEIVE_TIME;
	if (m_bNextLoop || m_pClient->m_socket.CanReceive(dwWaitTime))
	{
		DoReceiveSocket();
		if (!m_pClient || !theApp.MyPumpMessage())
		{
			return E_ABORT;
		}
		return S_OK;
	}
	else
	{
		return S_FALSE;
	}
}

void CSFTPFolderSFTP::DoReceiveSocket()
{
	//while (true)
	//{
	m_bNextLoop = false;
	if (m_pClient)
		OnSFTPSocketReceive(m_pClient->m_socket.CanReceive());
	//	if (!m_pClient || !m_pClient->m_socket.CanReceive())
	//	{
	//		break;
	//	}
	//}
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

HRESULT CSFTPFolderSFTP::DoDeleteFileOrDirectory(HWND hWndOwner, CMyStringArrayW& astrMsgs, bool bIsDirectory, LPCWSTR lpszFile, CFTPDirectoryBase* pDirectory)
{
	HRESULT hr;
	ULONG uMsgID = bIsDirectory ? m_pChannel->RemoveRemoteDirectory(lpszFile) : m_pChannel->Remove(lpszFile);
	if (!uMsgID)
	{
		CMyStringW str, str2;
		str2.LoadString(IDS_COMMAND_UNKNOWN_ERROR);
		str = lpszFile;
		str += L": ";
		str += str2;
		astrMsgs.Add(str);
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
				auto hr = PumpSocketAndMessage();
				if (FAILED(hr))
				{
					delete pData;
					//pDirectory->Release();
					return hr;
				}
				if (hr == S_FALSE)
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
				CMyStringW str, str2;
				GetSFTPStatusMessage(pData->nResult, pData->strMessage, str2);
				str = lpszFile;
				str += L": ";
				str += str2;
				astrMsgs.Add(str);
			}
			else if (pDirectory != NULL)
			{
				auto pName = wcsrchr(lpszFile, L'/');
				pName = pName != NULL ? pName + 1 : lpszFile;
				pDirectory->UpdateRemoveFile(pName, bIsDirectory);
			}
			delete pData;
		}
	}
	return hr;
}

HRESULT CSFTPFolderSFTP::_StatFile(HANDLE hFile, STATSTG* pStatstg, DWORD grfStatFlag)
{
	if (!hFile || !pStatstg)
		return E_POINTER;
	auto data = static_cast<CSFTPHandleData*>(hFile);

	::EnterCriticalSection(&m_csSocket);
	ULONG uMsg = m_pChannel->FStat(data->hSFTP);
	if (!uMsg)
	{
		::LeaveCriticalSection(&m_csSocket);
		return E_INVALIDARG;
	}

	CSFTPWaitAttrData* pAttr = new CSFTPWaitAttrData();
	pAttr->uMsgID = uMsg;
	pAttr->nType = CSFTPWaitAttrData::typeNormal;
	pAttr->nResult = SSH_FX_OK;
	m_listWaitResponse.Add(pAttr, uMsg);
	while (pAttr->uMsgID)
	{
		auto hr = PumpSocketAndMessage();
		if (FAILED(hr))
		{
			delete pAttr;
			::LeaveCriticalSection(&m_csSocket);
			return hr;
		}
		if (hr == S_FALSE)
		{
			if (pAttr->uMsgID)
				m_listWaitResponse.Remove(pAttr->uMsgID);
			delete pAttr;
			::LeaveCriticalSection(&m_csSocket);
			return E_ABORT;
		}
	}
	::LeaveCriticalSection(&m_csSocket);
	if (pAttr->nResult != SSH_FX_OK)
	{
		auto hr = SFTPResultToHResult(pAttr->nResult);
		delete pAttr;
		return hr;
	}

	auto uServerVersion = m_pChannel->GetServerVersion();

	AttributesToStatstg(pAttr->fileData, grfStatFlag, uServerVersion, data->strName, data->grfMode, 0, pStatstg);
	return S_OK;
}

void CSFTPFolderSFTP::OnSFTPSocketReceive(bool isSocketReceived)
{
	::EnterCriticalSection(&m_csReceive);
	UINT u = _OnSFTPSocketReceiveThreadUnsafe(isSocketReceived);
	::LeaveCriticalSection(&m_csReceive);
	if (u)
	{
		auto hWndOwner = m_hWndOwner;
		Disconnect();
		if (u != (UINT)-1 && hWndOwner)
			::MyMessageBoxW(hWndOwner, MAKEINTRESOURCEW(u), NULL, MB_ICONEXCLAMATION);
	}
}

UINT CSFTPFolderSFTP::_OnSFTPSocketReceiveThreadUnsafe(bool isSocketReceived)
{
	if (m_phase == Phase::First)
	{
		//m_bFirstMessageReceived = true;
		if (!m_pClient->OnFirstReceive())
			return IDS_FAILED_TO_CONNECT;
		m_phase = Phase::Handshake;
	}

	if (m_phase == Phase::Handshake)
	{
		auto r = m_pClient->OnHandshake(this);
		if (r < 0)
		{
			return IDS_FAILED_TO_CONNECT;
		}
		if (r == 0)
		{
			m_bNextLoop = true;
			return 0;
		}
		m_phase = Phase::Authenticating;
	}

	if (m_phase == Phase::Authenticating)
	{
		// m_bFirstAuthenticate が true のときは none 認証を行い、
		// 実際に可能な認証方法を得る
		EasySFTPAuthenticationMode mode;
		m_pUser->get_Type(&mode);
		auto r = m_pClient->Authenticate(m_pUser);
		if (r == AuthReturnType::Again)
			return 0;
		else if (r == AuthReturnType::Error)
		{
			char* msg;
			libssh2_session_last_error(m_pClient->GetSession(), &msg, NULL, 0);
#ifdef _DEBUG
			{
				CMyStringW strMsg = msg;
				CMyStringW str;
				str.Format(L"[libssh2] error %d: %s\n", libssh2_session_last_errno(m_pClient->GetSession()), strMsg.operator LPCWSTR());
				OutputDebugString(str);
			}
#endif
			if (m_pClient->CanRetryAuthenticate())
			{
				m_bFirstAuthenticate = false;
				m_bNextLoop = true;
				return 0;
			}
			else if (m_bFirstAuthenticate)
			{
				auto lpAuthTypes = m_pClient->AvailableAuthTypes();
				if (DoRetryAuthentication(lpAuthTypes, m_bFirstAuthenticate) > 0)
				{
					m_pClient->EndAuthenticate();
					m_bFirstAuthenticate = false;
					m_bNextLoop = true;
					// need to reconnect to avoid 'change of username or service not allowed' error
					m_bReconnect = true;
					OutputDebugStringW(L"[EasySFTP] reconnecting for authentication\n");
					return 0;
				}
				m_pClient->EndAuthenticate();
				return (UINT)-1;
			}
			m_pClient->EndAuthenticate();
			return IDS_FAILED_TO_CONNECT;
		}
		else
		{
			m_pClient->EndAuthenticate();

			m_phase = Phase::Authenticated;
			m_pChannel = new CSFTPChannel(this);
		}
	}
	if (m_phase == Phase::Authenticated)
	{
		auto r = m_pChannel->InitializeChannel(m_pClient->GetSession());
		if (r == SSHChannelReturnType::Error)
			return IDS_FAILED_TO_CONNECT;
		if (r == SSHChannelReturnType::Again)
		{
			m_bNextLoop = true;
			return 0;
		}
		m_phase = Phase::WaitingLoggedIn;
	}
	if (m_pChannel->Process(isSocketReceived))
		m_bNextLoop = true;
	m_pChannel->FlushAllMessages();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

static void __stdcall FingerPrintToString(const BYTE* pFingerPrint, size_t nLen, CMyStringW& string)
{
	WCHAR szBuffer[3];
	string.Empty();
	while (nLen--)
	{
		_snwprintf_s(szBuffer, 3, L"%02x", (UINT)*pFingerPrint++);
		szBuffer[2] = 0;
		string += szBuffer;
		if (nLen)
			string += L':';
	}
}

static void __stdcall _MergeFingerPrint(const CMyStringW& strHostName, const BYTE* pFingerPrint, size_t nLen)
{
	CGeneralSettings settings;
	CMyPtrArrayT<CEasySFTPHostSetting> aHostSettings;
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
				BYTE* pb = (BYTE*)realloc(pPrint->pFingerPrint, nLen);
				if (!pb)
					break;
				pPrint->pFingerPrint = pb;
				pPrint->nFingerPrintLen = nLen;
			}
			memcpy(pPrint->pFingerPrint, pFingerPrint, nLen);
			break;
		}
		pPrint = NULL;
	}
	if (!pPrint)
	{
		pPrint = new CKnownFingerPrint();
		pPrint->pFingerPrint = (BYTE*)malloc(nLen);
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
				str.Format(IDS_FINGER_PRINT_MISMATCH, (LPCWSTR)m_strHostName,
					(LPCWSTR)strPrint2, (LPCWSTR)strPrint);
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
	str.Format(IDS_NEW_FINGER_PRINT, (LPCWSTR)m_strHostName,
		(LPCWSTR)strPrint);
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

void CSFTPFolderSFTP::ChannelOpenFailure(CSSHChannel* pChannel, int nReason)
{
	if (pChannel != (CSSHChannel*)m_pChannel)
		return;
	Disconnect();
	//::MyMessageBoxW(m_hWndOwner, MAKEINTRESOURCEW(IDS_FAILED_TO_CONNECT), NULL, MB_ICONEXCLAMATION);
}

void CSFTPFolderSFTP::ChannelError(CSSHChannel* pChannel, int nReason)
{
	if (pChannel != (CSSHChannel*)m_pChannel)
		return;
	Disconnect();
}

void CSFTPFolderSFTP::ChannelOpened(CSSHChannel* pChannel)
{
	if (pChannel != (CSSHChannel*)m_pChannel)
		return;

	//m_pClient->NoMoreSessions();
	m_pChannel->Startup();
}

void CSFTPFolderSFTP::ChannelClosed(CSSHChannel* pChannel)
{
	if (pChannel != (CSSHChannel*)m_pChannel)
		return;
	Disconnect();
}

void CSFTPFolderSFTP::ChannelExitStatus(CSSHChannel* pChannel, int nExitCode)
{
	if (pChannel != (CSSHChannel*)m_pChannel)
		return;
}

void CSFTPFolderSFTP::ChannelConfirm(CSSHChannel* pChannel, bool bSucceeded, int nReason)
{
	if (pChannel != (CSSHChannel*)m_pChannel)
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
	m_phase = Phase::LoggedIn;
	pChannel->SetCharset((ServerCharset)m_nServerCharset);
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
			CSFTPWaitAttrData* pData = (CSFTPWaitAttrData*)p;
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
			CSFTPWaitDirectoryData* pData = (CSFTPWaitDirectoryData*)p;
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
			CSFTPWaitConfirm* pData = (CSFTPWaitConfirm*)p;
			pData->uMsgID = 0;
			pData->nResult = nStatus;
			pData->strMessage = strMessage;
			m_listWaitResponse.Remove(uMsgID);
		}
		break;
		case CWaitResponseData::WRD_SETSTAT:
		{
			CSFTPWaitSetStat* pData = (CSFTPWaitSetStat*)p;
			pData->uMsgID = 0;
			//*(pData->pbResult) = (nStatus == SSH_FX_OK);
			pData->nResult = nStatus;
			pData->strMessage = strMessage;
			m_listWaitResponse.Remove(uMsgID);
		}
		break;
		case CWaitResponseData::WRD_READDATA:
		{
			CSFTPWaitReadData* pData = static_cast<CSFTPWaitReadData*>(p);
			pData->uMsgID = 0;
			//*(pData->pbResult) = (nStatus == SSH_FX_OK);
			pData->readBytes = 0;
			pData->bIsEOF = true;
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
			CSFTPWaitDirectoryData* pData = (CSFTPWaitDirectoryData*)p;
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
			CSFTPWaitFileHandle* pData = (CSFTPWaitFileHandle*)p;
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
static void __stdcall _ParseSFTPFileName(CFTPDirectoryBase* pThis, const CSFTPFileData* aFiles, int nCount)
{
	auto* pRoot = pThis->GetRoot();
	while (nCount--)
	{
		CFTPFileItem* pItem;
		pItem = ParseSFTPData(pThis, pRoot->GetServerVersion(), aFiles++);
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
						p->strRealPath = pThis->m_strDirectory;
						if ((p->strRealPath.operator LPCWSTR())[p->strRealPath.GetLength() - 1] != L'/')
							p->strRealPath += L'/';
						p->strRealPath += pItem->strFileName;
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
			CSFTPWaitAttrData* pData = (CSFTPWaitAttrData*)p;
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
					if (((LPCWSTR)pData->strFileName)[pData->strFileName.GetLength() - 1] != L'/')
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
			CSFTPWaitDirectoryData* pData = (CSFTPWaitDirectoryData*)p;

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
					if (str.IsEmpty() || ((LPCWSTR)str)[str.GetLength() - 1] != L'/')
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
	if (m_listWaitResponse.IsItemKey(pMsg->uMsgID))
	{
		CWaitResponseData* pWait = m_listWaitResponse.GetItem(pMsg->uMsgID);
		m_listWaitResponse.Remove(pWait->uMsgID);
		pWait->uMsgID = 0;
		if (pWait->nWaitType == CWaitResponseData::WRD_READDATA)
		{
			CSFTPWaitReadData* pData = (CSFTPWaitReadData*)pWait;
			DWORD dwCopyBytes = pData->bufferCapacity > nLen ? static_cast<DWORD>(nLen) : pData->bufferCapacity;
			memcpy(pData->outBuffer, pvData, dwCopyBytes);
			pData->readBytes = dwCopyBytes;
			if (pbEOF && *pbEOF)
				pData->bIsEOF = true;
		}
	}
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
			CSFTPWaitAttrData* pData = (CSFTPWaitAttrData*)pWait;
			pData->nResult = SSH_FX_OK;
			switch (pData->nType)
			{
			case CSFTPWaitAttrData::typeNormal:
				memcpy(&pData->fileData, &attrs, sizeof(CSFTPFileAttribute));
				if (attrs.dwMask & SSH_FILEXFER_ATTR_EXTENDED)
				{
					pData->fileData.aExAttrs = (CSFTPFileAttributeExtendedData*)malloc(sizeof(CSFTPFileAttributeExtendedData) * attrs.nExAttrCount);
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
					CFTPFileItem* pItem = ParseSFTPData(NULL, pChannel->GetServerVersion(), &file);
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
