/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 FTPMsg.h - declarations of FTP passive message classes
 */

#pragma once

#include "MySocket.h"
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

class __declspec(novtable) CFTPPassiveMessage : public IUnknown
{
public:
	CFTPPassiveMessage() { m_bCanceled = false; }

	virtual bool SendPassive(CFTPConnection* pConnection, CFTPWaitPassive* pWait) = 0;
	// return true if succeeded
	// if return false, the implementation must delete pPassive
	virtual bool ConnectionEstablished(CTextSocket* pPassive) = 0;
	// return false if no data has received
	virtual bool OnReceive(CTextSocket* pPassive) = 0;
	// return false if finished
	virtual bool ReadyToWrite(CTextSocket* pPassive) = 0;
	virtual void EndReceive(UINT* puStatusMsgID) = 0;

	bool m_bCanceled;
	bool m_bForWrite;
};

class __declspec(novtable) CFTPFileListingListener
{
public:
	virtual bool ReceiveFileListing(CTextSocket* pPassive, bool bMListing) = 0;
	virtual void FinishFileListing() = 0;
};

class CFTPFileListingMessage : public CUnknownImplT<CFTPPassiveMessage>
{
public:
	CFTPFileListingMessage(CFTPFileListingListener* pListener, LPCWSTR lpszDirectory)
		: m_pListener(pListener), m_strDirectory(lpszDirectory) { m_bForWrite = false; }

	virtual bool SendPassive(CFTPConnection* pConnection, CFTPWaitPassive* pWait);
	virtual bool ConnectionEstablished(CTextSocket* pPassive) { return true; }
	virtual bool OnReceive(CTextSocket* pPassive);
	virtual bool ReadyToWrite(CTextSocket* pPassive) { return false; }
	virtual void EndReceive(UINT* puStatusMsgID);

public:
	CFTPFileListingListener* m_pListener;
	CMyStringW m_strDirectory;
};

class CFTPFileMListingMessage : public CUnknownImplT<CFTPPassiveMessage>
{
public:
	CFTPFileMListingMessage(CFTPFileListingListener* pListener, LPCWSTR lpszDirectory)
		: m_pListener(pListener), m_strDirectory(lpszDirectory) { m_bForWrite = false; }

	virtual bool SendPassive(CFTPConnection* pConnection, CFTPWaitPassive* pWait);
	virtual bool ConnectionEstablished(CTextSocket* pPassive) { return true; }
	virtual bool OnReceive(CTextSocket* pPassive);
	virtual bool ReadyToWrite(CTextSocket* pPassive) { return false; }
	virtual void EndReceive(UINT* puStatusMsgID);

public:
	CFTPFileListingListener* m_pListener;
	CMyStringW m_strDirectory;
};

//#define STREAM_BUFFER_SIZE    16384
#define STREAM_BUFFER_SIZE    20480
#define RECV_STREAM_BUFFER_SIZE    STREAM_BUFFER_SIZE

class CFTPFileSendMessage : public CUnknownImplT<CFTPPassiveMessage>
{
public:
	CFTPFileSendMessage(IStream* pStreamLocalData, LPCWSTR lpszRemoteFileName);
	virtual ~CFTPFileSendMessage();

	virtual bool SendPassive(CFTPConnection* pConnection, CFTPWaitPassive* pWait);
	virtual bool ConnectionEstablished(CTextSocket* pPassive) { return true; }
	virtual bool OnReceive(CTextSocket* pPassive) { return false; }
	virtual bool ReadyToWrite(CTextSocket* pPassive);
	virtual void EndReceive(UINT* puStatusMsgID);

public:
	IStream* m_pStreamLocalData;
	CMyStringW m_strRemoteFileName;
	void* m_pvBuffer;
	ULONGLONG m_uliOffset;
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

	CMyStringW m_strFileName;
	ULARGE_INTEGER m_uliDataSize;
	ULARGE_INTEGER m_uliNowPos;
	CTextSocket* m_pDataSocket;
	CSFTPFolderFTP* m_pRoot;
};

class CFTPFileRecvMessage : public CUnknownImplT<CFTPPassiveMessage>
{
public:
	CFTPFileRecvMessage(LPCWSTR lpszRemoteFileName, ULONGLONG uliOffse);
	virtual ~CFTPFileRecvMessage();

	virtual bool SendPassive(CFTPConnection* pConnection, CFTPWaitPassive* pWait);
	virtual bool ConnectionEstablished(CTextSocket* pPassive) { return true; }
	virtual bool OnReceive(CTextSocket* pPassive) { return true; }
	virtual bool ReadyToWrite(CTextSocket* pPassive) { return false; }
	virtual void EndReceive(UINT* puStatusMsgID) { }
	virtual void* GetSyncSendFile() { return m_pSendFile; }

public:
	IStream* m_pStreamLocalData;
	CMyStringW m_strRemoteFileName;
	void* m_pvBuffer;
	void* m_pSendFile;
	ULONGLONG m_uliOffset;
	bool m_bFinished;
	FILETIME m_ftFileTime;
};
