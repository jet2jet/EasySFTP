/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 SFTPStrm.cpp - implementations of CSFTPStream
 */

#include "stdafx.h"
#include "ShellDLL.h"
#include "SFTPStrm.h"

#define CACHE_SIZE  32768

////////////////////////////////////////////////////////////////////////////////

bool CSFTPSyncMessenger::WaitForCurrentMessage()
{
	bool bRet;
	ULONG uRegID = m_uMsgID;
	m_bFailed = false;

	if (!m_pChannel->RegisterMessageListener(uRegID, this))
		return false;

	DWORD dwStart = GetTickCount();
	while (true)
	{
		auto hr = m_pProcessor->PumpSocketAndMessage(0);
		if (FAILED(hr))
		{
			m_bFailed = true;
			bRet = false;
			break;
		}
		//if (m_bFailed)
		//	return false;
		if (!m_uMsgID)
		{
			bRet = !m_bFailed;
			break;
		}
		if (GetTickCount() - dwStart >= 10000)
		{
			bRet = false;
			break;
		}
		Sleep(10);
	}
	m_pChannel->UnregisterMessageListener(uRegID, this);
	return bRet;
}

bool CSFTPSyncMessenger::TryStat(CFTPFileItem* pFile)
{
	CMyStringW str(m_strDirectory);
	if (str.IsEmpty() || ((LPCWSTR) str)[str.GetLength() - 1] != L'/')
		str += L'/';
	str += pFile->strFileName;
	m_uMsgID = m_pChannel->Stat(str);
	if (m_uMsgID == 0)
		return false;
	return WaitForCurrentMessage();
}

bool CSFTPSyncMessenger::TryFStat(HSFTPHANDLE hFile)
{
	m_uMsgID = m_pChannel->FStat(hFile);
	if (m_uMsgID == 0)
		return false;
	return WaitForCurrentMessage();
}

void CSFTPSyncMessenger::ChannelClosed(CSSHChannel* pChannel)
{
	theApp.Log(EasySFTPLogLevel::Debug, "CSFTPSyncMessenger::ChannelClosed", S_OK);
	m_bFailed = true;
}

void CSFTPSyncMessenger::SFTPConfirm(CSFTPChannel* pChannel, CSFTPMessage* pMsg, int nStatus, const CMyStringW& strMessage)
{
	if (m_uMsgID == pMsg->uMsgID)
	{
		if (nStatus == SSH_FX_OK)
			;
		else
		{
			CMyStringW str;
			if (strMessage.IsEmpty())
				str.Format(L"CSFTPSyncMessenger::SFTPConfirm: msgtype = %d, status = %d",
					(int) pMsg->bSentMsg, nStatus);
			else
				str.Format(L"CSFTPSyncMessenger::SFTPConfirm: status = %d, message = %s",
					(int) pMsg->bSentMsg, nStatus, (LPCWSTR) strMessage);
			theApp.Log(EasySFTPLogLevel::Debug, str, E_FAIL);
			m_bFailed = true;
		}
		m_uMsgID = 0;
	}
	else
		m_bFailed = true;
}

// in FileList.cpp
extern "C" void __stdcall Time64AndNanoToFileTime(ULONGLONG uliTime64, DWORD dwNano, LPFILETIME pft);

