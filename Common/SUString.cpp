/*
 Copyright (C) 2010 Kuri-Applications

 SUString.cpp - implementations of _SecureStringW
 */

#include "StdAfx.h"
#include "SUString.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static size_t __stdcall MakeLengthX(size_t dwLength, BYTE nSeed)
{
	register BYTE n = nSeed;
	while (nSeed--)
	{
		register bool bCarry = ((dwLength & 1) != 0);
		dwLength >>= 1;
		if (bCarry)
			dwLength |= 0x80000000;
	}
	return dwLength - n;
}

static size_t __stdcall GetRealLength(size_t dwLengthX, BYTE nSeed)
{
	dwLengthX += nSeed;
	while (nSeed--)
	{
		register bool bCarry = ((dwLengthX & 0x80000000) != 0);
		dwLengthX <<= 1;
		if (bCarry)
			dwLengthX |= 1;
	}
	return dwLengthX & 0xFFFFFFFF;
}

static size_t __stdcall _GetPositionX(size_t nPos, size_t nMaxSize, BYTE nSeed)
{
	while (nSeed)
	{
		nPos += nSeed;
		while (nPos >= nMaxSize)
			nPos -= nMaxSize;
		nSeed--;
	}
	return nPos;
}

static size_t __stdcall _GetFromPositionX(size_t nPosX, size_t nMaxSize, BYTE nSeed)
{
	while (nSeed)
	{
		while (nPosX < nSeed)
			nPosX += nMaxSize;
		nPosX -= nSeed;
		nSeed--;
	}
	return nPosX;
}

static void __stdcall CopyToBufferX(PVOID pDestX, LPCVOID lpSrc, size_t nSize, BYTE nSeed)
{
	register char* pdstX = (char*) pDestX;
	register const char FAR* psrc = (const char FAR*) lpSrc;
	register size_t nPos;
	for (nPos = 0; nPos < nSize; nPos++)
		pdstX[_GetPositionX(nPos, nSize, nSeed)] = (char) (((BYTE) *psrc++) + nSeed - nPos);
}

static void __stdcall CopyFromBufferX(LPVOID lpDest, const void* pSrcX, size_t nSize, BYTE nSeed)
{
	register char FAR* pdst = (char FAR*) lpDest;
	register const char* psrcX = (const char*) pSrcX;
	register size_t nPos;
	for (nPos = 0; nPos < nSize; nPos++)
		*pdst++ = (char) ((BYTE) psrcX[_GetPositionX(nPos, nSize, nSeed)] - nSeed + nPos);
}

static void __stdcall EmptyBufferX(PVOID pBufferX, size_t nSize, BYTE nSeed)
{
	register char* pbx = (char*) pBufferX;
	register size_t n = nSize;
	while (n--)
		*pbx++ = (char) (BYTE)((DWORD) (rand() * (DWORD)((BYTE) (nSeed + 13)) / RAND_MAX) + nSeed + (n << nSeed));
	n = nSize;
	while (n--)
		*(--pbx) = (char) (BYTE)((DWORD) (rand() * n / RAND_MAX) - nSeed + n);
	SecureZeroMemory(pBufferX, nSize);
}

_SecureStringW::_SecureStringW(void)
	: m_lpszBuffer(NULL)
{
	m_nSeed = (BYTE) (rand() * 128 / RAND_MAX) + 1;
	m_dwLengthX = MakeLengthX(0, m_nSeed);
}

_SecureStringW::_SecureStringW(LPCWSTR lpszString)
	: m_lpszBuffer(NULL)
{
	m_nSeed = (BYTE) (rand() * 128 / RAND_MAX) + 1;
	SetString(lpszString);
}

_SecureStringW::_SecureStringW(const CMyStringW& string)
	: m_lpszBuffer(NULL)
{
	m_nSeed = (BYTE) (rand() * 128 / RAND_MAX) + 1;
	SetString(string);
}

