/*
 Copyright (C) 2010 jet (ジェット)

 INIFile.cpp - implementations of functions for INI file
 */

#include "stdafx.h"
#include "INIFile.h"

#include <ctype.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

typedef const BYTE FAR* LPCBYTE;

struct CINIFileData
{
	HANDLE hFile;
	HANDLE hMap;
	LPVOID lpMap;
	bool bUseComment;
};

struct CSectionData
{
	CINIFileData* pINIFile;
	// セクションのデータ(セクション名の後ろ)の開始位置
	LPCBYTE lpSectionPos;
};

extern int __stdcall FormatByteStringExW(LPWSTR lpszBuffer, int nBufferLen, LPCVOID lpData, DWORD dwSize);
extern DWORD __stdcall UnformatByteStringExW(LPCWSTR lpszBuffer, LPVOID lpBuffer, DWORD dwSize);
extern bool __stdcall GetDWordFromStringW(LPCWSTR lpszString, DWORD& dwRet);
extern void __stdcall GetStringFromDWordW(long nValue, LPWSTR lpszBuffer, int nMinCount, BYTE cType);

static /*__declspec(naked)*/ LPCBYTE __stdcall GetLastPosition(LPCBYTE lp)
{
	while (*lp)
		lp++;
	return (LPCBYTE) lp;
//	__asm
//	{
//		mov    eax, dword ptr[esp + 4]
//		cmp    eax, 0
//		jz     OnExit
//
//		push   edi
//		push   ecx
//		push   esi
//		mov    edi, eax
//
//		xor    eax, eax
//		repne  scasb
//		dec    edi
//		mov    eax, edi
//
//		pop    edi
//		pop    ecx
//		pop    esi
//
//OnExit:
//		ret    04h
//	}
}

