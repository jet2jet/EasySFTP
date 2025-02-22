/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 FTPMsg.cpp - implementations of FTP passive message classes
 */

#include "stdafx.h"
#include "ShellDLL.h"
#include "FTPMsg.h"

#include "FTPFldr.h"
#include "FTPConn.h"
#include "Transfer.h"

////////////////////////////////////////////////////////////////////////////////

bool CFTPWaitPassive::OnReceiveForHandshake()
{
	auto r = pPassive->ContinueHandshake();
	if (r == CFTPSocket::HandshakeResult::Success)
		nWaitFlags = CFTPWaitPassive::WaitFlags::WaitingForEstablish;
	else if (r == CFTPSocket::HandshakeResult::Error)
	{
		bWaiting = false;
		nWaitFlags = CFTPWaitPassive::WaitFlags::Error;
		pMessage->EndReceive(NULL);
		//break;
		//::LeaveCriticalSection(&m_csSocket);
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool CFTPFileListingMessage::SendPassive(CFTPConnection* pConnection, CFTPWaitPassive* pWait)
{
	//return pConnection->SendCommandWithType(L"NLST", L"-alL", L"A", pWait) != NULL;
	return pConnection->SendCommandWithType(L"LIST", m_strDirectory, L"A", pWait) != NULL;
}

bool CFTPFileListingMessage::OnReceive(CFTPSocket* pPassive)
{
	return m_pListener->ReceiveFileListing(pPassive, false);
}

void CFTPFileListingMessage::EndReceive(UINT* puStatusMsgID)
{
	m_pListener->FinishFileListing();
	//*puStatusMsgID = IDS_DIRLIST_RECEIVED;
}

////////////////////////////////////////////////////////////////////////////////

bool CFTPFileMListingMessage::SendPassive(CFTPConnection* pConnection, CFTPWaitPassive* pWait)
{
	return pConnection->SendCommandWithType(L"MLSD", m_strDirectory, L"A", pWait) != NULL;
}

bool CFTPFileMListingMessage::OnReceive(CFTPSocket* pPassive)
{
	return m_pListener->ReceiveFileListing(pPassive, true);
}

void CFTPFileMListingMessage::EndReceive(UINT* puStatusMsgID)
{
	m_pListener->FinishFileListing();
	//*puStatusMsgID = IDS_DIRLIST_RECEIVED;
}

////////////////////////////////////////////////////////////////////////////////

CFTPFileSendMessage::CFTPFileSendMessage(IStream* pStreamLocalData,
		LPCWSTR lpszRemoteFileName)
	: m_pStreamLocalData(pStreamLocalData), m_strRemoteFileName(lpszRemoteFileName)
	, m_bNeedRepeat(false), m_bFinished(false), m_pDispatcher(NULL)
{
	m_bForWrite = true;
	m_pvBuffer = malloc(STREAM_BUFFER_SIZE);
	m_uliOffset = 0;
	m_ulLastSize = 0;
	m_pStreamLocalData->AddRef();
	//if (lpszTouch)
	//	m_strTouch = lpszTouch;
	//else
	//	m_pDispatcher = NULL;
}

CFTPFileSendMessage::~CFTPFileSendMessage()
{
	free(m_pvBuffer);
	m_pStreamLocalData->Release();
}

bool CFTPFileSendMessage::SendPassive(CFTPConnection* pConnection, CFTPWaitPassive* pWait)
{
	return pConnection->SendCommandWithType(L"STOR", m_strRemoteFileName, L"I", pWait) != NULL;
}

bool CFTPFileSendMessage::ReadyToWrite(CFTPSocket* pPassive)
{
	if (m_bNeedRepeat)
		m_bNeedRepeat = false;
	else
	{
		HRESULT hr;
		ULONG uSize = 0;
		hr = m_pStreamLocalData->Read(m_pvBuffer, STREAM_BUFFER_SIZE, &uSize);
		if (FAILED(hr))
		{
			m_bCanceled = true;
			return false;
		}
		if (hr != S_OK || !uSize)
		{
			m_bFinished = true;
			return false;
		}
		m_ulLastSize = uSize;
	}
	if (pPassive->IsRemoteClosed())
		return false;
	bool needRepeat = false;
	pPassive->Send(m_pvBuffer, (SIZE_T)m_ulLastSize, 0, &needRepeat);
	if (needRepeat)
	{
		m_bNeedRepeat = true;
		return true;
	}
	m_uliOffset += m_ulLastSize;
	//return uSize == STREAM_BUFFER_SIZE;
	return true;
}

void CFTPFileSendMessage::EndReceive(UINT* puStatusMsgID)
{
}

////////////////////////////////////////////////////////////////////////////////

CFTPWriteFileMessage::CFTPWriteFileMessage(LPCWSTR lpszRemoteFileName)
	: m_strRemoteFileName(lpszRemoteFileName)
	, m_bFinished(false), m_pDispatcher(NULL)
{
	m_bForWrite = true;
	m_pvBuffer = NULL;
	m_dwSize = 0;
	//if (lpszTouch)
	//	m_strTouch = lpszTouch;
	//else
	//	m_pDispatcher = NULL;
}

CFTPWriteFileMessage::~CFTPWriteFileMessage()
{
}

bool CFTPWriteFileMessage::SendPassive(CFTPConnection* pConnection, CFTPWaitPassive* pWait)
{
	return pConnection->SendCommandWithType(L"STOR", m_strRemoteFileName, L"I", pWait) != NULL;
}

bool CFTPWriteFileMessage::ReadyToWrite(CFTPSocket* pPassive)
{
	if (!m_pvBuffer)
	{
		m_bCanceled = true;
		return false;
	}
	if (!m_dwSize)
	{
		m_bFinished = true;
		return false;
	}
	bool needRepeat;
	while (true)
	{
		needRepeat = false;
		if (pPassive->IsRemoteClosed())
			return false;
		pPassive->Send(m_pvBuffer, (SIZE_T)m_dwSize, 0, &needRepeat);
		if (!needRepeat)
			break;
	}
	m_pvBuffer = NULL;
	m_dwSize = 0;
	return true;
}

void CFTPWriteFileMessage::EndReceive(UINT* puStatusMsgID)
{
}

////////////////////////////////////////////////////////////////////////////////

CFTPStream::CFTPStream(LPCWSTR lpszFileName,
		ULARGE_INTEGER uliDataSize,
		CSFTPFolderFTP* pRoot)
	: m_strFileName(lpszFileName)
	, m_uliDataSize(uliDataSize)
	, m_pRoot(pRoot)
{
	m_pPassive = NULL;
	m_uliNowPos.QuadPart = 0;
	pRoot->AddRef();
}

CFTPStream::~CFTPStream()
{
	Close();
	m_pRoot->Release();
}

STDMETHODIMP CFTPStream::QueryInterface(REFIID riid, void** ppvObject)
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

STDMETHODIMP CFTPStream::Read(void* pv, ULONG cb, ULONG* pcbRead)
{
	if (!pv)
		return E_POINTER;
	HRESULT hr = InitDataSocket();
	if (FAILED(hr))
		return hr;
	::Sleep(0);
	if (m_pPassive->pPassive->IsRemoteClosed())
	{
		Close();
		if (pcbRead)
			*pcbRead = 0;
		return S_FALSE;
	}
	if (!m_pPassive->pPassive->CanReceive(WAIT_RECEIVE_TIME))
		return E_FAIL;
	ULONG read = 0;
	while (cb)
	{
		bool needMoreData = false;
		if (m_pPassive->pPassive->IsRemoteClosed())
		{
			Close();
			break;
		}
		if (!m_pPassive->pPassive->CanReceive(WAIT_RECEIVE_TIME))
			break;
		int ret = m_pPassive->pPassive->Recv(pv, (SIZE_T) cb, 0, &needMoreData);
		if (ret < 0)
			return E_FAIL;
		if (needMoreData)
			continue;
		if (!ret)
			break;
		m_uliNowPos.QuadPart += (ULONG) ret;
		read += (ULONG) ret;
		pv = ((LPBYTE) pv) + (ULONG) ret;
		cb -= (ULONG) ret;
	}
	if (pcbRead)
		*pcbRead = read;
	return read ? S_OK : S_FALSE;
}

STDMETHODIMP CFTPStream::Write(const void* pv, ULONG cb, ULONG* pcbWritten)
{
	return STG_E_ACCESSDENIED;
}

STDMETHODIMP CFTPStream::SetSize(ULARGE_INTEGER)
{
    return E_NOTIMPL;
}

#define CACHE_SIZE  32768

STDMETHODIMP CFTPStream::CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten)
{
	if (!pstm)
		return STG_E_INVALIDPOINTER;

	HRESULT hr = S_FALSE;
	ULONG ur, uw;
	void* pv = malloc(CACHE_SIZE);
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
	free(pv);
	return hr;
}

STDMETHODIMP CFTPStream::Commit(DWORD)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFTPStream::Revert()
{
    return E_NOTIMPL;
}

STDMETHODIMP CFTPStream::LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFTPStream::UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFTPStream::Clone(IStream**)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFTPStream::Seek(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer)
{ 
	if (dwOrigin == STREAM_SEEK_END || (dwOrigin == STREAM_SEEK_CUR && liDistanceToMove.QuadPart < 0) ||
		dwOrigin == STREAM_SEEK_SET && (ULONGLONG) liDistanceToMove.QuadPart < m_uliNowPos.QuadPart)
	{
		if (!m_pRoot->m_pConnection->IsCommandAvailable(L"REST"))
			return E_NOTIMPL;
		if (m_pPassive)
		{
			delete m_pPassive;
			m_pPassive = NULL;
		}
		ULONGLONG ulTo;
		switch (dwOrigin)
		{
			case STREAM_SEEK_SET:
				ulTo = (ULONGLONG) liDistanceToMove.QuadPart;
				break;
			case STREAM_SEEK_CUR:
				ulTo = m_uliNowPos.QuadPart + liDistanceToMove.QuadPart;
				break;
			case STREAM_SEEK_END:
				ulTo = m_uliDataSize.QuadPart + liDistanceToMove.QuadPart;
				break;
			default:
				return STG_E_INVALIDFUNCTION;
		}
		m_uliNowPos.QuadPart = ulTo;
		return S_OK;
	}
	ULONGLONG ulTo = (dwOrigin == STREAM_SEEK_SET ? (ULONGLONG) liDistanceToMove.QuadPart :
		m_uliNowPos.QuadPart + liDistanceToMove.QuadPart);
	void* pb = malloc(8192);
	while (m_uliNowPos.QuadPart < ulTo)
	{
		ULONG ul = 8192;
		if (ulTo - m_uliNowPos.QuadPart < 8192)
			ul = (ULONG) (ulTo - m_uliNowPos.QuadPart);
		HRESULT hr = Read(pb, ul, &ul);
		if (FAILED(hr))
		{
			free(pb);
			return hr;
		}
	}
	free(pb);
	if (lpNewFilePointer)
		lpNewFilePointer->QuadPart = m_uliNowPos.QuadPart;
	return S_OK;
}

STDMETHODIMP CFTPStream::Stat(STATSTG* pStatstg, DWORD grfStatFlag)
{
	if (!pStatstg)
		return E_POINTER;

	pStatstg->cbSize = m_uliDataSize;
	return S_OK;
}

HRESULT CFTPStream::InitDataSocket()
{
	if (m_pPassive)
		return S_OK;

	CMyScopedCSLock lock(m_pRoot->m_csSocket);
	CFTPFileRecvMessage* pMessage = new CFTPFileRecvMessage(m_strFileName, m_uliNowPos.QuadPart);
	CFTPWaitEstablishPassive* pEstablishWait = m_pRoot->StartPassive(pMessage);
	pMessage->Release();
	if (!pEstablishWait)
		return E_FAIL;
	if (!m_pRoot->WaitForReceive(&pEstablishWait->bWaiting) || !pEstablishWait->pRet)
	{
		delete pEstablishWait;
		return E_FAIL;
	}
	CFTPWaitPassive* pPassive = pEstablishWait->pRet;
	delete pEstablishWait;
	if (!m_pRoot->WaitForReceive(&pPassive->bWaiting, pPassive))
	{
		delete pPassive;
		return E_FAIL;
	}
	m_pPassive = pPassive;

	return S_OK;
}

void CFTPStream::Close()
{
	if (!m_pPassive)
		return;
	{
		CMyScopedCSLock lock(m_pRoot->m_csSocket);
		m_pPassive->pPassive->Close();
		m_pPassive->pDone->bWaiting = true;
		m_pRoot->WaitForReceive(&m_pPassive->pDone->bWaiting);
	}
	delete m_pPassive;
	m_pPassive = NULL;
}

CFTPFileRecvMessage::CFTPFileRecvMessage(LPCWSTR lpszRemoteFileName, ULONGLONG uliOffset)
	: m_strRemoteFileName(lpszRemoteFileName)
	, m_uliOffset(uliOffset)
{
	m_bForWrite = false;
}

CFTPFileRecvMessage::~CFTPFileRecvMessage()
{
}

bool CFTPFileRecvMessage::SendPassive(CFTPConnection* pConnection, CFTPWaitPassive* pWait)
{
	if (m_uliOffset && pConnection->IsCommandAvailable(L"REST"))
	{
		CMyStringW str;
		str.Format(L"%I64u", m_uliOffset);
		return pConnection->SendTripleCommand(
			L"TYPE", L"I",
			L"REST", str,
			L"RETR", m_strRemoteFileName,
			NULL, pWait, pWait) != NULL;
	}
	return pConnection->SendCommandWithType(L"RETR", m_strRemoteFileName, L"I", pWait) != NULL;
}
