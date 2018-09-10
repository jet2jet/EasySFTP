/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 SSHCli.cpp - implementations of CSSH2Client
 */

#include "StdAfx.h"
#include "SSHCli.h"

#include "Func.h"
#include "Cipher.h"

#define SSH2_CLIENT_VERSION   L"SSH-2.0-EasySFTP-1.0"
#define SSH2_CLIENT_VERSION_CR_LF  SSH2_CLIENT_VERSION L"\r\n"

////////////////////////////////////////////////////////////////////////////////

static const char* myproposal[PROPOSAL_MAX] = {
	KEX_DEFAULT_KEX,
	KEX_DEFAULT_PK_ALG,
	KEX_DEFAULT_ENCRYPT,
	KEX_DEFAULT_ENCRYPT,
	KEX_DEFAULT_MAC,
	KEX_DEFAULT_MAC,
	KEX_DEFAULT_COMP,
	KEX_DEFAULT_COMP,
	KEX_DEFAULT_LANG,
	KEX_DEFAULT_LANG
};

static void __stdcall ProposalsToBuffer(CExBuffer* pBuffer, const char* const proposal[PROPOSAL_MAX])
{
	pBuffer->Empty();
	memset(pBuffer->AppendToBuffer(NULL, KEX_COOKIE_LEN), 0, KEX_COOKIE_LEN);
	for (int i = 0; i < PROPOSAL_MAX; i++)
		pBuffer->AppendToBufferWithLenCE(proposal[i]);
	pBuffer->AppendToBuffer((BYTE) 0);  // first_kex_packet_follows
	pBuffer->AppendToBuffer((ULONG) 0); // reserved
}

static BYTE __stdcall BufferToProposals(char* proposal[PROPOSAL_MAX], const void* pvBuffer, size_t nLen)
{
	const BYTE* pb;
	size_t nTextSize;
	if (nLen < KEX_COOKIE_LEN)
		return 0;
	pb = (const BYTE*) pvBuffer;
	pb += KEX_COOKIE_LEN;
	nLen -= KEX_COOKIE_LEN;
	for (int i = 0; i < PROPOSAL_MAX; i++)
	{
		if (nLen < 4)
			return 0;
		nTextSize = ((ULONG) *pb++) << 24;
		nTextSize |= ((ULONG) *pb++) << 16;
		nTextSize |= ((ULONG) *pb++) << 8;
		nTextSize |= *pb++;
		nLen -= 4;
		if (nLen < nTextSize)
			return 0;
		pb += nTextSize;
		nLen -= nTextSize;
	}
	if (nLen < 5)
		return 0;
	BYTE bFirstKexFollows = *pb++;
	// ignoring reserved 4-bytes

	if (!proposal)
		return bFirstKexFollows;

	pb = (const BYTE*) pvBuffer;
	pb += KEX_COOKIE_LEN;
	for (int i = 0; i < PROPOSAL_MAX; i++)
	{
		nTextSize = ((ULONG) *pb++) << 24;
		nTextSize |= ((ULONG) *pb++) << 16;
		nTextSize |= ((ULONG) *pb++) << 8;
		nTextSize |= *pb++;
		proposal[i] = (char*) malloc(nTextSize + 1);
		if (nTextSize)
			memcpy(proposal[i], pb, nTextSize);
		proposal[i][nTextSize] = 0;
		pb += nTextSize;
	}
	return bFirstKexFollows;
}

extern "C" const char* __stdcall FindMatches(const char* pszExpect, char* pszActual, char chDelimiter)
{
	char* pszA;
	const char* pszFound, * pc;
	size_t nLen;
	pszFound = NULL;
	while (*pszExpect)
	{
		pc = strchr(pszExpect, chDelimiter);
		if (!pc)
			nLen = strlen(pszExpect);
		else
			nLen = (size_t) ((DWORD_PTR) pc - (DWORD_PTR) pszExpect) / sizeof(char);
		pszA = pszActual;
		while (*pszA)
		{
			if ((pszA[nLen] == chDelimiter || !pszA[nLen]) &&
				memcmp(pszExpect, pszA, sizeof(char) * nLen) == 0)
			{
				pszFound = pszA;
				pszA[nLen] = 0;
				break;
			}
			while (*pszA && *pszA != chDelimiter)
				pszA++;
			if (*pszA)
				pszA++;
		}
		if (pszFound)
			break;
		if (!pc)
			break;
		pszExpect = pc + 1;
	}
	return pszFound;
}

