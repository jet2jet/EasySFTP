#pragma once

#include "SSHAgent.h"

class CWinOpenSSHAgent : public CSSHAgent
{
public:
	CWinOpenSSHAgent() : m_hPipe(INVALID_HANDLE_VALUE), m_pACLKeep(NULL), m_pSecurityDescriptor(NULL) {}
	virtual ~CWinOpenSSHAgent();
	static bool IsAvailable();
	virtual bool Query(const void* dataSend, size_t dataSendSize, void** dataReceived, size_t* dataReceivedSize);

private:
	HANDLE m_hPipe;
	PACL m_pACLKeep;
	PSECURITY_DESCRIPTOR m_pSecurityDescriptor;
};
