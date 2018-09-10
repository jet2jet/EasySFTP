/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 TextStrm.cpp - implementations of text stream functions and CTextStream
 */

#include "stdafx.h"
#include "TextStrm.h"

#include "FileStrm.h"
#include "Unknown.h"
#include "ExBuffer.h"
#include "Convert.h"

class CTextStream : public CUnknownImplT<IFileStream>
{
public:
	CTextStream(IStream* pStream, BYTE fTextMode);
	virtual ~CTextStream();

public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject);

	// ISequentialStream Interface
public:
	STDMETHOD(Read)(void* pv, ULONG cb, ULONG* pcbRead);
	STDMETHOD(Write)(void const* pv, ULONG cb, ULONG* pcbWritten);

	// IStream Interface
public:
	STDMETHOD(SetSize)(ULARGE_INTEGER libNewSize)
		{ return m_pStream->SetSize(libNewSize); }
	STDMETHOD(CopyTo)(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten);
	STDMETHOD(Commit)(DWORD grfCommitFlags)
		{ return m_pStream->Commit(grfCommitFlags); }
	STDMETHOD(Revert)()
		{ m_buffer.Empty(); m_bufferChopped.Empty(); return m_pStream->Revert(); }
	STDMETHOD(LockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
		{ return m_pStream->LockRegion(libOffset, cb, dwLockType); }
	STDMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
		{ return m_pStream->LockRegion(libOffset, cb, dwLockType); }
	STDMETHOD(Clone)(IStream** ppstm)
	{
		IStream* pstm;
		HRESULT hr = m_pStream->Clone(&pstm);
		if (FAILED(hr))
			return hr;
		*ppstm = new CTextStream(pstm, m_fTextMode);
		pstm->Release();
		return *ppstm != NULL ? S_OK : E_OUTOFMEMORY;
	}
	STDMETHOD(Seek)(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer);
	STDMETHOD(Stat)(STATSTG* pStatstg, DWORD grfStatFlag)
	{
		HRESULT hr = m_pStream->Stat(pStatstg, grfStatFlag);
		if (FAILED(hr) || TEXTMODE_IS_NO_CONVERTION(m_fTextMode))
			return hr;
		//// return E_NOTIMPL if convertion is valid (cannot calculate the size of data)
		//pStatstg->cbSize.QuadPart = 0;
		pStatstg->cbSize.QuadPart *= 2;
		return hr;
		//return STG_E_INCOMPLETE;
	}

	// IFileStream Interface
public:
	STDMETHOD(GetFileHandle)(HANDLE FAR* phFile)
	{
		return MyGetFileHandleFromStream(m_pStream, phFile);
	}

protected:
	BYTE m_fTextMode;
	CExBuffer m_buffer;
	// 変換できなかったデータ
	// [Read] 次に生データを取得する際に先頭に結合される
	// [Write] 次の Write 呼び出し時に先頭に追加される
	CExBuffer m_bufferChopped;
	bool m_bBeforeWrite;
	IStream* m_pStream;
};

STDAPI MyCreateTextStream(IStream* pStreamBase, BYTE fTextMode, IStream** ppStreamOut)
{
	if (!ppStreamOut)
		return E_POINTER;
	CTextStream* p = new CTextStream(pStreamBase, fTextMode);
	*ppStreamOut = p;
	if (!p)
		return E_OUTOFMEMORY;
	return S_OK;
}

STDAPI MyOpenTextFileToStream(LPCWSTR pName, bool fWrite, BYTE fTextMode, IStream** ppStream)
{
	HRESULT hr = MyOpenFileToStream(pName, fWrite, ppStream);
	if (FAILED(hr))
		return hr;

	if (!TEXTMODE_IS_NO_CONVERTION(fTextMode))
	{
		IStream* ps = *ppStream;
		hr = MyCreateTextStream(ps, fTextMode, ppStream);
		ps->Release();
	}

	return hr;
}

STDAPI MyOpenTextFileToStreamEx(LPCWSTR lpFileName, DWORD dwDesiredAccess,
	DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes, HANDLE hTemplateFile, BYTE fTextMode, IStream** ppStream)
{
	HRESULT hr = MyOpenFileToStreamEx(lpFileName, dwDesiredAccess,
		dwShareMode, lpSecurityAttributes, dwCreationDisposition,
		dwFlagsAndAttributes, hTemplateFile, ppStream);
	if (FAILED(hr))
		return hr;

	if (!TEXTMODE_IS_NO_CONVERTION(fTextMode))
	{
		IStream* ps = *ppStream;
		hr = MyCreateTextStream(ps, fTextMode, ppStream);
		ps->Release();
	}

	return hr;
}

