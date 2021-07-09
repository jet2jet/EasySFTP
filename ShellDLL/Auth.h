/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 Auth.h - declarations of authentication classes for SSH
 */

#pragma once

#include "SUString.h"

#define USERINFO_SIGNATURE    0x1362109A

#include "Unknown.h"

class CUserInfo : public CUnknownImpl
{
public:
	CUserInfo() : dwSignature(USERINFO_SIGNATURE) { }
	~CUserInfo() { }
	DWORD dwSignature;
	CMyStringW strName;
	_SecureStringW strPassword, strNewPassword;
	char nAuthType;
	CMyStringW strPKeyFileName;
	const void* pvSessionID;
	size_t nSessionIDLen;
	bool bSecondary;
};

enum class AuthReturnType
{
	Success = 1,
	Again = 0,
	Error = -1
};

class __declspec(novtable) CAuthentication
{
public:
	virtual const char* GetAuthenticationType() = 0;
	inline bool IsMatchAuthenticationType(LPCSTR lpszName)
		{ return strcmp(lpszName, GetAuthenticationType()) == 0; }
	virtual AuthReturnType Authenticate(LIBSSH2_SESSION* pSession, CUserInfo* pUser, LPCSTR lpszService) = 0;
	virtual void FinishAndDelete() = 0;
	virtual bool CanRetry() { return false; }
};

class CPasswordAuthentication : public CAuthentication
{
public:
	CPasswordAuthentication() : m_lpszUser(NULL), m_dwUserLen(0), m_lpszPassword(NULL), m_dwPasswordLen(0) {}
	~CPasswordAuthentication();
	virtual const char* GetAuthenticationType()
		{ return "password"; }
	virtual AuthReturnType Authenticate(LIBSSH2_SESSION* pSession, CUserInfo* pUser, LPCSTR lpszService);
	virtual void FinishAndDelete()
		{ delete this; }
private:
	LPCSTR m_lpszUser;
	size_t m_dwUserLen;
	LPSTR m_lpszPassword;
	size_t m_dwPasswordLen;
};

class CChangePasswordAuthentication : public CAuthentication
{
public:
	CChangePasswordAuthentication() : m_lpszUser(NULL), m_dwUserLen(0), m_lpszPassword(NULL), m_dwPasswordLen(0) {}
	~CChangePasswordAuthentication();
	virtual const char* GetAuthenticationType()
		{ return "password"; }
	virtual AuthReturnType Authenticate(LIBSSH2_SESSION* pSession, CUserInfo* pUser, LPCSTR lpszService);
	virtual void FinishAndDelete()
		{ delete this; }
private:
	LPCSTR m_lpszUser;
	size_t m_dwUserLen;
	LPSTR m_lpszPassword;
	size_t m_dwPasswordLen;
};

class CPublicKeyAuthentication : public CAuthentication
{
public:
	CPublicKeyAuthentication() : m_lpszUser(NULL), m_dwUserLen(0), m_lpszPKeyFileName(NULL), m_lpszPassword(NULL), m_dwPasswordLen(0) {}
	~CPublicKeyAuthentication();
	virtual const char* GetAuthenticationType()
		{ return "publickey"; }
	virtual AuthReturnType Authenticate(LIBSSH2_SESSION* pSession, CUserInfo* pUser, LPCSTR lpszService);
	virtual void FinishAndDelete()
		{ delete this; }
private:
	LPCSTR m_lpszUser;
	size_t m_dwUserLen;
	LPCSTR m_lpszPKeyFileName;
	LPSTR m_lpszPassword;
	size_t m_dwPasswordLen;
};

class CSSHAgent;

class CPageantAuthentication : public CAuthentication
{
public:
	CPageantAuthentication();
	~CPageantAuthentication();
	virtual const char* GetAuthenticationType()
		{ return "publickey"; }
	virtual AuthReturnType Authenticate(LIBSSH2_SESSION* pSession, CUserInfo* pUser, LPCSTR lpszService);
	virtual void FinishAndDelete()
		{ delete this; }
	virtual bool CanRetry();

private:
	CSSHAgent* m_pAgent;
	LPCSTR m_lpszUser;
	LPBYTE m_lpPageantKeyList;
	LPBYTE m_lpCurrentKey;
	DWORD m_dwKeyCount;
	DWORD m_dwKeyIndex;
};

class CNoneAuthentication : public CAuthentication
{
public:
	CNoneAuthentication() : m_lpAuthList(NULL), m_lpszUser(NULL), m_dwUserLen(0) {}
	~CNoneAuthentication();
	virtual const char* GetAuthenticationType()
		{ return "none"; }
	virtual AuthReturnType Authenticate(LIBSSH2_SESSION* pSession, CUserInfo* pUser, LPCSTR lpszService);
	virtual void FinishAndDelete()
		{ delete this; }

	// such as: type1\0type2\0type3\0\0
	LPSTR m_lpAuthList;

private:
	LPCSTR m_lpszUser;
	size_t m_dwUserLen;
};
