/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 FTPMsg.cpp - implementations of cipher functions and classes for SSH
 */

#include "stdafx.h"
#include "Cipher.h"

#include "SUString.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

////////////////////////////////////////////////////////////////////////////////

#define SSH_EVP     1	/* OpenSSL EVP-based MAC */
#define SSH_UMAC    2	/* UMAC (not integrated with OpenSSL) */

static const struct
{
	LPCSTR lpszName;
	int nType;
	const EVP_MD* (*mdfunc)(void);
	int nTruncateBits;	/* truncate digest if != 0 */
	size_t nKeyLen;		/* just for UMAC */
	size_t nLen;		/* just for UMAC */
} s_macs[] = {
	{ "hmac-sha1",			SSH_EVP, EVP_sha1, 0, -1, -1 },
	{ "hmac-sha1-96",		SSH_EVP, EVP_sha1, 96, -1, -1 },
	{ "hmac-md5",			SSH_EVP, EVP_md5, 0, -1, -1 },
	{ "hmac-md5-96",		SSH_EVP, EVP_md5, 96, -1, -1 },
	{ "hmac-ripemd160",		SSH_EVP, EVP_ripemd160, 0, -1, -1 },
	{ "hmac-ripemd160@openssh.com",	SSH_EVP, EVP_ripemd160, 0, -1, -1 },
	//{ "umac-64@openssh.com",	SSH_UMAC, NULL, 0, 128, 64 },
	{ NULL,				0, NULL, 0, -1, -1 }
};

extern "C" void __stdcall MacSetupById(CMacData* pMac, int nID)
{
	int nEvpLen;
	pMac->nType = s_macs[nID].nType;
	if (pMac->nType == SSH_EVP)
	{
		pMac->pEvpCtx = HMAC_CTX_new();
		if (!pMac->pEvpCtx)
		{
			return;
		}
		pMac->pEVPMD = (*s_macs[nID].mdfunc)();
		if ((nEvpLen = EVP_MD_size(pMac->pEVPMD)) <= 0)
		{
			//fatal("mac %s len %d", pMac->name, evp_len);
			return;
		}
		pMac->nKeyLen = pMac->nMacLen = (size_t) nEvpLen;
	}
	else
	{
		pMac->pEvpCtx = NULL;
		pMac->pEVPMD = NULL;
		pMac->nMacLen = s_macs[nID].nLen / 8;
		pMac->nKeyLen = s_macs[nID].nKeyLen / 8;
		//pMac->umac_ctx = NULL;
	}
	pMac->lpszName = s_macs[nID].lpszName;
	pMac->pKeyData = NULL;
	if (s_macs[nID].nTruncateBits != 0)
		pMac->nMacLen = s_macs[nID].nTruncateBits / 8;
}

extern "C" bool __stdcall MacSetup(CMacData* pMac, LPCSTR lpszName)
{
	for (int i = 0; s_macs[i].lpszName; i++)
	{
		if (strcmp(lpszName, s_macs[i].lpszName) == 0)
		{
			if (pMac != NULL)
				MacSetupById(pMac, i);
			return true;
		}
	}
	return false;
}

extern "C" bool __stdcall MacInit(CMacData* pMac)
{
	if (!pMac->pKeyData)
		return false;
	switch (pMac->nType)
	{
		case SSH_EVP:
			if (!pMac->pEVPMD || !pMac->pEvpCtx)
				return false;
			HMAC_Init(pMac->pEvpCtx, pMac->pKeyData, (int) pMac->nKeyLen, pMac->pEVPMD);
			break;
		case SSH_UMAC:
			// unsupported
			return false;
		default:
			return false;
	}
	return true;
}

