/*
 Copyright (C) 2010 Kuri-Applications

 Convert.cpp - implementations of functions for convertion between charsets
 */

#include "stdafx.h"
#include "Convert.h"

typedef const BYTE FAR* LPCBYTE;

#define break_if_failed(e)  if (!(e))  break
#define flag_if_failed(e, d, f)  if (!(e))  d = (f)

#define GetFilePointer(hFile) SetFilePointer(hFile, 0, NULL, FILE_CURRENT)
#define GetVLFilePointer(hFile, lpPositionHigh) \
	(*lpPositionHigh = 0, \
	SetFilePointer(hFile, 0, lpPositionHigh, FILE_CURRENT))

extern "C" size_t __stdcall UnicodeToUTF8(LPCWSTR lpszUnicode, size_t dwLength, LPBYTE lpBuffer, size_t dwBufferLength)
{
	DWORD dw;
	WORD wSurrogate;
	size_t dwIndex;
	size_t dwCalcLength;
	if (dwLength == (size_t) -1)
		dwLength = wcslen(lpszUnicode);

	if (!lpBuffer || !dwBufferLength)
	{
		dwCalcLength = 0;
		wSurrogate = 0;
		for (dwIndex = 0; dwIndex < dwLength; dwIndex++)
		{
			dw = (DWORD) lpszUnicode[dwIndex];
			if (wSurrogate)
			{
				if (dw < 0xDC00ul || dw >= 0xE000ul)
					dwCalcLength += 3 + 3;
				else
				{
					// ここから UCS-4
					wSurrogate -= 0xD800ul;
					dw = (dw - 0xDC00) | ((DWORD) wSurrogate << 10);
					if (dw < 0x00200000ul)
						dwCalcLength += 4;
					else if (dw < 0x04000000ul)
						dwCalcLength += 5;
					else if (dw < 0x80000000ul)
						dwCalcLength += 6;
					else
						//{ /* unsupported */ }
						dwCalcLength += 3;
				}
				wSurrogate = 0;
			}
			else
			{
				if (dw < 0x0080ul)
					dwCalcLength++;
				else if (dw < 0x0800ul)
					dwCalcLength += 2;
				else if (dw >= 0xD800ul && dw < 0xDC00ul)
					wSurrogate = (WORD) dw;
				else //if (dw < 0xD800ul || dw >= 0xDC00ul)
					dwCalcLength += 3;
			}
		}

		return dwCalcLength;
	}

	dwCalcLength = 0;
	wSurrogate = 0;
	for (dwIndex = 0; dwIndex < dwLength; dwIndex++)
	{
		if (dwCalcLength >= dwBufferLength - 1)
			break;
		dw = (DWORD) lpszUnicode[dwIndex];
		if (wSurrogate)
		{
			if (dw < 0xDC00ul || dw >= 0xE000ul)
			{
				lpBuffer[dwCalcLength++] = 0xE0u | (BYTE)(wSurrogate >> 12);
				if (dwCalcLength >= dwBufferLength - 1)
					break;
				lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((wSurrogate >> 6) & 0x3Fu);
				if (dwCalcLength >= dwBufferLength - 1)
					break;
				lpBuffer[dwCalcLength++] = 0x80u | (BYTE)(wSurrogate & 0x3Fu);
				if (dwCalcLength >= dwBufferLength - 1)
					break;
				lpBuffer[dwCalcLength++] = 0xE0u | (BYTE)(dw >> 12);
				if (dwCalcLength >= dwBufferLength - 1)
					break;
				lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 6) & 0x3Fu);
				if (dwCalcLength >= dwBufferLength - 1)
					break;
				lpBuffer[dwCalcLength++] = 0x80u | (BYTE)(dw & 0x3Fu);
			}
			else
			{
				// ここから UCS-4
				wSurrogate -= 0xD800ul;
				dw = (dw - 0xDC00) | ((DWORD) wSurrogate << 10);
				if (dw < 0x00200000ul)
				{
					// 00000000.000wwwxx.xxxxyyyy.yyzzzzzz: 11110www 10xxxxxx 10yyyyyy 10zzzzzz
					lpBuffer[dwCalcLength++] = 0xF0u | (BYTE)(dw >> 18);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 12) & 0x3Fu);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 6) & 0x3Fu);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)(dw & 0x3Fu);
				}
				else if (dw < 0x04000000ul)
				{
					// 000000vv.wwwwwwxx.xxxxyyyy.yyzzzzzz: 111110vv 10wwwwww 10xxxxxx 10yyyyyy 10zzzzzz
					lpBuffer[dwCalcLength++] = 0xF8u | (BYTE)(dw >> 24);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 18) & 0x3Fu);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 12) & 0x3Fu);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 6) & 0x3Fu);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)(dw & 0x3Fu);
				}
				else if (dw < 0x80000000ul)
				{
					// 0uvvvvvv.wwwwwwxx.xxxxyyyy.yyzzzzzz: 1111110u 10vvvvvv 10wwwwww 10xxxxxx 10yyyyyy 10zzzzzz
					lpBuffer[dwCalcLength++] = 0xFCu | (BYTE)(dw >> 30);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 24) & 0x3Fu);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 18) & 0x3Fu);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 12) & 0x3Fu);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 6) & 0x3Fu);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)(dw & 0x3Fu);
				}
				else
				{
					/* unsupported */
					lpBuffer[dwCalcLength++] = 0xBFu;
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0xBFu;
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x3Du;
				}
			}
			wSurrogate = 0;
		}
		else
		{
			if (dw < 0x0080ul)
			{
				lpBuffer[dwCalcLength++] = (BYTE)(dw /*& 0x7F*/);
			}
			else if (dw < 0x0800ul)
			{
				// 00000yyy.yyzzzzzz: 110yyyyy 10zzzzzz
				lpBuffer[dwCalcLength++] = 0xC0u | (BYTE)(dw >> 6);
				if (dwCalcLength >= dwBufferLength - 1)
					break;
				lpBuffer[dwCalcLength++] = 0x80u | (BYTE)(dw & 0x3Fu);
			}
			else if (dw >= 0xD800ul && dw < 0xDC00ul)
				wSurrogate = (WORD) dw;
			else //if (dw < 0xD800ul || dw >= 0xDC00ul)
			{
				// xxxxyyyy.yyzzzzzz: 1110xxxx 10yyyyyy 10zzzzzz
				lpBuffer[dwCalcLength++] = 0xE0u | (BYTE)(dw >> 12);
				if (dwCalcLength >= dwBufferLength - 1)
					break;
				lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 6) & 0x3Fu);
				if (dwCalcLength >= dwBufferLength - 1)
					break;
				lpBuffer[dwCalcLength++] = 0x80u | (BYTE)(dw & 0x3Fu);
			}
		}
	}
	lpBuffer[dwCalcLength] = 0;
	return dwCalcLength;
}

