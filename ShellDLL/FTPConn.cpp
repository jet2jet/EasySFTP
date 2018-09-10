/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 FTPConn.cpp - implementations of FTP connection (command exchange, synchronization, and so on)
 */

#include "StdAfx.h"
#include "ShellDLL.h"
#include "FTPConn.h"

CFTPConnection::CFTPConnection(void)
{
	m_lpszAvailableCommands = NULL;
	::InitializeCriticalSection(&m_csSocket);
}

CFTPConnection::~CFTPConnection(void)
{
	Close();
	::DeleteCriticalSection(&m_csSocket);

	CFTPWaitResponse* p;
	while (m_aWaitResponse.Dequeue(&p))
		delete p;
	if (m_lpszAvailableCommands)
		free(m_lpszAvailableCommands);
}

bool CFTPConnection::Connect(int nPort, LPCWSTR lpszHostName)
{
	CMyStringW strHostName(lpszHostName);
	::EnterCriticalSection(&m_csSocket);
	const addrinfo* pai = m_socket.TryConnect(nPort, strHostName);
	if (!pai)
	{
		::LeaveCriticalSection(&m_csSocket);
		return false;
	}
	//SetStatusText(MAKEINTRESOURCEW(IDS_CONNECTING));
	if (!m_socket.CMySocket::Connect(pai))
	{
		::LeaveCriticalSection(&m_csSocket);
		return false;
	}
	CFTPWaitResponse* p = new CFTPWaitResponse();
	p->pWait = NULL;
	m_aWaitResponse.Add(p);
	::LeaveCriticalSection(&m_csSocket);
	return true;
}

void CFTPConnection::Close()
{
	::EnterCriticalSection(&m_csSocket);
	m_socket.Close();
	::LeaveCriticalSection(&m_csSocket);
}

CFTPWaitResponse* CFTPConnection::SendCommand(LPCWSTR lpszCommand, LPCWSTR lpszParam,
	CWaitResponseData* pWait)
{
	CFTPWaitResponse* p = new CFTPWaitResponse();
	p->strCommand = lpszCommand;
	p->strParameter = lpszParam;
	p->pWait = pWait;

	CMyStringW strMsg(lpszCommand);
	if (lpszParam)
	{
		strMsg += L' ';
		strMsg += lpszParam;
	}
	strMsg += L"\r\n";

	::EnterCriticalSection(&m_csSocket);
	m_socket.SendString(strMsg);
	m_aWaitResponse.Enqueue(p);

#ifdef _DEBUG
	{
		CMyStringW str;
		str.Format(L"(wait = %p) > ", pWait);
		strMsg.InsertString(str, 0);
		OutputDebugString(strMsg);
	}
#endif
	::LeaveCriticalSection(&m_csSocket);

	return p;
}

CFTPWaitResponse* CFTPConnection::SecureSendCommand(LPCWSTR lpszCommand,
	const _SecureStringW& strParam,
	CWaitResponseData* pWait)
{
	CFTPWaitResponse* p = new CFTPWaitResponse();
	p->strCommand = lpszCommand;
	//p->strParameter = strParam;
	p->pWait = pWait;

	_SecureStringW strMsg(lpszCommand);
	strMsg += L' ';
	strMsg += strParam;
	strMsg += L"\r\n";

	::EnterCriticalSection(&m_csSocket);
	m_socket.SecureSendString(strMsg);
	m_aWaitResponse.Enqueue(p);
	strMsg.Empty();

#ifdef _DEBUG
	{
		CMyStringW str2;
		str2.Format(L"(wait = %p) > ", pWait);
		str2 += lpszCommand;
		str2 += L" ****\n";
		OutputDebugString(str2);
	}
#endif
	::LeaveCriticalSection(&m_csSocket);

	return p;
}

CFTPWaitResponse* CFTPConnection::SendCommandWithType(LPCWSTR lpszCommand, LPCWSTR lpszParam, LPCWSTR lpszType,
	CWaitResponseData* pWait)
{
	return SendDoubleCommand(L"TYPE", lpszType, lpszCommand, lpszParam, NULL, pWait);
}

CFTPWaitResponse* CFTPConnection::SendDoubleCommand(LPCWSTR lpszCommand1, LPCWSTR lpszParam1,
	LPCWSTR lpszCommand2, LPCWSTR lpszParam2,
	CWaitResponseData* pWait1, CWaitResponseData* pWait2)
{
	::EnterCriticalSection(&m_csSocket);
	CFTPWaitResponse* p = new CFTPWaitResponse();
	p->strCommand = lpszCommand1;
	p->strParameter = lpszParam1;
	p->pWait = pWait1;

	CMyStringW strMsg(lpszCommand1);
	if (lpszParam1)
	{
		strMsg += L' ';
		strMsg += lpszParam1;
	}
	strMsg += L"\r\n";

	m_socket.SendString(strMsg);
	m_aWaitResponse.Enqueue(p);

#ifdef _DEBUG
	{
		CMyStringW str;
		str.Format(L"(wait = %p) > ", pWait1);
		strMsg.InsertString(str, 0);
		OutputDebugString(strMsg);
	}
#endif

	p = new CFTPWaitResponse();
	p->strCommand = lpszCommand2;
	p->strParameter = lpszParam2;
	p->pWait = pWait2;

	strMsg = lpszCommand2;
	if (lpszParam2)
	{
		strMsg += L' ';
		strMsg += lpszParam2;
	}
	strMsg += L"\r\n";

	m_socket.SendString(strMsg);
	m_aWaitResponse.Enqueue(p);

#ifdef _DEBUG
	{
		CMyStringW str;
		str.Format(L"(wait = %p) > ", pWait2);
		strMsg.InsertString(str, 0);
		OutputDebugString(strMsg);
	}
#endif

	::LeaveCriticalSection(&m_csSocket);

	return p;
}