////////////////////////////////////////////////////////////////////////////////

static ULONG __stdcall ConvertReturnModeW(bool bFromStream, BYTE fTextMode, void* pvDest, const void* pvBuffer, ULONG nSize, ULONG* pnChoppedSize);

static inline void __stdcall AppendReturnsA(char*& pszBuffer, BYTE fTextTo)
{
	switch (fTextTo)
	{
		case TEXTMODE_BUFFER_CR:
			*pszBuffer++ = '\r';
			break;
		case TEXTMODE_BUFFER_CRLF:
			*pszBuffer++ = '\r';
		case TEXTMODE_BUFFER_LF:
			*pszBuffer++ = '\n';
			break;
	}
}

static inline void __stdcall AppendReturnsW(wchar_t*& pszBuffer, BYTE fTextTo)
{
	switch (fTextTo)
	{
		case TEXTMODE_BUFFER_CR:
			*pszBuffer++ = L'\r';
			break;
		case TEXTMODE_BUFFER_CRLF:
			*pszBuffer++ = L'\r';
		case TEXTMODE_BUFFER_LF:
			*pszBuffer++ = L'\n';
			break;
	}
}

// return: new buffer size
// NOTE: max-size of pvDest must be at least nSize * 2
static ULONG __stdcall ConvertReturnMode(bool bFromStream, BYTE fTextMode, void* pvDest, const void* pvBuffer, ULONG nSize, ULONG* pnChoppedSize)
{
	BYTE fEncode = fTextMode & TEXTMODE_ENCODE_MASK;
	if (fEncode == TEXTMODE_UCS4)
		return ConvertReturnModeW(bFromStream, fTextMode & TEXTMODE_RETURN_MASK, pvDest, pvBuffer, nSize, pnChoppedSize);
	const char* psz = (const char*) pvBuffer;
	char* pszOut = (char*) pvDest;
	ULONG n;
	bool bEndLoop = false;
	BYTE fTextFrom, fTextTo;
	if (bFromStream)
	{
		fTextFrom = TEXTMODE_STREAM_TO_BUFFER_NO_ENCODE(fTextMode);
		fTextTo = fTextMode & TEXTMODE_BUFFER_MASK;
	}
	else
	{
		fTextFrom = fTextMode & TEXTMODE_BUFFER_MASK;
		fTextTo = TEXTMODE_STREAM_TO_BUFFER_NO_ENCODE(fTextMode);
	}
	*pnChoppedSize = 0;
	while (nSize)
	{
		switch (fEncode)
		{
			case TEXTMODE_NONE:
				break;
			case TEXTMODE_UTF8:
				n = (ULONG) GetUTF8Length((LPCBYTE) psz, nSize);
				if (!n)
				{
					*pnChoppedSize = nSize;
					bEndLoop = true;
					break;
				}
				else if (n != 1)
				{
					if (n != (DWORD) -1)
					{
						nSize -= n;
						while (n--)
							*pszOut++ = *psz++;
					}
					else
					{
						nSize--;
						*pszOut++ = *psz++;
					}
					continue;
				}
				break;
			default:
			case TEXTMODE_SHIFT_JIS:
				if (IsShiftJISFirstChar((BYTE) *psz))
				{
					if (nSize == 2)
					{
						*pnChoppedSize = nSize;
						bEndLoop = true;
						break;
					}
					nSize -= 2;
					*pszOut++ = *psz++;
					*pszOut++ = *psz++;
					continue;
				}
				break;
			case TEXTMODE_EUC_JP:
				if (IsEUCFirstChar((BYTE) *psz))
				{
					if (nSize == 2)
					{
						*pnChoppedSize = nSize;
						bEndLoop = true;
						break;
					}
					nSize -= 2;
					*pszOut++ = *psz++;
					*pszOut++ = *psz++;
					continue;
				}
				break;
		}
		if (bEndLoop)
			break;
		if (*psz != '\r' && *psz != '\n')
		{
			nSize--;
			*pszOut++ = *psz++;
			continue;
		}
		if (*psz == '\r' && fTextFrom == TEXTMODE_BUFFER_CRLF && nSize < 2)
		{
			*pnChoppedSize = nSize;
			break;
		}
		if ((*psz == '\r' && fTextFrom == TEXTMODE_BUFFER_CR) ||
			(*psz == '\n' && fTextFrom == TEXTMODE_BUFFER_LF) ||
			(*psz == '\r' && *(psz + 1) == '\n' && fTextFrom == TEXTMODE_BUFFER_CRLF))
		{
			AppendReturnsA(pszOut, fTextTo);
			if (fTextFrom == TEXTMODE_BUFFER_CRLF)
			{
				psz++;
				nSize--;
			}
		}
		else
			*pszOut++ = *psz;
		psz++;
		nSize--;
	}
	return (ULONG) ((ULONG_PTR) pszOut - (ULONG_PTR) pvDest);
}