#ifdef _WIN32
extern "C" size_t __stdcall UnicodeToUTF8File(LPCWSTR lpszUnicode, size_t dwLength, HANDLE hFile, BOOL* pbResult)
{
	DWORD dw;
	size_t dwIndex;
	WORD wSurrogate;
	size_t dwCalcLength;
	BYTE b;
	BOOL bRet;
	DWORD dwWritten;
	if (dwLength == (size_t) -1)
		dwLength = wcslen(lpszUnicode);

	dwCalcLength = 0;
	wSurrogate = 0;
	if (pbResult)
		*pbResult = TRUE;
	bRet = TRUE;
	for (dwIndex = 0; dwIndex < dwLength; dwIndex++)
	{
		dw = (DWORD) lpszUnicode[dwIndex];
		if (wSurrogate)
		{
			if (dw < 0xDC00ul || dw >= 0xE000ul)
			{
				b = 0xE0u | (BYTE)(wSurrogate >> 12);
				break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
				dwCalcLength++;
				b = 0x80u | (BYTE)((wSurrogate >> 6) & 0x3Fu);
				break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
				dwCalcLength++;
				b = 0x80u | (BYTE)(wSurrogate & 0x3Fu);
				break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
				dwCalcLength++;
				b = 0xE0u | (BYTE)(dw >> 12);
				break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
				dwCalcLength++;
				b = 0x80u | (BYTE)((dw >> 6) & 0x3Fu);
				break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
				dwCalcLength++;
				b = 0x80u | (BYTE)(dw & 0x3Fu);
				break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
				dwCalcLength++;
			}
			else
			{
				// ここから UCS-4
				wSurrogate -= 0xD800ul;
				dw = (dw - 0xDC00) | ((DWORD) wSurrogate << 10);
				if (dw < 0x00200000ul)
				{
					// 00000000.000wwwxx.xxxxyyyy.yyzzzzzz: 11110www 10xxxxxx 10yyyyyy 10zzzzzz
					b = 0xF0u | (BYTE)(dw >> 18);
					break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
					dwCalcLength++;
					b = 0x80u | (BYTE)((dw >> 12) & 0x3Fu);
					break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
					dwCalcLength++;
					b = 0x80u | (BYTE)((dw >> 6) & 0x3Fu);
					break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
					dwCalcLength++;
					b = 0x80u | (BYTE)(dw & 0x3Fu);
					break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
					dwCalcLength++;
				}
				else if (dw < 0x04000000ul)
				{
					// 000000vv.wwwwwwxx.xxxxyyyy.yyzzzzzz: 111110vv 10wwwwww 10xxxxxx 10yyyyyy 10zzzzzz
					b = 0xF8u | (BYTE)(dw >> 24);
					break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
					dwCalcLength++;
					b = 0x80u | (BYTE)((dw >> 18) & 0x3Fu);
					break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
					dwCalcLength++;
					b = 0x80u | (BYTE)((dw >> 12) & 0x3Fu);
					break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
					dwCalcLength++;
					b = 0x80u | (BYTE)((dw >> 6) & 0x3Fu);
					break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
					dwCalcLength++;
					b = 0x80u | (BYTE)(dw & 0x3Fu);
					break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
					dwCalcLength++;
				}
				else if (dw < 0x80000000ul)
				{
					// 0uvvvvvv.wwwwwwxx.xxxxyyyy.yyzzzzzz: 1111110u 10vvvvvv 10wwwwww 10xxxxxx 10yyyyyy 10zzzzzz
					b = 0xFCu | (BYTE)(dw >> 30);
					break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
					dwCalcLength++;
					b = 0x80u | (BYTE)((dw >> 24) & 0x3Fu);
					break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
					dwCalcLength++;
					b = 0x80u | (BYTE)((dw >> 18) & 0x3Fu);
					break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
					dwCalcLength++;
					b = 0x80u | (BYTE)((dw >> 12) & 0x3Fu);
					break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
					dwCalcLength++;
					b = 0x80u | (BYTE)((dw >> 6) & 0x3Fu);
					break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
					dwCalcLength++;
					b = 0x80u | (BYTE)(dw & 0x3Fu);
					break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
					dwCalcLength++;
				}
				else
				{
					/* unsupported */
					b = 0xBFu;
					break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
					dwCalcLength++;
					b = 0xBFu;
					break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
					dwCalcLength++;
					b = 0x3Du;
					break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
					dwCalcLength++;
				}
			}
			wSurrogate = 0;
		}
		else
		{
			if (dw < 0x0080ul)
			{
				b = (BYTE) (dw & 0x7F);
				break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
				dwCalcLength++;
			}
			else if (dw < 0x0800ul)
			{
				// 00000yyy.yyzzzzzz: 110yyyyy 10zzzzzz
				b = 0xC0u | (BYTE)(dw >> 6);
				break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
				dwCalcLength++;
				b = 0x80u | (BYTE)(dw & 0x3Fu);
				break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
				dwCalcLength++;
			}
			else if (dw >= 0xD800ul && dw < 0xDC00ul)
				wSurrogate = (WORD) dw;
			else //if (dw < 0xD800ul || dw >= 0xDC00ul)
			{
				// xxxxyyyy.yyzzzzzz: 1110xxxx 10yyyyyy 10zzzzzz
				b = 0xE0u | (BYTE)(dw >> 12);
				break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
				dwCalcLength++;
				b = 0x80u | (BYTE)((dw >> 6) & 0x3Fu);
				break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
				dwCalcLength++;
				b = 0x80u | (BYTE)(dw & 0x3Fu);
				break_if_failed(bRet = ::WriteFile(hFile, &b, 1, &dwWritten, NULL));
				dwCalcLength++;
			}
		}
	}
	if (!bRet && pbResult)
		*pbResult = FALSE;
	return dwCalcLength;
}

