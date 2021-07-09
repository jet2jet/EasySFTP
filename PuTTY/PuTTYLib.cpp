/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 PuTTYLib.h - implementations of functions for Pageant
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <stdlib.h>
#include <stdio.h>
extern "C" {
#include "putty.h"
#include "ssh.h"
}
#include "PuTTYLib.h"

extern "C"
{
// WINDOWS\WINPGNTC.C
extern int agent_exists(void);
extern int agent_query(void* in, int inlen, void** out, int* outlen,
	void (*callback)(void*, void*, int), void* callback_ctx);
}

EXTERN_C int WINAPI PuTTYGetKeyList1(LPBYTE* ppKeyList)
{
	void* pRet;

	char request[5], * pResponse;
	int nResponseLen, retval;
	request[4] = SSH1_AGENTC_REQUEST_RSA_IDENTITIES;
	PUT_32BIT(request, 4);

	pResponse = NULL;
	retval = agent_query(request, 5, (void**) &pResponse, &nResponseLen, NULL, NULL);
	if (retval != 1)
	{
		*ppKeyList = NULL;
		if (pResponse)
			sfree(pResponse);
		return 0;
	}
	//assert(retval == 1);
	if (nResponseLen < 5 || pResponse[4] != SSH1_AGENT_RSA_IDENTITIES_ANSWER)
	{
		*ppKeyList = NULL;
		sfree(pResponse);
		return 0;
	}

	nResponseLen -= 5;
	pRet = snewn(nResponseLen, char);
	memcpy(pRet, pResponse + 5, nResponseLen);
	sfree(pResponse);

	*ppKeyList = (LPBYTE) pRet;
	return nResponseLen - 5;
}

EXTERN_C int WINAPI PuTTYGetKeyList2(LPBYTE* ppKeyList)
{
	void* pRet;

	char request[5], * pResponse;
	int nResponseLen, retval;
	request[4] = SSH2_AGENTC_REQUEST_IDENTITIES;
	PUT_32BIT(request, 4);

	pResponse = NULL;
	retval = agent_query(request, 5, (void**) &pResponse, &nResponseLen, NULL, NULL);
	if (retval != 1)
	{
		*ppKeyList = NULL;
		if (pResponse)
			sfree(pResponse);
		return 0;
	}
	//assert(retval == 1);
	if (nResponseLen < 5 || pResponse[4] != SSH2_AGENT_IDENTITIES_ANSWER)
	{
		*ppKeyList = NULL;
		sfree(pResponse);
		return 0;
	}

	nResponseLen -= 5;
	pRet = snewn(nResponseLen, char);
	memcpy(pRet, pResponse + 5, nResponseLen);
	sfree(pResponse);

	*ppKeyList = (LPBYTE) pRet;
	return nResponseLen - 5;
}

/*
 * for SSH2
 *   公開鍵とデータ(同じく公開鍵)を渡し、
 *   公開鍵によって署名されたデータを得る
 */
EXTERN_C void* PuTTYSignSSH2Key(LPCBYTE pszPubKey, LPCBYTE pszData, DWORD nDataLen, DWORD* pnOutLen)
{
	void* ret;

	LPBYTE pRequest, pResponse;
	int nResponseLen, retval;
	DWORD nPubKeyLen, nReqLen;

	nPubKeyLen = GET_32BIT(pszPubKey);
	nReqLen = 4 + 1 + (4 + nPubKeyLen) + (4 + nDataLen);
	pRequest = (LPBYTE) malloc((size_t) nReqLen);

	// request length
	PUT_32BIT(pRequest, nReqLen);
	// request type
	pRequest[4] = SSH2_AGENTC_SIGN_REQUEST;
	// public key (length + data)
	memcpy(pRequest + 5, pszPubKey, 4 + nPubKeyLen);
	// sign data (length + data)
	PUT_32BIT(pRequest + 5 + 4 + nPubKeyLen, nDataLen);
	memcpy(pRequest + 5 + 4 + nPubKeyLen + 4, pszData, nDataLen);

	retval = agent_query(pRequest, (int) nReqLen, (void**) &pResponse, &nResponseLen, NULL, NULL);
	if (retval != 1)
	{
		sfree(pRequest);
		return NULL;
	}
	if (nResponseLen < 5 || pResponse[4] != SSH2_AGENT_SIGN_RESPONSE)
	{
		sfree(pRequest);
		return NULL;
	}

	nResponseLen -= 5;
	ret = snewn(nResponseLen, unsigned char);
	memcpy(ret, pResponse + 5, nResponseLen);
	sfree(pResponse);

	if (pnOutLen)
		*pnOutLen = (DWORD) nResponseLen;

	return ret;
}

