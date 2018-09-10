/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 Channel.cpp - implementations of CSSH2Channel
 */

#include "StdAfx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "SSHSock.h"
#include "Channel.h"

#include "Func.h"

static struct CSSH2ChannelList
{
	UINT nID;
	CSSH2Channel* pChannel;
	CSSH2ChannelList* pNext;
} * s_pChannelList = NULL;

static CSSH2Channel* __stdcall GetChannelFromID(UINT nID)
{
	register CSSH2ChannelList* pList = s_pChannelList;
	while (pList)
	{
		if (pList->nID == nID)
			return pList->pChannel;
		pList = pList->pNext;
	}
	return NULL;
}

static UINT __stdcall AppendChannel(CSSH2Channel* pChannel)
{
	CSSH2ChannelList* pList = (CSSH2ChannelList*) malloc(sizeof(CSSH2ChannelList));
	pList->nID = (UINT) PtrToInt(pChannel);
	pList->pChannel = pChannel;
	pList->pNext = s_pChannelList;
	s_pChannelList = pList;
	return pList->nID;
}

static UINT __stdcall AppendChannel(CSSH2Channel* pChannel, UINT nID)
{
	CSSH2ChannelList* pList = (CSSH2ChannelList*) malloc(sizeof(CSSH2ChannelList));
	pList->nID = nID;
	pList->pChannel = pChannel;
	pList->pNext = s_pChannelList;
	s_pChannelList = pList;
	return nID;
}

static void __stdcall RemoveChannel(CSSH2Channel* pChannel)
{
	register CSSH2ChannelList* pList = s_pChannelList;
	register CSSH2ChannelList* pPrev;
	pPrev = NULL;
	while (pList)
	{
		if (pList->pChannel == pChannel)
		{
			if (pPrev == NULL)
				s_pChannelList = pList->pNext;
			else
				pPrev->pNext = pList->pNext;
			free(pList);
			return;
		}
		pPrev = pList;
		pList = pList->pNext;
	}
}

CSSH2Channel::CSSH2Channel(CSSH2ChannelListener* pListener, LPCSTR lpszType, int nStatus, LPCSTR lpszRemoteName)
	: m_pListener(pListener)
	, m_lpszType(lpszType)
	, m_nStatus(nStatus)
	, m_lpszRemoteName(lpszRemoteName)
	, m_nLocalWindow(CHAN_SES_WINDOW_DEFAULT)
	, m_nLocalWindowMax(CHAN_SES_WINDOW_DEFAULT)
	, m_nLocalReceived(0)
	, m_nLocalMaxPacket(CHAN_SES_PACKET_DEFAULT)
	, m_bClosed(true)
{
	CommonConstruct();
}

CSSH2Channel::CSSH2Channel(CSSH2ChannelListener* pListener, LPCSTR lpszType, int nStatus, LPCSTR lpszRemoteName,
		UINT nLocalWindow, UINT nLocalMaxPacket)
	: m_pListener(pListener)
	, m_lpszType(lpszType)
	, m_nStatus(nStatus)
	, m_lpszRemoteName(lpszRemoteName)
	, m_nLocalWindow(nLocalWindow)
	, m_nLocalWindowMax(nLocalWindow)
	, m_nLocalReceived(0)
	, m_nLocalMaxPacket(nLocalMaxPacket)
	, m_bClosed(true)
{
	CommonConstruct();
}

void CSSH2Channel::CommonConstruct()
{
	//m_pListener->AddRef();
	m_dwServerCompat = m_pListener->ChannelGetServerCompatible();
	m_nID = AppendChannel(this);
	m_nRemoteID = (UINT) -1;
	m_nRemoteWindow = 0;
	m_nRemoteMaxPacket = 0;
}

CSSH2Channel::~CSSH2Channel()
{
	RemoveChannel(this);
	//m_pListener->Release();
}

bool CSSH2Channel::OpenChannel()
{
	CExBuffer buf;

	buf.AppendToBufferWithLenCE(m_lpszType);
	// id
	buf.AppendToBufferCE((ULONG) m_nID);
	buf.AppendToBufferCE((ULONG) m_nLocalWindow);
	buf.AppendToBufferCE((ULONG) m_nLocalMaxPacket);
	return m_pListener->ChannelSendPacket(SSH2_MSG_CHANNEL_OPEN, buf, buf.GetLength());
}

