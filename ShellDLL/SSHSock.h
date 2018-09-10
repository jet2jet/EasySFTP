/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 SSHSock.h - declarations of CSSH2Socket and definitions for SSH(2)
 */

#pragma once

#include "MySocket.h"
#include "ExBuffer.h"

// server specs
#define SSH_BUG_SIGBLOB     0x00000001
#define SSH_BUG_PKSERVICE   0x00000002
#define SSH_BUG_HMAC        0x00000004
#define SSH_BUG_X11FWD      0x00000008
#define SSH_OLD_SESSIONID   0x00000010
#define SSH_BUG_PKAUTH      0x00000020
#define SSH_BUG_DEBUG       0x00000040
#define SSH_BUG_BANNER      0x00000080
#define SSH_BUG_IGNOREMSG   0x00000100
#define SSH_BUG_PKOK        0x00000200
#define SSH_BUG_PASSWORDPAD 0x00000400
#define SSH_BUG_SCANNER     0x00000800
#define SSH_BUG_BIGENDIANAES 0x00001000
#define SSH_BUG_RSASIGMD5   0x00002000
#define SSH_OLD_DHGEX       0x00004000
#define SSH_BUG_NOREKEY     0x00008000
#define SSH_BUG_HBSERVICE   0x00010000
#define SSH_BUG_OPENFAILURE 0x00020000
#define SSH_BUG_DERIVEKEY   0x00040000
#define SSH_BUG_DUMMYCHAN   0x00100000
#define SSH_BUG_EXTEOF      0x00200000
#define SSH_BUG_PROBE       0x00400000
#define SSH_BUG_FIRSTKEX    0x00800000
#define SSH_OLD_FORWARD_ADDR   0x01000000
#define SSH_BUG_RFWD_ADDR   0x02000000
#define SSH_NEW_OPENSSH     0x04000000
// for SFTP SYMLINK problem
#define SSH_BUG_SYMLINK     0x08000000

// for SSH1

/* Ranges */
#define SSH_MSG_MIN				1
#define SSH_MSG_MAX				254
/* Message name */			/* msg code */	/* arguments */
#define SSH_MSG_NONE				0	/* no message */
#define SSH_MSG_DISCONNECT			1	/* cause (string) */
#define SSH_SMSG_PUBLIC_KEY			2	/* ck,msk,srvk,hostk */
#define SSH_CMSG_SESSION_KEY			3	/* key (BIGNUM) */
#define SSH_CMSG_USER				4	/* user (string) */
#define SSH_CMSG_AUTH_RHOSTS			5	/* user (string) */
#define SSH_CMSG_AUTH_RSA			6	/* modulus (BIGNUM) */
#define SSH_SMSG_AUTH_RSA_CHALLENGE		7	/* int (BIGNUM) */
#define SSH_CMSG_AUTH_RSA_RESPONSE		8	/* int (BIGNUM) */
#define SSH_CMSG_AUTH_PASSWORD			9	/* pass (string) */
#define SSH_CMSG_REQUEST_PTY			10	/* TERM, tty modes */
#define SSH_CMSG_WINDOW_SIZE			11	/* row,col,xpix,ypix */
#define SSH_CMSG_EXEC_SHELL			12	/* */
#define SSH_CMSG_EXEC_CMD			13	/* cmd (string) */
#define SSH_SMSG_SUCCESS			14	/* */
#define SSH_SMSG_FAILURE			15	/* */
#define SSH_CMSG_STDIN_DATA			16	/* data (string) */
#define SSH_SMSG_STDOUT_DATA			17	/* data (string) */
#define SSH_SMSG_STDERR_DATA			18	/* data (string) */
#define SSH_CMSG_EOF				19	/* */
#define SSH_SMSG_EXITSTATUS			20	/* status (int) */
#define SSH_MSG_CHANNEL_OPEN_CONFIRMATION	21	/* channel (int) */
#define SSH_MSG_CHANNEL_OPEN_FAILURE		22	/* channel (int) */
#define SSH_MSG_CHANNEL_DATA			23	/* ch,data (int,str) */
#define SSH_MSG_CHANNEL_CLOSE			24	/* channel (int) */
#define SSH_MSG_CHANNEL_CLOSE_CONFIRMATION	25	/* channel (int) */
/*      SSH_CMSG_X11_REQUEST_FORWARDING		26	   OBSOLETE */
#define SSH_SMSG_X11_OPEN			27	/* channel (int) */
#define SSH_CMSG_PORT_FORWARD_REQUEST		28	/* p,host,hp (i,s,i) */
#define SSH_MSG_PORT_OPEN			29	/* ch,h,p (i,s,i) */
#define SSH_CMSG_AGENT_REQUEST_FORWARDING	30	/* */
#define SSH_SMSG_AGENT_OPEN			31	/* port (int) */
#define SSH_MSG_IGNORE				32	/* string */
#define SSH_CMSG_EXIT_CONFIRMATION		33	/* */
#define SSH_CMSG_X11_REQUEST_FORWARDING		34	/* proto,data (s,s) */
#define SSH_CMSG_AUTH_RHOSTS_RSA		35	/* user,mod (s,mpi) */
#define SSH_MSG_DEBUG				36	/* string */
#define SSH_CMSG_REQUEST_COMPRESSION		37	/* level 1-9 (int) */
#define SSH_CMSG_MAX_PACKET_SIZE		38	/* size 4k-1024k (int) */
#define SSH_CMSG_AUTH_TIS			39	/* we use this for s/key */
#define SSH_SMSG_AUTH_TIS_CHALLENGE		40	/* challenge (string) */
#define SSH_CMSG_AUTH_TIS_RESPONSE		41	/* response (string) */
#define SSH_CMSG_AUTH_KERBEROS			42	/* (KTEXT) */
#define SSH_SMSG_AUTH_KERBEROS_RESPONSE		43	/* (KTEXT) */
#define SSH_CMSG_HAVE_KERBEROS_TGT		44	/* credentials (s) */
#define SSH_CMSG_HAVE_AFS_TOKEN			65	/* token (s) */