////////////////////////////////////////////////////////////////////////////////

static bool __stdcall GetEncryptionAlgorithm(CEncryption* pEnc, LPCSTR lpszFoundAlgs)
{
	if (!lpszFoundAlgs)
		return false;
	CCipher* p = GetCipherByName(lpszFoundAlgs);
	if (!p)
		return false;
	pEnc->lpszName = p->lpszName;
	pEnc->bEnabled = false;
	pEnc->pCipher = p;
	pEnc->nBlockSize = p->nBlockSize;
	pEnc->nKeyLen = p->nKeyLen;
	pEnc->pKeyData = NULL;
	pEnc->pIV = NULL;
	return true;
}

////////////////////////////////////////////////////////////////////////////////

static bool __stdcall GetMacAlgorithm(CMacData* pMac, LPCSTR lpszFoundAlgs, DWORD dwCompatFlags)
{
	if (!lpszFoundAlgs)
		return false;
	if (!MacSetup(pMac, lpszFoundAlgs))
		return false;
	if (dwCompatFlags & SSH_BUG_HMAC)
		pMac->nKeyLen = 16;
	pMac->bEnabled = false;
	return true;
}

////////////////////////////////////////////////////////////////////////////////

static bool __stdcall GetCompressAlgorithm(CCompressionData* pComp, LPCSTR lpszFoundAlgs)
{
	if (!lpszFoundAlgs)
		return false;
	if (strcmp(lpszFoundAlgs, "zlib@openssh.com") == 0)
	{
		pComp->lpszName = "zlib@openssh.com";
		pComp->nType = COMP_DELAYED;
	}
	else if (strcmp(lpszFoundAlgs, "zlib") == 0)
	{
		pComp->lpszName = "zlib";
		pComp->nType = COMP_ZLIB;
	}
	else if (strcmp(lpszFoundAlgs, "none") == 0)
	{
		pComp->lpszName = "none";
		pComp->nType = COMP_NONE;
	}
	else
		return false;
	pComp->bEnabled = false;
	return true;
}

////////////////////////////////////////////////////////////////////////////////

CSSH2Client::CSSH2Client(void)
	: m_pKex(NULL)
	, m_pPKey(NULL)
{
	m_pAuth = NULL;
	memset(&m_keyDataCtoS, 0, sizeof(m_keyDataCtoS));
	memset(&m_keyDataStoC, 0, sizeof(m_keyDataStoC));
}

CSSH2Client::~CSSH2Client(void)
{
	m_socket.Close("disconnected by user");

	if (m_pKex)
		delete m_pKex;
	MacCleanup(&m_keyDataCtoS.mac);
	MacCleanup(&m_keyDataStoC.mac);
}

bool CSSH2Client::OnFirstReceive()
{
	CMyStringW str;

	m_socket.ReceiveLine(m_strServerName);

	{
		LPCSTR lp = m_strServerName;
		int maj = 0, min = 0;
		if (strncmp(lp, "SSH-", 4))
			return false;
		lp += 4;
		while (*lp >= '0' && *lp <= '9')
		{
			maj *= 10;
			maj += (int) (*lp++ - '0');
		}
		if (*lp++ != '.')
			return false;
		while (*lp >= '0' && *lp <= '9')
		{
			min *= 10;
			min += (int) (*lp++ - '0');
		}
		if (*lp++ != '-')
			return false;
		if (maj < 2 && (maj != 1 && min != 99))
			return false;
		m_socket.InitServerVersion(lp);
	}

	str = SSH2_CLIENT_VERSION_CR_LF;
	m_socket.SendString(str);

	if (!StartKeyExchange())
		return false;

	return true;
}

bool CSSH2Client::StartKeyExchange()
{
	ProposalsToBuffer(&m_bufferMyProposal, myproposal);
	// make random for buffer[0 .. KEX_COOKIE_LEN - 1]
	BYTE* pb = (BYTE*) m_bufferMyProposal.GetCurrentBufferPermanent(KEX_COOKIE_LEN);
	if (!pb)
		return false;
	for (int i = 0; i < KEX_COOKIE_LEN; i++)
		pb[i] = (BYTE) (rand() * 256 / RAND_MAX);
	return m_socket.SendPacket(SSH2_MSG_KEXINIT, m_bufferMyProposal, m_bufferMyProposal.GetLength());
}

