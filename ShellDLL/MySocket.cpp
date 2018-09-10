/*
 Copyright (C) 2010 Kuri-Applications

 MySocket.cpp - implementations of CMySocket, CTextSocket, and CFTPSocket
 */

#include "StdAfx.h"
#include "UString.h"
#include "SUString.h"
#include "MySocket.h"

#include "Convert.h"
#include "Unknown.h"

CMySocket::CMySocket(void)
{
	m_socket = INVALID_SOCKET;
	m_pAI = NULL;
	m_pAddress = NULL;
	m_nEnable = 1;
	m_hWndAsync = NULL;
	m_uMsg = (UINT) -1;
	m_lEvent = 0;
}

CMySocket::~CMySocket(void)
{
	Close();
}

bool CMySocket::Connect(int port, const char* pszHostName, int family, int socktype, int protocol)
{
	const addrinfo* pai;

	pai = TryConnect(port, pszHostName, family, socktype, protocol);
	if (!pai)
		return false;

	return Connect(pai);
}

const addrinfo* CMySocket::TryConnect(int port, const char* pszHostName, int family, int socktype, int protocol)
{
	addrinfo ai, * pai;
	int ret;
	char portText[12];

	if (m_socket != INVALID_SOCKET)
		return NULL;

	memset(&ai, 0, sizeof(ai));
	ai.ai_family = family;
	ai.ai_socktype = socktype;
	ai.ai_protocol = protocol;

	_snprintf_s(portText, 11, "%d", port);

	ret = ::getaddrinfo(pszHostName, portText, &ai, &pai);
	if (ret)
		return NULL;
	if (m_pAI)
		::freeaddrinfo(m_pAI);
	m_pAI = pai;
	return pai;
}

bool CMySocket::Connect(const addrinfo* pai, int port)
{
	int ret;
	sockaddr* paddr;

	if (m_pAddress)
	{
		free(m_pAddress);
		m_pAddress = NULL;
	}

	while (pai)
	{
		m_socket = ::socket(pai->ai_family, pai->ai_socktype, pai->ai_protocol);
		if (m_socket != INVALID_SOCKET)
		{
			if (port != -1)
			{
				register size_t len = pai->ai_addrlen;
				paddr = (sockaddr*) malloc(len);
				memcpy(paddr, pai->ai_addr, len);
				((sockaddr_in6*) paddr)->sin6_port = htons((u_short) port);
				ret = ::connect(m_socket, paddr, (int) len);
			}
			else
			{
				paddr = NULL;
				ret = ::connect(m_socket, pai->ai_addr, (int) pai->ai_addrlen);
			}
			if (!ret)
			{
				if (paddr)
					m_pAddress = paddr;
				else
				{
					m_pAddress = (sockaddr*) malloc(pai->ai_addrlen);
					memcpy(m_pAddress, pai->ai_addr, pai->ai_addrlen);
				}
				m_nAddrLen = pai->ai_addrlen;
				return true;
			}
			::closesocket(m_socket);
			if (paddr)
				free(paddr);
			m_socket = INVALID_SOCKET;
		}
		pai = pai->ai_next;
	}
	return false;
}

