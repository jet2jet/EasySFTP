/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 KexCore.cpp - implementations of key exchange
 */

#include "stdafx.h"
#include "KexCore.h"

#include "SSHSock.h"
#include "SUString.h"

extern "C" bool __stdcall AppendBigNumToBuffer(CExBuffer* pBuffer, const BIGNUM* pbnum)
{
	size_t nSize;
	int nOut;
	BYTE* pbuf;

	if (BN_is_zero(pbnum))
	{
		pBuffer->AppendToBuffer((UINT) 0);
		return true;
	}
	if (pbnum->neg)
		return false;
	nSize = 1 + BN_num_bytes(pbnum);    // with padding byte
	if (nSize < 2)
		return false;
	pbuf = (BYTE*) malloc(nSize);
	if (!pbuf)
		return false;
	pbuf[0] = 0;
	nOut = BN_bn2bin(pbnum, (unsigned char*) pbuf + 1);
	if (nOut < 0 || (size_t) nOut != nSize - 1)
	{
		free(pbuf);
		return false;
	}
	// use padding byte?
	nOut = ((pbuf[1] & 0x80) != 0 ? 0 : 1);
	pBuffer->AppendToBufferWithLenCE(pbuf + nOut, nSize - nOut);
	//memset(pbuf, 0, nSize);
	free(pbuf);
	return true;
}

extern "C" bool __stdcall GetBigNumFromBuffer(CExBuffer* pBuffer, BIGNUM* pbnum)
{
	ULONG uSize;
	BYTE* pb;
	if (!pBuffer->GetAndSkipCE(uSize))
		return false;
	if (!(pb = (BYTE*) pBuffer->GetCurrentBufferPermanentAndSkip((size_t) uSize)))
		return false;
	if (uSize > 0 && (pb[0] & 0x80) != 0)
		return false;
	if (uSize > 8192)
		return false;
	if (BN_bin2bn(pb, (int) uSize, pbnum) == NULL)
		return false;
	return true;
}

////////////////////////////////////////////////////////////////////////////////

extern "C" LPCSTR __stdcall KeyTypeToName(KeyType keyType)
{
	switch (keyType)
	{
		case KEY_RSA:
			return "ssh-rsa";
		case KEY_RSA1:
			return "rsa1";
		case KEY_DSA:
			return "ssh-dss";
		default:
			return NULL;
	}
}

extern "C" KeyType __stdcall KeyTypeFromName(LPCSTR pszName)
{
	if (strcmp(pszName, "rsa1") == 0)
		return KEY_RSA1;
	else if (strcmp(pszName, "rsa") == 0)
		return KEY_RSA;
	else if (strcmp(pszName, "dsa") == 0)
		return KEY_DSA;
	else if (strcmp(pszName, "ssh-rsa") == 0)
		return KEY_RSA;
	// not typo
	else if (strcmp(pszName, "ssh-dss") == 0)
		return KEY_DSA;
	else
		return KEY_UNSPEC;
}

static bool __stdcall CreateKeyFromBlob(const void* pvData, size_t nLen, KeyType& keyType, KeyData& keyData)
{
	KeyType type;
	KeyData key;
	char* psz;
	ULONG uSize;
	CExBuffer buf;

	if (!buf.SetDataToBuffer(pvData, nLen))
		return false;
	if (!buf.GetAndSkipCE(uSize))
		return false;
	if (!buf.GetAndSkip(psz, (size_t) uSize))
		return false;
	type = KeyTypeFromName(psz);
	free(psz);
	switch (type)
	{
		case KEY_RSA1:
		case KEY_RSA:
			if (!(key = ::RSA_new()))
				return false;
			if (!(key.rsa()->n = ::BN_new()) || !(key.rsa()->e = ::BN_new()) ||
				!GetBigNumFromBuffer(&buf, key.rsa()->e) ||
				!GetBigNumFromBuffer(&buf, key.rsa()->n))
			{
				::RSA_free(key);
				return false;
			}
			break;
		case KEY_DSA:
			if (!(key = ::DSA_new()))
				return false;
			if (!(key.dsa()->p = ::BN_new()) || !(key.dsa()->q = ::BN_new()) ||
				!(key.dsa()->g = ::BN_new()) || !(key.dsa()->pub_key = ::BN_new()) ||
				!GetBigNumFromBuffer(&buf, key.dsa()->p) ||
				!GetBigNumFromBuffer(&buf, key.dsa()->q) ||
				!GetBigNumFromBuffer(&buf, key.dsa()->g) ||
				!GetBigNumFromBuffer(&buf, key.dsa()->pub_key))
			{
				::DSA_free(key);
				return false;
			}
			break;
		case KEY_UNSPEC:
			break;
	}
	if (buf.GetLength() != 0)
	{
		if (type == KEY_RSA1 || type == KEY_RSA1)
			::RSA_free(key);
		else if (type == KEY_DSA)
			::DSA_free(key);
		return false;
	}

	keyType = type;
	keyData = key;
	return true;
}

extern "C" bool __stdcall CreateBlobFromKey(KeyType keyType, const KeyData* pKeyData, void** ppvData, size_t* pnLen)
{
	CExBuffer buf;

	if (pKeyData == NULL || !(*pKeyData))
	{
		//error("key_to_blob: key == NULL");
		return 0;
	}
	switch (keyType)
	{
		case KEY_DSA:
			buf.AppendToBufferWithLenCE(KeyTypeToName(keyType));
			AppendBigNumToBuffer(&buf, pKeyData->dsa()->p);
			AppendBigNumToBuffer(&buf, pKeyData->dsa()->q);
			AppendBigNumToBuffer(&buf, pKeyData->dsa()->g);
			AppendBigNumToBuffer(&buf, pKeyData->dsa()->pub_key);
			break;
		case KEY_RSA:
		case KEY_RSA1:
			buf.AppendToBufferWithLenCE(KeyTypeToName(keyType));
			AppendBigNumToBuffer(&buf, pKeyData->rsa()->e);
			AppendBigNumToBuffer(&buf, pKeyData->rsa()->n);
			break;
		default:
			return false;
	}

	size_t nLen = buf.GetLength();
	if (pnLen != NULL)
		*pnLen = nLen;
	if (ppvData != NULL) {
		*ppvData = malloc(nLen);
		memcpy(*ppvData, buf, nLen);
	}
	_SecureStringW::SecureEmptyBuffer(buf, nLen);
	return true;
}

