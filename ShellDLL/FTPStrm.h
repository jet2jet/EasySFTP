/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 FTPStrm.h - declaration of CFTPFileStream
 */

#pragma once

#include "Dispatch.h"
#include "TferStat.h"
#include "Transfer.h"

EXTERN_C HRESULT __stdcall CopyStream(IStream* pSource, IStream* pDestination, void* pvObject, CTransferStatus* pStatus);

class CFTPDirectoryRootBase;

class CFTPFileStream : public CUnknownImplT<IStream>
{
public:
	CFTPFileStream(CFTPDirectoryRootBase* pRoot);
	virtual ~CFTPFileStream();

	void SetHandle(HANDLE hFile) { m_hFile = hFile; }

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

private:
	CFTPDirectoryRootBase* m_pRoot;
	HANDLE m_hFile;
};

class CEasySFTPStream : public CDispatchImplT<IEasySFTPStream>
{
public:
	CEasySFTPStream(IStream* pStreamBase, IEasySFTPFile* pFile, CTransferDialog* pTransferDialog, CTransferDialog::CTransferItem* pTransfer);
	virtual ~CEasySFTPStream();

	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject);

	STDMETHOD(Read)(void* buffer, long length, long* piRead);
	STDMETHOD(Write)(const void* buffer, long length, long* piWritten);
	STDMETHOD(ReadAsUtf8String)(long length, BSTR* pbRet);
	STDMETHOD(WriteAsUtf8String)(BSTR bstrString, long* piWritten);
	STDMETHOD(Seek)(long pos, EasySFTPSeekOrigin origin, long* pNew);
	STDMETHOD(Seek64)(hyper pos, EasySFTPSeekOrigin origin, hyper* pNew);
	STDMETHOD(get_File)(IEasySFTPFile** ppFile);

private:
	void UpdatePos(ULONGLONG newPos);

private:
	IStream* m_pStreamBase;
	IEasySFTPFile* m_pFile;
	CTransferDialog* m_pTransferDialog;
	CTransferDialog::CTransferItem* m_pTransfer;
	ULONGLONG m_uliPos;
};
