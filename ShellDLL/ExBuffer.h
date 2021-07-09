/*
 Copyright (C) 2010 Kuri-Applications

 ExBuffer.h - declarations of CExBuffer
 */

#pragma once

inline DWORD __stdcall ConvertEndian(DWORD dword)
{
	return MAKELONG(MAKEWORD(HIBYTE(HIWORD(dword)), LOBYTE(HIWORD(dword))),
		MAKEWORD(HIBYTE(LOWORD(dword)), LOBYTE(LOWORD(dword))));
}

class CExBuffer
{
public:
	CExBuffer();
	CExBuffer(size_t nInitSize);
	virtual ~CExBuffer();

public:
	operator void* () const;
	CExBuffer& operator = (const CExBuffer& buffer);

	// pvData can be NULL
	void* AppendToBuffer(const void* pvData, size_t nSize);
	void* GetCurrentBuffer(size_t nSize);
	void* GetCurrentBufferAndSkip(size_t nSize);
	void* GetCurrentBufferPermanent(size_t nSize);
	void* GetCurrentBufferPermanentAndSkip(size_t nSize);
	void* SkipPosition(long nPos);
	void* SetPosition(size_t nPos);
	size_t GetPosition() const;
	void ResetPosition();
	size_t GetLength() const;
	bool IsEmpty() const;
	void DeleteForwardData();
	void DeleteLastData(size_t nSizeLast);
	bool Collapse(size_t nNewLen);
	void Empty();
	// pvData cannot be NULL
	bool SetDataToBuffer(const void* pvData, size_t nSize);

	void* AppendToBuffer(BYTE b);
	void* AppendToBuffer(WORD w);
	void* AppendToBuffer(UINT ui);
	void* AppendToBuffer(ULONG ul);
	void* AppendToBuffer(ULONGLONG uli);
	void* AppendToBuffer(LPCSTR lpszString);
	void* AppendToBufferCE(WORD w);
	void* AppendToBufferCE(UINT ui);
	void* AppendToBufferCE(ULONG ul);
	void* AppendToBufferCE(ULONGLONG uli);
	void* AppendToBufferWithLenCE(const void* pvData, DWORD dwSize);
	void* AppendToBufferWithLenCE(const void* pvData, size_t nSize);
	void* AppendToBufferWithLenCE(LPCSTR lpszString);

	bool GetAndSkip(BYTE& b);
	bool GetAndSkip(WORD& w);
	bool GetAndSkip(UINT& ui);
	bool GetAndSkip(ULONG& ul);
	bool GetAndSkip(ULONGLONG& uli);
	bool GetAndSkip(LPSTR& psz, size_t nLen);
	bool GetAndSkipCE(WORD& w);
	bool GetAndSkipCE(UINT& ui);
	bool GetAndSkipCE(ULONG& ul);
	bool GetAndSkipCE(ULONGLONG& uli);

private:
	void* m_pvBuffer;
	size_t m_nPos;
	size_t m_nLen;
	size_t m_nMemSize;

	bool ExpandBuffer(size_t nReqSize);
};

inline CExBuffer::CExBuffer()
	: m_pvBuffer(NULL), m_nPos(0), m_nLen(0), m_nMemSize(0)
	{ }
inline CExBuffer::operator void* () const
	{ return (BYTE*) m_pvBuffer + m_nPos; }
inline void* CExBuffer::SkipPosition(long nPos)
	{ m_nPos += nPos; return (BYTE*) m_pvBuffer + m_nPos; }
inline void* CExBuffer::SetPosition(size_t nPos)
	{ m_nPos = nPos; return (BYTE*) m_pvBuffer + m_nPos; }
inline size_t CExBuffer::GetPosition() const
	{ return m_nPos; }
inline void CExBuffer::ResetPosition()
	{ m_nPos = 0; }
inline size_t CExBuffer::GetLength() const
	{ return m_nLen - m_nPos; }
inline bool CExBuffer::IsEmpty() const
	{ return m_nLen == m_nPos; }
inline bool CExBuffer::Collapse(size_t nNewLen)
{
	if (nNewLen > m_nLen)
		return false;
	m_nLen = nNewLen;
	return true;
}
inline void* CExBuffer::AppendToBuffer(BYTE b)
	{ return AppendToBuffer(&b, sizeof(b)); }
inline void* CExBuffer::AppendToBuffer(WORD w)
	{ return AppendToBuffer(&w, sizeof(w)); }
inline void* CExBuffer::AppendToBuffer(UINT ui)
	{ return AppendToBuffer(&ui, sizeof(ui)); }
inline void* CExBuffer::AppendToBuffer(ULONG ul)
	{ return AppendToBuffer(&ul, sizeof(ul)); }
inline void* CExBuffer::AppendToBuffer(ULONGLONG uli)
	{ return AppendToBuffer(&uli, sizeof(uli)); }
inline void* CExBuffer::AppendToBuffer(LPCSTR lpszString)
	{ return AppendToBuffer(lpszString, strlen(lpszString)); }

