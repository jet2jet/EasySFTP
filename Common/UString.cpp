/*
 Copyright (C) 2011 Kuri-Applications

 UString.cpp - implementations of CMyStringW
 */

#include "stdafx.h"
#include "UString.h"

#include <stdarg.h>
#include "Convert.h"

#ifndef _MSC_VER
#define _ismbblead(c)    (0)
#define _vsnprintf       vsnprintf
#define _vsnwprintf      vswprintf
#endif

#define _fmalloc     malloc
#define _frealloc    realloc
#define _ffree       free
#define _fmemcpy     memcpy
#define _fmemmove    memmove

#ifdef _WIN32
// in unicode.cpp
LPWSTR __stdcall UniLoadStringW(HINSTANCE hInstance, UINT uID);
#endif

static size_t mystrnlen(const char* sz, size_t nMaxLen)
{
	size_t n;
	n = 0;
	while (*sz)
	{
		if (!nMaxLen)
			break;
		nMaxLen--;
		sz++;
	}
	return n;
}

static size_t mywcsnlen(const wchar_t* sz, size_t nMaxLen)
{
	size_t n;
	n = 0;
	while (*sz)
	{
		if (!nMaxLen)
			break;
		nMaxLen--;
		sz++;
	}
	return n;
}

#define MBSWCS_BUFF_LEN  1024

static size_t _mbsntowcs(wchar_t* wszOut, size_t nwszBuff, const char* szIn, size_t nszLen)
{
	size_t nLen, nszLenAct, r;
	char* pbuff;

	nLen = 0;
	nszLenAct = mystrnlen(szIn, nszLen);
	while (nszLenAct < nszLen)
	{
		r = mbstowcs(wszOut, szIn, nwszBuff);
		if (r == (size_t) -1)
			return r;
		wszOut += r;
		szIn += nszLenAct;
		nwszBuff -= r;
		nszLen -= nszLenAct;
		nLen += r;
		if (nwszBuff)
		{
			*wszOut++ = 0;
			nLen++;
			szIn++;
			nszLen--;
		}
		if (!nwszBuff)
			return nLen;
		nszLenAct = mystrnlen(szIn, nszLen);
	}

	r = min(MBSWCS_BUFF_LEN, nszLen) + 1;
	pbuff = (char*) malloc(sizeof(char) * r);
	if (!pbuff)
		return (size_t) -1;
	while (nszLen >= MBSWCS_BUFF_LEN)
	{
		memcpy(pbuff, szIn, sizeof(char) * MBSWCS_BUFF_LEN);
		pbuff[MBSWCS_BUFF_LEN] = 0;
		r = mbstowcs(wszOut, pbuff, nwszBuff);
		if (r == (size_t) -1)
		{
			nLen = r;
			nwszBuff = 0;
			break;
		}
		nLen += r;
		wszOut += r;
		nwszBuff -= r;
		szIn += MBSWCS_BUFF_LEN;
		nszLen -= MBSWCS_BUFF_LEN;
		if (!nwszBuff)
			break;
	}
	if (nwszBuff)
	{
		memcpy(pbuff, szIn, sizeof(char) * nszLen);
		pbuff[nszLen] = 0;
		r = mbstowcs(wszOut, pbuff, nwszBuff);
		if (r == (size_t) -1)
			nLen = r;
		else
			nLen += r;
	}
	free(pbuff);
	return nLen;
}

static size_t _wcsntombs(char* szOut, size_t nszBuff, const wchar_t* wszIn, size_t nwszLen)
{
	size_t nLen, nwszLenAct, r;
	wchar_t* pbuff;

	nLen = 0;
	nwszLenAct = mywcsnlen(wszIn, nwszLen);
	while (nwszLenAct < nwszLen)
	{
		r = wcstombs(szOut, wszIn, nszBuff);
		if (r == (size_t) -1)
			return r;
		szOut += r;
		wszIn += nwszLenAct;
		nszBuff -= r;
		nwszLen -= nwszLenAct;
		nLen += r;
		if (nszBuff)
		{
			*szOut++ = 0;
			nLen++;
			wszIn++;
			nwszLen--;
		}
		if (!nszBuff)
			return nLen;
		nwszLenAct = mywcsnlen(wszIn, nwszLen);
	}

	r = min(MBSWCS_BUFF_LEN, nwszLen) + 1;
	pbuff = (wchar_t*) malloc(sizeof(wchar_t) * r);
	if (!pbuff)
		return (size_t) -1;
	while (nwszLen >= MBSWCS_BUFF_LEN)
	{
		memcpy(pbuff, wszIn, sizeof(char) * MBSWCS_BUFF_LEN);
		pbuff[MBSWCS_BUFF_LEN] = 0;
		r = wcstombs(szOut, pbuff, nszBuff);
		if (r == (size_t) -1)
		{
			nLen = r;
			nszBuff = 0;
			break;
		}
		nLen += r;
		szOut += r;
		nszBuff -= r;
		wszIn += MBSWCS_BUFF_LEN;
		nwszLen -= MBSWCS_BUFF_LEN;
		if (!nszBuff)
			break;
	}
	if (nszBuff)
	{
		memcpy(pbuff, wszIn, sizeof(wchar_t) * nwszLen);
		pbuff[nwszLen] = 0;
		r = wcstombs(szOut, pbuff, nszBuff);
		if (r == (size_t) -1)
			nLen = r;
		else
			nLen += r;
	}
	free(pbuff);
	return nLen;
}

///////////////////////////////////////////////////////////////////////////////
// support 64-bit length

static size_t __stdcall _MBSToWCS(UINT uCodePage, LPCSTR lpszString, size_t nLen, LPWSTR lpszOutput, size_t nBufferLen)
{
	size_t nRet = 0;
	if (!lpszOutput || !nBufferLen)
	{
		if (nLen == (size_t) -1)
		{
			nLen = strlen(lpszString);
			nRet = 1;
		}
		while (nLen >= 0x80000000)
		{
			int len;
#ifdef _WIN32
			if (::IsDBCSLeadByteEx(uCodePage, (BYTE) lpszString[0x7FFFFFFE]) /*&&
				!::IsDBCSLeadByteEx(uCodePage, (BYTE) lpszString[0x7FFFFFFD])*/)
#else
			if (_ismbblead((unsigned int) lpszString[0x7FFFFFFE]) /*&&
				!_ismbblead((unsigned int) lpszString[0x7FFFFFFD])*/)
#endif
				len = 0x7FFFFFFE;
			else
				len = 0x7FFFFFFF;
#ifdef _WIN32
			nRet += ::MultiByteToWideChar(uCodePage, 0, lpszString, len, NULL, 0);
#else
			nRet += mbstowcs(NULL, lpszString, (size_t) len);
#endif
			lpszString += len;
			nLen -= len;
		}
#ifdef _WIN32
		nRet += ::MultiByteToWideChar(uCodePage, 0, lpszString, (int) nLen, NULL, 0);
#else
		nRet += mbstowcs(NULL, lpszString, nLen);
#endif
	}
	else
	{
		int ret;
		if (nLen == (size_t) -1)
			nLen = strlen(lpszString) + 1;
		while (nLen >= 0x80000000)
		{
			int len;
#ifdef _WIN32
			if (::IsDBCSLeadByteEx(uCodePage, (BYTE) lpszString[0x7FFFFFFE]) /*&&
				!::IsDBCSLeadByteEx(uCodePage, (BYTE) lpszString[0x7FFFFFFD])*/)
#else
			if (_ismbblead((unsigned int) lpszString[0x7FFFFFFE]) /*&&
				!_ismbblead((unsigned int) lpszString[0x7FFFFFFD])*/)
#endif
				len = 0x7FFFFFFE;
			else
				len = 0x7FFFFFFF;
#ifdef _WIN32
			ret = ::MultiByteToWideChar(uCodePage, 0, lpszString, len, lpszOutput,
				nBufferLen >= 0x80000000 ? 0x7FFFFFFF : (int) nBufferLen);
			if (ret == 0 && ::GetLastError() != 0)
#else
			ret = (int) _mbsntowcs(lpszOutput, nBufferLen >= 0x80000000 ? 0x7FFFFFFF : (int) nBufferLen,
				lpszString, len);
			if (ret == -1)
#endif
				return 0;
			nRet += ret;
			lpszString += len;
			nLen -= len;
			lpszOutput += ret;
			nBufferLen -= ret;
		}
#ifdef _WIN32
		ret = ::MultiByteToWideChar(uCodePage, 0, lpszString, (int) nLen, lpszOutput,
			nBufferLen >= 0x80000000 ? 0x7FFFFFFF : (int) nBufferLen);
		if (ret == 0 && ::GetLastError() != 0)
#else
		ret = (int) _mbsntowcs(lpszOutput, nBufferLen >= 0x80000000 ? 0x7FFFFFFF : (int) nBufferLen,
			lpszString, nLen);
		if (ret == -1)
#endif
			return 0;
		nRet += ret;
	}
	return nRet;
}

