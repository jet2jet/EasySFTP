/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 KexCore.h - declarations for key exchanging
 */

#pragma once

#include "ExBuffer.h"

/* kex */

#define	KEX_DH1          "diffie-hellman-group1-sha1"
#define	KEX_DH14         "diffie-hellman-group14-sha1"
#define	KEX_DHGEX_SHA1   "diffie-hellman-group-exchange-sha1"
#define	KEX_DHGEX_SHA256 "diffie-hellman-group-exchange-sha256"

#define KEX_COOKIE_LEN	16

enum kex_init_proposals {
	PROPOSAL_KEX_ALGS = 0,
	PROPOSAL_SERVER_HOST_KEY_ALGS,
	PROPOSAL_ENC_ALGS_CTOS,
	PROPOSAL_ENC_ALGS_STOC,
	PROPOSAL_MAC_ALGS_CTOS,
	PROPOSAL_MAC_ALGS_STOC,
	PROPOSAL_COMP_ALGS_CTOS,
	PROPOSAL_COMP_ALGS_STOC,
	PROPOSAL_LANG_CTOS,
	PROPOSAL_LANG_STOC,
	PROPOSAL_MAX
};

/* Old OpenSSL doesn't support what we need for DHGEX-sha256 */
#if OPENSSL_VERSION_NUMBER < 0x00907000L
# define KEX_DEFAULT_KEX		\
	"diffie-hellman-group-exchange-sha1," \
	"diffie-hellman-group14-sha1," \
	"diffie-hellman-group1-sha1"
#else
# define KEX_DEFAULT_KEX		\
	"diffie-hellman-group-exchange-sha256," \
	"diffie-hellman-group-exchange-sha1," \
	"diffie-hellman-group14-sha1," \
	"diffie-hellman-group1-sha1"
//# define KEX_DEFAULT_KEX		\
//	"diffie-hellman-group14-sha1," \
//	"diffie-hellman-group1-sha1"
#endif

#define	KEX_DEFAULT_PK_ALG	"ssh-rsa,ssh-dss"
//#define	KEX_DEFAULT_PK_ALG	"ssh-dss,ssh-rsa"

#define	KEX_DEFAULT_ENCRYPT \
	/*"aes128-ctr,aes192-ctr,aes256-ctr,"*/ \
	"arcfour256,arcfour128," \
	"aes128-cbc,3des-cbc,blowfish-cbc,cast128-cbc," \
	"aes192-cbc,aes256-cbc,arcfour,rijndael-cbc@lysator.liu.se"
//#define	KEX_DEFAULT_MAC \
//	"hmac-md5,hmac-sha1,umac-64@openssh.com,hmac-ripemd160," \
//	"hmac-ripemd160@openssh.com," \
//	"hmac-sha1-96,hmac-md5-96"
#define	KEX_DEFAULT_MAC \
	"hmac-md5,hmac-sha1,hmac-ripemd160," \
	"hmac-ripemd160@openssh.com," \
	"hmac-sha1-96,hmac-md5-96"
//#define	KEX_DEFAULT_COMP	"none,zlib@openssh.com,zlib"
#define	KEX_DEFAULT_COMP	"none"
#define	KEX_DEFAULT_LANG	""

enum KeyType
{
	KEY_RSA1 = 0,
	KEY_RSA,
	KEY_DSA,
	KEY_UNSPEC
};

union KeyData
{
	void* pKeyUnknown;
	RSA* pRSA;
	DSA* pDSA;

	inline KeyData() : pKeyUnknown(NULL) { }
	inline KeyData(void* pv) : pKeyUnknown(pv) { }
	inline operator void* () const { return pKeyUnknown; }
	inline operator RSA* () const { return pRSA; }
	inline operator DSA* () const { return pDSA; }
	inline operator bool () const { return pKeyUnknown != NULL; }
	inline bool operator ! () const { return pKeyUnknown == NULL; }
	inline KeyData& operator = (const KeyData& key) { pKeyUnknown = key.pKeyUnknown; return *this; }
	inline KeyData& operator = (void* p) { pKeyUnknown = p; return *this; }
	inline KeyData& operator = (RSA* r) { pRSA = r; return *this; }
	inline KeyData& operator = (DSA* d) { pDSA = d; return *this; }
	inline RSA* rsa() const { return pRSA; }
	inline DSA* dsa() const { return pDSA; }
};

#define DH_GRP_MIN  1024
#define DH_GRP_MAX  8192

#define INTBLOB_LEN	20
#define SIGBLOB_LEN	(2*INTBLOB_LEN)

extern "C" LPCSTR __stdcall KeyTypeToName(KeyType keyType);
extern "C" KeyType __stdcall KeyTypeFromName(LPCSTR pszName);

extern "C" bool __stdcall CreateBlobFromKey(KeyType keyType, const KeyData* pKeyData, void** ppvData, size_t* pnLen);

class CSSH2Socket;

