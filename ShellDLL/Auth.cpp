/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 Auth.cpp - implementations of authentication classes for SSH
 */

#include "StdAfx.h"
#include "Auth.h"

#include "ExBuffer.h"
#include "PuTTYLib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool CPasswordAuthentication::Authenticate(CSSH2Socket* pSocket, CUserInfo* pUser, LPCSTR lpszService)
{
	LPCSTR lpszUser;

	lpszUser = (LPCSTR) pUser->strName.AllocUTF8String();

	CExBuffer buf;
	buf.AppendToBufferWithLenCE(lpszUser);
	buf.AppendToBufferWithLenCE(lpszService);
	buf.AppendToBufferWithLenCE("password");
	buf.AppendToBuffer((BYTE) 0);
	{
		CMyStringW str;
		pUser->strPassword.GetString(str);
		buf.AppendToBufferWithLenCE(str);
		_SecureStringW::SecureEmptyString(str);
	}
	bool bRet = pSocket->SendPacket(SSH2_MSG_USERAUTH_REQUEST, buf, buf.GetLength(), 64);
	return bRet;
}

////////////////////////////////////////////////////////////////////////////////

bool CChangePasswordAuthentication::Authenticate(CSSH2Socket* pSocket, CUserInfo* pUser, LPCSTR lpszService)
{
	LPCSTR lpszUser;

	lpszUser = (LPCSTR) pUser->strName.AllocUTF8String();

	CExBuffer buf;
	buf.AppendToBufferWithLenCE(lpszUser);
	buf.AppendToBufferWithLenCE(lpszService);
	buf.AppendToBufferWithLenCE("password");
	buf.AppendToBuffer((BYTE) 1);
	{
		CMyStringW str;
		pUser->strPassword.GetString(str);
		buf.AppendToBufferWithLenCE(str);
		_SecureStringW::SecureEmptyString(str);
		pUser->strNewPassword.GetString(str);
		buf.AppendToBufferWithLenCE(str);
		_SecureStringW::SecureEmptyString(str);
	}
	bool bRet = pSocket->SendPacket(SSH2_MSG_USERAUTH_REQUEST, buf, buf.GetLength(), 64);
	return bRet;
}

////////////////////////////////////////////////////////////////////////////////

