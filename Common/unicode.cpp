/*
 Copyright (C) 2010 jet (ジェット)

 unicode.cpp - implementations of utility functions for Unicode
 */

#include "stdafx.h"
#include "unicode.h"

#include "AppClass.h"
#include "MyFunc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

LPWSTR __stdcall AnsiToUnicodeStr(LPCSTR lpszString, UINT uLen)
{
	UINT u;
	LPWSTR lpw;
	u = (UINT) ::MultiByteToWideChar(CP_ACP, 0, lpszString, (int) uLen, NULL, 0);
	if (uLen != (UINT) -1)
		u++;
	lpw = (LPWSTR) malloc(sizeof(WCHAR) * u);
	lpw[u - 1] = 0;
	::MultiByteToWideChar(CP_ACP, 0, lpszString, (int) uLen, lpw, (int) u);
	return lpw;
}

static LPSTR __stdcall _MyLoadStringA(HINSTANCE hInstance, UINT uID)
{
	LPSTR szTemp, szRet;
	int nSize, nLen;
	if (!hInstance)
		hInstance = ::GetModuleHandle(NULL);
	szTemp = (LPSTR) malloc(sizeof(CHAR) * 256);
	szTemp[0] = 0;
	nSize = 256;
	nLen = ::LoadStringA(hInstance, uID, szTemp, nSize);
	if (nSize - nLen > 2)
	{
		if (nLen > 0)
		{
			szRet = (LPSTR) malloc(sizeof(CHAR) * (nLen + 1));
			memcpy(szRet, szTemp, sizeof(CHAR) * nLen);
			szRet[nLen] = 0;
		}
		else
			szRet = NULL;
		free(szTemp);
		return szRet;
	}

	do
	{
		nSize += 256;
		szTemp = (LPSTR) realloc(szTemp, sizeof(CHAR) * (nSize));
		szTemp[0] = 0;
		nLen = ::LoadStringA(hInstance, uID, szTemp, nSize);
	} while (nSize - nLen <= 2);
	szRet = (LPSTR) malloc(sizeof(CHAR) * (nLen + 1));
	memcpy(szRet, szTemp, sizeof(CHAR) * nLen);
	szRet[nLen] = 0;
	free(szTemp);
	return szRet;
}

LPWSTR __stdcall UniLoadStringW(HINSTANCE hInstance, UINT uID)
{
	LPWSTR szTemp, szRet;
	int nSize, nLen;
	if (!hInstance)
	{
		hInstance = MyGetCurrentInstance();
		if (!hInstance)
			hInstance = ::GetModuleHandle(NULL);
	}
	szTemp = (LPWSTR) malloc(sizeof(WCHAR) * 10);
	szTemp[0] = 0;
	nSize = 10;
	nLen = ::LoadStringW(hInstance, uID, szTemp, nSize);
	if (nSize - nLen > 1)
	{
		if (nLen > 0)
		{
			szRet = (LPWSTR) malloc(sizeof(WCHAR) * (nLen + 1));
			memcpy(szRet, szTemp, sizeof(WCHAR) * nLen);
			szRet[nLen] = 0;
		}
		else if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
		{
			free(szTemp);
			szRet = AnsiToUnicodeStr((LPSTR) (szTemp = (LPWSTR) _MyLoadStringA(hInstance, uID)), (UINT) -1);
		}
		else
			szRet = NULL;
		free(szTemp);
		return szRet;
	}

	do
	{
		nSize += 256;
		szTemp = (LPWSTR) realloc(szTemp, sizeof(WCHAR) * (nSize));
		szTemp[0] = 0;
		nLen = ::LoadStringW(hInstance, uID, szTemp, nSize);
	} while (nSize - nLen <= 2);
	szRet = (LPWSTR) malloc(sizeof(WCHAR) * (nLen + 1));
	memcpy(szRet, szTemp, sizeof(WCHAR) * nLen);
	szRet[nLen] = 0;
	free(szTemp);
	return szRet;
}