bool CSSH2Channel::SendChannelRequest(LPCSTR lpszService, bool bWantConfirm, const void* pvExtraData, size_t nExtraDataLen)
{
	if (m_nRemoteID == (UINT) -1)
		return false;

	CExBuffer buf;

	buf.AppendToBufferCE((ULONG) m_nRemoteID);
	buf.AppendToBufferWithLenCE(lpszService);
	buf.AppendToBuffer((BYTE) (bWantConfirm ? 1 : 0));
	buf.AppendToBufferWithLenCE(pvExtraData, nExtraDataLen);
	return m_pListener->ChannelSendPacket(SSH2_MSG_CHANNEL_REQUEST, buf, buf.GetLength());
}

bool CSSH2Channel::SendChannelData(const void* pvSendData, size_t nSendDataLen)
{
	if (m_nRemoteID == (UINT) -1)
		return false;

	CExBuffer buf, buf2;

	if (!m_bufferKeepChannelData.IsEmpty())
	{
		if (!buf2.SetDataToBuffer(m_bufferKeepChannelData, m_bufferKeepChannelData.GetLength()) ||
			!buf2.AppendToBuffer(pvSendData, nSendDataLen))
			return false;
		m_bufferKeepChannelData.Empty();
		pvSendData = buf2;
		nSendDataLen = buf2.GetLength();
	}
	if ((size_t) m_nRemoteWindow < nSendDataLen)
	{
		size_t n = nSendDataLen - (size_t) m_nRemoteWindow;
		m_bufferKeepChannelData.AppendToBuffer(((const BYTE*) pvSendData) + m_nRemoteWindow, n);
		nSendDataLen = (size_t) m_nRemoteWindow;
		if (!nSendDataLen)
			return true;
	}
	m_nRemoteWindow -= (UINT) nSendDataLen;

	buf.AppendToBufferCE((ULONG) m_nRemoteID);
	buf.AppendToBufferWithLenCE(pvSendData, nSendDataLen);
	return m_pListener->ChannelSendPacket(SSH2_MSG_CHANNEL_DATA, buf, buf.GetLength());
}

bool CSSH2Channel::CheckChannelWindow()
{
	if (m_nStatus == SSH_CHANNEL_OPEN &&
		!m_bClosed &&
		m_nLocalReceived > 0 &&
		((m_nLocalWindowMax - m_nLocalWindow > m_nLocalMaxPacket * 3) ||
		m_nLocalWindow < m_nLocalWindowMax / 2))
	{
		// adjust window
		CExBuffer buf;

		buf.AppendToBufferCE((ULONG) m_nRemoteID);
		buf.AppendToBufferCE((ULONG) m_nLocalReceived);
		if (!m_pListener->ChannelSendPacket(SSH2_MSG_CHANNEL_WINDOW_ADJUST, buf, buf.GetLength()))
			return false;
		m_nLocalWindow += m_nLocalReceived;
		m_nLocalReceived = 0;
	}
	return true;
}

void CSSH2Channel::CloseChannel()
{
	if (m_nRemoteID == (UINT) -1)
		return;

	CExBuffer buf;

	buf.AppendToBufferCE((ULONG) m_nRemoteID);
	m_pListener->ChannelSendPacket(SSH2_MSG_CHANNEL_CLOSE, buf, buf.GetLength());
	m_bClosed = true;
}

////////////////////////////////////////////////////////////////////////////////