static bool __stdcall SignSSH2Key(DWORD dwCompat, const KeyData& keyData, KeyType keyType, const void* pData, size_t nDataLen, void** ppSigned, size_t* pnSignedLen)
{
	void* pDigest;
	CExBuffer buf;

	if (!keyData.pKeyUnknown)
		return false;
	if (keyType != KEY_RSA && keyType != KEY_RSA1 && keyType != KEY_DSA)
		return false;
	pDigest = malloc(EVP_MAX_MD_SIZE);
	if (!pDigest)
		return false;
	if (keyType == KEY_RSA || keyType == KEY_RSA1)
	{
		const EVP_MD *evp_md;
		EVP_MD_CTX* pmd;
		void* pSig;
		UINT nSLen, nDLen;
		size_t nLen;
		int ret, nNID;

		pmd = EVP_MD_CTX_new();
		if (!pmd)
		{
			free(pDigest);
			return false;
		}

		nNID = (dwCompat & SSH_BUG_RSASIGMD5) ? NID_md5 : NID_sha1;
		if (!(evp_md = EVP_get_digestbynid(nNID)))
		{
			//error("ssh_rsa_sign: EVP_get_digestbynid %d failed", nNID);
			EVP_MD_CTX_free(pmd);
			free(pDigest);
			return false;
		}
		EVP_DigestInit(pmd, evp_md);
		EVP_DigestUpdate(pmd, pData, nDataLen);
		EVP_DigestFinal(pmd, (unsigned char*) pDigest, &nDLen);
		EVP_MD_CTX_free(pmd);

		nSLen = RSA_size(keyData);
		pSig = malloc(nSLen);
		if (!pSig)
		{
			_SecureStringW::SecureEmptyBuffer(pDigest, EVP_MAX_MD_SIZE);
			free(pDigest);
			return false;
		}

		nLen = 0;
		ret = RSA_sign(nNID, (unsigned char*) pDigest, nDLen, (unsigned char*) pSig, (UINT*) &nLen, keyData);
		_SecureStringW::SecureEmptyBuffer(pDigest, EVP_MAX_MD_SIZE);
		free(pDigest);

		if (ret != 1)
		{
			//int ecode = ERR_get_error();

			//error("ssh_rsa_sign: RSA_sign failed: %s",
			//	ERR_error_string(ecode, NULL));
			free(pSig);
			return false;
		}
		if (nLen < (size_t) nSLen)
		{
			size_t nDiff = (size_t) nSLen - nLen;
			//debug("nSLen %u > nLen %u", nSLen, nLen);
			memmove((unsigned char*) pSig + nDiff, pSig, nLen);
			memset(pSig, 0, nDiff);
		}
		else if (nLen > (size_t) nSLen)
		{
			//error("ssh_rsa_sign: nSLen %u nLen %u", nSLen, nLen);
			free(pSig);
			return false;
		}

		/// encode signature
		buf.AppendToBufferWithLenCE("ssh-rsa");
		buf.AppendToBufferWithLenCE(pSig, (size_t) nSLen);
		nLen = buf.GetLength();
		if (pnSignedLen != NULL)
			*pnSignedLen = nLen;
		if (ppSigned != NULL) {
			*ppSigned = malloc(nLen);
			memcpy(*ppSigned, buf, nLen);
		}
		_SecureStringW::SecureEmptyBuffer(pSig, (size_t) nSLen);
		free(pSig);
	}
	else if (keyType == KEY_DSA)
	{
		DSA_SIG* sig;
		const EVP_MD* evp_md = EVP_sha1();
		EVP_MD_CTX* pmd;
		void* pSigBlob;
		UINT nRLen, nSLen, nDLen;
		size_t nLen;

		pSigBlob = malloc(SIGBLOB_LEN);
		if (!pSigBlob)
		{
			free(pDigest);
			return false;
		}
		pmd = EVP_MD_CTX_new();
		if (!pmd)
		{
			free(pSigBlob);
			free(pDigest);
			return false;
		}
		EVP_DigestInit(pmd, evp_md);
		EVP_DigestUpdate(pmd, pData, nDataLen);
		EVP_DigestFinal(pmd, (unsigned char*) pDigest, &nDLen);
		EVP_MD_CTX_free(pmd);

		sig = DSA_do_sign((unsigned char*) pDigest, nDLen, keyData);
		_SecureStringW::SecureEmptyBuffer(pDigest, EVP_MAX_MD_SIZE);
		free(pDigest);

		if (sig == NULL)
		{
			//error("ssh_dss_sign: sign failed");
			return false;
		}

		const BIGNUM* r, * s;
		DSA_SIG_get0(sig, &r, &s);
		nRLen = BN_num_bytes(r);
		nSLen = BN_num_bytes(s);
		if (nRLen > INTBLOB_LEN || nSLen > INTBLOB_LEN)
		{
			//error("bad sig size %u %u", nRLen, nSLen);
			DSA_SIG_free(sig);
			return false;
		}
		memset(pSigBlob, 0, SIGBLOB_LEN);
		BN_bn2bin(r, (unsigned char*) pSigBlob + SIGBLOB_LEN - INTBLOB_LEN - nRLen);
		BN_bn2bin(s, (unsigned char*) pSigBlob + SIGBLOB_LEN - nSLen);
		DSA_SIG_free(sig);

		if (dwCompat & SSH_BUG_SIGBLOB)
		{
			if (pnSignedLen != NULL)
				*pnSignedLen = SIGBLOB_LEN;
			if (ppSigned != NULL)
			{
				*ppSigned = malloc(SIGBLOB_LEN);
				memcpy(*ppSigned, pSigBlob, SIGBLOB_LEN);
			}
		}
		else
		{
			// ietf-drafts
			buf.AppendToBufferWithLenCE("ssh-dss");
			buf.AppendToBufferWithLenCE(pSigBlob, (size_t) SIGBLOB_LEN);

			nLen = buf.GetLength();
			if (pnSignedLen != NULL)
				*pnSignedLen = nLen;
			if (ppSigned != NULL)
			{
				*ppSigned = malloc(nLen);
				memcpy(*ppSigned, buf, nLen);
			}
		}
		_SecureStringW::SecureEmptyBuffer(pSigBlob, SIGBLOB_LEN);
		free(pSigBlob);
	}
	return true;
}