void CSFTPSyncMessenger::SFTPReceiveAttributes(CSFTPChannel* pChannel, CSFTPMessage* pMsg, const CSFTPFileAttribute& attrs)
{
	if (m_uMsgID == pMsg->uMsgID)
	{
		if (pChannel->GetServerVersion() >= 4)
		{
			if ((attrs.dwMask & (SSH_FILEXFER_ATTR_SIZE | SSH_FILEXFER_ATTR_MODIFYTIME)) != (SSH_FILEXFER_ATTR_SIZE | SSH_FILEXFER_ATTR_MODIFYTIME))
				m_bFailed = true;
			else
			{
				m_uliDataSize.QuadPart = attrs.uliSize.QuadPart;
				::Time64AndNanoToFileTime(attrs.dwModifiedTime, attrs.dwModifiedTimeNano, &m_ftModTime);
			}
		}
		else
		{
			if ((attrs.dwMask & (SSH_FILEXFER_ATTR_SIZE | SSH_FILEXFER_ATTR_ACMODTIME)) != (SSH_FILEXFER_ATTR_SIZE | SSH_FILEXFER_ATTR_ACMODTIME))
				m_bFailed = true;
			else
			{
				m_uliDataSize.QuadPart = attrs.uliSize.QuadPart;
				::Time64AndNanoToFileTime(attrs.dwModifiedTime, 0, &m_ftModTime);
			}
		}
		m_uMsgID = 0;
	}
	else
		m_bFailed = true;
}

////////////////////////////////////////////////////////////////////////////////

CSFTPStream::CSFTPStream(IUnknown* pUnkOuter, CPumpMessageProcessor* pProcessor, CSFTPChannel* pChannel, LPCWSTR lpszDirectory)
	: CSFTPSyncMessenger(pProcessor, pChannel, lpszDirectory)
	, m_pUnkOuter(pUnkOuter)
	, m_hFile(NULL)
	, m_nCacheSize(CACHE_SIZE)
{
	if (pUnkOuter)
		pUnkOuter->AddRef();
	m_uliOffset.QuadPart = 0;
}

CSFTPStream::~CSFTPStream()
{
	//CMyStringW str;
	//int code;
	//delete m_pDataSocket;
	//if (m_pControlSocket)
	//{
	//	m_pControlSocket->ReceiveMessage(str, &code);
	//	OutputDebugFTPMessage(code, str);
	//}
	if (m_hFile)
		m_pChannel->CloseHandle(m_hFile);
	if (m_pUnkOuter)
		m_pUnkOuter->Release();
}

bool CSFTPStream::TryOpenFile(CFTPFileItem* pFile)
{
	CMyStringW str(m_strDirectory);
	if (str.IsEmpty() || ((LPCWSTR) str)[str.GetLength() - 1] != L'/')
		str += L'/';
	str += pFile->strFileName;
	m_strFileName = str;
	if (m_pChannel->GetServerVersion() >= 5)
		m_uMsgID = m_pChannel->OpenFileEx(str, ACE4_READ_DATA, SSH_FXF_OPEN_EXISTING);
	else
		m_uMsgID = m_pChannel->OpenFile(str, SSH_FXF_READ);
	if (m_uMsgID == 0)
		return false;
	return WaitForCurrentMessage();
}

void CSFTPStream::SFTPConfirm(CSFTPChannel* pChannel, CSFTPMessage* pMsg, int nStatus, const CMyStringW& strMessage)
{
	if (m_uMsgID == pMsg->uMsgID)
	{
		if (nStatus == SSH_FX_OK)
			;
		else if (nStatus == SSH_FX_EOF)
		{
			m_bEOF = true;
			theApp.Log(EasySFTPLogLevel::Info, CMyStringW(IDS_DOWNLOADED_FILE, m_strFileName.operator LPCWSTR()), S_OK);
		}
		else
			m_bFailed = true;
		m_uMsgID = 0;
	}
	else
		m_bFailed = true;
}

void CSFTPStream::SFTPFileHandle(CSFTPChannel* pChannel, CSFTPMessage* pMsg, HSFTPHANDLE hSFTP)
{
	if (m_uMsgID == pMsg->uMsgID)
	{
		m_hFile = hSFTP;
		m_bufferCache.Empty();
		m_bEOF = false;
		m_bNextEOF = false;
		m_uMsgID = 0;
	}
	else
		m_bFailed = true;
}

