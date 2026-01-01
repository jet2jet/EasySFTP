/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 SSHCli.cpp - implementations of CSSH2Client
 */

#include "StdAfx.h"
#include "SSHCli.h"

#include "Func.h"
#include "unicode.h"

////////////////////////////////////////////////////////////////////////////////

CSSH2Client::CSSH2Client(void)
	: m_pSession(NULL)
	, m_pAuth(NULL)
	, m_lpAuthList(NULL)
	, m_nAuthType(EasySFTPAuthenticationMode::None)
{
}

CSSH2Client::~CSSH2Client(void)
{
	if (m_pSession)
	{
		libssh2_session_disconnect_ex(m_pSession, SSH_DISCONNECT_BY_APPLICATION, "disconnected by user", "");
		libssh2_session_free(m_pSession);
		m_socket.Close();
	}
	if (m_lpAuthList)
		free(m_lpAuthList);
}

bool CSSH2Client::OnFirstReceive()
{
	if (m_pSession)
		return false;
	m_pSession = libssh2_session_init_ex(
		[](size_t count, void**) { return malloc(count); },
		[](void* ptr, void**) { return free(ptr); },
		[](void* ptr, size_t count, void**) { return realloc(ptr, count); },
		NULL
		);
	if (!m_pSession)
		return false;
#ifdef _DEBUG
	libssh2_trace(m_pSession, LIBSSH2_TRACE_KEX | LIBSSH2_TRACE_AUTH | LIBSSH2_TRACE_CONN);
	libssh2_trace_sethandler(m_pSession, NULL, [](LIBSSH2_SESSION* session,
		void* context,
		const char* data,
		size_t length) {
			OutputDebugStringA(data);
			if (length > 0 && data[length - 1] != '\n')
				OutputDebugStringA("\r\n");
		});
#endif
	libssh2_session_set_blocking(m_pSession, 0);
	libssh2_keepalive_config(m_pSession, 0, 2);

	if (!StartKeyExchange())
		return false;

	return true;
}

bool CSSH2Client::StartKeyExchange()
{
	auto ret = libssh2_session_handshake(m_pSession, m_socket);
	if (ret != 0 && ret != LIBSSH2_ERROR_EAGAIN)
	{
		return false;
	}
	return true;
}

int CSSH2Client::OnHandshake(CSSH2FingerPrintHandler* pHandler)
{
	auto ret = libssh2_session_handshake(m_pSession, m_socket);
	if (ret == LIBSSH2_ERROR_EAGAIN)
		return 0;
	if (ret != 0)
	{
		return -1;
	}
	auto fingerprint = libssh2_hostkey_hash(m_pSession, LIBSSH2_HOSTKEY_HASH_MD5);
	if (!pHandler->CheckFingerPrint(reinterpret_cast<const BYTE*>(fingerprint), 16))
		return -1;
	return 1;
}

////////////////////////////////////////////////////////////////////////////////

AuthReturnType CSSH2Client::Authenticate(IEasySFTPAuthentication2* pAuth)
{
	if (m_pAuth)
		m_pAuth->Release();
	m_pAuth = pAuth;
	pAuth->AddRef();
	if (m_lpAuthList)
	{
		free(m_lpAuthList);
		m_lpAuthList = NULL;
	}
	return CAuthentication::SSHAuthenticate(pAuth, m_pSession, "ssh-connection", &m_lpAuthList);
}

bool CSSH2Client::CanRetryAuthenticate()
{
	if (!m_pAuth)
		return false;
	return CAuthentication::CanRetry(m_pAuth);
}

void CSSH2Client::EndAuthenticate()
{
	m_pAuth->Release();
	m_pAuth = NULL;
}

LPSTR CSSH2Client::AvailableAuthTypes()
{
	if (!m_pAuth)
		return NULL;
	return m_lpAuthList;
}

void CSSH2Client::SendKeepAlive()
{
	int seconds = 0;
	auto r = libssh2_keepalive_send(m_pSession, &seconds);
	(void)seconds;
	(void)r;
#ifdef _DEBUG
	{
		CMyStringW str;
		str.Format(L"[ssh] Sending keep-alive result:%d, seconds:%d\n", r, seconds);
		OutputDebugStringW(str);
	}
#endif
}
