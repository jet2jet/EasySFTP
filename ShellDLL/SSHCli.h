/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 SSHCli.h - declarations of CSSH2Client
 */

#pragma once

#include "MySocket.h"
#include "Auth.h"
#include "ExBuffer.h"

#include "Unknown.h"

#define AUTHTYPE_NONE         0
#define AUTHTYPE_PASSWORD     1
#define AUTHTYPE_PUBLICKEY    2
#define AUTHTYPE_PAGEANT      3
#define AUTHTYPE_WINSSHAGENT  4

class __declspec(novtable) CSSH2FingerPrintHandler
{
public:
	virtual bool __stdcall CheckFingerPrint(const BYTE* pFingerPrint, size_t nLen) = 0;
};

class CSSH2Client : public CUnknownImpl
{
public:
	CSSH2Client(void);
	~CSSH2Client(void);

public:
	LIBSSH2_SESSION* GetSession() const { return m_pSession; }
	bool OnFirstReceive();
	// <0 : error, =0 : wait for first kex, >0 : succeeded
	int OnHandshake(CSSH2FingerPrintHandler* pHandler);
	AuthReturnType Authenticate(char nAuthType, CUserInfo* pUserInfo);
	bool CanRetryAuthenticate();
	void EndAuthenticate();
	LPSTR AvailableAuthTypes();
	void SendKeepAlive();

public:
	CMySocket m_socket;
	CMyStringW m_strServerName;

private:
	LIBSSH2_SESSION* m_pSession;
	CExBuffer m_bufferMyProposal, m_bufferSvProposal;
	CAuthentication* m_pAuth;
	char m_nAuthType;
	bool StartKeyExchange();
};
