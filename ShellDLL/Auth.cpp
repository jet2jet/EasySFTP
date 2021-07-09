/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 Auth.cpp - implementations of authentication classes for SSH
 */

#include "StdAfx.h"
#include "Auth.h"

#include "ExBuffer.h"
#include "PuTTYLib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPasswordAuthentication::~CPasswordAuthentication()
{
	if (m_lpszPassword)
	{
		_SecureStringW::SecureEmptyBuffer(m_lpszPassword, sizeof(char) * m_dwPasswordLen);
		free(m_lpszPassword);
	}
}

AuthReturnType CPasswordAuthentication::Authenticate(LIBSSH2_SESSION* pSession, CUserInfo* pUser, LPCSTR lpszService)
{
	if (!m_lpszUser)
	{
		m_lpszUser = reinterpret_cast<LPCSTR>(pUser->strName.AllocUTF8String(&m_dwUserLen));
	}
	if (!m_lpszPassword)
	{
		CMyStringW str;
		pUser->strPassword.GetString(str);
		m_lpszPassword = reinterpret_cast<LPSTR>(str.AllocUTF8StringC(&m_dwPasswordLen));
		_SecureStringW::SecureEmptyString(str);
		if (!m_lpszPassword)
			return AuthReturnType::Error;
	}

	auto ret = libssh2_userauth_password_ex(pSession, m_lpszUser, static_cast<UINT>(m_dwUserLen),
		m_lpszPassword, static_cast<UINT>(m_dwPasswordLen), NULL);
	if (ret == LIBSSH2_ERROR_EAGAIN)
		return AuthReturnType::Again;

	m_lpszUser = NULL;
	_SecureStringW::SecureEmptyBuffer(m_lpszPassword, sizeof(char) * m_dwPasswordLen);
	free(m_lpszPassword);
	m_lpszPassword = NULL;

	if (ret != 0)
		return AuthReturnType::Error;
	return AuthReturnType::Success;
}

////////////////////////////////////////////////////////////////////////////////

CChangePasswordAuthentication::~CChangePasswordAuthentication()
{
	if (m_lpszPassword)
	{
		_SecureStringW::SecureEmptyBuffer(m_lpszPassword, sizeof(char) * m_dwPasswordLen);
		free(m_lpszPassword);
	}
}

AuthReturnType CChangePasswordAuthentication::Authenticate(LIBSSH2_SESSION* pSession, CUserInfo* pUser, LPCSTR lpszService)
{
	if (!m_lpszUser)
	{
		m_lpszUser = reinterpret_cast<LPCSTR>(pUser->strName.AllocUTF8String(&m_dwUserLen));
	}
	if (!m_lpszPassword)
	{
		CMyStringW str;
		pUser->strPassword.GetString(str);
		m_lpszPassword = reinterpret_cast<LPSTR>(str.AllocUTF8StringC(&m_dwPasswordLen));
		_SecureStringW::SecureEmptyString(str);
		if (!m_lpszPassword)
			return AuthReturnType::Error;
	}

	auto cb = [](LIBSSH2_SESSION* pSession, char** newPassword, int* newPasswordLen, void** ppUserPtr) -> void
	{
		auto pUser = static_cast<CUserInfo*>(*ppUserPtr);
		CMyStringW str;
		pUser->strNewPassword.GetString(str);
		*newPassword = reinterpret_cast<LPSTR>(str.AllocUTF8StringC(reinterpret_cast<size_t*>(newPasswordLen)));
		_SecureStringW::SecureEmptyString(str);
	};
	*libssh2_session_abstract(pSession) = pUser;
	auto ret = libssh2_userauth_password_ex(pSession, m_lpszUser, static_cast<UINT>(m_dwUserLen),
		m_lpszPassword, static_cast<UINT>(m_dwPasswordLen),
		cb);
	if (ret == LIBSSH2_ERROR_EAGAIN)
		return AuthReturnType::Again;

	m_lpszUser = NULL;
	_SecureStringW::SecureEmptyBuffer(m_lpszPassword, sizeof(char) * m_dwPasswordLen);
	free(m_lpszPassword);
	m_lpszPassword = NULL;

	if (ret != 0)
		return AuthReturnType::Error;
	return AuthReturnType::Success;
}

////////////////////////////////////////////////////////////////////////////////

CPublicKeyAuthentication::~CPublicKeyAuthentication()
{
	if (m_lpszPassword)
	{
		_SecureStringW::SecureEmptyBuffer(m_lpszPassword, sizeof(char) * m_dwPasswordLen);
		free(m_lpszPassword);
	}
}

AuthReturnType CPublicKeyAuthentication::Authenticate(LIBSSH2_SESSION* pSession, CUserInfo* pUser, LPCSTR lpszService)
{
	if (!m_lpszUser)
	{
		m_lpszUser = reinterpret_cast<LPCSTR>(pUser->strName.AllocUTF8String(&m_dwUserLen));
	}
	if (!m_lpszPKeyFileName)
	{
		m_lpszPKeyFileName = reinterpret_cast<LPCSTR>(pUser->strPKeyFileName.AllocUTF8String());
	}
	if (!m_lpszPassword)
	{
		CMyStringW str;
		pUser->strPassword.GetString(str);
		m_lpszPassword = reinterpret_cast<LPSTR>(str.AllocUTF8StringC(&m_dwPasswordLen));
		_SecureStringW::SecureEmptyString(str);
		if (!m_lpszPassword)
			return AuthReturnType::Error;
	}

	auto ret = libssh2_userauth_publickey_fromfile_ex(pSession, m_lpszUser, static_cast<UINT>(m_dwUserLen), NULL, m_lpszPKeyFileName, m_lpszPassword);
	if (ret == LIBSSH2_ERROR_EAGAIN)
		return AuthReturnType::Again;

	m_lpszUser = NULL;
	_SecureStringW::SecureEmptyBuffer(m_lpszPassword, sizeof(char) * m_dwPasswordLen);
	free(m_lpszPassword);
	m_lpszPassword = NULL;

	if (ret != 0)
		return AuthReturnType::Error;
	return AuthReturnType::Success;
}

