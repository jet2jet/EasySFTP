/*
 Copyright (C) 2010 Kuri-Applications

 MySocket.h - declarations of CMySocket, CTextSocket, and CFTPSocket
 */

#pragma once

#include "UString.h"
#include "SUString.h"

enum ServerCharset
{
	scsUTF8 = 0,
	scsShiftJIS,
	scsEUC
};

class CMySocket
{
public:
	CMySocket();
	virtual ~CMySocket();

public:
	operator SOCKET() const { return m_socket; }

	bool Connect(int port, const char* pszHostName,
		int family = AF_UNSPEC,
		int socktype = 0,
		int protocol = PF_UNSPEC);
	bool Connect(const addrinfo* pai, int port = -1);
	const addrinfo* TryConnect(int port, const char* pszHostName,
		int family = AF_UNSPEC,
		int socktype = 0,
		int protocol = PF_UNSPEC);
	virtual void Close();
	bool IsConnected() const { return m_socket != INVALID_SOCKET; }
	virtual int Send(LPCVOID lpBuffer, SIZE_T nSize, int flags, bool* pbNeedRepeat = NULL)
	{
		if (pbNeedRepeat)
			*pbNeedRepeat = false;
		return ::send(m_socket, (const char*)lpBuffer, (int)nSize, flags);
	}
	virtual int Recv(LPVOID lpBuffer, SIZE_T nSize, int flags, bool* pbNeedMoreData = NULL)
	{
		if (pbNeedMoreData)
			*pbNeedMoreData = false;
		return ::recv(m_socket, (char*)lpBuffer, (int)nSize, flags);
	}
	bool AsyncSelect(HWND hWnd, UINT uMsg, long lEvent);
	bool SetSocketOption(int nLevel, int nOptionName, const void* pValue, SIZE_T nLen)
	{
		return ::setsockopt(m_socket, nLevel, nOptionName, (const char*)pValue, (int)nLen) == 0;
	}
	bool IoControl(long nCmd, DWORD dwArgument)
	{
		return ::ioctlsocket(m_socket, nCmd, &dwArgument) == 0;
	}
	bool IoControl(long nCmd, DWORD* pdwArgument)
	{
		return ::ioctlsocket(m_socket, nCmd, pdwArgument) == 0;
	}

	virtual bool CanReceive(DWORD dwWaitMilliseconds = 0, bool* pbIsError = NULL) const;
	bool IsRemoteClosed() const;
	bool EnableAsyncSelect(bool bEnable, bool bUseRefCount = false);

	//protected:
	const addrinfo* GetThisAddrInfo() const { return m_pAI; }
	const sockaddr* GetConnectedAddress(size_t* pnAddrLen) const
	{
		*pnAddrLen = m_nAddrLen; return m_pAddress;
	}

private:
	SOCKET m_socket;
	addrinfo* m_pAI;
	sockaddr* m_pAddress;
	size_t m_nAddrLen;

protected:
	int m_nEnable;
	HWND m_hWndAsync;
	UINT m_uMsg;
	long m_lEvent;
};

class CTextSocket : public CMySocket
{
public:
	CTextSocket();

	void SetCharset(ServerCharset charset) { m_charset = charset; }

	int SendString(const CMyStringW& string);
	int SecureSendString(const _SecureStringW& string);
	bool ReceiveLine(CMyStringW& ret, bool (*pfnPumpMessage)() = NULL);

private:
	ServerCharset m_charset;
};