bool CPublicKeyAuthentication::Authenticate(CSSH2Socket* pSocket, CUserInfo* pUser, LPCSTR lpszService)
{
	LPCSTR lpszUser, lpszKeyType;
	void* pBlob, * pSign;
	size_t nBlobLen, nSignLen;

	lpszUser = (LPCSTR) pUser->strName.AllocUTF8String();

	if (!(lpszKeyType = KeyTypeToName(pUser->keyType)))
		return false;
	if (!CreateBlobFromKey(pUser->keyType, &pUser->keyData, &pBlob, &nBlobLen))
	{
		return false;
	}

	CExBuffer buf;
	DWORD dwCompat = pSocket->GetServerCompatible();
	size_t nSkipLen;
	if (dwCompat & SSH_OLD_SESSIONID)
	{
		buf.AppendToBuffer(pUser->pvSessionID, pUser->nSessionIDLen);
		nSkipLen = pUser->nSessionIDLen;
	}
	else
	{
		buf.AppendToBufferWithLenCE(pUser->pvSessionID, pUser->nSessionIDLen);
		nSkipLen = buf.GetLength();
	}
	buf.AppendToBuffer((BYTE) SSH2_MSG_USERAUTH_REQUEST);
	buf.AppendToBufferWithLenCE(lpszUser);
	buf.AppendToBufferWithLenCE((dwCompat & SSH_BUG_PKSERVICE) ? "ssh-userauth" : lpszService);
	if (dwCompat & SSH_BUG_PKAUTH)
		buf.AppendToBuffer((BYTE) 1);   // bHaveSig
	else
	{
		buf.AppendToBufferWithLenCE("publickey");
		buf.AppendToBuffer((BYTE) 1);   // bHaveSig
		buf.AppendToBufferWithLenCE(lpszKeyType);
	}
	buf.AppendToBufferWithLenCE(pBlob, nBlobLen);

	if (!SignSSH2Key(dwCompat, pUser->keyData, pUser->keyType, buf, buf.GetLength(), &pSign, &nSignLen))
	{
		return false;
	}

	if (dwCompat & SSH_BUG_PKSERVICE)
	{
		buf.Empty();
		buf.AppendToBuffer(pUser->pvSessionID, pUser->nSessionIDLen);
		nSkipLen = pUser->nSessionIDLen;
		buf.AppendToBuffer((BYTE) SSH2_MSG_USERAUTH_REQUEST);
		buf.AppendToBufferWithLenCE(lpszUser);
		buf.AppendToBufferWithLenCE((dwCompat & SSH_BUG_PKSERVICE) ? "ssh-userauth" : lpszService);
		buf.AppendToBufferWithLenCE("publickey");
		buf.AppendToBuffer((BYTE) 1);   // bHaveSig
		if (!(dwCompat & SSH_BUG_PKAUTH))
			buf.AppendToBufferWithLenCE(lpszKeyType);
		buf.AppendToBufferWithLenCE(pBlob, nBlobLen);
	}
	_SecureStringW::SecureEmptyBuffer(pBlob, nBlobLen);
	free(pBlob);

	buf.AppendToBufferWithLenCE(pSign, nSignLen);
	_SecureStringW::SecureEmptyBuffer(pSign, nSignLen);
	free(pSign);

	register bool ret = false;
	if (buf.GetLength() >= nSkipLen + 1)
	{
		buf.SkipPosition((long) (nSkipLen + 1));
		ret = pSocket->SendPacket(SSH2_MSG_USERAUTH_REQUEST, buf, buf.GetLength());
		buf.ResetPosition();
	}
	_SecureStringW::SecureEmptyBuffer(buf, buf.GetLength());

	return ret;
}