static size_t __stdcall _WCSToMBS(UINT uCodePage, LPCWSTR lpszString, size_t nLen, LPSTR lpszOutput, size_t nBufferLen)
{
	size_t nRet = 0;
	if (!lpszOutput || !nBufferLen)
	{
		if (nLen == (size_t) -1)
		{
			nLen = wcslen(lpszString);
			nRet = 1;
		}
		while (nLen >= 0x40000000)
		{
#ifdef _WIN32
			nRet += ::WideCharToMultiByte(uCodePage, 0, lpszString, 0x3FFFFFFF, NULL, 0, NULL, NULL);
#else
			nRet += wcstombs(NULL, lpszString, 0x3FFFFFFF);
#endif
			lpszString += 0x3FFFFFFF;
			nLen -= 0x3FFFFFFF;
		}
#ifdef _WIN32
		nRet += ::WideCharToMultiByte(uCodePage, 0, lpszString, (int) nLen, NULL, 0, NULL, NULL);
#else
		nRet += wcstombs(NULL, lpszString, nLen);
#endif
	}
	else
	{
		int ret;
		if (nLen == (size_t) -1)
			nLen = wcslen(lpszString) + 1;
		while (nLen >= 0x3FFFFFFF)
		{
#ifdef _WIN32
			ret = ::WideCharToMultiByte(uCodePage, 0, lpszString, 0x3FFFFFFE, lpszOutput,
				nBufferLen >= 0x80000000 ? 0x7FFFFFFF : (int) nBufferLen, NULL, NULL);
			if (ret == 0 && ::GetLastError() != 0)
#else
			ret = (int) _wcsntombs(lpszOutput, nBufferLen >= 0x80000000 ? 0x7FFFFFFF : (int) nBufferLen,
				lpszString, 0x3FFFFFFE);
			if (ret == -1)
#endif
				return 0;
			nRet += ret;
			lpszString += 0x3FFFFFFE;
			nLen -= 0x3FFFFFFE;
			lpszOutput += ret;
			nBufferLen -= ret;
		}
#ifdef _WIN32
		ret = ::WideCharToMultiByte(uCodePage, 0, lpszString, (int) nLen, lpszOutput,
			nBufferLen >= 0x40000000 ? 0x3FFFFFFF : (int) nBufferLen, NULL, NULL);
		if (ret == 0 && ::GetLastError() != 0)
#else
		ret = (int) _wcsntombs(lpszOutput, nBufferLen >= 0x80000000 ? 0x7FFFFFFF : (int) nBufferLen,
			lpszString, nLen);
		if (ret == -1)
#endif
			return 0;
		nRet += ret;
	}
	return nRet;
}

////////////////////////////////////////////////////////////////////////////////

size_t WINAPI _AnsiLen(LPCSTR lpszString)
{
	if (!lpszString)
		return 0;
	size_t n = 0;
	while (*lpszString++)
		n++;
	return n;
}

size_t WINAPI _UnicodeLen(LPCWSTR lpszString)
{
	if (!lpszString)
		return 0;
	size_t n = 0;
	while (*lpszString++)
		n++;
	return n;
}

#ifdef ALIGNMENT_MACHINE
size_t WINAPI _UnicodeLen(LPCUWSTR lpszString)
{
	if (!lpszString)
		return 0;
	size_t n = 0;
	while (*lpszString++)
		n++;
	return n;
}
#endif

size_t WINAPI _AnsiLen(LPCWSTR lpszString, size_t uLength, UINT uCodePage = CP_ACP)
{
	size_t n;
	if (!lpszString)
		return 0;
	if (uLength == (size_t) -1)
		n = _WCSToMBS(uCodePage, lpszString, (size_t) -1, NULL, 0) - 1;
	else
		n = _WCSToMBS(uCodePage, lpszString, uLength, NULL, 0);
	return n;
}

#ifdef ALIGNMENT_MACHINE
size_t WINAPI _AnsiLen(LPCUWSTR lpszString, size_t uLength, UINT uCodePage = CP_ACP)
{
	size_t n;
	if (!lpszString)
		return 0;
	LPWSTR lpw;
	if (uLength == (size_t) -1)
		uLength = _UnicodeLen(lpszString);
	lpw = (LPWSTR) malloc(sizeof(WCHAR) * uLength);
	memcpy(lpw, lpszString, sizeof(WCHAR) * uLength);
	n = _WCSToMBS(uCodePage, lpw, uLength, NULL, 0);
	free(lpw);
	return n;
}
#endif

size_t WINAPI _UnicodeLen(LPCSTR lpszString, size_t uLength, UINT uCodePage = CP_ACP)
{
	size_t n;
	if (!lpszString)
		return 0;
	if (uLength == (size_t) -1)
		n = _MBSToWCS(uCodePage, lpszString, (size_t) -1, NULL, 0) - 1;
	else
		n = _MBSToWCS(uCodePage, lpszString, uLength, NULL, 0);
	return n;
}

size_t WINAPI _AnsiFromUnicode(LPSTR lpszBuffer, size_t nBufLen, LPCWSTR lpszString, size_t nLen, UINT uCodePage = CP_ACP)
{
	return _WCSToMBS(uCodePage, lpszString, nLen, lpszBuffer, nBufLen);
}

#ifdef ALIGNMENT_MACHINE
size_t WINAPI _AnsiFromUnicode(LPSTR lpszBuffer, size_t nBufLen, LPCUWSTR lpszString, size_t nLen, UINT uCodePage = CP_ACP)
{
	LPWSTR lpw;
	lpw = (LPWSTR) malloc(sizeof(WCHAR) * nLen);
	memcpy(lpw, lpszString, sizeof(WCHAR) * nLen);
	size_t n = _WCSToMBS(uCodePage, lpw, nLen, lpszBuffer, nBufLen);
	free(lpw);
	return n;
}
#endif

size_t WINAPI _UnicodeFromAnsi(LPWSTR lpszBuffer, size_t nBufLen, LPCSTR lpszString, size_t nLen, UINT uCodePage = CP_ACP)
{
	return _MBSToWCS(uCodePage, lpszString, nLen, lpszBuffer, nBufLen);
}

LPSTR WINAPI _AnsiFromUnicode2(LPCWSTR lpszString, size_t nLen, size_t* puLen = NULL, UINT uCodePage = CP_ACP)
{
	size_t uLen = _AnsiLen(lpszString, nLen) + 1;
	LPSTR lpstr = (LPSTR) ::_fmalloc(sizeof(CHAR) * uLen);
	_AnsiFromUnicode(lpstr, uLen, lpszString, nLen, uCodePage);
	if (puLen)
		*puLen = uLen - 1;
	lpstr[uLen - 1] = 0;
	return lpstr;
}

#ifdef ALIGNMENT_MACHINE
LPSTR WINAPI _AnsiFromUnicode2(LPCUWSTR lpszString, size_t nLen, size_t* puLen = NULL, UINT uCodePage = CP_ACP)
{
	size_t uLen = _AnsiLen(lpszString, nLen) + 1;
	LPSTR lpstr = (LPSTR) ::_fmalloc(sizeof(CHAR) * uLen);
	_AnsiFromUnicode(lpstr, uLen, lpszString, nLen, uCodePage);
	if (puLen)
		*puLen = uLen - 1;
	lpstr[uLen - 1] = 0;
	return lpstr;
}
#endif

LPWSTR WINAPI _UnicodeFromAnsi2(LPCSTR lpszString, size_t nLen, size_t* puLen = NULL, UINT uCodePage = CP_ACP)
{
	size_t uLen = _UnicodeLen(lpszString, nLen) + 1;
	LPWSTR lpwstr = (LPWSTR) ::_fmalloc(sizeof(WCHAR) * uLen);
	_UnicodeFromAnsi(lpwstr, uLen, lpszString, nLen, uCodePage);
	if (puLen)
		*puLen = uLen - 1;
	lpwstr[uLen - 1] = 0;
	return lpwstr;
}

///////////////////////////////////////////////////////////////////////////////

#define STRING_BUFFERW_SIGNATURE  0x4fa872eu
typedef struct _StringBufferW
{
	DWORD dwSig;
	size_t uLength;
	size_t uMaxLength;
	LPWSTR data() const
		{ return (LPWSTR) (this + 1); }
	bool IsValidSignature() const
		{ return dwSig == STRING_BUFFERW_SIGNATURE; }
} STRING_BUFFERW, FAR* LPSTRING_BUFFERW;

typedef struct _StringTempBufferA
{
	DWORD dwSig;
	size_t uLength;
	size_t uMaxLength;
	LPSTR data() const
		{ return (LPSTR) (this + 1); }
	bool IsValidSignature() const
		{ return dwSig == STRING_BUFFERW_SIGNATURE; }
} STRING_TEMP_BUFFERA, FAR* LPSTRING_TEMP_BUFFERA;

LPWSTR WINAPI AllocStringBuffer(LPWSTR lpszBuffer)
{
	if (!lpszBuffer)
		return NULL;
	LPSTRING_BUFFERW pbufBase = ((LPSTRING_BUFFERW) lpszBuffer) - 1;
	if (!pbufBase->IsValidSignature())
		return NULL;
	LPSTRING_BUFFERW pbuf = (LPSTRING_BUFFERW) _fmalloc(sizeof(STRING_BUFFERW) + sizeof(wchar_t) * (pbufBase->uLength + 1));
	if (!pbuf)
		return NULL;
	pbuf->dwSig = STRING_BUFFERW_SIGNATURE;
	pbuf->uLength = pbuf->uMaxLength = pbufBase->uLength;
	_fmemcpy(pbuf->data(), lpszBuffer, (pbufBase->uLength + 1) * sizeof(wchar_t));
	return pbuf->data();
}

LPWSTR WINAPI AllocString(LPCWSTR lpszString)
{
	size_t uLength = _UnicodeLen(lpszString);
	LPSTRING_BUFFERW pbuf = (LPSTRING_BUFFERW) _fmalloc(sizeof(STRING_BUFFERW) + sizeof(wchar_t) * (uLength + 1));
	if (!pbuf)
		return NULL;
	pbuf->dwSig = STRING_BUFFERW_SIGNATURE;
	pbuf->uLength = pbuf->uMaxLength = uLength;
	_fmemcpy(pbuf->data(), lpszString, uLength * sizeof(wchar_t));
	pbuf->data()[uLength] = 0;
	return pbuf->data();
}