int CSSH2Client::OnKeyExchangeInit(const void* pv, size_t nLen, bool bIgnoreMsg)
{
	CKeyExchange* pKex;
	CExBuffer svprop;

	if (m_pKex)
	{
		m_socket.ResetServerKeyData();
		delete m_pKex;
	}

	if (!bIgnoreMsg)
	{
		if (!m_bufferSvProposal.SetDataToBuffer(pv, nLen))
			return -1;
		BYTE bFirstKexFollows = BufferToProposals(NULL, pv, nLen);
		if (bFirstKexFollows && !(m_socket.GetServerCompatible() & SSH_BUG_FIRSTKEX))
			return 0;
	}
	char* proposal[PROPOSAL_MAX];
	int i;
	for (i = 0; i < PROPOSAL_MAX; i++)
		proposal[i] = NULL;
	BufferToProposals(proposal, m_bufferSvProposal, m_bufferSvProposal.GetLength());

	const char* pszFoundKexAlgs, * pszFoundHostKeyAlgs;
	const char* pszFoundEncAlgs[2], * pszFoundMacAlgs[2], * pszFoundCompAlgs[2];
	pszFoundKexAlgs = FindMatches(
		myproposal[PROPOSAL_KEX_ALGS],
		proposal[PROPOSAL_KEX_ALGS],
		',');
	pszFoundHostKeyAlgs = FindMatches(
		myproposal[PROPOSAL_SERVER_HOST_KEY_ALGS],
		proposal[PROPOSAL_SERVER_HOST_KEY_ALGS],
		',');
	pszFoundEncAlgs[0] = FindMatches(
		myproposal[PROPOSAL_ENC_ALGS_CTOS],
		proposal[PROPOSAL_ENC_ALGS_CTOS],
		',');
	pszFoundEncAlgs[1] = FindMatches(
		myproposal[PROPOSAL_ENC_ALGS_STOC],
		proposal[PROPOSAL_ENC_ALGS_STOC],
		',');
	pszFoundMacAlgs[0] = FindMatches(
		myproposal[PROPOSAL_MAC_ALGS_CTOS],
		proposal[PROPOSAL_MAC_ALGS_CTOS],
		',');
	pszFoundMacAlgs[1] = FindMatches(
		myproposal[PROPOSAL_MAC_ALGS_STOC],
		proposal[PROPOSAL_MAC_ALGS_STOC],
		',');
	pszFoundCompAlgs[0] = FindMatches(
		myproposal[PROPOSAL_COMP_ALGS_CTOS],
		proposal[PROPOSAL_COMP_ALGS_CTOS],
		',');
	pszFoundCompAlgs[1] = FindMatches(
		myproposal[PROPOSAL_COMP_ALGS_STOC],
		proposal[PROPOSAL_COMP_ALGS_STOC],
		',');

	DWORD dwCP = m_socket.GetServerCompatible();
	pKex = NULL;
	if (GetEncryptionAlgorithm(&m_keyDataCtoS.enc, pszFoundEncAlgs[0]) &&
		GetEncryptionAlgorithm(&m_keyDataStoC.enc, pszFoundEncAlgs[1]) &&
		GetMacAlgorithm(&m_keyDataCtoS.mac, pszFoundMacAlgs[0], dwCP) &&
		GetMacAlgorithm(&m_keyDataStoC.mac, pszFoundMacAlgs[1], dwCP) &&
		GetCompressAlgorithm(&m_keyDataCtoS.comp, pszFoundCompAlgs[0]) &&
		GetCompressAlgorithm(&m_keyDataStoC.comp, pszFoundCompAlgs[1]))
	{
		size_t nNeedKeyBytes = m_keyDataCtoS.enc.nKeyLen;
		if (nNeedKeyBytes < m_keyDataCtoS.enc.nBlockSize)
			nNeedKeyBytes = m_keyDataCtoS.enc.nBlockSize;
		if (nNeedKeyBytes < m_keyDataCtoS.mac.nKeyLen)
			nNeedKeyBytes = m_keyDataCtoS.mac.nKeyLen;
		if (nNeedKeyBytes < m_keyDataStoC.enc.nKeyLen)
			nNeedKeyBytes = m_keyDataStoC.enc.nKeyLen;
		if (nNeedKeyBytes < m_keyDataStoC.enc.nBlockSize)
			nNeedKeyBytes = m_keyDataStoC.enc.nBlockSize;
		if (nNeedKeyBytes < m_keyDataStoC.mac.nKeyLen)
			nNeedKeyBytes = m_keyDataStoC.mac.nKeyLen;

		//m_socket.SendPacket(SSH2_MSG_IGNORE, NULL, 0);
		//{
		//	CMyStringW str(L"test test message");
		//	CExBuffer bufDebug;
		//	bufDebug.AppendToBuffer((BYTE) 0);
		//	bufDebug.AppendToBufferWithLenCE(str);
		//	bufDebug.AppendToBufferWithLenCE(str);
		//	m_socket.SendPacket(SSH2_MSG_DEBUG, bufDebug, bufDebug.GetLength());
		//}
		//m_socket.SendPacket(SSH2_MSG_IGNORE, NULL, 0);

		pKex = CKeyExchange::InitKeyExchange(
			SSH2_CLIENT_VERSION, m_strServerName,
			pszFoundKexAlgs, pszFoundHostKeyAlgs, nNeedKeyBytes,
			m_bufferMyProposal, m_bufferSvProposal, &m_socket);
	}

	for (i = 0; i < PROPOSAL_MAX; i++)
	{
		if (proposal[i])
			free(proposal[i]);
	}

	m_pKex = pKex;
	return pKex != NULL ? 1 : -1;
}

