/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 SFTPChan.cpp - implementations of CSFTPChannel
 */

#include "StdAfx.h"
#include "SFTPChan.h"

#include "ShellDLL.h"
#include "Array.h"
#include "Convert.h"
#include "Func.h"

static char* __stdcall GetBufferString(CExBuffer& buffer)
{
	ULONG uSize;
	void* pv;
	char* pszRet;
	if (!buffer.GetAndSkipCE(uSize) ||
		!(pv = buffer.GetCurrentBufferPermanentAndSkip((size_t) uSize)))
		return NULL;
	pszRet = (char*) malloc((size_t) uSize + sizeof(char));
	if (!pszRet)
		return NULL;
	memcpy(pszRet, pv, (size_t) uSize);
	*((char*)(&((BYTE*) pszRet)[uSize])) = 0;
	return pszRet;
}

static bool __stdcall ParseSFTPAttributes(ULONG uVersion, CExBuffer& buffer, CSFTPFileAttribute& attr)
{
	if (!buffer.GetAndSkipCE(attr.dwMask))
		return false;
	if (uVersion >= 4)
	{
		if (!buffer.GetAndSkip(attr.bFileType))
			return false;
	}
	else
		attr.bFileType = SSH_FILEXFER_TYPE_REGULAR;
	if (attr.dwMask & SSH_FILEXFER_ATTR_SIZE)
	{
		if (!buffer.GetAndSkipCE(attr.uliSize.QuadPart))
			return false;
	}
	if (uVersion >= 6 && (attr.dwMask & SSH_FILEXFER_ATTR_ALLOCATION_SIZE))
	{
		if (!buffer.GetAndSkipCE(attr.uliAllocationSize.QuadPart))
			return false;
	}
	if (uVersion <= 3 && (attr.dwMask & SSH_FILEXFER_ATTR_UIDGID))
	{
		if (!buffer.GetAndSkipCE((ULONG&) attr.uUserID) ||
			!buffer.GetAndSkipCE((ULONG&) attr.uGroupID))
			return false;
	}
	if (uVersion >= 4 && (attr.dwMask & SSH_FILEXFER_ATTR_OWNERGROUP))
	{
		ULONG uOSize, uGSize;
		char* pszO, * pszG;
		if (!buffer.GetAndSkipCE(uOSize) ||
			!(pszO = (char*) buffer.GetCurrentBufferPermanentAndSkip((size_t) uOSize)) ||
			!buffer.GetAndSkipCE(uGSize) ||
			!(pszG = (char*) buffer.GetCurrentBufferPermanentAndSkip((size_t) uGSize)))
			return false;
		attr.strOwner.SetUTF8String((LPCBYTE) pszO, uOSize);
		attr.strGroup.SetUTF8String((LPCBYTE) pszG, uGSize);
	}
	if (attr.dwMask & SSH_FILEXFER_ATTR_PERMISSIONS)
	{
		if (!buffer.GetAndSkipCE(attr.dwPermissions))
			return false;
		if (uVersion <= 3)
		{
			switch (attr.dwPermissions & S_IFMT)
			{
				case S_IFREG:
					attr.bFileType = SSH_FILEXFER_TYPE_REGULAR;
					break;
				case S_IFLNK:
					attr.bFileType = SSH_FILEXFER_TYPE_SYMLINK;
					break;
				case S_IFBLK:
					attr.bFileType = SSH_FILEXFER_TYPE_BLOCK_DEVICE;
					break;
				case S_IFDIR:
					attr.bFileType = SSH_FILEXFER_TYPE_DIRECTORY;
					break;
				case S_IFCHR:
					attr.bFileType = SSH_FILEXFER_TYPE_CHAR_DEVICE;
					break;
				case S_IFSOCK:
					attr.bFileType = SSH_FILEXFER_TYPE_SOCKET;
					break;
				case S_IFIFO:
					attr.bFileType = SSH_FILEXFER_TYPE_FIFO;
					break;
				default:
					attr.bFileType = SSH_FILEXFER_TYPE_SPECIAL;
					break;
			}
		}
	}
	if (attr.dwMask & SSH_FILEXFER_ATTR_ACMODTIME)
	{
		// SSH_FILEXFER_ATTR_ACMODTIME == SSH_FILEXFER_ATTR_ACCESSTIME (for version 4)
		if (uVersion >= 4)
		{
			if (!buffer.GetAndSkipCE(attr.dwAccessTime))
				return false;
			if (attr.dwMask & SSH_FILEXFER_ATTR_SUBSECOND_TIMES)
			{
				if (!buffer.GetAndSkipCE(attr.dwAccessTimeNano))
					return false;
			}
			//else
			//	attr.dwAccessTimeNano = 0;
		}
		else
		{
			ULONG dwA, dwM;
			if (!buffer.GetAndSkipCE(dwA) ||
				!buffer.GetAndSkipCE(dwM))
				return false;
			attr.dwAccessTime = dwA;
			//attr.dwAccessTimeNano = 0;
			attr.dwModifiedTime = dwM;
			//attr.dwModifiedTimeNano = 0;
		}
	}
	if (uVersion >= 4)
	{
		if (attr.dwMask & SSH_FILEXFER_ATTR_CREATETIME)
		{
			if (!buffer.GetAndSkipCE(attr.dwCreateTime))
				return false;
			if (attr.dwMask & SSH_FILEXFER_ATTR_SUBSECOND_TIMES)
			{
				if (!buffer.GetAndSkipCE(attr.dwCreateTimeNano))
					return false;
			}
			//else
			//	attr.dwCreateTimeNano = 0;
		}
		if (attr.dwMask & SSH_FILEXFER_ATTR_MODIFYTIME)
		{
			if (!buffer.GetAndSkipCE(attr.dwModifiedTime))
				return false;
			if (attr.dwMask & SSH_FILEXFER_ATTR_SUBSECOND_TIMES)
			{
				if (!buffer.GetAndSkipCE(attr.dwModifiedTimeNano))
					return false;
			}
			//else
			//	attr.dwModifiedTimeNano = 0;
		}
		if (uVersion >= 6 && (attr.dwMask & SSH_FILEXFER_ATTR_CTIME))
		{
			if (!buffer.GetAndSkipCE(attr.dwAttrChangedTime))
				return false;
			if (attr.dwMask & SSH_FILEXFER_ATTR_SUBSECOND_TIMES)
			{
				if (!buffer.GetAndSkipCE(attr.dwAttrChangedTimeNano))
					return false;
			}
			//else
			//	attr.dwAttrChangedTimeNano = 0;
		}
		if (attr.dwMask & SSH_FILEXFER_ATTR_ACL)
		{
			ULONG uACLSize;
			void* pvACL;
			if (!buffer.GetAndSkipCE(uACLSize) ||
				!(pvACL = buffer.GetCurrentBufferPermanentAndSkip((size_t) uACLSize)))
				return false;
			attr.bufferACL.SetDataToBuffer(pvACL, (size_t) uACLSize);
		}
		if (uVersion >= 5)
		{
			if (attr.dwMask & SSH_FILEXFER_ATTR_BITS)
			{
				if (!buffer.GetAndSkipCE(attr.dwAttributesEx))
					return false;
				if (uVersion >= 6)
				{
					if (!buffer.GetAndSkipCE(attr.dwValidAttributesEx))
						return false;
					attr.dwAttributesEx &= attr.dwValidAttributesEx;
				}
			}
			if (uVersion >= 6)
			{
				if (attr.dwMask & SSH_FILEXFER_ATTR_TEXT_HINT)
				{
					if (!buffer.GetAndSkip(attr.bFileType))
						return false;
				}
				if (attr.dwMask & SSH_FILEXFER_ATTR_MIME_TYPE)
				{
					ULONG uMSize;
					char* pszM;
					if (!buffer.GetAndSkipCE(uMSize) ||
						!(pszM = (char*) buffer.GetCurrentBufferPermanentAndSkip((size_t) uMSize)))
						return false;
					attr.strMIMEType.SetUTF8String((LPCBYTE) pszM, uMSize);
				}
				if (attr.dwMask & SSH_FILEXFER_ATTR_LINK_COUNT)
				{
					if (!buffer.GetAndSkipCE(attr.dwLinkCount))
						return false;
				}
				if (attr.dwMask & SSH_FILEXFER_ATTR_UNTRANSLATED_NAME)
				{
					ULONG uUSize;
					char* pszU;
					if (!buffer.GetAndSkipCE(uUSize) ||
						!(pszU = (char*) buffer.GetCurrentBufferPermanentAndSkip((size_t) uUSize)))
						return false;
					attr.strUntranslatedName.SetUTF8String((LPCBYTE) pszU, uUSize);
				}
			}
		}
	}
	if (attr.dwMask & SSH_FILEXFER_ATTR_EXTENDED)
	{
		if (!buffer.GetAndSkipCE((ULONG&) attr.nExAttrCount))
			return false;
		attr.aExAttrs = (CSFTPFileAttributeExtendedData*)
			malloc(sizeof(CSFTPFileAttributeExtendedData) * attr.nExAttrCount);
		for (int i = 0; i < attr.nExAttrCount; i++)
		{
			char* pType;
			char* pData = NULL;
			pType = GetBufferString(buffer);
			if (pType)
				pData = GetBufferString(buffer);
			if (!pData)
			{
				if (pType)
					free(pType);
				while (i--)
				{
					free(attr.aExAttrs[i].pszType);
					free(attr.aExAttrs[i].pszData);
				}
				free(attr.aExAttrs);
				attr.nExAttrCount = 0;
				return false;
			}
			attr.aExAttrs[i].pszType = pType;
			attr.aExAttrs[i].pszData = pData;
		}
	}
	else
	{
		attr.nExAttrCount = 0;
		attr.aExAttrs = NULL;
	}
	return true;
}