extern "C" size_t __stdcall UnicodeFileToUTF8(HANDLE hFile, LPBYTE lpBuffer, size_t dwBufferLength)
{
	DWORD dw;
	DWORD dwIndex;
	WORD wSurrogate;
	size_t dwCalcLength, dwBack;
	LONG lnNowPointer1, lnNowPointer2;

	wSurrogate = 0;
	if (!lpBuffer || !dwBufferLength)
	{
		lnNowPointer1 = (LONG) GetVLFilePointer(hFile, &lnNowPointer2);
		dwCalcLength = 0;
		while (::ReadFile(hFile, &dw, sizeof(WORD), &dwIndex, NULL) && dwIndex)
		{
			if (wSurrogate)
			{
				if (dw < 0xDC00ul || dw >= 0xE000ul)
					dwCalcLength += 3 + 3;
				else
				{
					// ここから UCS-4
					wSurrogate -= 0xD800ul;
					dw = (dw - 0xDC00) | ((DWORD) wSurrogate << 10);
					if (dw < 0x00200000ul)
						dwCalcLength += 4;
					else if (dw < 0x04000000ul)
						dwCalcLength += 5;
					else if (dw < 0x80000000ul)
						dwCalcLength += 6;
					else
						//{ /* unsupported */ }
						dwCalcLength += 3;
				}
				wSurrogate = 0;
			}
			else
			{
				if (dw < 0x0080ul)
					dwCalcLength++;
				else if (dw < 0x0800ul)
					dwCalcLength += 2;
				else if (dw >= 0xD800ul && dw < 0xDC00ul)
				{
					wSurrogate = (WORD) dw;
					continue;
				}
				else //if (dw < 0xD800ul || dw >= 0xDC00ul)
					dwCalcLength += 3;
			}
			// 推奨サイズが指定されているとき、それ以上の最小サイズを返す
			if (!lpBuffer && dwBufferLength && dwCalcLength >= dwBufferLength)
				break;
		}

		SetFilePointer(hFile, lnNowPointer1, &lnNowPointer2, FILE_BEGIN);

		return dwCalcLength;
	}

	dwCalcLength = 0;
	dwBack = 0;
	while (::ReadFile(hFile, &dw, sizeof(WORD), &dwIndex, NULL) && dwIndex)
	{
		if (dwCalcLength >= dwBufferLength)
		{
			dwBack = 1;
			break;
		}
		if (wSurrogate)
		{
			if (dw < 0xDC00ul || dw >= 0xE000ul)
			{
				lpBuffer[dwCalcLength++] = 0xE0u | (BYTE)(wSurrogate >> 12);
				if (dwCalcLength >= dwBufferLength - 1)
					break;
				lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((wSurrogate >> 6) & 0x3Fu);
				if (dwCalcLength >= dwBufferLength - 1)
					break;
				lpBuffer[dwCalcLength++] = 0x80u | (BYTE)(wSurrogate & 0x3Fu);
				if (dwCalcLength >= dwBufferLength - 1)
					break;
				lpBuffer[dwCalcLength++] = 0xE0u | (BYTE)(dw >> 12);
				if (dwCalcLength >= dwBufferLength - 1)
					break;
				lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 6) & 0x3Fu);
				if (dwCalcLength >= dwBufferLength - 1)
					break;
				lpBuffer[dwCalcLength++] = 0x80u | (BYTE)(dw & 0x3Fu);
			}
			else
			{
				// ここから UCS-4
				wSurrogate -= 0xD800ul;
				dw = (dw - 0xDC00) | ((DWORD) wSurrogate << 10);
				if (dw < 0x00200000ul)
				{
					// 00000000.000wwwxx.xxxxyyyy.yyzzzzzz: 11110www 10xxxxxx 10yyyyyy 10zzzzzz
					lpBuffer[dwCalcLength++] = 0xF0u | (BYTE)(dw >> 18);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 12) & 0x3Fu);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 6) & 0x3Fu);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)(dw & 0x3Fu);
				}
				else if (dw < 0x04000000ul)
				{
					// 000000vv.wwwwwwxx.xxxxyyyy.yyzzzzzz: 111110vv 10wwwwww 10xxxxxx 10yyyyyy 10zzzzzz
					lpBuffer[dwCalcLength++] = 0xF8u | (BYTE)(dw >> 24);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 18) & 0x3Fu);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 12) & 0x3Fu);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 6) & 0x3Fu);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)(dw & 0x3Fu);
				}
				else if (dw < 0x80000000ul)
				{
					// 0uvvvvvv.wwwwwwxx.xxxxyyyy.yyzzzzzz: 1111110u 10vvvvvv 10wwwwww 10xxxxxx 10yyyyyy 10zzzzzz
					lpBuffer[dwCalcLength++] = 0xFCu | (BYTE)(dw >> 30);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 24) & 0x3Fu);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 18) & 0x3Fu);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 12) & 0x3Fu);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 6) & 0x3Fu);
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x80u | (BYTE)(dw & 0x3Fu);
				}
				else
				{
					/* unsupported */
					lpBuffer[dwCalcLength++] = 0xBFu;
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0xBFu;
					if (dwCalcLength >= dwBufferLength - 1)
						break;
					lpBuffer[dwCalcLength++] = 0x3Du;
				}
			}
			wSurrogate = 0;
		}
		else
		{
			if (dw < 0x0080ul)
			{
				lpBuffer[dwCalcLength++] = (BYTE)(dw /*& 0x7F*/);
			}
			else if (dw < 0x0800ul)
			{
				// 00000yyy.yyzzzzzz: 110yyyyy 10zzzzzz
				lpBuffer[dwCalcLength++] = 0xC0u | (BYTE)(dw >> 6);
				if (dwCalcLength >= dwBufferLength - 1)
					break;
				lpBuffer[dwCalcLength++] = 0x80u | (BYTE)(dw & 0x3Fu);
			}
			else if (dw >= 0xD800ul && dw < 0xDC00ul)
				wSurrogate = (WORD) dw;
			else //if (dw < 0xD800ul || dw >= 0xDC00ul)
			{
				// xxxxyyyy.yyzzzzzz: 1110xxxx 10yyyyyy 10zzzzzz
				lpBuffer[dwCalcLength++] = 0xE0u | (BYTE)(dw >> 12);
				if (dwCalcLength >= dwBufferLength - 1)
					break;
				lpBuffer[dwCalcLength++] = 0x80u | (BYTE)((dw >> 6) & 0x3Fu);
				if (dwCalcLength >= dwBufferLength - 1)
					break;
				lpBuffer[dwCalcLength++] = 0x80u | (BYTE)(dw & 0x3Fu);
			}
		}
	}
	if (dwBack)
	{
		//dwBack = (size_t) -dwBack;
		//dw = (DWORD) (dwBack >> 32);
		//dwBack &= 0xFFFFFFFF;
		//SetFilePointer(hFile, ((long) dwBack), (PLONG) &dw, FILE_CURRENT);
		SetFilePointer(hFile, -((long) dwBack), NULL, FILE_CURRENT);
	}
	lpBuffer[dwCalcLength] = 0;
	return dwCalcLength;
}
#endif

static bool __stdcall IsValidUTF8Byte(LPCBYTE lpb, char nLen)
{
	while (nLen--)
	{
		if (*lpb++ < 0x80)
			return false;
	}
	return true;
}

