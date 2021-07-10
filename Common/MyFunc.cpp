/*
 Copyright (C) 2010 Kuri-Applications

 MyFunc.cpp - implementations of useful functions
 */

#include "stdafx.h"
#include "MyFunc.h"

///////////////////////////////////////////////////////////////////////////////

// Windows API とほぼ同様の処理方法による文字列コピー
extern "C" int __stdcall MyCopyStringLenA(LPSTR lpszBuffer, LPCSTR lpszString, int nMaxLen)
{
	int nLen;
	nLen = (int) strlen(lpszString);
	if (!lpszBuffer || !nMaxLen)
		return nLen;
	if (nLen < nMaxLen)
	{
		strcpy(lpszBuffer, lpszString);
		return nLen;
	}
	memset(lpszBuffer, 0, sizeof(CHAR) * nMaxLen);
	strncpy(lpszBuffer, lpszString, (size_t) (nMaxLen - 1));
	return (int) strlen(lpszBuffer);
}

extern "C" int __stdcall MyCopyStringLenW(LPWSTR lpszBuffer, LPCWSTR lpszString, int nMaxLen)
{
	int nLen;
	nLen = (int) wcslen(lpszString);
	if (!lpszBuffer || !nMaxLen)
		return nLen;
	if (nLen < nMaxLen)
	{
		wcscpy(lpszBuffer, lpszString);
		return nLen;
	}
	memset(lpszBuffer, 0, sizeof(WCHAR) * nMaxLen);
	wcsncpy(lpszBuffer, lpszString, (size_t) (nMaxLen - 1));
	return (int) wcslen(lpszBuffer);
}

#ifdef __CCL_H__
void __stdcall SyncDialogData(HWND hWnd, int nID, CCCLString& rString, bool bGet)
{
	if (bGet)
	{
		int nLen = ::GetDlgItemTextLen(hWnd, nID) + 1;
#ifndef _UNICODE
		::GetDlgItemText(hWnd, nID, rString.GetBuffer((UINT) nLen), nLen);
		rString.ReleaseBuffer();
#else
		::GetDlgItemText(hWnd, nID, rString.GetBufferW((UINT) nLen), nLen);
		rString.ReleaseBufferW();
#endif
		/*LPTSTR lp = ::GetDlgItemTextBuffer(hWnd, nID);
		if (lp)
		{
			rString = lp;
			free(lp);
		}
		else
			rString.Clear();*/
	}
	else
	{
		::SetDlgItemText(hWnd, nID, rString);
	}
}
#endif // __CCL_H__

void __stdcall SyncDialogData(HWND hWnd, int nID, int& nInt, bool bGet)
{
	if (bGet)
		nInt = (int) ::GetDlgItemInt(hWnd, nID, NULL, TRUE);
	else
		::SetDlgItemInt(hWnd, nID, (UINT) nInt, TRUE);
}

void __stdcall SyncDialogData(HWND hWnd, int nID, DWORD& dwDWord, bool bGet)
{
	if (bGet)
		dwDWord = (DWORD) ::GetDlgItemInt(hWnd, nID, NULL, FALSE);
	else
		::SetDlgItemInt(hWnd, nID, (UINT) dwDWord, FALSE);
}

void __stdcall SyncDialogData(HWND hWnd, int nID, DWORD& dwDWord, bool bGet, int nMinCount, BYTE cType)
{
	TCHAR szBuffer[30];
	if (bGet)
	{
		::GetDlgItemText(hWnd, nID, szBuffer, 30);
		szBuffer[19] = 0;
		GetDWordFromString(szBuffer, dwDWord);
	}
	else
	{
		GetStringFromDWord((long) dwDWord, szBuffer, nMinCount, cType);
		::SetDlgItemText(hWnd, nID, szBuffer);
	}
}

void __stdcall SyncDialogData(HWND hWnd, int nID, WORD& wWord, bool bGet)
{
	UINT u;
	if (bGet)
	{
		u = ::GetDlgItemInt(hWnd, nID, NULL, FALSE);
		wWord = LOWORD(u);
	}
	else
		::SetDlgItemInt(hWnd, nID, (UINT) wWord, FALSE);
}

void __stdcall SyncDialogData(HWND hWnd, int nID, BYTE& bByte, bool bGet)
{
	UINT u;
	if (bGet)
	{
		u = ::GetDlgItemInt(hWnd, nID, NULL, FALSE);
		bByte = LOBYTE(LOWORD(u));
	}
	else
		::SetDlgItemInt(hWnd, nID, (UINT) bByte, FALSE);
}

void __stdcall SyncDialogData(HWND hWnd, int nID, DWORD& dw, DWORD dwBit, bool bGet)
{
	UINT uState;
	if (bGet)
	{
		uState = ::IsDlgButtonChecked(hWnd, nID);
		if (uState == BST_CHECKED)
			dw |= dwBit;
		else
			dw &= ~dwBit;
	}
	else
	{
		uState = ((dw & dwBit) == dwBit) ? BST_CHECKED : BST_UNCHECKED;
		::CheckDlgButton(hWnd, nID, uState);
	}
}

void __stdcall SyncDialogData(HWND hWnd, int nID, BYTE& by, BYTE byBit, bool bGet)
{
	UINT uState;
	if (bGet)
	{
		uState = ::IsDlgButtonChecked(hWnd, nID);
		if (uState == BST_CHECKED)
			by |= byBit;
		else
			by &= ~byBit;
	}
	else
	{
		uState = ((by & byBit) == byBit) ? BST_CHECKED : BST_UNCHECKED;
		::CheckDlgButton(hWnd, nID, uState);
	}
}

void __stdcall SyncDialogData(HWND hWnd, int nID, bool& b, bool bGet)
{
	UINT uState;
	if (bGet)
	{
		uState = ::IsDlgButtonChecked(hWnd, nID);
		b = (uState == BST_CHECKED);
	}
	else
	{
		uState = b ? BST_CHECKED : BST_UNCHECKED;
		::CheckDlgButton(hWnd, nID, uState);
	}
}

/////////////////////////////////////////////////////////////////////////////

HANDLE __stdcall MyCreateFile(LPCTSTR lpszFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
	DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes)
{
	//LPTSTR lp = NULL;
	//if (_tcschr(lpszFileName, _T('\\')) == NULL)
	//{
	//	lp = MyGetFullPath(theApp.m_lpszPath, lpszFileName);
	//	lpszFileName = lp;
	//}
	HANDLE h = CreateFile(lpszFileName, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, dwFlagsAndAttributes, NULL);
	//if (lp)
	//	free(lp);
	return h;
}

HANDLE __stdcall MyOpenFile(LPCTSTR lpszFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
	DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes)
{
	return MyCreateFile(lpszFileName, dwDesiredAccess, dwShareMode, dwCreationDisposition, dwFlagsAndAttributes);
}

extern "C" bool __stdcall MyIsBackSlashA(CHAR ch)
	{ return ch == '\\' || ch == '/'; }
extern "C" bool __stdcall MyIsBackSlashW(WCHAR ch)
	{ return ch == L'\\' || ch == L'/'; }
extern "C" bool __stdcall MyIsWinBackSlashA(CHAR ch)
	{ return ch == '\\'; }
extern "C" bool __stdcall MyIsWinBackSlashW(WCHAR ch)
	{ return ch == L'\\'; }

extern "C" LPSTR __stdcall MyFindBackSlashA(LPCSTR lpszString)
{
	LPSTR lp = (LPSTR) lpszString;
	while (*lp)
	{
		if (MyIsBackSlashA(*lp))
			return lp;
		else if (_ismbblead(*lp))
			lp++;
		lp++;
	}
	return NULL;
}

extern "C" LPWSTR __stdcall MyFindBackSlashW(LPCWSTR lpszString)
{
	LPWSTR lp = (LPWSTR) lpszString;
	while (*lp)
	{
		if (MyIsBackSlashW(*lp))
			return lp;
		lp++;
	}
	return NULL;
}

extern "C" LPSTR __stdcall MyFindBackSlashReverseA(LPCSTR lpszString)
{
	LPSTR lp = (LPSTR) lpszString + strlen(lpszString) - 1;
	while ((LPCSTR) lp >= lpszString)
	{
		if (MyIsBackSlashA(*lp))
			return lp;
		lp--;
		if ((LPCSTR) lp > lpszString && _ismbblead(*(lp - 1)))
			lp--;
	}
	return NULL;
}

extern "C" LPWSTR __stdcall MyFindBackSlashReverseW(LPCWSTR lpszString)
{
	LPWSTR lp = (LPWSTR) lpszString + wcslen(lpszString) - 1;
	while ((LPCWSTR) lp >= lpszString)
	{
		if (MyIsBackSlashW(*lp))
			return lp;
		lp--;
	}
	return NULL;
}

extern "C" LPSTR __stdcall MyFindReturnA(LPCSTR lpszString)
{
	LPSTR lp = (LPSTR) lpszString;
	while (*lp)
	{
		if (*lp == '\r' || *lp == '\n')
			return lp;
		else if (_ismbblead(*lp))
			lp++;
		lp++;
	}
	return NULL;
}

extern "C" LPWSTR __stdcall MyFindReturnW(LPCWSTR lpszString)
{
	LPWSTR lp = (LPWSTR) lpszString;
	while (*lp)
	{
		if (*lp == L'\r' || *lp == L'\n')
			return lp;
		lp++;
	}
	return NULL;
}

extern "C" void __stdcall MyRemoveDotsFromPathA(LPCSTR pszPath, LPSTR pszOutput)
{
	CHAR* psz, * p, * p2, * po, * poStart;
	bool bFirst;
	//if (!((MyIsWinBackSlashA(pszPath[0]) && MyIsWinBackSlashA(pszPath[1])) || (pszPath[1] == ':' && MyIsBackSlashA(pszPath[2]))))
	//{
	//	strcpy(pszOutput, pszPath);
	//	return;
	//}
	psz = _strdup(pszPath);
	if (MyIsWinBackSlashA(psz[0]) && MyIsWinBackSlashA(psz[1]))
	{
		pszOutput[0] = psz[0];
		pszOutput[1] = psz[1];
		pszOutput[2] = 0;
		p2 = &psz[2];
		poStart = &pszOutput[2];
	}
	else if (MyIsBackSlashA(psz[0]))
	{
		pszOutput[0] = psz[0];
		pszOutput[1] = 0;
		p2 = &psz[1];
		poStart = &pszOutput[1];
	}
	else if (psz[1] == ':' && MyIsBackSlashA(psz[2]))
	{
		pszOutput[0] = psz[0];
		pszOutput[1] = psz[1];
		pszOutput[2] = psz[2];
		pszOutput[3] = 0;
		p2 = &psz[3];
		poStart = &pszOutput[3];
	}
	else
	{
		p2 = psz;
		poStart = pszOutput;
	}
	bFirst = true;
	po = poStart;
	if (/*MyIsBackSlashA(p2[0]) ||*/ (p2[0] == '.' && p2[1] == '.' && (p2[2] == 0 || MyIsBackSlashA(p2[2]))))
	{
		goto OnFail;
	}
	p = p2;
	//if (!MyIsBackSlashA(p[0]))
	//{
	//	while (*p && !MyIsBackSlashA(*p))
	//	{
	//		if (_ismbblead(*p))
	//			p++;
	//		p++;
	//	}
	//	strncpy(po, p2, (p - p2));
	//	po += (p - p2);
	//	*po = 0;
	//	if (!*p)
	//		goto OnFinish;
	//	*po++ = *p;
	//	p2 = p + 1;
	//}
	while (*p2)
	{
		p = p2;
		if (!bFirst && MyIsBackSlashA(p[0]))
		{
			po--;
			goto OnNext;
		}
		else if (p[0] == '.')
		{
			if (p[1] == 0 || MyIsBackSlashA(p[1]))
			{
				*po-- = 0;
				p++;
				goto OnNext;
			}
			else if (p[1] == '.' && (p[2] == 0 || MyIsBackSlashA(p[2])))
			{
				if (po <= poStart)
					goto OnFail;
				do
				{
					if (po >= poStart + 2 && _ismbblead(*(po - 2)))
						po--;
					po--;
					if (!_ismbblead(*(po - 2)) &&
						(po <= poStart || MyIsBackSlashA(*(po - 1))))
						break;
				} while (true);
				*po-- = 0;
				p += 2;
				goto OnNext;
			}
		}
		while (*p && !MyIsBackSlashA(*p))
		{
			if (_ismbblead(*p))
				p++;
			p++;
		}
		strncpy(po, p2, (p - p2));
		po += (p - p2);
OnNext:
		bFirst = false;
		*po = 0;
		if (!*p || (MyIsBackSlashA(*p) && !p[1] && (p == psz || p[-1] != ':')))
			break;
		*po++ = *p;
		p2 = p + 1;
	}
	*po = 0;
	goto OnFinish;
OnFail:
	strcpy(pszOutput, psz);
OnFinish:
	free(psz);
}