static void __stdcall SetSFTPAttributesToBuffer(ULONG uVersion, CExBuffer& buffer, const CSFTPFileAttribute& attr)
{
	DWORD dwMask = attr.dwMask;
	if ((dwMask & SSH_FILEXFER_ATTR_UNTRANSLATED_NAME) &&
		!(attr.dwAttributesEx & SSH_FILEXFER_ATTR_FLAGS_TRANSLATION_ERR))
		dwMask ^= SSH_FILEXFER_ATTR_UNTRANSLATED_NAME;
	buffer.AppendToBufferCE(dwMask);
	if (uVersion >= 4)
		buffer.AppendToBuffer(attr.bFileType);
	if (attr.dwMask & SSH_FILEXFER_ATTR_SIZE)
		buffer.AppendToBufferCE(attr.uliSize.QuadPart);
	if (uVersion >= 6 && (dwMask & SSH_FILEXFER_ATTR_ALLOCATION_SIZE))
		buffer.AppendToBufferCE(attr.uliAllocationSize.QuadPart);
	if (uVersion <= 3 && (dwMask & SSH_FILEXFER_ATTR_UIDGID))
	{
		buffer.AppendToBufferCE((ULONG) attr.uUserID);
		buffer.AppendToBufferCE((ULONG) attr.uGroupID);
	}
	if (uVersion >= 4 && (dwMask & SSH_FILEXFER_ATTR_OWNERGROUP))
	{
		CMyStringW str(attr.strOwner);
		size_t dw;
		LPCBYTE lpb;
		lpb = str.AllocUTF8String(&dw);
		buffer.AppendToBufferWithLenCE(lpb, dw);
		str = attr.strGroup;
		lpb = str.AllocUTF8String(&dw);
		buffer.AppendToBufferWithLenCE(lpb, dw);
	}
	if (dwMask & SSH_FILEXFER_ATTR_PERMISSIONS)
		buffer.AppendToBufferCE((ULONG) attr.dwPermissions);
	if (dwMask & SSH_FILEXFER_ATTR_ACMODTIME)
	{
		if (uVersion >= 4)
		{
			buffer.AppendToBufferCE(attr.dwAccessTime);
			if (attr.dwMask & SSH_FILEXFER_ATTR_SUBSECOND_TIMES)
				buffer.AppendToBufferCE(attr.dwAccessTimeNano);
		}
		else
		{
			buffer.AppendToBufferCE((ULONG) attr.dwAccessTime);
			buffer.AppendToBufferCE((ULONG) attr.dwModifiedTime);
		}
	}
	if (uVersion >= 4)
	{
		if (dwMask & SSH_FILEXFER_ATTR_CREATETIME)
		{
			buffer.AppendToBufferCE(attr.dwCreateTime);
			if (attr.dwMask & SSH_FILEXFER_ATTR_SUBSECOND_TIMES)
				buffer.AppendToBufferCE(attr.dwCreateTimeNano);
		}
		if (dwMask & SSH_FILEXFER_ATTR_MODIFYTIME)
		{
			buffer.AppendToBufferCE(attr.dwModifiedTime);
			if (attr.dwMask & SSH_FILEXFER_ATTR_SUBSECOND_TIMES)
				buffer.AppendToBufferCE(attr.dwModifiedTimeNano);
		}
		if (uVersion >= 6 && (dwMask & SSH_FILEXFER_ATTR_CTIME))
		{
			buffer.AppendToBufferCE(attr.dwAttrChangedTime);
			if (attr.dwMask & SSH_FILEXFER_ATTR_SUBSECOND_TIMES)
				buffer.AppendToBufferCE(attr.dwAttrChangedTimeNano);
		}
		if (dwMask & SSH_FILEXFER_ATTR_ACL)
		{
			buffer.AppendToBufferWithLenCE(attr.bufferACL, attr.bufferACL.GetLength());
		}
		if (uVersion >= 5)
		{
			if (dwMask & SSH_FILEXFER_ATTR_BITS)
			{
				buffer.AppendToBufferCE(attr.dwAttributesEx);
				if (uVersion >= 6)
					buffer.AppendToBufferCE(attr.dwValidAttributesEx);
			}
			if (uVersion >= 6)
			{
				if (dwMask & SSH_FILEXFER_ATTR_TEXT_HINT)
					buffer.AppendToBuffer(attr.bTextHint);
				if (dwMask & SSH_FILEXFER_ATTR_MIME_TYPE)
				{
					CMyStringW str(attr.strMIMEType);
					size_t dw;
					LPCBYTE lpb;
					lpb = str.AllocUTF8String(&dw);
					buffer.AppendToBufferWithLenCE(lpb, dw);
				}
				if (dwMask & SSH_FILEXFER_ATTR_LINK_COUNT)
					buffer.AppendToBufferCE(attr.dwLinkCount);
				if (dwMask & SSH_FILEXFER_ATTR_UNTRANSLATED_NAME)
				{
					CMyStringW str(attr.strUntranslatedName);
					size_t dw;
					LPCBYTE lpb;
					lpb = str.AllocUTF8String(&dw);
					buffer.AppendToBufferWithLenCE(lpb, dw);
				}
			}
		}
	}
	if (dwMask & SSH_FILEXFER_ATTR_EXTENDED)
	{
		buffer.AppendToBufferCE((ULONG) attr.nExAttrCount);
		for (int i = 0; i < attr.nExAttrCount; i++)
		{
			buffer.AppendToBufferWithLenCE(attr.aExAttrs[i].pszType);
			buffer.AppendToBufferWithLenCE(attr.aExAttrs[i].pszData);
		}
	}
}