extern "C" bool __stdcall IsUTF8Data(LPCBYTE lpszUTF8, size_t dwByteLength)
{
	size_t dwIndex;
	bool bReadUTF8Flag = false;
	if (dwByteLength == (size_t) -1)
		dwByteLength = strlen((LPCSTR) lpszUTF8);
	for (dwIndex = 0; dwIndex < dwByteLength; )
	{
		if ((lpszUTF8[dwIndex] & 0xFC) == 0xFC)
		{
			if (!IsValidUTF8Byte(&lpszUTF8[dwIndex + 1], 5))
				return false;
			dwIndex += 6;
			bReadUTF8Flag = true;
		}
		else if ((lpszUTF8[dwIndex] & 0xF8) == 0xF8)
		{
			if (!IsValidUTF8Byte(&lpszUTF8[dwIndex + 1], 4))
				return false;
			dwIndex += 5;
			bReadUTF8Flag = true;
		}
		else if ((lpszUTF8[dwIndex] & 0xF0) == 0xF0)
		{
			if (!IsValidUTF8Byte(&lpszUTF8[dwIndex + 1], 3))
				return false;
			dwIndex += 4;
			bReadUTF8Flag = true;
		}
		else if ((lpszUTF8[dwIndex] & 0xE0) == 0xE0)
		{
			if (!IsValidUTF8Byte(&lpszUTF8[dwIndex + 1], 2))
				return false;
			dwIndex += 3;
			bReadUTF8Flag = true;
		}
		else if ((lpszUTF8[dwIndex] & 0xC0) == 0xC0)
		{
			if (!IsValidUTF8Byte(&lpszUTF8[dwIndex + 1], 1))
				return false;
			dwIndex += 2;
			bReadUTF8Flag = true;
		}
		else if ((lpszUTF8[dwIndex] & 0x80) == 0x80)
			return false;
		else
			dwIndex += 1;
	}

	// UTF-8 特有のデータを読んでいない場合は、英語のみのファイル
	return bReadUTF8Flag;
}

extern "C" size_t __stdcall GetUTF8Length(LPCBYTE lpszUTF8, size_t dwMaxByteLength)
{
	if (!dwMaxByteLength)
		return 0;
	if ((*lpszUTF8 & 0xFC) == 0xFC)
		return dwMaxByteLength >= 6 ? 6 : 0;
	if ((*lpszUTF8 & 0xF8) == 0xF8)
		return dwMaxByteLength >= 5 ? 5 : 0;
	if ((*lpszUTF8 & 0xF0) == 0xF0)
		return dwMaxByteLength >= 4 ? 4 : 0;
	if ((*lpszUTF8 & 0xE0) == 0xE0)
		return dwMaxByteLength >= 3 ? 3 : 0;
	if ((*lpszUTF8 & 0xC0) == 0xC0)
		return dwMaxByteLength >= 2 ? 2 : 0;
	if ((*lpszUTF8 & 0x80) == 0x80)
		return (size_t) -1;
	return 1;
}

extern "C" size_t __stdcall CalcActualUTF8Length(LPCBYTE lpszUTF8, size_t dwMaxByteLength)
{
	size_t dwRet = 0, dw;
	while (dwMaxByteLength)
	{
		dw = GetUTF8Length(lpszUTF8, dwMaxByteLength);
		if (!dw || dw == (size_t) -1)
			break;
		dwRet += dw;
		lpszUTF8 += dw;
		dwMaxByteLength -= dw;
	}
	return dwRet;
}

#ifdef _WIN32
extern "C" bool __stdcall IsUTF8File(HANDLE hFile)
{
	DWORD dwIndex;
	BYTE b, bData[5];
	LONG lnNowPointer1, lnNowPointer2;
	bool bFlags = true;
	bool bReadUTF8Flag = false;
	lnNowPointer1 = (LONG) GetVLFilePointer(hFile, &lnNowPointer2);
	while (::ReadFile(hFile, &b, 1, &dwIndex, NULL) && dwIndex)
	{
		if ((b & 0xFC) == 0xFC)
		{
			if (!::ReadFile(hFile, bData, 5, &dwIndex, NULL) ||
				dwIndex != 5 ||
				!IsValidUTF8Byte(bData, 5))
			{
				bFlags = false;
				break;
			}
			bReadUTF8Flag = true;
		}
		else if ((b & 0xF8) == 0xF8)
		{
			if (!::ReadFile(hFile, bData, 4, &dwIndex, NULL) ||
				dwIndex != 4 ||
				!IsValidUTF8Byte(bData, 4))
			{
				bFlags = false;
				break;
			}
			bReadUTF8Flag = true;
		}
		else if ((b & 0xF0) == 0xF0)
		{
			if (!::ReadFile(hFile, bData, 3, &dwIndex, NULL) ||
				dwIndex != 3 ||
				!IsValidUTF8Byte(bData, 3))
			{
				bFlags = false;
				break;
			}
			bReadUTF8Flag = true;
		}
		else if ((b & 0xE0) == 0xE0)
		{
			if (!::ReadFile(hFile, bData, 2, &dwIndex, NULL) ||
				dwIndex != 2 ||
				!IsValidUTF8Byte(bData, 2))
			{
				bFlags = false;
				break;
			}
			bReadUTF8Flag = true;
		}
		else if ((b & 0xC0) == 0xC0)
		{
			if (!::ReadFile(hFile, &b, 1, &dwIndex, NULL) ||
				dwIndex != 1 ||
				!IsValidUTF8Byte(&b, 1))
			{
				bFlags = false;
				break;
			}
			bReadUTF8Flag = true;
		}
		else if ((b & 0x80) == 0x80)
		{
			bFlags = false;
			break;
		}
		else
			{ /* 何もしない */ }
	}

	SetFilePointer(hFile, lnNowPointer1, &lnNowPointer2, FILE_BEGIN);

	return bFlags && bReadUTF8Flag;
}
#endif