EXTERN_C HANDLE WINAPI MyFindFirstFileW(__in LPCWSTR lpFileName, __out LPWIN32_FIND_DATAW lpFindFileData)
{
	HANDLE hRet;
	hRet = ::FindFirstFileW(lpFileName, lpFindFileData);
	if (::GetLastError() != ERROR_CALL_NOT_IMPLEMENTED)
		return hRet;

	CMyStringW str(lpFileName);
	WIN32_FIND_DATAA wfdA;
	hRet = ::FindFirstFileA(str, &wfdA);
	if (hRet != INVALID_HANDLE_VALUE)
	{
		lpFindFileData->dwFileAttributes = wfdA.dwFileAttributes;
		lpFindFileData->ftCreationTime = wfdA.ftCreationTime;
		lpFindFileData->ftLastAccessTime = wfdA.ftLastAccessTime;
		lpFindFileData->ftLastWriteTime = wfdA.ftLastWriteTime;
		lpFindFileData->nFileSizeHigh = wfdA.nFileSizeHigh;
		lpFindFileData->nFileSizeLow = wfdA.nFileSizeLow;
		lpFindFileData->dwReserved0 = wfdA.dwReserved0;
		lpFindFileData->dwReserved1 = wfdA.dwReserved1;
		::MultiByteToWideChar(CP_ACP, 0, wfdA.cFileName, MAX_PATH, lpFindFileData->cFileName, MAX_PATH);
		::MultiByteToWideChar(CP_ACP, 0, wfdA.cAlternateFileName, 14, lpFindFileData->cAlternateFileName, 14);
#ifdef _MAC
		lpFindFileData->dwFileType = wfdA.dwFileType;
		lpFindFileData->dwCreatorType = wfdA.dwCreatorType;
		lpFindFileData->wFinderFlags = wfdA.wFinderFlags;
#endif
	}
	else
		memset(lpFindFileData, 0, sizeof(WIN32_FIND_DATAW));
	return hRet;
}

EXTERN_C BOOL WINAPI MyFindNextFileW(__in  HANDLE hFindFile, __out LPWIN32_FIND_DATAW lpFindFileData)
{
	BOOL bRet;
	bRet = ::FindNextFileW(hFindFile, lpFindFileData);
	if (::GetLastError() != ERROR_CALL_NOT_IMPLEMENTED)
		return bRet;

	WIN32_FIND_DATAA wfdA;
	wfdA.dwFileAttributes = lpFindFileData->dwFileAttributes;
	wfdA.ftCreationTime = lpFindFileData->ftCreationTime;
	wfdA.ftLastAccessTime = lpFindFileData->ftLastAccessTime;
	wfdA.ftLastWriteTime = lpFindFileData->ftLastWriteTime;
	wfdA.nFileSizeHigh = lpFindFileData->nFileSizeHigh;
	wfdA.nFileSizeLow = lpFindFileData->nFileSizeLow;
	wfdA.dwReserved0 = lpFindFileData->dwReserved0;
	wfdA.dwReserved1 = lpFindFileData->dwReserved1;
	::WideCharToMultiByte(CP_ACP, 0, lpFindFileData->cFileName, MAX_PATH, wfdA.cFileName, MAX_PATH, NULL, NULL);
	::WideCharToMultiByte(CP_ACP, 0, lpFindFileData->cAlternateFileName, 14, wfdA.cAlternateFileName, 14, NULL, NULL);
#ifdef _MAC
	wfdA.dwFileType = lpFindFileData->dwFileType;
	wfdA.dwCreatorType = lpFindFileData->dwCreatorType;
	wfdA.wFinderFlags = lpFindFileData->wFinderFlags;
#endif
	bRet = ::FindNextFileA(hFindFile, &wfdA);
	if (bRet)
	{
		lpFindFileData->dwFileAttributes = wfdA.dwFileAttributes;
		lpFindFileData->ftCreationTime = wfdA.ftCreationTime;
		lpFindFileData->ftLastAccessTime = wfdA.ftLastAccessTime;
		lpFindFileData->ftLastWriteTime = wfdA.ftLastWriteTime;
		lpFindFileData->nFileSizeHigh = wfdA.nFileSizeHigh;
		lpFindFileData->nFileSizeLow = wfdA.nFileSizeLow;
		lpFindFileData->dwReserved0 = wfdA.dwReserved0;
		lpFindFileData->dwReserved1 = wfdA.dwReserved1;
		::MultiByteToWideChar(CP_ACP, 0, wfdA.cFileName, MAX_PATH, lpFindFileData->cFileName, MAX_PATH);
		::MultiByteToWideChar(CP_ACP, 0, wfdA.cAlternateFileName, 14, lpFindFileData->cAlternateFileName, 14);
#ifdef _MAC
		lpFindFileData->dwFileType = wfdA.dwFileType;
		lpFindFileData->dwCreatorType = wfdA.dwCreatorType;
		lpFindFileData->wFinderFlags = wfdA.wFinderFlags;
#endif
	}
	return bRet;
}