static void __stdcall FreeSFTPFileAttributeExtendedData(int nCount, CSFTPFileAttributeExtendedData* pData)
{
	register CSFTPFileAttributeExtendedData* p = pData;
	while (nCount--)
	{
		free(p->pszType);
		free(p->pszData);
	}
	if (pData)
		free(pData);
}

////////////////////////////////////////////////////////////////////////////////

#define SFTPHANDLE_SIGNATURE 0x19fc4fad

struct CSFTPHandle
{
	DWORD dwSignature;
	size_t nSize;
	//BYTE data[1];
	BYTE* data() const { return (BYTE*) (this + 1); }
	HSFTPHANDLE handle() const
		{ return (HSFTPHANDLE) this; }
	static inline CSFTPHandle* __stdcall FromHandle(HSFTPHANDLE h)
	{
		auto p = (CSFTPHandle*)h;
		if (p->dwSignature != SFTPHANDLE_SIGNATURE)
			return NULL;
		return p;
	}
};

typedef CMyPtrArrayT<CSFTPMessage> CSFTPMessageArray;
typedef CMyPtrArrayT<CExBuffer> CSFTPMessageQueue;

CSFTPChannel::CSFTPChannel(CSFTPChannelListener* pListener)
	: CSSHChannel(pListener)
	, m_bStartingSubsystem(false)
	, m_bInitializing(false)
	, m_bSymLinkBug(false)
	, m_uMsgID(0)
	, m_uCurMsgID(0)
	, m_uServerVersion(0)
	, m_pMsgTemp(NULL)
{
	m_pvMsgCache = new CSFTPMessageArray();
	m_pvMsgQueue = new CSFTPMessageQueue();
	::InitializeCriticalSection(&m_csMessages);
	::InitializeCriticalSection(&m_csQueue);
	::InitializeCriticalSection(&m_csBuffer);
	::InitializeCriticalSection(&m_csListeners);
}

CSFTPChannel::~CSFTPChannel()
{
	::DeleteCriticalSection(&m_csMessages);
	::DeleteCriticalSection(&m_csBuffer);
	::DeleteCriticalSection(&m_csQueue);
	::DeleteCriticalSection(&m_csListeners);
	if (m_pMsgTemp)
		free(m_pMsgTemp);
	{
		register CSFTPMessageQueue* pa = (CSFTPMessageQueue*) m_pvMsgQueue;
		for (int i = 0; i < pa->GetCount(); i++)
			delete pa->GetItem(i);
		delete pa;
	}
	{
		register CSFTPMessageArray* pa = (CSFTPMessageArray*) m_pvMsgCache;
		for (int i = 0; i < pa->GetCount(); i++)
			free(pa->GetItem(i));
		delete pa;
	}
}

bool CSFTPChannel::SendSFTPChannelData(const void* pvBuffer, size_t nLen)
{
	{
		auto str = CMyStringW(L"Sending SFTP message... (type = %d, msgid = %lu, size = %lu)",
			(int) *((const BYTE*) pvBuffer),
			(nLen >= 5 ? ConvertEndian(*((const ULONG*) (((const BYTE*) pvBuffer) + 1))) : (ULONG) 0),
			(ULONG) nLen);
		theApp.Log(EasySFTPLogLevel::Debug, str, S_OK);
	}
	CExBuffer buf;

	buf.AppendToBufferWithLenCE(pvBuffer, nLen);
	return SendChannelData(buf, buf.GetLength());
}

ULONG CSFTPChannel::GetNextMsgID(BYTE bSentMsg, LPCSTR lpszAddInfo)
{
	::EnterCriticalSection(&m_csMessages);
	register ULONG uMsgID = m_uMsgID++;
	if (uMsgID == 0)
		uMsgID = (m_uMsgID = 2) - 1;

	CSFTPMessage* pMsg = (CSFTPMessage*) malloc(sizeof(CSFTPMessage));
	pMsg->uMsgID = uMsgID;
	pMsg->lpszAddInfo = lpszAddInfo;
	pMsg->bSentMsg = bSentMsg;
	pMsg->bRecvMsg = 0;
	((CSFTPMessageArray*) m_pvMsgCache)->Add(pMsg);
	::LeaveCriticalSection(&m_csMessages);
	return uMsgID;
}

CSFTPMessage* CSFTPChannel::PopMessage(ULONG uMsgID)
{
	register CSFTPMessage* pRet = NULL;
	::EnterCriticalSection(&m_csMessages);
	register int c = ((CSFTPMessageArray*) m_pvMsgCache)->GetCount();
	for (int i = 0; i < c; i++)
	{
		CSFTPMessage* p = ((CSFTPMessageArray*) m_pvMsgCache)->GetItem(i);
		if (p->uMsgID == uMsgID)
		{
			if (m_pMsgTemp)
				free(m_pMsgTemp);
			m_pMsgTemp = p;
			((CSFTPMessageArray*) m_pvMsgCache)->RemoveItem(i);
			pRet = p;
			break;
		}
	}
	::LeaveCriticalSection(&m_csMessages);
	return pRet;
}

//bool CSFTPChannel::FlushMessage(ULONG uMsgID, CSFTPChannelListener* pListener)
//{
//	::EnterCriticalSection(&m_csQueue);
//	register int c = ((CSFTPMessageQueue*) m_pvMsgQueue)->GetCount();
//	for (int i = 0; i < c; i++)
//	{
//		CExBuffer* pBuffer = ((CSFTPMessageQueue*) m_pvMsgQueue)->GetItem(i);
//		BYTE bMsg;
//		if (pBuffer->GetAndSkip(bMsg))
//		{
//			if (bMsg != SSH_FXP_INIT)
//			{
//				ULONG u;
//				if (pBuffer->GetAndSkipCE(u))
//				{
//					if (u == uMsgID)
//					{
//						((CSFTPMessageQueue*) m_pvMsgQueue)->RemoveItem(i);
//						::LeaveCriticalSection(&m_csQueue);
//						CSFTPChannelListener* p = (CSFTPChannelListener*) m_pListener;
//						m_pListener = pListener;
//						pBuffer->SkipPosition(-(LONG)(sizeof(u) + sizeof(bMsg)));
//						bool bRet = ProcessQueuedBuffer(*pBuffer);
//						m_pListener = p;
//						delete pBuffer;
//						return bRet;
//					}
//					else
//						pBuffer->SkipPosition(-((long) (sizeof(u) + sizeof(bMsg))));
//				}
//				else
//					pBuffer->SkipPosition(-((long) sizeof(bMsg)));
//			}
//			else
//				pBuffer->SkipPosition(-((long) sizeof(bMsg)));
//		}
//	}
//	::LeaveCriticalSection(&m_csQueue);
//	return false;
//}