extern "C" bool __stdcall AppendBigNumToBuffer(CExBuffer* pBuffer, const BIGNUM* pbnum);
extern "C" bool __stdcall GetBigNumFromBuffer(CExBuffer* pBuffer, BIGNUM* pbnum);

////////////////////////////////////////////////////////////////////////////////

class __declspec(novtable) CFingerPrintHandler
{
public:
	virtual bool __stdcall CheckFingerPrint(const BYTE* pFingerPrint, size_t nLen) = 0;
};

#include "UString.h"

class CKeyExchangeClient;

#define MAX_DERIVE_KEYS  6

class CKeyExchange
{
private:
	CKeyExchange();
public:
	~CKeyExchange();

	static CKeyExchange* __stdcall InitKeyExchange(
		LPCWSTR lpszClientVersion,
		LPCWSTR lpszServerVersion,
		LPCSTR lpszFoundKexAlgs,
		LPCSTR lpszFoundHostKeyAlgs,
		size_t nNeedKeyBytes,
		const CExBuffer& myProposals, const CExBuffer& svProposals,
		CSSH2Socket* pSocket);

	// <0: error, =0: try again, >0: succeeded
	int OnReceiveKexMessages(CFingerPrintHandler* pHandler, CSSH2Socket* pSocket, BYTE bType, const void* pvData, size_t nLen);

	void DoDeriveKeys(DWORD dwCompatFlags, const void* pHash, size_t nHashLen, const BIGNUM* pShared);

public:
	CKeyExchangeClient* m_pKexClient;
	const EVP_MD* m_pEVPMD;
	size_t m_nNeedKeyBytes;
	UINT m_nKeyType;
	CMyStringW m_strClientVersion, m_strServerVersion;
	CExBuffer m_bufferMyProposals, m_bufferServerProposals;

	void* m_pvSessionID;
	size_t m_nSessionIDLen;

	void* m_pvDeriveKeys[MAX_DERIVE_KEYS];

private:
	void* GetDerivedKey(char id, const void* pHash, size_t nHashLen, const BIGNUM* pShared);
};

class CKeyExchangeClient
{
public:
	virtual ~CKeyExchangeClient() { }
	virtual bool Init(CKeyExchange* pKex, CSSH2Socket* pSocket) = 0;
	virtual int OnReceiveKexMessages(CFingerPrintHandler* pHandler, CKeyExchange* pKex, CSSH2Socket* pSocket, BYTE bType, const void* pvData, size_t nLen) = 0;
	virtual LPCSTR GetKexTypeName() = 0;
};

class CDHKeyExchangeClient : public CKeyExchangeClient
{
public:
	CDHKeyExchangeClient(bool bGrp14);
	virtual ~CDHKeyExchangeClient();

	virtual bool Init(CKeyExchange* pKex, CSSH2Socket* pSocket);
	virtual int OnReceiveKexMessages(CFingerPrintHandler* pHandler, CKeyExchange* pKex, CSSH2Socket* pSocket, BYTE bType, const void* pvData, size_t nLen);
	virtual LPCSTR GetKexTypeName() { return m_bGrp14 ? KEX_DH14 : KEX_DH1; }

private:
	static void __stdcall Hash(LPCSTR pszClientVersion, LPCSTR pszServerVersion,
		const void* pCKexInit, size_t nCKexInitLen,
		const void* pSKexInit, size_t nSKexInitLen,
		const void* pServerHostKeyBlob, size_t nServerHostKeyBlobLen,
		const BIGNUM* pClientDHPub, const BIGNUM* pServerDHPub, const BIGNUM* pShared,
		void** ppHash, size_t* pnHashLen);
	bool m_bGrp14;
	DH* m_pDH;
};

class CGEXKeyExchangeClient : public CKeyExchangeClient
{
public:
	CGEXKeyExchangeClient(bool bSha256);
	virtual ~CGEXKeyExchangeClient();

	virtual bool Init(CKeyExchange* pKex, CSSH2Socket* pSocket);
	virtual int OnReceiveKexMessages(CFingerPrintHandler* pHandler, CKeyExchange* pKex, CSSH2Socket* pSocket, BYTE bType, const void* pvData, size_t nLen);
	virtual LPCSTR GetKexTypeName() { return m_bSha256 ? KEX_DHGEX_SHA256 : KEX_DHGEX_SHA1; }

private:
	static void __stdcall Hash(const EVP_MD* pEVPMD,
		LPCSTR pszClientVersion, LPCSTR pszServerVersion,
		const void* pCKexInit, size_t nCKexInitLen,
		const void* pSKexInit, size_t nSKexInitLen,
		const void* pServerHostKeyBlob, size_t nServerHostKeyBlobLen,
		int nMin, int nWantBits, int nMax, const BIGNUM* pPrimeNum, BIGNUM* pGenNum,
		const BIGNUM* pClientDHPub, const BIGNUM* pServerDHPub, const BIGNUM* pShared,
		void** ppHash, size_t* pnHashLen);
	bool m_bSha256;
	DH* m_pDH;
	int m_nBits, m_nMin, m_nMax;
};

////////////////////////////////////////////////////////////////////////////////