/* protocol version 1.5 overloads some version 1.3 message types */
#define SSH_MSG_CHANNEL_INPUT_EOF	SSH_MSG_CHANNEL_CLOSE
#define SSH_MSG_CHANNEL_OUTPUT_CLOSE	SSH_MSG_CHANNEL_CLOSE_CONFIRMATION

/*
 * Authentication methods.  New types can be added, but old types should not
 * be removed for compatibility.  The maximum allowed value is 31.
 */
#define SSH_AUTH_RHOSTS		1
#define SSH_AUTH_RSA		2
#define SSH_AUTH_PASSWORD	3
#define SSH_AUTH_RHOSTS_RSA	4
#define SSH_AUTH_TIS		5
#define SSH_AUTH_KERBEROS	6
#define SSH_PASS_KERBEROS_TGT	7
				/* 8 to 15 are reserved */
#define SSH_PASS_AFS_TOKEN	21

/* Protocol flags.  These are bit masks. */
#define SSH_PROTOFLAG_SCREEN_NUMBER	1	/* X11 forwarding includes screen */
#define SSH_PROTOFLAG_HOST_IN_FWD_OPEN	2	/* forwarding opens contain host */

// for SSH2

/* ranges */

#define SSH2_MSG_TRANSPORT_MIN              1
#define SSH2_MSG_TRANSPORT_MAX              49
#define SSH2_MSG_USERAUTH_MIN               50
#define SSH2_MSG_USERAUTH_MAX               79
#define SSH2_MSG_USERAUTH_PER_METHOD_MIN    60
#define SSH2_MSG_USERAUTH_PER_METHOD_MAX    SSH2_MSG_USERAUTH_MAX
#define SSH2_MSG_CONNECTION_MIN             80
#define SSH2_MSG_CONNECTION_MAX             127
#define SSH2_MSG_RESERVED_MIN               128
#define SSH2_MSG_RESERVED_MAX               191
#define SSH2_MSG_LOCAL_MIN                  192
#define SSH2_MSG_LOCAL_MAX                  255
#define SSH2_MSG_MIN                        1
#define SSH2_MSG_MAX                        255

/* transport layer: generic */

#define SSH2_MSG_DISCONNECT                 1
#define SSH2_MSG_IGNORE                     2
#define SSH2_MSG_UNIMPLEMENTED              3
#define SSH2_MSG_DEBUG                      4
#define SSH2_MSG_SERVICE_REQUEST            5
#define SSH2_MSG_SERVICE_ACCEPT             6

/* transport layer: alg negotiation */

#define SSH2_MSG_KEXINIT                    20
#define SSH2_MSG_NEWKEYS                    21

/* transport layer: kex specific messages, can be reused */

#define SSH2_MSG_KEXDH_INIT                 30
#define SSH2_MSG_KEXDH_REPLY                31

/* dh-group-exchange */
#define SSH2_MSG_KEX_DH_GEX_REQUEST_OLD     30
#define SSH2_MSG_KEX_DH_GEX_GROUP           31
#define SSH2_MSG_KEX_DH_GEX_INIT            32
#define SSH2_MSG_KEX_DH_GEX_REPLY           33
#define SSH2_MSG_KEX_DH_GEX_REQUEST         34

/* user authentication: generic */

#define SSH2_MSG_USERAUTH_REQUEST           50
#define SSH2_MSG_USERAUTH_FAILURE           51
#define SSH2_MSG_USERAUTH_SUCCESS           52
#define SSH2_MSG_USERAUTH_BANNER            53

/* user authentication: method specific, can be reused */

#define SSH2_MSG_USERAUTH_PK_OK             60
#define SSH2_MSG_USERAUTH_PASSWD_CHANGEREQ  60
#define SSH2_MSG_USERAUTH_INFO_REQUEST      60
#define SSH2_MSG_USERAUTH_INFO_RESPONSE     61
#define SSH2_MSG_USERAUTH_JPAKE_CLIENT_STEP1    60
#define SSH2_MSG_USERAUTH_JPAKE_SERVER_STEP1    61
#define SSH2_MSG_USERAUTH_JPAKE_CLIENT_STEP2    62
#define SSH2_MSG_USERAUTH_JPAKE_SERVER_STEP2    63
#define SSH2_MSG_USERAUTH_JPAKE_CLIENT_CONFIRM  64
#define SSH2_MSG_USERAUTH_JPAKE_SERVER_CONFIRM  65

