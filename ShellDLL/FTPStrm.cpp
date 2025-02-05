/*
 EasySFTP - Copyright (C) 2025 Kuri-Applications

 FTPStrm.cpp - implementation of CFTPFileStream
 */

#include "stdafx.h"
#include "ShellDLL.h"
#include "FTPStrm.h"

#include "Folder.h"
#include "FoldRoot.h"

 ///////////////////////////////////////////////////////////////////////////////

CFTPFileStream::CFTPFileStream(CFTPDirectoryRootBase* pRoot)
	: m_pRoot(pRoot), m_hFile(NULL)
{
	pRoot->AddRef();
}

CFTPFileStream::~CFTPFileStream()
{
	m_pRoot->Release();
}

STDMETHODIMP CFTPFileStream::QueryInterface(REFIID riid, void** ppvObject)
{
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IStream) ||
		IsEqualIID(riid, IID_ISequentialStream))
	{
		*ppvObject = static_cast<IStream*>(this);
		AddRef();
		return S_OK;
	}
	else
		return E_NOINTERFACE;
}

STDMETHODIMP CFTPFileStream::Read(void* pv, ULONG cb, ULONG* pcbRead)
{
	if (!pv)
		return E_POINTER;
	return m_pRoot->ReadFile(m_hFile, pv, cb, pcbRead);
}

STDMETHODIMP CFTPFileStream::Write(const void* pv, ULONG cb, ULONG* pcbWritten)
{
	return m_pRoot->WriteFile(m_hFile, pv, cb, pcbWritten);
}

STDMETHODIMP CFTPFileStream::SetSize(ULARGE_INTEGER)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFTPFileStream::CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten)
{
	if (!pstm)
		return STG_E_INVALIDPOINTER;

	static constexpr ULONG CACHE_SIZE = 32768;

	HRESULT hr = S_FALSE;
	ULONG ur, uw;
	CExBuffer buf;
	void* pv = buf.AppendToBuffer(NULL, CACHE_SIZE);
	if (!pv)
		return E_OUTOFMEMORY;
	while (cb.QuadPart)
	{
		hr = Read(pv, CACHE_SIZE, &ur);
		if (hr != S_OK)
		{
			if (SUCCEEDED(hr))
				hr = S_OK;
			break;
		}
		if (pcbRead)
			pcbRead->QuadPart += ur;
		cb.QuadPart -= ur;
		hr = pstm->Write(pv, ur, &uw);
		if (FAILED(hr))
			break;
		if (pcbWritten)
			pcbWritten->QuadPart += uw;
	}
	return hr;
}

STDMETHODIMP CFTPFileStream::Commit(DWORD)
{
	return STG_E_INVALIDFUNCTION;
}

STDMETHODIMP CFTPFileStream::Revert()
{
	return STG_E_INVALIDFUNCTION;
}

STDMETHODIMP CFTPFileStream::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	return m_pRoot->LockRegion(m_hFile, libOffset, cb, dwLockType);
}

STDMETHODIMP CFTPFileStream::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	return m_pRoot->UnlockRegion(m_hFile, libOffset, cb, dwLockType);
}

STDMETHODIMP CFTPFileStream::Clone(IStream** ppstm)
{
	HANDLE hFile;
	auto p = new CFTPFileStream(m_pRoot);
	if (!p)
		return E_OUTOFMEMORY;
	auto hr = m_pRoot->DuplicateFile(m_hFile, &hFile);
	if (FAILED(hr))
	{
		delete p;
		return hr;
	}
	p->m_hFile = hFile;
	*ppstm = p;
	return S_OK;
}

STDMETHODIMP CFTPFileStream::Seek(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer)
{
	return m_pRoot->SeekFile(m_hFile, liDistanceToMove, dwOrigin, lpNewFilePointer);
}

STDMETHODIMP CFTPFileStream::Stat(STATSTG* pStatstg, DWORD grfStatFlag)
{
	return m_pRoot->StatFile(m_hFile, pStatstg, grfStatFlag);
}