// return: new buffer size
// NOTE: max-size of pvDest must be at least nSize * 2
static ULONG __stdcall ConvertReturnModeW(bool bFromStream, BYTE fTextMode, void* pvDest, const void* pvBuffer, ULONG nSize, ULONG* pnChoppedSize)
{
	const wchar_t* psz = (const wchar_t*) pvBuffer;
	wchar_t* pszOut = (wchar_t*) pvDest;
	bool bEndLoop = false;
	BYTE fTextFrom, fTextTo;
	if (bFromStream)
	{
		fTextFrom = TEXTMODE_STREAM_TO_BUFFER_NO_ENCODE(fTextMode);
		fTextTo = fTextMode & TEXTMODE_BUFFER_MASK;
	}
	else
	{
		fTextFrom = fTextMode & TEXTMODE_BUFFER_MASK;
		fTextTo = TEXTMODE_STREAM_TO_BUFFER_NO_ENCODE(fTextMode);
	}
	*pnChoppedSize = 0;
	while (nSize)
	{
		if (nSize < sizeof(wchar_t))
		{
			*pnChoppedSize = nSize;
		}
		if (*psz != L'\r' && *psz != L'\n')
		{
			nSize -= sizeof(wchar_t);
			*pszOut++ = *psz++;
			continue;
		}
		if (*psz == L'\r' && fTextFrom == TEXTMODE_BUFFER_CRLF && nSize < 2 * sizeof(wchar_t))
		{
			*pnChoppedSize = nSize;
			break;
		}
		if ((*psz == L'\r' && fTextFrom == TEXTMODE_BUFFER_CR) ||
			(*psz == L'\n' && fTextFrom == TEXTMODE_BUFFER_LF) ||
			(*psz == L'\r' && *(psz + 1) == L'\n' && fTextFrom == TEXTMODE_BUFFER_CRLF))
		{
			AppendReturnsW(pszOut, fTextTo);
			if (fTextFrom == TEXTMODE_BUFFER_CRLF)
			{
				psz++;
				nSize -= sizeof(wchar_t);
			}
		}
		else
			*pszOut++ = *psz;
		psz++;
		nSize -= sizeof(wchar_t);
	}
	return (ULONG) ((ULONG_PTR) pszOut - (ULONG_PTR) pvDest);
}

#define TFS_BUFFER_SIZE  8192

CTextStream::CTextStream(IStream* pStream, BYTE fTextMode)
	: m_pStream(pStream)
	, m_fTextMode(fTextMode)
	, m_bBeforeWrite(false)
{
	m_pStream->AddRef();
}

CTextStream::~CTextStream()
{
	m_pStream->Release();
}

STDMETHODIMP CTextStream::QueryInterface(REFIID riid, void** ppvObject)
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