extern "C" void* __stdcall MacCompute(CMacData* pMac, UINT uSeqNum, const void* pData, size_t nLen)
{
	void* pRet;
	BYTE b[4];
	//BYTE nonce[8];

	if (pMac->nMacLen > EVP_MAX_MD_SIZE)
		return NULL;

	pRet = malloc(EVP_MAX_MD_SIZE);
	if (!pRet)
		return NULL;

	switch (pMac->nType) {
		case SSH_EVP:
			b[0] = HIBYTE(HIWORD(uSeqNum));
			b[1] = LOBYTE(HIWORD(uSeqNum));
			b[2] = HIBYTE(LOWORD(uSeqNum));
			b[3] = LOBYTE(LOWORD(uSeqNum));
			/* reset HMAC context */
			HMAC_Init(pMac->pEvpCtx, NULL, 0, NULL);
			HMAC_Update(pMac->pEvpCtx, b, sizeof(b));
			HMAC_Update(pMac->pEvpCtx, (const unsigned char*) pData, nLen);
			HMAC_Final(pMac->pEvpCtx, (unsigned char*) pRet, NULL);
			break;
		//case SSH_UMAC:
		//	put_u64(nonce, uSeqNum);
		//	umac_update(pMac->umacCtx, (const unsigned char*) pData, nLen);
		//	umac_final(pMac->umacCtx, pRet, nonce);
		//	break;
		default:
			free(pRet);
			return NULL;
	}
	return pRet;
}

extern "C" void __stdcall MacCleanup(CMacData* pMac)
{
	if (pMac->nType == SSH_EVP)
	{
		if (pMac->pEVPMD && pMac->pEvpCtx)
		{
			HMAC_CTX_free(pMac->pEvpCtx);
			pMac->pEvpCtx = NULL;
		}
	}
	pMac->pEVPMD = NULL;
}

////////////////////////////////////////////////////////////////////////////////

static CCipher s_ciphers[] = {
	{ "none",		SSH_CIPHER_NONE, 8, 0, 0, 0, EVP_enc_null },
	{ "des",		SSH_CIPHER_DES, 8, 8, 0, 1, EVP_des_cbc },
	//{ "3des",		SSH_CIPHER_3DES, 8, 16, 0, 1, evp_ssh1_3des },
	//{ "blowfish",		SSH_CIPHER_BLOWFISH, 8, 32, 0, 1, evp_ssh1_bf },

	{ "3des-cbc",		SSH_CIPHER_SSH2, 8, 24, 0, 1, EVP_des_ede3_cbc },
	{ "blowfish-cbc",	SSH_CIPHER_SSH2, 8, 16, 0, 1, EVP_bf_cbc },
	{ "cast128-cbc",	SSH_CIPHER_SSH2, 8, 16, 0, 1, EVP_cast5_cbc },
	{ "arcfour",		SSH_CIPHER_SSH2, 8, 16, 0, 0, EVP_rc4 },
	{ "arcfour128",		SSH_CIPHER_SSH2, 8, 16, 1536, 0, EVP_rc4 },
	{ "arcfour256",		SSH_CIPHER_SSH2, 8, 32, 1536, 0, EVP_rc4 },
	{ "aes128-cbc",		SSH_CIPHER_SSH2, 16, 16, 0, 1, EVP_aes_128_cbc },
	{ "aes192-cbc",		SSH_CIPHER_SSH2, 16, 24, 0, 1, EVP_aes_192_cbc },
	{ "aes256-cbc",		SSH_CIPHER_SSH2, 16, 32, 0, 1, EVP_aes_256_cbc },
	{ "rijndael-cbc@lysator.liu.se",
				SSH_CIPHER_SSH2, 16, 32, 0, 1, EVP_aes_256_cbc },
	//{ "aes128-ctr",		SSH_CIPHER_SSH2, 16, 16, 0, 0, evp_aes_128_ctr },
	//{ "aes192-ctr",		SSH_CIPHER_SSH2, 16, 24, 0, 0, evp_aes_128_ctr },
	//{ "aes256-ctr",		SSH_CIPHER_SSH2, 16, 32, 0, 0, evp_aes_128_ctr },
#ifdef USE_CIPHER_ACSS
	{ "acss@openssh.org",	SSH_CIPHER_SSH2, 16, 5, 0, 0, EVP_acss },
#endif
	{ NULL,			SSH_CIPHER_INVALID, 0, 0, 0, 0, NULL }
};

extern "C" CCipher* __stdcall GetCipherByName(LPCSTR lpszName)
{
	for (int i = 0; s_ciphers[i].lpszName; i++)
	{
		if (strcmp(s_ciphers[i].lpszName, lpszName) == 0)
			return &s_ciphers[i];
	}
	return NULL;
}