static void __stdcall FreeKeyData(KeyType keyType, KeyData& keyData)
{
	switch (keyType)
	{
		case KEY_RSA1:
		case KEY_RSA:
			::RSA_free(keyData);
			break;
		case KEY_DSA:
			::DSA_free(keyData);
			break;
	}
	keyData = (void*) NULL;
}

////////////////////////////////////////////////////////////////////////////////

// 1: succeed, 0: invalid, -1: error
static int __stdcall VerifyDSSKey(DWORD dwSFlags, DSA* pDSA, const void* pvSignature, size_t nSignLen, const void* pHash, size_t nHashLen)
{
	if (!pDSA)
		return -1;

	void* pSigBlob;
	size_t nBlobLen;

	if (dwSFlags & SSH_BUG_SIGBLOB)
	{
		pSigBlob = malloc(nSignLen);
		memcpy(pSigBlob, pvSignature, nSignLen);
		nBlobLen = nSignLen;
	}
	else
	{
		CExBuffer b;
		void* pv;
		ULONG nLen;
		if (!b.SetDataToBuffer(pvSignature, nSignLen))
			return -1;
		if (!b.GetAndSkipCE(nLen) ||
			!(pv = b.GetCurrentBufferPermanentAndSkip((size_t) nLen)))
			return -1;
		if (strcmp((const char*) pv, "ssh-dss") != 0)
			return -1;
		if (!b.GetAndSkipCE(nLen) ||
			!(pv = b.GetCurrentBufferPermanentAndSkip((size_t) nLen)))
			return -1;
		if (!b.IsEmpty())
			return -1;
		pSigBlob = malloc((size_t) nLen);
		memcpy(pSigBlob, pv, (size_t) nLen);
		nBlobLen = (size_t) nLen;
	}
	if (nBlobLen != SIGBLOB_LEN)
	{
		_SecureStringW::SecureEmptyBuffer(pSigBlob, nBlobLen);
		free(pSigBlob);
		return -1;
	}

	DSA_SIG* pSig;
	if (!(pSig = DSA_SIG_new()))
	{
		_SecureStringW::SecureEmptyBuffer(pSigBlob, nBlobLen);
		free(pSigBlob);
		return -1;
	}
	if (!(pSig->r = BN_new()) || !(pSig->s = BN_new()))
	{
		DSA_SIG_free(pSig);
		_SecureStringW::SecureEmptyBuffer(pSigBlob, nBlobLen);
		free(pSigBlob);
		return -1;
	}
	if (!BN_bin2bn((unsigned char*) pSigBlob, INTBLOB_LEN, pSig->r) ||
		!BN_bin2bn((unsigned char*) pSigBlob + INTBLOB_LEN, INTBLOB_LEN, pSig->s))
	{
		DSA_SIG_free(pSig);
		_SecureStringW::SecureEmptyBuffer(pSigBlob, nBlobLen);
		free(pSigBlob);
		return -1;
	}
	_SecureStringW::SecureEmptyBuffer(pSigBlob, nBlobLen);
	free(pSigBlob);

	const EVP_MD *evp_md = EVP_sha1();
	EVP_MD_CTX md;
	void* pDigest = malloc(EVP_MAX_MD_SIZE);
	if (!pDigest)
	{
		DSA_SIG_free(pSig);
		return -1;
	}

	UINT nDigestLen;
	EVP_DigestInit(&md, evp_md);
	EVP_DigestUpdate(&md, pHash, nHashLen);
	EVP_DigestFinal(&md, (unsigned char*) pDigest, &nDigestLen);

	int ret = DSA_do_verify((unsigned char*) pDigest, nDigestLen, pSig, pDSA);
	_SecureStringW::SecureEmptyBuffer(pDigest, EVP_MAX_MD_SIZE);
	free(pDigest);
	DSA_SIG_free(pSig);
	return ret;
}

// in OpenSSH.cpp
extern "C" int __stdcall openssh_RSA_verify(int type, const u_char* hash, u_int hashlen,
    const u_char* sigbuf, u_int siglen, RSA* rsa);

/* Minimum modulus size (n) for RSA keys. */
#define SSH_RSA_MINIMUM_MODULUS_SIZE	768