extern "C" size_t __stdcall UTF8ToUnicode(LPCBYTE lpszUTF8, size_t dwByteLength, LPWSTR lpBuffer, size_t dwBufferLength)
{
	DWORD dw;
	size_t dwIndex;
	WORD wSurrogate;
	size_t dwCalcLength;
	if (dwByteLength == (size_t) -1)
		dwByteLength = strlen((LPCSTR) lpszUTF8);

	if (!lpBuffer || !dwBufferLength)
	{
		dwCalcLength = 0;
		for (dwIndex = 0; dwIndex < dwByteLength; )
		{
			dw = 0;
			wSurrogate = 0;
			if ((lpszUTF8[dwIndex] & 0xFC) == 0xFC)
			{
				wSurrogate = 1;
				dwIndex += 6;
			}
			else if ((lpszUTF8[dwIndex] & 0xF8) == 0xF8)
			{
				wSurrogate = 1;
				dwIndex += 5;
			}
			else if ((lpszUTF8[dwIndex] & 0xF0) == 0xF0)
			{
				wSurrogate = 1;
				dwIndex += 4;
			}
			else if ((lpszUTF8[dwIndex] & 0xE0) == 0xE0)
				dwIndex += 3;
			else if ((lpszUTF8[dwIndex] & 0xC0) == 0xC0)
				dwIndex += 2;
			else if ((lpszUTF8[dwIndex] & 0x80) == 0x80)
			{
				// 不正なデータ
				dw = 0xFFFFFFFF;
				dwIndex += 1;
			}
			else
				dwIndex += 1;
			if (dwIndex > dwByteLength)
				break;
			if (dw < 0x10000u)
			{
				dwCalcLength++;
				if (wSurrogate)
					dwCalcLength++;
			}
		}

		return dwCalcLength;
	}

	dwCalcLength = 0;
	for (dwIndex = 0; dwIndex < dwByteLength;)
	{
		if (dwCalcLength >= dwBufferLength - 1)
			break;
		wSurrogate = 0;
		// ここから UCS-4
		if ((lpszUTF8[dwIndex] & 0xFC) == 0xFC)
		{
			// 1111110u 10vvvvvv 10wwwwww 10xxxxxx 10yyyyyy 10zzzzzz: 0uvvvvvv.wwwwwwxx.xxxxyyyy.yyzzzzzz
			dw = (DWORD)(lpszUTF8[dwIndex++] & 0x1)   << 30;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F) << 24;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F) << 18;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F) << 12;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F) << 6;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F);
		}
		else if ((lpszUTF8[dwIndex] & 0xF8) == 0xF8)
		{
			// 111110vv 10wwwwww 10xxxxxx 10yyyyyy 10zzzzzz: 000000vv.wwwwwwxx.xxxxyyyy.yyzzzzzz
			dw = (DWORD)(lpszUTF8[dwIndex++] & 0x3)   << 24;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F) << 18;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F) << 12;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F) << 6;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F);
		}
		else if ((lpszUTF8[dwIndex] & 0xF0) == 0xF0)
		{
			// 11110www 10xxxxxx 10yyyyyy 10zzzzzz: 00000000.000wwwxx.xxxxyyyy.yyzzzzzz
			dw = (DWORD)(lpszUTF8[dwIndex++] & 0x7)   << 18;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F) << 12;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F) << 6;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F);
		}
		// ここまで UCS-4
		else if ((lpszUTF8[dwIndex] & 0xE0) == 0xE0)
		{
			// 1110xxxx 10yyyyyy 10zzzzzz: xxxxyyyy.yyzzzzzz
			dw = (DWORD)(lpszUTF8[dwIndex++] & 0x0F)  << 12;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F) << 6;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F);
		}
		else if ((lpszUTF8[dwIndex] & 0xC0) == 0xC0)
		{
			// 110yyyyy 10zzzzzz: 00000yyy.yyzzzzzz
			dw = (DWORD)(lpszUTF8[dwIndex++] & 0x1F)  << 6;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F);
		}
		else if ((lpszUTF8[dwIndex] & 0x80) == 0x80)
		{
			// 不正なデータ: パス
			dw = 0xFFFFFFFF;
			dwIndex++;
		}
		else
			dw = (DWORD) (lpszUTF8[dwIndex++] & 0x7F);

		if (dwIndex > dwByteLength)
			break;
		if (dw != 0xFFFFFFFF)
		{
			if (dw >= 0x10000ul)
			{
				wSurrogate = (WORD)(dw >> 10) | 0xD800;
				dw = (dw & 0x03FF) | 0xDC00;
				lpBuffer[dwCalcLength++] = (WCHAR) wSurrogate;
			}
			lpBuffer[dwCalcLength++] = (WCHAR)(WORD) dw;
		}
	}
	lpBuffer[dwCalcLength] = 0;
	return dwCalcLength;
}

#ifdef _WIN32
extern "C" size_t __stdcall UTF8ToUnicodeFile(LPCBYTE lpszUTF8, size_t dwByteLength, HANDLE hFile, BYTE bEndian)
{
	DWORD dw;
	size_t dwIndex;
	WORD wSurrogate;
	size_t dwCalcLength;
	DWORD dwTemp;
	if (dwByteLength == (size_t) -1)
		dwByteLength = strlen((LPCSTR) lpszUTF8);

	dwCalcLength = 0;
	for (dwIndex = 0; dwIndex < dwByteLength;)
	{
		wSurrogate = 0;
		// ここから UCS-4
		if ((lpszUTF8[dwIndex] & 0xFC) == 0xFC)
		{
			// 1111110u 10vvvvvv 10wwwwww 10xxxxxx 10yyyyyy 10zzzzzz: 0uvvvvvv.wwwwwwxx.xxxxyyyy.yyzzzzzz
			dw = (DWORD)(lpszUTF8[dwIndex++] & 0x1)   << 30;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F) << 24;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F) << 18;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F) << 12;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F) << 6;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F);
		}
		else if ((lpszUTF8[dwIndex] & 0xF8) == 0xF8)
		{
			// 111110vv 10wwwwww 10xxxxxx 10yyyyyy 10zzzzzz: 000000vv.wwwwwwxx.xxxxyyyy.yyzzzzzz
			dw = (DWORD)(lpszUTF8[dwIndex++] & 0x3)   << 24;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F) << 18;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F) << 12;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F) << 6;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F);
		}
		else if ((lpszUTF8[dwIndex] & 0xF0) == 0xF0)
		{
			// 11110www 10xxxxxx 10yyyyyy 10zzzzzz: 00000000.000wwwxx.xxxxyyyy.yyzzzzzz
			dw = (DWORD)(lpszUTF8[dwIndex++] & 0x7)   << 18;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F) << 12;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F) << 6;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F);
		}
		// ここまで UCS-4
		else if ((lpszUTF8[dwIndex] & 0xE0) == 0xE0)
		{
			// 1110xxxx 10yyyyyy 10zzzzzz: xxxxyyyy.yyzzzzzz
			dw = (DWORD)(lpszUTF8[dwIndex++] & 0x0F)  << 12;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F) << 6;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F);
		}
		else if ((lpszUTF8[dwIndex] & 0xC0) == 0xC0)
		{
			// 110yyyyy 10zzzzzz: 00000yyy.yyzzzzzz
			dw = (DWORD)(lpszUTF8[dwIndex++] & 0x1F)  << 6;
			dw |= (DWORD)(lpszUTF8[dwIndex++] & 0x3F);
		}
		else if ((lpszUTF8[dwIndex] & 0x80) == 0x80)
		{
			// 不正なデータ: パス
			dw = 0xFFFFFFFF;
			dwIndex++;
		}
		else
			dw = (DWORD) (lpszUTF8[dwIndex++] & 0x7F);

		if (dwIndex > dwByteLength)
			break;
		if (dw != 0xFFFFFFFF)
		{
			if (dw >= 0x10000ul)
			{
				wSurrogate = (WORD)(dw >> 10) | 0xD800;
				dw = (dw & 0x03FF) | 0xDC00;
				if (bEndian & 0x01)
				{
					dwTemp = wSurrogate / 0x100;
					wSurrogate = ((wSurrogate & 0xFF) * 0x100) | (WORD) dwTemp;
				}
				dwTemp = 0;
				if (!::WriteFile(hFile, &wSurrogate, sizeof(WORD), &dwTemp, NULL) ||
					dwTemp < sizeof(WORD))
				{
					dwCalcLength += dwTemp;
					break;
				}
			}
			if (bEndian & 0x01)
			{
				dwTemp = dw / 0x100;
				dw = ((dw & 0xFF) * 0x100) | dwTemp;
			}
			dwTemp = 0;
			if (!::WriteFile(hFile, &dw, sizeof(WORD), &dwTemp, NULL) ||
				dwTemp < sizeof(WORD))
			{
				dwCalcLength += dwTemp;
				break;
			}
		}
	}
	return dwCalcLength;
}

