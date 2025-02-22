/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 FTPConn.h - declarations of FTP connection (command exchange, synchronization, and so on)
 */

#pragma once

#include "MsgData.h"
#include "FTPMsg.h"
#include "FTPSock.h"

#define SVT_UNKNOWN           0
#define SVT_UNIX              1
#define SVT_DOS               2
#define SVT_WINDOWS           3

struct CFTPWaitResponse
{
	CMyStringW strCommand;
	CMyStringW strParameter;
	CWaitResponseData* pWait;
	bool bIsWaitingIgnorablePassiveDone;

	CFTPWaitResponse() : pWait(NULL), bIsWaitingIgnorablePassiveDone(false) {}
};

struct CFTPWaitEstablishPassive : public CWaitResponseData
{
	inline CFTPWaitEstablishPassive(CFTPPassiveMessage* pMessage)
		: CWaitResponseData(WRD_PASSIVE)
		, pMessage(pMessage)
		, pRet(NULL)
		{ pMessage->AddRef(); }
	~CFTPWaitEstablishPassive()
	{
		pMessage->Release();
	}
	CFTPPassiveMessage* pMessage;

	// out
	CFTPWaitPassive* pRet;
};

//struct CFTPWaitListingData : public CFTPWaitPassive
//{
//	inline CFTPWaitListingData() : CFTPWaitPassive(WRD_DIRECTORY) { }
//	// additional flags for CFTPWaitPassive::nWaitFlags
//	enum
//	{
//		flagWaitingForFileListings = 3
//	};
//};

struct CFTPWaitAttrData : public CWaitResponseData
{
	inline CFTPWaitAttrData() : CWaitResponseData(WRD_FTPWAITATTR) { }
	CMyStringW strFileName;
	bool bIsDir;
	CMyStringW strResult;
};

struct CFTPWaitConfirm : public CWaitResponseData
{
	inline CFTPWaitConfirm(int nWaitType) : CWaitResponseData(nWaitType) { }
	inline CFTPWaitConfirm() : CWaitResponseData(WRD_CONFIRM) { }
	int nCode;
	CMyStringW strMsg;
};

struct CWaitRenameData : public CFTPWaitConfirm
{
	inline CWaitRenameData() : CFTPWaitConfirm(WRD_RENAME) { }
	//int iItem;
	bool bSecondary;
};

struct CWaitMakeDirData : public CWaitResponseData
{
	inline CWaitMakeDirData() : CWaitResponseData(WRD_MAKEDIR) { }
	CMyStringW strLocalDirectory;
	CMyStringW strRemoteDirectory;
	enum
	{
		// waiting for response of MKD and send child files
		flagWaitingForMKDSend = 3,
		// waiting for response of MKD but not send child files
		// (strLocalDirectory may be empty, and cannot be used)
		// (pvData is an boolean value whether CServerFileView::StartCreateNewFolder() was used)
		flagWaitingForMKDNoSend = 4,
		// waiting for error response of STOR
		// (pvData is an instance of CFTPFileSendMessage)
		flagWaitingForSTOR = 5,
		// waiting for error response of RETR
		// (pvData is an instance of CFTPFileSendMessage)
		flagWaitingForRETR = 6,
		flagWaitingForPassive = 7,
		// waiting for response of CWD (ValidateDirectory)
		flagWaitingForRealPath1 = 8,
		// waiting for response of PWD (ValidateDirectory)
		flagWaitingForRealPath2 = 9,
		flagFinished = 0,
		flagError = -1
	};
	char nWaitFlags;
	union
	{
		// extra data (e.g. CFTPPassiveMessage instance, CFTPFileListingHandler instance, ...)
		void* pvData;
		CFTPWaitEstablishPassive* pWait;
		bool bRPError;
	};
	//CSyncSendRecvFile* pSendFile;
};

struct CWaitFeatureData : public CWaitResponseData
{
	inline CWaitFeatureData() : CWaitResponseData(WRD_GETFEATURE) { }
};

struct CWaitFileInfoData : public CWaitResponseData
{
	inline CWaitFileInfoData(char nInfoType)
		: CWaitResponseData(WRD_GETFILEINFO)
		, nInfoType(nInfoType)
		{ }
	enum
	{
		fileInfoSize = 1,
		fileInfoMDTM = 2
	};
	bool bSucceeded;
	char nInfoType;
	CMyStringW strFileName;
	ULARGE_INTEGER uliSize;
	FILETIME ftModifiedTime;
};

////////////////////////////////////////////////////////////////////////////////

class CFTPConnection : public CReferenceCountClassBase
{
public:
	CFTPConnection(void);
	~CFTPConnection(void);

	bool Connect(int nPort, LPCWSTR lpszHostName);
	void Close();
	CFTPWaitResponse* SendCommand(LPCWSTR lpszCommand, LPCWSTR lpszParam,
		CWaitResponseData* pWait = NULL);
	CFTPWaitResponse* SecureSendCommand(LPCWSTR lpszCommand, const _SecureStringW& strParam,
		CWaitResponseData* pWait = NULL);
	CFTPWaitResponse* SendCommandWithType(LPCWSTR lpszCommand, LPCWSTR lpszParam, LPCWSTR lpszType,
		CWaitResponseData* pWait = NULL);
	CFTPWaitResponse* SendDoubleCommand(LPCWSTR lpszCommand1, LPCWSTR lpszParam1,
		LPCWSTR lpszCommand2, LPCWSTR lpszParam2,
		CWaitResponseData* pWait1 = NULL, CWaitResponseData* pWait2 = NULL);
	CFTPWaitResponse* SendTripleCommand(LPCWSTR lpszCommand1, LPCWSTR lpszParam1,
		LPCWSTR lpszCommand2, LPCWSTR lpszParam2,
		LPCWSTR lpszCommand3, LPCWSTR lpszParam3,
		CWaitResponseData* pWait1 = NULL, CWaitResponseData* pWait2 = NULL, CWaitResponseData* pWait3 = NULL);
	bool ReceiveMessage(int& nCode, CMyStringW& rstrMessage, CWaitResponseData** ppWait,
		CMyStringW* pstrCommand = NULL);
	bool ReceivePassive(CFTPWaitPassive* pPassive);
	void WaitFinishPassive(CFTPWaitPassiveDone* pPassive);
	void MarkPassiveDoneIgnorable(CFTPWaitPassiveDone* pPassive);
	void ReplaceFinishPassive(CFTPWaitPassive* pPassive, CWaitResponseData* pWait);

	void InitAvaliableCommands(LPCWSTR lpszParam);
	LPCWSTR IsCommandAvailable(LPCWSTR lpszCommand) const;

	enum class FTPSHandshakeResult : BYTE
	{
		NotApplicable = 0,
		InProgress = 1,
		Success = 2,
		Failure = 3
	};

	void StartFTPSHandshake();
	FTPSHandshakeResult OnFirstFTPSHandshake(int code);
	FTPSHandshakeResult ProcessFTPSHandshake();

	CFTPSocket m_socket;
	CRITICAL_SECTION m_csSocket;
	CMyPtrArrayT<CFTPWaitResponse> m_aWaitResponse;

protected:
	enum class FTPSConnectionPhase : BYTE
	{
		None = 0,
		FirstReceive = 1,
		Handshake = 2,
	};

	LPWSTR m_lpszAvailableCommands;
	FTPSConnectionPhase m_FTPSConnectionPhase;
};