extern "C" void __stdcall MyRemoveDotsFromPathW(LPCWSTR pszPath, LPWSTR pszOutput)
{
	WCHAR* psz, * p, * p2, * po, * poStart;
	bool bFirst;
	//if (!((MyIsBackSlashW(pszPath[0]) && MyIsBackSlashW(pszPath[1])) || (pszPath[1] == L':' && MyIsBackSlashW(pszPath[2]))))
	//{
	//	wcscpy(pszOutput, pszPath);
	//	return;
	//}
	psz = _wcsdup(pszPath);
	if (MyIsWinBackSlashW(psz[0]) && MyIsWinBackSlashW(psz[1]))
	{
		pszOutput[0] = psz[0];
		pszOutput[1] = psz[1];
		pszOutput[2] = 0;
		p2 = &psz[2];
		poStart = &pszOutput[2];
	}
	else if (MyIsBackSlashW(psz[0]))
	{
		pszOutput[0] = psz[0];
		pszOutput[1] = 0;
		p2 = &psz[1];
		poStart = &pszOutput[1];
	}
	else if (psz[1] == L':' && MyIsBackSlashW(psz[2]))
	{
		pszOutput[0] = psz[0];
		pszOutput[1] = psz[1];
		pszOutput[2] = psz[2];
		pszOutput[3] = 0;
		p2 = &psz[3];
		poStart = &pszOutput[3];
	}
	else
	{
		p2 = psz;
		poStart = pszOutput;
	}
	po = poStart;
	bFirst = true;
	if (/*MyIsBackSlashW(p2[0]) ||*/ (p2[0] == L'.' && p2[1] == L'.' && (p2[2] == 0 || MyIsBackSlashW(p2[2]))))
	{
		goto OnFail;
	}
	p = p2;
	//if (!MyIsBackSlashW(p[0]))
	//{
	//	while (*p && !MyIsBackSlashW(*p))
	//		p++;
	//	wcsncpy(po, p2, (p - p2));
	//	po += (p - p2);
	//	*po = 0;
	//	if (!*p)
	//		goto OnFinish;
	//	*po++ = *p;
	//	p2 = p + 1;
	//}
	while (*p2)
	{
		p = p2;
		if (!bFirst && MyIsBackSlashW(p[0]))
		{
			po--;
			goto OnNext;
		}
		else if (p[0] == L'.')
		{
			if (p[1] == 0 || MyIsBackSlashW(p[1]))
			{
				*po-- = 0;
				p++;
				goto OnNext;
			}
			else if (p[1] == L'.' && (p[2] == 0 || MyIsBackSlashW(p[2])))
			{
				if (po <= poStart)
					goto OnFail;
				do
				{
					po--;
					if (po <= poStart || MyIsBackSlashW(*(po - 1)))
						break;
				} while (true);
				*po-- = 0;
				p += 2;
				goto OnNext;
			}
		}
		while (*p && !MyIsBackSlashW(*p))
			p++;
		wcsncpy(po, p2, (p - p2));
		po += (p - p2);
OnNext:
		bFirst = false;
		*po = 0;
		if (!*p || (MyIsBackSlashW(*p) && !p[1] && (p == psz || p[-1] != L':')))
			break;
		*po++ = *p;
		p2 = p + 1;
	}
	*po = 0;
	goto OnFinish;
OnFail:
	wcscpy(pszOutput, psz);
OnFinish:
	free(psz);
}

#ifdef __CCL_H__
void __stdcall MyRemoveDotsFromPathString(CCCLString& rstrPath)
{
	LPTSTR lp;
#ifdef _UNICODE
	lp = rstrPath.GetBufferW();
#else
	lp = rstrPath.GetBuffer();
#endif
	MyRemoveDotsFromPath(lp, lp);
#ifdef _UNICODE
	rstrPath.ReleaseBufferW();
#else
	rstrPath.ReleaseBuffer();
#endif
}
#endif

extern "C" int __stdcall MyGetFullPathA(LPCSTR lpszPath, LPCSTR lpszFile, LPSTR lpszBuffer, int nMaxLen)
{
	int n;
	int nLen1, nLen2, nLen3;
	bool bAddPath;
	// lpszFile の最初が "x:" や "\\" や "\" の時はそのままコピー
	if (!*lpszFile && (MyIsBackSlashA(*lpszFile) || lpszFile[1] == _T(':')))
		return MyCopyStringLenA(lpszBuffer, lpszFile, nMaxLen);
	bAddPath = false;
	nLen1 = (int) strlen(lpszPath);
	nLen2 = (int) strlen(lpszFile);
	nLen3 = nLen1 + nLen2;
	if (!MyIsBackSlashA(lpszPath[nLen1 - 1]))
	{
		bAddPath = true;
		nLen3++;
	}
	if (!lpszBuffer || !nMaxLen)
		return nLen3;
	nLen3 = MyCopyStringLenA(lpszBuffer, lpszPath, nMaxLen);
	if (nMaxLen + 1 <= nLen1)
		return nLen3;
	if (bAddPath)
	{
		lpszBuffer[nLen1++] = '\\';
		lpszBuffer[nLen1] = 0;
		if (nMaxLen + 1 == nLen1)
			return nLen1;
	}
	nMaxLen -= nLen1;
	n = MyCopyStringLenA(lpszBuffer + nLen1, lpszFile, nMaxLen);
	return n + nLen1;
}

extern "C" int __stdcall MyGetFullPathW(LPCWSTR lpszPath, LPCWSTR lpszFile, LPWSTR lpszBuffer, int nMaxLen)
{
	int n;
	int nLen1, nLen2, nLen3;
	bool bAddPath;
	// lpszFile の最初が "x:" や "\\" や "\" の時はそのままコピー
	if (MyIsBackSlashW(*lpszFile) || lpszFile[1] == L':')
		return MyCopyStringLenW(lpszBuffer, lpszFile, nMaxLen);
	bAddPath = false;
	nLen1 = (int) wcslen(lpszPath);
	nLen2 = (int) wcslen(lpszFile);
	nLen3 = nLen1 + nLen2;
	if (!MyIsBackSlashW(lpszPath[nLen1 - 1]))
	{
		bAddPath = true;
		nLen3++;
	}
	if (!lpszBuffer || !nMaxLen)
		return nLen3;
	nLen3 = MyCopyStringLenW(lpszBuffer, lpszPath, nMaxLen);
	if (nMaxLen + 1 <= nLen1)
		return nLen3;
	if (bAddPath)
	{
		lpszBuffer[nLen1++] = L'\\';
		lpszBuffer[nLen1] = 0;
		if (nMaxLen + 1 == nLen1)
			return nLen1;
	}
	nMaxLen -= nLen1;
	n = MyCopyStringLenW(lpszBuffer + nLen1, lpszFile, nMaxLen);
	return n + nLen1;
}

LPSTR __stdcall MyGetFullPath2A(LPCSTR lpszPath, LPCSTR lpszFile)
{
	int nLen;
	LPSTR lp;
	nLen = MyGetFullPathA(lpszPath, lpszFile, NULL, 0) + 1;
	lp = (LPSTR) malloc(sizeof(CHAR) * nLen);
	nLen = MyGetFullPathA(lpszPath, lpszFile, lp, nLen);
	lp[nLen] = 0;
	return lp;
}

LPWSTR __stdcall MyGetFullPath2W(LPCWSTR lpszPath, LPCWSTR lpszFile)
{
	int nLen;
	LPWSTR lp;
	nLen = MyGetFullPathW(lpszPath, lpszFile, NULL, 0) + 1;
	lp = (LPWSTR) malloc(sizeof(WCHAR) * nLen);
	nLen = MyGetFullPathW(lpszPath, lpszFile, lp, nLen);
	lp[nLen] = 0;
	return lp;
}

#ifdef __CCL_H__
void __stdcall MyGetFullPathString(LPCTSTR lpszPath, LPCTSTR lpszFile, CCCLString& rstrBuffer)
{
	int nLen;
	nLen = MyGetFullPath(lpszPath, lpszFile, NULL, 0) + 1;
	nLen = MyGetFullPath(lpszPath, lpszFile,
#ifdef _UNICODE
		rstrBuffer.GetBufferW((DWORD) nLen)
#else
		rstrBuffer.GetBuffer((DWORD) nLen)
#endif
		, nLen);
#ifdef _UNICODE
	rstrBuffer.ReleaseBufferW(TRUE, (DWORD) nLen);
#else
	rstrBuffer.ReleaseBuffer((DWORD) nLen);
#endif
}
#endif

extern "C" int __stdcall MyGetAbsolutePathA(LPCSTR lpszRelativePathName, LPCSTR lpszDirectory, LPSTR lpszBuffer, int nMaxLen)
{
	LPSTR lp;
	int n;
	// lpszDirectory が相対パスならそのままコピー
	// ※ "\" から始まるものは絶対パスとみなす
	if (!MyIsBackSlashA(*lpszDirectory) &&
		(_ismbblead(*lpszDirectory) || lpszDirectory[1] != ':'))
		return MyCopyStringLenA(lpszBuffer, lpszRelativePathName, nMaxLen);

	// lpszRelativePathName が絶対パスならそのままコピー
	// ※ "\" から始まるものは絶対パスとみなす
	if (MyIsBackSlashA(*lpszRelativePathName) ||
		(!_ismbblead(*lpszRelativePathName) && lpszRelativePathName[1] == ':'))
		return MyCopyStringLenA(lpszBuffer, lpszRelativePathName, nMaxLen);

	lp = MyGetFullPath2A(lpszDirectory, lpszRelativePathName);
	MyRemoveDotsFromPathA(lp, lp);
	n = MyCopyStringLenA(lpszBuffer, lp, nMaxLen);
	free(lp);
	return n;
}

extern "C" int __stdcall MyGetAbsolutePathW(LPCWSTR lpszRelativePathName, LPCWSTR lpszDirectory, LPWSTR lpszBuffer, int nMaxLen)
{
	LPWSTR lp;
	int n;
	// lpszDirectory が相対パスならそのままコピー
	// ※ "\" から始まるものは絶対パスとみなす
	if (!MyIsBackSlashW(*lpszDirectory) &&
		lpszDirectory[1] != L':')
		return MyCopyStringLenW(lpszBuffer, lpszRelativePathName, nMaxLen);

	// lpszRelativePathName が絶対パスならそのままコピー
	// ※ "\" から始まるものは絶対パスとみなす
	if (MyIsBackSlashW(*lpszRelativePathName) ||
		lpszRelativePathName[1] == L':')
		return MyCopyStringLenW(lpszBuffer, lpszRelativePathName, nMaxLen);

	lp = MyGetFullPath2W(lpszDirectory, lpszRelativePathName);
	MyRemoveDotsFromPathW(lp, lp);
	n = MyCopyStringLenW(lpszBuffer, lp, nMaxLen);
	free(lp);
	return n;
}

extern "C" LPSTR __stdcall MyGetAbsolutePath2A(LPCSTR lpszRelativePathName, LPCSTR lpszDirectory)
{
	int nLen;
	LPSTR lp;
	nLen = MyGetAbsolutePathA(lpszRelativePathName, lpszDirectory, NULL, 0) + 1;
	lp = (LPSTR) malloc(sizeof(CHAR) * nLen);
	nLen = MyGetAbsolutePathA(lpszRelativePathName, lpszDirectory, lp, nLen);
	lp[nLen] = 0;
	return lp;
}

extern "C" LPWSTR __stdcall MyGetAbsolutePath2W(LPCWSTR lpszRelativePathName, LPCWSTR lpszDirectory)
{
	int nLen;
	LPWSTR lp;
	nLen = MyGetAbsolutePathW(lpszRelativePathName, lpszDirectory, NULL, 0) + 1;
	lp = (LPWSTR) malloc(sizeof(WCHAR) * nLen);
	nLen = MyGetAbsolutePathW(lpszRelativePathName, lpszDirectory, lp, nLen);
	lp[nLen] = 0;
	return lp;
}

#ifdef __CCL_H__
void __stdcall MyGetAbsolutePathString(LPCTSTR lpszRelativePathName, LPCTSTR lpszDirectory, CCCLString& rstrBuffer)
{
	LPTSTR lp;
	// lpszDirectory が相対パスならそのままコピー
	// ※ "\" から始まるものは絶対パスとみなす
	if (!MyIsBackSlash(*lpszDirectory) &&
		lpszDirectory[1] != _T(':'))
	{
		rstrBuffer = lpszRelativePathName;
		return;
	}

	// lpszRelativePathName が絶対パスならそのままコピー
	// ※ "\" から始まるものは絶対パスとみなす
	if (MyIsBackSlash(*lpszRelativePathName) ||
		(!_istlead(*lpszRelativePathName) && lpszRelativePathName[1] == _T(':')))
	{
		rstrBuffer = lpszRelativePathName;
		return;
	}

	MyGetFullPathString(lpszDirectory, lpszRelativePathName, rstrBuffer);
#ifdef _UNICODE
	lp = rstrBuffer.GetBufferW();
#else
	lp = rstrBuffer.GetBuffer();
#endif
	MyRemoveDotsFromPath(lp, lp);
#ifdef _UNICODE
	rstrBuffer.ReleaseBufferW();
#else
	rstrBuffer.ReleaseBuffer();
#endif
}
#endif