bool CSFTPChannel::FlushAllMessages(CSFTPChannelListener* pListener)
{
	bool bRet = true;
	CSFTPChannelListener* p;

	::EnterCriticalSection(&m_csListeners);
	if (pListener)
		p = pListener;
	else
		p = (CSFTPChannelListener*) m_pListener;
	::LeaveCriticalSection(&m_csListeners);

	::EnterCriticalSection(&m_csQueue);
	CSFTPMessageQueue arr;
	arr.CopyArray(*(CSFTPMessageQueue*) m_pvMsgQueue);
	((CSFTPMessageQueue*) m_pvMsgQueue)->RemoveAll();
	::LeaveCriticalSection(&m_csQueue);

	register int c = arr.GetCount();
	for (int i = 0; i < c; i++)
	{
		CExBuffer* pBuffer = arr.GetItem(i);
		bool bRet2 = ProcessQueuedBuffer(p, *pBuffer);
		delete pBuffer;
		bRet = bRet && bRet2;
	}
	return bRet;
}

bool CSFTPChannel::HasQueue() const
{
	return (static_cast<CSFTPMessageQueue*>(m_pvMsgQueue)->GetCount() > 0);
}

bool CSFTPChannel::RegisterMessageListener(ULONG uMsgID, CSFTPChannelListener* pListener)
{
	::EnterCriticalSection(&m_csListeners);
	for (int i = 0; i < m_aListeners.GetCount(); i++)
	{
		CListenerData* pData = m_aListeners.GetItemPtr(i);
		if (pData->uMsgID == uMsgID)
		{
			//pListener->AddRef();
			//pData->pListener->Release();
			pData->pListener = pListener;
			::LeaveCriticalSection(&m_csListeners);
			return true;
		}
	}
	CListenerData data;
	data.uMsgID = uMsgID;
	data.pListener = pListener;
	//pListener->AddRef();
	m_aListeners.Add(data);
	::LeaveCriticalSection(&m_csListeners);
	return true;
}

void CSFTPChannel::UnregisterMessageListener(ULONG uMsgID, CSFTPChannelListener* pListener)
{
	::EnterCriticalSection(&m_csListeners);
	for (int i = 0; i < m_aListeners.GetCount(); i++)
	{
		CListenerData* pData = m_aListeners.GetItemPtr(i);
		if (pData->uMsgID == uMsgID)
		{
			if (pData->pListener == pListener)
				m_aListeners.RemoveItem(i);
			break;
		}
	}
	::LeaveCriticalSection(&m_csListeners);
}

bool CSFTPChannel::Startup()
{
	return SendProcessStartup("subsystem", "sftp", 4);
}

bool CSFTPChannel::InitSFTP()
{
	CExBuffer buf;

	m_bInitializing = true;
	buf.AppendToBuffer((BYTE) SSH_FXP_INIT);
	buf.AppendToBufferCE((DWORD) SFTP_VERSION);
	return SendSFTPChannelData(buf, buf.GetLength());
}

void CSFTPChannel::SetCharset(ServerCharset nCharset)
{
	if (m_uServerVersion >= 4)
		nCharset = scsUTF8;
	m_nCharset = nCharset;
}

static void* __stdcall AppendStringToBufferWithLenCE(CExBuffer& buf, CMyStringW& string, ServerCharset nCharset)
{
	LPCVOID lpv;
	size_t dwLen;
	switch (nCharset)
	{
		case scsUTF8:
			lpv = string.AllocUTF8String(&dwLen);
			return buf.AppendToBufferWithLenCE(lpv, dwLen);
		case scsShiftJIS:
			dwLen = string.GetLengthA();
			lpv = (LPCSTR) string;
			return buf.AppendToBufferWithLenCE(lpv, dwLen);
		case scsEUC:
		{
			register void* pv;
			dwLen = string.GetLengthA();
			lpv = (LPCSTR) string;
			buf.AppendToBufferCE(dwLen);
			pv = buf.AppendToBuffer(NULL, (size_t) dwLen);
			memcpy(pv, lpv, (size_t) dwLen);
			::ShiftJISToEUCString((LPSTR) pv, dwLen);
			return ((BYTE*) pv) - sizeof(DWORD);
		}
		default:
			return NULL;
	}
}