////////////////////////////////////////////////////////////////////////////////

bool CPageantAuthentication::Authenticate(CSSH2Socket* pSocket, CUserInfo* pUser, LPCSTR lpszService)
{
	if (!m_lpPageantKeyList)
	{
		m_lpPageantKeyList = pUser->lpPageantKeyList;
		pUser->lpPageantKeyList = NULL;
		if (!m_lpPageantKeyList)
			return false;
		m_dwKeyCount = ConvertEndian(*((DWORD*) m_lpPageantKeyList));
		m_dwKeyIndex = 0;
		m_lpCurrentKey = m_lpPageantKeyList + 4;
	}
	LPBYTE p = m_lpCurrentKey;

	LPCSTR lpszUser, lpszKeyType;
	void* pBlob;
	size_t nBlobLen;

	lpszUser = (LPCSTR) pUser->strName.AllocUTF8String();

	// get key type data (in the head of blob data)
	DWORD dwKeyTypeLen = ConvertEndian(*((DWORD*) (p + 4)));
	lpszKeyType = (LPCSTR) (p + 8);

	nBlobLen = (size_t) ConvertEndian(*((DWORD*) p));
	pBlob = (p + 4);
	p += nBlobLen + 4;

	CExBuffer buf;
	DWORD dwCompat = pSocket->GetServerCompatible();
	size_t nSkipLen;
	if (dwCompat & SSH_OLD_SESSIONID)
	{
		buf.AppendToBuffer(pUser->pvSessionID, pUser->nSessionIDLen);
		nSkipLen = pUser->nSessionIDLen + 1;
	}
	else
	{
		buf.AppendToBufferWithLenCE(pUser->pvSessionID, pUser->nSessionIDLen);
		nSkipLen = buf.GetLength() + 1;
	}
	buf.AppendToBuffer((BYTE) SSH2_MSG_USERAUTH_REQUEST);
	buf.AppendToBufferWithLenCE(lpszUser);
	buf.AppendToBufferWithLenCE((dwCompat & SSH_BUG_PKSERVICE) ? "ssh-userauth" : lpszService);
	if (dwCompat & SSH_BUG_PKAUTH)
		buf.AppendToBuffer((BYTE) (pUser->bSecondary ? 1 : 0));   // bHaveSig
	else
	{
		buf.AppendToBufferWithLenCE("publickey");
		buf.AppendToBuffer((BYTE) (pUser->bSecondary ? 1 : 0));   // bHaveSig
		buf.AppendToBufferWithLenCE(lpszKeyType, dwKeyTypeLen);
	}
	buf.AppendToBufferWithLenCE(pBlob, nBlobLen);

	if (pUser->bSecondary)
	{
		void* pSignMsg = malloc(buf.GetLength() + 4);
		*((DWORD*) pSignMsg) = ConvertEndian((DWORD) buf.GetLength());
		memcpy(((char*) pSignMsg) + 4, buf, buf.GetLength());
		DWORD nSignedLen;
		void* pSignedData = PuTTYSignSSH2Key(m_lpCurrentKey, (LPCBYTE) pSignMsg, &nSignedLen);
		_SecureStringW::SecureEmptyBuffer(pSignMsg, buf.GetLength() + 4);
		free(pSignMsg);
		if (!pSignedData)
		{
			_SecureStringW::SecureEmptyBuffer(buf, buf.GetLength());
			return false;
		}
		buf.Empty();
		nSkipLen = 0;
		buf.AppendToBufferWithLenCE(lpszUser);
		buf.AppendToBufferWithLenCE((dwCompat & SSH_BUG_PKSERVICE) ? "ssh-userauth" : lpszService);
		if (dwCompat & SSH_BUG_PKAUTH)
			buf.AppendToBuffer((BYTE) 1);   // bHaveSig
		else
		{
			buf.AppendToBufferWithLenCE("publickey");
			buf.AppendToBuffer((BYTE) 1);   // bHaveSig
			buf.AppendToBufferWithLenCE(lpszKeyType, dwKeyTypeLen);
		}
		buf.AppendToBufferWithLenCE(pBlob, nBlobLen);
		DWORD dw = ConvertEndian(*((DWORD*) pSignedData));
		buf.AppendToBufferWithLenCE(((LPBYTE) pSignedData) + 4, dw);
		_SecureStringW::SecureEmptyBuffer(pSignedData, nSignedLen);
		free(pSignedData);
	}
	//if (!SignSSH2Key(dwCompat, pUser->keyData, pUser->keyType, buf, buf.GetLength(), &pSign, &nSignLen))
	//{
	//	return false;
	//}

	//if (dwCompat & SSH_BUG_PKSERVICE)
	//{
	//	buf.Empty();
	//	buf.AppendToBuffer(pUser->pvSessionID, pUser->nSessionIDLen);
	//	nSkipLen = pUser->nSessionIDLen;
	//	buf.AppendToBuffer((BYTE) SSH2_MSG_USERAUTH_REQUEST);
	//	buf.AppendToBufferWithLenCE(lpszUser);
	//	buf.AppendToBufferWithLenCE((dwCompat & SSH_BUG_PKSERVICE) ? "ssh-userauth" : lpszService);
	//	buf.AppendToBufferWithLenCE("publickey");
	//	buf.AppendToBuffer((BYTE) 1);   // bHaveSig
	//	if (!(dwCompat & SSH_BUG_PKAUTH))
	//		buf.AppendToBufferWithLenCE(lpszKeyType);
	//	buf.AppendToBufferWithLenCE(pBlob, nBlobLen);
	//}
	//_SecureStringW::SecureEmptyBuffer(pBlob, nBlobLen);
	//free(pBlob);

	//buf.AppendToBufferWithLenCE(pSign, nSignLen);
	//_SecureStringW::SecureEmptyBuffer(pSign, nSignLen);
	//free(pSign);

	register bool ret = false;
	if (buf.GetLength() >= nSkipLen)
	{
		if (nSkipLen)
			buf.SkipPosition((long) (nSkipLen));
		ret = pSocket->SendPacket(SSH2_MSG_USERAUTH_REQUEST, buf, buf.GetLength());
		buf.ResetPosition();
	}
	_SecureStringW::SecureEmptyBuffer(buf, buf.GetLength());

	return ret;
}

