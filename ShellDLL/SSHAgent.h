#pragma once

class __declspec(novtable) CSSHAgent
{
public:
	virtual ~CSSHAgent() {}
	virtual bool Query(const void* dataSend, size_t dataSendSize, void** dataReceived, size_t* dataReceivedSize) = 0;

public:
	int GetKeyList2(LPBYTE* ppKeyList);
	void* SignSSH2Key(LPCBYTE pszPubKey, LPCBYTE pszData, size_t nDataLen, size_t* pnOutLen);
	void FreeKeyList(LPBYTE pKeyList);
};