extern "C" int __stdcall MyGetRelativePathA(LPCSTR lpszFullPathName, LPCSTR lpszDirectory, LPSTR lpszBuffer, int nMaxLen)
{
	LPCSTR lp1, lp2;
	bool bNoDrive;
	int nToParentCount, /*nLen, */n, nNowPos;

	// lpszFullPathName の始めは "x:\" または "\\" または "\" (ドライブ指定なし)
	if (!_ismbblead(*lpszFullPathName) && lpszFullPathName[1] != ':' && !MyIsBackSlashA(lpszFullPathName[2]))
	{
		if (!MyIsBackSlashA(*lpszFullPathName))
			return 0;
		bNoDrive = !MyIsWinBackSlashA(*lpszFullPathName) && !MyIsWinBackSlashA(lpszFullPathName[1]);
	}
	else
		bNoDrive = false;
	// ２文字めまで(bNoDrive == true の場合は1文字目が)合わない
	// → 無関係なパスか相対パスかのどちらか
	if (*lpszFullPathName != *lpszDirectory || (!bNoDrive && lpszFullPathName[1] != lpszDirectory[1]) ||
		(lpszFullPathName[1] == ':' && lpszFullPathName[2] != lpszDirectory[2]))
		return MyCopyStringLenA(lpszBuffer, lpszFullPathName, nMaxLen);
	if (bNoDrive)
	{
		lpszFullPathName++;
		lpszDirectory++;
	}
	else if (!_ismbblead(*lpszFullPathName) && lpszFullPathName[1] == ':')
	{
		lpszFullPathName += 3;
		lpszDirectory += 3;
	}
	else
	{
		lpszFullPathName += 2;
		lpszDirectory += 2;
	}
	lp1 = MyFindBackSlashA(lpszFullPathName);
	lp2 = MyFindBackSlashA(lpszDirectory);
	if (!lp2 && *lpszDirectory)
	{
		lp2 = lpszDirectory;
		while (*lp2++)
			lp2++;
	}
	while (lp1 && lp2)
	{
		// 長さが違う → 違うパス
		if ((lp1 - lpszFullPathName) != (lp2 - lpszDirectory))
			break;
		// 同じパスでなかったら中断
		if (_strnicmp(lpszFullPathName, lpszDirectory, (size_t)(lp1 - lpszFullPathName)) != 0)
			break;
		lpszFullPathName = lp1 + 1;
		if (!*lp2)
			lpszDirectory = lp2;
		else
			lpszDirectory = lp2 + 1;
		lp1 = MyFindBackSlashA(lpszFullPathName);
		lp2 = MyFindBackSlashA(lpszDirectory);
		if (!lp2 && *lpszDirectory)
		{
			lp2 = lpszDirectory;
			while (*lp2)
				lp2++;
		}
	}
	// ともに NULL → 同じパス
	if (!lp1 && !lp2)
		return MyCopyStringLenA(lpszBuffer, lpszFullPathName, nMaxLen);

	// "..\" をつける数を数える
	nToParentCount = 0;
	lp2 = MyFindBackSlashA(lpszDirectory);
	if (!lp2 && *lpszDirectory)
	{
		lp2 = lpszDirectory;
		while (*lp2++)
			lp2++;
	}
	while (lp2)
	{
		nToParentCount++;
		if (!*lp2)
			lpszDirectory = lp2;
		else
			lpszDirectory = lp2 + 1;
		lp2 = MyFindBackSlashA(lpszDirectory);
		if (!lp2 && *lpszDirectory)
		{
			lp2 = lpszDirectory;
			while (*lp2++)
				lp2++;
		}
	}

	// 文字列の長さ = (strlen("..\") * nToParentCount) + strlen(lpszFullPathName)
	// ※ lpszFullPathName は既に適当な位置にポインタを進めてある
	//nLen = 3 * nToParentCount + strlen(lpszFullPathName);
	if (!lpszBuffer || !nMaxLen)
		return 3 * nToParentCount + (int) strlen(lpszFullPathName);

	nNowPos = 0;
	while (nToParentCount--)
	{
		n = MyCopyStringLenA(lpszBuffer, "..\\", nMaxLen);
		if (nMaxLen <= 3)
			return nNowPos + n;
		nNowPos += 3;
		nMaxLen -= 3;
		lpszBuffer += 3;
	}
	n = MyCopyStringLenA(lpszBuffer, lpszFullPathName, nMaxLen);
	return nNowPos + n;
}

extern "C" int __stdcall MyGetRelativePathW(LPCWSTR lpszFullPathName, LPCWSTR lpszDirectory, LPWSTR lpszBuffer, int nMaxLen)
{
	LPCWSTR lp1, lp2;
	bool bNoDrive;
	int nToParentCount, /*nLen, */n, nNowPos;

	// lpszFullPathName の始めは "x:\" または "\\" または "\" (ドライブ指定なし)
	if (lpszFullPathName[1] != L':' && !MyIsBackSlashW(lpszFullPathName[2]))
	{
		if (!MyIsBackSlashW(*lpszFullPathName))
			return 0;
		bNoDrive = !MyIsWinBackSlashW(*lpszFullPathName) && !MyIsWinBackSlashW(lpszFullPathName[1]);
	}
	else
		bNoDrive = false;
	// ２文字めまで(bNoDrive == true の場合は1文字目が)合わない
	// → 無関係なパスか相対パスかのどちらか
	if (*lpszFullPathName != *lpszDirectory || (!bNoDrive && lpszFullPathName[1] != lpszDirectory[1]) ||
		(lpszFullPathName[1] == L':' && lpszFullPathName[2] != lpszDirectory[2]))
		return MyCopyStringLenW(lpszBuffer, lpszFullPathName, nMaxLen);
	if (bNoDrive)
	{
		lpszFullPathName++;
		lpszDirectory++;
	}
	else if (lpszFullPathName[1] == L':')
	{
		lpszFullPathName += 3;
		lpszDirectory += 3;
	}
	else
	{
		lpszFullPathName += 2;
		lpszDirectory += 2;
	}
	lp1 = MyFindBackSlashW(lpszFullPathName);
	lp2 = MyFindBackSlashW(lpszDirectory);
	if (!lp2 && *lpszDirectory)
	{
		lp2 = lpszDirectory;
		while (*lp2++)
			lp2++;
	}
	while (lp1 && lp2)
	{
		// 長さが違う → 違うパス
		if ((lp1 - lpszFullPathName) != (lp2 - lpszDirectory))
			break;
		// 同じパスでなかったら中断
		if (_wcsnicmp(lpszFullPathName, lpszDirectory, (size_t)(lp1 - lpszFullPathName)) != 0)
			break;
		lpszFullPathName = lp1 + 1;
		if (!*lp2)
			lpszDirectory = lp2;
		else
			lpszDirectory = lp2 + 1;
		lp1 = MyFindBackSlashW(lpszFullPathName);
		lp2 = MyFindBackSlashW(lpszDirectory);
		if (!lp2 && *lpszDirectory)
		{
			lp2 = lpszDirectory;
			while (*lp2)
				lp2++;
		}
	}
	// ともに NULL → 同じパス
	if (!lp1 && !lp2)
		return MyCopyStringLenW(lpszBuffer, lpszFullPathName, nMaxLen);

	// L"..\" をつける数を数える
	nToParentCount = 0;
	lp2 = MyFindBackSlashW(lpszDirectory);
	if (!lp2 && *lpszDirectory)
	{
		lp2 = lpszDirectory;
		while (*lp2++)
			lp2++;
	}
	while (lp2)
	{
		nToParentCount++;
		if (!*lp2)
			lpszDirectory = lp2;
		else
			lpszDirectory = lp2 + 1;
		lp2 = MyFindBackSlashW(lpszDirectory);
		if (!lp2 && *lpszDirectory)
		{
			lp2 = lpszDirectory;
			while (*lp2++)
				lp2++;
		}
	}

	// 文字列の長さ = (wcslen(L"..\") * nToParentCount) + wcslen(lpszFullPathName)
	// ※ lpszFullPathName は既に適当な位置にポインタを進めてある
	//nLen = 3 * nToParentCount + wcslen(lpszFullPathName);
	if (!lpszBuffer || !nMaxLen)
		return 3 * nToParentCount + (int) wcslen(lpszFullPathName);

	nNowPos = 0;
	while (nToParentCount--)
	{
		n = MyCopyStringLenW(lpszBuffer, L"..\\", nMaxLen);
		if (nMaxLen <= 3)
			return nNowPos + n;
		nNowPos += 3;
		nMaxLen -= 3;
		lpszBuffer += 3;
	}
	n = MyCopyStringLenW(lpszBuffer, lpszFullPathName, nMaxLen);
	return nNowPos + n;
}

extern "C" LPSTR __stdcall MyGetRelativePath2A(LPCSTR lpszFullPathName, LPCSTR lpszDirectory)
{
	int nLen;
	LPSTR lp;
	nLen = MyGetRelativePathA(lpszFullPathName, lpszDirectory, NULL, 0) + 1;
	lp = (LPSTR) malloc(sizeof(CHAR) * nLen);
	nLen = MyGetRelativePathA(lpszFullPathName, lpszDirectory, lp, nLen);
	lp[nLen] = 0;
	return lp;
}

extern "C" LPWSTR __stdcall MyGetRelativePath2W(LPCWSTR lpszFullPathName, LPCWSTR lpszDirectory)
{
	int nLen;
	LPWSTR lp;
	nLen = MyGetRelativePathW(lpszFullPathName, lpszDirectory, NULL, 0) + 1;
	lp = (LPWSTR) malloc(sizeof(WCHAR) * nLen);
	nLen = MyGetRelativePathW(lpszFullPathName, lpszDirectory, lp, nLen);
	lp[nLen] = 0;
	return lp;
}

#ifdef __CCL_H__
void __stdcall MyGetRelativePathString(LPCTSTR lpszFullPathName, LPCTSTR lpszDirectory, CCCLString& rstrBuffer)
{
	int nLen;
	nLen = MyGetRelativePath(lpszFullPathName, lpszDirectory, NULL, 0) + 1;
	nLen = MyGetRelativePath(lpszFullPathName, lpszDirectory,
#ifdef _UNICODE
		rstrBuffer.GetBufferW((DWORD) nLen)
#else
		rstrBuffer.GetBuffer((DWORD) nLen)
#endif
		, nLen);
#ifdef _UNICODE
	rstrBuffer.ReleaseBufferW(TRUE, (DWORD) nLen);
#else
	rstrBuffer.ReleaseBuffer((DWORD) nLen);
#endif
}
#endif

extern "C" int __stdcall MyMakeFullPathFromCurDirA(LPCSTR lpszPathName, LPSTR lpszBuffer, int nMaxLen)
{
	int n;
	LPSTR lp;
	// "\<path-name>" の形式の場合
#ifdef _WINDOWS_
	if (MyIsBackSlashA(lpszPathName[0]) && (!MyIsWinBackSlashA(lpszPathName[0]) || !MyIsWinBackSlashA(lpszPathName[1])))
	{
		if (!lpszBuffer || !nMaxLen)
			return (int) strlen(lpszPathName) + 2;
		if (!::GetCurrentDirectoryA((DWORD) nMaxLen, lpszBuffer))
			return 0;
		if (nMaxLen < 3)
			return nMaxLen;
		if (!_ismbblead(*lpszBuffer) && lpszBuffer[1] == ':')
			n = 2;
		else if (MyIsBackSlashA(*lpszBuffer) && MyIsBackSlashA(lpszBuffer[1]))
		{
			lpszBuffer[nMaxLen - 1] = 0;
			lp = MyFindBackSlashA(lpszBuffer + 2);
			// 相対パスをつなげる部分まで届いていない
			if (!lp)
				return nMaxLen;
			n = (int) (lp - lpszBuffer);
		}
		else
		{
			// ???
			return MyCopyStringLenA(lpszBuffer, lpszPathName, nMaxLen);
		}
		nMaxLen -= n;
		return MyCopyStringLenA(lpszBuffer + n, lpszPathName, nMaxLen);
	}
#endif
	// 絶対パスならそのまま
	if (lpszPathName[1] == ':' || MyIsBackSlashA(lpszPathName[0]))
	{
		if (!lpszBuffer || !nMaxLen)
			return (int) strlen(lpszPathName);
		return MyCopyStringLenA(lpszBuffer, lpszPathName, nMaxLen);
	}

	// MyGetAbsolutePathA を使って作成
	n = ::GetCurrentDirectoryA(0, lpszBuffer);
	if (!n)
		return 0;
	lp = (LPSTR) malloc(sizeof(CHAR) * (++n));
	::GetCurrentDirectoryA(n, lp);
	n = MyGetAbsolutePathA(lpszPathName, lp, lpszBuffer, nMaxLen);
	free(lp);
	return n;
}