ULONG CSFTPChannel::OpenDirectory(LPCWSTR lpszPath)
{
	CMyStringW strPath(lpszPath);
	CExBuffer buf;
	m_uCurMsgID = GetNextMsgID(SSH_FXP_OPENDIR);

	buf.AppendToBuffer((BYTE) SSH_FXP_OPENDIR);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	AppendStringToBufferWithLenCE(buf, strPath, m_nCharset);
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG CSFTPChannel::ReadDirectory(HSFTPHANDLE hSFTP)
{
	CExBuffer buf;
	CSFTPHandle* pHandle;
	pHandle = CSFTPHandle::FromHandle(hSFTP);
	if (!pHandle)
		return 0;

	m_uCurMsgID = GetNextMsgID(SSH_FXP_READDIR);

	buf.AppendToBuffer((BYTE) SSH_FXP_READDIR);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	buf.AppendToBufferWithLenCE(pHandle->data(), pHandle->nSize);
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;

	return m_uCurMsgID;
}

ULONG CSFTPChannel::CloseHandle(HSFTPHANDLE hSFTP)
{
	CExBuffer buf;
	CSFTPHandle* pHandle;
	pHandle = CSFTPHandle::FromHandle(hSFTP);
	if (!pHandle)
		return 0;

	m_uCurMsgID = GetNextMsgID(SSH_FXP_CLOSE);

	buf.AppendToBuffer((BYTE) SSH_FXP_CLOSE);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	buf.AppendToBufferWithLenCE(pHandle->data(), pHandle->nSize);
	if (!SendSFTPChannelData(buf, buf.GetLength()))
	{
		free(pHandle);
		return 0;
	}
	free(pHandle);
	return m_uCurMsgID;
}

ULONG CSFTPChannel::OpenFile(LPCWSTR lpszPath, int nMode)
{
	if (m_uServerVersion >= 5)
		return 0;

	CSFTPFileAttribute attr;
	attr.dwMask = 0;
	return OpenFile(lpszPath, nMode, attr);
}

ULONG CSFTPChannel::OpenFile(LPCWSTR lpszPath, int nMode, const CSFTPFileAttribute& attr)
{
	if (m_uServerVersion >= 5)
		return 0;

	CExBuffer buf;
	CMyStringW strPath(lpszPath);
	m_uCurMsgID = GetNextMsgID(SSH_FXP_OPEN);

	buf.AppendToBuffer((BYTE) SSH_FXP_OPEN);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	AppendStringToBufferWithLenCE(buf, strPath, m_nCharset);
	buf.AppendToBufferCE((DWORD) nMode);
	SetSFTPAttributesToBuffer(m_uServerVersion, buf, attr);
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG CSFTPChannel::OpenFileEx(LPCWSTR lpszPath, DWORD dwDesiredAccess, DWORD dwMode)
{
	if (m_uServerVersion < 5)
		return 0;

	CSFTPFileAttribute attr;
	attr.dwMask = 0;
	return OpenFileEx(lpszPath, dwDesiredAccess, dwMode, attr);
}

ULONG CSFTPChannel::OpenFileEx(LPCWSTR lpszPath, DWORD dwDesiredAccess, DWORD dwMode, const CSFTPFileAttribute& attr)
{
	if (m_uServerVersion < 5)
		return 0;

	CExBuffer buf;
	CMyStringW strPath(lpszPath);
	m_uCurMsgID = GetNextMsgID(SSH_FXP_OPEN);

	buf.AppendToBuffer((BYTE) SSH_FXP_OPEN);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	AppendStringToBufferWithLenCE(buf, strPath, m_nCharset);
	buf.AppendToBufferCE(dwDesiredAccess);
	buf.AppendToBufferCE(dwMode);
	SetSFTPAttributesToBuffer(m_uServerVersion, buf, attr);
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG CSFTPChannel::ReadFile(HSFTPHANDLE hSFTP, ULONGLONG uliOffset, size_t nLen)
{
	CExBuffer buf;
	CSFTPHandle* pHandle;
	pHandle = CSFTPHandle::FromHandle(hSFTP);
	if (!pHandle)
		return 0;

	m_uCurMsgID = GetNextMsgID(SSH_FXP_READ);

	{
		CMyStringW str;
		str.Format(L"CSFTPChannel::ReadFile : handle = 0x%p, uliOffset = %I64u, nLen = %lu",
			hSFTP, uliOffset, (ULONG) nLen);
		theApp.Log(EasySFTPLogLevel::Debug, str, S_OK);
	}
	buf.AppendToBuffer((BYTE) SSH_FXP_READ);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	buf.AppendToBufferWithLenCE(pHandle->data(), pHandle->nSize);
	buf.AppendToBufferCE(uliOffset);
	buf.AppendToBufferCE((ULONG) nLen);
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG CSFTPChannel::WriteFile(HSFTPHANDLE hSFTP, ULONGLONG uliOffset, const void* pvData, size_t nLen)
{
	CExBuffer buf;
	CSFTPHandle* pHandle;
	pHandle = CSFTPHandle::FromHandle(hSFTP);
	if (!pHandle)
		return 0;

	m_uCurMsgID = GetNextMsgID(SSH_FXP_WRITE);

	buf.AppendToBuffer((BYTE) SSH_FXP_WRITE);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	buf.AppendToBufferWithLenCE(pHandle->data(), pHandle->nSize);
	buf.AppendToBufferCE(uliOffset);
	buf.AppendToBufferWithLenCE(pvData, nLen);
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG CSFTPChannel::Remove(LPCWSTR lpszPath)
{
	CMyStringW strPath(lpszPath);
	CExBuffer buf;
	m_uCurMsgID = GetNextMsgID(SSH_FXP_REMOVE);

	buf.AppendToBuffer((BYTE) SSH_FXP_REMOVE);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	AppendStringToBufferWithLenCE(buf, strPath, m_nCharset);
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG CSFTPChannel::CreateRemoteDirectory(LPCWSTR lpszPath)
{
	CSFTPFileAttribute attr;
	attr.dwMask = 0;
	return CreateRemoteDirectory(lpszPath, attr);
}

ULONG CSFTPChannel::CreateRemoteDirectory(LPCWSTR lpszPath, const CSFTPFileAttribute& attr)
{
	CMyStringW strPath(lpszPath);
	CExBuffer buf;
	m_uCurMsgID = GetNextMsgID(SSH_FXP_MKDIR);

	buf.AppendToBuffer((BYTE) SSH_FXP_MKDIR);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	AppendStringToBufferWithLenCE(buf, strPath, m_nCharset);
	SetSFTPAttributesToBuffer(m_uServerVersion, buf, attr);
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG CSFTPChannel::RemoveRemoteDirectory(LPCWSTR lpszPath)
{
	CMyStringW strPath(lpszPath);
	CExBuffer buf;
	m_uCurMsgID = GetNextMsgID(SSH_FXP_RMDIR);

	buf.AppendToBuffer((BYTE) SSH_FXP_RMDIR);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	AppendStringToBufferWithLenCE(buf, strPath, m_nCharset);
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG CSFTPChannel::RealPath(LPCWSTR lpszPath)
{
	CMyStringW strPath(lpszPath);
	CExBuffer buf;
	m_uCurMsgID = GetNextMsgID(SSH_FXP_REALPATH);

	buf.AppendToBuffer((BYTE) SSH_FXP_REALPATH);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	AppendStringToBufferWithLenCE(buf, strPath, m_nCharset);
	// we need not append control byte even if the version is 6 or later
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG WINAPIV CSFTPChannel::RealPath(LPCWSTR lpszPath, BYTE bControl, int nComposePathCount, ...)
{
	if (m_uServerVersion < 6)
		return 0;

	CMyStringW strPath(lpszPath);
	CExBuffer buf;
	m_uCurMsgID = GetNextMsgID(SSH_FXP_REALPATH);

	buf.AppendToBuffer((BYTE) SSH_FXP_REALPATH);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	AppendStringToBufferWithLenCE(buf, strPath, m_nCharset);
	buf.AppendToBuffer(bControl);

	va_list va;
	va_start(va, nComposePathCount);
	while (nComposePathCount--)
	{
		strPath = va_arg(va, LPCWSTR);
		AppendStringToBufferWithLenCE(buf, strPath, m_nCharset);
	}
	va_end(va);

	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG CSFTPChannel::Stat(LPCWSTR lpszPath, DWORD dwMask)
{
	CMyStringW strPath(lpszPath);
	CExBuffer buf;
	BYTE bMsg = (m_uServerVersion == 0 ? SSH_FXP_STAT_VERSION_0 : SSH_FXP_STAT);
	m_uCurMsgID = GetNextMsgID(bMsg);

	buf.AppendToBuffer(bMsg);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	AppendStringToBufferWithLenCE(buf, strPath, m_nCharset);
	if (m_uServerVersion >= 4)
		buf.AppendToBufferCE(dwMask);
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG CSFTPChannel::LStat(LPCWSTR lpszPath, DWORD dwMask)
{
	if (m_uServerVersion == 0)
		return Stat(lpszPath);

	CMyStringW strPath(lpszPath);
	CExBuffer buf;
	m_uCurMsgID = GetNextMsgID(SSH_FXP_LSTAT);

	buf.AppendToBuffer((BYTE) SSH_FXP_LSTAT);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	AppendStringToBufferWithLenCE(buf, strPath, m_nCharset);
	if (m_uServerVersion >= 4)
		buf.AppendToBufferCE(dwMask);
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG CSFTPChannel::FStat(HSFTPHANDLE hSFTP, DWORD dwMask)
{
	CExBuffer buf;
	CSFTPHandle* pHandle;

	pHandle = CSFTPHandle::FromHandle(hSFTP);
	if (!pHandle)
		return 0;

	m_uCurMsgID = GetNextMsgID(SSH_FXP_FSTAT);

	buf.AppendToBuffer((BYTE) SSH_FXP_FSTAT);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	buf.AppendToBufferWithLenCE(pHandle->data(), pHandle->nSize);
	if (m_uServerVersion >= 4)
		buf.AppendToBufferCE(dwMask);
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG CSFTPChannel::SetStat(LPCWSTR lpszPath, const CSFTPFileAttribute& attr)
{
	CMyStringW strPath(lpszPath);
	CExBuffer buf;
	m_uCurMsgID = GetNextMsgID(SSH_FXP_SETSTAT);

	buf.AppendToBuffer((BYTE) SSH_FXP_SETSTAT);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	AppendStringToBufferWithLenCE(buf, strPath, m_nCharset);
	SetSFTPAttributesToBuffer(m_uServerVersion, buf, attr);
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG CSFTPChannel::FSetStat(HSFTPHANDLE hSFTP, const CSFTPFileAttribute& attr)
{
	CExBuffer buf;
	CSFTPHandle* pHandle;
	pHandle = CSFTPHandle::FromHandle(hSFTP);
	if (!pHandle)
		return 0;

	m_uCurMsgID = GetNextMsgID(SSH_FXP_FSETSTAT);

	buf.AppendToBuffer((BYTE) SSH_FXP_FSETSTAT);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	buf.AppendToBufferWithLenCE(pHandle->data(), pHandle->nSize);
	SetSFTPAttributesToBuffer(m_uServerVersion, buf, attr);
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG CSFTPChannel::Rename(LPCWSTR lpszOldPath, LPCWSTR lpszNewPath, DWORD dwFlags)
{
	CMyStringW strOldPath(lpszOldPath), strNewPath(lpszNewPath);
	CExBuffer buf;
	if (m_exts.m_bPosixRename)
		m_uCurMsgID = GetNextMsgID(SSH_FXP_EXTENDED, "posix-rename@openssh.com");
	else
		m_uCurMsgID = GetNextMsgID(SSH_FXP_RENAME);

	buf.AppendToBuffer((BYTE) (m_exts.m_bPosixRename ? SSH_FXP_EXTENDED : SSH_FXP_RENAME));
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	if (m_exts.m_bPosixRename)
		buf.AppendToBufferWithLenCE("posix-rename@openssh.com");
	AppendStringToBufferWithLenCE(buf, strOldPath, m_nCharset);
	AppendStringToBufferWithLenCE(buf, strNewPath, m_nCharset);
	if (m_uServerVersion >= 5)
		buf.AppendToBufferCE(dwFlags);
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG CSFTPChannel::SymLink(LPCWSTR lpszLinkPath, LPCWSTR lpszTargetPath)
{
	if (m_uServerVersion < 3)
		return 0;
	if (m_uServerVersion >= 6)
		return Link(lpszLinkPath, lpszTargetPath, true);

	CMyStringW strLinkPath(lpszLinkPath), strTargetPath(lpszTargetPath);
	CExBuffer buf;
	m_uCurMsgID = GetNextMsgID(SSH_FXP_SYMLINK);

	buf.AppendToBuffer((BYTE) SSH_FXP_SYMLINK);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	// NOTE: OpenSSH implements SSH_FXP_SYMLINK with incorrect order of arguments,
	// so we must reverse them when sending message to OpenSSH
	if (m_bSymLinkBug)
	{
		AppendStringToBufferWithLenCE(buf, strTargetPath, m_nCharset);
		AppendStringToBufferWithLenCE(buf, strLinkPath, m_nCharset);
	}
	else
	{
		AppendStringToBufferWithLenCE(buf, strLinkPath, m_nCharset);
		AppendStringToBufferWithLenCE(buf, strTargetPath, m_nCharset);
	}
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG CSFTPChannel::Link(LPCWSTR lpszNewLinkPath, LPCWSTR lpszExistingPath, bool bSymLink)
{
	if (m_uServerVersion < 6)
		return 0;

	CMyStringW strNewLinkPath(lpszNewLinkPath), strExistingPath(lpszExistingPath);
	CExBuffer buf;
	m_uCurMsgID = GetNextMsgID(SSH_FXP_LINK);

	buf.AppendToBuffer((BYTE) SSH_FXP_LINK);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	AppendStringToBufferWithLenCE(buf, strNewLinkPath, m_nCharset);
	AppendStringToBufferWithLenCE(buf, strExistingPath, m_nCharset);
	buf.AppendToBuffer((BYTE) (bSymLink ? 1 : 0));
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG CSFTPChannel::ReadLink(LPCWSTR lpszPath)
{
	CMyStringW strPath(lpszPath);
	CExBuffer buf;
	m_uCurMsgID = GetNextMsgID(SSH_FXP_READLINK);

	buf.AppendToBuffer((BYTE) SSH_FXP_READLINK);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	AppendStringToBufferWithLenCE(buf, strPath, m_nCharset);
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG CSFTPChannel::StatVFS(LPCWSTR lpszPath)
{
	if (!IsSupportStatVFS())
		return 0;

	CMyStringW strPath(lpszPath);
	CExBuffer buf;
	m_uCurMsgID = GetNextMsgID(SSH_FXP_EXTENDED, "statvfs@openssh.com");

	buf.AppendToBuffer((BYTE) SSH_FXP_EXTENDED);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	buf.AppendToBufferWithLenCE("statvfs@openssh.com");
	AppendStringToBufferWithLenCE(buf, strPath, m_nCharset);
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG CSFTPChannel::FStatVFS(HSFTPHANDLE hSFTP)
{
	if (!IsSupportStatVFS())
		return 0;

	CExBuffer buf;
	CSFTPHandle* pHandle;
	pHandle = CSFTPHandle::FromHandle(hSFTP);
	if (!pHandle)
		return 0;

	m_uCurMsgID = GetNextMsgID(SSH_FXP_EXTENDED, "fstatvfs@openssh.com");

	buf.AppendToBuffer((BYTE) SSH_FXP_EXTENDED);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	buf.AppendToBufferWithLenCE("fstatvfs@openssh.com");
	buf.AppendToBufferWithLenCE(pHandle->data(), pHandle->nSize);
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG CSFTPChannel::Block(HSFTPHANDLE hSFTP, ULONGLONG uliOffset, ULONGLONG uliLength, DWORD dwBlockMask)
{
	if (m_uServerVersion < 6)
		return 0;

	CExBuffer buf;
	CSFTPHandle* pHandle;
	pHandle = CSFTPHandle::FromHandle(hSFTP);
	if (!pHandle)
		return 0;

	m_uCurMsgID = GetNextMsgID(SSH_FXP_BLOCK);

	buf.AppendToBuffer((BYTE) SSH_FXP_BLOCK);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	buf.AppendToBufferWithLenCE(pHandle->data(), pHandle->nSize);
	buf.AppendToBufferCE(uliOffset);
	buf.AppendToBufferCE(uliLength);
	buf.AppendToBufferCE(dwBlockMask);
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

ULONG CSFTPChannel::Unblock(HSFTPHANDLE hSFTP, ULONGLONG uliOffset, ULONGLONG uliLength)
{
	if (m_uServerVersion < 6)
		return 0;

	CExBuffer buf;
	CSFTPHandle* pHandle;
	pHandle = CSFTPHandle::FromHandle(hSFTP);
	if (!pHandle)
		return 0;

	m_uCurMsgID = GetNextMsgID(SSH_FXP_BLOCK);

	buf.AppendToBuffer((BYTE) SSH_FXP_BLOCK);
	buf.AppendToBufferCE((DWORD) m_uCurMsgID);
	buf.AppendToBufferWithLenCE(pHandle->data(), pHandle->nSize);
	buf.AppendToBufferCE(uliOffset);
	buf.AppendToBufferCE(uliLength);
	if (!SendSFTPChannelData(buf, buf.GetLength()))
		return 0;
	return m_uCurMsgID;
}

////////////////////////////////////////////////////////////////////////////////

void CSFTPChannel::ProcessChannelData(CExBuffer& buffer)
{
	ULONG uSize, u;
	void* pv;

	::EnterCriticalSection(&m_csBuffer);

	m_bufferLastData.AppendToBuffer(buffer, buffer.GetLength());
	while (true)
	{
		pv = m_bufferLastData;
		uSize = (ULONG) m_bufferLastData.GetLength();

		u = ConvertEndian(*((DWORD*) pv));
		if (u > uSize - sizeof(uSize))
		{
			// wait for next SSH2_MSG_CHANNEL_DATA
			break;
		}

		CExBuffer* pBuf = new CExBuffer((size_t) u);
		if (!pBuf || !pBuf->SetDataToBuffer((BYTE*) pv + 4, (size_t) u))
		{
			m_bufferLastData.Empty();
			if (pBuf)
				delete pBuf;
			::LeaveCriticalSection(&m_csBuffer);
			return;
		}
		::EnterCriticalSection(&m_csQueue);
		((CSFTPMessageQueue*) m_pvMsgQueue)->Add(pBuf);
		::LeaveCriticalSection(&m_csQueue);
		//m_bufferLastData.Empty();
		m_bufferLastData.SkipPosition((long) (u + sizeof(u)));
		if (m_bufferLastData.IsEmpty())
			break;
	}
	::LeaveCriticalSection(&m_csBuffer);
}

bool CSFTPChannel::ProcessQueuedBuffer(CSFTPChannelListener* pListener, CExBuffer& buffer)
{
	BYTE bMsg;
	if (!buffer.GetAndSkip(bMsg))
		return false;

	if (m_bInitializing && bMsg != SSH_FXP_VERSION)
		return false;

	ULONG uMsgID;
	uMsgID = (bMsg == SSH_FXP_VERSION ? (ULONG) 0 : ConvertEndian(*((ULONG*)(void*) buffer)));

	{
		CMyStringW str;
		str.Format(L"Received SFTP message... (type = %d, msgid = %lu)",
			(int) bMsg,
			uMsgID);
		theApp.Log(EasySFTPLogLevel::Debug, str, S_OK);
	}

	::EnterCriticalSection(&m_csListeners);
	for (int i = 0; i < m_aListeners.GetCount(); i++)
	{
		CListenerData* pData = m_aListeners.GetItemPtr(i);
		if (pData->uMsgID == uMsgID)
		{
			pListener = pData->pListener;
			m_aListeners.RemoveItem(i);
			break;
		}
	}
	::LeaveCriticalSection(&m_csListeners);
	bool bRet = false;
	switch (bMsg)
	{
		case SSH_FXP_VERSION: bRet = ProcessSFTPVersion(pListener, buffer); break;
		case SSH_FXP_STATUS: bRet = ProcessSFTPStatus(pListener, buffer); break;
		case SSH_FXP_HANDLE: bRet = ProcessSFTPHandle(pListener, buffer); break;
		case SSH_FXP_DATA: bRet = ProcessSFTPData(pListener, buffer); break;
		case SSH_FXP_NAME: bRet = ProcessSFTPName(pListener, buffer); break;
		case SSH_FXP_ATTRS: bRet = ProcessSFTPAttrs(pListener, buffer); break;
	}

	return bRet;
}

static void __stdcall SetStringByCharset(CMyStringW& string, ServerCharset nCharset, LPCVOID lpvBuffer, DWORD dwLength)
{
	switch (nCharset)
	{
		case scsUTF8:
			string.SetUTF8String((LPCBYTE) lpvBuffer, dwLength);
			break;
		case scsShiftJIS:
			string.SetString((LPCSTR) lpvBuffer, dwLength);
			break;
		case scsEUC:
		{
			LPSTR lp = string.GetBufferA(dwLength);
			memcpy(lp, lpvBuffer, (size_t) dwLength);
			::EUCToShiftJISString(lp, dwLength);
			string.ReleaseBufferA(TRUE, dwLength);
		}
		break;
	}
}

bool CSFTPChannel::ProcessSFTPVersion(CSFTPChannelListener* pListener, CExBuffer& buffer)
{
	ULONG uVersion;
	if (!buffer.GetAndSkipCE(uVersion))
		return false;

	m_uServerVersion = uVersion;
	m_bInitializing = false;
	m_bSymLinkBug = false;
	memset(&m_exts, 0, sizeof(m_exts));
	while (!buffer.IsEmpty())
	{
		ULONG uNameLen, uValueLen;
		char* pszName, * pszValue;
		if (!buffer.GetAndSkipCE(uNameLen) ||
			!(pszName = (char*) buffer.GetCurrentBufferPermanentAndSkip((size_t) uNameLen)) ||
			!buffer.GetAndSkipCE(uValueLen) ||
			!(pszValue = (char*) buffer.GetCurrentBufferPermanentAndSkip((size_t) uValueLen)))
			return false;
		if (strcmplen1(pszName, (size_t) uNameLen, "posix-rename@openssh.com") == 0)
		{
			m_exts.m_bPosixRename = (strcmplen1(pszValue, (size_t) uValueLen, "2") == 0);
		}
		else if (strcmplen1(pszName, (size_t) uNameLen, "statvfs@openssh.com") == 0)
		{
			m_exts.m_bStatVFS = (strcmplen1(pszValue, (size_t) uValueLen, "2") == 0);
		}
		else if (strcmplen1(pszName, (size_t) uNameLen, "fstatvfs@openssh.com") == 0)
		{
			m_exts.m_bFStatVFS = (strcmplen1(pszValue, (size_t) uValueLen, "2") == 0);
		}
		// version 4
		else if (strcmplen1(pszName, (size_t) uNameLen, "newline") == 0)
		{
			m_exts.m_bNewLines = 1;
			SetStringByCharset(m_strNewLines, m_nCharset, (LPCBYTE) pszValue, uValueLen);
		}
		else
		{
			// unknown property
			CMyStringW str, strN, strV;
			strN.SetString(pszName, uNameLen);
			strV.SetString(pszValue, uValueLen);
			str.Format(L"Unknown SFTP property: %s = %s", (LPCWSTR) strN, (LPCWSTR) strV);
			theApp.Log(EasySFTPLogLevel::Debug, str, S_OK);
		}
	}

	m_uMsgID = 1;
	pListener->SFTPOpened(this);
	return true;
}

bool CSFTPChannel::ProcessSFTPStatus(CSFTPChannelListener* pListener, CExBuffer& buffer)
{
	ULONG uMsgID, uStatus, uSize, uLang;
	char* pszMsg, * pszLang;
	CSFTPMessage* pMsg;

	if (!buffer.GetAndSkipCE(uMsgID) ||
		!buffer.GetAndSkipCE(uStatus) ||
		(m_uServerVersion >= 3 &&
		(!buffer.GetAndSkipCE(uSize) ||
		!(pszMsg = (char*) buffer.GetCurrentBufferPermanentAndSkip((size_t) uSize)) ||
		!buffer.GetAndSkipCE(uLang) ||
		!(pszLang = (char*) buffer.GetCurrentBufferPermanentAndSkip((size_t) uLang)))) ||
		!buffer.IsEmpty() ||
		!(pMsg = PopMessage(uMsgID)))
		return false;

	CMyStringW str;
	SetStringByCharset(str, m_nCharset, pszMsg, uSize);
	pListener->SFTPConfirm(this, pMsg, (int) uStatus, str);
	return true;
}

bool CSFTPChannel::ProcessSFTPHandle(CSFTPChannelListener* pListener, CExBuffer& buffer)
{
	ULONG uMsgID, uSize;
	void* pvHandle;
	CSFTPMessage* pMsg;

	if (!buffer.GetAndSkipCE(uMsgID) ||
		!buffer.GetAndSkipCE(uSize) ||
		!(pvHandle = buffer.GetCurrentBufferPermanentAndSkip((size_t) uSize)) ||
		!buffer.IsEmpty() ||
		!(pMsg = PopMessage(uMsgID)))
		return false;

	CSFTPHandle* ph = (CSFTPHandle*) malloc(sizeof(CSFTPHandle) + uSize);
	ph->dwSignature = SFTPHANDLE_SIGNATURE;
	ph->nSize = (size_t) uSize;
	memcpy(ph->data(), pvHandle, (size_t) uSize);
	pListener->SFTPFileHandle(this, pMsg, ph->handle());
	return true;
}

bool CSFTPChannel::ProcessSFTPData(CSFTPChannelListener* pListener, CExBuffer& buffer)
{
	ULONG uMsgID, uSize;
	void* pvData;
	CSFTPMessage* pMsg;
	BYTE bEOF;

	if (!buffer.GetAndSkipCE(uMsgID) ||
		!buffer.GetAndSkipCE(uSize) ||
		!(pvData = buffer.GetCurrentBufferPermanentAndSkip((size_t) uSize)) ||
		(m_uServerVersion >= 6 && !buffer.IsEmpty() && !buffer.GetAndSkip(bEOF)) ||
		!buffer.IsEmpty() ||
		!(pMsg = PopMessage(uMsgID)))
		return false;

	if (m_uServerVersion >= 6)
	{
		bool bE = (bEOF != 0);
		pListener->SFTPReceiveData(this, pMsg, pvData, (size_t) uSize, &bE);
	}
	else
		pListener->SFTPReceiveData(this, pMsg, pvData, (size_t) uSize, NULL);

	return true;
}

bool CSFTPChannel::ProcessSFTPName(CSFTPChannelListener* pListener, CExBuffer& buffer)
{
	ULONG uMsgID, uCount, uShortLen, uLongLen, u;
	BYTE* pbShort, * pbLong;
	CSFTPFileData* pFiles;
	CSFTPMessage* pMsg;

	if (!buffer.GetAndSkipCE(uMsgID) ||
		!buffer.GetAndSkipCE(uCount) ||
		!(pMsg = PopMessage(uMsgID)))
		return false;

	pFiles = (CSFTPFileData*) malloc(sizeof(CSFTPFileData) * uCount);
	if (!pFiles)
		return false;
	for (u = 0; u < uCount; u++)
	{
		CallConstructor(&pFiles[u]);
		if (!buffer.GetAndSkipCE(uShortLen) ||
			!(pbShort = (BYTE*) buffer.GetCurrentBufferPermanentAndSkip((size_t) uShortLen)) ||
			(m_uServerVersion <= 3 &&
				(!buffer.GetAndSkipCE(uLongLen) ||
				!(pbLong = (BYTE*) buffer.GetCurrentBufferPermanentAndSkip((size_t) uLongLen)))
			) ||
			!ParseSFTPAttributes(m_uServerVersion, buffer, pFiles[u].attr))
		{
			pFiles[u].~CSFTPFileData();
			while (u--)
			{
				FreeSFTPFileAttributeExtendedData(pFiles[u].attr.nExAttrCount, pFiles[u].attr.aExAttrs);
				pFiles[u].~CSFTPFileData();
			}
			free(pFiles);
			return false;
		}
		SetStringByCharset(pFiles[u].strFileName, m_nCharset, pbShort, uShortLen);
		if (m_uServerVersion <= 3)
			SetStringByCharset(pFiles[u].strLongName, m_nCharset, pbLong, uLongLen);
		//else
		//	pFiles[u].strLongName.Empty();
	}

	bool bRet = false;
	if (buffer.IsEmpty())
	{
		bRet = true;
		pListener->SFTPReceiveFileName(this, pMsg, pFiles, (int) uCount);
	}

	for (u = 0; u < uCount; u++)
	{
		FreeSFTPFileAttributeExtendedData(pFiles[u].attr.nExAttrCount, pFiles[u].attr.aExAttrs);
		pFiles[u].~CSFTPFileData();
	}
	free(pFiles);

	return bRet;
}

bool CSFTPChannel::ProcessSFTPAttrs(CSFTPChannelListener* pListener, CExBuffer& buffer)
{
	ULONG uMsgID;
	CSFTPFileAttribute attr;
	CSFTPMessage* pMsg;

	if (!buffer.GetAndSkipCE(uMsgID) ||
		!ParseSFTPAttributes(m_uServerVersion, buffer, attr) ||
		!buffer.IsEmpty() ||
		!(pMsg = PopMessage(uMsgID)))
		return false;

	pListener->SFTPReceiveAttributes(this, pMsg, attr);

	FreeSFTPFileAttributeExtendedData(attr.nExAttrCount, attr.aExAttrs);

	return true;
}

bool CSFTPChannel::ProcessSFTPExtendedReply(CSFTPChannelListener* pListener, CExBuffer& buffer)
{
	ULONG uMsgID;
	CSFTPMessage* pMsg;

	if (!buffer.GetAndSkipCE(uMsgID) ||
		!(pMsg = PopMessage(uMsgID)))
		return false;

	if (pMsg->bSentMsg == SSH_FXP_EXTENDED && pMsg->lpszAddInfo &&
		(strcmp(pMsg->lpszAddInfo, "statvfs@openssh.com") == 0 ||
		strcmp(pMsg->lpszAddInfo, "fstatvfs@openssh.com") == 0))
	{
		sftp_statvfs stat;
		ULONGLONG flags;
		if (buffer.GetAndSkipCE(stat.f_bsize) &&
			buffer.GetAndSkipCE(stat.f_frsize) &&
			buffer.GetAndSkipCE(stat.f_blocks) &&
			buffer.GetAndSkipCE(stat.f_bfree) &&
			buffer.GetAndSkipCE(stat.f_bavail) &&
			buffer.GetAndSkipCE(stat.f_files) &&
			buffer.GetAndSkipCE(stat.f_ffree) &&
			buffer.GetAndSkipCE(stat.f_favail) &&
			buffer.GetAndSkipCE(stat.f_fsid) &&
			buffer.GetAndSkipCE(flags) &&
			buffer.GetAndSkipCE(stat.f_namemax) &&
			buffer.IsEmpty())
		{
			stat.f_flag = (flags & SSH_FXE_STATVFS_ST_RDONLY) ? ST_RDONLY : 0;
			stat.f_flag |= (flags & SSH_FXE_STATVFS_ST_NOSUID) ? ST_NOSUID : 0;
			pListener->SFTPReceiveStatVFS(this, pMsg, stat);
		}
		else
			return false;
	}

	return true;
}