STDMETHODIMP CTextStream::Read(void* pv, ULONG cb, ULONG* pcbRead)
{
	ULONG nLen, nLen2, nRead;
	void* pvb, * pvb2;
	HRESULT hr;

	if (m_bBeforeWrite)
		m_bufferChopped.Empty();
	m_bBeforeWrite = false;
	nRead = 0;
	nLen = (ULONG) m_buffer.GetLength();
	while (true)
	{
		// nLen にはバッファされているデータサイズが入っている
		if (nLen > 0)
		{
			if (cb <= nLen)
				nLen2 = cb;
			else
				nLen2 = nLen;
			pvb = m_buffer.GetCurrentBufferPermanentAndSkip((size_t) nLen2);
			memcpy(pv, pvb, (size_t) nLen2);
			nRead += nLen2;
			if (cb <= nLen)
				break;
			pv = ((BYTE*) pv) + nLen2;
			cb -= nLen2;
		}
		// pvb: 改行コード変換後のデータを入れる位置
		pvb = m_buffer.AppendToBuffer(NULL, TFS_BUFFER_SIZE * 2);
		if (!pvb)
			return E_OUTOFMEMORY;

		// pvb2: 改行コード変換前のデータを入れる位置
		if (!m_bufferChopped.IsEmpty())
		{
			pvb2 = m_buffer.AppendToBuffer(NULL, TFS_BUFFER_SIZE + m_bufferChopped.GetLength());
			if (!pvb2)
				return E_OUTOFMEMORY;
			memcpy(pvb2, m_bufferChopped, m_bufferChopped.GetLength());
			pvb2 = ((BYTE*) pvb2) + m_bufferChopped.GetLength();
			nLen2 = TFS_BUFFER_SIZE - (ULONG) m_bufferChopped.GetLength();
		}
		else
		{
			pvb2 = m_buffer.AppendToBuffer(NULL, TFS_BUFFER_SIZE);
			if (!pvb2)
			{
				m_bufferChopped.Empty();
				return E_OUTOFMEMORY;
			}
			nLen2 = TFS_BUFFER_SIZE;
		}

		hr = m_pStream->Read(pvb2, nLen2, &nLen);

		if (FAILED(hr))
		{
			m_buffer.Empty();
			m_bufferChopped.Empty();
			return hr;
		}
		if (!nLen)
		{
			m_buffer.Empty();
			if (!m_bufferChopped.IsEmpty())
			{
				nLen = (ULONG) m_bufferChopped.GetLength();
				if (nLen > cb)
					nLen = cb;
				memcpy(pv, m_bufferChopped.GetCurrentBufferPermanentAndSkip((size_t) nLen), (size_t) nLen);
				nRead += nLen;
			}
			break;
		}
		//if (nLen != TFS_BUFFER_SIZE)
		//	m_buffer.DeleteLastData((size_t) (TFS_BUFFER_SIZE - nLen));

		// バッファ再割り当てによる位置変更を調整
		pvb = m_buffer;

		if ((nLen2 = (ULONG) m_bufferChopped.GetLength()) != 0)
		{
			nLen += nLen2;
			pvb2 = ((BYTE*) pvb2) - nLen2;
			m_bufferChopped.Empty();
		}
		ULONG nLenOld = nLen;
		nLen = ConvertReturnMode(true, m_fTextMode, pvb, pvb2, nLen, &nLen2);
		// 未使用のデータ(変換不可能だったデータ)があればキープしておく
		if (nLen2)
		{
			if (!m_bufferChopped.AppendToBuffer(((const BYTE*) pvb2) + nLenOld - nLen2, (size_t) nLen2))
				return E_OUTOFMEMORY;
			//LARGE_INTEGER li = { -((LONGLONG) nLen2) };
			//hr = m_pStream->Seek(li, STREAM_SEEK_CUR, NULL);
			//if (FAILED(hr))
			//	return hr;
		}
		if (!m_buffer.Collapse(nLen))
		{
			m_buffer.Empty();
			m_bufferChopped.Empty();
			return E_FAIL;
		}
	}
	if (pcbRead)
		*pcbRead = nRead;

	return nRead ? S_OK : S_FALSE;
}