extern "C" size_t __stdcall UTF8FileToUnicode(HANDLE hFile, LPWSTR lpBuffer, size_t dwBufferLength)
{
	DWORD dw;
	size_t dwCalcLength;
	WORD wSurrogate;
	DWORD dwTemp, dwTemp2;
	BYTE b, bData[5];
	LONG lnNowPointer1, lnNowPointer2;

	if (!lpBuffer || !dwBufferLength)
	{
		lnNowPointer1 = (LONG) GetVLFilePointer(hFile, &lnNowPointer2);
		dwCalcLength = 0;
		while (::ReadFile(hFile, &b, 1, &dwTemp, NULL) && dwTemp)
		{
			dw = 0;
			wSurrogate = 0;
			if ((b & 0xFC) == 0xFC)
			{
				wSurrogate = 1;
				dwTemp = 6;
			}
			else if ((b & 0xF8) == 0xF8)
			{
				wSurrogate = 1;
				dwTemp = 5;
			}
			else if ((b & 0xF0) == 0xF0)
			{
				wSurrogate = 1;
				dwTemp = 4;
			}
			else if ((b & 0xE0) == 0xE0)
				dwTemp = 3;
			else if ((b & 0xC0) == 0xC0)
				dwTemp = 2;
			else if ((b & 0x80) == 0x80)
			{
				dw = 0xFFFFFFFF;
				dwTemp = 1;
			}
			else
				dwTemp = 1;
			if (dwTemp > 1)
			{
				if (!::ReadFile(hFile, bData, dwTemp - 1, &dwTemp2, NULL) ||
					dwTemp2 < dwTemp - 1)
					break;
			}
			if (dw != 0xFFFFFFFF)
			{
				dwCalcLength++;
				if (wSurrogate)
					dwCalcLength++;
				// 推奨サイズが指定されているとき、それ以上の最小サイズを返す
				if (!lpBuffer && dwBufferLength && dwCalcLength >= dwBufferLength)
					break;
			}
		}

		//dw = (DWORD) GetVLFilePointer(hFile, (LPLONG)&dwTemp);
		SetFilePointer(hFile, lnNowPointer1, &lnNowPointer2, FILE_BEGIN);

		return dwCalcLength;
	}

	dwCalcLength = 0;
	while (::ReadFile(hFile, &b, 1, &dwTemp, NULL) && dwTemp)
	{
		wSurrogate = 0;
		// ここから UCS-4
		if ((b & 0xFC) == 0xFC)
		{
			// 1111110u 10vvvvvv 10wwwwww 10xxxxxx 10yyyyyy 10zzzzzz: 0uvvvvvv.wwwwwwxx.xxxxyyyy.yyzzzzzz
			if (!::ReadFile(hFile, bData, 5, &dwTemp, NULL) ||
				dwTemp < 5)
				break;
			dw = (DWORD)(b & 0x1)          << 30;
			dw |= (DWORD)(bData[0] & 0x3F) << 24;
			dw |= (DWORD)(bData[1] & 0x3F) << 18;
			dw |= (DWORD)(bData[2] & 0x3F) << 12;
			dw |= (DWORD)(bData[3] & 0x3F) << 6;
			dw |= (DWORD)(bData[4] & 0x3F);
		}
		else if ((b & 0xF8) == 0xF8)
		{
			// 111110vv 10wwwwww 10xxxxxx 10yyyyyy 10zzzzzz: 000000vv.wwwwwwxx.xxxxyyyy.yyzzzzzz
			if (!::ReadFile(hFile, bData, 4, &dwTemp, NULL) ||
				dwTemp < 4)
				break;
			dw = (DWORD)(b & 0x3)          << 24;
			dw |= (DWORD)(bData[0] & 0x3F) << 18;
			dw |= (DWORD)(bData[1] & 0x3F) << 12;
			dw |= (DWORD)(bData[2] & 0x3F) << 6;
			dw |= (DWORD)(bData[3] & 0x3F);
		}
		else if ((b & 0xF0) == 0xF0)
		{
			// 11110www 10xxxxxx 10yyyyyy 10zzzzzz: 00000000.000wwwxx.xxxxyyyy.yyzzzzzz
			if (!::ReadFile(hFile, bData, 3, &dwTemp, NULL) ||
				dwTemp < 3)
				break;
			dw = (DWORD)(b & 0x7)          << 18;
			dw |= (DWORD)(bData[0] & 0x3F) << 12;
			dw |= (DWORD)(bData[1] & 0x3F) << 6;
			dw |= (DWORD)(bData[2] & 0x3F);
		}
		// ここまで UCS-4
		else if ((b & 0xE0) == 0xE0)
		{
			// 1110xxxx 10yyyyyy 10zzzzzz: xxxxyyyy.yyzzzzzz
			if (!::ReadFile(hFile, bData, 2, &dwTemp, NULL) ||
				dwTemp < 2)
				break;
			dw = (DWORD)(b & 0xF)          << 12;
			dw |= (DWORD)(bData[0] & 0x3F) << 6;
			dw |= (DWORD)(bData[1] & 0x3F);
		}
		else if ((b & 0xC0) == 0xC0)
		{
			// 110yyyyy 10zzzzzz: 00000yyy.yyzzzzzz
			if (!::ReadFile(hFile, bData, 1, &dwTemp, NULL) ||
				dwTemp < 1)
				break;
			dw = (DWORD)(b & 0x1F)         << 6;
			dw |= (DWORD)(bData[0] & 0x3F);
		}
		else if ((b & 0x80) == 0x80)
		{
			// 不正なデータ: パス
			dw = 0xFFFFFFFF;
		}
		else
			dw = (DWORD) (b & 0x7F);

		if (dw != 0xFFFFFFFF)
		{
			if (dw >= 0x10000ul)
			{
				wSurrogate = (WORD)(dw >> 10) | 0xD800;
				dw = (dw & 0x03FF) | 0xDC00;
				lpBuffer[dwCalcLength++] = (WCHAR) wSurrogate;
			}
			lpBuffer[dwCalcLength++] = (WCHAR)(WORD) dw;
		}

		if (dwCalcLength >= dwBufferLength)
			break;
	}
	if (dwCalcLength < dwBufferLength)
		lpBuffer[dwCalcLength] = 0;
	return dwCalcLength;
}
#endif

///////////////////////////////////////////////////////////////////////////////

static size_t __stdcall _UTF32Length(const DWORD* pdwData)
{
	size_t nLen = 0;
	while (*pdwData++)
		nLen++;
	return nLen;
}