#ifdef ALIGNMENT_MACHINE
LPWSTR WINAPI AllocString(LPCUWSTR lpszString)
{
	size_t uLength = _UnicodeLen(lpszString);
	LPSTRING_BUFFERW pbuf = (LPSTRING_BUFFERW) _fmalloc(sizeof(STRING_BUFFERW) + sizeof(wchar_t) * (uLength + 1));
	if (!pbuf)
		return NULL;
	pbuf->dwSig = STRING_BUFFERW_SIGNATURE;
	pbuf->uLength = pbuf->uMaxLength = uLength;
	_fmemcpy(pbuf->data(), lpszString, uLength * sizeof(wchar_t));
	pbuf->data()[uLength] = 0;
	return pbuf->data();
}
#endif

LPWSTR WINAPI ReAllocString(LPWSTR lpBuf, LPCWSTR lpszString)
{
	size_t uLength = _UnicodeLen(lpszString);
	LPSTRING_BUFFERW pbuf = ((LPSTRING_BUFFERW) lpBuf) - 1;
	if (!pbuf->IsValidSignature())
		return NULL;
	if (pbuf->uMaxLength < uLength)
	{
		pbuf = (LPSTRING_BUFFERW) _frealloc(pbuf, sizeof(STRING_BUFFERW) + sizeof(wchar_t) * (uLength + 1));
		if (!pbuf)
			return NULL;
		pbuf->uMaxLength = uLength;
	}
	pbuf->uLength = uLength;
	_fmemcpy(pbuf->data(), lpszString, uLength * sizeof(wchar_t));
	pbuf->data()[uLength] = 0;
	return pbuf->data();
}

#ifdef ALIGNMENT_MACHINE
LPWSTR WINAPI ReAllocString(LPWSTR lpBuf, LPCUWSTR lpszString)
{
	size_t uLength = _UnicodeLen(lpszString);
	LPSTRING_BUFFERW pbuf = ((LPSTRING_BUFFERW) lpBuf) - 1;
	if (!pbuf->IsValidSignature())
		return NULL;
	if (pbuf->uMaxLength < uLength)
	{
		pbuf = (LPSTRING_BUFFERW) _frealloc(pbuf, sizeof(STRING_BUFFERW) + sizeof(wchar_t) * (uLength + 1));
		if (!pbuf)
			return NULL;
		pbuf->uMaxLength = uLength;
	}
	pbuf->uLength = uLength;
	_fmemcpy(pbuf->data(), lpszString, uLength * sizeof(wchar_t));
	pbuf->data()[uLength] = 0;
	return pbuf->data();
}
#endif

LPWSTR WINAPI AllocStringLen(LPCWSTR lpszString, size_t nLen)
{
	size_t uLength = nLen;
	if (uLength == (size_t) -1)
		uLength = _UnicodeLen(lpszString);
	LPSTRING_BUFFERW pbuf = (LPSTRING_BUFFERW) _fmalloc(sizeof(STRING_BUFFERW) + sizeof(wchar_t) * (uLength + 1));
	if (!pbuf)
		return NULL;
	pbuf->dwSig = STRING_BUFFERW_SIGNATURE;
	pbuf->uLength = pbuf->uMaxLength = uLength;
	if (lpszString)
		_fmemcpy(pbuf->data(), lpszString, uLength * sizeof(wchar_t));
	else
		pbuf->data()[0] = 0;
	pbuf->data()[uLength] = 0;
	return pbuf->data();
}

#ifdef ALIGNMENT_MACHINE
LPWSTR WINAPI AllocStringLen(LPCUWSTR lpszString, size_t nLen)
{
	size_t uLength = nLen;
	if (uLength == (size_t) -1)
		uLength = _UnicodeLen(lpszString);
	LPSTRING_BUFFERW pbuf = (LPSTRING_BUFFERW) _fmalloc(sizeof(STRING_BUFFERW) + sizeof(wchar_t) * (uLength + 1));
	if (!pbuf)
		return NULL;
	pbuf->dwSig = STRING_BUFFERW_SIGNATURE;
	pbuf->uLength = pbuf->uMaxLength = uLength;
	if (lpszString)
		_fmemcpy(pbuf->data(), lpszString, uLength * sizeof(wchar_t));
	else
		pbuf->data()[0] = 0;
	pbuf->data()[uLength] = 0;
	return pbuf->data();
}
#endif

LPWSTR WINAPI ReAllocStringLen(LPWSTR lpBuf, LPCWSTR lpszString, size_t nLen)
{
	size_t uLength = nLen;
	if (uLength == (size_t) -1)
		uLength = _UnicodeLen(lpszString);
	LPSTRING_BUFFERW pbuf = ((LPSTRING_BUFFERW) lpBuf) - 1;
	if (!pbuf->IsValidSignature())
		return NULL;
	if (pbuf->uMaxLength < uLength)
	{
		pbuf = (LPSTRING_BUFFERW) _frealloc(pbuf, sizeof(STRING_BUFFERW) + sizeof(wchar_t) * (uLength + 1));
		if (!pbuf)
			return NULL;
		pbuf->uMaxLength = uLength;
	}
	pbuf->uLength = uLength;
	_fmemcpy(pbuf->data(), lpszString, uLength* sizeof(wchar_t));
	pbuf->data()[uLength] = 0;
	return pbuf->data();
}

#ifdef ALIGNMENT_MACHINE
LPWSTR WINAPI ReAllocStringLen(LPWSTR lpBuf, LPCUWSTR lpszString, size_t nLen)
{
	size_t uLength = nLen;
	if (uLength == (size_t) -1)
		uLength = _UnicodeLen(lpszString);
	LPSTRING_BUFFERW pbuf = ((LPSTRING_BUFFERW) lpBuf) - 1;
	if (!pbuf->IsValidSignature())
		return NULL;
	if (pbuf->uMaxLength < uLength)
	{
		pbuf = (LPSTRING_BUFFERW) _frealloc(pbuf, sizeof(STRING_BUFFERW) + sizeof(wchar_t) * (uLength + 1));
		if (!pbuf)
			return NULL;
		pbuf->uMaxLength = uLength;
	}
	pbuf->uLength = uLength;
	_fmemcpy(pbuf->data(), lpszString, uLength* sizeof(wchar_t));
	pbuf->data()[uLength] = 0;
	return pbuf->data();
}
#endif

LPWSTR WINAPI AllocStringA(LPCSTR lpszString, UINT uCodePage = CP_ACP)
{
	size_t uAnsiLength = _AnsiLen(lpszString);
	size_t uUniLength = _UnicodeLen(lpszString, uAnsiLength) + 1;
	LPSTRING_BUFFERW pbuf = (LPSTRING_BUFFERW) _fmalloc(sizeof(STRING_BUFFERW) + sizeof(wchar_t) * uUniLength);
	if (!pbuf)
		return NULL;
	pbuf->dwSig = STRING_BUFFERW_SIGNATURE;
	pbuf->uLength = pbuf->uMaxLength = uUniLength - 1;
	_UnicodeFromAnsi(pbuf->data(), uUniLength, lpszString, uAnsiLength, uCodePage);
	pbuf->data()[uUniLength - 1] = 0;
	return pbuf->data();
}

LPWSTR WINAPI ReAllocStringA(LPWSTR lpBuf, LPCSTR lpszString, UINT uCodePage = CP_ACP)
{
	size_t uAnsiLength = _AnsiLen(lpszString);
	size_t uUniLength = _UnicodeLen(lpszString, uAnsiLength) + 1;
	LPSTRING_BUFFERW pbuf = ((LPSTRING_BUFFERW) lpBuf) - 1;
	if (!pbuf->IsValidSignature())
		return NULL;
	if (pbuf->uMaxLength < uUniLength - 1)
	{
		pbuf = (LPSTRING_BUFFERW) _frealloc(pbuf, sizeof(STRING_BUFFERW) + sizeof(wchar_t) * uUniLength);
		if (!pbuf)
			return NULL;
		pbuf->uMaxLength = uUniLength - 1;
	}
	pbuf->uLength = uUniLength - 1;
	_UnicodeFromAnsi(pbuf->data(), uUniLength, lpszString, uAnsiLength, uCodePage);
	pbuf->data()[uUniLength - 1] = 0;
	return pbuf->data();
}

LPWSTR WINAPI AllocStringLenA(LPCSTR lpszString, size_t nLen, UINT uCodePage = CP_ACP)
{
	size_t uAnsiLength = nLen;
	if (uAnsiLength == (size_t) -1)
		uAnsiLength = _AnsiLen(lpszString);
	size_t uUniLength = _UnicodeLen(lpszString, uAnsiLength) + 1;
	LPSTRING_BUFFERW pbuf = (LPSTRING_BUFFERW) _fmalloc(sizeof(STRING_BUFFERW) + sizeof(wchar_t) * uUniLength);
	if (!pbuf)
		return NULL;
	LPWSTR lp = pbuf->data();
	pbuf->dwSig = STRING_BUFFERW_SIGNATURE;
	pbuf->uLength = pbuf->uMaxLength = uUniLength - 1;
	_UnicodeFromAnsi(lp, uUniLength, lpszString, uAnsiLength, uCodePage);
	lp[uUniLength - 1] = 0;
	return lp;
}

LPWSTR WINAPI ReAllocStringLenA(LPWSTR lpBuf, LPCSTR lpszString, size_t nLen, UINT uCodePage = CP_ACP)
{
	size_t uAnsiLength = nLen;
	if (uAnsiLength == (size_t) -1)
		uAnsiLength = _AnsiLen(lpszString);
	size_t uUniLength = _UnicodeLen(lpszString, uAnsiLength) + 1;
	LPSTRING_BUFFERW pbuf = ((LPSTRING_BUFFERW) lpBuf) - 1;
	if (!pbuf->IsValidSignature())
		return NULL;
	if (pbuf->uMaxLength < uUniLength - 1)
	{
		pbuf = (LPSTRING_BUFFERW) _frealloc(pbuf, sizeof(STRING_BUFFERW) + sizeof(wchar_t) * uUniLength);
		if (!pbuf)
			return NULL;
		pbuf->uMaxLength = uUniLength - 1;
	}
	LPWSTR lp = pbuf->data();
	pbuf->uLength = uUniLength - 1;
	_UnicodeFromAnsi(lp, uUniLength, lpszString, uAnsiLength, uCodePage);
	lp[uUniLength - 1] = 0;
	return lp;
}

