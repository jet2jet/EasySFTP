/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 FTPConn.cpp - implementations of FTP connection (command exchange, synchronization, and so on)
 */

#include "StdAfx.h"
#include "ShellDLL.h"
#include "FTPConn.h"

///////////////////////////////////////////////////////////////////////////////

CFTPWaitEstablishPassive::~CFTPWaitEstablishPassive()
{
	if (pConnection)
		delete pConnection;
	pMessage->Release();
}

///////////////////////////////////////////////////////////////////////////////

CFTPConnection::CFTPConnection(void)
{
	m_lpszAvailableCommands = NULL;
	m_FTPSConnectionPhase = FTPSConnectionPhase::None;
	m_bIsLoggingIn = true;
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
		CMyStringW str(IDS_UNKNOWN_HOST, lpszHostName);
		theApp.Log(EasySFTPLogLevel::Error, str, E_FAIL);
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

	::EnterCriticalSection(&m_csSocket);
	m_socket.SendString(strMsg + L"\r\n");
	m_aWaitResponse.Enqueue(p);

	{
		CMyStringW str;
		str.Format(L"(wait = %p) > ", pWait);
		strMsg.InsertString(str, 0);
		theApp.Log(EasySFTPLogLevel::Debug, strMsg, S_OK);
	}
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

	{
		CMyStringW str2;
		str2.Format(L"(wait = %p) > ", pWait);
		str2 += lpszCommand;
		str2 += L" ****";
		theApp.Log(EasySFTPLogLevel::Debug, str2, S_OK);
	}
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

	m_socket.SendString(strMsg + L"\r\n");
	m_aWaitResponse.Enqueue(p);

	{
		CMyStringW str;
		str.Format(L"(wait = %p) > ", pWait1);
		strMsg.InsertString(str, 0);
		theApp.Log(EasySFTPLogLevel::Debug, strMsg, S_OK);
	}

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

	m_socket.SendString(strMsg + L"\r\n");
	m_aWaitResponse.Enqueue(p);

	{
		CMyStringW str;
		str.Format(L"(wait = %p) > ", pWait2);
		strMsg.InsertString(str, 0);
		theApp.Log(EasySFTPLogLevel::Debug, strMsg, S_OK);
	}

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

	m_socket.SendString(strMsg + L"\r\n");
	m_aWaitResponse.Enqueue(p);

	{
		CMyStringW str;
		str.Format(L"(wait = %p) > ", pWait1);
		strMsg.InsertString(str, 0);
		theApp.Log(EasySFTPLogLevel::Debug, strMsg, S_OK);
	}

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

	m_socket.SendString(strMsg + L"\r\n");
	m_aWaitResponse.Enqueue(p);

	{
		CMyStringW str;
		str.Format(L"(wait = %p) > ", pWait2);
		strMsg.InsertString(str, 0);
		theApp.Log(EasySFTPLogLevel::Debug, strMsg, S_OK);
	}

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

	m_socket.SendString(strMsg + L"\r\n");
	m_aWaitResponse.Enqueue(p);

	{
		CMyStringW str;
		str.Format(L"(wait = %p) > ", pWait2);
		strMsg.InsertString(str, 0);
		theApp.Log(EasySFTPLogLevel::Debug, strMsg, S_OK);
	}

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
		while (!m_socket.ReceiveLine(str, []() { return theApp.MyPumpMessage2(); }))
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

	CFTPWaitResponse* p = NULL;
	bool b = false;
	for (auto i = 0; i < m_aWaitResponse.GetCount(); ++i)
	{
		p = m_aWaitResponse.GetItem(i);
		// find Passive waiting if code is 226 or 451
		if (r == 226 || r == 451)
		{
			if (p->pWait && p->pWait->nWaitType == CWaitResponseData::WRD_ENDPASSIVE)
			{
				b = true;
				m_aWaitResponse.RemoveItem(i);
				break;
			}
		}
		else
		{
			if (!p->pWait || p->pWait->nWaitType != CWaitResponseData::WRD_ENDPASSIVE)
			{
				b = true;
				m_aWaitResponse.RemoveItem(i);
				break;
			}
		}
	}

	{
		CMyStringW str;
		str.Format(L"FTP: %d %s (wait = %p)", nCode, (LPCWSTR) rstrMessage, b ? p->pWait : (void*) NULL);
		theApp.Log(EasySFTPLogLevel::Debug, str, S_OK);
	}
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
		pPassive->nWaitFlags = CFTPWaitPassive::WaitFlags::Finished;
		pPassive->pMessage->EndReceive(NULL);

		pPassive->pPassive->Close();
		//delete pPassive->pPassive;
		//pPassive->pPassive = NULL;
		return true;
	}
	if (!pPassive->pMessage->OnReceive(pPassive->pPassive) &&
		!pPassive->pPassive->IsRemoteClosed())
	{
		pPassive->bWaiting = false;
		pPassive->nWaitFlags = CFTPWaitPassive::WaitFlags::Error;
		pPassive->pMessage->EndReceive(NULL);
		//break;
		//::LeaveCriticalSection(&m_csSocket);
		return false;
	}
	//::LeaveCriticalSection(&m_csSocket);
	return true;
}