void CMySocket::Close()
{
	if (m_socket != INVALID_SOCKET)
	{
		::closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
	if (m_pAddress)
	{
		free(m_pAddress);
		m_pAddress = NULL;
	}
	if (m_pAI)
	{
		::freeaddrinfo(m_pAI);
		m_pAI = NULL;
	}
}

bool CMySocket::AsyncSelect(HWND hWnd, UINT uMsg, long lEvent)
{
	if (m_socket == INVALID_SOCKET)
		return false;
	if (::WSAAsyncSelect(m_socket, hWnd, uMsg, lEvent) != 0)
		return false;
	m_nEnable = 1;
	m_hWndAsync = hWnd;
	m_uMsg = uMsg;
	m_lEvent = lEvent;
	return true;
}

bool CMySocket::CanReceive(DWORD dwWaitMilliseconds) const
{
	fd_set fset;
	timeval tv;
	int ret;

	if (m_socket == INVALID_SOCKET)
		return false;

	FD_ZERO(&fset);
	FD_SET(m_socket, &fset);
	tv.tv_sec = (long) (dwWaitMilliseconds / 1000);
	tv.tv_usec = (dwWaitMilliseconds % 1000) * 1000;
	ret = ::select(1, &fset, NULL, NULL, &tv);
	if (ret == SOCKET_ERROR)
		return false;
	if (!ret)
		return false;
	if (!FD_ISSET(m_socket, &fset))
		return false;
	ret = ::recv(m_socket, (char*) &tv, 1, MSG_PEEK);
	if (ret < 1)
	{
#ifdef _DEBUG
		if (ret < 0)
		{
			CMyStringW str;
			int err = ::WSAGetLastError();
			str.Format(L"WSA error has occurred: %d\n", err);
			OutputDebugString(str);
		}
#endif
		return false;
	}
	return true;
}

bool CMySocket::IsRemoteClosed() const
{
	fd_set fset;
	timeval tv;
	int ret;

	if (m_socket == INVALID_SOCKET)
		return true;

	FD_ZERO(&fset);
	FD_SET(m_socket, &fset);
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	ret = ::select(1, &fset, NULL, NULL, &tv);
	if (ret == SOCKET_ERROR)
		return false;
	if (!ret)
		return false;
	if (!FD_ISSET(m_socket, &fset))
		return false;
	ret = ::recv(m_socket, (char*) &tv, 1, MSG_PEEK);
	return ret == 0;
}

bool CMySocket::EnableAsyncSelect(bool bEnable, bool bUseRefCount)
{
	if (m_socket == INVALID_SOCKET || m_uMsg == (UINT) -1)
		return false;
	int ne = m_nEnable;
	if (bUseRefCount)
	{
		if (bEnable)
			ne++;
		else
			ne--;
		bEnable = (ne > 0);
	}
	else
	{
		ne = bEnable ? 1 : 0;
	}
	if ((m_nEnable > 0) == bEnable)
		return true;
#ifdef _DEBUG
	//{
	//	CMyStringW str;
	//	str.Format(L"EnableAsyncSelect: socket = 0x%p, bEnable = %s\n", (void*) m_socket, bEnable ? L"true" : L"false");
	//	OutputDebugString(str);
	//}
#endif
	if (::WSAAsyncSelect(m_socket, m_hWndAsync, m_uMsg, bEnable ? m_lEvent : 0) != 0)
		return false;
	if (!bEnable)
	{
		ULONG u = 0;
		::ioctlsocket(m_socket, FIONBIO, &u);
		// remove all messages whose wParam is equal to m_socket
		MSG msg;
		int nCount = 16, i = 0;
		MSG* pmsgCache = (MSG*) malloc(sizeof(MSG) * nCount);
		while (::PeekMessage(&msg, m_hWndAsync, m_uMsg, m_uMsg, PM_REMOVE))
		{
			if ((SOCKET) msg.wParam != m_socket)
			{
				if (i == nCount)
				{
					nCount += 16;
					MSG* pmg = (MSG*) realloc(pmsgCache, sizeof(MSG) * nCount);
					if (!pmg)
						break;
					pmsgCache = pmg;
				}
				memcpy(&pmsgCache[i++], &msg, sizeof(MSG));
			}
		}
		while (i--)
			::PostMessage(pmsgCache[i].hwnd, pmsgCache[i].message, pmsgCache[i].wParam, pmsgCache[i].lParam);
		free(pmsgCache);
	}
	m_nEnable = ne;
	return true;
}

////////////////////////////////////////////////////////////////////////////////

CTextSocket::CTextSocket()
{
	m_charset = scsUTF8;
}

#define RECV_BUFFER_SIZE  1024

static bool __stdcall _ExpandBuffer(LPBYTE& lpBuffer, size_t nPos, size_t& nBufferSize)
{
	register LPBYTE lpNewBuffer;
	if (nPos >= nBufferSize)
	{
		lpNewBuffer = (LPBYTE) realloc(lpBuffer, nBufferSize + RECV_BUFFER_SIZE);
		if (!lpNewBuffer)
			return false;
		lpBuffer = lpNewBuffer;
		nBufferSize += RECV_BUFFER_SIZE;
	}
	return true;
}

int CTextSocket::SendString(const CMyStringW& string)
{
	register LPBYTE lp;
	CMyStringW buff(string);
	LPCBYTE lpSend;
	size_t dwSize;
	int ret;

	lp = NULL;
	switch (m_charset)
	{
		case scsUTF8:
			lpSend = buff.AllocUTF8String(&dwSize);
			break;
		case scsShiftJIS:
			dwSize = buff.GetLengthA();
			lpSend = (LPCBYTE)(LPCSTR) buff;
			break;
		case scsEUC:
			dwSize = buff.GetLengthA();
			lpSend = (LPCBYTE)(LPCSTR) buff;
			lp = (LPBYTE) malloc((size_t) dwSize);
			memcpy(lp, lpSend, (size_t) dwSize);
			::ShiftJISToEUCString((LPSTR) lp, dwSize);
			lpSend = lp;
			break;
		default:
			return 0;
	}
	ret = Send(lpSend, (SIZE_T) dwSize, 0);
	if (lp)
		free(lp);
	return ret;
}

int CTextSocket::SecureSendString(const _SecureStringW& string)
{
	register LPBYTE lp;
	CMyStringW buff;
	LPCBYTE lpSend;
	size_t dwSize;
	int ret;

	string.GetString(buff);
	lp = NULL;
	switch (m_charset)
	{
		case scsUTF8:
			lpSend = buff.AllocUTF8String(&dwSize);
			break;
		case scsShiftJIS:
			dwSize = buff.GetLengthA();
			lpSend = (LPCBYTE)(LPCSTR) buff;
			break;
		case scsEUC:
			dwSize = buff.GetLengthA();
			lpSend = (LPCBYTE)(LPCSTR) buff;
			lp = (LPBYTE) malloc((size_t) dwSize);
			memcpy(lp, lpSend, (size_t) dwSize);
			::ShiftJISToEUCString((LPSTR) lp, dwSize);
			lpSend = lp;
			break;
		default:
			_SecureStringW::SecureEmptyString(buff);
			return 0;
	}
	ret = Send(lpSend, (SIZE_T) dwSize, 0);
	if (lp)
	{
		_SecureStringW::SecureEmptyBuffer(lp, (size_t) dwSize);
		free(lp);
	}
	_SecureStringW::SecureEmptyString(buff);
	return ret;
}

bool CTextSocket::ReceiveLine(CMyStringW& ret)
{
	LPBYTE lpBuffer, lpPos;
	BYTE b, b2;
	size_t nBufferSize;
	int iRet;

	ret.Empty();
	nBufferSize = RECV_BUFFER_SIZE;
	lpBuffer = (LPBYTE) malloc(RECV_BUFFER_SIZE);
	if (!lpBuffer)
		return false;
	lpPos = lpBuffer;
	b2 = 0;
	while (true)
	{
		iRet = Recv(&b, 1, 0);
		if (iRet < 0)
		{
			free(lpBuffer);
			return false;
		}
		if (!iRet)
			break;
		//if (b == '\n' && b2 == '\r')
		if (b == '\n')
			break;
		else
		{
			if (b2 == '\r')
			{
				*lpPos++ = b2;
				if (!_ExpandBuffer(lpBuffer, lpPos - lpBuffer, nBufferSize))
					break;
			}
			if (b != '\r')
				*lpPos++ = b;
			if (!_ExpandBuffer(lpBuffer, lpPos - lpBuffer, nBufferSize))
				break;
			b2 = b;
		}
	}
	if (lpBuffer == lpPos)
		return false;
	nBufferSize = (size_t) (lpPos - lpBuffer);
	switch (m_charset)
	{
		case scsUTF8:
			ret.SetUTF8String(lpBuffer, (DWORD) nBufferSize);
			break;
		case scsEUC:
			::EUCToShiftJISString((LPSTR) lpBuffer, (DWORD) nBufferSize);
		case scsShiftJIS:
			ret.SetString((LPCSTR) lpBuffer, (DWORD) nBufferSize);
			break;
		default:
			break;
	}
	free(lpBuffer);
	return true;
}
