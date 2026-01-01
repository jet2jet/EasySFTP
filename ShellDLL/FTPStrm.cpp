/*
 EasySFTP - Copyright (C) 2025 jet (ジェット)

 FTPStrm.cpp - implementation of CFTPFileStream
 */

#include "stdafx.h"
#include "ShellDLL.h"
#include "FTPStrm.h"

#include "Folder.h"
#include "FoldRoot.h"

static_assert(static_cast<int>(EasySFTPSeekOrigin::SeekBegin) == STREAM_SEEK_SET, "Unexpected value");
static_assert(static_cast<int>(EasySFTPSeekOrigin::SeekCurrent) == STREAM_SEEK_CUR, "Unexpected value");
static_assert(static_cast<int>(EasySFTPSeekOrigin::SeekEnd) == STREAM_SEEK_END, "Unexpected value");

 ///////////////////////////////////////////////////////////////////////////////

EXTERN_C HRESULT __stdcall CopyStream(IStream* pSource, IStream* pDestination, void* pvObject, CTransferStatus* pStatus)
{
	void* buffer = malloc(32768);
	if (!buffer)
		return E_OUTOFMEMORY;
	HRESULT hr = S_OK;
	ULONGLONG uliOffset = 0;
	while (true)
	{
		while (theApp.CheckQueueMessage())
		{
			if (!theApp.MyPumpMessage2())
			{
				hr = E_UNEXPECTED;
				break;
			}
		}
		ULONG uSize = 32768, u = 0;
		hr = pSource->Read(buffer, uSize, &u);
		if (FAILED(hr))
			break;
		if (u == 0)
		{
			hr = S_OK;
			break;
		}
		if (pStatus->TransferIsCanceled(pvObject))
		{
			hr = S_FALSE;
			break;
		}
		hr = pDestination->Write(buffer, u, &u);
		if (FAILED(hr))
			break;
		uliOffset += u;
		pStatus->TransferInProgress(pvObject, uliOffset);
	}
	free(buffer);
	return hr;
}

 ///////////////////////////////////////////////////////////////////////////////

CFTPFileStream::CFTPFileStream(CFTPDirectoryRootBase* pRoot)
	: m_pRoot(pRoot), m_hFile(NULL)
{
	pRoot->AddRef();
}

CFTPFileStream::~CFTPFileStream()
{
	if (m_hFile)
		m_pRoot->CloseFile(m_hFile);
	m_pRoot->Release();
}

STDMETHODIMP CFTPFileStream::QueryInterface(REFIID riid, void** ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	*ppvObject = NULL;
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
	if (m_pRoot->m_bIsTransferCanceled)
		return E_ABORT;
	return m_pRoot->ReadFile(m_hFile, pv, cb, pcbRead);
}

STDMETHODIMP CFTPFileStream::Write(const void* pv, ULONG cb, ULONG* pcbWritten)
{
	if (m_pRoot->m_bIsTransferCanceled)
		return E_ABORT;
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
	if (m_pRoot->m_bIsTransferCanceled)
		return E_ABORT;

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
	if (m_pRoot->m_bIsTransferCanceled)
		return E_ABORT;
	return m_pRoot->LockRegion(m_hFile, libOffset, cb, dwLockType);
}

STDMETHODIMP CFTPFileStream::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	if (m_pRoot->m_bIsTransferCanceled)
		return E_ABORT;
	return m_pRoot->UnlockRegion(m_hFile, libOffset, cb, dwLockType);
}

STDMETHODIMP CFTPFileStream::Clone(IStream** ppstm)
{
	if (!ppstm)
		return E_POINTER;
	*ppstm = NULL;
	if (m_pRoot->m_bIsTransferCanceled)
		return E_ABORT;
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
	if (m_pRoot->m_bIsTransferCanceled)
		return E_ABORT;
	return m_pRoot->SeekFile(m_hFile, liDistanceToMove, dwOrigin, lpNewFilePointer);
}

STDMETHODIMP CFTPFileStream::Stat(STATSTG* pStatstg, DWORD grfStatFlag)
{
	if (m_pRoot->m_bIsTransferCanceled)
		return E_ABORT;
	return m_pRoot->StatFile(m_hFile, pStatstg, grfStatFlag);
}

///////////////////////////////////////////////////////////////////////////////

CEasySFTPStream::CEasySFTPStream(IStream* pStreamBase, IEasySFTPFile* pFile, CTransferDialog* pTransferDialog, CTransferDialog::CTransferItem* pTransfer)
	: CDispatchImplT(theApp.GetTypeInfo(IID_IEasySFTPStream))
	, m_pStreamBase(pStreamBase)
	, m_pFile(pFile)
	, m_pTransferDialog(pTransferDialog)
	, m_pTransfer(pTransfer)
	, m_uliPos(0)
{
	pStreamBase->AddRef();
	pFile->AddRef();
}

CEasySFTPStream::~CEasySFTPStream()
{
	if (m_pTransferDialog && m_pTransfer)
		m_pTransferDialog->RemoveTransferItem(m_pTransfer, m_uliPos < m_pTransfer->uliMax);
	m_pFile->Release();
	m_pStreamBase->Release();
}