_SecureStringW::_SecureStringW(const _SecureStringW& string)
	: m_lpszBuffer(NULL)
{
	m_nSeed = (BYTE) (rand() * 128 / RAND_MAX) + 1;
	SetString(string);
}

_SecureStringW::~_SecureStringW(void)
{
	Empty();
}

void _SecureStringW::Empty()
{
	if (m_lpszBuffer)
	{
		size_t dwL = GetRealLength(m_dwLengthX, m_nSeed);
		EmptyBufferX(m_lpszBuffer, sizeof(WCHAR) * dwL, m_nSeed);
		free(m_lpszBuffer);
		m_lpszBuffer = NULL;
		m_dwLengthX = MakeLengthX(0, m_nSeed);
	}
}

void __stdcall _SecureStringW::SecureEmptyString(CMyStringW& string)
{
	if (string.IsEmpty())
		return;
	register size_t dwL = string.GetLength();
	register LPWSTR lpw = string.GetBuffer(dwL);
	EmptyBufferX(lpw, sizeof(WCHAR) * dwL, rand() * 255 / RAND_MAX);
	lpw = NULL;
	string.Empty();
}

void __stdcall _SecureStringW::SecureEmptyBuffer(LPVOID lpBuffer, size_t nSize)
{
	if (!nSize)
		return;
	EmptyBufferX(lpBuffer, nSize, rand() * 255 / RAND_MAX);
}

void _SecureStringW::GetString(CMyStringW& string) const
{
	if (IsEmpty())
	{
		string.Empty();
		return;
	}
	register size_t dwL = GetRealLength(m_dwLengthX, m_nSeed);
	register LPWSTR lpw = string.GetBuffer(dwL);
	CopyFromBufferX(lpw, m_lpszBuffer, sizeof(WCHAR) * dwL, m_nSeed);
	string.ReleaseBuffer(dwL);
	lpw = NULL;
}

void _SecureStringW::AppendToString(CMyStringW& string) const
{
	if (IsEmpty())
		return;
	register size_t dwL = GetRealLength(m_dwLengthX, m_nSeed);
	register size_t dwU = string.GetLength();
	register LPWSTR lpw = string.GetBuffer(dwU + dwL);
	CopyFromBufferX(lpw + dwU, m_lpszBuffer, sizeof(WCHAR) * dwL, m_nSeed);
	string.ReleaseBuffer(dwU + dwL);
}

bool _SecureStringW::GetStringFromWindowText(HWND hWnd)
{
	CMyStringW strTemp;
	register DWORD dwL = (DWORD) ::GetWindowTextLengthW(hWnd);
	register DWORD dwE = ::GetLastError();
	if (dwE == ERROR_CALL_NOT_IMPLEMENTED)
	{
		::SetLastError(ERROR_SUCCESS);
		dwL = (DWORD) ::GetWindowTextLengthA(hWnd);
		if (::GetLastError() != ERROR_SUCCESS)
			return false;
		if (!dwL)
		{
			Empty();
			return true;
		}
		LPSTR lp = strTemp.GetBufferA(dwL);
		if (!lp)
			return false;
		dwL = (DWORD) ::GetWindowTextA(hWnd, lp, (int) (dwL + 1));
		lp = NULL;
		strTemp.ReleaseBufferA(TRUE, dwL);
	}
	else if (dwE == ERROR_SUCCESS)
	{
		if (!dwL)
		{
			Empty();
			return true;
		}
		LPWSTR lpw = strTemp.GetBuffer(dwL);
		if (!lpw)
			return false;
		dwL = (DWORD) ::GetWindowTextW(hWnd, lpw, (int) (dwL + 1));
		lpw = NULL;
		strTemp.ReleaseBuffer(dwL);
	}
	else
		return false;
	operator = (strTemp);
	SecureEmptyString(strTemp);
	return true;
}

