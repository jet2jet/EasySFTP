/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 Auth.h - declarations of authentication classes for SSH
 */

#pragma once

#include "SUString.h"
#include "SSHSock.h"
#include "KexCore.h"

#define USERINFO_SIGNATURE    0x1362109A

#include "Unknown.h"

class CUserInfo : public CUnknownImpl
{
public:
	CUserInfo() : dwSignature(USERINFO_SIGNATURE) { }
	~CUserInfo() { keyData = (void*) NULL; }
	DWORD dwSignature;
	CMyStringW strName;
	_SecureStringW strPassword, strNewPassword;
	char nAuthType;
	KeyData keyData;
	KeyType keyType;
	const void* pvSessionID;
	size_t nSessionIDLen;
	// freed by CPageantAuthentication
	LPBYTE lpPageantKeyList;
	bool bSecondary;
};

class __declspec(novtable) CAuthentication
{
public:
	virtual const char* GetAuthenticationType() = 0;
	inline bool IsMatchAuthenticationType(LPCSTR lpszName)
		{ return strcmp(lpszName, GetAuthenticationType()) == 0; }
	virtual bool Authenticate(CSSH2Socket* pSocket, CUserInfo* pUser, LPCSTR lpszService) = 0;
	virtual void FinishAndDelete() = 0;
	virtual bool CanRetry() { return false; }
};

class CPasswordAuthentication : public CAuthentication
{
public:
	virtual const char* GetAuthenticationType()
		{ return "password"; }
	virtual bool Authenticate(CSSH2Socket* pSocket, CUserInfo* pUser, LPCSTR lpszService);
	virtual void FinishAndDelete()
		{ delete this; }
};

class CChangePasswordAuthentication : public CAuthentication
{
public:
	virtual const char* GetAuthenticationType()
		{ return "password"; }
	virtual bool Authenticate(CSSH2Socket* pSocket, CUserInfo* pUser, LPCSTR lpszService);
	virtual void FinishAndDelete()
		{ delete this; }
};

class CPublicKeyAuthentication : public CAuthentication
{
public:
	virtual const char* GetAuthenticationType()
		{ return "publickey"; }
	virtual bool Authenticate(CSSH2Socket* pSocket, CUserInfo* pUser, LPCSTR lpszService);
	virtual void FinishAndDelete()
		{ delete this; }
};

class CPageantAuthentication : public CAuthentication
{
public:
	CPageantAuthentication() : m_lpPageantKeyList(NULL), m_lpCurrentKey(NULL), m_dwKeyCount(0) { }
	virtual const char* GetAuthenticationType()
		{ return "publickey"; }
	virtual bool Authenticate(CSSH2Socket* pSocket, CUserInfo* pUser, LPCSTR lpszService);
	virtual void FinishAndDelete();
	virtual bool CanRetry();

	LPBYTE m_lpPageantKeyList;
	LPBYTE m_lpCurrentKey;
	DWORD m_dwKeyCount;
	DWORD m_dwKeyIndex;
};

class CNoneAuthentication : public CAuthentication
{
public:
	virtual const char* GetAuthenticationType()
		{ return "none"; }
	virtual bool Authenticate(CSSH2Socket* pSocket, CUserInfo* pUser, LPCSTR lpszService);
	virtual void FinishAndDelete()
		{ delete this; }
};