size_t WINAPI GetStringLen(LPWSTR lpBuf)
{
	LPSTRING_BUFFERW pbuf = ((LPSTRING_BUFFERW) lpBuf) - 1;
	if (!pbuf->IsValidSignature())
		return 0;
	return pbuf->uLength;
}

LPWSTR WINAPI JoinString(LPWSTR lpbuf, LPCWSTR lpszString)
{
	if (!lpbuf)
		return AllocString(lpszString);
	LPSTRING_BUFFERW pbuf = ((LPSTRING_BUFFERW) lpbuf) - 1;
	if (!pbuf->IsValidSignature())
		return NULL;
	size_t uOldLength = pbuf->uLength;
	size_t uLength = _UnicodeLen(lpszString);
	size_t uNewLength = pbuf->uLength + uLength + 1;
	if (pbuf->uMaxLength < uNewLength - 1)
	{
		pbuf = (LPSTRING_BUFFERW) _frealloc(pbuf, sizeof(STRING_BUFFERW) + sizeof(wchar_t) * uNewLength);
		if (!pbuf)
			return NULL;
		pbuf->uMaxLength = uNewLength - 1;
	}
	pbuf->uLength = uNewLength - 1;
	_fmemcpy(pbuf->data() + uOldLength, lpszString, uLength * sizeof(wchar_t));
	pbuf->data()[uNewLength - 1] = 0;
	return pbuf->data();
}

#ifdef ALIGNMENT_MACHINE
LPWSTR WINAPI JoinString(LPWSTR lpbuf, LPCUWSTR lpszString)
{
	if (!lpbuf)
		return AllocString(lpszString);
	LPSTRING_BUFFERW pbuf = ((LPSTRING_BUFFERW) lpbuf) - 1;
	if (!pbuf->IsValidSignature())
		return NULL;
	size_t uOldLength = pbuf->uLength;
	size_t uLength = _UnicodeLen(lpszString);
	size_t uNewLength = pbuf->uLength + uLength + 1;
	if (pbuf->uMaxLength < uNewLength - 1)
	{
		pbuf = (LPSTRING_BUFFERW) _frealloc(pbuf, sizeof(STRING_BUFFERW) + sizeof(wchar_t) * uNewLength);
		if (!pbuf)
			return NULL;
		pbuf->uMaxLength = uNewLength - 1;
	}
	pbuf->uLength = uNewLength - 1;
	_fmemcpy(pbuf->data() + uOldLength, lpszString, uLength * sizeof(wchar_t));
	pbuf->data()[uNewLength - 1] = 0;
	return pbuf->data();
}
#endif

LPWSTR WINAPI JoinStringLen(LPWSTR lpbuf, LPCWSTR lpszString, size_t uLen)
{
	if (!lpbuf)
		return AllocStringLen(lpszString, uLen);
	if (uLen == (size_t) -1)
		uLen = _UnicodeLen(lpszString);
	LPSTRING_BUFFERW pbuf = ((LPSTRING_BUFFERW) lpbuf) - 1;
	if (!pbuf->IsValidSignature())
		return NULL;
	size_t uOldLength = pbuf->uLength;
	size_t uLength = uLen;
	size_t uNewLength = pbuf->uLength + uLength + 1;
	if (pbuf->uMaxLength < uNewLength - 1)
	{
		pbuf = (LPSTRING_BUFFERW) _frealloc(pbuf, sizeof(STRING_BUFFERW) + sizeof(wchar_t) * uNewLength);
		if (!pbuf)
			return NULL;
		pbuf->uMaxLength = uNewLength - 1;
	}
	pbuf->uLength = uNewLength - 1;
	_fmemcpy(pbuf->data() + uOldLength, lpszString, uLength * sizeof(wchar_t));
	pbuf->data()[uNewLength - 1] = 0;
	return pbuf->data();
}

#ifdef ALIGNMENT_MACHINE
LPWSTR WINAPI JoinStringLen(LPWSTR lpbuf, LPCUWSTR lpszString, size_t uLen)
{
	if (!lpbuf)
		return AllocStringLen(lpszString, uLen);
	if (uLen == (size_t) -1)
		uLen = _UnicodeLen(lpszString);
	LPSTRING_BUFFERW pbuf = ((LPSTRING_BUFFERW) lpbuf) - 1;
	if (!pbuf->IsValidSignature())
		return NULL;
	size_t uOldLength = pbuf->uLength;
	size_t uLength = uLen;
	size_t uNewLength = pbuf->uLength + uLength + 1;
	if (pbuf->uMaxLength < uNewLength - 1)
	{
		pbuf = (LPSTRING_BUFFERW) _frealloc(pbuf, sizeof(STRING_BUFFERW) + sizeof(wchar_t) * uNewLength);
		if (!pbuf)
			return NULL;
		pbuf->uMaxLength = uNewLength - 1;
	}
	pbuf->uLength = uNewLength - 1;
	_fmemcpy(pbuf->data() + uOldLength, lpszString, uLength * sizeof(wchar_t));
	pbuf->data()[uNewLength - 1] = 0;
	return pbuf->data();
}
#endif

LPWSTR WINAPI JoinStringA(LPWSTR lpbuf, LPCSTR lpszString, UINT uCodePage = CP_ACP)
{
	if (!lpbuf)
		return AllocStringA(lpszString);
	LPSTRING_BUFFERW pbuf = ((LPSTRING_BUFFERW) lpbuf) - 1;
	if (!pbuf->IsValidSignature())
		return NULL;
	size_t uOldLength = pbuf->uLength;
	size_t uAnsiLength = _AnsiLen(lpszString);
	size_t uUniLength = _UnicodeLen(lpszString, uAnsiLength) + 1;
	size_t uNewLength = pbuf->uLength + uUniLength;
	if (pbuf->uMaxLength < uNewLength - 1)
	{
		pbuf = (LPSTRING_BUFFERW) _frealloc(pbuf, sizeof(STRING_BUFFERW) + sizeof(wchar_t) * uNewLength);
		if (!pbuf)
			return NULL;
		pbuf->uMaxLength = uNewLength - 1;
	}
	pbuf->uLength = uNewLength - 1;
	_UnicodeFromAnsi(pbuf->data() + uOldLength, uUniLength, lpszString, uAnsiLength, uCodePage);
	pbuf->data()[uNewLength - 1] = 0;
	return pbuf->data();
}

LPWSTR WINAPI JoinStringLenA(LPWSTR lpbuf, LPCSTR lpszString, size_t uLen, UINT uCodePage = CP_ACP)
{
	if (!lpbuf)
		return AllocStringLenA(lpszString, uLen);
	if (uLen == (size_t) -1)
		uLen = _AnsiLen(lpszString);
	LPSTRING_BUFFERW pbuf = ((LPSTRING_BUFFERW) lpbuf) - 1;
	if (!pbuf->IsValidSignature())
		return NULL;
	size_t uOldLength = pbuf->uLength;
	size_t uAnsiLength = uLen;
	size_t uUniLength = _UnicodeLen(lpszString, uAnsiLength) + 1;
	size_t uNewLength = pbuf->uLength + uUniLength;
	if (pbuf->uMaxLength < uNewLength - 1)
	{
		pbuf = (LPSTRING_BUFFERW) _frealloc(pbuf, sizeof(STRING_BUFFERW) + sizeof(wchar_t) * uNewLength);
		if (!pbuf)
			return NULL;
		pbuf->uMaxLength = uNewLength - 1;
	}
	pbuf->uLength = uNewLength - 1;
	LPWSTR lp = pbuf->data();
	_UnicodeFromAnsi(lp + uOldLength, uUniLength, lpszString, uAnsiLength, uCodePage);
	lp[uNewLength - 1] = 0;
	return lp;
}

LPWSTR WINAPI JoinStringBuffer(LPWSTR lpbuf, LPWSTR lpszSource)
{
	if (!lpbuf)
		return AllocStringBuffer(lpszSource);
	else if (!lpszSource)
		return AllocStringBuffer(lpbuf);
	LPSTRING_BUFFERW pbuf = ((LPSTRING_BUFFERW) lpbuf) - 1;
	if (!pbuf->IsValidSignature())
		return NULL;
	LPSTRING_BUFFERW pbufSrc = ((LPSTRING_BUFFERW) lpszSource) - 1;
	if (!pbufSrc->IsValidSignature())
		return NULL;
	size_t uOldLength = pbuf->uLength;
	size_t uLength = pbufSrc->uLength + 1;
	size_t uNewLength = pbuf->uLength + uLength;
	if (pbuf->uMaxLength < uNewLength - 1)
	{
		pbuf = (LPSTRING_BUFFERW) _frealloc(pbuf, sizeof(STRING_BUFFERW) + sizeof(wchar_t) * uNewLength);
		if (!pbuf)
			return NULL;
		pbuf->uMaxLength = uNewLength - 1;
	}
	pbuf->uLength = uNewLength - 1;
	_fmemcpy(pbuf->data() + uOldLength, lpszSource, uLength * sizeof(wchar_t));
	pbuf->data()[uNewLength - 1] = 0;
	return pbuf->data();
}