////////////////////////////////////////////////////////////////////////////////

CPageantAuthentication::~CPageantAuthentication()
{
	if (m_lpPageantKeyList)
		PuTTYFreeKeyList(m_lpPageantKeyList);
}

AuthReturnType CPageantAuthentication::Authenticate(LIBSSH2_SESSION* pSession, CUserInfo* pUser, LPCSTR lpszService)
{
	if (!m_lpszUser)
	{
		m_lpszUser = reinterpret_cast<LPCSTR>(pUser->strName.AllocUTF8String());
	}
	if (!m_lpPageantKeyList)
	{
		m_lpPageantKeyList = pUser->lpPageantKeyList;
		pUser->lpPageantKeyList = NULL;
		if (!m_lpPageantKeyList)
			return AuthReturnType::Error;
		m_dwKeyCount = ConvertEndian(*((DWORD*) m_lpPageantKeyList));
		m_dwKeyIndex = 0;
		m_lpCurrentKey = m_lpPageantKeyList + 4;
	}
	LPBYTE p = m_lpCurrentKey;

	LPCSTR lpszKeyType;
	LPCBYTE pBlob;
	size_t nBlobLen;

	// get key type data (in the head of blob data)
	DWORD dwKeyTypeLen = ConvertEndian(*((DWORD*) (p + 4)));
	lpszKeyType = (LPCSTR) (p + 8);

	nBlobLen = (size_t) ConvertEndian(*((DWORD*) p));
	pBlob = (p + 4);
	p += nBlobLen + 4;

	void* abstract = m_lpCurrentKey;
	auto ret = libssh2_userauth_publickey(pSession, m_lpszUser, pBlob, nBlobLen,
		[](LIBSSH2_SESSION*, LPBYTE* sig, size_t* sig_len, LPCBYTE data, size_t data_len, void** abstract) -> int
		{
			LPBYTE lpCurrentKey = static_cast<LPBYTE>(*abstract);
			DWORD nSignedLen;
			void* pSignedData = PuTTYSignSSH2Key(lpCurrentKey, data, static_cast<DWORD>(data_len), &nSignedLen);
			*sig = static_cast<LPBYTE>(pSignedData);
			*sig_len = static_cast<size_t>(nSignedLen);
			return 0;
		},
		&abstract);
	
	if (ret == LIBSSH2_ERROR_EAGAIN)
		return AuthReturnType::Again;

	m_lpszUser = NULL;

	if (ret != 0)
		return AuthReturnType::Error;
	return AuthReturnType::Success;
}

bool CPageantAuthentication::CanRetry()
{
	if (!m_lpPageantKeyList)
		return false;
	m_dwKeyIndex++;
	if (m_dwKeyIndex >= m_dwKeyCount)
	{
		PuTTYFreeKeyList(m_lpPageantKeyList);
		m_lpPageantKeyList = NULL;
		m_dwKeyCount = 0;
		return false;
	}

	DWORD dw = ConvertEndian(*((DWORD*) m_lpCurrentKey));
	m_lpCurrentKey += dw + 4;
	dw = ConvertEndian(*((DWORD*) m_lpCurrentKey));
	m_lpCurrentKey += dw + 4;
	return true;
}

////////////////////////////////////////////////////////////////////////////////

CNoneAuthentication::~CNoneAuthentication()
{
	if (m_lpAuthList)
		free(m_lpAuthList);
}

AuthReturnType CNoneAuthentication::Authenticate(LIBSSH2_SESSION* pSession, CUserInfo* pUser, LPCSTR lpszService)
{
	if (!m_lpszUser)
	{
		m_lpszUser = reinterpret_cast<LPCSTR>(pUser->strName.AllocUTF8String(&m_dwUserLen));
	}

	auto ret = libssh2_userauth_list(pSession, m_lpszUser, static_cast<UINT>(m_dwUserLen));
	if (!ret)
	{
		if (libssh2_userauth_authenticated(pSession))
			return AuthReturnType::Success;
		auto err = libssh2_session_last_errno(pSession);
		if (err == LIBSSH2_ERROR_EAGAIN)
			return AuthReturnType::Again;
		m_lpAuthList = NULL;
		return AuthReturnType::Error;
	}

	auto len = strlen(ret) + 1;
	m_lpAuthList = static_cast<char*>(malloc(sizeof(char) * (len + 1)));
	if (!m_lpAuthList)
		return AuthReturnType::Error;
	for (size_t i = 0; i < len; ++i)
	{
		if (ret[i] == ',')
			m_lpAuthList[i] = 0;
		else
			m_lpAuthList[i] = ret[i];
	}
	m_lpAuthList[len] = 0;
	return AuthReturnType::Error;
}