bool __stdcall CSSH2Channel::ProcessChannelMsg(CSSH2ChannelListener* pListener, BYTE bType, const void* pv, size_t nSize)
{
	CExBuffer buf;
	if (!buf.SetDataToBuffer(pv, nSize))
		return false;

	if (bType == SSH2_MSG_CHANNEL_CLOSE ||
		bType == SSH2_MSG_CHANNEL_DATA ||
		bType == SSH2_MSG_CHANNEL_EOF ||
		bType == SSH2_MSG_CHANNEL_EXTENDED_DATA ||
		bType == SSH2_MSG_CHANNEL_OPEN_CONFIRMATION ||
		bType == SSH2_MSG_CHANNEL_OPEN_FAILURE ||
		bType == SSH2_MSG_CHANNEL_REQUEST ||
		bType == SSH2_MSG_CHANNEL_WINDOW_ADJUST ||
		bType == SSH2_MSG_CHANNEL_SUCCESS ||
		bType == SSH2_MSG_CHANNEL_FAILURE)
	{
		UINT nID;
		if (!buf.GetAndSkipCE(nID))
			return false;
		CSSH2Channel* p = GetChannelFromID(nID);
		if (!p)
			return false;
		switch (bType)
		{
			case SSH2_MSG_CHANNEL_CLOSE: return p->ProcessChannelClose(buf);
			case SSH2_MSG_CHANNEL_DATA: return p->ProcessChannelData(buf);
			case SSH2_MSG_CHANNEL_EOF: return p->ProcessChannelEOF(buf);
			case SSH2_MSG_CHANNEL_EXTENDED_DATA: return p->ProcessChannelExtendedData(buf);
			case SSH2_MSG_CHANNEL_OPEN_CONFIRMATION: return p->ProcessChannelOpenConfirmation(buf);
			case SSH2_MSG_CHANNEL_OPEN_FAILURE: return p->ProcessChannelOpenFailure(buf);
			case SSH2_MSG_CHANNEL_REQUEST: return p->ProcessChannelRequest(buf);
			case SSH2_MSG_CHANNEL_WINDOW_ADJUST: return p->ProcessChannelWindowAdjust(buf);
			case SSH2_MSG_CHANNEL_SUCCESS: return p->ProcessChannelSuccess(buf);
			case SSH2_MSG_CHANNEL_FAILURE: return p->ProcessChannelFailure(buf);
			//default: return false;
		}
	}
	else if (bType == SSH2_MSG_CHANNEL_OPEN)
	{
		ULONG uSize, uRemoteID, uWindow, uMaxPacket;
		LPCSTR pszType;
		if (!buf.GetAndSkipCE(uSize) ||
			!(pszType = (LPCSTR) buf.GetCurrentBufferPermanentAndSkip((size_t) uSize)) ||
			!buf.GetAndSkipCE(uRemoteID) ||
			!buf.GetAndSkipCE(uWindow) ||
			!buf.GetAndSkipCE(uMaxPacket))
			return false;

		// unsupported
		buf.Empty();
		buf.AppendToBufferCE(uRemoteID);
		buf.AppendToBufferCE((ULONG) SSH2_OPEN_ADMINISTRATIVELY_PROHIBITED);
		if (!(pListener->ChannelGetServerCompatible() & SSH_BUG_OPENFAILURE))
		{
			buf.AppendToBufferWithLenCE("open failed");
			buf.AppendToBufferWithLenCE("");
		}
		return pListener->ChannelSendPacket(SSH2_MSG_CHANNEL_OPEN_FAILURE, buf, buf.GetLength());
	}
	//else
		return false;
}

bool CSSH2Channel::ProcessChannelClose(CExBuffer& buffer)
{
	if (!buffer.IsEmpty())
		return false;
	m_nRemoteID = (UINT) -1;
	m_bClosed = true;
	m_pListener->ChannelClosed(this);
	return true;
}

bool CSSH2Channel::ProcessChannelData(CExBuffer& buffer)
{
	ULONG uSize;
	void* pvData;

	if (!buffer.GetAndSkipCE(uSize) ||
		!(pvData = buffer.GetCurrentBufferPermanentAndSkip((size_t) uSize)) ||
		!buffer.IsEmpty())
		return false;

	if (!CheckChannelWindow())
		return false;
	//if (uSize > m_nLocalMaxPacket)
	//	;
	//if (uSize > m_nLocalWindow)
	//	;
	m_nLocalWindow -= uSize;
	m_nLocalReceived += uSize;

	m_pListener->ChannelDataReceived(this, pvData, (size_t) uSize);
	return true;
}

bool CSSH2Channel::ProcessChannelEOF(CExBuffer& buffer)
{
	return true;
}

bool CSSH2Channel::ProcessChannelExtendedData(CExBuffer& buffer)
{
	return true;
}