extern "C" int __stdcall MyMakeFullPathFromCurDirW(LPCWSTR lpszPathName, LPWSTR lpszBuffer, int nMaxLen)
{
	int n;
	LPWSTR lp;
	// "\<path-name>" の形式の場合
#ifdef _WINDOWS_
	if (MyIsBackSlashW(lpszPathName[0]) && (!MyIsWinBackSlashW(lpszPathName[0]) || !MyIsWinBackSlashW(lpszPathName[1])))
	{
		if (!lpszBuffer || !nMaxLen)
			return (int) wcslen(lpszPathName) + 2;
		if (!::GetCurrentDirectoryW((DWORD) nMaxLen, lpszBuffer))
		{
			if (::GetLastError() != ERROR_CALL_NOT_IMPLEMENTED)
				return 0;
			n = (int) ::GetCurrentDirectoryA(0, (LPSTR) lpszBuffer) + 1;
			// lp を LPSTR として利用
			lp = (LPWSTR) malloc(sizeof(CHAR) * n);
			::GetCurrentDirectoryA(n, (LPSTR) lp);
			::MultiByteToWideChar(CP_ACP, 0, (LPCSTR) lp, -1, lpszBuffer, nMaxLen);
			free(lp);
		}
		if (nMaxLen < 3)
			return nMaxLen;
		if (lpszBuffer[1] == L':')
			n = 2;
		else if (MyIsBackSlashW(*lpszBuffer) && MyIsBackSlashW(lpszBuffer[1]))
		{
			lpszBuffer[nMaxLen - 1] = 0;
			lp = MyFindBackSlashW(lpszBuffer + 2);
			// 相対パスをつなげる部分まで届いていない
			if (!lp)
				return nMaxLen;
			n = (int) (lp - lpszBuffer);
		}
		else
		{
			// ???
			return MyCopyStringLenW(lpszBuffer, lpszPathName, nMaxLen);
		}
		nMaxLen -= n;
		return MyCopyStringLenW(lpszBuffer + n, lpszPathName, nMaxLen);
	}
#endif
	// 絶対パスならそのまま
	if (lpszPathName[1] == ':' || MyIsBackSlashW(lpszPathName[0]))
	{
		if (!lpszBuffer || !nMaxLen)
			return (int) wcslen(lpszPathName);
		return MyCopyStringLenW(lpszBuffer, lpszPathName, nMaxLen);
	}

	// MyGetAbsolutePathW を使って作成
	n = ::GetCurrentDirectoryW(0, lpszBuffer);
	if (!n)
	{
		if (::GetLastError() != ERROR_CALL_NOT_IMPLEMENTED)
			return 0;
		LPSTR lp2;
		n = (int) ::GetCurrentDirectoryA(0, (LPSTR) lpszBuffer) + 1;
		lp2 = (LPSTR) malloc(sizeof(CHAR) * n);
		::GetCurrentDirectoryA(n, lp2);
		n = ::MultiByteToWideChar(CP_ACP, 0, lp2, -1, NULL, 0);
		lp = (LPWSTR) malloc(sizeof(WCHAR) * n);
		::MultiByteToWideChar(CP_ACP, 0, lp2, -1, lp, n);
		free(lp);
	}
	else
	{
		lp = (LPWSTR) malloc(sizeof(WCHAR) * (++n));
		::GetCurrentDirectoryW(n, lp);
	}
	n = MyGetAbsolutePathW(lpszPathName, lp, lpszBuffer, nMaxLen);
	free(lp);
	return n;
}

extern "C" LPSTR __stdcall MyMakeFullPathFromCurDir2A(LPCSTR lpszPathName)
{
	int nLen;
	LPSTR lp;
	nLen = MyMakeFullPathFromCurDirA(lpszPathName, NULL, 0) + 1;
	lp = (LPSTR) malloc(sizeof(char) * nLen);
	nLen = MyMakeFullPathFromCurDirA(lpszPathName, lp, nLen);
	lp[nLen] = 0;
	return lp;
}

extern "C" LPWSTR __stdcall MyMakeFullPathFromCurDir2W(LPCWSTR lpszPathName)
{
	int nLen;
	LPWSTR lp;
	nLen = MyMakeFullPathFromCurDirW(lpszPathName, NULL, 0) + 1;
	lp = (LPWSTR) malloc(sizeof(WCHAR) * nLen);
	nLen = MyMakeFullPathFromCurDirW(lpszPathName, lp, nLen);
	lp[nLen] = 0;
	return lp;
}

#ifdef __CCL_H__
void __stdcall MyMakeFullPathFromCurDirString(LPCTSTR lpszPathName, CCCLString& rstrBuffer)
{
	int nLen;
	nLen = MyMakeFullPathFromCurDir(lpszPathName, NULL, 0) + 1;
	nLen = MyMakeFullPathFromCurDir(lpszPathName,
#ifdef _UNICODE
		rstrBuffer.GetBufferW((DWORD) nLen)
#else
		rstrBuffer.GetBuffer((DWORD) nLen)
#endif
		, nLen);
#ifdef _UNICODE
	rstrBuffer.ReleaseBufferW(TRUE, (DWORD) nLen);
#else
	rstrBuffer.ReleaseBuffer((DWORD) nLen);
#endif
}
#endif

extern "C" int __stdcall MySearchPath(LPCTSTR lpszPathName, LPTSTR lpszBuffer, int nMaxLen)
{
	LPTSTR lp;
	return (int) ::SearchPath(NULL, lpszPathName, NULL, (DWORD) nMaxLen, lpszBuffer, &lp);
}

#ifdef __CCL_H__
void __stdcall MySearchPathString(LPCTSTR lpszPathName, CCCLString& rstrBuffer)
{
	int nLen;
	nLen = MySearchPath(lpszPathName, NULL, 0) + 1;
	nLen = MySearchPath(lpszPathName,
#ifdef _UNICODE
		rstrBuffer.GetBufferW((DWORD) nLen)
#else
		rstrBuffer.GetBuffer((DWORD) nLen)
#endif
		, nLen);
#ifdef _UNICODE
	rstrBuffer.ReleaseBufferW(TRUE, (DWORD) nLen);
#else
	rstrBuffer.ReleaseBuffer((DWORD) nLen);
#endif
}
#endif

// MyIsBackSlashA などは使わない
extern "C" DWORD __stdcall MyGetLongPathNameA(LPCSTR lpszPath, LPSTR lpszBuffer, DWORD dwMaxLen)
{
	LPSTR lpPath = _strdup(lpszPath);
	LPSTR lp, lp2;
	LPSTR lpTempBuffer;
	LPSTR lpCopy;
	WIN32_FIND_DATAA wfd;
	HANDLE hFind;
	DWORD dwLen;
	DWORD dwCopied;
	if (lpszBuffer && dwMaxLen)
		lpTempBuffer = (LPSTR) malloc(sizeof(CHAR) * (dwMaxLen + 14));
	else
	{
		lpszBuffer = NULL;
		lpTempBuffer = (LPSTR) malloc(sizeof(CHAR) * (dwMaxLen = MAX_PATH));
	}
	lpCopy = lpTempBuffer;
	if (*lpPath == '\\' && lpPath[1] == '\\')
	{
		goto OnNotCopied;
	}
	else
	{
		lp = strchr(lpPath, '\\');
		if (!lp)
			goto OnNotCopied;
		lp2 = strchr(lp + 1, '\\');
		if (!lp2)
		{
			lp2 = lp;
			while (*++lp2);
			//goto OnNotCopied;
		}
		dwCopied = 0;
		dwLen = (DWORD)((++lp) - lpPath);
		if (lpszBuffer)
		{
			if (dwLen >= dwMaxLen)
				goto OnCopyLen;
			dwMaxLen -= dwLen;
		}
		strncpy(lpCopy, lpPath, dwLen);
		lpCopy += dwLen;
		*lpCopy = 0;
		dwCopied += dwLen;
		do
		{
			dwLen = (DWORD)(lp2 - lp);
			if (lpszBuffer)
			{
				if (dwLen >= dwMaxLen + 13)
					goto OnCopyLen;
				//dwMaxLen -= dwLen;
			}
			else if (dwCopied + dwLen >= dwMaxLen)
			{
				LPSTR lpRe;
				dwMaxLen += dwLen + 1;
				lpRe = (LPSTR) realloc(lpTempBuffer, sizeof(CHAR) * dwMaxLen);
				lpCopy = (lpRe + (DWORD)(lpCopy - lpTempBuffer));
				lpTempBuffer = lpRe;
			}
			strncpy(lpCopy, lp, dwLen);
			lpCopy[dwLen] = 0;
			//lpCopy += dwLen;
			hFind = ::FindFirstFileA(lpTempBuffer, &wfd);
			if (hFind == INVALID_HANDLE_VALUE)
				goto OnFailed;
			::FindClose(hFind);
			dwLen = (DWORD) strlen(wfd.cFileName);
			if (lpszBuffer)
			{
				if (dwLen >= dwMaxLen)
				{
					lp = wfd.cFileName;
					goto OnCopyLen;
				}
				dwMaxLen -= dwLen;
			}
			else if (dwCopied + dwLen >= dwMaxLen)
			{
				LPSTR lpRe;
				dwMaxLen += dwLen + 2;
				lpRe = (LPSTR) realloc(lpTempBuffer, sizeof(CHAR) * dwMaxLen);
				lpCopy = (lpRe + (DWORD)(lpCopy - lpTempBuffer));
				lpTempBuffer = lpRe;
			}
			strncpy(lpCopy, wfd.cFileName, dwLen);
			lpCopy += dwLen;
			*lpCopy = 0;
			dwCopied += dwLen;
			if (!*lp2)
				break;
			if (lpszBuffer)
			{
				if (dwMaxLen == 0)
				{
					dwLen = dwCopied;
					goto OnClean;
				}
				dwMaxLen--;
			}
			*lpCopy++ = '\\';
			*lpCopy = 0;
			dwCopied++;
			lp = lp2 + 1;
			lp2 = strchr(lp, '\\');
			if (!lp2)
			{
				if (!*lp)
					break;
				lp2 = lp;
				while (*++lp2);
			}
		} while (true);
	}
	dwLen = dwCopied;
	goto OnClean;

OnCopyLen:
	strncpy(lpCopy, lp, dwMaxLen);
	lpCopy[dwMaxLen - 1] = 0;
	dwCopied += dwMaxLen - 1;
	dwLen = dwCopied;
	goto OnClean;

OnFailed:
	if (lpszBuffer && dwMaxLen)
		*lpTempBuffer = 0;
	dwLen = 0;
	goto OnClean;

OnNotCopied:
	dwLen = (DWORD) strlen(lpPath);
	if (lpszBuffer && dwMaxLen)
	{
		strncpy(lpTempBuffer, lpPath, dwMaxLen);
		dwLen = min(dwLen, dwMaxLen);
	}
OnClean:
	if (lpszBuffer)
	{
		memcpy(lpszBuffer, lpTempBuffer, sizeof(CHAR) * (dwLen + 1));
	}
	free(lpTempBuffer);
	free(lpPath);
	return dwLen;
}

// MyIsBackSlashW などは使わない
extern "C" DWORD __stdcall MyGetLongPathNameW(LPCWSTR lpszPath, LPWSTR lpszBuffer, DWORD dwMaxLen)
{
	LPWSTR lpPath = _wcsdup(lpszPath);
	LPWSTR lp, lp2;
	LPWSTR lpTempBuffer;
	LPWSTR lpCopy;
	WIN32_FIND_DATAW wfd;
	HANDLE hFind;
	DWORD dwLen;
	DWORD dwCopied;
	if (lpszBuffer && dwMaxLen)
		lpTempBuffer = (LPWSTR) malloc(sizeof(WCHAR) * (dwMaxLen + 14));
	else
	{
		lpszBuffer = NULL;
		lpTempBuffer = (LPWSTR) malloc(sizeof(WCHAR) * (dwMaxLen = MAX_PATH));
	}
	lpCopy = lpTempBuffer;
	if (*lpPath == L'\\' && lpPath[1] == L'\\')
	{
		goto OnNotCopied;
	}
	else
	{
		lp = wcschr(lpPath, L'\\');
		if (!lp)
			goto OnNotCopied;
		lp2 = wcschr(lp + 1, L'\\');
		if (!lp2)
		{
			lp2 = lp;
			while (*++lp2);
			//goto OnNotCopied;
		}
		dwCopied = 0;
		dwLen = (DWORD)((++lp) - lpPath);
		if (lpszBuffer)
		{
			if (dwLen >= dwMaxLen)
				goto OnCopyLen;
			dwMaxLen -= dwLen;
		}
		wcsncpy(lpCopy, lpPath, dwLen);
		lpCopy += dwLen;
		*lpCopy = 0;
		dwCopied += dwLen;
		do
		{
			dwLen = (DWORD)(lp2 - lp);
			if (lpszBuffer)
			{
				if (dwLen >= dwMaxLen + 13)
					goto OnCopyLen;
				//dwMaxLen -= dwLen;
			}
			else if (dwCopied + dwLen >= dwMaxLen)
			{
				LPWSTR lpRe;
				dwMaxLen += dwLen + 1;
				lpRe = (LPWSTR) realloc(lpTempBuffer, sizeof(WCHAR) * dwMaxLen);
				lpCopy = (lpRe + (DWORD)(lpCopy - lpTempBuffer));
				lpTempBuffer = lpRe;
			}
			wcsncpy(lpCopy, lp, dwLen);
			lpCopy[dwLen] = 0;
			//lpCopy += dwLen;
			hFind = ::FindFirstFileW(lpTempBuffer, &wfd);
			if (hFind == INVALID_HANDLE_VALUE)
				goto OnFailed;
			::FindClose(hFind);
			dwLen = (DWORD) wcslen(wfd.cFileName);
			if (lpszBuffer)
			{
				if (dwLen >= dwMaxLen)
				{
					lp = wfd.cFileName;
					goto OnCopyLen;
				}
				dwMaxLen -= dwLen;
			}
			else if (dwCopied + dwLen >= dwMaxLen)
			{
				LPWSTR lpRe;
				dwMaxLen += dwLen + 2;
				lpRe = (LPWSTR) realloc(lpTempBuffer, sizeof(WCHAR) * dwMaxLen);
				lpCopy = (lpRe + (DWORD)(lpCopy - lpTempBuffer));
				lpTempBuffer = lpRe;
			}
			wcsncpy(lpCopy, wfd.cFileName, dwLen);
			lpCopy += dwLen;
			*lpCopy = 0;
			dwCopied += dwLen;
			if (!*lp2)
				break;
			if (lpszBuffer)
			{
				if (dwMaxLen == 0)
				{
					dwLen = dwCopied;
					goto OnClean;
				}
				dwMaxLen--;
			}
			*lpCopy++ = L'\\';
			*lpCopy = 0;
			dwCopied++;
			lp = lp2 + 1;
			lp2 = wcschr(lp, L'\\');
			if (!lp2)
			{
				if (!*lp)
					break;
				lp2 = lp;
				while (*++lp2);
			}
		} while (true);
	}
	dwLen = dwCopied;
	goto OnClean;

OnCopyLen:
	wcsncpy(lpCopy, lp, dwMaxLen);
	lpCopy[dwMaxLen - 1] = 0;
	dwCopied += dwMaxLen - 1;
	dwLen = dwCopied;
	goto OnClean;

OnFailed:
	if (lpszBuffer && dwMaxLen)
		*lpTempBuffer = 0;
	dwLen = 0;
	goto OnClean;

OnNotCopied:
	dwLen = (DWORD) wcslen(lpPath);
	if (lpszBuffer && dwMaxLen)
	{
		wcsncpy(lpTempBuffer, lpPath, dwMaxLen);
		dwLen = min(dwLen, dwMaxLen);
	}
OnClean:
	if (lpszBuffer)
	{
		memcpy(lpszBuffer, lpTempBuffer, sizeof(WCHAR) * (dwLen + 1));
	}
	free(lpTempBuffer);
	free(lpPath);
	return dwLen;
}