// 1: succeed, 0: invalid, -1: error
static int __stdcall VerifyRSAKey(DWORD dwSFlags, RSA* pRSA, const void* pvSignature, size_t nSignLen, const void* pHash, size_t nHashLen)
{
	if (!pRSA)
		return -1;
	if (BN_num_bits(pRSA->n) < SSH_RSA_MINIMUM_MODULUS_SIZE)
		return -1;

	void* pSigBlob;
	size_t nBlobLen;

	{
		CExBuffer b;
		void* pv;
		ULONG nLen;
		if (!b.SetDataToBuffer(pvSignature, nSignLen))
			return -1;
		if (!b.GetAndSkipCE(nLen) ||
			!(pv = b.GetCurrentBufferPermanentAndSkip((size_t) nLen)))
			return -1;
		if (strcmp((const char*) pv, "ssh-rsa") != 0)
			return -1;
		if (!b.GetAndSkipCE(nLen) ||
			!(pv = b.GetCurrentBufferPermanentAndSkip((size_t) nLen)))
			return -1;
		if (!b.IsEmpty())
			return -1;
		pSigBlob = malloc((size_t) nLen);
		memcpy(pSigBlob, pv, (size_t) nLen);
		nBlobLen = (size_t) nLen;
	}

	size_t nRSALen = (size_t) RSA_size(pRSA);
	if (nBlobLen > nRSALen)
	{
		_SecureStringW::SecureEmptyBuffer(pSigBlob, nBlobLen);
		free(pSigBlob);
		return -1;
	}
	else if (nBlobLen < nRSALen)
	{
		size_t diff = nRSALen - nBlobLen;
		void* pv = realloc(pSigBlob, nRSALen);
		if (!pv || pv != pSigBlob)
		{
			_SecureStringW::SecureEmptyBuffer(pSigBlob, nBlobLen);
			free(pSigBlob);
			return -1;
		}
		memmove((BYTE*) pSigBlob + diff, pSigBlob, nBlobLen);
		memset(pSigBlob, 0, diff);
		nBlobLen = nRSALen;
	}
	int nid = (dwSFlags & SSH_BUG_RSASIGMD5) ? NID_md5 : NID_sha1;
	const EVP_MD *evp_md = EVP_get_digestbynid(nid);
	if (!evp_md)
	{
		_SecureStringW::SecureEmptyBuffer(pSigBlob, nBlobLen);
		free(pSigBlob);
		return -1;
	}
	EVP_MD_CTX md;
	void* pDigest = malloc(EVP_MAX_MD_SIZE);
	if (!pDigest)
	{
		_SecureStringW::SecureEmptyBuffer(pSigBlob, nBlobLen);
		free(pSigBlob);
		return -1;
	}

	UINT nDigestLen;
	EVP_DigestInit(&md, evp_md);
	EVP_DigestUpdate(&md, pHash, nHashLen);
	EVP_DigestFinal(&md, (unsigned char*) pDigest, &nDigestLen);

	int ret = openssh_RSA_verify(nid, (unsigned char*) pDigest, nDigestLen, (unsigned char*) pSigBlob, (UINT) nBlobLen, pRSA);
	_SecureStringW::SecureEmptyBuffer(pDigest, EVP_MAX_MD_SIZE);
	_SecureStringW::SecureEmptyBuffer(pSigBlob, nBlobLen);
	free(pDigest);
	free(pSigBlob);
	return ret;
}

// >0: valid, =0: invalid, <0: error
static int __stdcall VerifyServerKey(DWORD dwCompatFlags, KeyType keyType, const KeyData& keyData,
	const void* pvSignature, size_t nSignLen, const void* pHash, size_t nHashLen)
{
	if (!nSignLen)
		return -1;
	switch (keyType)
	{
		case KEY_DSA:
			return VerifyDSSKey(dwCompatFlags, keyData, pvSignature, nSignLen, pHash, nHashLen);
		case KEY_RSA:
			return VerifyRSAKey(dwCompatFlags, keyData, pvSignature, nSignLen, pHash, nHashLen);
		default:
			return -1;
	}
}

static void* __stdcall MakeKeyFromHash(CKeyExchange* pKex, DWORD dwCompatFlags, char id, size_t nSizeNeed,
	const void* pHash, size_t nHashLen, const BIGNUM* pShared)
{
	CExBuffer buf;
	EVP_MD_CTX md;
	size_t nHave;
	int mdsz;
	void* pDigest;

	if ((mdsz = EVP_MD_size(pKex->m_pEVPMD)) <= 0)
	{
		return NULL;
	}
	pDigest = malloc(ROUNDUP(nSizeNeed, (size_t) mdsz));

	AppendBigNumToBuffer(&buf, pShared);

	/* K1 = HASH(K || H || "A" || session_id) */
	EVP_DigestInit(&md, pKex->m_pEVPMD);
	if (!(dwCompatFlags & SSH_BUG_DERIVEKEY))
		EVP_DigestUpdate(&md, buf, buf.GetLength());
	EVP_DigestUpdate(&md, pHash, nHashLen);
	EVP_DigestUpdate(&md, &id, 1);
	EVP_DigestUpdate(&md, pKex->m_pvSessionID, pKex->m_nSessionIDLen);
	EVP_DigestFinal(&md, (unsigned char*) pDigest, NULL);

	/*
	 * expand key:
	 * Kn = HASH(K || H || K1 || K2 || ... || Kn-1)
	 * Key = K1 || K2 || ... || Kn
	 */
	for (nHave = mdsz; nSizeNeed > nHave; nHave += (size_t) mdsz) {
		EVP_DigestInit(&md, pKex->m_pEVPMD);
		if (!(dwCompatFlags & SSH_BUG_DERIVEKEY))
			EVP_DigestUpdate(&md, buf, buf.GetLength());
		EVP_DigestUpdate(&md, pHash, nHashLen);
		EVP_DigestUpdate(&md, (unsigned char*) pDigest, nHave);
		EVP_DigestFinal(&md, (unsigned char*) pDigest + nHave, NULL);
	}

	return pDigest;
}

////////////////////////////////////////////////////////////////////////////////

