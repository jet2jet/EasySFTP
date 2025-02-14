#include "stdafx.h"
#include "ShellDLL.h"
#include "SSHAgent.h"

#include "ExBuffer.h"

/*
 * OpenSSH's SSH-2 agent messages.
 */
#define SSH2_AGENTC_REQUEST_IDENTITIES          11
#define SSH2_AGENT_IDENTITIES_ANSWER            12
#define SSH2_AGENTC_SIGN_REQUEST                13
#define SSH2_AGENT_SIGN_RESPONSE                14
#define SSH2_AGENTC_ADD_IDENTITY                17
#define SSH2_AGENTC_REMOVE_IDENTITY             18
#define SSH2_AGENTC_REMOVE_ALL_IDENTITIES       19

int CSSHAgent::GetKeyList2(LPBYTE* ppKeyList)
{
	void* pRet;

	CExBuffer request;
	void* pResponse;
	size_t nResponseLen;
	bool retval;
	request.AppendToBufferCE(static_cast<DWORD>(1));
	request.AppendToBuffer(static_cast<BYTE>(SSH2_AGENTC_REQUEST_IDENTITIES));
	request.ResetPosition();

	pResponse = NULL;
	retval = Query(request, 5, &pResponse, &nResponseLen);
	if (!retval)
	{
		*ppKeyList = NULL;
		if (pResponse)
			free(pResponse);
		return 0;
	}
	CExBuffer response;
	response.AppendToBuffer(pResponse, nResponseLen);
	free(pResponse);

	DWORD dwHead;
	BYTE bMsg;
	if (nResponseLen < 5 || !response.GetAndSkipCE(dwHead) || !response.GetAndSkip(bMsg) ||
		bMsg != SSH2_AGENT_IDENTITIES_ANSWER)
	{
		*ppKeyList = NULL;
		return 0;
	}

	nResponseLen -= 5;
	if (!nResponseLen)
	{
		*ppKeyList = NULL;
		return 0;
	}
	pRet = malloc(sizeof(char) * nResponseLen);
	memcpy(pRet, response.GetCurrentBufferPermanent(nResponseLen), nResponseLen);

	*ppKeyList = (LPBYTE) pRet;
	return static_cast<int>(nResponseLen) - 5;
}

void* CSSHAgent::SignSSH2Key(LPCBYTE pszPubKey, LPCBYTE pszData, size_t nDataLen, size_t* pnOutLen)
{
	void* ret;

	CExBuffer request;
	LPBYTE pResponse;
	size_t nResponseLen;
	bool retval;
	DWORD nPubKeyLen;
	size_t nReqLen;

	nPubKeyLen = ConvertEndian(*reinterpret_cast<const DWORD*>(pszPubKey));
	nReqLen = 4 + 1 + (4 + nPubKeyLen) + (4 + nDataLen) + 4;

	// request length
	request.AppendToBufferCE(static_cast<DWORD>(nReqLen - 4));
	// request type
	request.AppendToBuffer(static_cast<BYTE>(SSH2_AGENTC_SIGN_REQUEST));
	// public key (length + data)
	request.AppendToBuffer(pszPubKey, 4 + nPubKeyLen);
	// sign data (length + data)
	request.AppendToBufferWithLenCE(pszData, nDataLen);
	// flags
	request.AppendToBufferCE(static_cast<DWORD>(0));

	retval = Query(request, nReqLen, (void**)&pResponse, &nResponseLen);
	if (!retval)
	{
		return NULL;
	}
	CExBuffer response;
	response.AppendToBuffer(pResponse, nResponseLen);
	free(pResponse);

	DWORD dwHead;
	BYTE bMsg;
	if (nResponseLen < 5 || !response.GetAndSkipCE(dwHead) ||
		!response.GetAndSkip(bMsg) || bMsg != SSH2_AGENT_SIGN_RESPONSE)
	{
		return NULL;
	}

	nResponseLen -= 5;
	ret = malloc(sizeof(unsigned char) * nResponseLen);
	memcpy(ret, response.GetCurrentBufferPermanent(nResponseLen), nResponseLen);

	if (pnOutLen)
		*pnOutLen = nResponseLen;

	return ret;
}

void CSSHAgent::FreeKeyList(LPBYTE pKeyList)
{
	free(pKeyList);
}