inline void* CExBuffer::AppendToBufferCE(WORD w)
{
	register BYTE* pb = (BYTE*) AppendToBuffer(NULL, sizeof(w));
	pb[0] = HIBYTE(w);
	pb[1] = LOBYTE(w);
	return pb;
}
inline void* CExBuffer::AppendToBufferCE(ULONG ul)
{
	register BYTE* pb = (BYTE*) AppendToBuffer(NULL, sizeof(ul));
	pb[0] = HIBYTE(HIWORD(ul));
	pb[1] = LOBYTE(HIWORD(ul));
	pb[2] = HIBYTE(LOWORD(ul));
	pb[3] = LOBYTE(LOWORD(ul));
	return pb;
}
inline void* CExBuffer::AppendToBufferCE(ULONGLONG uli)
{
	register BYTE* pb = (BYTE*) AppendToBuffer(NULL, sizeof(uli));
	register BYTE* pbuli = (BYTE*) &uli;
	pb[0] = pbuli[7];
	pb[1] = pbuli[6];
	pb[2] = pbuli[5];
	pb[3] = pbuli[4];
	pb[4] = pbuli[3];
	pb[5] = pbuli[2];
	pb[6] = pbuli[1];
	pb[7] = pbuli[0];
	return pb;
}
inline void* CExBuffer::AppendToBufferCE(UINT ui)
{
	if (sizeof(UINT) == 2)
		return AppendToBufferCE((WORD) ui);
	else
		return AppendToBufferCE((ULONG) ui);
}
inline void* CExBuffer::AppendToBufferWithLenCE(const void* pv, DWORD dwSize)
{
	register BYTE* pb = (BYTE*) AppendToBuffer(NULL, sizeof(dwSize) + dwSize);
	pb[0] = HIBYTE(HIWORD(dwSize));
	pb[1] = LOBYTE(HIWORD(dwSize));
	pb[2] = HIBYTE(LOWORD(dwSize));
	pb[3] = LOBYTE(LOWORD(dwSize));
	memcpy(pb + 4, pv, (size_t) dwSize);
	return pb;
}
inline void* CExBuffer::AppendToBufferWithLenCE(const void* pv, size_t nSize)
	{ return AppendToBufferWithLenCE(pv, (DWORD) nSize); }
inline void* CExBuffer::AppendToBufferWithLenCE(LPCSTR lpszString)
	{ return AppendToBufferWithLenCE(lpszString, strlen(lpszString)); }

inline bool CExBuffer::GetAndSkip(BYTE& b)
{
	register BYTE* p = (BYTE*) GetCurrentBufferPermanentAndSkip(sizeof(b));
	if (p)
		b = *p;
	return p != NULL;
}
inline bool CExBuffer::GetAndSkip(WORD& w)
{
	register WORD* p = (WORD*) GetCurrentBufferPermanentAndSkip(sizeof(w));
	if (p)
		w = *p;
	return p != NULL;
}
inline bool CExBuffer::GetAndSkip(UINT& ui)
{
	register UINT* p = (UINT*) GetCurrentBufferPermanentAndSkip(sizeof(ui));
	if (p)
		ui = *p;
	return p != NULL;
}
inline bool CExBuffer::GetAndSkip(ULONG& ul)
{
	register ULONG* p = (ULONG*) GetCurrentBufferPermanentAndSkip(sizeof(ul));
	if (p)
		ul = *p;
	return p != NULL;
}
inline bool CExBuffer::GetAndSkip(ULONGLONG& uli)
{
	register ULONGLONG* p = (ULONGLONG*) GetCurrentBufferPermanentAndSkip(sizeof(uli));
	if (p)
		uli = *p;
	return p != NULL;
}
inline bool CExBuffer::GetAndSkipCE(WORD& w)
{
	register BYTE* p = (BYTE*) GetCurrentBufferPermanentAndSkip(sizeof(w));
	if (p)
		w = MAKEWORD(p[1], p[0]);
	return p != NULL;
}
inline bool CExBuffer::GetAndSkipCE(ULONG& ul)
{
	register BYTE* p = (BYTE*) GetCurrentBufferPermanentAndSkip(sizeof(ul));
	if (p)
		ul = MAKELONG(MAKEWORD(p[3], p[2]), MAKEWORD(p[1], p[0]));
	return p != NULL;
}
inline bool CExBuffer::GetAndSkipCE(ULONGLONG& uli)
{
	register BYTE* p = (BYTE*) GetCurrentBufferPermanentAndSkip(sizeof(uli));
	register BYTE* pbuli = (BYTE*) &uli;
	if (p)
	{
		pbuli[0] = p[7];
		pbuli[1] = p[6];
		pbuli[2] = p[5];
		pbuli[3] = p[4];
		pbuli[4] = p[3];
		pbuli[5] = p[2];
		pbuli[6] = p[1];
		pbuli[7] = p[0];
	}
	return p != NULL;
}
inline bool CExBuffer::GetAndSkipCE(UINT& ui)
{
	if (sizeof(UINT) == 2)
		return GetAndSkipCE((WORD&) ui);
	else
		return GetAndSkipCE((ULONG&) ui);
}