CFTPWaitResponse* CFTPConnection::SendTripleCommand(LPCWSTR lpszCommand1, LPCWSTR lpszParam1,
	LPCWSTR lpszCommand2, LPCWSTR lpszParam2,
	LPCWSTR lpszCommand3, LPCWSTR lpszParam3,
	CWaitResponseData* pWait1, CWaitResponseData* pWait2, CWaitResponseData* pWait3)
{
	::EnterCriticalSection(&m_csSocket);
	CFTPWaitResponse* p = new CFTPWaitResponse();
	p->strCommand = lpszCommand1;
	p->pWait = pWait1;

	CMyStringW strMsg(lpszCommand1);
	if (lpszParam1)
	{
		p->strParameter = lpszParam1;
		strMsg += L' ';
		strMsg += lpszParam1;
	}
	strMsg += L"\r\n";

	m_socket.SendString(strMsg);
	m_aWaitResponse.Enqueue(p);

#ifdef _DEBUG
	{
		CMyStringW str;
		str.Format(L"(wait = %p) > ", pWait1);
		strMsg.InsertString(str, 0);
		OutputDebugString(strMsg);
	}
#endif

	p = new CFTPWaitResponse();
	p->strCommand = lpszCommand2;
	p->pWait = pWait2;

	strMsg = lpszCommand2;
	if (lpszParam2)
	{
		p->strParameter = lpszParam2;
		strMsg += L' ';
		strMsg += lpszParam2;
	}
	strMsg += L"\r\n";

	m_socket.SendString(strMsg);
	m_aWaitResponse.Enqueue(p);

#ifdef _DEBUG
	{
		CMyStringW str;
		str.Format(L"(wait = %p) > ", pWait2);
		strMsg.InsertString(str, 0);
		OutputDebugString(strMsg);
	}
#endif

	p = new CFTPWaitResponse();
	p->strCommand = lpszCommand3;
	p->pWait = pWait3;

	strMsg = lpszCommand3;
	if (lpszParam3)
	{
		p->strParameter = lpszParam3;
		strMsg += L' ';
		strMsg += lpszParam3;
	}
	strMsg += L"\r\n";

	m_socket.SendString(strMsg);
	m_aWaitResponse.Enqueue(p);

#ifdef _DEBUG
	{
		CMyStringW str;
		str.Format(L"(wait = %p) > ", pWait2);
		strMsg.InsertString(str, 0);
		OutputDebugString(strMsg);
	}
#endif

	::LeaveCriticalSection(&m_csSocket);

	return p;
}

bool CFTPConnection::ReceiveMessage(int& nCode, CMyStringW& rstrMessage, CWaitResponseData** ppWait,
	CMyStringW* pstrCommand)
{
	rstrMessage.Empty();
	::EnterCriticalSection(&m_csSocket);

	LPCWSTR lp, lp2, lp3;
	bool bMultiple, bSecond;
	CMyStringW str;
	int r;
	//bool b = (m_nEnable > 0);

	if (m_socket.IsRemoteClosed())
	{
		*ppWait = NULL;
		nCode = -1;
		::LeaveCriticalSection(&m_csSocket);
		return false;
	}
	if (!m_socket.CanReceive(WAIT_RECEIVE_TIME))
	{
		*ppWait = NULL;
		nCode = -1;
		::LeaveCriticalSection(&m_csSocket);
		return false;
	}

	bMultiple = false;
	bSecond = false;
	while (true)
	{
		while (!m_socket.ReceiveLine(str))
		{
			if (!bSecond || !m_socket.CanReceive(WAIT_RECEIVE_TIME))
			{
				*ppWait = NULL;
				nCode = -1;
				rstrMessage.Empty();
				::LeaveCriticalSection(&m_csSocket);
				return false;
			}
		}
		lp = wcschr(lp2 = str, L' ');
		lp3 = wcschr(lp2, L'-');
		if (bSecond)
		{
			if (lp == lp2)
				bMultiple = true;
			else if (lp3 && (!lp || lp3 < lp))
			{
				lp = lp3;
				bMultiple = true;
			}
			else
			{
				if (!lp)
				{
					*ppWait = NULL;
					nCode = -1;
					rstrMessage.Empty();
					::LeaveCriticalSection(&m_csSocket);
					return false;
				}
				if (lp != lp2 + 1)
					bSecond = false;
			}
			rstrMessage.AppendChar(L'\n');
		}
		else if (bMultiple = (lp3 && (!lp || lp3 < lp)))
			lp = lp3;
		if (lp)
		{
			lp++;
			register DWORD dwl = ((DWORD)((LPCBYTE) lp - (LPCBYTE) lp2)) / sizeof(WCHAR);
			rstrMessage.AppendString(lp, str.GetLength() - dwl);
			str.ReleaseBuffer(dwl);
		}
		lp = lp2 = str;
		if (!bSecond)
		{
			r = (int) wcstol(lp2, (wchar_t**) &lp, 10);
			if (lp == lp2)
			{
				nCode = -1;
				*ppWait = NULL;
				rstrMessage.Empty();
				::LeaveCriticalSection(&m_csSocket);
				return false;
			}
		}
		if (bMultiple)
		{
			bSecond = true;
			bMultiple = false;
		}
		else
			break;
	}
	nCode = r;

	CFTPWaitResponse* p;
	bool b = m_aWaitResponse.Dequeue(&p);

#ifdef _DEBUG
	{
		CMyStringW str;
		str.Format(L"FTP: %d %s (wait = %p)\n", nCode, (LPCWSTR) rstrMessage, b ? p->pWait : (void*) NULL);
		::OutputDebugString(str);
	}
#endif
	::LeaveCriticalSection(&m_csSocket);

	if (b)
	{
		*ppWait = p->pWait;
		if (pstrCommand)
			*pstrCommand = p->strCommand;
		delete p;
	}
	else
	{
		*ppWait = NULL;
		if (pstrCommand)
			pstrCommand->Empty();
	}
	return true;
}