extern "C" CKeyExchange* __stdcall CKeyExchange::InitKeyExchange(
	LPCWSTR lpszClientVersion,
	LPCWSTR lpszServerVersion,
	LPCSTR lpszFoundKexAlgs,
	LPCSTR lpszFoundHostKeyAlgs,
	size_t nNeedKeyBytes,
	const CExBuffer& myProposals, const CExBuffer& svProposals,
	CSSH2Socket* pSocket)
{
	CKeyExchange* pKex;
	CKeyExchangeClient* pClient;
	const EVP_MD* pEvp = NULL;
	if (strcmp(lpszFoundKexAlgs, KEX_DH1) == 0)
	{
		pClient = new CDHKeyExchangeClient(false);
		pEvp = EVP_sha1();
	}
	else if (strcmp(lpszFoundKexAlgs, KEX_DH14) == 0)
	{
		pClient = new CDHKeyExchangeClient(true);
		pEvp = EVP_sha1();
	}
	else if (strcmp(lpszFoundKexAlgs, KEX_DHGEX_SHA1) == 0)
	{
		pClient = new CGEXKeyExchangeClient(false);
		pEvp = EVP_sha1();
	}
#if OPENSSL_VERSION_NUMBER >= 0x00907000L
	else if (strcmp(lpszFoundKexAlgs, KEX_DHGEX_SHA256) == 0)
	{
		pClient = new CGEXKeyExchangeClient(true);
		pEvp = EVP_sha256();
	}
#endif

	if (!pEvp)
		return NULL;

	pKex = new CKeyExchange();
	pKex->m_pKexClient = pClient;
	pKex->m_pEVPMD = pEvp;
	pKex->m_nNeedKeyBytes = nNeedKeyBytes;
	pKex->m_nKeyType = KeyTypeFromName(lpszFoundHostKeyAlgs);
	pKex->m_strClientVersion = lpszClientVersion;
	pKex->m_strServerVersion = lpszServerVersion;
	pKex->m_bufferMyProposals = myProposals;
	pKex->m_bufferServerProposals = svProposals;

	if (!pClient->Init(pKex, pSocket))
	{
		delete pKex;
		pKex = NULL;
	}

	return pKex;
}

////////////////////////////////////////////////////////////////////////////////

CKeyExchange::CKeyExchange()
{
	m_pKexClient = NULL;
	m_pEVPMD = NULL;
	m_nNeedKeyBytes = 0;
	m_pvSessionID = NULL;
	m_nSessionIDLen = 0;
	for (char i = 0; i < MAX_DERIVE_KEYS; i++)
	{
		m_pvDeriveKeys[i] = NULL;
	}
}

CKeyExchange::~CKeyExchange()
{
	for (char i = 0; i < MAX_DERIVE_KEYS; i++)
	{
		if (m_pvDeriveKeys[i])
			free(m_pvDeriveKeys[i]);
	}
	if (m_pvSessionID)
		free(m_pvSessionID);
	if (m_pKexClient)
		delete m_pKexClient;
}

int CKeyExchange::OnReceiveKexMessages(CFingerPrintHandler* pHandler, CSSH2Socket* pSocket, BYTE bType, const void* pvData, size_t nLen)
{
	return m_pKexClient->OnReceiveKexMessages(pHandler, this, pSocket, bType, pvData, nLen);
}

void CKeyExchange::DoDeriveKeys(DWORD dwCompatFlags, const void* pHash, size_t nHashLen, const BIGNUM* pShared)
{
	for (char i = 0; i < MAX_DERIVE_KEYS; i++)
		m_pvDeriveKeys[i] = MakeKeyFromHash(this, dwCompatFlags, 'A' + i,
			m_nNeedKeyBytes, pHash, nHashLen, pShared);
}

////////////////////////////////////////////////////////////////////////////////

static DH* __stdcall CreateDHByString(const char* pszGen, const char* pszGroup)
{
	DH* dh = DH_new();
	if (!dh || !BN_hex2bn(&dh->p, pszGroup) || !BN_hex2bn(&dh->g, pszGen))
	{
		if (dh)
			DH_free(dh);
		return NULL;
	}
	return dh;
}

static DH* __stdcall CreateDH1()
{
	static const char* gen1 = "2", * group1 =
	    "FFFFFFFF" "FFFFFFFF" "C90FDAA2" "2168C234" "C4C6628B" "80DC1CD1"
	    "29024E08" "8A67CC74" "020BBEA6" "3B139B22" "514A0879" "8E3404DD"
	    "EF9519B3" "CD3A431B" "302B0A6D" "F25F1437" "4FE1356D" "6D51C245"
	    "E485B576" "625E7EC6" "F44C42E9" "A637ED6B" "0BFF5CB6" "F406B7ED"
	    "EE386BFB" "5A899FA5" "AE9F2411" "7C4B1FE6" "49286651" "ECE65381"
	    "FFFFFFFF" "FFFFFFFF";
	return CreateDHByString(gen1, group1);
}

static DH* __stdcall CreateDH14()
{
	static const char* gen14 = "2", * group14 =
	    "FFFFFFFF" "FFFFFFFF" "C90FDAA2" "2168C234" "C4C6628B" "80DC1CD1"
	    "29024E08" "8A67CC74" "020BBEA6" "3B139B22" "514A0879" "8E3404DD"
	    "EF9519B3" "CD3A431B" "302B0A6D" "F25F1437" "4FE1356D" "6D51C245"
	    "E485B576" "625E7EC6" "F44C42E9" "A637ED6B" "0BFF5CB6" "F406B7ED"
	    "EE386BFB" "5A899FA5" "AE9F2411" "7C4B1FE6" "49286651" "ECE45B3D"
	    "C2007CB8" "A163BF05" "98DA4836" "1C55D39A" "69163FA8" "FD24CF5F"
	    "83655D23" "DCA3AD96" "1C62F356" "208552BB" "9ED52907" "7096966D"
	    "670C354E" "4ABC9804" "F1746C08" "CA18217C" "32905E46" "2E36CE3B"
	    "E39E772C" "180E8603" "9B2783A2" "EC07A28F" "B5C55DF0" "6F4C52C9"
	    "DE2BCBF6" "95581718" "3995497C" "EA956AE5" "15D22618" "98FA0510"
	    "15728E5A" "8AACAA68" "FFFFFFFF" "FFFFFFFF";
	return CreateDHByString(gen14, group14);
}

