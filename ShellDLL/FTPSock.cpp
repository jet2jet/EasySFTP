/*
 EasySFTP - Copyright (C) 2025 Kuri-Applications

 FTPSock.cpp - implementation of CFTPSocket
 */

#include "stdafx.h"
#include "FTPSock.h"

#include "ShellDLL.h"

CFTPSocket::CFTPSocket() : m_pSSLCTX(NULL), m_pSSL(NULL)
{
}

CFTPSocket::~CFTPSocket()
{
}

void CFTPSocket::Close()
{
	if (m_pSSL)
	{
		SSL_shutdown(m_pSSL);
		SSL_free(m_pSSL);
		m_pSSL = NULL;
	}
	if (m_pSSLCTX)
	{
		SSL_CTX_free(m_pSSLCTX);
		m_pSSLCTX = NULL;
	}
	CTextSocket::Close();
}

int CFTPSocket::Send(LPCVOID lpBuffer, SIZE_T nSize, int flags, bool* pbNeedRepeat)
{
	if (m_pSSL)
	{
		if (pbNeedRepeat)
			*pbNeedRepeat = false;
		auto r = SSL_write(m_pSSL, lpBuffer, static_cast<int>(nSize));
		if (r < 0)
		{
			auto err = SSL_get_error(m_pSSL, r);
			if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
			{
				if (pbNeedRepeat)
					*pbNeedRepeat = true;
				return 0;
			}
		}
		return r;
	}
	else
		return CTextSocket::Send(lpBuffer, nSize, flags, pbNeedRepeat);
}

int CFTPSocket::Recv(LPVOID lpBuffer, SIZE_T nSize, int flags, bool* pbNeedMoreData)
{
	if (m_pSSL)
	{
		if (pbNeedMoreData)
			*pbNeedMoreData = false;
		auto r = SSL_read(m_pSSL, lpBuffer, static_cast<int>(nSize));
		if (r < 0)
		{
			auto err = SSL_get_error(m_pSSL, r);
			if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
			{
				if (pbNeedMoreData)
					*pbNeedMoreData = true;
				return 0;
			}
		}
		return r;
	}
	else
		return CTextSocket::Recv(lpBuffer, nSize, flags, pbNeedMoreData);
}

bool CFTPSocket::CanReceive(DWORD dwWaitMilliseconds, bool* pbIsError) const
{
	if (m_pSSL)
	{
		if (SSL_pending(m_pSSL) > 0)
			return true;
	}
	return CTextSocket::CanReceive(dwWaitMilliseconds, pbIsError);
}

CFTPSocket::HandshakeResult CFTPSocket::StartHandshake()
{
	if (operator SOCKET() == INVALID_SOCKET)
		return HandshakeResult::Error;
	if(m_pSSL)
		return HandshakeResult::Success;
	m_pSSLCTX = SSL_CTX_new(TLS_client_method());
	if (!m_pSSLCTX)
		return HandshakeResult::Error;
	m_pSSL = SSL_new(m_pSSLCTX);
	if (!m_pSSL)
	{
		SSL_CTX_free(m_pSSLCTX);
		m_pSSLCTX = NULL;
		return HandshakeResult::Error;
	}
	SSL_set_fd(m_pSSL, static_cast<int>(operator SOCKET()));
	auto r = SSL_connect(m_pSSL);
	{
		CMyStringW str;
		str.Format(L"CFTPSocket::StartHandshake() SSL_connect return %d", r);
		theApp.Log(EasySFTPLogLevel::Debug, str, S_OK);
	}
	if (r < 0)
	{
		auto err = SSL_get_error(m_pSSL, r);
		{
			CMyStringW str;
			str.Format(L"CFTPSocket::StartHandshake() SSL_get_error return %d", err);
			theApp.Log(EasySFTPLogLevel::Debug, str, S_OK);
		}
		if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
			return HandshakeResult::Waiting;
		return HandshakeResult::Error;
	}
	if (r == 0)
		return HandshakeResult::Error;
	return HandshakeResult::Success;
}

CFTPSocket::HandshakeResult CFTPSocket::ContinueHandshake()
{
	if (operator SOCKET() == INVALID_SOCKET)
		return HandshakeResult::Error;
	if(!m_pSSL || !m_pSSLCTX)
		return HandshakeResult::Error;
	auto r = SSL_connect(m_pSSL);
	{
		CMyStringW str;
		str.Format(L"[EasySFTP] CFTPSocket::ContinueHandshake() SSL_connect return %d", r);
		theApp.Log(EasySFTPLogLevel::Debug, str, S_OK);
	}
	if (r < 0)
	{
		auto err = SSL_get_error(m_pSSL, r);
		{
			CMyStringW str;
			str.Format(L"[EasySFTP] CFTPSocket::ContinueHandshake() SSL_get_error return %d", err);
			theApp.Log(EasySFTPLogLevel::Debug, str, S_OK);
		}
		if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
			return HandshakeResult::Waiting;
		return HandshakeResult::Error;
	}
	if (r == 0)
		return HandshakeResult::Error;
	return HandshakeResult::Success;
}