#ifdef __CCL_H__
void __stdcall MyGetLongPathNameString(LPCTSTR lpszPath, CCCLString& rstrBuffer)
{
	DWORD nLen;
	nLen = MyGetLongPathName(lpszPath, NULL, 0) + 1;
	nLen = MyGetLongPathName(lpszPath,
#ifdef _UNICODE
		rstrBuffer.GetBufferW(nLen)
#else
		rstrBuffer.GetBuffer(nLen)
#endif
		, nLen);
#ifdef _UNICODE
	rstrBuffer.ReleaseBufferW(TRUE, nLen);
#else
	rstrBuffer.ReleaseBuffer(nLen);
#endif
}
#endif

extern "C" bool __stdcall CompareFilePathA(LPCSTR lpszPath1, LPCSTR lpszPath2)
{
	LPSTR lp1, lp2;
	DWORD dw1, dw2;
	bool bRet;
	dw1 = MyGetLongPathNameA(lpszPath1, NULL, 0) + 1;
	if (dw1 > 1)
	{
		lp1 = (LPSTR) malloc(sizeof(CHAR) * dw1);
		MyGetLongPathNameA(lpszPath1, lp1, dw1);
	}
	else
		lp1 = _strdup(lpszPath1);
	dw2 = MyGetLongPathNameA(lpszPath2, NULL, 0) + 1;
	if (dw2 > 1)
	{
		lp2 = (LPSTR) malloc(sizeof(CHAR) * dw2);
		MyGetLongPathNameA(lpszPath2, lp2, dw2);
	}
	else
		lp2 = _strdup(lpszPath2);
	bRet = (_stricmp(lp1, lp2) == 0);
	free(lp1);
	free(lp2);
	return bRet;
}

extern "C" bool __stdcall CompareFilePathW(LPCWSTR lpszPath1, LPCWSTR lpszPath2)
{
	LPWSTR lp1, lp2;
	DWORD dw1, dw2;
	bool bRet;
	dw1 = MyGetLongPathNameW(lpszPath1, NULL, 0) + 1;
	if (dw1 > 1)
	{
		lp1 = (LPWSTR) malloc(sizeof(WCHAR) * dw1);
		MyGetLongPathNameW(lpszPath1, lp1, dw1);
	}
	else
		lp1 = _wcsdup(lpszPath1);
	dw2 = MyGetLongPathNameW(lpszPath2, NULL, 0) + 1;
	if (dw2 > 1)
	{
		lp2 = (LPWSTR) malloc(sizeof(WCHAR) * dw2);
		MyGetLongPathNameW(lpszPath2, lp2, dw2);
	}
	else
		lp2 = _wcsdup(lpszPath2);
	bRet = (_wcsicmp(lp1, lp2) == 0);
	free(lp1);
	free(lp2);
	return bRet;
}

extern "C" bool __stdcall CompareFilePathLenA(LPCSTR lpszPath1, LPCSTR lpszPath2, int nLen)
{
	LPSTR lp1, lp2;
	DWORD dw1, dw2;
	bool bRet;
	dw1 = MyGetLongPathNameA(lpszPath1, NULL, 0) + 1;
	if (dw1 > 1)
	{
		lp1 = (LPSTR) malloc(sizeof(CHAR) * dw1);
		MyGetLongPathNameA(lpszPath1, lp1, dw1);
	}
	else
		lp1 = _strdup(lpszPath1);
	dw2 = MyGetLongPathNameA(lpszPath2, NULL, 0) + 1;
	if (dw2 > 1)
	{
		lp2 = (LPSTR) malloc(sizeof(CHAR) * dw2);
		MyGetLongPathNameA(lpszPath2, lp2, dw2);
	}
	else
		lp2 = _strdup(lpszPath2);
	bRet = (_strnicmp(lp1, lp2, (size_t) nLen) == 0);
	free(lp1);
	free(lp2);
	return bRet;
}

extern "C" bool __stdcall CompareFilePathLenW(LPCWSTR lpszPath1, LPCWSTR lpszPath2, int nLen)
{
	LPWSTR lp1, lp2;
	DWORD dw1, dw2;
	bool bRet;
	dw1 = MyGetLongPathNameW(lpszPath1, NULL, 0) + 1;
	if (dw1 > 1)
	{
		lp1 = (LPWSTR) malloc(sizeof(WCHAR) * dw1);
		MyGetLongPathNameW(lpszPath1, lp1, dw1);
	}
	else
		lp1 = _wcsdup(lpszPath1);
	dw2 = MyGetLongPathNameW(lpszPath2, NULL, 0) + 1;
	if (dw2 > 1)
	{
		lp2 = (LPWSTR) malloc(sizeof(WCHAR) * dw2);
		MyGetLongPathNameW(lpszPath2, lp2, dw2);
	}
	else
		lp2 = _wcsdup(lpszPath2);
	bRet = (_wcsnicmp(lp1, lp2, (size_t) nLen) == 0);
	free(lp1);
	free(lp2);
	return bRet;
}

#ifdef __CCL_H__
int __stdcall MyMessageBox(HWND hWnd, LPCTSTR lpszText, LPCTSTR lpszCaption, UINT uType)
{
	CCCLString strText, strCaption;
	if (HIWORD(lpszText) == 0 && lpszText != NULL)
	{
		strText.LoadString((UINT) LOWORD(lpszText));
		lpszText = strText;
	}
	if (lpszCaption == NULL)
		lpszCaption = (LPCTSTR) IDS_APP_TITLE;
	if (HIWORD(lpszCaption) == 0 && lpszCaption != NULL)
	{
		strCaption.LoadString((UINT) LOWORD(lpszCaption));
		lpszCaption = strCaption;
	}
	if (!hWnd && CCLGetApp() /*&& CCLGetApp()->m_pMainWnd*/)
		hWnd = CCLGetApp()->m_pMainWnd->GetSafeHwnd();
	::CCLHookCreate(NULL);
	int nRet = ::MessageBox(hWnd, lpszText, lpszCaption, uType);
	::CCLUnhookCreate();
	return nRet;
}
#endif // __CCL_H__

extern "C" int __stdcall MyGetFileTitleA(LPCSTR lpszFile, LPSTR lpszBuffer, int nMaxLen)
{
	LPSTR lp = _strdup(lpszFile);
	LPSTR lp2 = MyFindBackSlashReverseA(lpszFile);
	int n;
	if (!lp2)
	{
		n = (int) strlen(lp);
		if (lpszBuffer && nMaxLen)
		{
			if (nMaxLen >= n)
				nMaxLen = n;
			strncpy(lpszBuffer, lp, nMaxLen);
			n = nMaxLen;
		}
		free(lp);
		return n;
	}
	if (MyIsBackSlashA(*lp2))
		lp2++;
	n = (int) strlen(lp2);
	if (lpszBuffer)
	{
		if (nMaxLen > n)
			nMaxLen = n + 1;
		strncpy(lpszBuffer, lp2, nMaxLen);
		lpszBuffer[nMaxLen - 1] = 0;
	}
	free(lp);
	return n;
}

extern "C" int __stdcall MyGetFileTitleW(LPCWSTR lpszFile, LPWSTR lpszBuffer, int nMaxLen)
{
	LPWSTR lp = _wcsdup(lpszFile);
	LPWSTR lp2 = MyFindBackSlashReverseW(lpszFile);
	int n;
	if (!lp2)
	{
		n = (int) wcslen(lp);
		if (lpszBuffer && nMaxLen)
		{
			if (nMaxLen >= n)
				nMaxLen = n;
			wcsncpy(lpszBuffer, lp, nMaxLen);
			n = nMaxLen;
		}
		free(lp);
		return n;
	}
	if (MyIsBackSlashW(*lp2))
		lp2++;
	n = (int) wcslen(lp2);
	if (lpszBuffer)
	{
		if (nMaxLen > n)
			nMaxLen = n + 1;
		wcsncpy(lpszBuffer, lp2, nMaxLen);
		lpszBuffer[nMaxLen - 1] = 0;
	}
	free(lp);
	return n;
}

#ifdef __CCL_H__
int __stdcall MyGetFileTitleString(LPCTSTR lpszFile, CCCLString& rstrFileTitle)
{
	int n = MyGetFileTitle(lpszFile, NULL, 0) + 1;
#ifdef _UNICODE
	n = MyGetFileTitle(lpszFile, rstrFileTitle.GetBufferW((DWORD) n), n);
#else
	n = MyGetFileTitle(lpszFile, rstrFileTitle.GetBuffer((DWORD) n), n);
#endif
	rstrFileTitle.ReleaseBuffer((DWORD) n);
	return n;
}
#endif

extern "C" int __stdcall MyGetModuleFileTitleA(HINSTANCE hInstance, LPSTR lpszBuffer, int nMaxLen)
{
	CHAR szBuffer[MAX_PATH];
	::GetModuleFileNameA(hInstance, szBuffer, MAX_PATH);
	return MyGetFileTitleA(szBuffer, lpszBuffer, nMaxLen);
}