static DH* __stdcall CreateDH(BIGNUM* gen, BIGNUM* modulus)
{
	DH* dh = DH_new();
	if (!dh)
		return NULL;
	dh->p = modulus;
	dh->g = gen;
	return dh;
}

static bool __stdcall IsValidDHPubKey(DH* pDH, BIGNUM* pDHPubNum)
{
	BIGNUM* pTemp;
	int n = BN_num_bits(pDHPubNum), nBitsSet = 0;
	if (pDHPubNum->neg)
		return false;
	if (BN_cmp(pDHPubNum, BN_value_one()) != 1) // pub_exp <= 1
		return false;
	if (!(pTemp = BN_new()))
		return false;
	if (!BN_sub(pTemp, pDH->p, BN_value_one()) ||
		BN_cmp(pDHPubNum, pTemp) != -1) // pub_exp > p-2
	{
		BN_clear_free(pTemp);
		return false;
	}
	BN_clear_free(pTemp);

	for (int i = 0; i <= n; i++)
	{
		if (BN_is_bit_set(pDHPubNum, i))
			nBitsSet++;
	}
	return (nBitsSet > 1);
}

// DHåÆÇê∂ê¨Ç∑ÇÈ
static void __stdcall GenerateDHKey(DH* dh, int nNeedKeyBytes)
{
	int i;

	dh->priv_key = NULL;

	// îÈñßÇ…Ç∑Ç◊Ç´óêêî(X)Çê∂ê¨
	for (i = 0 ; i < 10 ; i++) { // retry counter
		if (dh->priv_key != NULL) {
			BN_clear_free(dh->priv_key);
		}
		dh->priv_key = BN_new();
		if (dh->priv_key == NULL)
			goto error;
		if (BN_rand(dh->priv_key, 2 * nNeedKeyBytes, 0, 0) == 0)
			goto error;
		if (DH_generate_key(dh) == 0)
			goto error;
		if (IsValidDHPubKey(dh, dh->pub_key))
			break;
	}
	if (i >= 10) {
		goto error;
	}
	return;

error:
	//notify_fatal_error(pvar, "error occurred @ dh_gen_key()");
;
}

/*
 * Estimates the group order for a Diffie-Hellman group that has an
 * attack complexity approximately the same as O(2**bits).  Estimate
 * with:  O(exp(1.9223 * (ln q)^(1/3) (ln ln q)^(2/3)))
 */

static int __stdcall EstimateDH(int nBits)
{
	if (nBits <= 128)
		return 1024;	/* O(2**86) */
	if (nBits <= 192)
		return 2048;	/* O(2**116) */
	return 4096;		/* O(2**156) */
}

////////////////////////////////////////////////////////////////////////////////

static BYTE* __stdcall GetFingerPrint(KeyType keyType, const KeyData& keyData, size_t* pnRetLen)
{
	const EVP_MD* md = EVP_md5(); // EVP_sha1();
	BYTE* pBlob, * pRet;
	size_t nLen;
	pBlob = NULL;
	*pnRetLen = 0;
	switch (keyType)
	{
		case KEY_RSA1:
		{
			size_t n = BN_num_bytes(keyData.rsa()->n);
			size_t e = BN_num_bytes(keyData.rsa()->e);
			nLen = n + e;
			pBlob = (BYTE*) malloc(nLen);
			if (!pBlob)
				return NULL;
			BN_bn2bin(keyData.rsa()->n, pBlob);
			BN_bn2bin(keyData.rsa()->e, pBlob + n);
		}
		break;
		case KEY_DSA:
		case KEY_RSA:
			if (!CreateBlobFromKey(keyType, &keyData, (void**) &pBlob, &nLen))
				return NULL;
			break;
		case KEY_UNSPEC:
		default:
			return NULL;
	}
	if (pBlob)
	{
		EVP_MD_CTX ctx;
		unsigned int u;
		pRet = (BYTE*) malloc(EVP_MAX_MD_SIZE);
		::EVP_DigestInit(&ctx, md);
		::EVP_DigestUpdate(&ctx, pBlob, nLen);
		::EVP_DigestFinal(&ctx, pRet, &u);
		*pnRetLen = (size_t) u;
		memset(pBlob, 0, nLen);
		free(pBlob);
		return pRet;
	}
	else
		return NULL;
}

static bool __stdcall VerifyServerHostKey(CFingerPrintHandler* pHandler, KeyType keyType, const KeyData& keyData)
{
	size_t nLen;
	BYTE* pFingerPrint;
	pFingerPrint = GetFingerPrint(keyType, keyData, &nLen);
	if (!pFingerPrint)
		return false;
	bool bRet = pHandler->CheckFingerPrint(pFingerPrint, nLen);
	free(pFingerPrint);
	return bRet;
}

////////////////////////////////////////////////////////////////////////////////

CDHKeyExchangeClient::CDHKeyExchangeClient(bool bGrp14)
	: m_bGrp14(bGrp14)
	, m_pDH(NULL)
{
}

CDHKeyExchangeClient::~CDHKeyExchangeClient()
{
	if (m_pDH)
		DH_free(m_pDH);
}

bool CDHKeyExchangeClient::Init(CKeyExchange* pKex, CSSH2Socket* pSocket)
{
	if (m_bGrp14)
		m_pDH = CreateDH14();
	else
		m_pDH = CreateDH1();
	if (!m_pDH)
		return false;
	GenerateDHKey(m_pDH, (int) (pKex->m_nNeedKeyBytes * 8));

	CExBuffer buf;
	if (!AppendBigNumToBuffer(&buf, m_pDH->pub_key))
		return false;
	if (!pSocket->SendPacket(SSH2_MSG_KEXDH_INIT, buf, buf.GetLength()))
		return false;

	return true;
}