/*
 * for SSH1
 *   公開鍵と暗号化データを渡し
 *   復号データのハッシュを得る
 */
EXTERN_C void* WINAPI PuTTYHashSSH1Challenge(LPCBYTE pPubKey, DWORD nPubKeyLen,
	LPCBYTE pData, DWORD nDataLen, LPCSTR pszSessionId, DWORD* pnOutLen)
{
	void* ret;

	LPBYTE pRequest, pResponse, p;
	int nResponseLen, retval;
	DWORD nReqLen;

	nReqLen = 4 + 1 + nPubKeyLen + nDataLen + 16 + 4;
	pRequest = (LPBYTE) malloc(nReqLen);
	p = pRequest;

	// request length
	PUT_32BIT(pRequest, nReqLen);
	// request type
	pRequest[4] = SSH1_AGENTC_RSA_CHALLENGE;
	p += 5;

	// public key
	memcpy(p, pPubKey, nPubKeyLen);
	p += nPubKeyLen;
	// challange from server
	memcpy(p, pData, nDataLen);
	p += nDataLen;
	// session_id
	memcpy(p, pszSessionId, 16);
	p += 16;
	// terminator?
	PUT_32BIT(p, 1);

	retval = agent_query(pRequest, (int) nReqLen, (void**) &pResponse, &nResponseLen, NULL, NULL);
	if (retval != 1)
	{
		if (pnOutLen)
			*pnOutLen = 0;
		return NULL;
	}
	//assert(retval == 1);
	if (nResponseLen < 5 || pResponse[4] != SSH1_AGENT_RSA_RESPONSE)
		return NULL;

	nResponseLen -= 5;
	ret = snewn(nResponseLen, unsigned char);
	memcpy(ret, pResponse + 5, nResponseLen);
	sfree(pResponse);

	if (pnOutLen)
		*pnOutLen = (DWORD) nResponseLen;

	return ret;
}

EXTERN_C int WINAPI PuTTYGetSSH1KeyLen(LPBYTE pKey, int nMaxLen)
{
	return rsa_public_blob_len(pKey, nMaxLen);
}

EXTERN_C void WINAPI PuTTYFreeKeyList(LPBYTE pKeyList)
{
	sfree(pKeyList);
}

////////////////////////////////////////////////////////////////////////////////

// SSHRSA.C
/* Given a public blob, determine its length. */
EXTERN_C int rsa_public_blob_len(void* data, int maxlen)
{
	unsigned char* p = (unsigned char*) data;
	int n;

	if (maxlen < 4)
		return -1;
	p += 4;			       /* length word */
	maxlen -= 4;

	n = ssh1_read_bignum(p, maxlen, NULL);    /* exponent */
	if (n < 0)
		return -1;
	p += n;

	n = ssh1_read_bignum(p, maxlen, NULL);    /* modulus */
	if (n < 0)
		return -1;
	p += n;

	return p - (unsigned char*) data;
}

// WINDOWS\WINDOW.C
/*
 * Print a modal (Really Bad) message box and perform a fatal exit.
 */
EXTERN_C void modalfatalbox(char* fmt, ...)
{
	va_list ap;
	char* stuff, morestuff[100];

	va_start(ap, fmt);
	stuff = dupvprintf(fmt, ap);
	va_end(ap);
	sprintf(morestuff, "%.70s Fatal Error", "TTSSH");
	MessageBox(NULL, stuff, morestuff, MB_SYSTEMMODAL | MB_ICONERROR | MB_OK);
	sfree(stuff);
}