/* connection protocol: generic */

#define SSH2_MSG_GLOBAL_REQUEST             80
#define SSH2_MSG_REQUEST_SUCCESS            81
#define SSH2_MSG_REQUEST_FAILURE            82

/* channel related messages */

#define SSH2_MSG_CHANNEL_OPEN               90
#define SSH2_MSG_CHANNEL_OPEN_CONFIRMATION  91
#define SSH2_MSG_CHANNEL_OPEN_FAILURE       92
#define SSH2_MSG_CHANNEL_WINDOW_ADJUST      93
#define SSH2_MSG_CHANNEL_DATA               94
#define SSH2_MSG_CHANNEL_EXTENDED_DATA      95
#define SSH2_MSG_CHANNEL_EOF                96
#define SSH2_MSG_CHANNEL_CLOSE              97
#define SSH2_MSG_CHANNEL_REQUEST            98
#define SSH2_MSG_CHANNEL_SUCCESS            99
#define SSH2_MSG_CHANNEL_FAILURE            100

/* disconnect reason code */

#define SSH2_DISCONNECT_HOST_NOT_ALLOWED_TO_CONNECT  1
#define SSH2_DISCONNECT_PROTOCOL_ERROR               2
#define SSH2_DISCONNECT_KEY_EXCHANGE_FAILED          3
#define SSH2_DISCONNECT_HOST_AUTHENTICATION_FAILED   4
#define SSH2_DISCONNECT_RESERVED                     4
#define SSH2_DISCONNECT_MAC_ERROR                    5
#define SSH2_DISCONNECT_COMPRESSION_ERROR            6
#define SSH2_DISCONNECT_SERVICE_NOT_AVAILABLE        7
#define SSH2_DISCONNECT_PROTOCOL_VERSION_NOT_SUPPORTED  8
#define SSH2_DISCONNECT_HOST_KEY_NOT_VERIFIABLE      9
#define SSH2_DISCONNECT_CONNECTION_LOST              10
#define SSH2_DISCONNECT_BY_APPLICATION               11
#define SSH2_DISCONNECT_TOO_MANY_CONNECTIONS         12
#define SSH2_DISCONNECT_AUTH_CANCELLED_BY_USER       13
#define SSH2_DISCONNECT_NO_MORE_AUTH_METHODS_AVAILABLE  14
#define SSH2_DISCONNECT_ILLEGAL_USER_NAME            15

/* misc */

#define SSH2_OPEN_ADMINISTRATIVELY_PROHIBITED  1
#define SSH2_OPEN_CONNECT_FAILED            2
#define SSH2_OPEN_UNKNOWN_CHANNEL_TYPE      3
#define SSH2_OPEN_RESOURCE_SHORTAGE         4

#define SSH2_EXTENDED_DATA_STDERR           1

#include "Cipher.h"


class CSSH2Socket : public CTextSocket
{
public:
	CSSH2Socket();
	virtual ~CSSH2Socket();

	void InitServerVersion(LPCSTR lpszServerVersion);
	bool SetPrivateKey(EVP_PKEY* privKey, bool bReleaseMyself = true);
	DWORD GetServerCompatible() const { return m_dwServerFlags; }
	bool CheckServerCompatible(DWORD dwCompatFlags);
	void UpdateServerKeyData(bool bForSend, CNewKeyData& keyData);
	void ResetServerKeyData();

	void Close(LPCSTR lpszMessage = NULL);

public:
	bool SendPacket(BYTE bType, const void* pBuffer, size_t nLen, size_t nExtraPad = 0);
	bool SendPacketString(BYTE bType, const char* pszString, size_t nExtraPad = 0);
	void* ReceivePacket(BYTE& bType, size_t& nLen);
	bool HasReceivedData() const;

private:
	DWORD m_dwServerFlags;
	EVP_PKEY* m_privKey;
	bool m_bReleaseMyself;
	void* m_pKeyDataSend;
	void* m_pKeyDataRecv;
	UINT m_nSendSequenceNumber;
	UINT m_nRecvSequenceNumber;

	CRITICAL_SECTION m_csRecvBuffer;
	//CExBuffer m_bufferSend;
	CExBuffer m_bufferReceived;
	CExBuffer m_bufferPacket;
	size_t m_nLastPacketSize;

//#ifdef _DEBUG
//	CExBuffer m_bufferLastSent;
//	CExBuffer m_bufferEncLastSent;
//#endif

	bool ReceiveAllData();
	bool _ReceivePacket(void*& pvRet, BYTE& bType, size_t& nLen);

	void CryptSend(void* pDest, const void* pSrc, size_t nLen);
	void CryptRecv(void* pDest, const void* pSrc, size_t nLen);
};

inline bool CSSH2Socket::HasReceivedData() const
	{ return !m_bufferReceived.IsEmpty(); }