int CDHKeyExchangeClient::OnReceiveKexMessages(CFingerPrintHandler* pHandler, CKeyExchange* pKex, CSSH2Socket* pSocket, BYTE bType, const void* pvData, size_t nLen)
{
	ULONG uSize;
	if (!pvData || bType != SSH2_MSG_KEXDH_REPLY)
		return -1;
	CExBuffer buf;
	if (!buf.SetDataToBuffer(pvData, nLen))
		return -1;

	void* pbuf;
	if (!buf.GetAndSkipCE(uSize) ||
		!(pbuf = buf.GetCurrentBufferPermanentAndSkip((size_t) uSize)))
	{
		return -1;
	}
	CExBuffer bufBlob;
	if (!bufBlob.SetDataToBuffer(pbuf, (size_t) uSize))
		return -1;

	KeyType keyType;
	KeyData keyData;
	if (!CreateKeyFromBlob(pbuf, uSize, keyType, keyData))
		return -1;
	if ((UINT) keyType != pKex->m_nKeyType)
	{
		FreeKeyData(keyType, keyData);
		return -1;
	}
	//bufBlob.ResetPosition();

	// verify server key (pSocket->...)
	if (!VerifyServerHostKey(pHandler, keyType, keyData))
	{
		FreeKeyData(keyType, keyData);
		return -1;
	}

	BIGNUM* pDHServerPublic = BN_new();
	if (!pDHServerPublic)
	{
		FreeKeyData(keyType, keyData);
		return -1;
	}
	if (!GetBigNumFromBuffer(&buf, pDHServerPublic))
	{
		BN_clear_free(pDHServerPublic);
		FreeKeyData(keyType, keyData);
		return -1;
	}
	void* pvSignature;
	if (!buf.GetAndSkipCE(uSize) ||
		!(pvSignature = buf.GetCurrentBufferPermanentAndSkip((size_t) uSize)))
	{
		BN_clear_free(pDHServerPublic);
		FreeKeyData(keyType, keyData);
		return -1;
	}
	if (!buf.IsEmpty())
	{
		BN_clear_free(pDHServerPublic);
		FreeKeyData(keyType, keyData);
		return -1;
	}
	// NOTE: don't use "buf" for write here
#define buf
#define uSize

	if (!IsValidDHPubKey(m_pDH, pDHServerPublic))
	{
		BN_clear_free(pDHServerPublic);
		FreeKeyData(keyType, keyData);
		return -1;
	}
	nLen = (size_t) DH_size(m_pDH);
	pbuf = malloc(nLen);
	int nRet;
	if ((nRet = DH_compute_key((unsigned char*) pbuf, pDHServerPublic, m_pDH)) < 0)
	{
		free(pbuf);
		BN_clear_free(pDHServerPublic);
		FreeKeyData(keyType, keyData);
		return -1;
	}

	BIGNUM* pShared = BN_new();
	if (!pShared)
	{
		free(pbuf);
		BN_clear_free(pDHServerPublic);
		FreeKeyData(keyType, keyData);
		return -1;
	}
	if (!BN_bin2bn((unsigned char*) pbuf, nRet, pShared))
	{
		BN_clear_free(pShared);
		free(pbuf);
		BN_clear_free(pDHServerPublic);
		FreeKeyData(keyType, keyData);
		return -1;
	}
	_SecureStringW::SecureEmptyBuffer(pbuf, nLen);
	free(pbuf);

	void* pHash;
	size_t nHashLen;
	Hash(pKex->m_strClientVersion, pKex->m_strServerVersion,
		pKex->m_bufferMyProposals, pKex->m_bufferMyProposals.GetLength(),
		pKex->m_bufferServerProposals, pKex->m_bufferServerProposals.GetLength(),
		bufBlob, bufBlob.GetLength(),
		m_pDH->pub_key, pDHServerPublic, pShared, &pHash, &nHashLen);
	BN_clear_free(pDHServerPublic);
	DH_free(m_pDH);
	m_pDH = NULL;

#undef uSize
	if (VerifyServerKey(pSocket->GetServerCompatible(), keyType, keyData,
		pvSignature, (size_t) uSize, pHash, nHashLen) != 1)
	{
		BN_clear_free(pShared);
		FreeKeyData(keyType, keyData);
		return -1;
	}
	FreeKeyData(keyType, keyData);
	if (!pKex->m_pvSessionID)
	{
		pKex->m_pvSessionID = malloc(nHashLen);
		pKex->m_nSessionIDLen = nHashLen;
		memcpy(pKex->m_pvSessionID, pHash, nHashLen);
	}

	pKex->DoDeriveKeys(pSocket->GetServerCompatible(), pHash, nHashLen, pShared);
	BN_clear_free(pShared);
	_SecureStringW::SecureEmptyBuffer(pHash, nHashLen);
	free(pHash);
//	kex_finish(kex);

#undef buf
	return 1;
}