extern "C" int __stdcall MyGetModuleFileTitleW(HINSTANCE hInstance, LPWSTR lpszBuffer, int nMaxLen)
{
	WCHAR szBuffer[MAX_PATH];
	if (::GetModuleFileNameW(hInstance, szBuffer, MAX_PATH) == 0 &&
		::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
	{
		char* p;
		p = (char*) malloc(sizeof(char) * MAX_PATH);
		::GetModuleFileNameA(hInstance, p, MAX_PATH);
		p[MAX_PATH - 1] = 0;
		::MultiByteToWideChar(CP_ACP, 0, p, -1, szBuffer, MAX_PATH);
		free(p);
	}
	return MyGetFileTitleW(szBuffer, lpszBuffer, nMaxLen);
}

#undef tolower
#undef toupper
#undef towlower
#undef towupper

static bool __stdcall IsEqualStringNoCaseA(LPCSTR lpsz1, LPCSTR lpsz2)
{
	CHAR ch1, ch2;
	while (*lpsz1 && *lpsz2)
	{
		ch1 = tolower(*lpsz1++);
		ch2 = tolower(*lpsz2++);
		if (ch1 != ch2)
			return false;
	}
	return true;
}

static bool __stdcall IsEqualStringNoCaseW(LPCWSTR lpsz1, LPCWSTR lpsz2)
{
	WCHAR ch1, ch2;
	while (*lpsz1 && *lpsz2)
	{
		ch1 = towlower(*lpsz1++);
		ch2 = towlower(*lpsz2++);
		if (ch1 != ch2)
			return false;
	}
	return true;
}

LPSTR __stdcall MyFindStringNoCaseA(LPSTR lpszTarget, LPSTR lpszFind)
{
	LPSTR lp = lpszTarget;
	while (*lp)
	{
		if (IsEqualStringNoCaseA(lp, lpszFind))
			return lp;
		lp++;
	}
	return NULL;
}

LPWSTR __stdcall MyFindStringNoCaseW(LPWSTR lpszTarget, LPWSTR lpszFind)
{
	LPWSTR lp = lpszTarget;
	while (*lp)
	{
		if (IsEqualStringNoCaseW(lp, lpszFind))
			return lp;
		lp++;
	}
	return NULL;
}

LPCSTR __stdcall MyFindStringNoCaseA(LPCSTR lpszTarget, LPCSTR lpszFind)
{
	LPCSTR lp = lpszTarget;
	while (*lp)
	{
		if (IsEqualStringNoCaseA(lp, lpszFind))
			return lp;
		lp++;
	}
	return NULL;
}

LPCWSTR __stdcall MyFindStringNoCaseW(LPCWSTR lpszTarget, LPCWSTR lpszFind)
{
	LPCWSTR lp = lpszTarget;
	while (*lp)
	{
		if (IsEqualStringNoCaseW(lp, lpszFind))
			return lp;
		lp++;
	}
	return NULL;
}

#ifdef __CCL_H__
void __stdcall GetWindowTextString(HWND hWnd, CCCLString& rstrBuffer)
{
	int n = GetWindowTextLength(hWnd);
	if (n == 0)
	{
		rstrBuffer.Empty();
		return;
	}
	n++;
	GetWindowText(hWnd, rstrBuffer.GetBuffer(n), n);
	rstrBuffer.ReleaseBuffer(n - 1);
}
#endif

extern "C" BOOL __stdcall EnableDlgItem(HWND hDlg, int nIDDlgItem, BOOL bEnable)
{
	HWND h = ::GetDlgItem(hDlg, nIDDlgItem);
	return ::EnableWindow(h, bEnable);
}

extern "C" int __stdcall GetDlgItemTextLen(HWND hDlg, int nIDDlgItem)
{
	return PtrToInt((void*) SendDlgItemMessage(hDlg, nIDDlgItem, WM_GETTEXTLENGTH, 0, 0));
}

//LPTSTR __stdcall GetDlgItemTextBuffer(HWND hDlg, int nIDDlgItem)
//{
//	int n = GetDlgItemTextLen(hDlg, nIDDlgItem);
//	if (n == 0)
//		return NULL;
//	n++;
//	LPTSTR lp = (LPTSTR) malloc(sizeof(TCHAR) * n);
//	GetDlgItemText(hDlg, nIDDlgItem, lp, n);
//	lp[n - 1] = 0;
//	return lp;
//}

#ifdef __CCL_H__
void __stdcall GetDlgItemTextString(HWND hDlg, int nIDDlgItem, CCCLString& rstrBuffer)
{
	int n = GetDlgItemTextLen(hDlg, nIDDlgItem);
	if (n == 0)
	{
		rstrBuffer.Empty();
		return;
	}
	n++;
	GetDlgItemText(hDlg, nIDDlgItem, rstrBuffer.GetBuffer(n), n);
	rstrBuffer.ReleaseBuffer(n - 1);
}
#endif // __CCL_H__

extern "C" HWND __stdcall SetDlgItemFocus(HWND hDlg, int nIDDlgItem)
{
	HWND h = ::GetDlgItem(hDlg, nIDDlgItem);
	return ::SetFocus(h);
}

extern "C" BOOL __stdcall IsExistFile(LPCTSTR lpszFileName)
{
#ifdef CCL_WIN16
	OFSTRUCT os;
	os.cBytes = sizeof(os);
	return (::OpenFile(lpszFileName, &os, OF_EXIST) != HFILE_ERROR);
#else
	WIN32_FIND_DATA wfd;
	HANDLE h = ::FindFirstFile(lpszFileName, &wfd);
	if (h != INVALID_HANDLE_VALUE)
		::FindClose(h);
	return (h != INVALID_HANDLE_VALUE);
#endif
}

extern "C" HFONT __stdcall MyCreateBoldFont(HFONT hFontNormal)
{
	LOGFONT lf;
	::GetObject(hFontNormal, sizeof(lf), &lf);
	lf.lfWeight = FW_BOLD;
	return ::CreateFontIndirect(&lf);
}

/////////////////////////////////////////////////////////////////////////////

extern "C" void __stdcall GetTimeDateString(LPTSTR lpszBuffer, DWORD dwMaxLen)
{
	LPTSTR lp;
	SYSTEMTIME st;
	DWORD n;
	::GetLocalTime(&st);
	::GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS | TIME_NOTIMEMARKER | TIME_FORCE24HOURFORMAT, &st, NULL, lpszBuffer, dwMaxLen);
	n = (DWORD) _tcslen(lpszBuffer);
	lp = lpszBuffer + n++;
	*lp++ = _T(' ');
	::GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, lp, dwMaxLen - n);
}

int __stdcall FormatByteStringExA(LPSTR lpszBuffer, int nBufferLen, LPCVOID lpData, DWORD dwSize)
{
	CHAR szBuffer[4];
	int nLen = 0;
	if (lpszBuffer)
		*lpszBuffer = 0;
	for (DWORD n = 0; n < dwSize; n++)
	{
		sprintf_s(szBuffer, "%02X", (UINT) ((LPBYTE) lpData)[n]);
		if (n > 0)
		{
			if (lpszBuffer)
			{
				if (nLen >= nBufferLen - 1)
					break;
				*lpszBuffer++ = ' ';
			}
			nLen++;
		}
		if (lpszBuffer)
		{
			if (nLen >= nBufferLen - 2)
			{
				if (nLen == nBufferLen - 2)
				{
					*lpszBuffer++ = szBuffer[0];
					nLen++;
				}
				break;
			}
			*lpszBuffer++ = szBuffer[0];
			*lpszBuffer++ = szBuffer[1];
		}
		nLen += 2;
	}
	if (lpszBuffer)
		*lpszBuffer = 0;
	return nLen;
}

int __stdcall FormatByteStringExW(LPWSTR lpszBuffer, int nBufferLen, LPCVOID lpData, DWORD dwSize)
{
	WCHAR szBuffer[4];
	int nLen = 0;
	if (lpszBuffer)
		*lpszBuffer = 0;
	for (DWORD n = 0; n < dwSize; n++)
	{
		swprintf_s(szBuffer, L"%02X", (UINT) ((LPBYTE) lpData)[n]);
		if (n > 0)
		{
			if (lpszBuffer)
			{
				if (nLen >= nBufferLen - 1)
					break;
				*lpszBuffer++ = L' ';
			}
			nLen++;
		}
		if (lpszBuffer)
		{
			if (nLen >= nBufferLen - 2)
			{
				if (nLen == nBufferLen - 2)
				{
					*lpszBuffer++ = szBuffer[0];
					nLen++;
				}
				break;
			}
			*lpszBuffer++ = szBuffer[0];
			*lpszBuffer++ = szBuffer[1];
		}
		nLen += 2;
	}
	if (lpszBuffer)
		*lpszBuffer = 0;
	return nLen;
}

DWORD __stdcall UnformatByteStringExA(LPCSTR lpszBuffer, LPVOID lpBuffer, DWORD dwSize)
{
	bool bFirst;
	BYTE b;
	CHAR tch;
	int nLen;
	int n;
	DWORD dw;
	if (!*lpszBuffer)
		return 0;
	nLen = (int) strlen(lpszBuffer);
	dw = 1;
	for (n = 2; n < nLen; n += 3)
	{
		if (lpszBuffer[n] != ' ')
			return 0;
		dw++;
	}
	if (dwSize != -1 && dw != dwSize)
	{
		if (!dwSize)
			return dw;
		return 0;
	}
	bFirst = true;
	dw = 0;
	while (*lpszBuffer)
	{
		tch = *lpszBuffer;
		if (tch >= 'a' && tch <= 'z')
			tch = (tch - 'a' + 'A');
		if ((tch < '0' || tch > '9') && (tch < 'A' || tch > 'F'))
			return 0;
		if (tch >= '0' && tch <= '9')
			b = (BYTE) (tch - '0');
		else
			b = (BYTE) ((tch - 'A') + 0xA);
		if (bFirst)
		{
			if (lpBuffer)
				((LPBYTE) lpBuffer)[dw] = b * 0x10;
		}
		else
		{
			if (lpBuffer)
				((LPBYTE) lpBuffer)[dw] |= b;
			dw++;
			lpszBuffer++;
			if (*lpszBuffer == 0)
				break;
		}
		bFirst = !bFirst;
		lpszBuffer++;
	}

	// ok!
	return dw;
}

DWORD __stdcall UnformatByteStringExW(LPCWSTR lpszBuffer, LPVOID lpBuffer, DWORD dwSize)
{
	bool bFirst;
	BYTE b;
	WCHAR tch;
	int nLen;
	int n;
	DWORD dw;
	if (!*lpszBuffer)
		return 0;
	nLen = (int) wcslen(lpszBuffer);
	dw = 1;
	for (n = 2; n < nLen; n += 3)
	{
		if (lpszBuffer[n] != L' ')
			return 0;
		dw++;
	}
	if (!lpBuffer || !dwSize)
		return dw;
	if (dw != dwSize)
		return 0;
	bFirst = true;
	dw = 0;
	while (*lpszBuffer)
	{
		tch = *lpszBuffer;
		if (tch >= L'a' && tch <= L'z')
			tch = (tch - L'a' + L'A');
		if ((tch < L'0' || tch > L'9') && (tch < L'A' || tch > L'F'))
			return false;
		if (tch >= L'0' && tch <= L'9')
			b = (BYTE) (tch - L'0');
		else
			b = (BYTE) ((tch - L'A') + 0xA);
		if (bFirst)
		{
			if (lpBuffer)
				((LPBYTE) lpBuffer)[dw] = b * 0x10;
		}
		else
		{
			if (lpBuffer)
				((LPBYTE) lpBuffer)[dw] |= b;
			dw++;
			lpszBuffer++;
			if (*lpszBuffer == 0)
				break;
		}
		bFirst = !bFirst;
		lpszBuffer++;
	}

	// ok!
	return dw;
}

bool __stdcall GetDWordFromStringA(LPCSTR lpszString, DWORD& dwRet)
{
	DWORD dw = 0;
	CHAR tch;
	dwRet = 0;
	if ((lpszString[0] == '0' && (lpszString[1] == 'X' || lpszString[1] == 'x')) ||
		(lpszString[0] == '&' && (lpszString[1] == 'H' || lpszString[1] == 'h')))
	{
		lpszString += 2;
		if (*lpszString == 0)
			return false;
		while (*lpszString)
		{
			tch = *lpszString;
			if (tch >= 'a' && tch <= 'z')
				tch = (tch - 'a' + 'A');
			if ((tch < '0' || tch > '9') && (tch < 'A' || tch > 'F'))
				return false;
			dw *= 0x10;
			if (tch >= '0' && tch <= '9')
				dw += (DWORD) (tch - '0');
			else // it must be (tch >= 'A' && tch <= 'F')
				dw += (DWORD) (tch - 'A') + 0xA;
			lpszString++;
		}
	}
	else if (lpszString[0] == '0' && lpszString[1] != 0)
	{
		lpszString++;
		if (*lpszString == 0)
			return false;
		while (*lpszString)
		{
			tch = *lpszString;
			if (tch < '0' || tch > '7')
				return false;
			dw *= 010;
			dw += (DWORD) (tch - '0');
			lpszString++;
		}
	}
	else
	{
		bool bMinus = false;
		if (*lpszString == 0)
			return false;
		if (*lpszString == '-')
		{
			bMinus = true;
			lpszString++;
		}
		while (*lpszString)
		{
			tch = *lpszString;
			if (tch < '0' || tch > '9')
				return false;
			dw *= 10;
			dw += (DWORD) (tch - '0');
			lpszString++;
		}
		if (bMinus)
			dw = (DWORD) -((LONG) dw);
	}
	dwRet = dw;
	return true;
}

bool __stdcall GetDWordFromStringW(LPCWSTR lpszString, DWORD& dwRet)
{
	DWORD dw = 0;
	WCHAR tch;
	dwRet = 0;
	if ((lpszString[0] == L'0' && (lpszString[1] == L'X' || lpszString[1] == L'x')) ||
		(lpszString[0] == L'&' && (lpszString[1] == L'H' || lpszString[1] == L'h')))
	{
		lpszString += 2;
		if (*lpszString == 0)
			return false;
		while (*lpszString)
		{
			tch = *lpszString;
			if (tch >= L'a' && tch <= L'z')
				tch = (tch - L'a' + L'A');
			if ((tch < L'0' || tch > L'9') && (tch < L'A' || tch > L'F'))
				return false;
			dw *= 0x10;
			if (tch >= L'0' && tch <= L'9')
				dw += (DWORD) (tch - L'0');
			else // it must be (tch >= L'A' && tch <= L'F')
				dw += (DWORD) (tch - L'A') + 0xA;
			lpszString++;
		}
	}
	else if (lpszString[0] == L'0' && lpszString[1] != 0)
	{
		lpszString++;
		if (*lpszString == 0)
			return false;
		while (*lpszString)
		{
			tch = *lpszString;
			if (tch < L'0' || tch > L'7')
				return false;
			dw *= 010;
			dw += (DWORD) (tch - L'0');
			lpszString++;
		}
	}
	else
	{
		bool bMinus = false;
		if (*lpszString == 0)
			return false;
		if (*lpszString == L'-')
		{
			bMinus = true;
			lpszString++;
		}
		while (*lpszString)
		{
			tch = *lpszString;
			if (tch < L'0' || tch > L'9')
				return false;
			dw *= 10;
			dw += (DWORD) (tch - L'0');
			lpszString++;
		}
		if (bMinus)
			dw = (DWORD) -((LONG) dw);
	}
	dwRet = dw;
	return true;
}