void CSFTPStream::SFTPReceiveData(CSFTPChannel* pChannel, CSFTPMessage* pMsg, const void* pvData, size_t nLen, const bool* pbEOF)
{
	if (m_uMsgID == pMsg->uMsgID)
	{
		m_bufferCache.AppendToBuffer(pvData, nLen);
		m_uliOffset.QuadPart += nLen;
		if (m_uReqSize < nLen)
		{
			//m_bFailed = true;
			//return;
			nLen = (size_t) m_uReqSize;
		}
		void* pv = m_bufferCache.GetCurrentBufferPermanentAndSkip(nLen);
		memcpy(m_pvCurBuffer, pv, nLen);
		m_pvCurBuffer = (void*) (((DWORD_PTR) m_pvCurBuffer) + nLen);
		m_uReqSize -= (ULONG) nLen;
		if (m_pcbRead)
			*m_pcbRead += (ULONG) nLen;
		if (pbEOF && *pbEOF)
			m_bNextEOF = true;
		m_bAnyRead = true;
		m_uMsgID = 0;
	}
	else
		m_bFailed = true;
}

STDMETHODIMP CSFTPStream::QueryInterface(REFIID riid, void** ppvObject)
{ 
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IStream) ||
		IsEqualIID(riid, IID_ISequentialStream))
	{
		*ppvObject = static_cast<IStream*>(this);
		AddRef();
		return S_OK;
	}
	else
		return E_NOINTERFACE; 
}

STDMETHODIMP CSFTPStream::Read(void* pv, ULONG cb, ULONG* pcbRead)
{
	if (!pv)
		return E_POINTER;
	//if (!m_pClient || m_pClient->m_socket.IsRemoteClosed())
	//	return E_ABORT;
	if (m_bufferCache.GetLength() >= (size_t) cb)
	{
		void* pv2 = m_bufferCache.GetCurrentBufferPermanentAndSkip((size_t) cb);
		memcpy(pv, pv2, (size_t) cb);
		if (pcbRead)
			*pcbRead = cb;
		return S_OK;
	}
	//if (m_uMsgID == 0)
	//{
		m_pvCurBuffer = pv;
		m_uReqSize = cb;
		if (pcbRead)
			*pcbRead = 0;
		m_pcbRead = pcbRead;
		if (m_nCacheSize < (size_t) cb)
			m_nCacheSize = (size_t) cb;
		m_bAnyRead = false;
		m_uMsgID = m_pChannel->ReadFile(m_hFile, m_uliOffset.QuadPart, m_nCacheSize);
		if (m_uMsgID == 0)
			return E_FAIL;
	//}
	//if (!m_pChannel->RegisterMessageListener(m_uMsgID, this))
	//	return E_FAIL;
	//if (pcbRead)
	//	*pcbRead = 0;
	//return S_OK;

	while (true)
	{
		if (!WaitForCurrentMessage())
			return E_FAIL;
		// if reaches EOF, then SSH_FXP_STATUS has been received with status SSH_FX_EOF
		if (m_bEOF)
		{
			if (m_bufferCache.GetLength() > 0)
			{
				// must be GetLength() < cb
				if (cb > (ULONG) m_bufferCache.GetLength())
					cb = (ULONG) m_bufferCache.GetLength();
				memcpy(pv, m_bufferCache.GetCurrentBufferPermanentAndSkip((size_t) cb), (size_t) cb);
				if (pcbRead)
					*pcbRead += cb;
				return S_OK;
			}
			return m_bAnyRead ? S_OK : S_FALSE;
		}
		if (m_uReqSize && m_bufferCache.GetLength() >= (size_t) m_uReqSize)
		{
			memcpy((BYTE*) pv + (cb - m_uReqSize), m_bufferCache.GetCurrentBufferPermanentAndSkip((size_t) m_uReqSize), (size_t) m_uReqSize);
			m_uReqSize = 0;
		}
		if (m_bNextEOF)
		{
			m_bEOF = true;
			theApp.Log(EasySFTPLogLevel::Info, CMyStringW(IDS_DOWNLOADED_FILE, m_strFileName.operator LPCWSTR()), S_OK);
		}
		else if (m_uReqSize > 0)
		{
			m_uMsgID = m_pChannel->ReadFile(m_hFile, m_uliOffset.QuadPart, m_nCacheSize);
			if (m_uMsgID == 0)
				return E_FAIL;
			continue;
		}
		break;
	}
	return S_OK;
}