void CPageantAuthentication::FinishAndDelete()
{
	if (m_lpPageantKeyList)
		PuTTYFreeKeyList(m_lpPageantKeyList);
	delete this;
}

bool CPageantAuthentication::CanRetry()
{
	if (!m_lpPageantKeyList)
		return false;
	m_dwKeyIndex++;
	if (m_dwKeyIndex >= m_dwKeyCount)
	{
		PuTTYFreeKeyList(m_lpPageantKeyList);
		m_lpPageantKeyList = NULL;
		m_dwKeyCount = 0;
		return false;
	}

	DWORD dw = ConvertEndian(*((DWORD*) m_lpCurrentKey));
	m_lpCurrentKey += dw + 4;
	dw = ConvertEndian(*((DWORD*) m_lpCurrentKey));
	m_lpCurrentKey += dw + 4;
	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool CNoneAuthentication::Authenticate(CSSH2Socket* pSocket, CUserInfo* pUser, LPCSTR lpszService)
{
	LPCSTR lpszUser;

	lpszUser = (LPCSTR) pUser->strName.AllocUTF8String();

	CExBuffer buf;
	buf.AppendToBufferWithLenCE(lpszUser ? lpszUser : "");
	buf.AppendToBufferWithLenCE(lpszService);
	buf.AppendToBufferWithLenCE("none");
	return pSocket->SendPacket(SSH2_MSG_USERAUTH_REQUEST, buf, buf.GetLength());
}
