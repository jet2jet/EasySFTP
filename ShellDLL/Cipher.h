/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 Cipher.h - declarations of cipher functions and classes for SSH
 */

#pragma once

////////////////////////////////////////////////////////////////////////////////

#define COMP_NONE        0
#define COMP_ZLIB        1
#define COMP_DELAYED     2

struct CCipher;

struct CEncryption
{
	LPCSTR lpszName;
	bool bEnabled;
	CCipher* pCipher;
	size_t nKeyLen;
	size_t nBlockSize;
	void* pKeyData;
	void* pIV;
};

struct CMacData
{
	LPCSTR lpszName;
	bool bEnabled;
	int nType;
	size_t nMacLen;
	void* pKeyData;
	size_t nKeyLen;
	const EVP_MD* pEVPMD;
	HMAC_CTX evpCtx;
	//umac_ctx
};

struct CCompressionData
{
	LPCSTR lpszName;
	bool bEnabled;
	int nType;
};

extern "C" void __stdcall MacSetupById(CMacData* pMac, int nID);
extern "C" bool __stdcall MacSetup(CMacData* pMac, LPCSTR lpszName);
extern "C" bool __stdcall MacInit(CMacData* pMac);
extern "C" void* __stdcall MacCompute(CMacData* pMac, UINT uSeqNum, const void* pData, size_t nLen);
extern "C" void __stdcall MacCleanup(CMacData* pMac);

////////////////////////////////////////////////////////////////////////////////

/*
 * CCipher types for SSH-1.  New types can be added, but old types should not
 * be removed for compatibility.  The maximum allowed value is 31.
 */
#define SSH_CIPHER_SSH2		-3
#define SSH_CIPHER_INVALID	-2	/* No valid CCipher selected. */
#define SSH_CIPHER_NOT_SET	-1	/* None selected (invalid number). */
#define SSH_CIPHER_NONE		0	/* no encryption */
#define SSH_CIPHER_IDEA		1	/* IDEA CFB */
#define SSH_CIPHER_DES		2	/* DES CBC */
#define SSH_CIPHER_3DES		3	/* 3DES CBC */
#define SSH_CIPHER_BROKEN_TSS	4	/* TRI's Simple Stream encryption CBC */
#define SSH_CIPHER_BROKEN_RC4	5	/* Alleged RC4 */
#define SSH_CIPHER_BLOWFISH	6
#define SSH_CIPHER_RESERVED	7
#define SSH_CIPHER_MAX		31

struct CCipher
{
	LPCSTR lpszName;
	int nType;		/* for ssh1 only */
	size_t nBlockSize;
	size_t nKeyLen;
	size_t nDiscardLen;
	UINT nCBCMode;
	const EVP_CIPHER* (* GetEvpType)(void);
};

struct CNewKeyData
{
	CEncryption enc;
	CMacData mac;
	CCompressionData comp;
};

struct CCipherContext
{
	CCipher* pCipher;
	EVP_CIPHER_CTX evp;
};

extern "C" CCipher* __stdcall GetCipherByName(LPCSTR lpszName);
extern "C" bool __stdcall InitCipherContext(CCipherContext* pContext, CCipher* pCipher,
	const void* pKey, size_t nKeyLen, const void* pIV, size_t nIVLen, bool bEncrypt);
extern "C" void __stdcall CleanupCipherContext(CCipherContext* pContext);
extern "C" bool __stdcall CryptWithCipherContext(CCipherContext* pContext, void* pvDest, const void* pvSrc, size_t nSize);

////////////////////////////////////////////////////////////////////////////////