STDMETHODIMP CSFTPStream::Write(const void* pv, ULONG cb, ULONG* pcbWritten)
{
	return STG_E_ACCESSDENIED;
}

STDMETHODIMP CSFTPStream::SetSize(ULARGE_INTEGER)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSFTPStream::CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten)
{
	if (!pstm)
		return STG_E_INVALIDPOINTER;

	HRESULT hr = S_FALSE;
	ULONG ur, uw;
	CExBuffer buf;
	void* pv = buf.AppendToBuffer(NULL, CACHE_SIZE);
	if (!pv)
		return E_OUTOFMEMORY;
	while (cb.QuadPart)
	{
		hr = Read(pv, CACHE_SIZE, &ur);
		if (hr != S_OK)
		{
			if (SUCCEEDED(hr))
				hr = S_OK;
			break;
		}
		if (pcbRead)
			pcbRead->QuadPart += ur;
		hr = pstm->Write(pv, ur, &uw);
		if (FAILED(hr))
			break;
		if (pcbWritten)
			pcbWritten->QuadPart += uw;
	}
	return hr;
}

STDMETHODIMP CSFTPStream::Commit(DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSFTPStream::Revert()
{
	return E_NOTIMPL;
}

STDMETHODIMP CSFTPStream::LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSFTPStream::UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSFTPStream::Clone(IStream**)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSFTPStream::Seek(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer)
{ 
	if (lpNewFilePointer && ::IsBadWritePtr(lpNewFilePointer, sizeof(ULARGE_INTEGER)))
		return STG_E_INVALIDPOINTER;
	switch (dwOrigin)
	{
		case STREAM_SEEK_SET:
			if ((ULONGLONG) liDistanceToMove.QuadPart > m_uliDataSize.QuadPart)
				m_uliOffset.QuadPart = m_uliDataSize.QuadPart;
			else
				m_uliOffset.QuadPart = (ULONGLONG) liDistanceToMove.QuadPart;
			break;
		case STREAM_SEEK_CUR:
			if (!liDistanceToMove.QuadPart)
			{
				if (lpNewFilePointer)
					lpNewFilePointer->QuadPart = m_uliOffset.QuadPart - m_bufferCache.GetLength();
				return S_OK;
			}
			if (liDistanceToMove.QuadPart < 0 && m_uliOffset.QuadPart < (ULONGLONG) -liDistanceToMove.QuadPart)
				return STG_E_INVALIDFUNCTION;
			else
				m_uliOffset.QuadPart += liDistanceToMove.QuadPart;
			if (m_uliOffset.QuadPart > m_uliDataSize.QuadPart)
				m_uliOffset.QuadPart = m_uliDataSize.QuadPart;
			break;
		case STREAM_SEEK_END:
			if (liDistanceToMove.QuadPart < 0 && m_uliDataSize.QuadPart < (ULONGLONG) -liDistanceToMove.QuadPart)
				return STG_E_INVALIDFUNCTION;
			else if (liDistanceToMove.QuadPart >= 0)
				m_uliOffset.QuadPart = m_uliDataSize.QuadPart;
			else
				m_uliOffset.QuadPart += liDistanceToMove.QuadPart;
			break;
		default:
			return STG_E_INVALIDFUNCTION;
	}
	m_bufferCache.Empty();
	if (lpNewFilePointer)
		lpNewFilePointer->QuadPart = m_uliOffset.QuadPart;
	return S_OK;
}

STDMETHODIMP CSFTPStream::Stat(STATSTG* pStatstg, DWORD grfStatFlag)
{
	if (!pStatstg)
		return E_POINTER;

	pStatstg->cbSize = m_uliDataSize;
	return S_OK;
}