LPWSTR WINAPI ExpandStringLen(LPWSTR lpBuf, size_t nLen)
{
	if (!lpBuf)
		return AllocStringLen((LPCWSTR) NULL, nLen);
	size_t uLength = nLen;
	LPSTRING_BUFFERW pbuf = ((LPSTRING_BUFFERW) lpBuf) - 1;
	if (!pbuf->IsValidSignature())
		return NULL;
	if (pbuf->uMaxLength < uLength)
	{
		pbuf = (LPSTRING_BUFFERW) _frealloc(pbuf, sizeof(STRING_BUFFERW) + sizeof(wchar_t) * (uLength + 1));
		if (!pbuf)
			return NULL;
		pbuf->uMaxLength = uLength;
	}
	pbuf->uLength = uLength;
	return pbuf->data();
}

LPWSTR WINAPI SetStringLen(LPWSTR lpBuf, size_t nLen)
{
	if (!lpBuf)
	{
		if (nLen == (size_t) -1)
			return NULL;
		return AllocStringLen((LPCWSTR) NULL, nLen);
	}
	size_t uLength = nLen != (size_t) -1 ? nLen : _UnicodeLen(lpBuf);
	LPSTRING_BUFFERW pbuf = ((LPSTRING_BUFFERW) lpBuf) - 1;
	if (!pbuf->IsValidSignature())
		return NULL;
	if (pbuf->uMaxLength < uLength)
	{
		pbuf = (LPSTRING_BUFFERW) _frealloc(pbuf, sizeof(STRING_BUFFERW) + sizeof(wchar_t) * (uLength + 1));
		if (!pbuf)
			return NULL;
		pbuf->uMaxLength = uLength;
	}
	pbuf->data()[uLength] = 0;
	while (pbuf->uLength > uLength)
	{
		pbuf->uLength--;
		pbuf->data()[pbuf->uLength] = 0;
	}
	pbuf->uLength = uLength;
	return pbuf->data();
}

void WINAPI FreeStringBuffer(LPWSTR lpbuf)
{
	if (!lpbuf)
		return;
	LPSTRING_BUFFERW pbuf = ((LPSTRING_BUFFERW) lpbuf) - 1;
	if (!pbuf->IsValidSignature())
		return;
	_ffree(pbuf);
}

/////////////////////////////////////////////////////////////////////////////

CMyStringW::CMyStringW()
	: m_lpszData(NULL), m_lpszLast(NULL), m_uCodePage(CP_ACP)
{
}

CMyStringW::CMyStringW(LPCWSTR lpszString)
	: m_lpszData(NULL), m_lpszLast(NULL), m_uCodePage(CP_ACP)
{
#ifndef NO_LOADSTRING
	if (HIWORD(lpszString))
#endif
		m_lpszData = AllocString(lpszString);
#ifndef NO_LOADSTRING
	else if (lpszString)
		LoadString(LOWORD(lpszString));
	else
		m_lpszData = NULL;
#endif
}

#ifdef ALIGNMENT_MACHINE
CMyStringW::CMyStringW(LPCUWSTR lpszString)
	: m_lpszData(NULL), m_lpszLast(NULL), m_uCodePage(CP_ACP)
{
#ifndef NO_LOADSTRING
	if (HIWORD(lpszString))
#endif
		m_lpszData = AllocString(lpszString);
#ifndef NO_LOADSTRING
	else if (lpszString)
		LoadString(LOWORD(lpszString));
	else
		m_lpszData = NULL;
#endif
}
#endif

CMyStringW::CMyStringW(LPCSTR lpszString, UINT uCodePage)
	: m_lpszData(NULL), m_lpszLast(NULL)
{
#ifndef NO_LOADSTRING
	if (HIWORD(lpszString))
#endif
		m_lpszData = AllocStringA(lpszString, uCodePage);
#ifndef NO_LOADSTRING
	else if (lpszString)
		LoadString(LOWORD(lpszString));
	else
		m_lpszData = NULL;
#endif
}

CMyStringW::CMyStringW(const CMyStringW& _string)
	: m_lpszLast(NULL), m_uCodePage(_string.m_uCodePage)
{
	if (!_string.IsEmpty())
		m_lpszData = AllocStringBuffer(_string.m_lpszData);
	else
		m_lpszData = NULL;
}

CMyStringW::CMyStringW(wchar_t wch, size_t nCount)
	: m_lpszLast(NULL), m_uCodePage(CP_ACP)
{
	if (nCount > 0)
	{
		wchar_t* pch = (wchar_t*) malloc(sizeof(wchar_t) * (nCount + 1));
		for (size_t n = 0; n < nCount; n++)
			pch[n] = wch;
		pch[nCount] = 0;
		m_lpszData = AllocStringLen(pch, nCount);
		free(pch);
	}
	else
		m_lpszData = NULL;
}

CMyStringW::CMyStringW(char ch, size_t nCount, UINT uCodePage)
	: m_lpszLast(NULL), m_uCodePage(uCodePage)
{
	if (nCount > 0)
	{
		char* pch = (char*) malloc(sizeof(char) * (nCount + 1));
		for (size_t n = 0; n < nCount; n++)
			pch[n] = ch;
		pch[nCount] = 0;
		m_lpszData = AllocStringLenA(pch, nCount, uCodePage);
		free(pch);
	}
	else
		m_lpszData = NULL;
}

const CMyStringW& CMyStringW::operator = (LPCWSTR lpszString)
{
	Empty();
	if (lpszString)
		m_lpszData = AllocString(lpszString);
	return *this;
}

#ifdef ALIGNMENT_MACHINE
const CMyStringW& CMyStringW::operator = (LPCUWSTR lpszString)
{
	Empty();
	if (lpszString)
		m_lpszData = AllocString(lpszString);
	return *this;
}
#endif

const CMyStringW& CMyStringW::operator = (LPCSTR lpszString)
{
	Empty();
	if (lpszString)
		m_lpszData = AllocStringA(lpszString, m_uCodePage);
	return *this;
}

const CMyStringW& CMyStringW::operator = (const CMyStringW& _string)
{
	Empty();
	if (!_string.IsEmpty())
		m_lpszData = AllocStringBuffer(_string.m_lpszData);
	m_uCodePage = _string.m_uCodePage;
	return *this;
}

CMyStringW CMyStringW::operator + (LPCWSTR lpszString) const
{
	if (!lpszString)
		return *this;
	CMyStringW str;
	LPWSTR lp = AllocStringBuffer(m_lpszData);
	if (!lp)
		return str;
	str.m_lpszData = JoinString(lp, lpszString);
	if (!str.m_lpszData)
	{
		FreeStringBuffer(lp);
		return str;
	}
	return str;
}

#ifdef ALIGNMENT_MACHINE
CMyStringW CMyStringW::operator + (LPCUWSTR lpszString) const
{
	if (!lpszString)
		return *this;
	CMyStringW str;
	LPWSTR lp = AllocStringBuffer(m_lpszData);
	if (!lp)
		return str;
	str.m_lpszData = JoinString(lp, lpszString);
	if (!str.m_lpszData)
	{
		FreeStringBuffer(lp);
		return str;
	}
	return str;
}
#endif

CMyStringW CMyStringW::operator + (LPCSTR lpszString) const
{
	if (!lpszString)
		return *this;
	CMyStringW str;
	LPWSTR lp = AllocStringBuffer(m_lpszData);
	if (!lp)
		return str;
	str.m_lpszData = JoinStringA(lp, lpszString, m_uCodePage);
	if (!str.m_lpszData)
	{
		FreeStringBuffer(lp);
		return str;
	}
	return str;
}

CMyStringW CMyStringW::operator + (const CMyStringW& _string) const
{
	if (_string.IsEmpty())
		return *this;
	CMyStringW str;
	LPWSTR lp = AllocStringBuffer(m_lpszData);
	if (!lp)
		return str;
	if (_string.m_lpszData)
		str.m_lpszData = JoinStringBuffer(lp, _string.m_lpszData);
	else
		str.m_lpszData = lp;
	if (!str.m_lpszData)
	{
		FreeStringBuffer(lp);
		return str;
	}
	return str;
}

CMyStringW CMyStringW::operator + (wchar_t ch) const
{
	CMyStringW str;
	LPWSTR lp = AllocStringBuffer(m_lpszData);
	if (!lp)
		return str;
	str.m_lpszData = JoinStringLen(lp, &ch, 1);
	if (!str.m_lpszData)
	{
		FreeStringBuffer(lp);
		return str;
	}
	return str;
}

CMyStringW CMyStringW::operator + (char ch) const
{
	CMyStringW str;
	LPWSTR lp = AllocStringBuffer(m_lpszData);
	if (!lp)
		return str;
	str.m_lpszData = JoinStringLenA(lp, &ch, 1, m_uCodePage);
	if (!str.m_lpszData)
	{
		FreeStringBuffer(lp);
		return str;
	}
	return str;
}

CMyStringW CMyStringW::operator & (LPCWSTR lpszString) const
{
	if (!lpszString)
		return *this;
	CMyStringW str;
	LPWSTR lp = AllocStringBuffer(m_lpszData);
	if (!lp)
		return str;
	str.m_lpszData = JoinString(lp, lpszString);
	if (!str.m_lpszData)
	{
		FreeStringBuffer(lp);
		return str;
	}
	return str;
}

#ifdef ALIGNMENT_MACHINE
CMyStringW CMyStringW::operator & (LPCUWSTR lpszString) const
{
	if (!lpszString)
		return *this;
	CMyStringW str;
	LPWSTR lp = AllocStringBuffer(m_lpszData);
	if (!lp)
		return str;
	str.m_lpszData = JoinString(lp, lpszString);
	if (!str.m_lpszData)
	{
		FreeStringBuffer(lp);
		return str;
	}
	return str;
}
#endif

