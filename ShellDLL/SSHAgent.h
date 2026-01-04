#pragma once

/* Signature request methods */
#define SSH_AGENT_RSA_SHA2_256 2
#define SSH_AGENT_RSA_SHA2_512 4

class __declspec(novtable) CSSHAgent
{
public:
	virtual ~CSSHAgent() {}
	virtual bool Query(const void* dataSend, size_t dataSendSize, void** dataReceived, size_t* dataReceivedSize) = 0;

public:
	int GetKeyList2(LPBYTE* ppKeyList);
	void* SignSSH2Key(LPCBYTE pszPubKey, int flags, LPCBYTE pszData, size_t nDataLen, size_t* pnOutLen);
	void FreeKeyList(LPBYTE pKeyList);
};
