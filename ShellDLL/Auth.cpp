/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 Auth.cpp - implementations of authentication classes for SSH
 */

#include "StdAfx.h"
#include "Auth.h"

#include "ShellDLL.h"
#include "ExBuffer.h"
#include "Pageant.h"
#include "WinOpSSH.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAuthSession::~CAuthSession()
{
	if (pAgent)
	{
		if (lpPageantKeyList)
			pAgent->FreeKeyList(lpPageantKeyList);
		delete pAgent;
	}
}

CAuthentication::CAuthentication()
	: CDispatchImplT(theApp.GetTypeInfo(IID_IEasySFTPAuthentication2))
	, m_Mode(EasySFTPAuthenticationMode::None)
	, m_pPrivateKeyData(nullptr)
	, m_nPrivateKeyData(0)
	, m_pSession(nullptr)
{
}

CAuthentication::~CAuthentication()
{
	if (m_pPrivateKeyData != nullptr)
	{
		_SecureStringW::SecureEmptyBuffer(m_pPrivateKeyData, m_nPrivateKeyData);
		free(m_pPrivateKeyData);
	}
	if (m_pSession != NULL)
	{
		auto* p = static_cast<CAuthSession*>(m_pSession);
		if (p->dwSignature == AUTH_SESSION_SIGNATURE)
			delete p;
	}
}

STDMETHODIMP CAuthentication::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	*ppv = NULL;
	if (!IsEqualIID(riid, IID_IUnknown) && !IsEqualIID(riid, IID_IDispatch) &&
		!IsEqualIID(riid, IID_IEasySFTPAuthentication) &&
		!IsEqualIID(riid, IID_IEasySFTPAuthentication2))
	{
		return E_NOINTERFACE;
	}
	*ppv = static_cast<IEasySFTPAuthentication2*>(this);
	AddRef();
	return S_OK;
}

