/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 FTPMsg.h - declarations of FTP passive message classes
 */

#pragma once

#include "FTPSock.h"
#include "MsgData.h"
#include "UString.h"
#include "SUString.h"
//#include "SFileVw.h"

struct CFTPWaitPassive;
class CFTPConnection;
class CSFTPFolderFTP;

class __declspec(novtable) CFTPMessageDispatcher
{
public:
	virtual void SendCommand(LPCWSTR lpszType, LPCWSTR lpszParam) = 0;
	virtual void SecureSendCommand(LPCWSTR lpszType, const _SecureStringW& strParam) = 0;
	virtual bool ReceiveMessage(CMyStringW& rstrMessage, int* pnCode) = 0;
};

class __declspec(novtable) CFTPPassiveMessage : public CReferenceCountClassBase
{
public:
	CFTPPassiveMessage() { m_bCanceled = false; }

	virtual bool SendPassive(CFTPConnection* pConnection, CFTPWaitPassive* pWait) = 0;
	// return true if succeeded
	// if return false, the implementation must delete pWait->pPassive
	virtual bool ConnectionEstablished(CFTPWaitPassive* pWait) = 0;
	// return false if no data has received
	virtual bool OnReceive(CFTPSocket* pPassive) = 0;
	// return false if finished
	virtual bool ReadyToWrite(CFTPSocket* pPassive) = 0;
	virtual void EndReceive(UINT* puStatusMsgID) = 0;

	bool m_bCanceled;
	bool m_bForWrite;
};

class __declspec(novtable) CFTPFileListingListener
{
public:
	virtual bool ReceiveFileListing(CFTPSocket* pPassive, bool bMListing) = 0;
	virtual void FinishFileListing() = 0;
};

struct CFTPWaitPassive : public CWaitResponseData
{
	enum class WaitFlags : char
	{
		WaitingForHandshake = 1,
		WaitingForEstablish = 2,
		ReceivingData = 3,
		ReceivingDataExternal = 4,
		SendingData = 5,
		WaitingForPassiveDone = 6,
		Finished = 0,
		Error = -1
	};

	inline CFTPWaitPassive(WaitFlags nWaitFlags,
		CFTPPassiveMessage* pMessage,
		CFTPConnection* pConnection,
		CFTPSocket* pPassive)
		: CWaitResponseData(WRD_STARTPASSIVE)
		, nWaitFlags(nWaitFlags)
		, pMessage(pMessage)
		, pConnection(pConnection)
		, pPassive(pPassive)
		, nCode(0)
	{
		pMessage->AddRef();
	}
	~CFTPWaitPassive();
	CFTPPassiveMessage* pMessage;
	CFTPConnection* pConnection;
	CFTPSocket* pPassive;
	WaitFlags nWaitFlags;
	int nCode;
	CMyStringW strMsg;
};

class CFTPFileListingMessage : public CFTPPassiveMessage
{
public:
	CFTPFileListingMessage(CFTPFileListingListener* pListener, LPCWSTR lpszDirectory)
		: m_pListener(pListener), m_strDirectory(lpszDirectory) { m_bForWrite = false; }

	virtual bool SendPassive(CFTPConnection* pConnection, CFTPWaitPassive* pWait) override;
	virtual bool ConnectionEstablished(CFTPWaitPassive* pWait) override
	{
		pWait->nWaitFlags = CFTPWaitPassive::WaitFlags::ReceivingData;
		return true;
	}
	virtual bool OnReceive(CFTPSocket* pPassive) override;
	virtual bool ReadyToWrite(CFTPSocket* pPassive) override { return false; }
	virtual void EndReceive(UINT* puStatusMsgID) override;

public:
	CFTPFileListingListener* m_pListener;
	CMyStringW m_strDirectory;
};

class CFTPFileMListingMessage : public CFTPPassiveMessage
{
public:
	CFTPFileMListingMessage(CFTPFileListingListener* pListener, LPCWSTR lpszDirectory)
		: m_pListener(pListener), m_strDirectory(lpszDirectory) { m_bForWrite = false; }

	virtual bool SendPassive(CFTPConnection* pConnection, CFTPWaitPassive* pWait) override;
	virtual bool ConnectionEstablished(CFTPWaitPassive* pWait) override
	{
		pWait->nWaitFlags = CFTPWaitPassive::WaitFlags::ReceivingData;
		return true;
	}
	virtual bool OnReceive(CFTPSocket* pPassive) override;
	virtual bool ReadyToWrite(CFTPSocket* pPassive) override { return false; }
	virtual void EndReceive(UINT* puStatusMsgID);

public:
	CFTPFileListingListener* m_pListener;
	CMyStringW m_strDirectory;
};