int CSSH2Client::OnReceiveKexMessages(CFingerPrintHandler* pHandler, BYTE bType, const void* pv, size_t nLen)
{
	if (!m_pKex)
		return -1;

	int r = m_pKex->OnReceiveKexMessages(pHandler, &m_socket, bType, pv, nLen);
	if (r > 0)
	{
		m_keyDataCtoS.enc.pIV = m_pKex->m_pvDeriveKeys[0];
		m_keyDataStoC.enc.pIV = m_pKex->m_pvDeriveKeys[1];
		m_keyDataCtoS.enc.pKeyData = m_pKex->m_pvDeriveKeys[2];
		m_keyDataStoC.enc.pKeyData = m_pKex->m_pvDeriveKeys[3];
		m_keyDataCtoS.mac.pKeyData = m_pKex->m_pvDeriveKeys[4];
		m_keyDataStoC.mac.pKeyData = m_pKex->m_pvDeriveKeys[5];

		m_socket.SendPacket(SSH2_MSG_NEWKEYS, NULL, 0);
		m_socket.UpdateServerKeyData(true, m_keyDataCtoS);
	}
	return r;
}

////////////////////////////////////////////////////////////////////////////////

bool CSSH2Client::Authenticate(LPCSTR lpszAuthService, char nAuthType, const void* pvServiceData, size_t nDataLen, CUserInfo* pUserInfo)
{
	CExBuffer buf;
	ULONG uSize;
	char* psz;
	if (!buf.SetDataToBuffer(pvServiceData, nDataLen) ||
		!buf.GetAndSkipCE(uSize) ||
		!(psz = (char*) buf.GetCurrentBufferPermanentAndSkip((size_t) uSize)) ||
		!buf.IsEmpty())
		return false;
	if (strcmplen1(psz, sizeof(char) * uSize, lpszAuthService) != 0)
		return false;

	return DoAuthenticate(nAuthType, pUserInfo);
}

bool CSSH2Client::DoAuthenticate(char nAuthType, CUserInfo* pUserInfo)
{
	if (!m_pAuth)
	{
		switch (nAuthType)
		{
			case AUTHTYPE_PASSWORD:
				m_pAuth = new CPasswordAuthentication();
				break;
			case AUTHTYPE_PUBLICKEY:
				m_pAuth = new CPublicKeyAuthentication();
				break;
			case AUTHTYPE_PAGEANT:
				m_pAuth = new CPageantAuthentication();
				break;
			default:
			case AUTHTYPE_NONE:
				m_pAuth = new CNoneAuthentication();
				break;
		}
	}
	bool bRet = m_pAuth->Authenticate(&m_socket, pUserInfo, "ssh-connection");
	//pAuth->FinishAndDelete();

	return bRet;
}

bool CSSH2Client::CanRetryAuthenticate()
{
	if (!m_pAuth)
		return false;
	return m_pAuth->CanRetry();
}

void CSSH2Client::EndAuthenticate()
{
	m_pAuth->FinishAndDelete();
	m_pAuth = NULL;
}

bool CSSH2Client::NoMoreSessions()
{
	if (!(m_socket.GetServerCompatible() & SSH_NEW_OPENSSH))
		return true;

	CExBuffer buf;
	buf.AppendToBufferWithLenCE("no-more-sessions@openssh.com");
	buf.AppendToBuffer((BYTE) 0);
	return m_socket.SendPacket(SSH2_MSG_GLOBAL_REQUEST, buf, buf.GetLength());
}
