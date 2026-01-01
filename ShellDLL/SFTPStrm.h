/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 SFTPStrm.h - declarations of CSFTPStream
 */

#pragma once

#include "SSHCli.h"
#include "SFTPChan.h"

class __declspec(novtable) CPumpMessageProcessor
{
public:
	virtual HRESULT PumpSocketAndMessage(DWORD dwWaitTime = 0xFFFFFFFF) = 0;
};

class CSFTPSyncMessenger : public CSFTPChannelListener
{
public:
	CSFTPSyncMessenger(CPumpMessageProcessor* pProcessor, CSFTPChannel* pChannel, LPCWSTR lpszDirectory)
		: m_pProcessor(pProcessor)
		, m_pChannel(pChannel)
		, m_strDirectory(lpszDirectory)
		, m_bFailed(false)
		, m_uMsgID(0)
		, m_uRef(1)
	{
		pChannel->AddRef();
	}
	virtual ~CSFTPSyncMessenger()
	{
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
	virtual void ChannelOpenFailure(CSSHChannel* pChannel, int nReason)
		{ }
	virtual void ChannelError(CSSHChannel* pChannel, int nReason)
		{ }
	virtual void ChannelOpened(CSSHChannel* pChannel)
		{ }
	virtual void ChannelClosed(CSSHChannel* pChannel);
	virtual void ChannelExitStatus(CSSHChannel* pChannel, int nExitCode)
		{ }
	virtual void ChannelConfirm(CSSHChannel* pChannel, bool bSucceeded, int nReason)
		{ }
	virtual void ChannelDataReceived(CSSHChannel* pChannel, const void* pvData, size_t nSize)
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
	CPumpMessageProcessor* m_pProcessor;
	CSFTPChannel* m_pChannel;
	CMyStringW m_strDirectory;

protected:
	bool WaitForCurrentMessage();
};

//class CSFTPStream : public CManagedUnknownImplT<IStream>, public CSFTPSyncMessenger
class CSFTPStream : public CUnknownImplT<IStream>, public CSFTPSyncMessenger
{
public:
	CSFTPStream(IUnknown* pUnkOuter, CPumpMessageProcessor* pProcessor, CSFTPChannel* pChannel, LPCWSTR lpszDirectory);
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
	CMyStringW m_strFileName;

	void* m_pvCurBuffer;
	ULONG m_uReqSize;
	ULONG* m_pcbRead;
	bool m_bAnyRead;
	bool m_bEOF, m_bNextEOF;

	CExBuffer m_bufferCache;
	size_t m_nCacheSize;
};