CMyStringW CMyStringW::operator & (LPCSTR lpszString) const
{
	if (!lpszString)
		return *this;
	CMyStringW str;
	LPWSTR lp = AllocStringBuffer(m_lpszData);
	if (!lp)
		return str;
	str.m_lpszData = JoinStringA(lp, lpszString, m_uCodePage);
	if (!str.m_lpszData)
	{
		FreeStringBuffer(lp);
		return str;
	}
	return str;
}

CMyStringW CMyStringW::operator & (const CMyStringW& _string) const
{
	if (_string.IsEmpty())
		return *this;
	CMyStringW str;
	LPWSTR lp = AllocStringBuffer(m_lpszData);
	if (!lp)
		return str;
	if (_string.m_lpszData)
		str.m_lpszData = JoinStringBuffer(lp, _string.m_lpszData);
	else
		str.m_lpszData = lp;
	if (!str.m_lpszData)
	{
		FreeStringBuffer(lp);
		return str;
	}
	return str;
}

CMyStringW CMyStringW::operator & (wchar_t ch) const
{
	CMyStringW str;
	LPWSTR lp = AllocStringBuffer(m_lpszData);
	if (!lp)
		return str;
	str.m_lpszData = JoinStringLen(lp, &ch, 1);
	if (!str.m_lpszData)
	{
		FreeStringBuffer(lp);
		return str;
	}
	return str;
}

CMyStringW CMyStringW::operator & (char ch) const
{
	CMyStringW str;
	LPWSTR lp = AllocStringBuffer(m_lpszData);
	if (!lp)
		return str;
	str.m_lpszData = JoinStringLenA(lp, &ch, 1, m_uCodePage);
	if (!str.m_lpszData)
	{
		FreeStringBuffer(lp);
		return str;
	}
	return str;
}

CMyStringW operator + (LPCWSTR lpszString, const CMyStringW& _string)
	{ CMyStringW str(lpszString); return str + _string; }
#ifdef ALIGNMENT_MACHINE
CMyStringW operator + (LPCUWSTR lpszString, const CMyStringW& _string)
	{ CMyStringW str(lpszString); return str + _string; }
#endif
CMyStringW operator + (LPCSTR lpszString, const CMyStringW& _string)
	{ CMyStringW str(lpszString); return str + _string; }
CMyStringW operator + (char ch, const CMyStringW& _string)
	{ CMyStringW str(ch, 1); return str + _string; }
CMyStringW operator + (wchar_t ch, const CMyStringW& _string)
	{ CMyStringW str(ch, 1); return str + _string; }

const CMyStringW& CMyStringW::operator += (LPCWSTR lpszString)
{
	if (!lpszString)
		return *this;
	LPWSTR lpszData = JoinString(m_lpszData, lpszString);
	if (!lpszData)
		return *this;
	m_lpszData = lpszData;
	return *this;
}

#ifdef ALIGNMENT_MACHINE
const CMyStringW& CMyStringW::operator += (LPCUWSTR lpszString)
{
	if (!lpszString)
		return *this;
	LPWSTR lpszData = JoinString(m_lpszData, lpszString);
	if (!lpszData)
		return *this;
	m_lpszData = lpszData;
	return *this;
}
#endif

const CMyStringW& CMyStringW::operator += (LPCSTR lpszString)
{
	if (!lpszString)
		return *this;
	LPWSTR lpszData = JoinStringA(m_lpszData, lpszString, m_uCodePage);
	if (!lpszData)
		return *this;
	m_lpszData = lpszData;
	return *this;
}

const CMyStringW& CMyStringW::operator += (const CMyStringW& _string)
{
	if (_string.IsEmpty())
		return *this;
	LPWSTR lpszData = JoinStringBuffer(m_lpszData, _string.m_lpszData);
	if (!lpszData)
		return *this;
	m_lpszData = lpszData;
	return *this;
}

const CMyStringW& CMyStringW::operator += (wchar_t ch)
{
	AppendChar(ch);
	return *this;
}

const CMyStringW& CMyStringW::operator += (char ch)
{
	//LPSTR lpszData = JoinStringLenA(m_lpszData, &ch, 1, m_uCodePage);
	//if (!lpszData)
	//	return *this;
	//m_lpszData = lpszData;
	AppendChar(ch);
	return *this;
}
//

CMyStringW::operator LPCWSTR() const
{
	return m_lpszData;
}

CMyStringW::operator LPSTR()
{
	if (!m_lpszData)
		return NULL;
	if (m_lpszLast)
	{
		LPSTRING_TEMP_BUFFERA lpb = (((LPSTRING_TEMP_BUFFERA) m_lpszLast) - 1);
		if (!lpb->IsValidSignature())
			_ffree(m_lpszLast);
		else
			_ffree(lpb);
	}
	return m_lpszLast = _AnsiFromUnicode2(m_lpszData, (int) GetStringLen(m_lpszData), NULL, m_uCodePage);
}

wchar_t CMyStringW::operator [] (size_t uIndex) const
{
	return m_lpszData[uIndex];
}

wchar_t& CMyStringW::operator [] (size_t uIndex)
{
	return m_lpszData[uIndex];
}

CMyStringW::~CMyStringW()
{
	Empty();
}

void CMyStringW::Empty()
{
	if (m_lpszLast)
	{
		LPSTRING_TEMP_BUFFERA lpb = (((LPSTRING_TEMP_BUFFERA) m_lpszLast) - 1);
		if (!lpb->IsValidSignature())
			_ffree(m_lpszLast);
		else
			_ffree(lpb);
	}
	m_lpszLast = NULL;
	if (m_lpszData)
		FreeStringBuffer(m_lpszData);
	m_lpszData = NULL;
}

size_t CMyStringW::GetLength() const
{
	if (!m_lpszData)
		return 0;
	return GetStringLen(m_lpszData);
}

size_t CMyStringW::GetLengthA() const
{
	if (!m_lpszData)
		return 0;
	return _AnsiLen(m_lpszData, GetStringLen(m_lpszData), m_uCodePage);
}

LPWSTR CMyStringW::GetBuffer(size_t uLength)
{
	if (uLength > GetLength())
	{
		LPWSTR lp = ExpandStringLen(m_lpszData, uLength);
		if (!lp)
			return NULL;
		m_lpszData = lp;
	}
	return m_lpszData;
}

void CMyStringW::ReleaseBuffer(size_t uNewLength)
{
	LPWSTR lp = SetStringLen(m_lpszData, uNewLength);
	if (lp)
		m_lpszData = lp;
}

bool CMyStringW::IsEmpty() const
{
	if (!m_lpszData)
		return true;
	return GetLength() == 0;
}

void CMyStringW::SetString(LPCWSTR lpszString, size_t uLength)
{
	register LPWSTR lpb = NULL;
	if (lpszString)
		lpb = AllocStringLen(lpszString, uLength);
	Empty();
	m_lpszData = lpb;
}

#ifdef ALIGNMENT_MACHINE
void CMyStringW::SetString(LPCUWSTR lpszString, size_t uLength)
{
	register LPWSTR lpb = NULL;
	if (lpszString)
		lpb = AllocStringLen(lpszString, uLength);
	Empty();
	m_lpszData = lpb;
}
#endif

void CMyStringW::SetString(LPCSTR lpszString, size_t uLength)
{
	register LPWSTR lpb = NULL;
	if (lpszString)
		lpb = AllocStringLenA(lpszString, uLength, m_uCodePage);
	Empty();
	m_lpszData = lpb;
}

void CMyStringW::SetString(const CMyStringW& strData, size_t uLength)
{
	Empty();
	if (uLength == (size_t) -1)
		uLength = strData.GetLength();
	if (!strData.IsEmpty())
		m_lpszData = AllocStringLen(strData, uLength);
	m_uCodePage = strData.m_uCodePage;
}

LPSTR CMyStringW::GetBufferA(size_t uLength)
{
	size_t u = m_lpszData ? _AnsiLen(m_lpszData, GetLength()) : 0;
	if (!uLength || uLength < u)
		uLength = u;
	if (uLength == 0)
		return NULL;
	LPSTRING_TEMP_BUFFERA lpb;
	if (m_lpszLast)
	{
		lpb = ((LPSTRING_TEMP_BUFFERA) m_lpszLast) - 1;
		if (!lpb->IsValidSignature() || lpb->uMaxLength < uLength)
		{
			if (!lpb->IsValidSignature())
				lpb = (LPSTRING_TEMP_BUFFERA) m_lpszLast;
			LPSTRING_TEMP_BUFFERA lpb2 = (LPSTRING_TEMP_BUFFERA) _frealloc(lpb, sizeof(STRING_TEMP_BUFFERA) + sizeof(char) * (uLength + 1));
			if (!lpb2)
				return NULL;
			lpb = lpb2;
			lpb->uMaxLength = uLength;
			lpb->dwSig = STRING_BUFFERW_SIGNATURE;
		}
		// prevent from being released by Empty()
		m_lpszLast = NULL;
	}
	else
	{
		lpb = (LPSTRING_TEMP_BUFFERA) _fmalloc(sizeof(STRING_TEMP_BUFFERA) + sizeof(char) * (uLength + 1));
		if (!lpb)
			return NULL;
		lpb->dwSig = STRING_BUFFERW_SIGNATURE;
	}
	lpb->uLength = uLength;
	LPSTR lp = lpb->data();
	if (m_lpszData)
		_AnsiFromUnicode(lp, uLength + 1, m_lpszData, GetLength(), m_uCodePage);
	lp[u] = 0;
	Empty();
	m_lpszLast = lp;
	return lp;
}