STDMETHODIMP CAuthentication::get_UserName(BSTR* p)
{
	if (!p)
		return E_POINTER;
	*p = MyStringToBSTR(m_strUserName);
	return *p ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CAuthentication::put_UserName(BSTR p)
{
	MyBSTRToString(p, m_strUserName);
	return S_OK;
}

STDMETHODIMP CAuthentication::get_Password(BSTR* p)
{
	if (!p)
		return E_POINTER;
	CMyStringW str;
	m_strPassword.GetString(str);
	*p = MyStringToBSTR(str);
	_SecureStringW::SecureEmptyString(str);
	return *p ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CAuthentication::put_Password(BSTR p)
{
	CMyStringW str;
	MyBSTRToString(p, str);
	m_strPassword = str;
	_SecureStringW::SecureEmptyString(str);
	return S_OK;
}

STDMETHODIMP CAuthentication::get_PrivateKeyFileName(BSTR* p)
{
	if (!p)
		return E_POINTER;
	*p = MyStringToBSTR(m_strPrivateKeyFileName);
	return *p ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CAuthentication::put_PrivateKeyFileName(BSTR p)
{
	MyBSTRToString(p, m_strPrivateKeyFileName);
	return S_OK;
}

STDMETHODIMP CAuthentication::get_Type(EasySFTPAuthenticationMode* pMode)
{
	if (!pMode)
		return E_POINTER;
	*pMode = m_Mode;
	return S_OK;
}

STDMETHODIMP CAuthentication::put_Type(EasySFTPAuthenticationMode mode)
{
	if (mode < EasySFTPAuthenticationMode::None || mode > EasySFTPAuthenticationMode::WinOpenSSH)
		return E_INVALIDARG;
	m_Mode = mode;
	return S_OK;
}

STDMETHODIMP CAuthentication::get_AuthSession(LONG_PTR* pOut)
{
	if (!pOut)
		return E_POINTER;
	*pOut = reinterpret_cast<INT_PTR>(m_pSession);
	return S_OK;
}

STDMETHODIMP CAuthentication::put_AuthSession(LONG_PTR session)
{
	m_pSession = reinterpret_cast<void*>(session);
	return S_OK;
}

STDMETHODIMP CAuthentication::SetPrivateKeyBinary(const void* buffer, long length)
{
	if (length < 0)
		return E_INVALIDARG;
	if (length != 0 && !buffer)
		return E_POINTER;
	if (m_pPrivateKeyData != nullptr)
	{
		_SecureStringW::SecureEmptyBuffer(m_pPrivateKeyData, m_nPrivateKeyData);
		free(m_pPrivateKeyData);
		m_pPrivateKeyData = nullptr;
	}
	m_nPrivateKeyData = length;
	if (length > 0)
	{
		m_pPrivateKeyData = malloc(static_cast<size_t>(length));
		if (!m_pPrivateKeyData)
			return E_OUTOFMEMORY;
		memcpy(m_pPrivateKeyData, buffer, static_cast<size_t>(length));
	}
	return S_OK;
}

STDMETHODIMP CAuthentication::GetPrivateKeyBinary(const void** buffer, long* pLength)
{
	if (!buffer || !pLength)
		return E_POINTER;
	*buffer = m_pPrivateKeyData;
	*pLength = m_nPrivateKeyData;
	return m_nPrivateKeyData > 0 ? S_OK : S_FALSE;
}

AuthReturnType CAuthentication::SSHAuthenticate(IEasySFTPAuthentication2* pAuth, LIBSSH2_SESSION* pSession, LPCSTR lpszService, char** ppAuthList)
{
	EasySFTPAuthenticationMode mode;
	if (FAILED(pAuth->get_Type(&mode)))
		return AuthReturnType::Error;
	CMyStringW strUserName;
	BSTR bstr;
	if (FAILED(pAuth->get_UserName(&bstr)))
		return AuthReturnType::Error;
	MyBSTRToString(bstr, strUserName);
	::SysFreeString(bstr);
	size_t nUserLen = 0;
	auto* pUser = reinterpret_cast<LPCSTR>(strUserName.AllocUTF8String(&nUserLen));
	switch (mode)
	{
		case EasySFTPAuthenticationMode::None:
		{
			auto ret = libssh2_userauth_list(pSession, pUser, static_cast<UINT>(nUserLen));
			if (ret)
			{
				if (ppAuthList)
				{
					auto len = strlen(ret) + 1;
					auto* lpAuthList = static_cast<char*>(malloc(sizeof(char) * (len + 1)));
					if (!lpAuthList)
						return AuthReturnType::Error;
					for (size_t i = 0; i < len; ++i)
					{
						if (ret[i] == ',')
							lpAuthList[i] = 0;
						else
							lpAuthList[i] = ret[i];
					}
					lpAuthList[len] = 0;
					*ppAuthList = lpAuthList;
				}
				return AuthReturnType::Error;
			}
			else
			{
				if (ppAuthList)
					*ppAuthList = NULL;
				if (libssh2_userauth_authenticated(pSession))
					return AuthReturnType::Success;
				auto err = libssh2_session_last_errno(pSession);
				if (err == LIBSSH2_ERROR_EAGAIN)
					return AuthReturnType::Again;
				return AuthReturnType::Error;
			}
		}
		break;
		case EasySFTPAuthenticationMode::Password:
		{
			if (FAILED(pAuth->get_Password(&bstr)))
				return AuthReturnType::Error;
			CMyStringW str;
			MyBSTRToString(bstr, str);
			_SecureStringW::SecureEmptyBStr(bstr);
			::SysFreeString(bstr);
			size_t nPasswordLen = 0;
			auto* lpszPassword = reinterpret_cast<LPSTR>(str.AllocUTF8StringC(&nPasswordLen));
			_SecureStringW::SecureEmptyString(str);
			if (!lpszPassword)
				return AuthReturnType::Error;

			auto ret = libssh2_userauth_password_ex(pSession, pUser, static_cast<UINT>(nUserLen),
				lpszPassword, static_cast<UINT>(nPasswordLen), NULL);
			_SecureStringW::SecureEmptyBuffer(lpszPassword, sizeof(char) * nPasswordLen);
			free(lpszPassword);
			if (ret == LIBSSH2_ERROR_EAGAIN)
				return AuthReturnType::Again;

			if (ret != 0)
				return AuthReturnType::Error;
			return AuthReturnType::Success;
		}
		break;
		case EasySFTPAuthenticationMode::PrivateKey:
		{
			const void* pKeyBuffer = nullptr;
			long nKeyLength = 0;
			auto hr = pAuth->GetPrivateKeyBinary(&pKeyBuffer, &nKeyLength);
			if (FAILED(hr))
				return AuthReturnType::Error;
			if (hr == S_OK && nKeyLength > 0 && pKeyBuffer != nullptr)
			{
				if (FAILED(pAuth->get_Password(&bstr)))
					return AuthReturnType::Error;
				CMyStringW str;
				MyBSTRToString(bstr, str);
				_SecureStringW::SecureEmptyBStr(bstr);
				::SysFreeString(bstr);
				size_t nPasswordLen = 0;
				auto* lpszPassword = reinterpret_cast<LPSTR>(str.AllocUTF8StringC(&nPasswordLen));
				_SecureStringW::SecureEmptyString(str);
				if (!lpszPassword)
					return AuthReturnType::Error;

				auto ret = libssh2_userauth_publickey_frommemory(pSession, pUser, static_cast<UINT>(nUserLen), NULL, 0,
					static_cast<const char*>(pKeyBuffer), static_cast<size_t>(nKeyLength), lpszPassword);
				_SecureStringW::SecureEmptyBuffer(lpszPassword, sizeof(char) * nPasswordLen);
				free(lpszPassword);
				if (ret == LIBSSH2_ERROR_EAGAIN)
					return AuthReturnType::Again;

				if (ret != 0)
					return AuthReturnType::Error;
			}
			else
			{
				if (FAILED(pAuth->get_PrivateKeyFileName(&bstr)))
					return AuthReturnType::Error;
				CMyStringW strPrivateKeyFileName;
				MyBSTRToString(bstr, strPrivateKeyFileName);
				::SysFreeString(bstr);
				auto* lpszPKeyFileName = reinterpret_cast<LPCSTR>(strPrivateKeyFileName.AllocUTF8String());
				if (FAILED(pAuth->get_Password(&bstr)))
					return AuthReturnType::Error;
				CMyStringW str;
				MyBSTRToString(bstr, str);
				_SecureStringW::SecureEmptyBStr(bstr);
				::SysFreeString(bstr);
				size_t nPasswordLen = 0;
				auto* lpszPassword = reinterpret_cast<LPSTR>(str.AllocUTF8StringC(&nPasswordLen));
				_SecureStringW::SecureEmptyString(str);
				if (!lpszPassword)
					return AuthReturnType::Error;

				auto ret = libssh2_userauth_publickey_fromfile_ex(pSession, pUser, static_cast<UINT>(nUserLen), NULL, lpszPKeyFileName, lpszPassword);
				_SecureStringW::SecureEmptyBuffer(lpszPassword, sizeof(char) * nPasswordLen);
				free(lpszPassword);
				if (ret == LIBSSH2_ERROR_EAGAIN)
					return AuthReturnType::Again;

				if (ret != 0)
					return AuthReturnType::Error;
			}
			return AuthReturnType::Success;
		}
		break;
		case EasySFTPAuthenticationMode::Pageant:
			return SSHAuthenticateWithAgent(pAuth, strUserName, pSession, lpszService, []() -> CSSHAgent* { return new CPageantAgent(); });
		case EasySFTPAuthenticationMode::WinOpenSSH:
			return SSHAuthenticateWithAgent(pAuth, strUserName, pSession, lpszService, []() -> CSSHAgent* { return new CWinOpenSSHAgent(); });
		default:
			return AuthReturnType::Error;
	}
}

bool CAuthentication::CanRetry(IEasySFTPAuthentication* pAuth)
{
	EasySFTPAuthenticationMode mode;
	if (FAILED(pAuth->get_Type(&mode)))
		return false;
	if (mode != EasySFTPAuthenticationMode::Pageant && mode != EasySFTPAuthenticationMode::WinOpenSSH)
		return false;
	CAuthSession* pAuthSession = NULL;
	{
		void* p = NULL;
		if (SUCCEEDED(pAuth->get_AuthSession(reinterpret_cast<LONG_PTR*>(&p))) && p)
		{
			if (*static_cast<DWORD*>(p) == AUTH_SESSION_SIGNATURE)
				pAuthSession = static_cast<CAuthSession*>(p);
		}
	}
	if (!pAuthSession)
		return false;

	if (!pAuthSession->lpPageantKeyList)
		return false;
	pAuthSession->dwKeyIndex++;
	if (pAuthSession->dwKeyIndex >= pAuthSession->dwKeyCount)
	{
		pAuth->put_AuthSession(reinterpret_cast<__int3264>(nullptr));
		delete pAuthSession;
		return false;
	}

	DWORD dw = ConvertEndian(*reinterpret_cast<DWORD*>(pAuthSession->lpCurrentKey));
	pAuthSession->lpCurrentKey += dw + 4;
	dw = ConvertEndian(*reinterpret_cast<DWORD*>(pAuthSession->lpCurrentKey));
	pAuthSession->lpCurrentKey += dw + 4;
	return true;
}

AuthReturnType CAuthentication::SSHAuthenticateWithAgent(IEasySFTPAuthentication* pAuth, CMyStringW& strUserName, LIBSSH2_SESSION* pSession, LPCSTR lpszService, CSSHAgent* (*CreateAgent)())
{
	size_t nUserLen = 0;
	auto* lpszUser = reinterpret_cast<LPCSTR>(strUserName.AllocUTF8String(&nUserLen));

	CAuthSession* pAuthSession = NULL;
	{
		void* p = NULL;
		if (SUCCEEDED(pAuth->get_AuthSession(reinterpret_cast<LONG_PTR*>(&p))) && p)
		{
			if (*static_cast<DWORD*>(p) == AUTH_SESSION_SIGNATURE)
				pAuthSession = static_cast<CAuthSession*>(p);
		}
	}

	if (!pAuthSession || !pAuthSession->lpPageantKeyList)
	{
		LPBYTE lpKeyList = NULL;
		auto* pAgent = CreateAgent();
		auto r = pAgent->GetKeyList2(&lpKeyList);
		if (!lpKeyList)
		{
			delete pAgent;
			return AuthReturnType::Error;
		}
		pAuthSession = new CAuthSession();
		pAuthSession->dwSignature = AUTH_SESSION_SIGNATURE;
		pAuthSession->pAgent = pAgent;
		pAuthSession->lpPageantKeyList = lpKeyList;
		pAuthSession->dwKeyCount = ConvertEndian(*((DWORD*)lpKeyList));
		pAuthSession->dwKeyIndex = 0;
		pAuthSession->lpCurrentKey = lpKeyList + 4;
		if (FAILED(pAuth->put_AuthSession(reinterpret_cast<__int3264>(pAuthSession))))
		{
			delete pAuthSession;
			return AuthReturnType::Error;
		}
	}
	LPBYTE p = pAuthSession->lpCurrentKey;

	LPCSTR lpszKeyType;
	LPCBYTE pBlob;
	size_t nBlobLen;

	// get key type data (in the head of blob data)
	DWORD dwKeyTypeLen = ConvertEndian(*((DWORD*)(p + 4)));
	lpszKeyType = (LPCSTR)(p + 8);

	nBlobLen = (size_t)ConvertEndian(*((DWORD*)p));
	pBlob = (p + 4);
	p += nBlobLen + 4;

	// get the comment of key
	{
		DWORD dwCommentLen = ConvertEndian(*((DWORD*)p));
		CMyStringW str;
		str.SetUTF8String((LPCBYTE)(p + 4), static_cast<size_t>(dwCommentLen));
		p += dwCommentLen + 4;
		CMyStringW strType(lpszKeyType), strDebug;
		strDebug.Format(L"trying key '%s' (type: %s)", str.operator LPCWSTR(), strType.operator LPCWSTR());
		theApp.Log(EasySFTPLogLevel::Debug, strDebug, S_OK);
	}

	void* abstract = pAuthSession;
	auto ret = libssh2_userauth_publickey(pSession, lpszUser, pBlob, nBlobLen,
		[](LIBSSH2_SESSION*, LPBYTE* sig, size_t* sig_len, LPCBYTE data, size_t data_len, void** abstract) -> int
		{
			*sig = NULL;
			*sig_len = 0;
			CAuthSession* pAuthSession = static_cast<CAuthSession*>(*abstract);
			LPBYTE lpCurrentKey = pAuthSession->lpCurrentKey;
			size_t nSignedLen;
			auto buff = pAuthSession->pAgent->SignSSH2Key(lpCurrentKey, data, data_len, &nSignedLen);
			LPBYTE pSignedData = static_cast<LPBYTE>(buff);
			if (nSignedLen < 4 || !buff)
			{
				if (buff)
					free(buff);
				return LIBSSH2_ERROR_AGENT_PROTOCOL;
			}
			// skip signature length
			auto entireLen = ConvertEndian(*((DWORD*)pSignedData));
			pSignedData += 4;
			nSignedLen -= 4;
			// skip signing method
			if (nSignedLen < 4)
			{
				free(buff);
				return LIBSSH2_ERROR_AGENT_PROTOCOL;
			}
			auto methodLen = ConvertEndian(*((DWORD*)pSignedData));
			pSignedData += 4;
			nSignedLen -= 4;
			if (nSignedLen < methodLen)
			{
				free(buff);
				return LIBSSH2_ERROR_AGENT_PROTOCOL;
			}
			pSignedData += methodLen;
			nSignedLen -= methodLen;

			// read signature
			if (nSignedLen < 4)
			{
				free(buff);
				return LIBSSH2_ERROR_AGENT_PROTOCOL;
			}
			auto signatureLen = ConvertEndian(*((DWORD*)pSignedData));
			pSignedData += 4;
			nSignedLen -= 4;
			if (nSignedLen < signatureLen)
			{
				free(buff);
				return LIBSSH2_ERROR_AGENT_PROTOCOL;
			}
			auto newSig = malloc(sizeof(BYTE) * signatureLen);
			if (!newSig)
			{
				free(buff);
				return LIBSSH2_ERROR_ALLOC;
			}
			memcpy(newSig, pSignedData, sizeof(BYTE) * signatureLen);

			free(buff);
			*sig = static_cast<LPBYTE>(newSig);
			*sig_len = static_cast<size_t>(signatureLen);
			return 0;
		},
		&abstract);

	if (ret == LIBSSH2_ERROR_EAGAIN)
		return AuthReturnType::Again;

	if (ret != 0)
		return AuthReturnType::Error;

	delete pAuthSession;
	pAuth->put_AuthSession(reinterpret_cast<__int3264>(nullptr));
	return AuthReturnType::Success;
}