void CDHKeyExchangeClient::Hash(
	LPCSTR pszClientVersion,
	LPCSTR pszServerVersion,
	const void* pCKexInit, size_t nCKexInitLen,
	const void* pSKexInit, size_t nSKexInitLen,
	const void* pServerHostKeyBlob, size_t nServerHostKeyBlobLen,
	const BIGNUM* pClientDHPub, const BIGNUM* pServerDHPub, const BIGNUM* pShared,
	void** ppHash, size_t* pnHashLen)
{
	CExBuffer buf;
	EVP_MD_CTX md;
	const EVP_MD* evp_md = EVP_sha1();
	void* pDigest = malloc(EVP_MAX_MD_SIZE);
	if (!pDigest)
	{
		*ppHash = NULL;
		return;
	}
	buf.AppendToBufferWithLenCE(pszClientVersion);
	buf.AppendToBufferWithLenCE(pszServerVersion);

	// kexinit messages: fake header: len+SSH2_MSG_KEXINIT
	buf.AppendToBufferCE((ULONG) nCKexInitLen + 1);
	buf.AppendToBuffer((BYTE) SSH2_MSG_KEXINIT);
	buf.AppendToBuffer(pCKexInit, nCKexInitLen);
	buf.AppendToBufferCE((ULONG) nSKexInitLen + 1);
	buf.AppendToBuffer((BYTE) SSH2_MSG_KEXINIT);
	buf.AppendToBuffer(pSKexInit, nSKexInitLen);

	buf.AppendToBufferWithLenCE(pServerHostKeyBlob, nServerHostKeyBlobLen);
	AppendBigNumToBuffer(&buf, pClientDHPub);
	AppendBigNumToBuffer(&buf, pServerDHPub);
	AppendBigNumToBuffer(&buf, pShared);

	EVP_DigestInit(&md, evp_md);
	EVP_DigestUpdate(&md, buf, buf.GetLength());
	EVP_DigestFinal(&md, (unsigned char*) pDigest, NULL);

	*ppHash = pDigest;
	*pnHashLen = EVP_MD_size(evp_md);
}

////////////////////////////////////////////////////////////////////////////////

CGEXKeyExchangeClient::CGEXKeyExchangeClient(bool bSha256)
	: m_bSha256(bSha256)
	, m_pDH(NULL)
{
}

CGEXKeyExchangeClient::~CGEXKeyExchangeClient()
{
	if (m_pDH)
		DH_free(m_pDH);
}

bool CGEXKeyExchangeClient::Init(CKeyExchange* pKex, CSSH2Socket* pSocket)
{
	CExBuffer buf;

	m_nBits = EstimateDH((int) pKex->m_nNeedKeyBytes);

	if (pSocket->GetServerCompatible() & SSH_OLD_DHGEX)
	{
		buf.AppendToBufferCE((DWORD) m_nBits);
		pSocket->SendPacket(SSH2_MSG_KEX_DH_GEX_REQUEST_OLD, buf, buf.GetLength());
		m_nMin = DH_GRP_MIN;
		m_nMax = DH_GRP_MAX;
	}
	else
	{
		m_nMin = DH_GRP_MIN;
		m_nMax = DH_GRP_MAX;
		buf.AppendToBufferCE((DWORD) m_nMin);
		buf.AppendToBufferCE((DWORD) m_nBits);
		buf.AppendToBufferCE((DWORD) m_nMax);
		pSocket->SendPacket(SSH2_MSG_KEX_DH_GEX_REQUEST, buf, buf.GetLength());
	}
	return true;
}

int CGEXKeyExchangeClient::OnReceiveKexMessages(CFingerPrintHandler* pHandler, CKeyExchange* pKex, CSSH2Socket* pSocket, BYTE bType, const void* pvData, size_t nLen)
{
	if (!pvData)
		return -1;

	BIGNUM* p, * g;
	CExBuffer buf;

	if (!buf.SetDataToBuffer(pvData, nLen))
		return -1;

	if (bType == SSH2_MSG_KEX_DH_GEX_GROUP)
	{
		if ((p = BN_new()) == NULL)
			return -1;
		if (!GetBigNumFromBuffer(&buf, p))
		{
			BN_free(p);
			return -1;
		}
		if ((g = BN_new()) == NULL)
		{
			BN_free(p);
			return -1;
		}
		if (!GetBigNumFromBuffer(&buf, g) || !buf.IsEmpty())
		{
			BN_free(p);
			BN_free(g);
			return -1;
		}

		if (BN_num_bits(p) < m_nMin || BN_num_bits(p) > m_nMax)
		{
			BN_free(p);
			BN_free(g);
			return -1;
		}

		m_pDH = CreateDH(g, p);
		if (!m_pDH)
		{
			BN_free(p);
			BN_free(g);
			return -1;
		}
		GenerateDHKey(m_pDH, (int) (pKex->m_nNeedKeyBytes * 8));

		buf.Empty();
		if (!AppendBigNumToBuffer(&buf, m_pDH->pub_key))
			return -1;
		pSocket->SendPacket(SSH2_MSG_KEX_DH_GEX_INIT, buf, buf.GetLength());
		return 0;
	}
	else if (bType == SSH2_MSG_KEX_DH_GEX_REPLY)
	{
		ULONG uSize;
		void* pv;
		if (!buf.GetAndSkipCE(uSize) ||
			!(pv = buf.GetCurrentBufferPermanentAndSkip((size_t) uSize)))
		{
			return -1;
		}
		CExBuffer bufBlob;
		if (!bufBlob.SetDataToBuffer(pv, (size_t) uSize))
			return -1;

		KeyType keyType;
		KeyData keyData;
		if (!CreateKeyFromBlob(pv, uSize, keyType, keyData))
			return -1;
		if ((UINT) keyType != pKex->m_nKeyType)
		{
			FreeKeyData(keyType, keyData);
			return -1;
		}

		// verify server key (pSocket->...)
		if (!VerifyServerHostKey(pHandler, keyType, keyData))
		{
			FreeKeyData(keyType, keyData);
			return -1;
		}

		BIGNUM* pDHServerPublic = BN_new();
		if (!pDHServerPublic)
		{
			FreeKeyData(keyType, keyData);
			return -1;
		}
		if (!GetBigNumFromBuffer(&buf, pDHServerPublic))
		{
			BN_clear_free(pDHServerPublic);
			FreeKeyData(keyType, keyData);
			return -1;
		}
		void* pvSignature;
		if (!buf.GetAndSkipCE(uSize) ||
			!(pvSignature = buf.GetCurrentBufferPermanentAndSkip((size_t) uSize)))
		{
			BN_clear_free(pDHServerPublic);
			FreeKeyData(keyType, keyData);
			return -1;
		}
		if (!buf.IsEmpty())
		{
			BN_clear_free(pDHServerPublic);
			FreeKeyData(keyType, keyData);
			return -1;
		}
		// NOTE: don't use "buf" for write here
#define buf
#define uSize

		if (!IsValidDHPubKey(m_pDH, pDHServerPublic))
		{
			BN_clear_free(pDHServerPublic);
			FreeKeyData(keyType, keyData);
			return -1;
		}
		nLen = (size_t) DH_size(m_pDH);
		pv = malloc(nLen);
		int nRet;
		if ((nRet = DH_compute_key((unsigned char*) pv, pDHServerPublic, m_pDH)) < 0)
		{
			free(pv);
			BN_clear_free(pDHServerPublic);
			FreeKeyData(keyType, keyData);
			return -1;
		}

		BIGNUM* pShared = BN_new();
		if (!pShared)
		{
			free(pv);
			BN_clear_free(pDHServerPublic);
			FreeKeyData(keyType, keyData);
			return -1;
		}
		if (!BN_bin2bn((unsigned char*) pv, nRet, pShared))
		{
			BN_clear_free(pShared);
			free(pv);
			BN_clear_free(pDHServerPublic);
			FreeKeyData(keyType, keyData);
			return -1;
		}
		_SecureStringW::SecureEmptyBuffer(pv, nLen);
		free(pv);

		if (pSocket->GetServerCompatible() & SSH_OLD_DHGEX)
			m_nMin = m_nMax = -1;

		void* pHash;
		size_t nHashLen;
		Hash(pKex->m_pEVPMD, pKex->m_strClientVersion, pKex->m_strServerVersion,
			pKex->m_bufferMyProposals, pKex->m_bufferMyProposals.GetLength(),
			pKex->m_bufferServerProposals, pKex->m_bufferServerProposals.GetLength(),
			bufBlob, bufBlob.GetLength(),
			m_nMin, m_nBits, m_nMax, m_pDH->p, m_pDH->g,
			m_pDH->pub_key, pDHServerPublic, pShared, &pHash, &nHashLen);
		BN_clear_free(pDHServerPublic);
		DH_free(m_pDH);
		m_pDH = NULL;

#undef uSize
		if (VerifyServerKey(pSocket->GetServerCompatible(), keyType, keyData,
			pvSignature, (size_t) uSize, pHash, nHashLen) != 1)
		{
			BN_clear_free(pShared);
			FreeKeyData(keyType, keyData);
			return -1;
		}
		FreeKeyData(keyType, keyData);
		if (!pKex->m_pvSessionID)
		{
			pKex->m_pvSessionID = malloc(nHashLen);
			pKex->m_nSessionIDLen = nHashLen;
			memcpy(pKex->m_pvSessionID, pHash, nHashLen);
		}

		pKex->DoDeriveKeys(pSocket->GetServerCompatible(), pHash, nHashLen, pShared);
		BN_clear_free(pShared);
		_SecureStringW::SecureEmptyBuffer(pHash, nHashLen);
		free(pHash);
		//kex_finish(kex);

#undef buf
	}
	else
		return -1;
	return 1;
}