EXTERN_C HANDLE WINAPI MyCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess,
	DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	HANDLE hFile;
	::SetLastError(ERROR_SUCCESS);
	hFile = ::CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
		dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
	{
		CMyStringW str(lpFileName);
		return ::CreateFileA(str, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
			dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
	return hFile;
}

EXTERN_C BOOL WINAPI MyCreateDirectoryW(LPCWSTR lpszDirectoryName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	::SetLastError(ERROR_SUCCESS);
	BOOL b = ::CreateDirectoryW(lpszDirectoryName, lpSecurityAttributes);
	if (!b && ::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
	{
		CMyStringW str(lpszDirectoryName);
		b = ::CreateDirectoryA(str, lpSecurityAttributes);
	}
	return b;
}

EXTERN_C BOOL WINAPI MyRemoveDirectoryW(LPCWSTR lpPathName)
{
	::SetLastError(ERROR_SUCCESS);
	BOOL b = ::RemoveDirectoryW(lpPathName);
	if (!b && ::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
	{
		CMyStringW str(lpPathName);
		b = ::RemoveDirectoryA(str);
	}
	return b;
}

EXTERN_C BOOL WINAPI MyDeleteFileW(LPCWSTR lpszFileName)
{
	::SetLastError(ERROR_SUCCESS);
	BOOL b = ::DeleteFileW(lpszFileName);
	if (!b && ::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
	{
		CMyStringW str(lpszFileName);
		b = ::DeleteFileA(str);
	}
	return b;
}

EXTERN_C BOOL WINAPI MyIsEmptyDirectoryW(LPCWSTR lpszPathName)
{
	if (!lpszPathName || !*lpszPathName)
		return FALSE;
	if (!::MyIsDirectoryW(lpszPathName))
		return FALSE;
	bool bLastIsNotDelimiter;
	WIN32_FIND_DATAW wfd;
	HANDLE hFind;
	CMyStringW str(lpszPathName);
	BOOL bFound = FALSE;

	if (bLastIsNotDelimiter = (lpszPathName[str.GetLength() - 1] != L'\\'))
		str += L'\\';
	str += L'*';
	hFind = ::MyFindFirstFileW(str, &wfd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		while (true)
		{
			if (wfd.cFileName[0] != L'.' || (wfd.cFileName[1] &&
				(wfd.cFileName[1] != L'.' || wfd.cFileName[2])))
			{
				bFound = TRUE;
				break;
			}
			if (!::MyFindNextFileW(hFind, &wfd))
				break;
		}
		::FindClose(hFind);
	}
	return !bFound;
}

EXTERN_C int WINAPI MyRemoveDirectoryRecursiveW(LPCWSTR lpPathName)
{
	if (!lpPathName || !*lpPathName)
		return -1;
	if (!::MyIsDirectoryW(lpPathName))
		return -1;
	bool bLastIsNotDelimiter;
	WIN32_FIND_DATAW wfd;
	HANDLE hFind;
	CMyStringW str(lpPathName);
	int ret = 1;

	if (bLastIsNotDelimiter = (lpPathName[str.GetLength() - 1] != L'\\'))
		str += L'\\';
	str += L'*';
	hFind = ::MyFindFirstFileW(str, &wfd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		while (true)
		{
			if (wfd.cFileName[0] != L'.' || (wfd.cFileName[1] &&
				(wfd.cFileName[1] != L'.' || wfd.cFileName[2])))
			{
				str = lpPathName;
				if (bLastIsNotDelimiter)
					str += L'\\';
				str += wfd.cFileName;
				if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (::MyRemoveDirectoryRecursiveW(str) <= 0)
						ret = 0;
				}
				else
				{
					if (!::MyDeleteFileW(str))
						ret = 0;
				}
			}
			if (!::MyFindNextFileW(hFind, &wfd))
				break;
		}
		::FindClose(hFind);
	}
	if (!::MyRemoveDirectoryW(lpPathName))
		ret = 0;
	return ret;
}

extern "C" bool __stdcall MyIsExistFileW(LPCWSTR lpszFileName)
{
	::SetLastError(ERROR_SUCCESS);
	DWORD dw = ::GetFileAttributesW(lpszFileName);
	if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
	{
		CMyStringW str(lpszFileName);
		dw = ::GetFileAttributesA(str);
	}
	return (dw != 0xFFFFFFFF);
}

extern "C" bool __stdcall MyIsDirectoryW(LPCWSTR lpszFileName)
{
	DWORD dw = ::GetFileAttributesW(lpszFileName);
	if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
	{
		CMyStringW str(lpszFileName);
		dw = ::GetFileAttributesA(str);
	}
	return dw == 0xFFFFFFFF ? false : (dw & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

extern "C" int __stdcall MySearchPathW(LPCWSTR lpszPathName, LPWSTR lpszBuffer, int nMaxLen)
{
	LPWSTR lp;
	int ret = (int) ::SearchPathW(NULL, lpszPathName, NULL, (DWORD) nMaxLen, lpszBuffer, &lp);
	if (!ret && ::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
	{
		int nLenTemp = MAX_PATH;
		LPSTR lpBuff = (LPSTR) malloc(sizeof(char) * (nLenTemp + 1));
		if (!lpBuff)
		{
			::SetLastError(ERROR_OUTOFMEMORY);
			return 0;
		}
		CMyStringW str(lpszPathName);
		while (true)
		{
			*lpBuff = 0;
			ret = (int) ::SearchPathA(NULL, str, NULL, (DWORD) nLenTemp, lpBuff, NULL);
			if (!ret)
				break;
			if (ret <= nLenTemp)
			{
				lpBuff[ret] = 0;
				nLenTemp = ::MyGetAnsiStringLenAsUnicode(lpBuff);
				if (nMaxLen && lpszBuffer)
				{
					ret = ::MyCopyAnsiToUnicode(lpBuff, lpszBuffer, nMaxLen);
					if (ret < nLenTemp)
						ret = nLenTemp + 1;
				}
				else
					ret = nLenTemp + 1;
				break;
			}
			nLenTemp += MAX_PATH;
			LPSTR lp = (LPSTR) realloc(lpBuff, sizeof(char) * (nLenTemp + 1));
			if (!lp)
			{
				::SetLastError(ERROR_OUTOFMEMORY);
				ret = 0;
				break;
			}
			lpBuff = lp;
		}
		free(lpBuff);
	}
	return ret;
}

////////////////////////////////////////////////////////////////////////////////

void __stdcall MyRemoveDotsFromPathStringW(CMyStringW& rstrPath)
{
	LPWSTR lp;
	lp = rstrPath.GetBuffer();
	MyRemoveDotsFromPathW(lp, lp);
	rstrPath.ReleaseBuffer();
}

void __stdcall MyGetFullPathStringW(LPCWSTR lpszPath, LPCWSTR lpszFile, CMyStringW& rstrBuffer)
{
	int nLen;
	nLen = MyGetFullPathW(lpszPath, lpszFile, NULL, 0) + 1;
	nLen = MyGetFullPathW(lpszPath, lpszFile,
		rstrBuffer.GetBuffer((DWORD) nLen)
		, nLen);
	rstrBuffer.ReleaseBuffer((DWORD) nLen);
}

void __stdcall MyGetAbsolutePathStringW(LPCWSTR lpszRelativePathName, LPCWSTR lpszDirectory, CMyStringW& rstrBuffer)
{
	LPWSTR lp;
	// Copy lpszDirectory as-is if it is a relative path
	// * The path which begin with "\" is treated as an absolute path
	if (!MyIsBackSlashW(*lpszDirectory) &&
		lpszDirectory[1] != L':')
	{
		rstrBuffer = lpszRelativePathName;
		return;
	}

	// Copy lpszRelativePathName as-is if it is an absolute path
	// * The path which begin with "\" is treated as an absolute path
	if (MyIsBackSlashW(*lpszRelativePathName) ||
		(lpszRelativePathName[1] == L':'))
	{
		rstrBuffer = lpszRelativePathName;
		return;
	}

	MyGetFullPathStringW(lpszDirectory, lpszRelativePathName, rstrBuffer);
	lp = rstrBuffer.GetBuffer();
	MyRemoveDotsFromPathW(lp, lp);
	rstrBuffer.ReleaseBuffer();
}

void __stdcall MyGetRelativePathStringW(LPCWSTR lpszFullPathName, LPCWSTR lpszDirectory, CMyStringW& rstrBuffer)
{
	int nLen;
	nLen = MyGetRelativePathW(lpszFullPathName, lpszDirectory, NULL, 0) + 1;
	nLen = MyGetRelativePathW(lpszFullPathName, lpszDirectory,
		rstrBuffer.GetBuffer((DWORD) nLen)
		, nLen);
	rstrBuffer.ReleaseBuffer((DWORD) nLen);
}

void __stdcall MyMakeFullPathFromCurDirStringW(LPCWSTR lpszPathName, CMyStringW& rstrBuffer)
{
	int nLen;
	nLen = MyMakeFullPathFromCurDirW(lpszPathName, NULL, 0) + 1;
	nLen = MyMakeFullPathFromCurDirW(lpszPathName,
		rstrBuffer.GetBuffer((DWORD) nLen)
		, nLen);
	rstrBuffer.ReleaseBuffer((DWORD) nLen);
}

bool __stdcall MySearchPathStringW(LPCWSTR lpszPathName, CMyStringW& rstrBuffer)
{
//	CMyStringW strPathName(lpszPathName);
	int nLen;
//	nLen = MySearchPath(strPathName, NULL, 0) + 1;
//	nLen = MySearchPath(strPathName,
//#ifndef _UNICODE
//		rstrBuffer.GetBufferA((DWORD) nLen)
//#else
//		rstrBuffer.GetBuffer((DWORD) nLen)
//#endif
//		, nLen);
//#ifndef _UNICODE
//	rstrBuffer.ReleaseBufferA(TRUE, (DWORD) nLen);
//#else
//	rstrBuffer.ReleaseBuffer((DWORD) nLen);
//#endif
	nLen = MySearchPathW(lpszPathName, NULL, 0);
	if (!nLen)
		return false;
	nLen++;
	nLen = MySearchPathW(lpszPathName, rstrBuffer.GetBuffer((DWORD) nLen), nLen);
	rstrBuffer.ReleaseBuffer((DWORD) nLen);
	return nLen > 0;
}

void __stdcall MyGetLongPathNameStringW(LPCWSTR lpszPath, CMyStringW& rstrBuffer)
{
	DWORD nLen;
	nLen = MyGetLongPathNameW(lpszPath, NULL, 0) + 1;
	nLen = MyGetLongPathNameW(lpszPath,
		rstrBuffer.GetBuffer(nLen)
		, nLen);
	rstrBuffer.ReleaseBuffer(nLen);
}

int __stdcall MyGetFileTitleStringW(LPCWSTR lpszFile, CMyStringW& rstrFileTitle)
{
	int n = MyGetFileTitleW(lpszFile, NULL, 0) + 1;
	n = MyGetFileTitleW(lpszFile, rstrFileTitle.GetBuffer((DWORD) n), n);
	rstrFileTitle.ReleaseBuffer((DWORD) n);
	return n;
}

void __stdcall MyReplaceStringW(CMyStringW& rstrText, LPCWSTR lpszFind, LPCWSTR lpszReplace)
{
	size_t nCount;
	size_t nFindLen;
	size_t nReplaceLen;
	LPCWSTR lpc;
	LPWSTR lp, lpBuffer;
	size_t uLength, uNewLength;
	if (rstrText.IsEmpty())
		return;
	nFindLen = wcslen(lpszFind);
	nReplaceLen = wcslen(lpszReplace);
	uLength = rstrText.GetLength();
	nCount = 0;
	lpc = wcsstr(rstrText, lpszFind);
	while (lpc)
	{
		lpc++;
		nCount++;
		lpc = wcsstr(lpc, lpszFind);
	}
	if (!nCount)
		return;
	uNewLength = uLength + (nReplaceLen - nFindLen) * nCount;
	if (uNewLength > uLength)
		lpBuffer = rstrText.GetBuffer(uNewLength);
	else
		lpBuffer = rstrText.GetBuffer(0);
	lp = wcsstr(lpBuffer, lpszFind);
	while (lp)
	{
		nCount = uLength - ((int) (lp - lpBuffer)) - nFindLen + 1;
		memmove(lp + nReplaceLen, lp + nFindLen, sizeof(WCHAR) * nCount);
		memcpy(lp, lpszReplace, sizeof(WCHAR) * nReplaceLen);
		uLength += nReplaceLen - nFindLen;
		lp = wcsstr(lp + nReplaceLen, lpszFind);
	}
	rstrText.ReleaseBuffer(uNewLength);
}

#ifdef _OBJBASE_H_
BSTR __stdcall MyStringToBSTR(
#ifndef OLE2ANSI
	const
#endif
	CMyStringW& string)
{
#ifdef OLE2ANSI
	return ::SysAllocStringLen(string.IsEmpty() ? "" : string, (UINT) string.GetLengthA());
#else
	return ::SysAllocStringLen(string.IsEmpty() ? L"" : string, (UINT)string.GetLength());
#endif
}

void __stdcall MyBSTRToString(BSTR bstr, CMyStringW& rstrString)
{
	if (!bstr)
		rstrString.Empty();
	else
		rstrString.SetString(bstr, ::SysStringLen(bstr));
}
#endif

#if defined(GUID_DEFINED) && defined(__LPGUID_DEFINED__) && defined(_REFGUID_DEFINED)
void __stdcall MyStringFromGUIDW(REFGUID rguid, CMyStringW& rstrRet)
{
	rstrRet.Format(L"{%08lX-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		rguid.Data1, rguid.Data2, rguid.Data3,
		rguid.Data4[0], rguid.Data4[1], rguid.Data4[2], rguid.Data4[3],
		rguid.Data4[4], rguid.Data4[5], rguid.Data4[6], rguid.Data4[7]);
}
#endif

size_t __stdcall MyGetUnalignedStringLen(const WCHAR UNALIGNED* pwstr)
{
	auto* p = reinterpret_cast<const BYTE UNALIGNED*>(pwstr);
	while (p[0] || p[1])
		p += 2;
	return static_cast<size_t>(reinterpret_cast<const WCHAR UNALIGNED*>(p) - pwstr);
}

void __stdcall MyGetUnalignedString(const WCHAR UNALIGNED* pwstr, CMyStringW& rstrString)
{
	auto len = MyGetUnalignedStringLen(pwstr);
	auto out = rstrString.GetBuffer(len);
	memcpy(out, pwstr, sizeof(WCHAR) * len);
}

#ifdef _SHLOBJ_H_
EXTERN_C int __stdcall MyDragQueryFileCount(HDROP hDrop)
{
	DROPFILES FAR* pDrop = (DROPFILES FAR*) ::GlobalLock((HGLOBAL) hDrop);
	bool bWide = (pDrop->fWide != 0);
	union
	{
		LPBYTE lpb;
		LPWSTR lpw;
	};
	lpb = (((LPBYTE) pDrop) + pDrop->pFiles);
	int ret = 0;
	while (true)
	{
		if (bWide)
		{
			if (!*lpw)
				break;
			while (*lpw++);
		}
		else
		{
			if (!*lpb)
				break;
			while (*lpb++);
		}
		ret++;
	}
	::GlobalUnlock((HDROP) hDrop);
	return ret;
}

void __stdcall MyDragQueryFileStringW(HDROP hDrop, UINT iFile, CMyStringW& rstrFile)
{
	DROPFILES FAR* pDrop = (DROPFILES FAR*) ::GlobalLock((HGLOBAL) hDrop);
	bool bWide = (pDrop->fWide != 0);
	union
	{
		LPBYTE lpb;
		LPWSTR lpw;
	};
	lpb = (((LPBYTE) pDrop) + pDrop->pFiles);
	while (true)
	{
		if (bWide)
		{
			if (!*lpw)
				break;
			if (!iFile)
			{
				rstrFile = lpw;
				break;
			}
			while (*lpw++);
		}
		else
		{
			if (!*lpb)
				break;
			if (!iFile)
			{
				rstrFile = (LPSTR) lpb;
				break;
			}
			while (*lpb++);
		}
		iFile--;
	}
	::GlobalUnlock((HDROP) hDrop);
}

EXTERN_C void __stdcall MyDragFinish(HDROP hDrop)
{
	//::DragFinish(hDrop);
	::GlobalFree((HGLOBAL) hDrop);
}
#endif

/////////////////////////////////////////////////////////////////////////////

EXTERN_C ATOM WINAPI MyAddAtomW(LPCWSTR lpString)
{
	CMyStringW str(lpString);
	return ::AddAtomA((LPCSTR) str.AllocUTF8String());
}

EXTERN_C ATOM WINAPI MyFindAtomW(LPCWSTR lpString)
{
	CMyStringW str(lpString);
	return ::FindAtomA((LPCSTR) str.AllocUTF8String());
}

EXTERN_C ATOM WINAPI MyDeleteAtom(ATOM nAtom)
{
	return ::DeleteAtom(nAtom);
}

////////////////////////////////////////////////////////////////////////////////

//// in RichUtil.cpp
//void __stdcall RichEditGetTextW(HWND hWndRich, CMyStringW& rString);
//void __stdcall RichEditSetTextW(HWND hWndRich, LPCWSTR lpszText, int nLen = -1);
//void __stdcall RichEditReplaceSelW(HWND hWndRich, LPCWSTR lpszText, int nLen = -1);
//
//void __stdcall _GetDlgRichTextUnicodeText(HWND hDlg, int nIDDlgItem, CMyStringW& rString)
//{
//	RichEditGetTextW(::GetDlgItem(hDlg, nIDDlgItem), rString);
//}
//
//void __stdcall _SetDlgRichTextUnicodeText(HWND hDlg, int nIDDlgItem, LPCWSTR lpszString)
//{
//	RichEditSetTextW(::GetDlgItem(hDlg, nIDDlgItem), lpszString, -1);
//}

void __stdcall MyGetWindowTextStringW(HWND hWnd, CMyStringW& rString)
{
	int nLen = ::GetWindowTextLengthW(hWnd) + 1;
	if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
	{
		nLen = ::GetWindowTextLengthA(hWnd) + 1;
		::GetWindowTextA(hWnd, rString.GetBufferA((UINT) nLen), nLen);
		rString.ReleaseBufferA();
	}
	else
	{
		::GetWindowTextW(hWnd, rString.GetBuffer((UINT) nLen), nLen);
		rString.ReleaseBuffer();
	}
}

extern "C" bool __stdcall MySetWindowTextW(HWND hWnd, LPCWSTR lpszString)
{
	bool bRet = (::SetWindowTextW(hWnd, lpszString) != 0);
	if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
	{
		CMyStringW str(lpszString);
		bRet = (::SetWindowTextA(hWnd, str) != 0);
	}
	return bRet;
}

extern "C" bool __stdcall MySetDlgItemTextW(HWND hDlg, int nIDDlgItem, LPCWSTR lpszString)
{
	HWND h = ::GetDlgItem(hDlg, nIDDlgItem);
	if (!h)
		return false;
	return ::MySetWindowTextW(h, lpszString);
}

void __stdcall MyGetDlgUnicodeText(HWND hDlg, int nIDDlgItem, CMyStringW& rString)
{
	int nLen = ::GetWindowTextLengthW(::GetDlgItem(hDlg, nIDDlgItem)) + 1;
	if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
	{
		nLen = ::GetWindowTextLengthA(::GetDlgItem(hDlg, nIDDlgItem)) + 1;
		::GetDlgItemTextA(hDlg, nIDDlgItem, rString.GetBufferA((UINT) nLen), nLen);
		rString.ReleaseBufferA();
	}
	else
	{
		::GetDlgItemTextW(hDlg, nIDDlgItem, rString.GetBuffer((UINT) nLen), nLen);
		rString.ReleaseBuffer();
	}
}

void __stdcall SyncDialogData(HWND hWnd, int nID, CMyStringW& rString, bool bGet)
{
	if (bGet)
	{
		::MyGetDlgUnicodeText(hWnd, nID, rString);
	}
	else
	{
		::SetDlgItemTextW(hWnd, nID, rString);
		if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
			::SetDlgItemTextA(hWnd, nID, rString);
	}
}

extern "C" int __stdcall AddDlgListBoxStringW(HWND hWnd, int nID, LPCWSTR lpszString)
{
	register HWND h = ::GetDlgItem(hWnd, nID);
	if (IsWindowUnicode(h))
		return (int) (::SendMessageW(h, LB_ADDSTRING, 0, (LPARAM) lpszString));
	else
	{
		CMyStringW str(lpszString);
		return (int) (::SendMessageA(h, LB_ADDSTRING, 0, (LPARAM)(LPCSTR) str));
	}
}

extern "C" int __stdcall InsertDlgListBoxStringW(HWND hWnd, int nID, int nIndex, LPCWSTR lpszString)
{
	register HWND h = ::GetDlgItem(hWnd, nID);
	if (IsWindowUnicode(h))
		return (int) (::SendMessageW(h, LB_INSERTSTRING, (WPARAM) nIndex, (LPARAM) lpszString));
	else
	{
		CMyStringW str(lpszString);
		return (int) (::SendMessageA(h, LB_INSERTSTRING, (WPARAM) nIndex, (LPARAM)(LPCSTR) str));
	}
}

extern "C" int __stdcall AddDlgComboBoxStringW(HWND hWnd, int nID, LPCWSTR lpszString)
{
	register HWND h = ::GetDlgItem(hWnd, nID);
	if (IsWindowUnicode(h))
		return (int) (::SendMessageW(h, CB_ADDSTRING, 0, (LPARAM) lpszString));
	else
	{
		CMyStringW str(lpszString);
		return (int) (::SendMessageA(h, CB_ADDSTRING, 0, (LPARAM)(LPCSTR) str));
	}
}

EXTERN_C HMENU __stdcall MyLoadMenuW(_In_opt_ HINSTANCE hInstance, _In_ LPCWSTR lpMenuName)
{
	HMENU h = ::LoadMenuW(hInstance, lpMenuName);
	if (h != NULL || ::GetLastError() != ERROR_CALL_NOT_IMPLEMENTED)
		return h;
	if (IS_INTRESOURCE(lpMenuName))
		return ::LoadMenuA(hInstance, reinterpret_cast<LPCSTR>(lpMenuName));
	CMyStringW str(lpMenuName);
	return ::LoadMenuA(hInstance, str);
}

EXTERN_C BOOL __stdcall MyInsertMenuItemW(_In_ HMENU hmenu, _In_ UINT item, _In_ BOOL fByPosition, _In_ LPCMENUITEMINFOW lpmi)
{
	BOOL bRet;
	if ((bRet = ::InsertMenuItemW(hmenu, item, fByPosition, lpmi)) || ::GetLastError() != ERROR_CALL_NOT_IMPLEMENTED)
		return bRet;
	MENUITEMINFOA miiA;
	if (sizeof(miiA) < lpmi->cbSize)
		return FALSE;
	memcpy(&miiA, lpmi, lpmi->cbSize);
	char* buffer = NULL;
	if ((miiA.fMask & MIIM_STRING) || ((miiA.fMask & MIIM_TYPE) && miiA.fType == MFT_STRING))
	{
		buffer = MyUnicodeToAnsiString(lpmi->dwTypeData);
		if (!buffer)
		{
			::SetLastError(ERROR_OUTOFMEMORY);
			return FALSE;
		}
		miiA.dwTypeData = buffer;
	}
	if (!::InsertMenuItemA(hmenu, item, fByPosition, &miiA))
	{
		if (buffer)
		{
			auto last = ::GetLastError();
			free(buffer);
			::SetLastError(last);
		}
		return FALSE;
	}
	free(buffer);
	return TRUE;
}

EXTERN_C BOOL __stdcall MyGetMenuItemInfoW(_In_ HMENU hmenu, _In_ UINT item, _In_ BOOL fByPosition, _Inout_ LPMENUITEMINFOW lpmii)
{
	BOOL bRet;
	if ((bRet = ::GetMenuItemInfoW(hmenu, item, fByPosition, lpmii)) || ::GetLastError() != ERROR_CALL_NOT_IMPLEMENTED)
		return bRet;
	MENUITEMINFOA miiA;
	if (sizeof(miiA) < lpmii->cbSize)
		return FALSE;
	char* buffer = NULL;
	if (lpmii->fMask & (MIIM_TYPE | MIIM_STRING))
	{
		if (lpmii->dwTypeData)
		{
			miiA.cch = lpmii->cch * 2;
			buffer = static_cast<char*>(malloc(sizeof(char) * miiA.cch));
			if (!buffer)
			{
				::SetLastError(ERROR_OUTOFMEMORY);
				return FALSE;
			}
			miiA.dwTypeData = buffer;
		}
		else
		{
			miiA.cbSize = MENUITEMINFO_SIZE_V1A;
			miiA.fMask = MIIM_TYPE;
			miiA.cch = 0;
			miiA.dwTypeData = NULL;
			if (!::GetMenuItemInfoA(hmenu, item, fByPosition, &miiA))
				return FALSE;
			if (miiA.fType == MFT_STRING)
			{
				buffer = static_cast<char*>(malloc(sizeof(char) * (++miiA.cch)));
				if (!buffer)
				{
					::SetLastError(ERROR_OUTOFMEMORY);
					return FALSE;
				}
				miiA.dwTypeData = buffer;
			}
		}
	}
	else
	{
		miiA.cch = 0;
		miiA.dwTypeData = NULL;
	}
	miiA.cbSize = lpmii->cbSize;
	miiA.fMask = lpmii->fMask | MIIM_TYPE;
	if (!::GetMenuItemInfoA(hmenu, item, fByPosition, &miiA))
	{
		if (buffer)
		{
			auto last = ::GetLastError();
			free(buffer);
			::SetLastError(last);
		}
		return FALSE;
	}
	auto* p = lpmii->dwTypeData;
	memcpy(lpmii, &miiA, lpmii->cbSize);
	if (lpmii->fMask & (MIIM_TYPE | MIIM_STRING))
	{
		if (miiA.fType == MFT_STRING)
		{
			lpmii->dwTypeData = p;
			if (buffer)
			{
				if (p)
					lpmii->cch = static_cast<UINT>(::MultiByteToWideChar(CP_ACP, 0, buffer, static_cast<int>(miiA.cch), p, static_cast<int>(lpmii->cch)));
				else
					lpmii->cch = static_cast<UINT>(::MultiByteToWideChar(CP_ACP, 0, buffer, static_cast<int>(miiA.cch), NULL, 0));
			}
		}
	}
	if (buffer)
		free(buffer);
	return TRUE;
}

EXTERN_C BOOL __stdcall MySetMenuItemInfoW(_In_ HMENU hmenu, _In_ UINT item, _In_ BOOL fByPosition, _In_ LPCMENUITEMINFOW lpmii)
{
	BOOL bRet;
	if ((bRet = ::SetMenuItemInfoW(hmenu, item, fByPosition, lpmii)) || ::GetLastError() != ERROR_CALL_NOT_IMPLEMENTED)
		return bRet;
	MENUITEMINFOA miiA;
	if (sizeof(miiA) < lpmii->cbSize)
		return FALSE;
	memcpy(&miiA, lpmii, lpmii->cbSize);
	char* buffer = NULL;
	if ((miiA.fMask & MIIM_STRING) || ((miiA.fMask & MIIM_TYPE) && miiA.fType == MFT_STRING))
	{
		buffer = MyUnicodeToAnsiString(lpmii->dwTypeData);
		if (!buffer)
		{
			::SetLastError(ERROR_OUTOFMEMORY);
			return FALSE;
		}
		miiA.dwTypeData = buffer;
	}
	if (!::SetMenuItemInfoA(hmenu, item, fByPosition, &miiA))
	{
		if (buffer)
		{
			auto last = ::GetLastError();
			free(buffer);
			::SetLastError(last);
		}
		return FALSE;
	}
	free(buffer);
	return TRUE;
}

void __stdcall MyGetErrorMessageString(_In_ DWORD dwError, _Inout_ CMyStringW& rString)
{
	CMyStringW strBuf;
	LPVOID lpMsgBuf = NULL;
	if (!::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPWSTR>(&lpMsgBuf),
		0,
		NULL) && GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
	{
		::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dwError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPSTR>(&lpMsgBuf),
			0,
			NULL);
		if (lpMsgBuf)
		{
			rString = reinterpret_cast<LPCSTR>(lpMsgBuf);
			::LocalFree(lpMsgBuf);
		}
	}
	else
	{
		rString = reinterpret_cast<LPCWSTR>(lpMsgBuf);
		::LocalFree(lpMsgBuf);
	}
	auto len = rString.GetLength();
	if (len > 0)
	{
		if (rString[len - 1] == L'\n')
			--len;
	}
	if (len > 0)
	{
		if (rString[len - 1] == L'\r')
			--len;
	}
	rString.ReleaseBuffer(len);
}
