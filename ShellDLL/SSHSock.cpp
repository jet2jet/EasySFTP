/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 SSHSock.cpp - implementations of CSSH2Socket
 */

#include "StdAfx.h"
#include "MyFunc.h"
#include "SSHSock.h"

#include "ExBuffer.h"
#include "KexCore.h"

static const struct
{
	const char* pszServerVersion;
	DWORD dwFlags;
} s_serverCompatibilities[] = {
	{ "OpenSSH-2.0*\0"
	  "OpenSSH-2.1*\0"
	  "OpenSSH_2.1*\0"
	  "OpenSSH_2.2*\0",	SSH_OLD_SESSIONID|SSH_BUG_BANNER|
				SSH_OLD_DHGEX|SSH_BUG_NOREKEY|
				SSH_BUG_EXTEOF|SSH_OLD_FORWARD_ADDR|SSH_BUG_SYMLINK},
	{ "OpenSSH_2.3.0*\0",	SSH_BUG_BANNER|SSH_BUG_BIGENDIANAES|
				SSH_OLD_DHGEX|SSH_BUG_NOREKEY|
				SSH_BUG_EXTEOF|SSH_OLD_FORWARD_ADDR|SSH_BUG_SYMLINK},
	{ "OpenSSH_2.3.*\0",	SSH_BUG_BIGENDIANAES|SSH_OLD_DHGEX|
				SSH_BUG_NOREKEY|SSH_BUG_EXTEOF|
				SSH_OLD_FORWARD_ADDR|SSH_BUG_SYMLINK},
	{ "OpenSSH_2.5.0p1*\0"
	  "OpenSSH_2.5.1p1*\0",
				SSH_BUG_BIGENDIANAES|SSH_OLD_DHGEX|
				SSH_BUG_NOREKEY|SSH_BUG_EXTEOF|
				SSH_OLD_FORWARD_ADDR|SSH_BUG_SYMLINK},
	{ "OpenSSH_2.5.0*\0"
	  "OpenSSH_2.5.1*\0"
	  "OpenSSH_2.5.2*\0",	SSH_OLD_DHGEX|SSH_BUG_NOREKEY|
				SSH_BUG_EXTEOF|SSH_OLD_FORWARD_ADDR|SSH_BUG_SYMLINK},
	{ "OpenSSH_2.5.3*\0",	SSH_BUG_NOREKEY|SSH_BUG_EXTEOF|
				SSH_OLD_FORWARD_ADDR|SSH_BUG_SYMLINK},
	{ "OpenSSH_2.*\0"
	  "OpenSSH_3.0*\0"
	  "OpenSSH_3.1*\0",	SSH_BUG_EXTEOF|SSH_OLD_FORWARD_ADDR|SSH_BUG_SYMLINK},
	{ "OpenSSH_3.*\0",	SSH_OLD_FORWARD_ADDR|SSH_BUG_SYMLINK },
	{ "Sun_SSH_1.0*\0",	SSH_BUG_NOREKEY|SSH_BUG_EXTEOF|SSH_BUG_SYMLINK},
	{ "OpenSSH_4*\0",		SSH_BUG_SYMLINK },
	{ "OpenSSH*\0",		SSH_NEW_OPENSSH|SSH_BUG_SYMLINK },
	{ "*MindTerm*\0",		0 },
	{ "2.1.0*\0",		SSH_BUG_SIGBLOB|SSH_BUG_HMAC|
				SSH_OLD_SESSIONID|SSH_BUG_DEBUG|
				SSH_BUG_RSASIGMD5|SSH_BUG_HBSERVICE|
				SSH_BUG_FIRSTKEX },
	{ "2.1 *\0",		SSH_BUG_SIGBLOB|SSH_BUG_HMAC|
				SSH_OLD_SESSIONID|SSH_BUG_DEBUG|
				SSH_BUG_RSASIGMD5|SSH_BUG_HBSERVICE|
				SSH_BUG_FIRSTKEX },
	{ "2.0.13*\0"
	  "2.0.14*\0"
	  "2.0.15*\0"
	  "2.0.16*\0"
	  "2.0.17*\0"
	  "2.0.18*\0"
	  "2.0.19*\0",		SSH_BUG_SIGBLOB|SSH_BUG_HMAC|
				SSH_OLD_SESSIONID|SSH_BUG_DEBUG|
				SSH_BUG_PKSERVICE|SSH_BUG_X11FWD|
				SSH_BUG_PKOK|SSH_BUG_RSASIGMD5|
				SSH_BUG_HBSERVICE|SSH_BUG_OPENFAILURE|
				SSH_BUG_DUMMYCHAN|SSH_BUG_FIRSTKEX },
	{ "2.0.11*\0"
	  "2.0.12*\0",		SSH_BUG_SIGBLOB|SSH_BUG_HMAC|
				SSH_OLD_SESSIONID|SSH_BUG_DEBUG|
				SSH_BUG_PKSERVICE|SSH_BUG_X11FWD|
				SSH_BUG_PKAUTH|SSH_BUG_PKOK|
				SSH_BUG_RSASIGMD5|SSH_BUG_OPENFAILURE|
				SSH_BUG_DUMMYCHAN|SSH_BUG_FIRSTKEX },
	{ "2.0.*\0",		SSH_BUG_SIGBLOB|SSH_BUG_HMAC|
				SSH_OLD_SESSIONID|SSH_BUG_DEBUG|
				SSH_BUG_PKSERVICE|SSH_BUG_X11FWD|
				SSH_BUG_PKAUTH|SSH_BUG_PKOK|
				SSH_BUG_RSASIGMD5|SSH_BUG_OPENFAILURE|
				SSH_BUG_DERIVEKEY|SSH_BUG_DUMMYCHAN|
				SSH_BUG_FIRSTKEX },
	{ "2.2.0*\0"
	  "2.3.0*\0",		SSH_BUG_HMAC|SSH_BUG_DEBUG|
				SSH_BUG_RSASIGMD5|SSH_BUG_FIRSTKEX },
	{ "2.3.*\0",		SSH_BUG_DEBUG|SSH_BUG_RSASIGMD5|
				SSH_BUG_FIRSTKEX },
	{ "2.4\0",		SSH_OLD_SESSIONID },	/* Van Dyke */
	{ "2.*\0",		SSH_BUG_DEBUG|SSH_BUG_FIRSTKEX|
				SSH_BUG_RFWD_ADDR },
	{ "3.0.*\0",		SSH_BUG_DEBUG },
	{ "3.0 SecureCRT*\0",	SSH_OLD_SESSIONID },
	{ "1.7 SecureFX*\0",	SSH_OLD_SESSIONID },
	{ "1.2.18*\0"
	  "1.2.19*\0"
	  "1.2.20*\0"
	  "1.2.21*\0"
	  "1.2.22*\0",		SSH_BUG_IGNOREMSG },
	{ "1.3.2*\0",		/* F-Secure */
				SSH_BUG_IGNOREMSG },
	{ "*SSH Compatible Server*\0",			/* Netscreen */
				SSH_BUG_PASSWORDPAD },
	{ "*OSU_0*\0"
	  "OSU_1.0*\0"
	  "OSU_1.1*\0"
	  "OSU_1.2*\0"
	  "OSU_1.3*\0"
	  "OSU_1.4*\0"
	  "OSU_1.5alpha1*\0"
	  "OSU_1.5alpha2*\0"
	  "OSU_1.5alpha3*\0",	SSH_BUG_PASSWORDPAD },
	{ "*SSH_Version_Mapper*\0",
				SSH_BUG_SCANNER },
	{ "Probe-*\0",
				SSH_BUG_PROBE }//,
	//{ NULL,			0 }
};