static int __stdcall GetFigureCount(long nValue, BYTE cType)
{
	int nRet = 0;
	DWORD dwValue = (DWORD) nValue;
	if (nValue == 0)
		return 1;
	if ((cType & 0x0F) == CUDF_DECIMAL)
	{
		while (nValue != 0)
		{
			nRet++;
			nValue /= 10;
		}
	}
	else if ((cType & 0x0F) == CUDF_HEXADECIMAL || (cType & 0x0F) == CUDF_HEXADECIMAL_UPPER)
	{
		while (dwValue != 0)
		{
			nRet++;
			dwValue /= 16;
		}
	}
	else if ((cType & 0x0F) == CUDF_OCTADECIMAL)
	{
		while (dwValue != 0)
		{
			nRet++;
			dwValue /= 8;
		}
	}
	return nRet;
}

void __stdcall GetStringFromDWordA(long nValue, LPSTR lpszBuffer, int nMinCount, BYTE cType)
{
	char chTemp;
	DWORD dwValue = (DWORD) nValue;
	int nCount = GetFigureCount(nValue, cType);
	if ((cType & 0x0F) == CUDF_DECIMAL)
	{
		if (nValue < 0)
		{
			*lpszBuffer++ = '-';
			nValue = -nValue;
		}
		if (nValue == 0)
		{
			*lpszBuffer++ = '0';
			if (--nMinCount > 0)
			{
				while (nMinCount--)
					*lpszBuffer++ = '0';
			}
		}
		else
		{
			lpszBuffer = (lpszBuffer + max(nMinCount, nCount) - 1);
			chTemp = (char)(nValue % 10);
			*lpszBuffer-- = ('0' + chTemp);
			nValue /= 10;
			nMinCount--;
			while (nValue != 0)
			{
				chTemp = (char)(nValue % 10);
				*lpszBuffer-- = ('0' + chTemp);
				nValue /= 10;
				nMinCount--;
			}
			if (nMinCount > 0)
			{
				while (nMinCount--)
				{
					*lpszBuffer-- = '0';
					nCount++;
				}
			}
			lpszBuffer = (lpszBuffer + nCount + 1);
		}
	}
	else if ((cType & 0x0F) == CUDF_HEXADECIMAL ||
		(cType & 0x0F) == CUDF_HEXADECIMAL_UPPER)
	{
		TCHAR tchA = ((cType & 0x0F) == CUDF_HEXADECIMAL_UPPER) ? 'A' : 'a';
		if (cType & 0xF0)
		{
			*lpszBuffer++ = '0';
			*lpszBuffer++ = 'x';
		}
		if (dwValue == 0)
		{
			*lpszBuffer++ = '0';
			if (--nMinCount > 0)
			{
				while (nMinCount--)
					*lpszBuffer++ = '0';
			}
		}
		else
		{
			lpszBuffer = (lpszBuffer + max(nMinCount, nCount) - 1);
			chTemp = (char)(dwValue % 16);
			if (chTemp > 9)
				*lpszBuffer-- = (tchA + (chTemp - 0x0Au));
			else
				*lpszBuffer-- = ('0' + chTemp);
			dwValue /= 16;
			nMinCount--;
			while (dwValue != 0)
			{
				chTemp = (char)(dwValue % 16);
				if (chTemp > 9)
					*lpszBuffer-- = (tchA + (chTemp - 0x0Au));
				else
					*lpszBuffer-- = ('0' + chTemp);
				dwValue /= 16;
				nMinCount--;
			}
			if (nMinCount > 0)
			{
				while (nMinCount--)
				{
					*lpszBuffer-- = '0';
					nCount++;
				}
			}
			lpszBuffer = (lpszBuffer + nCount + 1);
		}
	}
	else if ((cType & 0x0F) == CUDF_OCTADECIMAL)
	{
		if (cType & 0xF0)
		{
			*lpszBuffer++ = '0';
		}
		if (dwValue == 0)
		{
			*lpszBuffer++ = '0';
			if (--nMinCount > 0)
			{
				while (nMinCount--)
					*lpszBuffer++ = '0';
			}
		}
		else
		{
			lpszBuffer = (lpszBuffer + max(nMinCount, nCount) - 1);
			chTemp = (char)(dwValue % 8);
			*lpszBuffer-- = ('0' + chTemp);
			dwValue /= 8;
			nMinCount--;
			while (dwValue != 0)
			{
				chTemp = (char)(dwValue % 8);
				*lpszBuffer-- = ('0' + chTemp);
				dwValue /= 8;
				nMinCount--;
			}
			if (nMinCount > 0)
			{
				while (nMinCount--)
				{
					*lpszBuffer-- = '0';
					nCount++;
				}
			}
			lpszBuffer = (lpszBuffer + nCount + 1);
		}
	}
	*lpszBuffer++ = 0;
}

void __stdcall GetStringFromDWordW(long nValue, LPWSTR lpszBuffer, int nMinCount, BYTE cType)
{
	char chTemp;
	DWORD dwValue = (DWORD) nValue;
	int nCount = GetFigureCount(nValue, cType);
	if ((cType & 0x0F) == CUDF_DECIMAL)
	{
		if (nValue < 0)
		{
			*lpszBuffer++ = L'-';
			nValue = -nValue;
		}
		if (nValue == 0)
		{
			*lpszBuffer++ = L'0';
			if (--nMinCount > 0)
			{
				while (nMinCount--)
					*lpszBuffer++ = L'0';
			}
		}
		else
		{
			lpszBuffer = (lpszBuffer + max(nMinCount, nCount) - 1);
			chTemp = (char)(nValue % 10);
			*lpszBuffer-- = (L'0' + chTemp);
			nValue /= 10;
			nMinCount--;
			while (nValue != 0)
			{
				chTemp = (char)(nValue % 10);
				*lpszBuffer-- = (L'0' + chTemp);
				nValue /= 10;
				nMinCount--;
			}
			if (nMinCount > 0)
			{
				while (nMinCount--)
				{
					*lpszBuffer-- = L'0';
					nCount++;
				}
			}
			lpszBuffer = (lpszBuffer + nCount + 1);
		}
	}
	else if ((cType & 0x0F) == CUDF_HEXADECIMAL ||
		(cType & 0x0F) == CUDF_HEXADECIMAL_UPPER)
	{
		TCHAR tchA = ((cType & 0x0F) == CUDF_HEXADECIMAL_UPPER) ? L'A' : L'a';
		if (cType & 0xF0)
		{
			*lpszBuffer++ = L'0';
			*lpszBuffer++ = L'x';
		}
		if (dwValue == 0)
		{
			*lpszBuffer++ = L'0';
			if (--nMinCount > 0)
			{
				while (nMinCount--)
					*lpszBuffer++ = L'0';
			}
		}
		else
		{
			lpszBuffer = (lpszBuffer + max(nMinCount, nCount) - 1);
			chTemp = (char)(dwValue % 16);
			if (chTemp > 9)
				*lpszBuffer-- = (tchA + (chTemp - 0x0Au));
			else
				*lpszBuffer-- = (L'0' + chTemp);
			dwValue /= 16;
			nMinCount--;
			while (dwValue != 0)
			{
				chTemp = (char)(dwValue % 16);
				if (chTemp > 9)
					*lpszBuffer-- = (tchA + (chTemp - 0x0Au));
				else
					*lpszBuffer-- = (L'0' + chTemp);
				dwValue /= 16;
				nMinCount--;
			}
			if (nMinCount > 0)
			{
				while (nMinCount--)
				{
					*lpszBuffer-- = L'0';
					nCount++;
				}
			}
			lpszBuffer = (lpszBuffer + nCount + 1);
		}
	}
	else if ((cType & 0x0F) == CUDF_OCTADECIMAL)
	{
		if (cType & 0xF0)
		{
			*lpszBuffer++ = L'0';
		}
		if (dwValue == 0)
		{
			*lpszBuffer++ = L'0';
			if (--nMinCount > 0)
			{
				while (nMinCount--)
					*lpszBuffer++ = L'0';
			}
		}
		else
		{
			lpszBuffer = (lpszBuffer + max(nMinCount, nCount) - 1);
			chTemp = (char)(dwValue % 8);
			*lpszBuffer-- = (L'0' + chTemp);
			dwValue /= 8;
			nMinCount--;
			while (dwValue != 0)
			{
				chTemp = (char)(dwValue % 8);
				*lpszBuffer-- = (L'0' + chTemp);
				dwValue /= 8;
				nMinCount--;
			}
			if (nMinCount > 0)
			{
				while (nMinCount--)
				{
					*lpszBuffer-- = L'0';
					nCount++;
				}
			}
			lpszBuffer = (lpszBuffer + nCount + 1);
		}
	}
	*lpszBuffer++ = 0;
}

/////////////////////////////////////////////////////////////////////////////

static const char* __stdcall strchrlimit(const char* string, char ch, size_t len)
{
	while (len--)
	{
		if (*string == ch)
			return string;
		string++;
	}
	return NULL;
}

static const wchar_t* __stdcall wcschrlimit(const wchar_t* string, wchar_t ch, size_t len)
{
	while (len--)
	{
		if (*string == ch)
			return string;
		string++;
	}
	return NULL;
}

static const char* __stdcall strstrlimit(const char* string, const char* find, size_t strlen, size_t findlen)
{
	while (strlen)
	{
		if (strlen < findlen)
			break;
		strlen--;
		if (memcmp(string, find, sizeof(char) * findlen) == 0)
			return string;
		string++;
	}
	return NULL;
}

static const wchar_t* __stdcall wcsstrlimit(const wchar_t* string, const wchar_t* find, size_t strlen, size_t findlen)
{
	while (strlen)
	{
		if (strlen < findlen)
			break;
		strlen--;
		if (memcmp(string, find, sizeof(wchar_t) * findlen) == 0)
			return string;
		string++;
	}
	return NULL;
}

static bool __stdcall _MatchSingleWildcardLenA(LPCSTR lpszTarget, LPCSTR lpszPattern, size_t nLen)
{
	while (nLen--)
	{
		if (*lpszPattern != '?' && *lpszTarget != *lpszPattern)
			return false;
		lpszTarget++;
		lpszPattern++;
	}
	return true;
}

extern "C" bool __stdcall MyMatchWildcardA(LPCSTR lpszTarget, LPCSTR lpszPattern)
{
	return MyMatchWildcardLenA(lpszTarget, strlen(lpszTarget), lpszPattern, strlen(lpszPattern));
}

extern "C" bool __stdcall MyMatchWildcardLenA(LPCSTR lpszTarget, size_t nTargetLen, LPCSTR lpszPattern, size_t nPatternLen)
{
	if (!nTargetLen && !nPatternLen)
		return true;
	LPCSTR lpszPatternLast = lpszPattern + nPatternLen;
	LPCSTR lp = strchrlimit(lpszPattern, '*', nPatternLen);
	LPCSTR lpNext, lpPos;
	size_t nPartLen;
	if (lp && lp > lpszPattern + 1)
	{
		if (!_MatchSingleWildcardLenA(lpszTarget, lpszPattern,
			(size_t) (((DWORD_PTR) lp - (DWORD_PTR) lpszPattern) / sizeof(char))))
			return false;
	}
	if (lp)
	{
		lp++;
		if (lp == lpszPatternLast)
			return true;
		nPatternLen--;
		lpNext = strchrlimit(lp, '*', nPatternLen);
		if (!lpNext)
			lpNext = lp + nPatternLen;
		nPartLen = (size_t) ((DWORD_PTR) lpNext - (DWORD_PTR) lp) / sizeof(char);
		nPatternLen -= nPartLen;
		while (true)
		{
			lpPos = strstrlimit(lpszTarget, lp, nTargetLen, nPartLen);
			if (!lpPos)
				break;
			//nTargetLen = (size_t) ((DWORD_PTR) lpPos - (DWORD_PTR) lpszTarget) / sizeof(char);
			nTargetLen -= (size_t) ((DWORD_PTR) lpPos - (DWORD_PTR) lpszTarget) / sizeof(char);
			if (MyMatchWildcardLenA(lpPos + nPartLen, nTargetLen - nPartLen, lpNext, nPatternLen))
				return true;
			lpPos++;
			nTargetLen--;
			lpszTarget = lpPos;
		}
		return false;
	}
	else
	{
		if (nTargetLen != nPatternLen)
			return false;
		return _MatchSingleWildcardLenA(lpszTarget, lpszPattern, nTargetLen);
	}
}

static bool __stdcall _MatchSingleWildcardLenW(LPCWSTR lpszTarget, LPCWSTR lpszPattern, size_t nLen)
{
	while (nLen--)
	{
		if (*lpszPattern != L'?' && *lpszTarget != *lpszPattern)
			return false;
		lpszTarget++;
		lpszPattern++;
	}
	return true;
}

extern "C" bool __stdcall MyMatchWildcardW(LPCWSTR lpszTarget, LPCWSTR lpszPattern)
{
	return MyMatchWildcardLenW(lpszTarget, wcslen(lpszTarget), lpszPattern, wcslen(lpszPattern));
}