STDMETHODIMP CEasySFTPStream::QueryInterface(REFIID riid, void** ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	*ppvObject = NULL;
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IDispatch) ||
		IsEqualIID(riid, IID_IEasySFTPStream))
	{
		*ppvObject = static_cast<IEasySFTPStream*>(this);
		AddRef();
		return S_OK;
	}
	else
		return E_NOINTERFACE;
}

STDMETHODIMP CEasySFTPStream::Read(void* buffer, long length, long* piRead)
{
	if (piRead)
		*piRead = 0;
	if (!buffer)
		return E_POINTER;
	if (length < 0)
		return E_INVALIDARG;
	ULONG u = 0;
	auto hr = m_pStreamBase->Read(buffer, static_cast<ULONG>(length), &u);
	if (SUCCEEDED(hr))
	{
		UpdatePos(m_uliPos + u);
		if (piRead)
			*piRead = static_cast<long>(u);
	}
	return hr;
}

STDMETHODIMP CEasySFTPStream::Write(const void* buffer, long length, long* piWritten)
{
	if (piWritten)
		*piWritten = 0;
	if (!buffer)
		return E_POINTER;
	if (length < 0)
		return E_INVALIDARG;
	ULONG u = 0;
	auto hr = m_pStreamBase->Write(buffer, static_cast<ULONG>(length), &u);
	if (SUCCEEDED(hr))
	{
		UpdatePos(m_uliPos + u);
		if (piWritten)
			*piWritten = static_cast<long>(u);
	}
	return hr;
}

STDMETHODIMP CEasySFTPStream::ReadAsUtf8String(long length, BSTR* pbRet)
{
	if (!pbRet)
		return E_POINTER;
	*pbRet = NULL;
	if (length < 0)
		return E_INVALIDARG;
	auto* buffer = malloc(static_cast<size_t>(length));
	if (!buffer)
		return E_OUTOFMEMORY;
	ULONG u = 0;
	auto hr = m_pStreamBase->Read(buffer, static_cast<ULONG>(length), &u);
	if (FAILED(hr))
	{
		free(buffer);
		return hr;
	}
	UpdatePos(m_uliPos + u);
	CMyStringW str;
	str.SetUTF8String(static_cast<LPCBYTE>(buffer), static_cast<size_t>(u));
	free(buffer);
	if (u > 0 && str.IsEmpty())
		return E_OUTOFMEMORY;
	*pbRet = MyStringToBSTR(str);
	return *pbRet ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CEasySFTPStream::WriteAsUtf8String(BSTR bstrString, long* piWritten)
{
	if (!bstrString)
		return E_INVALIDARG;
	if (piWritten)
		*piWritten = 0;
	CMyStringW str;
	MyBSTRToString(bstrString, str);
	size_t length = 0;
	auto buffer = str.AllocUTF8String(&length);
	ULONG u = 0;
	auto hr = m_pStreamBase->Write(buffer, static_cast<ULONG>(length), &u);
	if (FAILED(hr))
		return hr;
	UpdatePos(m_uliPos + u);
	if (piWritten)
		*piWritten = static_cast<long>(u);
	return S_OK;
}

STDMETHODIMP CEasySFTPStream::Seek(long pos, EasySFTPSeekOrigin origin, long* pNew)
{
	LARGE_INTEGER liDistanceToMove;
	liDistanceToMove.QuadPart = static_cast<LONGLONG>(pos);
	DWORD dwOrigin = static_cast<DWORD>(origin);
	ULARGE_INTEGER newFilePointer = {};
	auto hr = m_pStreamBase->Seek(liDistanceToMove, dwOrigin, &newFilePointer);
	if (SUCCEEDED(hr))
	{
		UpdatePos(newFilePointer.QuadPart);
		if (pNew)
			*pNew = static_cast<long>(newFilePointer.LowPart);
	}
	return hr;
}

STDMETHODIMP CEasySFTPStream::Seek64(hyper pos, EasySFTPSeekOrigin origin, hyper* pNew)
{
	LARGE_INTEGER liDistanceToMove;
	liDistanceToMove.QuadPart = pos;
	DWORD dwOrigin = static_cast<DWORD>(origin);
	ULARGE_INTEGER newFilePointer = {};
	auto hr = m_pStreamBase->Seek(liDistanceToMove, dwOrigin, &newFilePointer);
	if (SUCCEEDED(hr))
	{
		UpdatePos(newFilePointer.QuadPart);
		if (pNew)
			*pNew = static_cast<hyper>(newFilePointer.QuadPart);
	}
	return hr;
}

STDMETHODIMP CEasySFTPStream::get_File(IEasySFTPFile** ppFile)
{
	if (!ppFile)
		return E_POINTER;
	*ppFile = m_pFile;
	if (m_pFile)
		m_pFile->AddRef();
	return S_OK;
}

void CEasySFTPStream::UpdatePos(ULONGLONG newPos)
{
	m_uliPos = newPos;
	if (m_pTransferDialog && m_pTransfer)
		m_pTransferDialog->UpdateTransferItem(m_pTransfer, newPos);
}