static bool __stdcall _MatchWildcardEx(LPCSTR lpszTarget, LPCSTR lpszPatternMulti)
{
	while (*lpszPatternMulti)
	{
		if (MyMatchWildcardA(lpszTarget, lpszPatternMulti))
			return true;
		while (*lpszPatternMulti++);
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////

struct CMyKeyData
{
	CCipherContext context;
	CNewKeyData* pKey;
};

////////////////////////////////////////////////////////////////////////////////

CSSH2Socket::CSSH2Socket()
	: m_dwServerFlags(0)
	, m_privKey(NULL)
	, m_bReleaseMyself(false)
	, m_pKeyDataSend(NULL)
	, m_pKeyDataRecv(NULL)
	, m_nLastPacketSize(0)
	, m_nSendSequenceNumber(0)
	, m_nRecvSequenceNumber(0)
{
	::InitializeCriticalSection(&m_csRecvBuffer);
}

CSSH2Socket::~CSSH2Socket()
{
	Close();

	ResetServerKeyData();
	SetPrivateKey(NULL, false);
	::DeleteCriticalSection(&m_csRecvBuffer);
}

void CSSH2Socket::InitServerVersion(LPCSTR lpszServerVersion)
{
	for (size_t n = 0; n < sizeof(s_serverCompatibilities) / sizeof(s_serverCompatibilities[0]); n++)
	{
		if (_MatchWildcardEx(lpszServerVersion, s_serverCompatibilities[n].pszServerVersion))
		{
			m_dwServerFlags = s_serverCompatibilities[n].dwFlags;
			return;
		}
	}
}

bool CSSH2Socket::SetPrivateKey(EVP_PKEY* privKey, bool bReleaseMyself)
{
	if (m_bReleaseMyself && m_privKey)
		::EVP_PKEY_free(m_privKey);
	m_privKey = privKey;
	m_bReleaseMyself = privKey ? bReleaseMyself : false;
	return privKey != NULL;
}

void CSSH2Socket::UpdateServerKeyData(bool bForSend, CNewKeyData& keyData)
{
	CMyKeyData** ppData;

	ppData = (CMyKeyData**) (bForSend ? &m_pKeyDataSend : &m_pKeyDataRecv);
	if (*ppData)
	{
		CleanupCipherContext(&(*ppData)->context);
		//free(*ppContext);
	}
	else
		*ppData = (CMyKeyData*) malloc(sizeof(CMyKeyData));
	if (MacInit(&keyData.mac))
		keyData.mac.bEnabled = true;
	InitCipherContext(&(*ppData)->context, keyData.enc.pCipher, keyData.enc.pKeyData,
		keyData.enc.nKeyLen, keyData.enc.pIV, keyData.enc.nBlockSize, bForSend);
	//if (keyData.comp.nType == COMP_ZLIB || keyData.comp.nType == COMP_DELAYED)
	//{
	//}
	//if (keyData.enc.nBlockSize >= 16)
	(*ppData)->pKey = &keyData;
}

void CSSH2Socket::ResetServerKeyData()
{
	if (m_pKeyDataSend)
	{
		CleanupCipherContext(&((CMyKeyData*) m_pKeyDataSend)->context);
		free(m_pKeyDataSend);
		m_pKeyDataSend = NULL;
	}
	if (m_pKeyDataRecv)
	{
		CleanupCipherContext(&((CMyKeyData*) m_pKeyDataRecv)->context);
		free(m_pKeyDataRecv);
		m_pKeyDataRecv = NULL;
	}
}

static bool __stdcall _DoSendSocket(CMySocket* pSocket, LPCVOID pvData, size_t nSize)
{
	size_t nRet;
	while (nSize > 0)
	{
		nRet = pSocket->Send(pvData, nSize, 0);
		if (nRet == (size_t) -1)
			return false;
		nSize -= nRet;
		pvData = ((BYTE*) pvData) + nRet;
	}
	return true;
}

static bool __stdcall _DoSendSocketBlocking(CMySocket* pSocket, LPCVOID pvData, size_t nSize)
{
	if (pSocket->IsRemoteClosed())
		return false;
	pSocket->EnableAsyncSelect(false, true);
	if (!pSocket->IoControl(FIONBIO, (DWORD) 0))
		return false;
	if (!_DoSendSocket(pSocket, pvData, nSize))
	{
		pSocket->EnableAsyncSelect(true, true);
		return false;
	}
	pSocket->EnableAsyncSelect(true, true);
	return true;
}

// for SSH2
#define BLOCK_SIZE       8
#define MAC_LEN          0

bool CSSH2Socket::SendPacket(BYTE bType, const void* pBuffer, size_t nLen, size_t nExtraPad)
{
	BYTE* pvBuffer, * pvSend;
	void* pvMacBuf;
	size_t nBufferLen, nPacketLen, nBlockSize, nMacLen;
	size_t nPaddingLen;

	nBlockSize = (m_pKeyDataSend ? ((CMyKeyData*) m_pKeyDataSend)->pKey->enc.nBlockSize : BLOCK_SIZE);
	nBufferLen = nLen + 6;
	nPaddingLen = nBlockSize - (nBufferLen % nBlockSize);
	if (nPaddingLen < 4)
		nPaddingLen += nBlockSize;
	if (nExtraPad)
	{
		nExtraPad = ROUNDUP(nExtraPad, nBlockSize);
		size_t nPad = nExtraPad - ((nBufferLen + nPaddingLen) % nExtraPad);
		nPaddingLen += nPad;
	}
	nPacketLen = nPaddingLen + nBufferLen - 4;
	pvBuffer = (BYTE*) malloc(nPaddingLen + nBufferLen);
	pvBuffer[0] = HIBYTE(HIWORD(nPacketLen));
	pvBuffer[1] = LOBYTE(HIWORD(nPacketLen));
	pvBuffer[2] = HIBYTE(LOWORD(nPacketLen));
	pvBuffer[3] = LOBYTE(LOWORD(nPacketLen));
	pvBuffer[4] = (BYTE) nPaddingLen;
	pvBuffer[5] = bType;
	memcpy(pvBuffer + 6, pBuffer, nLen);

	// compress (pvBuffer + 5 to pvBuffer + 6 + nLen)
	if (m_pKeyDataSend && ((CMyKeyData*) m_pKeyDataSend)->pKey->comp.bEnabled)
	{
	}

	// fill padding field
	nPacketLen = nBufferLen;
	nBufferLen += nPaddingLen;
	while (nPacketLen < nBufferLen)
		pvBuffer[nPacketLen++] = 1;

	// compute MAC
	if (m_pKeyDataSend && ((CMyKeyData*) m_pKeyDataSend)->pKey->mac.bEnabled)
	{
		pvMacBuf = MacCompute(&((CMyKeyData*) m_pKeyDataSend)->pKey->mac, m_nSendSequenceNumber,
			pvBuffer, nBufferLen);
		nMacLen = ((CMyKeyData*) m_pKeyDataSend)->pKey->mac.nMacLen;
	}
	else
		nMacLen = 0;

//#ifdef _DEBUG
//	m_bufferLastSent.SetDataToBuffer(pvBuffer, nBufferLen);
//#endif
	pvSend = (BYTE*) malloc(nBufferLen + nMacLen);
	//pvSend = (BYTE*) m_bufferSend.GetCurrentBuffer(nBufferLen + nMacLen);
	CryptSend(pvSend, pvBuffer, nBufferLen);
//#ifdef _DEBUG
//	m_bufferEncLastSent.SetDataToBuffer(pvSend, nBufferLen);
//#endif
	//memcpy(pvSend + nBufferLen, pMacData, MAC_LEN);
	if (nMacLen)
	{
		memcpy((BYTE*) pvSend + nBufferLen, pvMacBuf, nMacLen);
		free(pvMacBuf);
		nBufferLen += nMacLen;
	}
	free(pvBuffer);

	m_nSendSequenceNumber++;

	// 送信が完了する前にバッファが再利用されないように、
	// データをブロッキングモードで送信する
	// (TeraTerm の ttxssh/ssh.c、send_packet_blocking を参照)
	bool ret = _DoSendSocketBlocking(this, pvSend, nBufferLen);
	free(pvSend);
	return ret;
	//while (nBufferLen > 0)
	//{
	//	nPacketLen = Send(pvSend, nBufferLen, 0);
	//	if (nPacketLen < 0)
	//		return false;
	//	nBufferLen -= nPacketLen;
	//	pvSend = ((BYTE*) pvSend) + nPacketLen;
	//}

	////free(pvSend);
	//return nPacketLen == nBufferLen;
}

bool CSSH2Socket::SendPacketString(BYTE bType, const char* pszString, size_t nExtraPad)
{
	size_t nLen;
	BYTE* pv, * pb;
	nLen = strlen(pszString);
	pv = pb = (BYTE*) malloc(nLen + 4);
	*pb++ = HIBYTE(HIWORD(nLen));
	*pb++ = LOBYTE(HIWORD(nLen));
	*pb++ = HIBYTE(LOWORD(nLen));
	*pb++ = LOBYTE(LOWORD(nLen));
	memcpy(pb, pszString, nLen);
	bool ret = SendPacket(bType, pv, nLen + 4, nExtraPad);
	free(pv);
	return ret;
}

bool CSSH2Socket::ReceiveAllData()
{
	size_t nPos;
	//size_t nOld;
	BYTE* p;

	if (!CanReceive(5000))
		return false;

	nPos = 0;
	//nOld = m_bufferReceived.GetPosition();
	p = (BYTE*) m_bufferReceived.AppendToBuffer(NULL, 1024);
	while (CanReceive())
	{
		/*if (nPos == 1024)
		{
			nPos = 0;
			p = (BYTE*) m_bufferReceived.AppendToBuffer(NULL, 1024);
		}
		if (Recv(p++, 1, 0) != 1)
			break;
		nPos++;*/
		nPos = Recv(p, 1024, 0);
		if (nPos < 1024)
			break;
		nPos = 0;
		p = (BYTE*) m_bufferReceived.AppendToBuffer(NULL, 1024);
	}
	m_bufferReceived.DeleteLastData(1024 - nPos);
	//m_bufferReceived.SetPosition(nOld);
	return true;
}

bool CSSH2Socket::_ReceivePacket(void*& pvRet, BYTE& bType, size_t& nLen)
{
	BYTE* pb, * pb2;
	void* pvMacBuf;
	size_t nNeedLen;
	size_t nBlockSize;
	size_t nPacketSize;
	size_t nMacLen;
	BYTE nPaddingLen;

	nMacLen = (m_pKeyDataRecv && ((CMyKeyData*) m_pKeyDataRecv)->pKey->mac.bEnabled) ?
		((CMyKeyData*) m_pKeyDataRecv)->pKey->mac.nMacLen : 0;
	nBlockSize = (m_pKeyDataRecv) ?
		((CMyKeyData*) m_pKeyDataRecv)->pKey->enc.nBlockSize : BLOCK_SIZE;
	if (m_bufferPacket.IsEmpty())
	{
		pb = (BYTE*) m_bufferReceived.GetCurrentBufferPermanentAndSkip(nBlockSize);
		if (!pb)
		{
			return false;
		}
		pb2 = (BYTE*) m_bufferPacket.AppendToBuffer(NULL, nBlockSize);
		CryptRecv(pb2, pb, nBlockSize);

		ULONG u;
		if (!m_bufferPacket.GetAndSkipCE(u))
		{
			return false;
		}
		m_bufferPacket.SkipPosition(-((LONG) sizeof(u)));
		m_nLastPacketSize = nPacketSize = (size_t) u;
		if (nPacketSize < 1 + 4 || nPacketSize > 0x7FFFFFFF)
		{
			return false;
		}
	}
	else
		nPacketSize = m_nLastPacketSize;

	nNeedLen = 4 + nPacketSize - nBlockSize;
	if (nNeedLen % nBlockSize != 0)
	{
		// invalid padding
		return false;
	}
	if (m_bufferReceived.GetLength() < nNeedLen + nMacLen)
	{
		return false;
	}
	pb = (BYTE*) m_bufferReceived.GetCurrentBufferPermanentAndSkip(nNeedLen);
	pb2 = (BYTE*) m_bufferPacket.AppendToBuffer(NULL, nNeedLen);
	CryptRecv(pb2, pb, nNeedLen);
	if (m_pKeyDataRecv && ((CMyKeyData*) m_pKeyDataRecv)->pKey->mac.bEnabled) // MAC
	{
		pvMacBuf = MacCompute(&((CMyKeyData*) m_pKeyDataRecv)->pKey->mac, m_nRecvSequenceNumber,
			m_bufferPacket, m_bufferPacket.GetLength());
		if (!pvMacBuf)
		{
			Close();
			return false;
		}
		if (!(pb2 = (BYTE*) m_bufferReceived.GetCurrentBufferPermanentAndSkip(nMacLen)) ||
			memcmp(pvMacBuf, pb2, nMacLen) != 0)
		{
			free(pvMacBuf);
			Close();
			return false;
		}
		free(pvMacBuf);
	}

	m_nRecvSequenceNumber++;

	// padding len
	pb = (BYTE*)(void*) m_bufferPacket;
	nPaddingLen = pb[4];
	if (nPaddingLen < 4)
	{
		Close();
		pvRet = NULL;
		return true;
	}
	m_bufferPacket.SkipPosition(5);
	m_bufferPacket.DeleteLastData(nPaddingLen);

	// decompress for current m_bufferPacket
	if (m_pKeyDataRecv && ((CMyKeyData*) m_pKeyDataRecv)->pKey->comp.bEnabled)
	{
	}

	pb = (BYTE*) m_bufferPacket.GetCurrentBufferPermanentAndSkip(1);
	bType = pb[0];
	nLen = m_bufferPacket.GetLength();
	void* pvBuff = m_bufferPacket.GetCurrentBufferPermanentAndSkip(nLen);
	pvRet = malloc(nLen);
	memcpy(pvRet, pvBuff, nLen);
	return true;
}

void* CSSH2Socket::ReceivePacket(BYTE& bType, size_t& nLen)
{
	void* pv;
	::EnterCriticalSection(&m_csRecvBuffer);
	while (true)
	{
		if (_ReceivePacket(pv, bType, nLen))
			break;
		if (!ReceiveAllData())
		{
			pv = NULL;
			break;
		}
	}
	::LeaveCriticalSection(&m_csRecvBuffer);
	return pv;
}

void CSSH2Socket::Close(LPCSTR lpszMessage)
{
	if (operator SOCKET() == INVALID_SOCKET)
		return;

	CExBuffer buf;
	size_t nLen;
	buf.AppendToBufferCE((DWORD) SSH2_DISCONNECT_BY_APPLICATION);
	if (lpszMessage)
		nLen = strlen(lpszMessage);
	else
		nLen = 0;
	buf.AppendToBufferCE((DWORD) nLen);
	if (nLen)
		buf.AppendToBuffer(lpszMessage, nLen);
	SendPacket(SSH2_MSG_DISCONNECT, buf, buf.GetLength());

	CTextSocket::Close();
}

////////////////////////////////////////////////////////////////////////////////

void CSSH2Socket::CryptSend(void* pDest, const void* pSrc, size_t nLen)
{
	if (m_pKeyDataSend)
		CryptWithCipherContext(&((CMyKeyData*) m_pKeyDataSend)->context, pDest, pSrc, nLen);
	else
		memcpy(pDest, pSrc, nLen);
}

void CSSH2Socket::CryptRecv(void* pDest, const void* pSrc, size_t nLen)
{
	if (m_pKeyDataRecv)
		CryptWithCipherContext(&((CMyKeyData*) m_pKeyDataRecv)->context, pDest, pSrc, nLen);
	else
		memcpy(pDest, pSrc, nLen);
}