void _SecureStringW::SetStringToWindowText(HWND hWnd) const
{
	CMyStringW strTemp;
	GetString(strTemp);
	if (!::SetWindowTextW(hWnd, strTemp) && ::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
		::SetWindowTextA(hWnd, strTemp);
	SecureEmptyString(strTemp);
}

void _SecureStringW::SetString(LPCWSTR lpszString)
{
	register DWORD dwL = (DWORD) wcslen(lpszString);
	register size_t nSize = sizeof(WCHAR) * dwL;
	Empty();
	m_lpszBuffer = (LPWSTR) malloc(nSize);
	CopyToBufferX(m_lpszBuffer, lpszString, nSize, m_nSeed);
	m_dwLengthX = MakeLengthX(dwL, m_nSeed);
}

void _SecureStringW::SetString(const CMyStringW& string)
{
	register size_t dwL = string.GetLength();
	register size_t nSize = sizeof(WCHAR) * dwL;
	Empty();
	m_lpszBuffer = (LPWSTR) malloc(nSize);
	CopyToBufferX(m_lpszBuffer, (LPCWSTR) string, nSize, m_nSeed);
	m_dwLengthX = MakeLengthX(dwL, m_nSeed);
}

void _SecureStringW::SetString(const _SecureStringW& string)
{
	register size_t dwL = GetRealLength(string.m_dwLengthX, string.m_nSeed);
	register size_t nSize = sizeof(WCHAR) * dwL;
	Empty();
	register LPWSTR lpw = (LPWSTR) malloc(nSize);
	m_lpszBuffer = (LPWSTR) malloc(nSize);
	CopyFromBufferX(lpw, string.m_lpszBuffer, nSize, string.m_nSeed);
	CopyToBufferX(m_lpszBuffer, lpw, nSize, m_nSeed);
	m_dwLengthX = MakeLengthX(dwL, m_nSeed);
	EmptyBufferX(lpw, nSize, m_nSeed);
	free(lpw);
}

void _SecureStringW::AppendChar(WCHAR wch)
{
	CMyStringW str;
	if (IsEmpty())
	{
		str.AppendChar(wch);
		SetString(str);
		return;
	}
	GetString(str);
	str += wch;
	SetString(str);
	SecureEmptyString(str);
}

void _SecureStringW::AppendString(LPCWSTR lpszString)
{
	if (IsEmpty())
	{
		SetString(lpszString);
		return;
	}
	CMyStringW str;
	GetString(str);
	str += lpszString;
	SetString(str);
	SecureEmptyString(str);
}

void _SecureStringW::AppendString(const CMyStringW& string)
{
	if (IsEmpty())
	{
		SetString(string);
		return;
	}
	CMyStringW str;
	GetString(str);
	str += string;
	SetString(str);
	SecureEmptyString(str);
}

void _SecureStringW::AppendString(const _SecureStringW& string)
{
	if (IsEmpty())
	{
		SetString(string);
		return;
	}
	CMyStringW str;
	GetString(str);
	string.AppendToString(str);
	SetString(str);
	SecureEmptyString(str);
}

_SecureStringW& _SecureStringW::operator = (LPCWSTR lpszString)
{
	SetString(lpszString);
	return *this;
}

_SecureStringW& _SecureStringW::operator = (const CMyStringW& string)
{
	SetString(string);
	return *this;
}

_SecureStringW& _SecureStringW::operator = (const _SecureStringW& string)
{
	SetString(string);
	return *this;
}

_SecureStringW& _SecureStringW::operator += (WCHAR wch)
{
	AppendChar(wch);
	return *this;
}

_SecureStringW& _SecureStringW::operator += (LPCWSTR lpszString)
{
	AppendString(lpszString);
	return *this;
}

_SecureStringW& _SecureStringW::operator += (const CMyStringW& string)
{
	AppendString(string);
	return *this;
}

_SecureStringW& _SecureStringW::operator += (const _SecureStringW& string)
{
	AppendString(string);
	return *this;
}