void CFTPConnection::WaitFinishPassive(CFTPWaitPassive* pPassive)
{
	::EnterCriticalSection(&m_csSocket);

	CFTPWaitResponse* p = new CFTPWaitResponse();
	p->pWait = pPassive;
	m_aWaitResponse.Enqueue(p);
	{
		CMyStringW str;
		str.Format(L"(wait = %p) : wait for 226 msg", pPassive);
		theApp.Log(EasySFTPLogLevel::Debug, str, S_OK);
	}
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

void CFTPConnection::CopyAvailableCommands(const CFTPConnection* pConnectionFrom)
{
	if (m_lpszAvailableCommands)
		free(m_lpszAvailableCommands);
	if (!pConnectionFrom->m_lpszAvailableCommands)
		m_lpszAvailableCommands = NULL;
	else
	{
		auto* pStart = pConnectionFrom->m_lpszAvailableCommands;
		auto* pEnd = pStart;
		while (*pEnd)
		{
			while (*pEnd++);
		}
		size_t nLen = static_cast<size_t>(pEnd - pStart) + 1;
		auto* pNew = static_cast<LPWSTR>(malloc(sizeof(WCHAR) * nLen));
		if (pNew)
			memcpy(pNew, pStart, sizeof(WCHAR) * nLen);
		m_lpszAvailableCommands = pNew;
	}
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

void CFTPConnection::StartFTPSHandshake()
{
	SendCommand(L"AUTH", L"TLS");
	m_FTPSConnectionPhase = FTPSConnectionPhase::FirstReceive;
}

CFTPConnection::FTPSHandshakeResult CFTPConnection::OnFirstFTPSHandshake(int code)
{
	if (m_FTPSConnectionPhase != FTPSConnectionPhase::FirstReceive)
		return FTPSHandshakeResult::NotApplicable;
	switch (code)
	{
		case 234:
			{
				auto r = m_socket.StartHandshake();
				if (r == CFTPSocket::HandshakeResult::Success)
				{
					m_FTPSConnectionPhase = FTPSConnectionPhase::None;
					SendDoubleCommand(L"PBSZ", L"0", L"PROT", L"P");
					return FTPSHandshakeResult::Success;
				}
				else if (r == CFTPSocket::HandshakeResult::Error)
				{
					LogLastSSLError();
					return FTPSHandshakeResult::Failure;
				}
				else
				{
					m_FTPSConnectionPhase = FTPSConnectionPhase::Handshake;
					return FTPSHandshakeResult::InProgress;
				}
			}
		case 530:
		default:
			return FTPSHandshakeResult::Failure;
	}
}

CFTPConnection::FTPSHandshakeResult CFTPConnection::ProcessFTPSHandshake()
{
	if (m_FTPSConnectionPhase != FTPSConnectionPhase::Handshake)
		return FTPSHandshakeResult::NotApplicable;
	auto r = m_socket.ContinueHandshake();
	if (r == CFTPSocket::HandshakeResult::Success)
	{
		m_FTPSConnectionPhase = FTPSConnectionPhase::None;
		return FTPSHandshakeResult::Success;
	}
	else if (r == CFTPSocket::HandshakeResult::Error)
	{
		return FTPSHandshakeResult::Failure;
	}
	return FTPSHandshakeResult::InProgress;
}