void CMyStringW::ReleaseBufferA(BOOL bSetLength, size_t uNewLength)
{
	if (!m_lpszLast)
		return;
	size_t uLength;
	LPSTRING_TEMP_BUFFERA lpb = ((LPSTRING_TEMP_BUFFERA) m_lpszLast) - 1;
	if (!lpb->IsValidSignature())
		return;
	uLength = lpb->uLength;
	if (bSetLength)
	{
		if (uNewLength == (size_t) -1)
			uLength = _AnsiLen(m_lpszLast);
		else if (uNewLength < uLength)
			uLength = uNewLength;
	}
	if (m_lpszData)
		FreeStringBuffer(m_lpszData);
	m_lpszData = AllocStringLenA(m_lpszLast, uLength, m_uCodePage);
	_ffree(lpb);
	m_lpszLast = NULL;
}

#ifndef NO_LOADSTRING
BOOL CMyStringW::LoadString(UINT uID)
{
	LPWSTR lpw;
	lpw = UniLoadStringW(NULL, uID);
	if (!lpw)
		return FALSE;
	operator = (lpw);
	free(lpw);
	return TRUE;
}
#endif


size_t CMyStringW::AppendChar(wchar_t wch)
{
	size_t uLength = GetLength();
	LPWSTR lp = ExpandStringLen(m_lpszData, uLength + 1);
	if (!lp)
		return (size_t) -1;
	m_lpszData = lp;
	lp[uLength] = wch;
	lp[uLength + 1] = 0;
	return uLength + 1;
}

size_t CMyStringW::AppendChar(char ch)
{
	LPSTR lp;
	size_t uUniLen;
	size_t uAnsiLen;
	uUniLen = GetLength();
	if (!uUniLen)
	{
		char szBuffer[2];
		szBuffer[0] = ch;
		szBuffer[1] = 0;
		if (m_lpszData)
			Empty();
		m_lpszData = AllocStringLenA(szBuffer, 1, m_uCodePage);
		return 1;
	}
	uAnsiLen = _AnsiLen(m_lpszData, uUniLen);
	uAnsiLen++;
	lp = (LPSTR) malloc(sizeof(char) * (uAnsiLen + 1));
	_AnsiFromUnicode(lp, uAnsiLen, m_lpszData, (int) (uUniLen + 1), m_uCodePage);
	lp[uAnsiLen - 1] = ch;
	lp[uAnsiLen] = 0;
	Empty();
	m_lpszData = AllocStringLenA(lp, uUniLen, m_uCodePage);
	free(lp);
	return uAnsiLen;
}

size_t CMyStringW::AppendString(LPCWSTR lpszString, size_t uLength)
{
	if (!lpszString)
		return (size_t) -1;
	LPWSTR lpszData = JoinStringLen(m_lpszData, lpszString, uLength);
	if (!lpszData)
		return (size_t) -1;
	m_lpszData = lpszData;
	return GetStringLen(m_lpszData);
}

#ifdef ALIGNMENT_MACHINE
size_t CMyStringW::AppendString(LPCUWSTR lpszString, size_t uLength)
{
	if (!lpszString)
		return (size_t) -1;
	LPWSTR lpszData = JoinStringLen(m_lpszData, lpszString, uLength);
	if (!lpszData)
		return (size_t) -1;
	m_lpszData = lpszData;
	return GetStringLen(m_lpszData);
}
#endif

size_t CMyStringW::AppendString(LPCSTR lpszString, size_t uLength)
{
	if (!lpszString)
		return (size_t) -1;
	LPWSTR lpszData = JoinStringLenA(m_lpszData, lpszString, uLength, m_uCodePage);
	if (!lpszData)
		return (size_t) -1;
	m_lpszData = lpszData;
	return _AnsiLen(m_lpszData, GetStringLen(m_lpszData), m_uCodePage);
}

size_t CMyStringW::AppendString(const CMyStringW& _string)
{
	if (_string.IsEmpty())
		return (size_t) -1;
	LPWSTR lpszData = JoinStringBuffer(m_lpszData, _string.m_lpszData);
	if (!lpszData)
		return (size_t) -1;
	m_lpszData = lpszData;
	return GetStringLen(m_lpszData);
}

size_t CMyStringW::InsertChar(wchar_t wch, size_t uPos)
{
	size_t uLength = GetLength();
	if (uPos > uLength)
		return (size_t) -1;
	LPWSTR lp = ExpandStringLen(m_lpszData, uLength + 1);
	if (!lp)
		return (size_t) -1;
	m_lpszData = lp;
	_fmemmove(lp + uPos + 1, lp + uPos, (size_t)(sizeof(wchar_t) * (uLength - uPos + 1)));
	lp[uPos] = wch;
	return uPos + 1;
}

size_t CMyStringW::InsertChar(char ch, size_t uPos)
{
	LPSTR lp;
	size_t uUniLen;
	size_t uAnsiLen;
	uUniLen = GetLength();
	if (!uUniLen)
	{
		char szBuffer[2];
		szBuffer[0] = ch;
		szBuffer[1] = 0;
		if (m_lpszData)
			Empty();
		m_lpszData = AllocStringLenA(szBuffer, 1, m_uCodePage);
		return 1;
	}
	uAnsiLen = _AnsiLen(m_lpszData, uUniLen);
	if (uPos > uAnsiLen)
		return (size_t) -1;
	uAnsiLen++;
	lp = (LPSTR) malloc(sizeof(char) * (uAnsiLen + 1));
	_AnsiFromUnicode(lp, uAnsiLen, m_lpszData, (uUniLen + 1), m_uCodePage);
	memmove(lp + uPos + 1, lp + uPos, (size_t)(sizeof(char) * (uAnsiLen - uPos /*+ 1*/)));
	lp[uPos] = ch;
	Empty();
	m_lpszData = AllocStringLenA(lp, uAnsiLen, m_uCodePage);
	free(lp);
	return uPos + 1;
}

size_t CMyStringW::InsertString(LPCWSTR lpszString, size_t uPos, size_t uStrLen)
{
	size_t uLength = GetLength();
	if (uPos > uLength)
		return (size_t) -1;
	if (uStrLen == (size_t) -1)
		uStrLen = _UnicodeLen(lpszString);
	LPWSTR lp = ExpandStringLen(m_lpszData, uLength + uStrLen);
	if (!lp)
		return (size_t) -1;
	m_lpszData = lp;
	_fmemmove(lp + uPos + uStrLen, lp + uPos, (size_t)(sizeof(wchar_t) * (uLength - uPos + 1)));
	_fmemcpy(lp + uPos, lpszString, (size_t)(sizeof(wchar_t) * uStrLen));
	return uPos + uStrLen;
}

#ifdef ALIGNMENT_MACHINE
size_t CMyStringW::InsertString(LPCUWSTR lpszString, size_t uPos, size_t uStrLen)
{
	size_t uLength = GetLength();
	if (uPos > uLength)
		return (size_t) -1;
	if (uStrLen == (size_t) -1)
		uStrLen = _UnicodeLen(lpszString);
	LPWSTR lp = ExpandStringLen(m_lpszData, uLength + uStrLen);
	if (!lp)
		return (size_t) -1;
	m_lpszData = lp;
	_fmemmove(lp + uPos + uStrLen, lp + uPos, (size_t)(sizeof(wchar_t) * (uLength - uPos + 1)));
	_fmemcpy(lp + uPos, lpszString, (size_t)(sizeof(wchar_t) * uStrLen));
	return uPos + uStrLen;
}
#endif

size_t CMyStringW::InsertString(LPCSTR lpszString, size_t uPos, size_t uStrLen)
{
	LPSTR lp;
	size_t uUniLen;
	size_t uAnsiLen;
	uUniLen = GetLength();
	if (uStrLen == (size_t) -1)
		uStrLen = _AnsiLen(lpszString);
	if (!uUniLen)
	{
		if (m_lpszData)
			Empty();
		m_lpszData = AllocStringLenA(lpszString, uStrLen, m_uCodePage);
		return 1;
	}
	uAnsiLen = _AnsiLen(m_lpszData, uUniLen);
	if (uPos > uAnsiLen)
		return (size_t) -1;
	uUniLen++;
	lp = (LPSTR) malloc(sizeof(char) * (uAnsiLen + uStrLen));
	_AnsiFromUnicode(lp, uUniLen, m_lpszData, (uAnsiLen + 1), m_uCodePage);
	memmove(lp + uPos + uStrLen, lp + uPos, (size_t)(sizeof(char) * (uAnsiLen - uPos /*+ 1*/)));
	memcpy(lp + uPos, lpszString, (size_t)(sizeof(char) * uStrLen));
	Empty();
	m_lpszData = AllocStringLenA(lp, uAnsiLen + uStrLen - 1, m_uCodePage);
	free(lp);
	return uPos + 1;
}

size_t CMyStringW::InsertString(const CMyStringW& strInsert, size_t uPos)
{
	size_t uStrLen = strInsert.GetLength();
	size_t uLength = GetLength();
	if (uPos > uLength)
		return (size_t) -1;
	if (strInsert.IsEmpty())
		return uLength;
	LPWSTR lp = ExpandStringLen(m_lpszData, uLength + uStrLen);
	if (!lp)
		return (size_t) -1;
	m_lpszData = lp;
	_fmemmove(lp + uPos + uStrLen, lp + uPos, (size_t)(sizeof(wchar_t) * (uLength - uPos + 1)));
	_fmemcpy(lp + uPos, (LPCWSTR) strInsert, (size_t)(sizeof(wchar_t) * uStrLen));
	return uPos + uStrLen;
}

