/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 FTPStrm.h - declaration of CFTPFileStream
 */

#pragma once

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
