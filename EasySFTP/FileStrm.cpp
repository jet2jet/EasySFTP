/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 FileStrm.cpp - implementations of file stream functions and CFileStream
 */

#include "stdafx.h"
#include "FileStrm.h"

#include "Unknown.h"
#include "Unicode.h"
#include "UString.h"

// {6CF190E9-90EF-4b4b-8446-446A624A8C9D}
static const IID IID_IFileStream =
	{ 0x6cf190e9, 0x90ef, 0x4b4b, { 0x84, 0x46, 0x44, 0x6a, 0x62, 0x4a, 0x8c, 0x9d } };

//implement filestream that derives from IStream
class CFileStream : public CUnknownImplT<IFileStream>
{
public:
	CFileStream(HANDLE hFile, DWORD dwDesiredAccess);
	~CFileStream();

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

	// IFileStream Interface
public:
	STDMETHOD(GetFileHandle)(HANDLE FAR* phFile)
	{
		if (!phFile)
			return E_POINTER;
		*phFile = m_hFile;
		return S_OK;
	}

protected:
	HANDLE m_hFile;
	DWORD m_dwDesiredAccess;
};

STDAPI MyOpenFileToStream(LPCWSTR pName, bool fWrite, IStream** ppStream)
{
	return MyOpenFileToStreamEx(pName, fWrite ? GENERIC_WRITE : GENERIC_READ,
		FILE_SHARE_READ, NULL,
		fWrite ? CREATE_ALWAYS : OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL, ppStream);
}

STDAPI MyOpenFileToStreamEx(LPCWSTR lpFileName, DWORD dwDesiredAccess,
	DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes, HANDLE hTemplateFile, IStream** ppStream)
{
	HANDLE hFile;
	DWORD dwError;

	hFile = MyCreateFileW(lpFileName, dwDesiredAccess,
		dwShareMode, lpSecurityAttributes, dwCreationDisposition,
		dwFlagsAndAttributes, hTemplateFile);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		dwError = ::GetLastError();
		return HRESULT_FROM_WIN32(dwError);
	}

	IStream* ps = new CFileStream(hFile, dwDesiredAccess);
	if (ps == NULL)
	{
		::CloseHandle(hFile);
		return E_OUTOFMEMORY;
	}

	*ppStream = ps;
	return S_OK;
}

STDAPI MyGetFileHandleFromStream(IStream* pStream, HANDLE* phFile)
{
	IFileStream* p;
	HRESULT hr;
	hr = pStream->QueryInterface(IID_IFileStream, (void**) &p);
	if (FAILED(hr))
		return hr;
	hr = p->GetFileHandle(phFile);
	p->Release();
	return hr;
}

////////////////////////////////////////////////////////////////////////////////

CFileStream::CFileStream(HANDLE hFile, DWORD dwDesiredAccess)
{
	m_hFile = hFile;
	m_dwDesiredAccess = dwDesiredAccess;
}

CFileStream::~CFileStream()
{
	if (m_hFile != NULL)
		::CloseHandle(m_hFile);
}

STDMETHODIMP CFileStream::QueryInterface(REFIID riid, void** ppvObject)
{ 
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IStream) ||
		IsEqualIID(riid, IID_ISequentialStream) ||
		IsEqualIID(riid, IID_IFileStream))
	{
		*ppvObject = static_cast<IFileStream*>(this);
		AddRef();
		return S_OK;
	}
	else
		return E_NOINTERFACE; 
}

STDMETHODIMP CFileStream::Read(void* pv, ULONG cb, ULONG* pcbRead)
{
	if (!pv)
		return E_POINTER;
	ULONG u;
	if (!::ReadFile(m_hFile, pv, cb, &u, NULL))
		return HRESULT_FROM_WIN32(::GetLastError());
	if (pcbRead)
		*pcbRead = u;
	return (u) ? S_OK : S_FALSE;
}

STDMETHODIMP CFileStream::Write(const void* pv, ULONG cb, ULONG* pcbWritten)
{
	if (!pv)
		return E_POINTER;
	ULONG u;
	if (!::WriteFile(m_hFile, pv, cb, &u, NULL))
		return HRESULT_FROM_WIN32(::GetLastError());
	if (pcbWritten)
		*pcbWritten = u;
	return (u) ? S_OK : E_FAIL;
}

STDMETHODIMP CFileStream::SetSize(ULARGE_INTEGER)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFileStream::CopyTo(IStream *, ULARGE_INTEGER, ULARGE_INTEGER *, ULARGE_INTEGER *)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFileStream::Commit(DWORD)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFileStream::Revert()
{
    return E_NOTIMPL;
}

STDMETHODIMP CFileStream::LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFileStream::UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFileStream::Clone(IStream**)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFileStream::Seek(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER *lpNewFilePointer)
{ 
	DWORD dwMoveMethod;

	switch (dwOrigin)
	{
		case STREAM_SEEK_SET:
			dwMoveMethod = FILE_BEGIN;
			break;
		case STREAM_SEEK_CUR:
			dwMoveMethod = FILE_CURRENT;
			break;
		case STREAM_SEEK_END:
			dwMoveMethod = FILE_END;
			break;
		default:   
			return STG_E_INVALIDFUNCTION;
	}

	liDistanceToMove.LowPart = ::SetFilePointer(m_hFile, liDistanceToMove.LowPart, &liDistanceToMove.HighPart, dwMoveMethod);
	DWORD dw;
	if (liDistanceToMove.LowPart == INVALID_SET_FILE_POINTER &&
		(dw = ::GetLastError()) != ERROR_SUCCESS)
		return HRESULT_FROM_WIN32(dw);

	if (lpNewFilePointer)
		lpNewFilePointer->QuadPart = (ULONGLONG) liDistanceToMove.QuadPart;

	return S_OK;
}

STDMETHODIMP CFileStream::Stat(STATSTG* pStatstg, DWORD grfStatFlag)
{
	if (!pStatstg)
		return E_POINTER;

	DWORD dw;
	if (!(grfStatFlag & STATFLAG_NONAME))
		pStatstg->pwcsName[0] = 0;
	pStatstg->type = STGTY_STREAM;
	pStatstg->cbSize.LowPart = ::GetFileSize(m_hFile, &pStatstg->cbSize.HighPart);
	if (pStatstg->cbSize.LowPart == (DWORD) -1 &&
		(dw = ::GetLastError()) != ERROR_SUCCESS)
		return HRESULT_FROM_WIN32(dw);
	if (!::GetFileTime(m_hFile, &pStatstg->ctime, &pStatstg->atime, &pStatstg->mtime))
		return HRESULT_FROM_WIN32(::GetLastError());
	if (m_dwDesiredAccess == (GENERIC_READ | GENERIC_WRITE))
		pStatstg->grfMode = STGM_READWRITE;
	else if (m_dwDesiredAccess == GENERIC_READ)
		pStatstg->grfMode = STGM_READ;
	else if (m_dwDesiredAccess == GENERIC_WRITE)
		pStatstg->grfMode = STGM_WRITE;
	else
		pStatstg->grfMode = 0;
	pStatstg->grfLocksSupported = 0;

	return S_OK;
}