extern "C" bool __stdcall InitCipherContext(CCipherContext* pContext, CCipher* pCipher,
	const void* pKey, size_t nKeyLen, const void* pIV, size_t nIVLen, bool bEncrypt)
{
	//static int dowarn = 1;
	const EVP_CIPHER* pType;
	int nKLen;
	void* pJunk, * pDiscard;

	if (pCipher->nType == SSH_CIPHER_DES) {
		//if (dowarn) {
		//	error("Warning: use of DES is strongly discouraged "
		//	    "due to cryptographic weaknesses");
		//	dowarn = 0;
		//}
		if (nKeyLen > 8)
			nKeyLen = 8;
	}
	//pContext->plaintext = (cipher->nType == SSH_CIPHER_NONE);

	if (nKeyLen < pCipher->nKeyLen)
	{
		//fatal("cipher_init: key length %d is insufficient for %s.",
		//    nKeyLen, pCipher->lpszName);
		return false;
	}
	if (pIV != NULL && nIVLen < pCipher->nBlockSize)
	{
		//fatal("cipher_init: iv length %d is insufficient for %s.",
		//    nIVLen, pCipher->lpszName);
		return false;
	}
	pContext->pCipher = pCipher;

	pContext->pEvp = EVP_CIPHER_CTX_new();
	if (!pContext->pEvp)
	{
		return false;
	}

	pType = (*pCipher->GetEvpType)();

	EVP_CIPHER_CTX_init(pContext->pEvp);
	if (EVP_CipherInit(pContext->pEvp, pType, NULL, (unsigned char*) pIV,
	    !!bEncrypt) == 0)
	{
		//fatal("cipher_init: EVP_CipherInit failed for %s",
		//    pCipher->lpszName);
		EVP_CIPHER_CTX_free(pContext->pEvp);
		pContext->pEvp = NULL;
		return false;
	}
	nKLen = EVP_CIPHER_CTX_key_length(pContext->pEvp);
	if (nKLen > 0 && nKeyLen != (size_t) nKLen)
	{
		if (EVP_CIPHER_CTX_set_key_length(pContext->pEvp, (int) nKeyLen) == 0)
		{
			//fatal("cipher_init: set keylen failed (%d -> %d)",
			//    nKLen, nKeyLen);
			EVP_CIPHER_CTX_free(pContext->pEvp);
			pContext->pEvp = NULL;
			return false;
		}
	}
	if (EVP_CipherInit(pContext->pEvp, NULL, (unsigned char*) pKey, NULL, -1) == 0)
	{
		//fatal("cipher_init: EVP_CipherInit: set key failed for %s",
		//    pCipher->lpszName);
		EVP_CIPHER_CTX_free(pContext->pEvp);
		pContext->pEvp = NULL;
		return false;
	}

	if (pCipher->nDiscardLen > 0)
	{
		pJunk = malloc(pCipher->nDiscardLen);
		pDiscard = malloc(pCipher->nDiscardLen);
		if (EVP_Cipher(pContext->pEvp, (unsigned char*) pDiscard,
			(unsigned char*) pJunk, (unsigned int) pCipher->nDiscardLen) == 0)
		{
			//fatal("evp_crypt: EVP_Cipher failed during discard");
			EVP_CIPHER_CTX_free(pContext->pEvp);
			pContext->pEvp = NULL;
			return false;
		}
		_SecureStringW::SecureEmptyBuffer(pDiscard, pCipher->nDiscardLen);
		free(pJunk);
		free(pDiscard);
	}
	return true;
}

extern "C" void __stdcall CleanupCipherContext(CCipherContext* pContext)
{
	EVP_CIPHER_CTX_cleanup(pContext->pEvp);
	EVP_CIPHER_CTX_free(pContext->pEvp);
	pContext->pEvp = NULL;
}

extern "C" bool __stdcall CryptWithCipherContext(CCipherContext* pContext, void* pvDest, const void* pvSrc, size_t nSize)
{
	if ((nSize % pContext->pCipher->nBlockSize) != 0)
		return false;
	return EVP_Cipher(pContext->pEvp, (unsigned char*) pvDest, (const unsigned char*) pvSrc, (unsigned int) nSize) != 0;
}

////////////////////////////////////////////////////////////////////////////////
