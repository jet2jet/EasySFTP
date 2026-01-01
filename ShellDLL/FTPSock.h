/*
 EasySFTP - Copyright (C) 2025 jet (ジェット)

 FTPSock.h - declaration of CFTPSocket
 */

#pragma once

#include "MySocket.h"

class CFTPSocket : public CTextSocket
{
public:
	CFTPSocket();
	virtual ~CFTPSocket();

	virtual void Close() override;
	virtual int Send(LPCVOID lpBuffer, SIZE_T nSize, int flags, bool* pbNeedRepeat = NULL) override;
	virtual int Recv(LPVOID lpBuffer, SIZE_T nSize, int flags, bool* pbNeedMoreData = NULL) override;
	virtual bool CanReceive(DWORD dwWaitMilliseconds = 0, bool* pbIsError = NULL) const override;

	enum class HandshakeResult : char
	{
		Success = 1,
		Error = 0,
		Waiting = -1
	};
	HandshakeResult StartHandshake();
	HandshakeResult ContinueHandshake();

private:
	SSL_CTX* m_pSSLCTX;
	SSL* m_pSSL;
};
