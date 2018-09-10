/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 SFTPStrm.h - declarations of CSFTPStream
 */

#pragma once

#include "SSHCli.h"
#include "SFTPChan.h"

class CSFTPSyncMessenger : public CSFTPChannelListener
{
public:
	CSFTPSyncMessenger(CSSH2Client* pClient, CSFTPChannel* pChannel, LPCWSTR lpszDirectory)
		: m_pClient(pClient)
		, m_pChannel(pChannel)
		, m_strDirectory(lpszDirectory)
		, m_bFailed(false)
		, m_uMsgID(0)
		, m_uRef(1)
	{
		pChannel->AddRef();
		pClient->AddRef();
		pClient->m_socket.EnableAsyncSelect(false, true);
	}
	virtual ~CSFTPSyncMessenger()
	{
		m_pClient->m_socket.EnableAsyncSelect(true, true);
		m_pClient->Release();
		m_pChannel->Release();
	}

//	// IUnknown
//public:
//	STDMETHOD(QueryInterface)(REFIID riid, void** ppv) { return E_NOINTERFACE; }
//	STDMETHOD_(ULONG, AddRef)() { return ++m_uRef; }
//	STDMETHOD_(ULONG, Release)()
//	{
//		ULONG u = --m_uRef;
//		if (!u)
//			delete this;
//		return u;
//	}

	// CSFTPChannelListener
public:
	virtual DWORD ChannelGetServerCompatible()
		{ return m_pClient->m_socket.GetServerCompatible(); }
	virtual bool ChannelSendPacket(BYTE bType, const void* pData, size_t nSize)
		{ return m_pClient->m_socket.SendPacket(bType, pData, nSize); }
	virtual void ChannelOpenFailure(CSSH2Channel* pChannel, int nReason, const CMyStringW& strMessage)
		{ }
	virtual void ChannelOpened(CSSH2Channel* pChannel)
		{ }
	virtual void ChannelClosed(CSSH2Channel* pChannel);
	virtual void ChannelExitStatus(CSSH2Channel* pChannel, int nExitCode)
		{ }
	virtual void ChannelConfirm(CSSH2Channel* pChannel, bool bSucceeded)
		{ }
	virtual void ChannelDataReceived(CSSH2Channel* pChannel, const void* pvData, size_t nSize)
		{ }
	virtual void SFTPOpened(CSFTPChannel* pChannel)
		{ }
	virtual void SFTPConfirm(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
		int nStatus, const CMyStringW& strMessage);
	virtual void SFTPFileHandle(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
		HSFTPHANDLE hSFTP)
		{ }
	virtual void SFTPReceiveFileName(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
		const CSFTPFileData* aFiles, int nCount)
		{ }
	virtual void SFTPReceiveData(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
		const void* pvData, size_t nLen, const bool* pbEOF)
		{ }
	virtual void SFTPReceiveAttributes(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
		const CSFTPFileAttribute& attrs);
	virtual void SFTPReceiveStatVFS(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
		const struct sftp_statvfs& statvfs)
		{ }

public:
	bool TryStat(CFTPFileItem* pItem);
	bool TryFStat(HSFTPHANDLE hFile);

private:
	ULONG m_uRef;

public:
	bool m_bFailed;
	ULONG m_uMsgID;
	ULARGE_INTEGER m_uliDataSize;
	FILETIME m_ftModTime;
	CSSH2Client* m_pClient;
	CSFTPChannel* m_pChannel;
	CMyStringW m_strDirectory;

protected:
	bool WaitForCurrentMessage();
};

//class CSFTPStream : public CManagedUnknownImplT<IStream>, public CSFTPSyncMessenger
class CSFTPStream : public CUnknownImplT<IStream>, public CSFTPSyncMessenger
{
public:
	CSFTPStream(IUnknown* pUnkOuter, CSSH2Client* pClient, CSFTPChannel* pChannel, LPCWSTR lpszDirectory);
	virtual ~CSFTPStream();

public:
	virtual void SFTPConfirm(CSFTPChannel* pChannel, CSFTPMessage* pMsg, int nStatus, const CMyStringW& strMessage);
	virtual void SFTPFileHandle(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
		HSFTPHANDLE hSFTP);
	virtual void SFTPReceiveData(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
		const void* pvData, size_t nLen, const bool* pbEOF);

public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject);
	STDMETHOD_(ULONG, AddRef)() { return ++CUnknownImplT<IStream>::m_uRef; }
	STDMETHOD_(ULONG, Release)()
	{
		ULONG u = --CUnknownImplT<IStream>::m_uRef;
		if (!u)
			delete this;
		return u;
	}

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
	bool TryOpenFile(CFTPFileItem* pFileItem);
	bool TryFStat()
		{ return CSFTPSyncMessenger::TryFStat(m_hFile); }

public:
	IUnknown* m_pUnkOuter;
	HSFTPHANDLE m_hFile;
	ULARGE_INTEGER m_uliOffset;

	void* m_pvCurBuffer;
	ULONG m_uReqSize;
	ULONG* m_pcbRead;
	bool m_bAnyRead;
	bool m_bEOF, m_bNextEOF;

	CExBuffer m_bufferCache;
	size_t m_nCacheSize;
};
