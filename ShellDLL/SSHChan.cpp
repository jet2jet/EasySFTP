#include "stdafx.h"
#include "ShellDLL.h"
#include "SSHChan.h"

#define EXPECTED_MAX_PACKET_SIZE  (256 * 1024)

CSSHChannel::CSSHChannel(CSSHChannelListener* pListener)
	: m_pListener(pListener)
	, m_pChannel(NULL)
	, m_pendingStartup()
	, m_pendingSendChannelData()
	, m_pendingRead()
	, m_pendingWindowAdjust()
{
}

CSSHChannel::~CSSHChannel()
{
	if (m_pChannel)
		libssh2_channel_free(m_pChannel);
}

SSHChannelReturnType CSSHChannel::InitializeChannel(LIBSSH2_SESSION* pSession)
{
	if (m_pChannel)
		return SSHChannelReturnType::Success;

	auto ret = libssh2_channel_open_ex(pSession, "session", sizeof("session") - 1,
		LIBSSH2_CHANNEL_WINDOW_DEFAULT, LIBSSH2_CHANNEL_PACKET_DEFAULT, NULL, 0);
	if (!ret)
	{
		auto err = libssh2_session_last_errno(pSession);
		if (err == LIBSSH2_ERROR_EAGAIN)
			return SSHChannelReturnType::Again;
		m_pListener->ChannelOpenFailure(this, err);
		return SSHChannelReturnType::Error;
	}

	m_pendingRead.dwReadLen = 256;
	m_pendingBuffer.ResetPosition();
	m_pendingRead.pvBuffer = m_pendingBuffer.AppendToBuffer(NULL, static_cast<size_t>(m_pendingRead.dwReadLen));
	if (!m_pendingRead.pvBuffer)
	{
		libssh2_channel_close(ret);
		m_pListener->ChannelOpenFailure(this, LIBSSH2_ERROR_ALLOC);
		return SSHChannelReturnType::Error;
	}
	m_pChannel = ret;
	m_pListener->ChannelOpened(this);
	return SSHChannelReturnType::Success;
}

bool CSSHChannel::SendProcessStartup(LPCSTR lpszService, const void* pvExtraData, size_t nExtraDataLen)
{
	m_pendingStartup.lpszService = lpszService;
	m_pendingStartup.pvExtraData = pvExtraData;
	m_pendingStartup.nExtraDataLen = nExtraDataLen;
	m_pendingStartup.isPending = true;
	return true;
}

bool CSSHChannel::SendChannelData(const void* pvSendData, size_t nSendDataLen)
{
	m_pendingSendChannelData.pendingData.AppendToBuffer(pvSendData, nSendDataLen);
	m_pendingSendChannelData.isPending = true;
	return true;
}

bool CSSHChannel::CheckChannelWindow(DWORD dwLength)
{
	auto rRecv = libssh2_channel_window_read_ex(m_pChannel, NULL, NULL);
	if (dwLength <= rRecv)
		return false;
	m_pendingWindowAdjust.isPending = true;
	m_pendingWindowAdjust.dwLength = dwLength * 2;
	return true;
}

void CSSHChannel::CloseChannel()
{
	libssh2_channel_close(m_pChannel);
}

bool CSSHChannel::IsSendingData() const
{
	return m_pendingStartup.isPending;
}

bool CSSHChannel::Process(bool isSocketReceived)
{
	if (ProcessPendingStartup())
		return true;
	if (ProcessSendChannelData())
		return true;
	if (ProcessWindowAdjust())
		return true;
	if (ProcessRead())
		return true;
	return false;
}

void CSSHChannel::ProcessChannelData(CExBuffer& data)
{
	m_pListener->ChannelDataReceived(this,
		m_pendingRead.pvBuffer,
		static_cast<size_t>(m_pendingRead.dwReadLen));
}

bool CSSHChannel::ProcessPendingStartup()
{
	if (!m_pendingStartup.isPending)
		return false;
	auto r = libssh2_channel_process_startup(m_pChannel, m_pendingStartup.lpszService, strlen(m_pendingStartup.lpszService),
		static_cast<const char*>(m_pendingStartup.pvExtraData), static_cast<unsigned int>(m_pendingStartup.nExtraDataLen));
	if (r == LIBSSH2_ERROR_EAGAIN)
		return true;
	m_pendingStartup.isPending = false;
	if (r < 0)
		m_pListener->ChannelConfirm(this, false, r);
	else
		m_pListener->ChannelConfirm(this, true, 0);
	return false;
}

bool CSSHChannel::ProcessSendChannelData()
{
	if (!m_pendingSendChannelData.isPending && !m_pendingSendChannelData.isSending)
		return false;
	if (!m_pendingSendChannelData.isSending)
	{
		m_pendingSendChannelData.sendData.ResetPosition();
		m_pendingSendChannelData.sendData.Collapse(0);
		m_pendingSendChannelData.pendingData.ResetPosition();
		m_pendingSendChannelData.sendData.AppendToBuffer(m_pendingSendChannelData.pendingData,
			m_pendingSendChannelData.pendingData.GetLength());
		m_pendingSendChannelData.pendingData.Collapse(0);
		m_pendingSendChannelData.sendData.ResetPosition();
		m_pendingSendChannelData.isSending = true;
		m_pendingSendChannelData.isPending = false;
	}
	auto r = libssh2_channel_write_ex(m_pChannel, 0,
		static_cast<const char*>(static_cast<const void*>(m_pendingSendChannelData.sendData)),
		m_pendingSendChannelData.sendData.GetLength());
	if (r == LIBSSH2_ERROR_EAGAIN)
		return true;
	m_pendingSendChannelData.isSending = false;
	if (r < 0)
	{
		m_pListener->ChannelError(this, r);
	}
	return false;
}

bool CSSHChannel::ProcessRead()
{
	auto r = libssh2_channel_read_ex(m_pChannel, 0,
		static_cast<char*>(m_pendingRead.pvBuffer),
		m_pendingRead.dwReadLen);
	if (r == LIBSSH2_ERROR_EAGAIN || !r)
	{
		// no data left
		return false;
	}
	if (r < 0)
	{
		if (r == LIBSSH2_ERROR_CHANNEL_CLOSED)
			m_pListener->ChannelClosed(this);
		else
			m_pListener->ChannelError(this, r);
		return false;
	}
	m_pendingBuffer.ResetPosition();
	m_pendingBuffer.Collapse(static_cast<size_t>(r));
	ProcessChannelData(m_pendingBuffer);
	m_pendingBuffer.ResetPosition();
	m_pendingBuffer.Collapse(0);
	m_pendingRead.pvBuffer = m_pendingBuffer.AppendToBuffer(NULL, static_cast<size_t>(m_pendingRead.dwReadLen));
	// data may remain
	return true;
}

bool CSSHChannel::ProcessWindowAdjust()
{
	if (!m_pendingWindowAdjust.isPending)
		return false;
	auto r = libssh2_channel_receive_window_adjust2(m_pChannel, m_pendingWindowAdjust.dwLength, 1, NULL);
	if (r == LIBSSH2_ERROR_EAGAIN)
		return true;
	m_pendingWindowAdjust.isPending = false;
	if (r < 0)
		m_pListener->ChannelError(this, r);
	return false;
}