extern "C" size_t __stdcall CalcUTF32ToUnicodeLength(const char* pszBuffer, size_t nLen, size_t* pnReadLen, bool bBigEndian)
{
	if (nLen == (size_t) -1)
		nLen = _UTF32Length((const DWORD*) pszBuffer) * 4 + 4;
	const DWORD* pdw = (const DWORD*) pszBuffer;
	size_t nReadLen = 0, nRet = 0;
	while (nReadLen + 4 <= nLen)
	{
		DWORD dw = *pdw;
		if (bBigEndian)
			dw = MAKELONG(MAKEWORD(HIBYTE(HIWORD(dw)), LOBYTE(HIWORD(dw))),
				MAKEWORD(HIBYTE(LOWORD(dw)), LOBYTE(LOWORD(dw))));
		if (dw >= 0x10000ul)
			nRet += 2;
		else
			nRet++;
		pdw++;
		nReadLen += 4;
	}
	if (pnReadLen)
		*pnReadLen = nReadLen;
	return nRet;
}

extern "C" size_t __stdcall UTF32ToUnicode(const char* pszBuffer, size_t nLen, size_t* pnReadLen,
	LPWSTR lpszBuffer, size_t nBufferLen, bool bBigEndian)
{
	if (!lpszBuffer || !nBufferLen)
		return CalcUTF32ToUnicodeLength(pszBuffer, nLen, pnReadLen, bBigEndian);

	if (nLen == (size_t) -1)
		nLen = _UTF32Length((const DWORD*) pszBuffer) * 4 + 4;
	const DWORD* pdw = (const DWORD*) pszBuffer;
	size_t nReadLen = 0, nRet = 0;
	while (nReadLen + 4 <= nLen)
	{
		DWORD dw = *pdw;
		if (bBigEndian)
			dw = MAKELONG(MAKEWORD(HIBYTE(HIWORD(dw)), LOBYTE(HIWORD(dw))),
				MAKEWORD(HIBYTE(LOWORD(dw)), LOBYTE(LOWORD(dw))));
		if (dw >= 0x10000ul)
		{
			if (nBufferLen < 2)
				break;
			WORD wSurrogate = (WORD)(dw >> 10) | 0xD800;
			dw = (dw & 0x03FF) | 0xDC00;
			*lpszBuffer++ = (WCHAR) wSurrogate;
			nBufferLen--;
			nRet++;
		}
		if (nBufferLen < 1)
			break;
		*lpszBuffer++ = (WCHAR) dw;
		nBufferLen--;
		nRet++;
		pdw++;
		nReadLen += 4;
	}
	if (pnReadLen)
		*pnReadLen = nReadLen;
	return nRet;
}

///////////////////////////////////////////////////////////////////////////////

// 半角になったときは ch2 を 0 にする
static void __stdcall EUCCharsToShiftJISChars(BYTE& ch1, BYTE& ch2)
{
	if (!IsEUCFirstChar(ch1))
	{
		ch2 = 0;
		return;
	}
	if (ch1 == 0x8E)
	{
		ch1 = ch2;
		ch2 = 0;
		return;
	}
	else if (ch1 == 0x8F)
	{
		// 定義されていない → 「〓」を設定
		ch1 = 0x81;
		ch2 = 0xAC;
		return;
	}
	//ch1 -= 0x80;
	//ch2 -= 0x80;
	//ch1 -= 0x21;
	//if (ch1 & 0x01)
	//	ch2 += 0x7E;
	//else
	//{
	//	ch2 += 0x1F;
	//	if (ch2 >= 0x7F)
	//		ch2++;
	//}
	//ch1 >>= 0x01;
	//if (ch1 < 0x1F)
	//	ch1 += 0x81;
	//else
	//	ch1 += 0xC1;
	if (ch1 & 0x01)
	{
		ch2 -= 0x61;
		if (ch2 >= 0x7F)
			ch2++;
	}
	else
		ch2 -= 0x02;
	ch1 = (ch1 < 0xDF ? 0x30 : 0x70) + (ch1 & 0x01) + (ch1 >> 1);
}

// 半角になったときは ch2 を 0 にする
static void __stdcall ShiftJISCharsToEUCChars(BYTE& ch1, BYTE& ch2)
{
	if (!IsShiftJISFirstChar(ch1))
	{
		ch2 = 0;
	}
	else if (ch1 >= 0xA0 && ch1 <= 0xDF)
	{
		ch2 = ch1;
		ch1 = 0x8E;
	}
	else
	{
		ch1 = (BYTE)(((int) ch1 << 1) - (ch1 >= 0xE0 ? 0xE0 : 0x60));
		if (ch2 >= 0x9F)
			ch2 += 2;
		else
		{
			ch1--;
			ch2 += 0x60;
			if (ch2 < 0xDF)
				ch2++;
		}
	}
}

extern "C" bool __stdcall ShiftJISToEUCString(LPSTR lpBuffer, size_t dwBufferLength)
{
	size_t dwPos;
	BYTE b;
	if (dwBufferLength == (size_t) -1)
		dwBufferLength = strlen(lpBuffer);
	for (dwPos = 0; dwPos < dwBufferLength; dwPos++, lpBuffer++)
	{
		if (IsShiftJISFirstChar((BYTE) *lpBuffer) && dwPos < dwBufferLength - 1)
		{
			b = *((LPBYTE) lpBuffer + 1);
			ShiftJISCharsToEUCChars(*((LPBYTE) lpBuffer), b);
			if (b)
			{
				lpBuffer++;
				dwPos++;
				*lpBuffer = (CHAR) b;
			}
		}
	}
	return true;
}

extern "C" bool __stdcall EUCToShiftJISString(LPSTR lpBuffer, size_t dwBufferLength)
{
	size_t dwPos;
	BYTE b;
	if (dwBufferLength == (size_t) -1)
		dwBufferLength = strlen(lpBuffer);
	for (dwPos = 0; dwPos < dwBufferLength; dwPos++, lpBuffer++)
	{
		if (IsEUCFirstChar((BYTE) *lpBuffer) && dwPos < dwBufferLength - 1)
		{
			b = *((LPBYTE) lpBuffer + 1);
			EUCCharsToShiftJISChars(*((LPBYTE) lpBuffer), b);
			if (b)
			{
				lpBuffer++;
				dwPos++;
				*lpBuffer = (CHAR) b;
			}
		}
	}
	return true;
}

#ifdef _WIN32
static BYTE s_bKeepEUCCharCalc = 0;
static BYTE s_bKeepEUCCharRead = 0;