bool CSSH2Channel::ProcessChannelOpenConfirmation(CExBuffer& buffer)
{
	if (m_nStatus != SSH_CHANNEL_OPENING)
		return false;

	ULONG uRemoteID, uRWindow, uRMaxPacket;
	if (!buffer.GetAndSkipCE(uRemoteID) ||
		// SSH2
		!buffer.GetAndSkipCE(uRWindow) ||
		!buffer.GetAndSkipCE(uRMaxPacket) ||
		!buffer.IsEmpty())
		return false;
	m_nRemoteID = (UINT) uRemoteID;
	m_nRemoteWindow = (UINT) uRWindow;
	m_nRemoteMaxPacket = (UINT) uRMaxPacket;
	m_nStatus = SSH_CHANNEL_OPEN;
	m_bClosed = false;

	m_pListener->ChannelOpened(this);

	return true;
}

bool CSSH2Channel::ProcessChannelOpenFailure(CExBuffer& buffer)
{
	if (m_nStatus != SSH_CHANNEL_OPENING)
		return false;

	ULONG uReason;
	char* pszMsg, * pszLang;
	ULONG uMsgSize, uLangSize;

	// SSH2
	if (!buffer.GetAndSkipCE(uReason))
		return false;
	if (!(m_dwServerCompat & SSH_BUG_OPENFAILURE))
	{
		if (!buffer.GetAndSkipCE(uMsgSize) ||
			!(pszMsg = (char*) buffer.GetCurrentBufferPermanentAndSkip((size_t) uMsgSize)) ||
			!buffer.GetAndSkipCE(uLangSize) ||
			!(pszLang = (char*) buffer.GetCurrentBufferPermanentAndSkip((size_t) uLangSize)))
			return false;
	}
	else
		uMsgSize = uLangSize = 0;
	if (!buffer.IsEmpty())
		return false;

	CMyStringW strMessage;
	if (uMsgSize)
		strMessage.SetString(pszMsg, (DWORD) (uMsgSize / sizeof(char)));
	m_pListener->ChannelOpenFailure(this, (int) uReason, strMessage);

	return true;
}

bool CSSH2Channel::ProcessChannelRequest(CExBuffer& buffer)
{
	ULONG uTypeSize;
	char* pszType;
	BYTE bReply;
	bool bSucceeded = false;

	if (!buffer.GetAndSkipCE(uTypeSize) ||
		!(pszType = (char*) buffer.GetCurrentBufferPermanentAndSkip((size_t) uTypeSize)) ||
		!buffer.GetAndSkip(bReply))
		return false;

	if (strcmplen1(pszType, (size_t) uTypeSize, "eow@openssh.com") == 0)
	{
		if (buffer.IsEmpty())
			bSucceeded = true;
	}
	else if (strcmplen1(pszType, (size_t) uTypeSize, "exit-status") == 0)
	{
		if (buffer.GetAndSkipCE(uTypeSize) &&
			buffer.IsEmpty())
		{
			m_pListener->ChannelExitStatus(this, (int) uTypeSize);
			bSucceeded = true;
		}
	}
	else
	{
		// unknown type
	}
	if (bReply)
	{
		CExBuffer buf;
		buf.AppendToBufferCE((ULONG) m_nRemoteID);
		m_pListener->ChannelSendPacket(bSucceeded ? SSH2_MSG_CHANNEL_SUCCESS : SSH2_MSG_CHANNEL_FAILURE,
			buf, buf.GetLength());
	}

	return true;
}

bool CSSH2Channel::ProcessChannelWindowAdjust(CExBuffer& buffer)
{
	ULONG uAdjust;
	if (!buffer.GetAndSkipCE(uAdjust) ||
		!buffer.IsEmpty())
		return false;
	m_nRemoteWindow += uAdjust;
	return true;
}

bool CSSH2Channel::ProcessChannelSuccess(CExBuffer& buffer)
{
	if (!buffer.IsEmpty())
		return false;
	m_pListener->ChannelConfirm(this, true);
	return true;
}

bool CSSH2Channel::ProcessChannelFailure(CExBuffer& buffer)
{
	if (!buffer.IsEmpty())
		return false;
	m_pListener->ChannelConfirm(this, false);
	return true;
}