void CGEXKeyExchangeClient::Hash(const EVP_MD* pEVPMD,
	LPCSTR pszClientVersion, LPCSTR pszServerVersion,
	const void* pCKexInit, size_t nCKexInitLen,
	const void* pSKexInit, size_t nSKexInitLen,
	const void* pServerHostKeyBlob, size_t nServerHostKeyBlobLen,
	int nMin, int nWantBits, int nMax, const BIGNUM* pPrimeNum, BIGNUM* pGenNum,
	const BIGNUM* pClientDHPub, const BIGNUM* pServerDHPub, const BIGNUM* pShared,
	void** ppHash, size_t* pnHashLen)
{
	CExBuffer buf;
	EVP_MD_CTX md;
	void* pDigest = malloc(EVP_MAX_MD_SIZE);
	if (!pDigest)
	{
		*ppHash = NULL;
		return;
	}
	buf.AppendToBufferWithLenCE(pszClientVersion);
	buf.AppendToBufferWithLenCE(pszServerVersion);

	// kexinit messages: fake header: len+SSH2_MSG_KEXINIT
	buf.AppendToBufferCE((ULONG) nCKexInitLen + 1);
	buf.AppendToBuffer((BYTE) SSH2_MSG_KEXINIT);
	buf.AppendToBuffer(pCKexInit, nCKexInitLen);
	buf.AppendToBufferCE((ULONG) nSKexInitLen + 1);
	buf.AppendToBuffer((BYTE) SSH2_MSG_KEXINIT);
	buf.AppendToBuffer(pSKexInit, nSKexInitLen);

	buf.AppendToBufferWithLenCE(pServerHostKeyBlob, nServerHostKeyBlobLen);
	if (nMin == -1 || nMax == -1)
		buf.AppendToBufferCE((ULONG) nWantBits);
	else
	{
		buf.AppendToBufferCE((ULONG) nMin);
		buf.AppendToBufferCE((ULONG) nWantBits);
		buf.AppendToBufferCE((ULONG) nMax);
	}
	AppendBigNumToBuffer(&buf, pPrimeNum);
	AppendBigNumToBuffer(&buf, pGenNum);
	AppendBigNumToBuffer(&buf, pClientDHPub);
	AppendBigNumToBuffer(&buf, pServerDHPub);
	AppendBigNumToBuffer(&buf, pShared);

	EVP_DigestInit(&md, pEVPMD);
	EVP_DigestUpdate(&md, buf, buf.GetLength());
	EVP_DigestFinal(&md, (unsigned char*) pDigest, NULL);

	*ppHash = pDigest;
	*pnHashLen = EVP_MD_size(pEVPMD);
}