//#define STREAM_BUFFER_SIZE    16384
#define STREAM_BUFFER_SIZE    20480
#define RECV_STREAM_BUFFER_SIZE    STREAM_BUFFER_SIZE

class CFTPFileSendMessage : public CFTPPassiveMessage
{
public:
	CFTPFileSendMessage(IStream* pStreamLocalData, LPCWSTR lpszRemoteFileName);
	virtual ~CFTPFileSendMessage();

	virtual bool SendPassive(CFTPConnection* pConnection, CFTPWaitPassive* pWait) override;
	virtual bool ConnectionEstablished(CFTPWaitPassive* pWait) override
	{
		pWait->nWaitFlags = CFTPWaitPassive::WaitFlags::SendingData;
		return true;
	}
	virtual bool OnReceive(CFTPSocket* pPassive) override { return false; }
	virtual bool ReadyToWrite(CFTPSocket* pPassive) override;
	virtual void EndReceive(UINT* puStatusMsgID) override;

public:
	IStream* m_pStreamLocalData;
	CMyStringW m_strRemoteFileName;
	void* m_pvBuffer;
	ULONGLONG m_uliOffset;
	ULONG m_ulLastSize;
	bool m_bNeedRepeat;
	bool m_bFinished;
	CFTPMessageDispatcher* m_pDispatcher;
};

class CFTPWriteFileMessage : public CFTPPassiveMessage
{
public:
	CFTPWriteFileMessage(LPCWSTR lpszRemoteFileName);
	virtual ~CFTPWriteFileMessage();

	virtual bool SendPassive(CFTPConnection* pConnection, CFTPWaitPassive* pWait);
	virtual bool ConnectionEstablished(CFTPWaitPassive* pWait) override
	{
		pWait->nWaitFlags = CFTPWaitPassive::WaitFlags::SendingData;
		return true;
	}
	virtual bool OnReceive(CFTPSocket* pPassive) override { return false; }
	virtual bool ReadyToWrite(CFTPSocket* pPassive) override;
	virtual void EndReceive(UINT* puStatusMsgID) override;

public:
	CMyStringW m_strRemoteFileName;
	const void* m_pvBuffer;
	DWORD m_dwSize;
	bool m_bFinished;
	CFTPMessageDispatcher* m_pDispatcher;
};

//class CFTPStream : public CManagedUnknownImplT<IStream>
class CFTPStream : public CUnknownImplT<IStream>
{
public:
	CFTPStream(LPCWSTR lpszFileName, ULARGE_INTEGER uliDataSize, CSFTPFolderFTP* pRoot);
	~CFTPStream();

public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject);

	// ISequentialStream Interface
public:
	STDMETHOD(Read)(void* pv, ULONG cb, ULONG* pcbRead);
	STDMETHOD(Write)(void const* pv, ULONG cb, ULONG* pcbWritten);

	// IStream Interface
public:
	STDMETHOD(SetSize)(ULARGE_INTEGER libNewSize);
	STDMETHOD(CopyTo)(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten);
	STDMETHOD(Commit)(DWORD grfCommitFlags);
	STDMETHOD(Revert)();
	STDMETHOD(LockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
	STDMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
	STDMETHOD(Clone)(IStream** ppstm);
	STDMETHOD(Seek)(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer);
	STDMETHOD(Stat)(STATSTG* pStatstg, DWORD grfStatFlag);

public:
	HRESULT InitDataSocket();
	void Close();

	CMyStringW m_strFileName;
	ULARGE_INTEGER m_uliDataSize;
	ULARGE_INTEGER m_uliNowPos;
	CFTPWaitPassive* m_pPassive;
	CSFTPFolderFTP* m_pRoot;
	bool m_bClosed;
};

class CFTPFileRecvMessage : public CFTPPassiveMessage
{
public:
	CFTPFileRecvMessage(LPCWSTR lpszRemoteFileName, ULONGLONG uliOffse);
	virtual ~CFTPFileRecvMessage();

	virtual bool SendPassive(CFTPConnection* pConnection, CFTPWaitPassive* pWait) override;
	virtual bool ConnectionEstablished(CFTPWaitPassive* pWait) override
	{
		pWait->nWaitFlags = CFTPWaitPassive::WaitFlags::ReceivingDataExternal;
		return true;
	}
	virtual bool OnReceive(CFTPSocket* pPassive) override { return true; }
	virtual bool ReadyToWrite(CFTPSocket* pPassive) override { return false; }
	virtual void EndReceive(UINT* puStatusMsgID) override { }

public:
	IStream* m_pStreamLocalData;
	CMyStringW m_strRemoteFileName;
	void* m_pvBuffer;
	void* m_pSendFile;
	ULONGLONG m_uliOffset;
	bool m_bFinished;
	FILETIME m_ftFileTime;
};