extern "C" bool __stdcall MyMatchWildcardLenW(LPCWSTR lpszTarget, size_t nTargetLen, LPCWSTR lpszPattern, size_t nPatternLen)
{
	if (!nTargetLen && !nPatternLen)
		return true;
	LPCWSTR lpszPatternLast = lpszPattern + nPatternLen;
	LPCWSTR lp = wcschrlimit(lpszPattern, L'*', nPatternLen);
	LPCWSTR lpNext, lpPos;
	size_t nPartLen;
	if (lp && lp > lpszPattern + 1)
	{
		if (!_MatchSingleWildcardLenW(lpszTarget, lpszPattern,
			(size_t) (((DWORD_PTR) lp - (DWORD_PTR) lpszPattern) / sizeof(WCHAR))))
			return false;
	}
	if (lp)
	{
		lp++;
		if (lp == lpszPatternLast)
			return true;
		nPatternLen--;
		lpNext = wcschrlimit(lp, L'*', nPatternLen);
		if (!lpNext)
			lpNext = lp + nPatternLen;
		nPartLen = (size_t) ((DWORD_PTR) lpNext - (DWORD_PTR) lp) / sizeof(WCHAR);
		nPatternLen -= nPartLen;
		while (true)
		{
			lpPos = wcsstrlimit(lpszTarget, lp, nTargetLen, nPartLen);
			if (!lpPos)
				break;
			//nTargetLen = (size_t) ((DWORD_PTR) lpPos - (DWORD_PTR) lpszTarget) / sizeof(WCHAR);
			nTargetLen -= (size_t) ((DWORD_PTR) lpPos - (DWORD_PTR) lpszTarget) / sizeof(WCHAR);
			if (MyMatchWildcardLenW(lpPos + nPartLen, nTargetLen - nPartLen, lpNext, nPatternLen))
				return true;
			lpPos++;
			nTargetLen--;
			lpszTarget = lpPos;
		}
		return false;
	}
	else
	{
		if (nTargetLen != nPatternLen)
			return false;
		return _MatchSingleWildcardLenW(lpszTarget, lpszPattern, nTargetLen);
	}
}

/////////////////////////////////////////////////////////////////////////////

extern "C" int __stdcall MyCopyAnsiFromUnicode(LPCWSTR lpszString, LPSTR lpBuffer, int nMaxLen)
{
	int n;
	if (nMaxLen == -1)
		n = ::WideCharToMultiByte(CP_ACP, 0, lpszString, -1, NULL, 0, NULL, NULL);
	else
		n = nMaxLen;
	n = ::WideCharToMultiByte(CP_ACP, 0, lpszString, -1, lpBuffer, n, NULL, NULL);
	return (nMaxLen == -1 && n) ? n - 1 : n;
}

extern "C" int __stdcall MyCopyAnsiToUnicode(LPCSTR lpszString, LPWSTR lpBuffer, int nMaxLen)
{
	int n;
	if (nMaxLen == -1)
		n = ::MultiByteToWideChar(CP_ACP, 0, lpszString, -1, NULL, 0);
	else
		n = nMaxLen;
	n = ::MultiByteToWideChar(CP_ACP, 0, lpszString, -1, lpBuffer, n);
	return (nMaxLen == -1 && n) ? n - 1 : n;
}

extern "C" LPWSTR __stdcall MyCopyAnsiToUnicodeRealloc(LPCSTR lpszString, LPWSTR lpBuffer)
{
	int n;
	LPWSTR lpw;
	n = ::MultiByteToWideChar(CP_ACP, 0, lpszString, -1, NULL, 0);
	if (lpBuffer)
		lpw = (LPWSTR) realloc(lpBuffer, sizeof(WCHAR) * n);
	else
		lpw = (LPWSTR) malloc(sizeof(WCHAR) * n);
	if (!lpw)
		return NULL;
	::MultiByteToWideChar(CP_ACP, 0, lpszString, -1, lpw, n);
	return lpw;
}

extern "C" LPWSTR __stdcall MyCopyUnicodeToUnicodeRealloc(LPCWSTR lpszString, LPWSTR lpBuffer)
{
	int n;
	LPWSTR lpw;
	n = (int) wcslen(lpszString) + 1;
	if (lpBuffer)
		lpw = (LPWSTR) realloc(lpBuffer, sizeof(WCHAR) * n);
	else
		lpw = (LPWSTR) malloc(sizeof(WCHAR) * n);
	if (!lpw)
		return NULL;
	memcpy(lpw, lpszString, sizeof(WCHAR) * n);
	return lpw;
}

extern "C" LPSTR __stdcall MyUnicodeToAnsiString(LPCWSTR lpszString)
{
	LPSTR szBuffer;
	int n = ::WideCharToMultiByte(CP_ACP, 0, lpszString, -1, NULL, 0, NULL, NULL);
	szBuffer = (LPSTR) ::malloc(sizeof(CHAR) * n);
	if (!szBuffer)
		return NULL;
	::WideCharToMultiByte(CP_ACP, 0, lpszString, -1, szBuffer, n, NULL, NULL);
	return szBuffer;
}

extern "C" LPWSTR __stdcall MyAnsiStringToUnicode(LPCSTR lpszString)
{
	LPWSTR szBuffer;
	int n = ::MultiByteToWideChar(CP_ACP, 0, lpszString, -1, NULL, 0);
	szBuffer = (LPWSTR) ::malloc(sizeof(WCHAR) * n);
	if (!szBuffer)
		return NULL;
	::MultiByteToWideChar(CP_ACP, 0, lpszString, -1, szBuffer, n);
	return szBuffer;
}

extern "C" int __stdcall MyGetAnsiStringLenAsUnicode(LPCSTR lpszString)
{
	int n = MultiByteToWideChar(CP_ACP, 0, lpszString, -1, NULL, 0);
	return n ? n - 1 : 0;
}

#ifdef __CCL_H__
void __stdcall ReplaceString(CCCLString& rstrText, LPCTSTR lpszFind, LPCTSTR lpszReplace)
{
	int nCount;
	int nFindLen;
	int nReplaceLen;
	LPTSTR lp, lpBuffer;
	UINT uLength, uNewLength;
	if (rstrText.IsEmpty())
		return;
	nFindLen = (int) _tcslen(lpszFind);
	nReplaceLen = lpszReplace ? (int) _tcslen(lpszReplace) : 0;
#ifndef _UNICODE
	uLength = rstrText.GetLength();
#else
	uLength = UnicodeLen((LPCSTR) rstrText, rstrText.GetLength());
#endif
	nCount = 0;
	lp = (LPTSTR) _tcsstr(rstrText, lpszFind);
	while (lp)
	{
		lp++;
		nCount++;
		lp = _tcsstr(lp, lpszFind);
	}
	if (!nCount)
		return;
	uNewLength = uLength + (nReplaceLen - nFindLen) * nCount;
	if (uNewLength > uLength)
#ifndef _UNICODE
		lpBuffer = rstrText.GetBuffer(uNewLength);
#else
		lpBuffer = rstrText.GetBufferW(uNewLength);
#endif
	else
#ifndef _UNICODE
		lpBuffer = rstrText.GetBuffer(0);
#else
		lpBuffer = rstrText.GetBufferW(0);
#endif
	lp = _tcsstr(lpBuffer, lpszFind);
	while (lp)
	{
		nCount = uLength - ((int) (lp - lpBuffer)) - nFindLen + 1;
		memmove(lp + nReplaceLen, lp + nFindLen, sizeof(TCHAR) * nCount);
		if (lpszReplace)
			memcpy(lp, lpszReplace, sizeof(TCHAR) * nReplaceLen);
		uLength += nReplaceLen - nFindLen;
		lp = _tcsstr(lp + nReplaceLen, lpszFind);
	}
#ifndef _UNICODE
	rstrText.ReleaseBuffer(uNewLength);
#else
	rstrText.ReleaseBufferW(TRUE, uNewLength);
#endif
}
#endif // __CCL_H__

///////////////////////////////////////////////////////////////////////////////

#ifdef __CCL_H__
bool __stdcall MyGetEnvironmentVariableString(LPCTSTR lpName, CCCLString& strBuffer)
{
	DWORD dw, dw2;
	LPTSTR lp;
	dw = 0;
	do
	{
		dw += MAX_PATH;
#ifdef _UNICODE
		lp = strBuffer.GetBufferW(dw);
#else
		lp = strBuffer.GetBuffer(dw);
#endif
		dw2 = GetEnvironmentVariable(lpName, lp, dw);
		if (!dw2)
		{
			strBuffer.Empty();
			return false;
		}
	} while(dw2 >= dw - 2);
#ifdef _UNICODE
	strBuffer.ReleaseBufferW();
#else
	strBuffer.ReleaseBuffer();
#endif
	return true;
}
#endif

///////////////////////////////////////////////////////////////////////////////

extern "C" BOOL __stdcall FillSolidRect(HDC hDC, const RECT* lpRect, COLORREF crColor)
{
	HBRUSH hbr = CreateSolidBrush(crColor);
	if (!hbr)
		return FALSE;
	BOOL bRet = FillRect(hDC, lpRect, hbr);
	DeleteObject(hbr);
	return bRet;
}

///////////////////////////////////////////////////////////////////////////////

#if defined(GUID_DEFINED) && defined(__LPGUID_DEFINED__) && defined(_REFGUID_DEFINED)

bool __stdcall HexStringToNumberA(LPCSTR& lpszString, DWORD& dwData, int nLen)
{
	dwData = 0;
	while (nLen--)
	{
		dwData *= 0x10;
		if (*lpszString < '0' || *lpszString > '9')
		{
			if (*lpszString < 'A' || *lpszString > 'F')
			{
				if (*lpszString < 'a' || *lpszString > 'f')
					return false;
				dwData |= (DWORD)(*lpszString - 'a' + 0xA);
			}
			else
				dwData |= (DWORD)(*lpszString - 'A' + 0xA);
		}
		else
			dwData |= (DWORD)(*lpszString - '0');
		lpszString++;
	}
	return true;
}

bool __stdcall HexStringToNumberW(LPCWSTR& lpszString, DWORD& dwData, int nLen)
{
	dwData = 0;
	while (nLen--)
	{
		dwData *= 0x10;
		if (*lpszString < L'0' || *lpszString > L'9')
		{
			if (*lpszString < L'A' || *lpszString > L'F')
			{
				if (*lpszString < L'a' || *lpszString > L'f')
					return false;
				dwData |= (DWORD)(*lpszString - L'a' + 0xA);
			}
			else
				dwData |= (DWORD)(*lpszString - L'A' + 0xA);
		}
		else
			dwData |= (DWORD)(*lpszString - L'0');
		lpszString++;
	}
	return true;
}

extern "C" bool __stdcall MyGUIDFromStringA(LPCSTR lpszString, LPGUID lpGuid)
{
	// {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}
	if (strlen(lpszString) != 38)
		return false;
	if (lpszString[0] != '{' || lpszString[37] != '}' ||
		lpszString[9] != '-' || lpszString[14] != '-' ||
		lpszString[19] != '-' || lpszString[24] != '-')
		return false;
	lpszString++;

	DWORD dw;
	int n;
	if (!HexStringToNumberA(lpszString, lpGuid->Data1, 8))
		return false;
	lpszString++;
	if (!HexStringToNumberA(lpszString, dw, 4))
		return false;
	lpGuid->Data2 = (WORD) dw;
	lpszString++;
	if (!HexStringToNumberA(lpszString, dw, 4))
		return false;
	lpGuid->Data3 = (WORD) dw;
	lpszString++;

	for (n = 0; n < 8; n++)
	{
		if (!HexStringToNumberA(lpszString, dw, 2))
			return false;
		lpGuid->Data4[n] = (BYTE) dw;
		if (n == 1)
			lpszString++;
	}
	return true;
}

extern "C" bool __stdcall MyGUIDFromStringW(LPCWSTR lpszString, LPGUID lpGuid)
{
	// {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}
	if (wcslen(lpszString) != 38)
		return false;
	if (lpszString[0] != L'{' || lpszString[37] != L'}' ||
		lpszString[9] != L'-' || lpszString[14] != L'-' ||
		lpszString[19] != L'-' || lpszString[24] != L'-')
		return false;
	lpszString++;

	DWORD dw;
	int n;
	if (!HexStringToNumberW(lpszString, lpGuid->Data1, 8))
		return false;
	lpszString++;
	if (!HexStringToNumberW(lpszString, dw, 4))
		return false;
	lpGuid->Data2 = (WORD) dw;
	lpszString++;
	if (!HexStringToNumberW(lpszString, dw, 4))
		return false;
	lpGuid->Data3 = (WORD) dw;
	lpszString++;

	for (n = 0; n < 8; n++)
	{
		if (!HexStringToNumberW(lpszString, dw, 2))
			return false;
		lpGuid->Data4[n] = (BYTE) dw;
		if (n == 1)
			lpszString++;
	}
	return true;
}

extern "C" bool __stdcall MyIsNullGUID(REFGUID rguid)
{
	LPDWORD lpdw = (LPDWORD) &rguid;
	return lpdw[0] == 0 && lpdw[1] == 0 && lpdw[2] == 0 && lpdw[3] == 0;
}

#endif // defined(GUID_DEFINED) && defined(__LPGUID_DEFINED__) && defined(_REFGUID_DEFINED)
