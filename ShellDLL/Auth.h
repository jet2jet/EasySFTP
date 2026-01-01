/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 Auth.h - declarations of authentication classes for SSH
 */

#pragma once

#include "SUString.h"

#define USERINFO_SIGNATURE    0x1362109A

#include "EasySFTP_h.h"
#include "Unknown.h"
#include "Dispatch.h"

enum class AuthReturnType
{
	Success = 1,
	Again = 0,
	Error = -1
};

class CSSHAgent;

#define AUTH_SESSION_SIGNATURE 0xa0b29dff

struct CAuthSession
{
	union
	{
		DWORD dwSignature;
		void* _padding;
	};
	CSSHAgent* pAgent;
	LPBYTE lpPageantKeyList;
	LPBYTE lpCurrentKey;
	DWORD dwKeyCount;
	DWORD dwKeyIndex;

	~CAuthSession();
};

class CAuthentication : public CDispatchImplT<IEasySFTPAuthentication2>
{
public:
	CAuthentication();
	virtual ~CAuthentication();

	virtual void* GetThisForDispatch() override { return static_cast<IEasySFTPAuthentication*>(this); }

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override;

	STDMETHOD(get_UserName)(BSTR* pRet) override;
	STDMETHOD(put_UserName)(BSTR Name) override;
	STDMETHOD(get_Password)(BSTR* pRet) override;
	STDMETHOD(put_Password)(BSTR Password) override;
	STDMETHOD(get_PrivateKeyFileName)(BSTR* pRet) override;
	STDMETHOD(put_PrivateKeyFileName)(BSTR FileName) override;
	STDMETHOD(get_Type)(EasySFTPAuthenticationMode* pMode) override;
	STDMETHOD(put_Type)(EasySFTPAuthenticationMode mode) override;
	STDMETHOD(get_AuthSession)(LONG_PTR* pOut) override;
	STDMETHOD(put_AuthSession)(LONG_PTR session) override;

	STDMETHOD(SetPrivateKeyBinary)(const void* buffer, long length) override;
	STDMETHOD(GetPrivateKeyBinary)(const void** buffer, long* pLength) override;

	static AuthReturnType SSHAuthenticate(IEasySFTPAuthentication2* pAuth, LIBSSH2_SESSION* pSession, LPCSTR lpszService, char** ppAuthList = NULL);
	static bool CanRetry(IEasySFTPAuthentication* pAuth);

private:
	static AuthReturnType SSHAuthenticateWithAgent(IEasySFTPAuthentication* pAuth, CMyStringW& strUserName, LIBSSH2_SESSION* pSession, LPCSTR lpszService, CSSHAgent* (* CreateAgent)());

public:
	EasySFTPAuthenticationMode m_Mode;
	CMyStringW m_strUserName;
	_SecureStringW m_strPassword;
	CMyStringW m_strPrivateKeyFileName;
	void* m_pPrivateKeyData;
	long m_nPrivateKeyData;
	void* m_pSession;
};