extern "C" size_t __stdcall EUCFileToShiftJIS(HANDLE hFile, LPSTR lpBuffer, size_t dwBufferLength)
{
	size_t dwCalcLength;
	DWORD dwTemp;
	BYTE b1, b2;
	LPSTR lpTemp;
	LONG lnNowPointer1, lnNowPointer2;

	if (!lpBuffer || !dwBufferLength)
	{
		lnNowPointer1 = (LONG) GetVLFilePointer(hFile, &lnNowPointer2);
		dwCalcLength = 0;
		if (s_bKeepEUCCharCalc)
		{
			b1 = s_bKeepEUCCharCalc;
			s_bKeepEUCCharCalc = 0;
			goto OnLoopCalcLength;
		}
		while (::ReadFile(hFile, &b1, 1, &dwTemp, NULL) && dwTemp)
		{
OnLoopCalcLength:
			dwTemp = 1;
			if (IsEUCFirstChar(b1))
			{
				if (!::ReadFile(hFile, &b2, 1, &dwTemp, NULL) ||
					dwTemp != 1)
				{
					s_bKeepEUCCharCalc = b1;
					break;
				}
				EUCCharsToShiftJISChars(b1, b2);
				if (b2)
					dwTemp++;
			}

			// 推奨サイズが指定されているとき、それ以下の最大サイズを返す
			if (!lpBuffer && dwBufferLength && dwCalcLength + dwTemp > dwBufferLength)
				break;

			dwCalcLength += dwTemp;
		}

		//dw = (DWORD) GetVLFilePointer(hFile, (LPLONG)&dwTemp);
		SetFilePointer(hFile, lnNowPointer1, &lnNowPointer2, FILE_BEGIN);

		return dwCalcLength;
	}

	dwCalcLength = 0;
	lpTemp = lpBuffer;
	if (s_bKeepEUCCharRead)
	{
		b1 = s_bKeepEUCCharRead;
		s_bKeepEUCCharRead = 0;
		goto OnLoopFileRead;
	}
	while (::ReadFile(hFile, &b1, 1, &dwTemp, NULL) && dwTemp)
	{
OnLoopFileRead:
		if (IsEUCFirstChar(b1))
		{
			if (!::ReadFile(hFile, &b2, 1, &dwTemp, NULL) ||
				dwTemp != 1)
			{
				s_bKeepEUCCharRead = b1;
				break;
			}
			EUCCharsToShiftJISChars(b1, b2);
			dwCalcLength += (dwTemp = b2 ? 2 : 1);
		}
		else
		{
			b2 = 0;
			dwCalcLength++;
		}
		if (dwBufferLength < dwCalcLength)
		{
			SetFilePointer(hFile, -((LONG) dwTemp), NULL, FILE_CURRENT);
			break;
		}
		*lpBuffer++ = b1;
		if (b2)
			*lpBuffer++ = b2;
	}
	return (size_t) (lpBuffer - lpTemp) / sizeof(CHAR);
}

extern "C" size_t __stdcall ShiftJISToEUCFile(LPCSTR lpBuffer, size_t dwBufferLength, HANDLE hFile, BOOL* pbResult)
{
	size_t dwCalcLength;
	DWORD dwTemp;
	BYTE b[2];

	dwCalcLength = 0;
	if (pbResult)
		*pbResult = TRUE;
	while (dwBufferLength--)
	{
		b[0] = *lpBuffer++;
		if (IsShiftJISFirstChar(b[0]))
		{
			if (!dwBufferLength--)
				break;
			b[1] = *lpBuffer++;
			ShiftJISCharsToEUCChars(b[0], b[1]);
			if (!b[1])
				goto OnWriteSingle;
			if (!::WriteFile(hFile, b, 2, &dwTemp, NULL) ||
				dwTemp != 2)
			{
				*pbResult = FALSE;
				break;
			}
			dwCalcLength += 2;
		}
		else
		{
OnWriteSingle:
			if (!::WriteFile(hFile, b, 1, &dwTemp, NULL) ||
				dwTemp != 1)
			{
				*pbResult = FALSE;
				break;
			}
			dwCalcLength++;
		}
	}
	return dwCalcLength;
}
#endif

///////////////////////////////////////////////////////////////////////////////

//static void __stdcall ShiftJISCharsToJISChars(BYTE& ch1, BYTE& ch2)
//{
//	register BYTE _ch1 = ch1, _ch2 = ch2;
//	if (_ch1 <= 0x9F)
//		_ch1 -= 0x71;
//	else
//		_ch1 -= 0xB1;
//
//	_ch1 <<= 1;
//	_ch1++;
//
//	if (_ch2 >= 0x7F)
//		_ch2--;
//
//	if (_ch2 >= 0x9E)
//	{
//		_ch2 -= 0x7D;
//		_ch1++;
//	}
//	else
//		_ch2 -= 0x1F;
//	ch1 = _ch1;
//	ch2 = _ch2;
//}
//
//static void __stdcall JISCharsToShiftJISChars(BYTE& ch1, BYTE& ch2)
//{
//	register BYTE _ch1 = ch1, _ch2 = ch2;
//	if (_ch1 & 1)
//		_ch2 += 0x1F;
//	else
//		_ch2 += 0x7D;
//
//	if (_ch2 >= 0x7F)
//		_ch2++;
//
//	_ch1 = (_ch1 - 0x21) / 2 + 0x81;
//
//	if (_ch1 >= 0x9E)
//		_ch1 += 0x40;
//	ch1 = _ch1;
//	ch2 = _ch2;
//}
//
//extern "C" bool __stdcall ShiftJISToJISString(LPCSTR lpszShiftJIS, size_t dwLength, LPBYTE lpBuffer, size_t dwBufferLength)
//{
//	size_t dwPos, dwCalcLength;
//	BYTE b;
//	if (dwLength == (size_t) -1)
//		dwLength = strlen(lpszShiftJIS);
//	dwCalcLength = 0;
//	for (dwPos = 0; dwPos < dwLength; dwPos++)
//	{
//	}
//	for (dwPos = 0; dwPos < dwBufferLength; dwPos++, lpBuffer++)
//	{
//		if (IsShiftJISFirstChar((BYTE) *lpBuffer) && dwPos < dwBufferLength - 1)
//		{
//			b = *((LPBYTE) lpBuffer + 1);
//			ShiftJISCharsToJISChars(*((LPBYTE) lpBuffer), b);
//			if (b)
//			{
//				lpBuffer++;
//				dwPos++;
//				*lpBuffer = (CHAR) b;
//			}
//		}
//	}
//	return true;
//}
//
//extern "C" bool __stdcall EUCToShiftJISString(LPSTR lpBuffer, size_t dwBufferLength)
//{
//	size_t dwPos;
//	BYTE b;
//	if (dwBufferLength == (size_t) -1)
//		dwBufferLength = strlen(lpBuffer);
//	for (dwPos = 0; dwPos < dwBufferLength; dwPos++, lpBuffer++)
//	{
//		if (IsEUCFirstChar((BYTE) *lpBuffer) && dwPos < dwBufferLength - 1)
//		{
//			b = *((LPBYTE) lpBuffer + 1);
//			JISCharsToShiftJISChars(*((LPBYTE) lpBuffer), b);
//			if (b)
//			{
//				lpBuffer++;
//				dwPos++;
//				*lpBuffer = (CHAR) b;
//			}
//		}
//	}
//	return true;
//}