STDMETHODIMP CTextStream::Write(const void* pv, ULONG cb, ULONG* pcbWritten)
{
	ULONG nLen, nLen2, nUsedLen, nWritten, nOldChoppedLen;
	void* pvb, * pvb2, * pvbPos;
	HRESULT hr;

	nOldChoppedLen = 0;
	if (!m_bBeforeWrite)
		m_bufferChopped.Empty();
	else if (!m_bufferChopped.IsEmpty())
		nOldChoppedLen = (ULONG) m_bufferChopped.GetLength();
	m_bBeforeWrite = true;
	nWritten = 0;
	while (true)
	{
		m_buffer.Empty();
		// pvb: 実際に書き込むデータ
		pvb = m_buffer.AppendToBuffer(NULL, TFS_BUFFER_SIZE * 2);
		if (!pvb)
			return E_OUTOFMEMORY;
		nLen = TFS_BUFFER_SIZE;
		if (!m_bufferChopped.IsEmpty())
		{
			pvbPos = m_buffer.AppendToBuffer(m_bufferChopped, m_bufferChopped.GetLength());
			if (!pvbPos)
				return E_OUTOFMEMORY;
			nLen2 = (ULONG) m_bufferChopped.GetLength();
			nLen -= nLen2;
			m_bufferChopped.Empty();
		}
		else
			pvbPos = NULL;
		// pvb2: 元のデータ
		pvb2 = m_buffer.AppendToBuffer(NULL, (size_t) nLen);
		if (!pvb2)
			return E_OUTOFMEMORY;

		// バッファ再割り当てによる位置変更を調整
		pvb = m_buffer;
		if (pvbPos)
			pvbPos = ((BYTE*) pvb) + TFS_BUFFER_SIZE * 2;

		if (cb > nLen)
			nUsedLen = nLen;
		else
			nUsedLen = nLen = cb;
		memcpy(pvb2, pv, (size_t) nLen);
		if (pvbPos)
		{
			nLen += nLen2;
			//nUsedLen += nLen2;
			pvb2 = pvbPos;
		}
		nLen = ConvertReturnMode(false, m_fTextMode, pvb, pvb2, nLen, &nLen2);
		if (nLen2 > 0)
		{
			//m_buffer.DeleteLastData((size_t) nLen2);
			if (cb <= TFS_BUFFER_SIZE)
				m_bufferChopped.AppendToBuffer(((const BYTE*) pvb2) + nUsedLen - nLen2, (size_t) nLen2);
			else
				nUsedLen -= nLen2;
		}
		nLen2 = nLen;
		hr = m_pStream->Write(pvb, nLen, &nLen2);
		if (FAILED(hr))
		{
			m_buffer.Empty();
			return HRESULT_FROM_WIN32(::GetLastError());
		}
		if (nLen2 != nLen)
		{
			// サイズ計算のため改行コードを逆変換する
			if (!m_buffer.Collapse((size_t) nLen2))
			{
				m_buffer.Empty();
				return E_FAIL;
			}
			// pvb には変換後のデータが入っている
			pvb = m_buffer;
			pvb2 = m_buffer.AppendToBuffer(NULL, nLen2 * 2);
			nUsedLen = ConvertReturnMode(true, m_fTextMode, pvb2, pvb, nLen2, &nLen);
			nUsedLen -= nOldChoppedLen;
			//nUsedLen += nLen;
			// nLen: 1 なら続行、0 ならループ終了 (ここではループを終わらせる)
			nLen = 0;
		}
		else
		{
			// nLen: 1 なら続行、0 ならループ終了
			if (cb > TFS_BUFFER_SIZE)
				nLen = 1;
			else
				nLen = 0;
		}
		nWritten += nUsedLen;
		if (!nLen)
			break;
		pv = ((const BYTE*) pv) + nUsedLen;
		cb -= nUsedLen;
		nOldChoppedLen = 0;
	}
	if (pcbWritten)
		*pcbWritten = nWritten;

	// Read が呼ばれても問題ないようにバッファを空にする
	m_buffer.Empty();
	return S_OK;
}

// オリジナルの CopyTo ではなく、改行コードを変換できるように Read/Write を使う
STDMETHODIMP CTextStream::CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten)
{
	if (!pstm)
		return STG_E_INVALIDPOINTER;

	HRESULT hr = S_FALSE;
	ULONG ur, uw;
	CExBuffer buf;
	void* pv = buf.AppendToBuffer(NULL, TFS_BUFFER_SIZE);
	if (!pv)
		return E_OUTOFMEMORY;
	while (cb.QuadPart)
	{
		hr = Read(pv, TFS_BUFFER_SIZE, &ur);
		if (hr != S_OK)
		{
			if (SUCCEEDED(hr))
				hr = S_OK;
			break;
		}
		if (pcbRead)
			pcbRead->QuadPart += ur;
		hr = pstm->Write(pv, ur, &uw);
		if (FAILED(hr))
			break;
		if (pcbWritten)
			pcbWritten->QuadPart += uw;
	}
	return hr;
}

STDMETHODIMP CTextStream::Seek(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer)
{
	// ポインタ移動が発生する場合はバッファを空にする
	// (先頭・末端からの移動は常に移動するものと見なす)
	if (dwOrigin != STREAM_SEEK_CUR || liDistanceToMove.QuadPart != 0)
	{
		m_bufferChopped.Empty();
		m_buffer.Empty();
	}
	return m_pStream->Seek(liDistanceToMove, dwOrigin, lpNewFilePointer);
}