wchar_t CMyStringW::DeleteChar(size_t uPos)
{
	LPSTRING_BUFFERW pbuf;
	if (!m_lpszData)
		return 0;
	pbuf = ((LPSTRING_BUFFERW) m_lpszData) - 1;
	if (!pbuf->IsValidSignature())
		return 0;
	if (uPos >= pbuf->uLength)
		return 0;
	wchar_t chRet = m_lpszData[uPos];
	if (pbuf->uLength == 1)
	{
		Empty();
		return chRet;
	}
	_fmemmove(m_lpszData + uPos, m_lpszData + uPos + 1, (size_t)(sizeof(wchar_t) * (pbuf->uLength - uPos)));
	pbuf->uLength--;
	return chRet;
}

bool CMyStringW::DeleteString(size_t uStart, size_t uLength)
{
	LPSTRING_BUFFERW pbuf;
	if (!m_lpszData)
		return false;
	pbuf = ((LPSTRING_BUFFERW) m_lpszData) - 1;
	if (!pbuf->IsValidSignature())
		return false;
	if (uStart + uLength > pbuf->uLength)
		return false;
	if (uStart == 0 && pbuf->uLength == uLength)
	{
		Empty();
		return true;
	}
	_fmemmove(m_lpszData + uStart, m_lpszData + uStart + uLength, (size_t)(sizeof(wchar_t) * (pbuf->uLength - uStart - uLength + 1)));
	pbuf->uLength -= uLength;
	return true;
}

/////////////////////////////////////////////////////////////////////////////

void __stdcall _FormatStringA(LPWSTR& lpszData, LPCSTR lpszFormat, va_list va, UINT uCodePage = CP_ACP)
{
	int n, nRet;
	LPSTR lp;

	n = 255;
	lp = (LPSTR) malloc(sizeof(CHAR) * 255);
	nRet = _vsnprintf(lp, n, lpszFormat, va);
	while (nRet < 0)
	{
		n += 255;
		lp = (LPSTR) realloc(lp, sizeof(CHAR) * n);
		nRet = _vsnprintf(lp, n, lpszFormat, va);
	}
	if (lpszData)
		lpszData = ReAllocStringLenA(lpszData, lp, nRet, uCodePage);
	else
		lpszData = AllocStringLenA(lp, nRet, uCodePage);
	free(lp);
}

void __stdcall _FormatStringW(LPWSTR& lpszData, LPCWSTR lpszFormat, va_list va)
{
	int n, nRet;
	LPWSTR lp;

	n = 255;
	lp = (LPWSTR) malloc(sizeof(WCHAR) * 255);
	nRet = _vsnwprintf(lp, n, lpszFormat, va);
	while (nRet < 0)
	{
		n += 255;
		lp = (LPWSTR) realloc(lp, sizeof(WCHAR) * n);
		nRet = _vsnwprintf(lp, n, lpszFormat, va);
	}
	if (lpszData)
		lpszData = ReAllocStringLen(lpszData, lp, nRet);
	else
		lpszData = AllocStringLen(lp, nRet);
	free(lp);
}

BOOL WINAPIV CMyStringW::Format(LPCSTR lpszString, ...)
{
	va_list va;
	va_start(va, lpszString);
	//Clear();
	//int nLen = _vscprintf(lpszString, va);
	//if (!nLen)
	//	return FALSE;
	//LPSTR lp = GetBuffer((size_t) nLen);
	//nLen = _vsprintf(lp, lpszString, va);
	//ReleaseBuffer((size_t) nLen);
	_FormatStringA(m_lpszData, lpszString, va, m_uCodePage);
	va_end(va);
	return TRUE;
}

BOOL WINAPIV CMyStringW::Format(LPCWSTR lpszString, ...)
{
	va_list va;
	va_start(va, lpszString);
	//Clear();
	//int nLen = _vscwprintf(lpszString, va);
	//if (!nLen)
	//	return FALSE;
	//LPWSTR lp = GetBufferW((size_t) nLen);
	//nLen = _vswprintf(lp, lpszString, va);
	//ReleaseBufferW(TRUE, (size_t) nLen);
	_FormatStringW(m_lpszData, lpszString, va);
	va_end(va);
	return TRUE;
}

BOOL WINAPIV CMyStringW::Format(const CMyStringW* pstrFormat, ...)
{
	va_list va;
	va_start(va, pstrFormat);
	//Clear();
	//int nLen = _vscprintf(lpszString, va);
	//if (!nLen)
	//	return FALSE;
	//LPSTR lp = GetBuffer((size_t) nLen);
	//nLen = _vsprintf(lp, lpszString, va);
	//ReleaseBuffer((size_t) nLen);
	_FormatStringW(m_lpszData, pstrFormat->m_lpszData, va);
	va_end(va);
	return TRUE;
}

#ifndef NO_LOADSTRING
BOOL WINAPIV CMyStringW::Format(UINT uID, ...)
{
	CMyStringW str(MAKEINTRESOURCE(uID));
	va_list va;
	va_start(va, uID);
	//Clear();
	//int nLen = _vscprintf((LPCSTR) str, va);
	//if (!nLen)
	//	return FALSE;
	//LPSTR lp = GetBuffer((size_t) nLen);
	//nLen = _vsprintf(lp, (LPCSTR) str, va);
	//ReleaseBuffer((size_t) nLen);
	_FormatStringW(m_lpszData, str, va);
	va_end(va);
	return TRUE;
}
#endif

int CMyStringW::Compare(LPCWSTR lpszString, bool bNoCase, size_t uLength) const
{
	if (IsEmpty())
	{
		if (uLength == 0 || (uLength == (size_t) -1 && (!lpszString || !*lpszString)))
			return 0;
		return -1;
	}
	else if (bNoCase)
	{
		if (uLength == (size_t) -1)
			return (int) _wcsicmp(m_lpszData, lpszString);
		else
			return (int) _wcsnicmp(m_lpszData, lpszString, (size_t) uLength);
	}
	else
	{
		if (uLength == (size_t) -1)
			return (int) wcscmp(m_lpszData, lpszString);
		else
			return (int) wcsncmp(m_lpszData, lpszString, (size_t) uLength);
	}
}

int CMyStringW::Compare(LPCSTR lpszString, bool bNoCase, size_t uLength) const
{
	if (IsEmpty())
	{
		if (uLength == 0 || (uLength == (size_t) -1 && (!lpszString || !*lpszString)))
			return 0;
		return -1;
	}

	LPSTR lp = AllocAnsiString();
	int ret;
	if (bNoCase)
	{
		if (uLength == (size_t) -1)
			ret = (int) _stricmp(lp, lpszString);
		else
			ret = (int) _strnicmp(lp, lpszString, (size_t) uLength);
	}
	else
	{
		if (uLength == (size_t) -1)
			ret = (int) strcmp(lp, lpszString);
		else
			ret = (int) strncmp(lp, lpszString, (size_t) uLength);
	}
	free(lp);
	return ret;
}

int CMyStringW::Compare(const CMyStringW& strCompare, bool bNoCase) const
{
	size_t uLength = strCompare.GetLength() + 1;
	if (IsEmpty())
	{
		if (strCompare.IsEmpty())
			return 0;
		return -1;
	}
	else if (strCompare.IsEmpty())
		return 1;
	else if (bNoCase)
	{
		if (uLength == (size_t) -1)
			return (int) _wcsicmp(m_lpszData, strCompare);
		else
			return (int) _wcsnicmp(m_lpszData, strCompare, (size_t) uLength);
	}
	else
	{
		if (uLength == (size_t) -1)
			return (int) wcscmp(m_lpszData, strCompare);
		else
			return (int) wcsncmp(m_lpszData, strCompare, (size_t) uLength);
	}
}

LPSTR CMyStringW::AllocAnsiString() const
{
	return _AnsiFromUnicode2(m_lpszData, (int) GetStringLen(m_lpszData), NULL, m_uCodePage);
}

UINT CMyStringW::GetCodePage() const
{
	return m_uCodePage;
}

void CMyStringW::SetCodePage(UINT uCodePage)
{
	m_uCodePage = uCodePage;
}

///////////////////////////////////////////////////////////////////////////////

void CMyStringW::SetUTF8String(LPCBYTE lpBuffer, size_t dwByteLength)
{
	size_t dwNewLength;
	LPWSTR lpw;
	Empty();
	dwNewLength = ::UTF8ToUnicode(lpBuffer, dwByteLength, NULL, 0);
	if (!dwNewLength)
		return;
	lpw = AllocStringLen((LPCWSTR) NULL, dwNewLength);
	::UTF8ToUnicode(lpBuffer, dwByteLength, lpw, dwNewLength + 1);
	m_lpszData = lpw;
}

LPCBYTE CMyStringW::AllocUTF8String(size_t* lpdwLength)
{
	LPBYTE lp;
	size_t dwLength;

	if (IsEmpty())
		return NULL;

	dwLength = ::UnicodeToUTF8(m_lpszData, GetLength(), NULL, 0) + 1;
	lp = (LPBYTE) _fmalloc(sizeof(BYTE) * dwLength);
	::UnicodeToUTF8(m_lpszData, GetLength(), lp, dwLength);
	lp[dwLength - 1] = 0;
	if (m_lpszLast)
		_ffree(m_lpszLast);
	m_lpszLast = (LPSTR) lp;
	if (lpdwLength)
		*lpdwLength = dwLength - 1;
	return (LPCBYTE) lp;
}

LPBYTE CMyStringW::AllocUTF8StringC(size_t* lpdwLength) const
{
	LPBYTE lp;
	size_t dwLength;

	if (IsEmpty())
		return NULL;

	dwLength = ::UnicodeToUTF8(m_lpszData, GetLength(), NULL, 0) + 1;
	lp = (LPBYTE) _fmalloc(sizeof(BYTE) * dwLength);
	::UnicodeToUTF8(m_lpszData, GetLength(), lp, dwLength);
	lp[dwLength - 1] = 0;
	if (lpdwLength)
		*lpdwLength = dwLength - 1;
	return (LPBYTE) lp;
}