HINIFILE __stdcall MyLoadINIFileA(LPCSTR lpszFileName, bool bUseComment)
{
	CINIFileData* pINIFile;
	DWORD dwL, dwH;

	pINIFile = (CINIFileData*) malloc(sizeof(CINIFileData));
	if (!pINIFile)
		return NULL;

	pINIFile->hFile = ::CreateFileA(lpszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (pINIFile->hFile == INVALID_HANDLE_VALUE || !pINIFile->hFile)
	{
		free(pINIFile);
		return NULL;
	}

	dwL = ::GetFileSize(pINIFile->hFile, &dwH);
	if (dwL == 0 && dwH == 0)
	{
		pINIFile->hMap = INVALID_HANDLE_VALUE;
		pINIFile->lpMap = NULL;
	}
	else
	{
		pINIFile->hMap = ::CreateFileMapping(pINIFile->hFile, NULL, PAGE_READONLY, 0, 0, NULL);
		if (pINIFile->hMap == INVALID_HANDLE_VALUE)
		{
			::CloseHandle(pINIFile->hFile);
			free(pINIFile);
			return NULL;
		}
		pINIFile->lpMap = ::MapViewOfFile(pINIFile->hMap, FILE_MAP_READ, 0, 0, 0);
		if (pINIFile->lpMap == NULL)
		{
			::CloseHandle(pINIFile->hMap);
			::CloseHandle(pINIFile->hFile);
			free(pINIFile);
			return NULL;
		}
	}
	pINIFile->bUseComment = bUseComment;
	return (HINIFILE) pINIFile;
}

HINIFILE __stdcall MyLoadINIFileW(LPCWSTR lpszFileName, bool bUseComment)
{
	CINIFileData* pINIFile;
	DWORD dwL, dwH;

	pINIFile = (CINIFileData*) malloc(sizeof(CINIFileData));
	if (!pINIFile)
		return NULL;

	pINIFile->hFile = ::CreateFileW(lpszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ((pINIFile->hFile == INVALID_HANDLE_VALUE || !pINIFile->hFile) && ::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
	{
		dwL = (DWORD) ::WideCharToMultiByte(CP_ACP, 0, lpszFileName, -1, NULL, 0, NULL, NULL);
		LPSTR lp = (LPSTR) malloc(sizeof(char) * dwL);
		::WideCharToMultiByte(CP_ACP, 0, lpszFileName, -1, lp, (int) dwL, NULL, NULL);
		lp[dwL - 1] = 0;
		pINIFile->hFile = ::CreateFileA(lp, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		free(lp);
	}
	if (pINIFile->hFile == INVALID_HANDLE_VALUE || !pINIFile->hFile)
	{
		free(pINIFile);
		return NULL;
	}

	dwL = ::GetFileSize(pINIFile->hFile, &dwH);
	if (dwL == 0 && dwH == 0)
	{
		pINIFile->hMap = INVALID_HANDLE_VALUE;
		pINIFile->lpMap = NULL;
	}
	else
	{
		pINIFile->hMap = ::CreateFileMapping(pINIFile->hFile, NULL, PAGE_READONLY, 0, 0, NULL);
		if (pINIFile->hMap == INVALID_HANDLE_VALUE)
		{
			::CloseHandle(pINIFile->hFile);
			free(pINIFile);
			return NULL;
		}
		pINIFile->lpMap = ::MapViewOfFile(pINIFile->hMap, FILE_MAP_READ, 0, 0, 0);
		if (pINIFile->lpMap == NULL)
		{
			::CloseHandle(pINIFile->hMap);
			::CloseHandle(pINIFile->hFile);
			free(pINIFile);
			return NULL;
		}
	}
	pINIFile->bUseComment = bUseComment;
	return (HINIFILE) pINIFile;
}

void __stdcall MyCloseINIFile(HINIFILE hINIFile)
{
	CINIFileData* pFile;

	pFile = (CINIFileData*) hINIFile;
	if (pFile && pFile->hFile != INVALID_HANDLE_VALUE)
	{
		if (pFile->lpMap != NULL)
			::UnmapViewOfFile(pFile->lpMap);
		if (pFile->hMap != INVALID_HANDLE_VALUE)
			::CloseHandle(pFile->hMap);
		::CloseHandle(pFile->hFile);
		free(pFile);
	}
}

static bool __stdcall IsEqualStringNoCaseWithCharW(LPCBYTE lpUTF8, LPCWSTR lpsz2, size_t nUnicodeLen, WCHAR wchLast)
{
	DWORD dw2;
	WCHAR wch;
	if (!nUnicodeLen || nUnicodeLen == (size_t) -1)
		nUnicodeLen = wcslen(lpsz2);
	if (wchLast)
		nUnicodeLen++;
	while (nUnicodeLen--)
	{
		// ここから UCS-4
		if ((*lpUTF8 & 0xFC) == 0xFC)
		{
			// 1111110u 10vvvvvv 10wwwwww 10xxxxxx 10yyyyyy 10zzzzzz: 0uvvvvvv.wwwwwwxx.xxxxyyyy.yyzzzzzz
			dw2 = (DWORD)(*lpUTF8++ & 0x1)   << 30;
			dw2 |= (DWORD)(*lpUTF8++ & 0x3F) << 24;
			dw2 |= (DWORD)(*lpUTF8++ & 0x3F) << 18;
			dw2 |= (DWORD)(*lpUTF8++ & 0x3F) << 12;
			dw2 |= (DWORD)(*lpUTF8++ & 0x3F) << 6;
			dw2 |= (DWORD)(*lpUTF8++ & 0x3F);
		}
		else if ((*lpUTF8 & 0xF8) == 0xF8)
		{
			// 111110vv 10wwwwww 10xxxxxx 10yyyyyy 10zzzzzz: 000000vv.wwwwwwxx.xxxxyyyy.yyzzzzzz
			dw2 = (DWORD)(*lpUTF8++ & 0x3)   << 24;
			dw2 |= (DWORD)(*lpUTF8++ & 0x3F) << 18;
			dw2 |= (DWORD)(*lpUTF8++ & 0x3F) << 12;
			dw2 |= (DWORD)(*lpUTF8++ & 0x3F) << 6;
			dw2 |= (DWORD)(*lpUTF8++ & 0x3F);
		}
		else if ((*lpUTF8 & 0xF0) == 0xF0)
		{
			// 11110www 10xxxxxx 10yyyyyy 10zzzzzz: 00000000.000wwwxx.xxxxyyyy.yyzzzzzz
			dw2 = (DWORD)(*lpUTF8++ & 0x7)   << 18;
			dw2 |= (DWORD)(*lpUTF8++ & 0x3F) << 12;
			dw2 |= (DWORD)(*lpUTF8++ & 0x3F) << 6;
			dw2 |= (DWORD)(*lpUTF8++ & 0x3F);
		}
		// ここまで UCS-4
		else if ((*lpUTF8 & 0xE0) == 0xE0)
		{
			// 1110xxxx 10yyyyyy 10zzzzzz: xxxxyyyy.yyzzzzzz
			dw2 = (DWORD)(*lpUTF8++ & 0xF)   << 12;
			dw2 |= (DWORD)(*lpUTF8++ & 0x3F) << 6;
			dw2 |= (DWORD)(*lpUTF8++ & 0x3F);
		}
		else if ((*lpUTF8 & 0xC0) == 0xC0)
		{
			// 110yyyyy 10zzzzzz: 00000yyy.yyzzzzzz
			dw2 = (DWORD)(*lpUTF8++ & 0x1F)  << 6;
			dw2 |= (DWORD)(*lpUTF8++ & 0x3F);
		}
		else if ((*lpUTF8 & 0x80) == 0x80)
		{
			// 不正なデータ: パス
			dw2 = 0xFFFFFFFF;
		}
		else
			dw2 = (DWORD) (*lpUTF8++ & 0x7F);

		if (dw2 != 0xFFFFFFFF)
		{
			if (dw2 >= 0x10000ul)
			{
				if (wchLast && !nUnicodeLen)
					return false;
				if (((WORD)(dw2 >> 10) | 0xD800) != *lpsz2++)
					return false;
				nUnicodeLen--;
				wch = (WCHAR) ((dw2 & 0x03FF) | 0xDC00);
			}
			else
				wch = (WCHAR) dw2;
			if (wchLast && !nUnicodeLen)
			{
				if (towlower(wch) != towlower(wchLast))
					return false;
			}
			else
			{
				if (towlower(wch) != towlower(*lpsz2++))
					return false;
			}
		}
		else
			return false;
		if (!*lpUTF8)
			return false;
	}
	return true;
}

//static bool __stdcall IsEqualStringNoCaseW(LPCBYTE lpUTF8, LPCWSTR lpsz2, size_t nUnicodeLen)
//{
//	return IsEqualStringNoCaseWithCharW(lpUTF8, lpsz2, nUnicodeLen, 0);
//}

//static LPCBYTE __stdcall FindNoCaseW(LPCBYTE lpUTF8, LPCWSTR lpszFind, size_t nUnicodeLen)
//{
//	while (*lpUTF8)
//	{
//		if (IsEqualStringNoCaseW(lpUTF8, lpszFind, nUnicodeLen))
//			return lpUTF8;
//		lpUTF8++;
//	}
//	return NULL;
//}

//static LPCBYTE __stdcall FindNoCaseW(LPCBYTE lpUTF8, LPCWSTR lpszFind)
//{
//	return FindNoCaseW(lpUTF8, lpszFind, (size_t) -1);
//}

static DWORD __stdcall _MyCalcUTF8Length(LPCWSTR lpszString)
{
	DWORD dwCalcLength, dwIndex, dw, dwLength;
	WORD wSurrogate;
	dwLength = (DWORD) wcslen(lpszString);
	dwCalcLength = 0;
	wSurrogate = 0;
	for (dwIndex = 0; dwIndex < dwLength; dwIndex++)
	{
		dw = ((DWORD) lpszString[dwIndex]) & 0xFFFF;
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

static LPCBYTE __stdcall FindSectionPosW(LPCBYTE lpszTarget, LPCWSTR lpszSection)
{
	size_t nSize = wcslen(lpszSection);
	LPCBYTE lp = (LPCBYTE) strchr((LPCSTR) lpszTarget, '[');
	while (lp)
	{
		if (lp == lpszTarget || *(lp - 1) == (BYTE) '\r' || *(lp - 1) == (BYTE) '\n')
		{
			lp++;
			if (IsEqualStringNoCaseWithCharW(lp, lpszSection, nSize, L']'))
			{
				return lp - 1;
			}
		}
		else
			lp++;
		lp = (LPCBYTE) strchr((LPCSTR) lp, '[');
	}
	return NULL;
}

static LPCBYTE __stdcall GetNextReturn(LPCBYTE lpsz)
{
	LPCBYTE lp1 = (LPCBYTE) strchr((LPCSTR) lpsz, '\r');
	LPCBYTE lp2 = (LPCBYTE) strchr((LPCSTR) lpsz, '\n');
	if (!lp1)
	{
		if (!lp2)
			return GetLastPosition(lpsz);
		lp1 = lp2;
	}
	else if (!lp2)
		return lp1;
	return min(lp1, lp2);
}

static LPCBYTE __stdcall SkipReturn(LPCBYTE lpsz)
{
	if (*lpsz == (BYTE) '\r' || *lpsz == (BYTE) '\n')
	{
		if (*lpsz == (BYTE) '\r' && *(lpsz + 1) == (BYTE) '\n')
			lpsz++;
		lpsz++;
	}
	return lpsz;
}

PVOID __stdcall MyGetProfileSectionW(HINIFILE hINIFile, LPCWSTR lpszSection)
{
	CINIFileData* pFile;
	if (hINIFile == NULL)
		return NULL;
	pFile = (CINIFileData*) hINIFile;
	if (pFile->hFile == INVALID_HANDLE_VALUE || !pFile->lpMap)
		return NULL;

	LPCBYTE lp;
	if (lpszSection)
	{
		lp = FindSectionPosW((LPCBYTE) pFile->lpMap, lpszSection);
		if (!lp)
			return NULL;
	}

	CSectionData* pData = (CSectionData*) malloc(sizeof(CSectionData));
	pData->pINIFile = pFile;
	if (lpszSection)
	{
		pData->lpSectionPos = SkipReturn(GetNextReturn(lp + _MyCalcUTF8Length(lpszSection) + 2));
	}
	else
	{
		pData->lpSectionPos = (LPCBYTE) pFile->lpMap;
	}
	return pData;
}

LPWSTR __stdcall MyGetProfileStringW(PVOID lpSectionBuffer, LPCWSTR lpszKey, LPCWSTR lpszDefault)
{
	if (!lpSectionBuffer)
	{
		if (lpszDefault)
			return _wcsdup(lpszDefault);
		return NULL;
	}

	CSectionData* pData = (CSectionData*) lpSectionBuffer;

	LPWSTR lpBuffer, lpEqual, lpw;
	DWORD dw, dw2, dwCur;
	WORD wSurrogate;

	LPCBYTE lpLast = (LPCBYTE) strchr((LPCSTR) pData->lpSectionPos, '[');
	if (!lpLast)
		lpLast = GetLastPosition(pData->lpSectionPos);
	LPCBYTE lp = pData->lpSectionPos;
	LPCBYTE lpNext;
	while (lp && (!lpLast || lp < lpLast))
	{
		lpNext = GetNextReturn(lp);
		if (lp != lpNext)
		{
			dw = (DWORD) (lpNext - lp);

			// Unicode string length <= UTF-8 (binary) length
			lpBuffer = (LPWSTR) malloc(sizeof(WCHAR) * (dw + 1));
			dwCur = 0;

			while (lp < lpNext)
			{
				// ここから UCS-4
				if ((*lp & 0xFC) == 0xFC)
				{
					// 1111110u 10vvvvvv 10wwwwww 10xxxxxx 10yyyyyy 10zzzzzz: 0uvvvvvv.wwwwwwxx.xxxxyyyy.yyzzzzzz
					dw2 = (DWORD)(*lp++ & 0x1)   << 30;
					dw2 |= (DWORD)(*lp++ & 0x3F) << 24;
					dw2 |= (DWORD)(*lp++ & 0x3F) << 18;
					dw2 |= (DWORD)(*lp++ & 0x3F) << 12;
					dw2 |= (DWORD)(*lp++ & 0x3F) << 6;
					dw2 |= (DWORD)(*lp++ & 0x3F);
				}
				else if ((*lp & 0xF8) == 0xF8)
				{
					// 111110vv 10wwwwww 10xxxxxx 10yyyyyy 10zzzzzz: 000000vv.wwwwwwxx.xxxxyyyy.yyzzzzzz
					dw2 = (DWORD)(*lp++ & 0x3)   << 24;
					dw2 |= (DWORD)(*lp++ & 0x3F) << 18;
					dw2 |= (DWORD)(*lp++ & 0x3F) << 12;
					dw2 |= (DWORD)(*lp++ & 0x3F) << 6;
					dw2 |= (DWORD)(*lp++ & 0x3F);
				}
				else if ((*lp & 0xF0) == 0xF0)
				{
					// 11110www 10xxxxxx 10yyyyyy 10zzzzzz: 00000000.000wwwxx.xxxxyyyy.yyzzzzzz
					dw2 = (DWORD)(*lp++ & 0x7)   << 18;
					dw2 |= (DWORD)(*lp++ & 0x3F) << 12;
					dw2 |= (DWORD)(*lp++ & 0x3F) << 6;
					dw2 |= (DWORD)(*lp++ & 0x3F);
				}
				// ここまで UCS-4
				else if ((*lp & 0xE0) == 0xE0)
				{
					// 1110xxxx 10yyyyyy 10zzzzzz: xxxxyyyy.yyzzzzzz
					dw2 = (DWORD)(*lp++ & 0xF)   << 12;
					dw2 |= (DWORD)(*lp++ & 0x3F) << 6;
					dw2 |= (DWORD)(*lp++ & 0x3F);
				}
				else if ((*lp & 0xC0) == 0xC0)
				{
					// 110yyyyy 10zzzzzz: 00000yyy.yyzzzzzz
					dw2 = (DWORD)(*lp++ & 0x1F)  << 6;
					dw2 |= (DWORD)(*lp++ & 0x3F);
				}
				else if ((*lp & 0x80) == 0x80)
				{
					// 不正なデータ: パス
					dw2 = 0xFFFFFFFF;
					lp++;
				}
				else
					dw2 = (DWORD) (*lp++ & 0x7F);

				if (dw2 != 0xFFFFFFFF)
				{
					if (dw2 >= 0x10000ul)
					{
						wSurrogate = (WORD)(dw2 >> 10) | 0xD800;
						dw2 = (dw2 & 0x03FF) | 0xDC00;
						lpBuffer[dwCur++] = (WCHAR) wSurrogate;
					}
					lpBuffer[dwCur++] = (WCHAR)(WORD) dw2;
				}
			}
			lpBuffer[dwCur] = 0;

			// could find
			lpEqual = wcschr(lpBuffer, L'=');
			if (lpEqual)
			{
				*lpEqual++ = 0;
				if (_wcsicmp(lpszKey, lpBuffer) == 0)
				{
					if (pData->pINIFile->bUseComment)
					{
						lpw = wcschr(lpEqual, L';');
						if (lpw)
							*lpw = 0;
					}
					lpEqual = _wcsdup(lpEqual);
					free(lpBuffer);
					// dwSectionPos はそのまま
					return lpEqual;
				}
			}
			free(lpBuffer);
		}

		lp = SkipReturn(lpNext);
	}
	if (lpszDefault)
		return _wcsdup(lpszDefault);
	return NULL;
}

int __stdcall MyGetProfileIntW(PVOID lpSectionBuffer, LPCWSTR lpszKey, int nDefault)
{
	if (!lpSectionBuffer)
		return nDefault;
	LPWSTR lp = MyGetProfileStringW(lpSectionBuffer, lpszKey, NULL);
	if (lp)
	{
		DWORD dw;
		if (!GetDWordFromStringW(lp, dw))
		{
			free(lp);
			return nDefault;
		}
		free(lp);
		return (int) dw;
	}
	return nDefault;
}

DWORD __stdcall MyGetProfileDWordW(PVOID lpSectionBuffer, LPCWSTR lpszKey, DWORD dwDefault)
{
	if (!lpSectionBuffer)
		return dwDefault;
	LPWSTR lp = MyGetProfileStringW(lpSectionBuffer, lpszKey, NULL);
	if (lp)
	{
		DWORD dw;
		if (!GetDWordFromStringW(lp, dw))
		{
			free(lp);
			return dwDefault;
		}
		free(lp);
		return dw;
	}
	return dwDefault;
}

bool __stdcall MyGetProfileBooleanW(PVOID lpSectionBuffer, LPCWSTR lpszKey, bool bDefault)
{
	if (!lpSectionBuffer)
		return bDefault;
	LPWSTR lp = MyGetProfileStringW(lpSectionBuffer, lpszKey, NULL);
	if (lp)
	{
		DWORD dw;
		if (!GetDWordFromStringW(lp, dw))
		{
			if (_wcsicmp(lp, L"true") == 0 || _wcsicmp(lp, L"yes") == 0)
				bDefault = true;
			if (_wcsicmp(lp, L"false") == 0 || _wcsicmp(lp, L"no") == 0)
				bDefault = false;
			free(lp);
			return bDefault;
		}
		free(lp);
		return dw != 0;
	}
	return bDefault;
}

DWORD __stdcall MyGetProfileBinaryW(PVOID lpSectionBuffer, LPCWSTR lpszKey, LPVOID lpBuffer, DWORD dwSize)
{
	if (!lpSectionBuffer)
		return 0;
	LPWSTR lp = MyGetProfileStringW(lpSectionBuffer, lpszKey, NULL);
	if (lp)
	{
		DWORD dw;
		dw = UnformatByteStringExW(lp, NULL, (DWORD) -1);
		if (lpBuffer)
		{
			if (dw <= dwSize)
				UnformatByteStringExW(lp, lpBuffer, dw);
			else
				dw = 0;
		}
		free(lp);
		return dw;
	}
	return 0;
}

LPWSTR __stdcall MyGetNextProfileStringW(PVOID lpSectionBuffer, int nIndex, LPWSTR* lplpszValue)
{
	if (!lpSectionBuffer)
	{
		if (lplpszValue)
			*lplpszValue = NULL;
		return NULL;
	}

	CSectionData* pData = (CSectionData*) lpSectionBuffer;

	LPWSTR lpBuffer, lpEqual, lpw;
	DWORD dw, dw2, dwCur;
	WORD wSurrogate;
	int nPos;

	LPCBYTE lpLast = (LPCBYTE) strchr((LPCSTR) pData->lpSectionPos, '[');
	if (!lpLast)
		lpLast = GetLastPosition(pData->lpSectionPos);
	LPCBYTE lp = pData->lpSectionPos;
	LPCBYTE lpNext;
	nPos = 0;
	while (lp && (!lpLast || lp < lpLast))
	{
		lpNext = GetNextReturn(lp);
		if (lp != lpNext)
		{
			dw = (DWORD) (lpNext - lp);

			// Unicode string length <= UTF-8 (binary) length
			lpBuffer = (LPWSTR) malloc(sizeof(WCHAR) * (dw + 1));
			dwCur = 0;

			while (lp < lpNext)
			{
				// ここから UCS-4
				if ((*lp & 0xFC) == 0xFC)
				{
					// 1111110u 10vvvvvv 10wwwwww 10xxxxxx 10yyyyyy 10zzzzzz: 0uvvvvvv.wwwwwwxx.xxxxyyyy.yyzzzzzz
					dw2 = (DWORD)(*lp++ & 0x1)   << 30;
					dw2 |= (DWORD)(*lp++ & 0x3F) << 24;
					dw2 |= (DWORD)(*lp++ & 0x3F) << 18;
					dw2 |= (DWORD)(*lp++ & 0x3F) << 12;
					dw2 |= (DWORD)(*lp++ & 0x3F) << 6;
					dw2 |= (DWORD)(*lp++ & 0x3F);
				}
				else if ((*lp & 0xF8) == 0xF8)
				{
					// 111110vv 10wwwwww 10xxxxxx 10yyyyyy 10zzzzzz: 000000vv.wwwwwwxx.xxxxyyyy.yyzzzzzz
					dw2 = (DWORD)(*lp++ & 0x3)   << 24;
					dw2 |= (DWORD)(*lp++ & 0x3F) << 18;
					dw2 |= (DWORD)(*lp++ & 0x3F) << 12;
					dw2 |= (DWORD)(*lp++ & 0x3F) << 6;
					dw2 |= (DWORD)(*lp++ & 0x3F);
				}
				else if ((*lp & 0xF0) == 0xF0)
				{
					// 11110www 10xxxxxx 10yyyyyy 10zzzzzz: 00000000.000wwwxx.xxxxyyyy.yyzzzzzz
					dw2 = (DWORD)(*lp++ & 0x7)   << 18;
					dw2 |= (DWORD)(*lp++ & 0x3F) << 12;
					dw2 |= (DWORD)(*lp++ & 0x3F) << 6;
					dw2 |= (DWORD)(*lp++ & 0x3F);
				}
				// ここまで UCS-4
				else if ((*lp & 0xE0) == 0xE0)
				{
					// 1110xxxx 10yyyyyy 10zzzzzz: xxxxyyyy.yyzzzzzz
					dw2 = (DWORD)(*lp++ & 0xF)   << 12;
					dw2 |= (DWORD)(*lp++ & 0x3F) << 6;
					dw2 |= (DWORD)(*lp++ & 0x3F);
				}
				else if ((*lp & 0xC0) == 0xC0)
				{
					// 110yyyyy 10zzzzzz: 00000yyy.yyzzzzzz
					dw2 = (DWORD)(*lp++ & 0x1F)  << 6;
					dw2 |= (DWORD)(*lp++ & 0x3F);
				}
				else if ((*lp & 0x80) == 0x80)
				{
					// 不正なデータ: パス
					dw2 = 0xFFFFFFFF;
				}
				else
					dw2 = (DWORD) (*lp++ & 0x7F);

				if (dw2 != 0xFFFFFFFF)
				{
					if (dw2 >= 0x10000ul)
					{
						wSurrogate = (WORD)(dw2 >> 10) | 0xD800;
						dw2 = (dw2 & 0x03FF) | 0xDC00;
						lpBuffer[dwCur++] = (WCHAR) wSurrogate;
					}
					lpBuffer[dwCur++] = (WCHAR)(WORD) dw2;
				}
			}
			lpBuffer[dwCur] = 0;

			// could find
			lpEqual = wcschr(lpBuffer, L'=');
			if (lpEqual)
			{
				*lpEqual++ = 0;
				if (nPos == nIndex)
				{
					if (pData->pINIFile->bUseComment)
					{
						lpw = wcschr(lpEqual, L';');
						if (lpw)
							*lpw = 0;
					}
					*lplpszValue = _wcsdup(lpEqual);
					lpEqual = _wcsdup(lpBuffer);
					free(lpBuffer);
					return lpEqual;
				}

				nPos++;
			}
			free(lpBuffer);
		}

		lp = SkipReturn(lpNext);
	}
	if (lplpszValue)
		*lplpszValue = NULL;
	return NULL;
}

void __stdcall MyEndReadProfileSectionW(PVOID lpSectionBuffer)
{
	if (lpSectionBuffer)
		free(lpSectionBuffer);
}

///////////////////////////////////////////////////////////////////////////////

#include "Convert.h"

void __stdcall MyWriteStringW(HANDLE hFile, LPCWSTR lpszString)
{
	if (lpszString)
		::UnicodeToUTF8File(lpszString, (size_t) -1, hFile, NULL);
}

void __stdcall MyWriteCRLFW(HANDLE hFile)
{
	DWORD dw;
	// "\r\n" will be treated as UTF-8.
	::WriteFile(hFile, "\r\n", 2 * sizeof(CHAR), &dw, NULL);
}

void __stdcall MyWriteStringLineW(HANDLE hFile, LPCWSTR lpszString)
{
	MyWriteStringW(hFile, lpszString);
	MyWriteCRLFW(hFile);
}

static const CHAR s_szINIBracket[] = "[]";

void __stdcall MyWriteINISectionW(HANDLE hFile, LPCWSTR lpszSectionName)
{
	DWORD dw;
	::WriteFile(hFile, s_szINIBracket, (DWORD) (1 * sizeof(CHAR)), &dw, NULL);
	::UnicodeToUTF8File(lpszSectionName, (size_t) -1, hFile, NULL);
	::WriteFile(hFile, &s_szINIBracket[1], (DWORD) (1 * sizeof(CHAR)), &dw, NULL);
	MyWriteCRLFW(hFile);
}

void __stdcall MyWriteINIValueW(HANDLE hFile, LPCWSTR lpszKey, LPCWSTR lpszValue)
{
	DWORD dw;
	if (!lpszKey)
		return;
	::UnicodeToUTF8File(lpszKey, (size_t) -1, hFile, NULL);
	::WriteFile(hFile, "=", (DWORD) sizeof(CHAR), &dw, NULL);
	if (lpszValue)
		::UnicodeToUTF8File(lpszValue, (size_t) -1, hFile, NULL);
	MyWriteCRLFW(hFile);
}

void __stdcall MyWriteINIValueW(HANDLE hFile, LPCWSTR lpszKey, int nValue)
{
	WCHAR szBuffer[11];
	if (!lpszKey)
		return;
	_itow_s(nValue, szBuffer, 10);
	MyWriteINIValueW(hFile, lpszKey, szBuffer);
}

void __stdcall MyWriteINIValueW(HANDLE hFile, LPCWSTR lpszKey, DWORD dwValue, BYTE cType)
{
	WCHAR szBuffer[30];
	if (!lpszKey)
		return;
	GetStringFromDWordW((long) dwValue, szBuffer, 0, cType);
	MyWriteINIValueW(hFile, lpszKey, szBuffer);
}

void __stdcall MyWriteINIValueW(HANDLE hFile, LPCWSTR lpszKey, LPCVOID lpBuffer, DWORD dwSize)
{
	LPWSTR lp;
	int nLen;
	if (!lpszKey)
		return;
	nLen = FormatByteStringExW(NULL, 0, lpBuffer, dwSize);
	lp = (LPWSTR) malloc(sizeof(WCHAR) * (nLen + 1));
	FormatByteStringExW(lp, nLen + 1, lpBuffer, dwSize);
	MyWriteINIValueW(hFile, lpszKey, lp);
	free(lp);
}
