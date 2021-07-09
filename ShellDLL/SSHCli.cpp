/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 SSHCli.cpp - implementations of CSSH2Client
 */

#include "StdAfx.h"
#include "SSHCli.h"

#include "Func.h"

////////////////////////////////////////////////////////////////////////////////

CSSH2Client::CSSH2Client(void)
	: m_pSession(NULL)
	, m_pAuth(NULL)
	, m_nAuthType(0)
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
}

bool CSSH2Client::OnFirstReceive()
{
	if (m_pSession)
		return false;
	m_pSession = libssh2_session_init();
	if (!m_pSession)
		return false;
	libssh2_session_set_blocking(m_pSession, 0);

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
	auto fingerprint = libssh2_hostkey_hash(m_pSession, LIBSSH2_HOSTKEY_HASH_SHA1);
	if (!pHandler->CheckFingerPrint(reinterpret_cast<const BYTE*>(fingerprint), 20))
		return -1;
	return 1;
}

////////////////////////////////////////////////////////////////////////////////

AuthReturnType CSSH2Client::Authenticate(char nAuthType, CUserInfo* pUserInfo)
{
	if (!m_pAuth)
	{
		m_nAuthType = nAuthType;
		switch (nAuthType)
		{
			case AUTHTYPE_PASSWORD:
				m_pAuth = new CPasswordAuthentication();
				break;
			case AUTHTYPE_PUBLICKEY:
				m_pAuth = new CPublicKeyAuthentication();
				break;
			case AUTHTYPE_PAGEANT:
				m_pAuth = new CPageantAuthentication();
				break;
			default:
			case AUTHTYPE_NONE:
				m_pAuth = new CNoneAuthentication();
				m_nAuthType = AUTHTYPE_NONE;
				break;
		}
	}
	return m_pAuth->Authenticate(m_pSession, pUserInfo, "ssh-connection");
}

bool CSSH2Client::CanRetryAuthenticate()
{
	if (!m_pAuth)
		return false;
	return m_pAuth->CanRetry();
}

void CSSH2Client::EndAuthenticate()
{
	m_pAuth->FinishAndDelete();
	m_pAuth = NULL;
}

LPSTR CSSH2Client::AvailableAuthTypes()
{
	if (!m_pAuth)
		return NULL;
	if (m_nAuthType != AUTHTYPE_NONE)
		return NULL;
	return static_cast<CNoneAuthentication*>(m_pAuth)->m_lpAuthList;
}
