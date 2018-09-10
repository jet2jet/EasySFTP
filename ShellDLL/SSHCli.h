/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 SSHCli.h - declarations of CSSH2Client
 */

#pragma once

#include "SSHSock.h"
#include "KexCore.h"
#include "Auth.h"
#include "ExBuffer.h"

#include "Unknown.h"

#define AUTHTYPE_NONE         0
#define AUTHTYPE_PASSWORD     1
#define AUTHTYPE_PUBLICKEY    2
#define AUTHTYPE_PAGEANT      3

class CSSH2Client : public CUnknownImpl
{
public:
	CSSH2Client(void);
	~CSSH2Client(void);

public:
	bool OnFirstReceive();
	// <0 : error, =0 : wait for first kex, >0 : succeeded
	int OnKeyExchangeInit(const void* pv, size_t nLen, bool bIgnoreMsg = false);
	// <0: error, =0: try again, >0: succeeded
	int OnReceiveKexMessages(CFingerPrintHandler* pHandler, BYTE bType, const void* pv, size_t nLen);
	void UpdateServerReceiveKeyData()
		{ m_socket.UpdateServerKeyData(false, m_keyDataStoC); }
	bool Authenticate(LPCSTR lpszAuthService, char nAuthType, const void* pvServiceData, size_t nDataLen, CUserInfo* pUserInfo);
	bool DoAuthenticate(char nAuthType, CUserInfo* pUserInfo);
	bool CanRetryAuthenticate();
	void EndAuthenticate();
	bool NoMoreSessions();

public:
	CSSH2Socket m_socket;
	CKeyExchange* m_pKex;
	CNewKeyData m_keyDataCtoS;
	CNewKeyData m_keyDataStoC;
	CMyStringW m_strServerName;

private:
	CExBuffer m_bufferMyProposal, m_bufferSvProposal;
	EVP_PKEY* m_pPKey;
	CAuthentication* m_pAuth;
	bool StartKeyExchange();
};