bool CFTPConnection::ReceivePassive(CFTPWaitPassive* pPassive)
{
	//::EnterCriticalSection(&m_csSocket);
	if (pPassive->pPassive->IsRemoteClosed())
	{
		pPassive->bWaiting = true;
		pPassive->nWaitFlags = CFTPWaitPassive::flagFinished;
		pPassive->pMessage->EndReceive(NULL);

		ClosePassiveSocket(pPassive->pPassive, pPassive);
		//delete pPassive->pPassive;
		//pPassive->pPassive = NULL;
		return true;
	}
	if (!pPassive->pMessage->OnReceive(pPassive->pPassive) &&
		!pPassive->pPassive->IsRemoteClosed())
	{
		pPassive->bWaiting = false;
		pPassive->nWaitFlags = CFTPWaitPassive::flagError;
		pPassive->pMessage->EndReceive(NULL);
		//break;
		//::LeaveCriticalSection(&m_csSocket);
		return false;
	}
	//::LeaveCriticalSection(&m_csSocket);
	return true;
}

void CFTPConnection::ClosePassiveSocket(CTextSocket* pPassive, CWaitResponseData* pWait)
{
	::EnterCriticalSection(&m_csSocket);
	pPassive->Close();

	CFTPWaitResponse* p = new CFTPWaitResponse();
	p->pWait = pWait;
	m_aWaitResponse.Enqueue(p);
#ifdef _DEBUG
	{
		CMyStringW str;
		str.Format(L"(wait = %p) : wait for 226 msg\n", pWait);
		OutputDebugString(str);
	}
#endif
	::LeaveCriticalSection(&m_csSocket);
}

void CFTPConnection::InitAvaliableCommands(LPCWSTR lpszParam)
{
	if (m_lpszAvailableCommands)
		free(m_lpszAvailableCommands);
	if (!lpszParam)
	{
		m_lpszAvailableCommands = NULL;
		return;
	}

	CMyStringW str;
	LPCWSTR lpw, lpw2, lpw3;
	lpw = wcschr(lpszParam, '\n');
	if (lpw)
	{
		lpw++;
		while (*lpw == L' ')
			lpw++;
		lpw2 = lpw;
		lpw3 = wcschr(lpw2, '\n');
		while (lpw3)
		{
			str.AppendString(lpw2, ((DWORD) ((LPCBYTE) lpw3 - (LPCBYTE) lpw2)) / sizeof(WCHAR));
			str.AppendChar(L'\0');
			lpw2 = lpw3 + 1;
			while (*lpw2 == L' ')
				lpw2++;
			lpw3 = wcschr(lpw2, '\n');
		}
	}

	register LPWSTR w;
	size_t nLen = sizeof(WCHAR) * (str.GetLength() + 1);
	w = (LPWSTR) malloc(nLen);
	if (w)
	{
		if (str.IsEmpty())
			*w = 0;
		else
			memcpy(w, (LPCWSTR) str, nLen);
	}
	m_lpszAvailableCommands = w;
}

LPCWSTR CFTPConnection::IsCommandAvailable(LPCWSTR lpszCommand) const
{
	if (!lpszCommand)
		return NULL;
	if (!m_lpszAvailableCommands)
		return NULL;
	LPWSTR lpw = m_lpszAvailableCommands;
	size_t nLen = wcslen(lpszCommand);
	while (*lpw)
	{
		if (_wcsnicmp(lpw, lpszCommand, nLen) == 0 &&
			(!lpw[nLen] || lpw[nLen] == L' '))
			return lpw;
		while (*lpw++);
	}
	return NULL;
}
