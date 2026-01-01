/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 FileList.cpp - implementations of file-list-parsing functions
 */

#include "stdafx.h"
#include "ShellDLL.h"
#include "MyFunc.h"
#include "FileList.h"

#include "Folder.h"
#include "SFTPChan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

////////////////////////////////////////////////////////////////////////////////
// parsing Unix-style file list

static bool __stdcall ParseUnixAttrMode(LPCWSTR lpszString, int& nUnixMode)
{
	WCHAR wch;
	#define SET_UNIX_MODE(wch, targ, val, mode, bit) \
		wch = (*targ++); \
		if (wch == val) \
			mode |= bit; \
		else if (wch != L'-') \
			return false

	nUnixMode = 0;
	// directory, symlink, etc.
	switch (*lpszString++)
	{
		case L'b':
			nUnixMode = S_IFBLK;
			break;
		case L'c':
			nUnixMode = S_IFCHR;
			break;
		case L'd':
			nUnixMode = S_IFDIR;
			break;
		case L'l':
			nUnixMode = S_IFLNK;
			break;
		case L'-':
			break;
		default:
			return false;
	}
	// owner read
	SET_UNIX_MODE(wch, lpszString, L'r', nUnixMode, S_IRUSR);
	// owner write
	SET_UNIX_MODE(wch, lpszString, L'w', nUnixMode, S_IWUSR);
	// owner execute
	switch (*lpszString++)
	{
		case L'S':
			nUnixMode |= S_ISUID;
			break;
		case L's':
			nUnixMode |= S_ISUID;
		case L'x':
			nUnixMode |= S_IXUSR;
			break;
		case L'-':
			break;
		default:
			return false;
	}
	// group read
	SET_UNIX_MODE(wch, lpszString, L'r', nUnixMode, S_IRGRP);
	// group write
	SET_UNIX_MODE(wch, lpszString, L'w', nUnixMode, S_IWGRP);
	// group execute
	switch (*lpszString++)
	{
		case L'S':
			nUnixMode |= S_ISGID;
			break;
		case L's':
			nUnixMode |= S_ISGID;
		case L'x':
			nUnixMode |= S_IXGRP;
			break;
		case L'-':
			break;
		default:
			return false;
	}
	// other read
	SET_UNIX_MODE(wch, lpszString, L'r', nUnixMode, S_IROTH);
	// other write
	SET_UNIX_MODE(wch, lpszString, L'w', nUnixMode, S_IWOTH);
	// other execute
	switch (*lpszString++)
	{
		case L'T':
			nUnixMode |= S_ISVTX;
			break;
		case L't':
			nUnixMode |= S_ISVTX;
		case L'x':
			nUnixMode |= S_IXOTH;
			break;
		case L'-':
			break;
		default:
			return false;
	}

	return true;
	#undef SET_UNIX_MODE
}

static int __stdcall ParseEnglishMonthName(LPCWSTR lpszString)
{
	if (lpszString[3] != L' ')
		return 0;
	if (wcsncmp(lpszString, L"Jan", 3) == 0) return 1;
	if (wcsncmp(lpszString, L"Feb", 3) == 0) return 2;
	if (wcsncmp(lpszString, L"Mar", 3) == 0) return 3;
	if (wcsncmp(lpszString, L"Apr", 3) == 0) return 4;
	if (wcsncmp(lpszString, L"May", 3) == 0) return 5;
	if (wcsncmp(lpszString, L"Jun", 3) == 0) return 6;
	if (wcsncmp(lpszString, L"Jul", 3) == 0) return 7;
	if (wcsncmp(lpszString, L"Aug", 3) == 0) return 8;
	if (wcsncmp(lpszString, L"Sep", 3) == 0) return 9;
	if (wcsncmp(lpszString, L"Oct", 3) == 0) return 10;
	if (wcsncmp(lpszString, L"Nov", 3) == 0) return 11;
	if (wcsncmp(lpszString, L"Dec", 3) == 0) return 12;
	return 0;
}

static int __stdcall ParseJapaneseMonthName(LPCWSTR lpszString)
{
	if (lpszString[1] == 0x6708) // L'月'
	{
		if (lpszString[0] < L'1' || lpszString[0] > L'9')
			return 0;
		return (lpszString[0] - L'0');
	}
	else if (lpszString[2] == 0x6708) // L'月'
	{
		if (lpszString[0] != L'1' || (lpszString[1] < L'0' || lpszString[1] > L'2'))
			return 0;
		return (lpszString[1] - L'0') + 10;
	}
	return 0;
}

// the first character may be L' '
static int __stdcall ParseJapaneseDayName(LPCWSTR lpszString, LPCWSTR* lplpPos)
{
	int r;
	if (lpszString[1] == 0x65E5) // L'日'
	{
		if (lpszString[2] != L' ' || lpszString[0] < L'1' || lpszString[0] > L'9')
			return 0;
		*lplpPos = lpszString + 2;
		return (lpszString[0] - L'0');
	}
	if (lpszString[2] != 0x65E5 || lpszString[3] != L' ' ||
		lpszString[1] < L'0' || lpszString[1] > L'9')
		return 0;
	if (lpszString[0] == L' ')
		r = lpszString[1] - L'0';
	else if (lpszString[0] < L'0' || lpszString[0] > L'9')
		return 0;
	else
	{
		r = (lpszString[0] - L'0') * 10;
		r += (lpszString[1] - L'0');
	}
	*lplpPos = lpszString + 3;
	return r;
}

static void __stdcall AppendMillisecondsToSystemTime(SYSTEMTIME* pst, long nMilliseconds)
{
	FILETIME ft;
	ULARGE_INTEGER uli;
	LONGLONG ll;
	::SystemTimeToFileTime(pst, &ft);
	uli.LowPart = ft.dwLowDateTime;
	uli.HighPart = ft.dwHighDateTime;
	ll = Int32x32To64(nMilliseconds, 10000);
	uli.QuadPart += (ULONGLONG) ll;
	ft.dwLowDateTime = uli.LowPart;
	ft.dwHighDateTime = uli.HighPart;
	::FileTimeToSystemTime(&ft, pst);
}

static bool __stdcall ParseYearOrHourMinute(LPCWSTR& lpszString, SYSTEMTIME* pst)
{
	LPCWSTR lpw;
	int n;
	while (*lpszString == L' ')
		lpszString++;
	n = (int) wcstoul(lpszString, (wchar_t**) &lpw, 10);
	if (!lpw)
		return false;
	if (*lpw == L':' && lpw == lpszString + 2)
	{
		pst->wHour = (WORD) n;
		lpszString = lpw + 1;
		n = (int) wcstoul(lpszString, (wchar_t**) &lpw, 10);
		if (!lpw || *lpw != ' ' || lpw != lpszString + 2)
			return false;
		pst->wMinute = (WORD) n;
		lpszString = lpw + 1;
	}
	// 0x5E74: L'年'
	else if (*lpw == 0x5E74 && *(lpw + 1) == L' ')
	{
		pst->wYear = (WORD) n;
		lpszString = lpw + 2;
	}
	else if (*lpw != L' ' || lpw != lpszString + 4)
		return false;
	else
	{
		pst->wYear = (WORD) n;
		lpszString = lpw + 1;
	}
	return true;
}

static bool __stdcall ParseUnixDate(LPCWSTR& lpszString, FILETIME* pftDateTime)
{
	SYSTEMTIME st, stCur;
	int n;
	LPCWSTR lpw;
	bool bUTC;

	bUTC = false;
	st.wYear = 0;
	st.wDayOfWeek = 0;
	st.wHour = st.wMinute = st.wSecond = st.wMilliseconds = 0;
	::GetLocalTime(&stCur);
	n = ParseEnglishMonthName(lpszString);
	if (n)
	{
		lpszString += 4;
		st.wMonth = (WORD) n;
		n = (int) wcstoul(lpszString, (wchar_t**) &lpw, 10);
		if (!lpw || *lpw != L' ' || n < 1 || n > 31)
			return false;
		st.wDay = (WORD) n;
		lpszString = lpw + 1;
		if (!ParseYearOrHourMinute(lpszString, &st))
			return false;
	}
	else
	{
		n = ParseJapaneseMonthName(lpszString);
		if (n)
		{
			if (n >= 10)
				lpszString += 3;
			else
				lpszString += 2;
			st.wMonth = (WORD) n;
			n = ParseJapaneseDayName(lpszString, &lpw);
			if (!n)
				return false;
			st.wDay = (WORD) n;
			lpszString = lpw + 1;
			if (!ParseYearOrHourMinute(lpszString, &st))
				return false;
		}
		else
		{
			// short-iso
			if (lpszString[2] == L'-')
			{
				n = (int) wcstoul(lpszString, (wchar_t**) &lpw, 10);
				if (!lpw || lpw != lpszString + 2)
					return false;
				st.wMonth = (WORD) n;
				lpszString = lpw + 1;
				n = (int) wcstoul(lpszString, (wchar_t**) &lpw, 10);
				if (!lpw || *lpw != L' ')
					return false;
				st.wDay = (WORD) n;
				lpszString = lpw + 1;
			}
			// yyyy-mm-dd, long-iso, full-iso
			else
			{
				n = (int) wcstoul(lpszString, (wchar_t**) &lpw, 10);
				if (!lpw || *lpw != L'-' || lpw != lpszString + 4)
					return false;
				st.wYear = (WORD) n;
				lpszString = lpw + 1;
				n = (int) wcstoul(lpszString, (wchar_t**) &lpw, 10);
				if (!lpw || *lpw != L'-' || lpw != lpszString + 2)
					return false;
				st.wMonth = (WORD) n;
				lpszString = lpw + 1;
				n = (int) wcstoul(lpszString, (wchar_t**) &lpw, 10);
				if (!lpw || *lpw != L' ')
					return false;
				st.wDay = (WORD) n;
				lpszString = lpw + 1;
				n = (int) wcslen(lpszString);
				if (n >= 25 && lpszString[2] == L':' && lpszString[5] == L':' &&
					lpszString[8] == L'.' && lpszString[18] == L' ' &&
					(lpszString[19] == L'+' || lpszString[19] == L'-') &&
					lpszString[24] == L' ')
				{
					// full-iso
					n = (int) wcstoul(lpszString, (wchar_t**) &lpw, 10);
					if (!lpw || *lpw != L':' || lpw != lpszString + 2)
						return false;
					st.wHour = (WORD) n;
					lpszString = lpw + 1;
					n = (int) wcstoul(lpszString, (wchar_t**) &lpw, 10);
					if (!lpw || *lpw != L':' || lpw != lpszString + 2)
						return false;
					st.wMinute = (WORD) n;
					lpszString = lpw + 1;
					n = (int) wcstoul(lpszString, (wchar_t**) &lpw, 10);
					if (!lpw || *lpw != L'.' || lpw != lpszString + 2)
						return false;
					st.wSecond = (WORD) n;
					lpszString = lpw + 1;
					n = (int) wcstoul(lpszString, (wchar_t**) &lpw, 10);
					if (!lpw || *lpw != L' ' || lpw != lpszString + 9)
						return false;
					st.wMilliseconds = (WORD) (n / 1000000);
					lpszString = lpw + 1;
					n = wcstol(lpszString, (wchar_t**) &lpw, 10);
					if (!lpw || *lpw != L' ' || lpw != lpszString + 5)
						return false;
					if (n > 0)
					{
						AppendMillisecondsToSystemTime(&st, -((int) (n / 100) * 60 * 60 * 1000));
						AppendMillisecondsToSystemTime(&st, -((int) (n % 100) * 60 * 1000));
					}
					else if (n < 0)
					{
						n = -n;
						AppendMillisecondsToSystemTime(&st, -((int) (n / 100) * 60 * 60 * 1000));
						AppendMillisecondsToSystemTime(&st, -((int) (n % 100) * 60 * 1000));
					}
					lpszString = lpw + 1;
					bUTC = true;
				}
				else if (n >= 9 && lpszString[2] == L':' && lpszString[5] == L':' &&
					lpszString[8] == L' ')
				{
					// long-iso
					n = (int) wcstoul(lpszString, (wchar_t**) &lpw, 10);
					if (!lpw || *lpw != L':' || lpw != lpszString + 2)
						return false;
					st.wHour = (WORD) n;
					lpszString = lpw + 1;
					n = (int) wcstoul(lpszString, (wchar_t**) &lpw, 10);
					if (!lpw || *lpw != L':' || lpw != lpszString + 2)
						return false;
					st.wMinute = (WORD) n;
					lpszString = lpw + 1;
					n = (int) wcstoul(lpszString, (wchar_t**) &lpw, 10);
					if (!lpw || *lpw != L' ' || lpw != lpszString + 2)
						return false;
					st.wSecond = (WORD) n;
					lpszString = lpw + 1;
				}
				else if (n >= 6 && lpszString[2] == L':' && lpszString[5] == L' ')
				{
					// hh:mm (iso)
					n = (int) wcstoul(lpszString, (wchar_t**) &lpw, 10);
					if (!lpw || *lpw != L':' || lpw != lpszString + 2)
						return false;
					st.wHour = (WORD) n;
					lpszString = lpw + 1;
					n = (int) wcstoul(lpszString, (wchar_t**) &lpw, 10);
					if (!lpw || *lpw != L' ' || lpw != lpszString + 2)
						return false;
					st.wMinute = (WORD) n;
					lpszString = lpw + 1;
				}
			}
		}
	}
	if (!st.wYear)
	{
		st.wYear = stCur.wYear;
		if (st.wMonth >= stCur.wMonth && st.wDay >= stCur.wDay)
			st.wYear--;
	}
	if (!::SystemTimeToFileTime(&st, pftDateTime))
		return false;
	if (!bUTC)
	{
		FILETIME ft;
		::LocalFileTimeToFileTime(pftDateTime, &ft);
		pftDateTime->dwLowDateTime = ft.dwLowDateTime;
		pftDateTime->dwHighDateTime = ft.dwHighDateTime;
	}
	return true;
}

extern "C" CFTPFileItem* __stdcall PickupUnixFileList(CFTPDirectoryBase* pDirectory, LPCWSTR lpszString, LPCWSTR lpszFileName, CFTPFileItem* pItem)
{
	CFTPFileItem* pRet;
	LPCWSTR lpw;
	int nMode;
	UINT nLinkNum, nUID, nGID;
	ULONGLONG nFileSize;
	FILETIME ft;

	if (!ParseUnixAttrMode(lpszString, nMode))
		return NULL;
	lpszString += 10;
	if (*lpszString != L' ')
		return NULL;
	while (*lpszString == L' ')
		lpszString++;

	nLinkNum = wcstoul(lpszString, (wchar_t**) &lpw, 10);
	if (!lpw || *lpw != L' ')
		return NULL;
	lpszString = lpw;
	while (*lpszString == L' ')
		lpszString++;

	nGID = wcstoul(lpszString, (wchar_t**) &lpw, 10);
	if (!lpw || *lpw != L' ')
	{
		lpw = lpszString;
		while (*lpw++ != L' ');
		nGID = 0;
	}
	lpszString = lpw;
	while (*lpszString == L' ')
		lpszString++;

	nUID = wcstoul(lpszString, (wchar_t**) &lpw, 10);
	if (!lpw || *lpw != L' ')
	{
		lpw = lpszString;
		while (*lpw++ != L' ');
		nUID = 0;
	}
	lpszString = lpw;
	while (*lpszString == L' ')
		lpszString++;

	nFileSize = _wcstoui64(lpszString, (wchar_t**) &lpw, 10);
	if (!lpw || *lpw != L' ')
		return NULL;
	lpszString = lpw;
	while (*lpszString == L' ')
		lpszString++;

	if (!ParseUnixDate(lpszString, &ft))
		return NULL;
	while (*lpszString == L' ')
		lpszString++;

	CMyStringW strFile(lpszString), strTarget;
	lpszString = strFile;
	lpw = wcsstr(lpszString, L"->");
	if (lpw)
	{
		LPCWSTR lpw2 = lpw + 2;
		while (*lpw2 == L' ')
			lpw2++;
		while (lpw > lpszString && *(lpw - 1) == L' ')
			lpw--;
		strFile.ReleaseBuffer((DWORD) ((DWORD_PTR) lpw - (DWORD_PTR) lpszString) / sizeof(WCHAR));
		strTarget = lpw2;
	}
	if (lpszFileName && strFile.Compare(lpszFileName) != 0)
		return NULL;

	if (pItem)
		pRet = pItem;
	else
		pRet = new CFTPFileItem(pDirectory);
	if (!pRet)
		return NULL;
	pRet->iIconIndex = -1;
	pRet->bWinAttr = false;
	memset(&pRet->permissions, 0, sizeof(pRet->permissions));
	pRet->nUnixMode = nMode;
	memset(&pRet->ftCreateTime, 0, sizeof(pRet->ftCreateTime));
	memcpy(&pRet->ftModifyTime, &ft, sizeof(ft));
	{
		size_t dwLen = strFile.GetLength();
		LPWSTR lp = pRet->strFileName.GetBuffer(dwLen);
		MyRemoveDotsFromPathW(strFile, lp);
		MyGetFileTitleW(pRet->strFileName, lp, (int) dwLen);
		pRet->strFileName.ReleaseBuffer();
	}
	pRet->strTargetFile = strTarget;
	pRet->uliSize.QuadPart = nFileSize;
	pRet->uUID = nUID;
	pRet->uGID = nGID;
	if (nMode & S_IFDIR)
	{
		pRet->type = fitypeDir;
		pRet->permissions.directory = pRet->permissions.dirCreatable =
			pRet->permissions.creatableInDir = pRet->permissions.listable = 1;
	}
	else
		pRet->type = fitypeFile;
	pRet->permissions.append = pRet->permissions.readable = pRet->permissions.writable =
		pRet->permissions.deletable = pRet->permissions.renameAllowed = 1;

	if (pDirectory)
		pDirectory->MakeUrl(pRet->strFileName, pRet->strUrl);
	return pRet;
}

extern "C" CFTPFileItem* __stdcall ParseUnixFileList(CFTPDirectoryBase* pDirectory, LPCWSTR lpszString)
{
	return PickupUnixFileList(pDirectory, lpszString, NULL, NULL);
}

////////////////////////////////////////////////////////////////////////////////
// parsing DOS-style file list

static WORD s_wNowYear = 0;

static bool __stdcall ParseDOSDate(LPCWSTR& lpszString, FILETIME* pft, char& nYearFollows, bool& bY2KProblem)
{
	LPCWSTR lpw;
	WCHAR wchDelimiter;
	SYSTEMTIME st;
	UINT n;

	if (!s_wNowYear)
	{
		::GetSystemTime(&st);
		s_wNowYear = st.wYear;
	}
	st.wDayOfWeek = st.wSecond = st.wMilliseconds = 0;
	while (*lpszString == L' ')
		lpszString++;
	n = wcstoul(lpszString, (wchar_t**) &lpw, 10);
	if (!lpw)
		return false;
	if (*lpw == L'/')
		wchDelimiter = L'/';
	else if (*lpw == L'-')
		wchDelimiter = L'-';
	else
		return false;
	if (lpw == lpszString + 4 || ((lpw == lpszString + 2 || lpw == lpszString + 3) && (nYearFollows > 0 || n > 12)))
	{
		if (lpw != lpszString + 4)
		{
			nYearFollows = 1;
			if (n >= 100)
			{
				if (lpw != lpszString + 3)
					return false;
				bY2KProblem = true;
				n += 1900;
			}
			else
			{
				if (lpw != lpszString + 2)
					return false;
				register WORD w = (WORD)(s_wNowYear / 100) * 100;
				if (s_wNowYear - w < (WORD) n)
					w -= 100;
				n += w;
			}
		}
		st.wYear = (WORD) n;
		lpszString = lpw + 1;
		n = wcstoul(lpszString, (wchar_t**) &lpw, 10);
		if (!lpw || *lpw != wchDelimiter || lpw != lpszString + 2)
			return false;
		st.wMonth = (WORD) n;
		lpszString = lpw + 1;
		n = wcstoul(lpszString, (wchar_t**) &lpw, 10);
		if (!lpw || *lpw != L' ' || lpw != lpszString + 2)
			return false;
		st.wDay = (WORD) n;
		lpszString = lpw + 1;
	}
	else if (lpw == lpszString + 2)
	{
		nYearFollows = -1;
		st.wMonth = (WORD) n;
		lpszString = lpw + 1;
		n = wcstoul(lpszString, (wchar_t**) &lpw, 10);
		if (!lpw || *lpw != wchDelimiter || lpw != lpszString + 2)
			return false;
		st.wDay = (WORD) n;
		lpszString = lpw + 1;
		n = wcstoul(lpszString, (wchar_t**) &lpw, 10);
		if (!lpw || *lpw != L' ')
			return false;
		if (n >= 100)
		{
			if (lpw != lpszString + 3)
				return false;
			bY2KProblem = true;
			n += 1900;
		}
		else
		{
			if (lpw != lpszString + 2)
				return false;
			register WORD w = (WORD)(s_wNowYear / 100) * 100;
			if (s_wNowYear - w < (WORD) n)
				w -= 100;
			n += w;
		}
		st.wYear = (WORD) n;
		lpszString = lpw + 1;
	}
	else
		return false;
	while (*lpszString == L' ')
		lpszString++;
	n = wcstoul(lpszString, (wchar_t**) &lpw, 10);
	if (!lpw || *lpw != L':' || lpw != lpszString + 2)
		return false;
	// 12 or 24 hours
	st.wHour = (WORD) n;
	lpszString = lpw + 1;
	n = wcstoul(lpszString, (wchar_t**) &lpw, 10);
	if (!lpw || lpw != lpszString + 2)
		return false;
	st.wMinute = (WORD) n;
	lpszString = lpw + 1;
	if (*lpw == L':')
	{
		// seconds
		n = wcstoul(lpszString, (wchar_t**) &lpw, 10);
		if (!lpw || *lpw != L' ' || lpw != lpszString + 2)
			return false;
		st.wSecond = (WORD) n;
		lpszString = lpw + 1;
	}
	else if (*lpw == L'a' || *lpw == L'A' || *lpw == L'p' || *lpw == L'P')
	{
		if (*lpw == L'p' || *lpw == L'P')
		{
			if (st.wHour != 12)
				st.wHour += 12;
		}
		lpw++;
		if (*lpw == L'm' || *lpw == L'M')
			lpw++;
		if (*lpw != L' ')
			return false;
		lpszString = lpw + 1;
	}
	else if (*lpw != L' ')
		return false;
	{
		FILETIME ft;
		::SystemTimeToFileTime(&st, &ft);
		::LocalFileTimeToFileTime(&ft, pft);
	}
	return true;
}

// *pnYearFollows ... year flags (>0 ... yy-mm-dd formats, <0 ... mm-dd-yy formats)
extern "C" CFTPFileItem* __stdcall PickupDOSFileList(CFTPDirectoryBase* pDirectory, LPCWSTR lpszString, LPCWSTR lpszFileName, CFTPFileItem* pItem, char* pnYearFollows, bool* pbY2KProblem)
{
	CFTPFileItem* pRet;
	LPCWSTR lpw;
	DWORD dwAttributes;
	ULONGLONG nFileSize;
	FILETIME ft;

	if (!ParseDOSDate(lpszString, &ft, *pnYearFollows, *pbY2KProblem))
		return NULL;
	while (*lpszString == L' ')
		lpszString++;
	if (!_wcsnicmp(lpszString, L"<DIR>", 5) && lpszString[5] == L' ')
	{
		nFileSize = 0;
		dwAttributes = FILE_ATTRIBUTE_DIRECTORY;
		lpszString += 5;
	}
	else
	{
		nFileSize = _wcstoui64(lpszString, (wchar_t**) &lpw, 10);
		if (!lpw || *lpw != L' ')
			return NULL;
		dwAttributes = FILE_ATTRIBUTE_NORMAL;
		lpszString = lpw;
	}

	while (*lpszString == L' ')
		lpszString++;

	if (lpszFileName && _wcsicmp(lpszString, lpszFileName) != 0)
		return NULL;

	if (pItem)
		pRet = pItem;
	else
		pRet = new CFTPFileItem(pDirectory);
	if (!pRet)
		return NULL;
	pRet->iIconIndex = -1;
	pRet->bWinAttr = true;
	memset(&pRet->permissions, 0, sizeof(pRet->permissions));
	pRet->dwAttributes = dwAttributes;
	memset(&pRet->ftCreateTime, 0, sizeof(pRet->ftCreateTime));
	memcpy(&pRet->ftModifyTime, &ft, sizeof(ft));
	{
		size_t nLen = wcslen(lpszString);
		LPWSTR lp = pRet->strFileName.GetBuffer((DWORD) nLen);
		MyRemoveDotsFromPathW(lpszString, lp);
		MyGetFileTitleW(pRet->strFileName, lp, (int) nLen);
		pRet->strFileName.ReleaseBuffer();
	}
	pRet->uUID = pRet->uGID = 0;
	pRet->uliSize.QuadPart = nFileSize;
	if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		pRet->type = fitypeDir;
		pRet->permissions.directory = pRet->permissions.dirCreatable =
			pRet->permissions.creatableInDir = pRet->permissions.listable = 1;
	}
	else
		pRet->type = fitypeFile;
	pRet->permissions.append = pRet->permissions.readable = pRet->permissions.writable =
		pRet->permissions.deletable = pRet->permissions.renameAllowed = 1;

	if (pDirectory)
		pDirectory->MakeUrl(pRet->strFileName, pRet->strUrl);
	return pRet;
}

// *pnYearFollows ... year flags (>0 ... yy-mm-dd formats, <0 ... mm-dd-yy formats)
extern "C" CFTPFileItem* __stdcall ParseDOSFileList(CFTPDirectoryBase* pDirectory, LPCWSTR lpszString, char* pnYearFollows, bool* pbY2KProblem)
{
	return PickupDOSFileList(pDirectory, lpszString, NULL, NULL, pnYearFollows, pbY2KProblem);
}

////////////////////////////////////////////////////////////////////////////////
// parsing MLSx

static bool __stdcall _IsEqualKey(LPCWSTR lpszString, LPCWSTR lpszKey)
{
	size_t n = wcslen(lpszKey);
	return !_wcsnicmp(lpszString, lpszKey, n) && lpszString[n] == L'=';
}

static int __stdcall _TypeNameToType(LPCWSTR lpszString, LPCWSTR lpszStop)
{
	size_t nSize;

	if (!lpszStop)
		nSize = wcslen(lpszString);
	else
		nSize = ((DWORD) ((LPCBYTE) lpszStop - (LPCBYTE) lpszString)) / sizeof(WCHAR);
	if (!_wcsnicmp(lpszString, L"cdir", nSize))
		return fitypeCurDir;
	if (!_wcsnicmp(lpszString, L"pdir", nSize))
		return fitypeParentDir;
	if (!_wcsnicmp(lpszString, L"dir", nSize))
		return fitypeDir;
	if (!_wcsnicmp(lpszString, L"file", nSize))
		return fitypeFile;
	return 0;
}

static void __stdcall _PermStringToPerm(LPCWSTR lpszString, LPCWSTR lpszStop, CPermissionData* pPerm)
{
	if (!lpszStop)
	{
		lpszStop = lpszString;
		while (*lpszStop)
			lpszStop++;
	}
	while (lpszString < lpszStop)
	{
		switch (*lpszString++)
		{
			case 'a': case 'A': pPerm->append = 1; break;
			case 'c': case 'C': pPerm->creatableInDir = 1; break;
			case 'd': case 'D': pPerm->deletable = 1; break;
			case 'e': case 'E': pPerm->directory = 1; break;
			case 'f': case 'F': pPerm->renameAllowed = 1; break;
			case 'l': case 'L': pPerm->listable = 1; break;
			case 'm': case 'M': pPerm->dirCreatable = 1; break;
			case 'p': case 'P': pPerm->purgable = 1; break;
			case 'r': case 'R': pPerm->readable = 1; break;
			case 'w': case 'W': pPerm->writable = 1; break;
		}
	}
}

#define REACH_STOP(str, stop) (str == stop)

extern "C" bool __stdcall _ParseToFileTime(LPCWSTR lpszString, LPCWSTR lpszStop, FILETIME* pftTime)
{
	SYSTEMTIME st;
	//FILETIME ft;
	if (!lpszStop)
	{
		lpszStop = lpszString;
		while (*lpszStop)
			lpszStop++;
	}

	st.wYear = (*lpszString++ - L'0') * 1000; if (REACH_STOP(lpszString, lpszStop)) return false;
	st.wYear += (*lpszString++ - L'0') * 100; if (REACH_STOP(lpszString, lpszStop)) return false;
	st.wYear += (*lpszString++ - L'0') * 10; if (REACH_STOP(lpszString, lpszStop)) return false;
	st.wYear += (*lpszString++ - L'0'); if (REACH_STOP(lpszString, lpszStop)) return false;
	st.wMonth = (*lpszString++ - L'0') * 10; if (REACH_STOP(lpszString, lpszStop)) return false;
	st.wMonth += (*lpszString++ - L'0'); if (REACH_STOP(lpszString, lpszStop)) return false;
	st.wDay = (*lpszString++ - L'0') * 10; if (REACH_STOP(lpszString, lpszStop)) return false;
	st.wDay += (*lpszString++ - L'0'); if (REACH_STOP(lpszString, lpszStop)) return false;
	st.wDayOfWeek = 0;
	st.wHour = (*lpszString++ - L'0') * 10; if (REACH_STOP(lpszString, lpszStop)) return false;
	st.wHour += (*lpszString++ - L'0'); if (REACH_STOP(lpszString, lpszStop)) return false;
	st.wMinute = (*lpszString++ - L'0') * 10; if (REACH_STOP(lpszString, lpszStop)) return false;
	st.wMinute += (*lpszString++ - L'0'); if (REACH_STOP(lpszString, lpszStop)) return false;
	st.wSecond = (*lpszString++ - L'0') * 10; if (REACH_STOP(lpszString, lpszStop)) return false;
	st.wSecond += (*lpszString++ - L'0');
	if (lpszStop ? lpszString != lpszStop : *lpszString)
	{
		if (*lpszString == L'.')
		{
			lpszString++; if (REACH_STOP(lpszString, lpszStop)) return false;
			st.wMilliseconds = (*lpszString++ - L'0') * 100; if (REACH_STOP(lpszString, lpszStop)) return false;
			st.wMilliseconds += (*lpszString++ - L'0') * 10; if (REACH_STOP(lpszString, lpszStop)) return false;
			st.wMilliseconds += (*lpszString++ - L'0');
			if (lpszStop ? lpszString != lpszStop : *lpszString)
				return false;
		}
		else
			return false;
	}
	else
		st.wMilliseconds = 0;
	//if (!::SystemTimeToFileTime(&st, &ft))
	//	return false;
	//if (!::FileTimeToLocalFileTime(&ft, pftTime))
	//	return false;
	if (!::SystemTimeToFileTime(&st, pftTime))
		return false;
	return true;
}

extern "C" CFTPFileItem* __stdcall ParseMLSxData(CFTPDirectoryBase* pDirectory, LPCWSTR lpszString)
{
	return ParseMLSxDataEx(pDirectory, lpszString, NULL);
}

extern "C" CFTPFileItem* __stdcall ParseMLSxDataEx(CFTPDirectoryBase* pDirectory, LPCWSTR lpszString, CFTPFileItem* pItem)
{
	LPCWSTR lp, lpEq, lp2;
	CFTPFileItem* pRet;

	while (*lpszString == L' ')
		lpszString++;

	if (pItem)
		pRet = pItem;
	else
		pRet = new CFTPFileItem(pDirectory);
	pRet->iIconIndex = -1;
	pRet->bWinAttr = false;
	pRet->type = fitypeFile;
	memset(&pRet->permissions, 0, sizeof(pRet->permissions));
	pRet->nUnixMode = 0;
	memset(&pRet->ftCreateTime, 0, sizeof(pRet->ftCreateTime));
	memset(&pRet->ftModifyTime, 0, sizeof(pRet->ftModifyTime));
	pRet->uliSize.QuadPart = 0;
	while (*lpszString)
	{
		lp = wcschr(lpszString, L';');
		lpEq = wcschr(lpszString, L'=') + 1;

		if (_IsEqualKey(lpszString, L"Size"))
		{
			pRet->uliSize.QuadPart = _wcstoui64(lpEq, (wchar_t**) &lp2, 10);
		}
		else if (_IsEqualKey(lpszString, L"Create"))
		{
			_ParseToFileTime(lpEq, lp, &pRet->ftCreateTime);
		}
		else if (_IsEqualKey(lpszString, L"Modify"))
		{
			_ParseToFileTime(lpEq, lp, &pRet->ftModifyTime);
		}
		else if (_IsEqualKey(lpszString, L"Type"))
		{
			pRet->type = _TypeNameToType(lpEq, lp);
		}
		//else if (_IsEqualKey(lpszString, L"Unique"))
		//{
		//}
		else if (_IsEqualKey(lpszString, L"Perm"))
		{
			_PermStringToPerm(lpEq, lp, &pRet->permissions);
		}
		//else if (_IsEqualKey(lpszString, L"Lang"))
		//{
		//}
		//else if (_IsEqualKey(lpszString, L"Media-Type"))
		//{
		//}
		//else if (_IsEqualKey(lpszString, L"CharSet"))
		//{
		//}
		else if (_IsEqualKey(lpszString, L"Unix.mode"))
		{
			pRet->nUnixMode = (int) wcstol(lpEq, (wchar_t**) &lp2, 8);
		}
		else if (_IsEqualKey(lpszString, L"Unix.owner"))
		{
			pRet->uUID = (UINT) wcstoul(lpEq, (wchar_t**) &lp2, 10);
		}
		else if (_IsEqualKey(lpszString, L"Unix.group"))
		{
			pRet->uGID = (UINT) wcstoul(lpEq, (wchar_t**) &lp2, 10);
		}

		if (!lp)
			break;
		lpszString = lp + 1;
		//while (*lpszString && *lpszString == L' ')
		//	lpszString++;
		if (*lpszString == L' ')
		{
			while (*lpszString == L' ')
				lpszString++;
			lp = wcsrchr(lpszString, L'/');
			if (lp)
				lpszString = lp + 1;
			pRet->strFileName = lpszString;
			break;
		}
	}
	if (pDirectory)
		pDirectory->MakeUrl(pRet->strFileName, pRet->strUrl);
	return pRet;
}

////////////////////////////////////////////////////////////////////////////////

extern "C" void __stdcall TimetToFileTime(time_t t, LPFILETIME pft)
{
	LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD) ll;
	pft->dwHighDateTime = ll >>32;
}

extern "C" void __stdcall Time64AndNanoToFileTime(ULONGLONG uliTime64, DWORD dwNano, LPFILETIME pft)
{
	LONGLONG ll = uliTime64 * 10000000 + Int32x32To64(dwNano, 10) + 116444736000000000;
	pft->dwLowDateTime = (DWORD) ll;
	pft->dwHighDateTime = ll >>32;
}

extern "C" void __stdcall FileTimeToTimet(time_t* pt, const FILETIME* pft)
{
	ULONGLONG ll = (((ULONGLONG) pft->dwHighDateTime) << 32) + pft->dwLowDateTime;
	ll -= 116444736000000000;
	ll /= 10000000;
	*pt = (time_t) (ll & 0xFFFFFFFF);
}

extern "C" void __stdcall FileTimeToTime64AndNano(ULONGLONG* puliTime64, DWORD* pdwNano, const FILETIME* pft)
{
	ULONGLONG ll = (((ULONGLONG) pft->dwHighDateTime) << 32) + pft->dwLowDateTime;
	ll -= 116444736000000000;
	ll /= 10;
	ULONGLONG ll2 = ll / 10000000;
	*pdwNano = (DWORD) (ll - ll2 * 10000000);
	*puliTime64 = ll2;
}

extern "C" void __stdcall ParseSFTPAttributes(ULONG uServerVersion, CFTPFileItem* pItem, const CSFTPFileAttribute* pAttr)
{
	int nMode;

	if (pAttr->dwMask & SSH_FILEXFER_ATTR_PERMISSIONS)
		nMode = (int) pAttr->dwPermissions;
	else
		nMode = 0777;
	memset(&pItem->permissions, 0, sizeof(pItem->permissions));
	pItem->nUnixMode = nMode;
	memset(&pItem->ftCreateTime, 0, sizeof(pItem->ftCreateTime));
	memset(&pItem->ftModifyTime, 0, sizeof(pItem->ftModifyTime));
	pItem->uUID = pItem->uGID = 0;
	if (uServerVersion >= 4)
	{
		if (pAttr->dwMask & SSH_FILEXFER_ATTR_MODIFYTIME)
			Time64AndNanoToFileTime(pAttr->dwModifiedTime,
				pAttr->dwModifiedTimeNano,
				&pItem->ftModifyTime);
		if (pAttr->dwMask & SSH_FILEXFER_ATTR_CREATETIME)
			Time64AndNanoToFileTime(pAttr->dwCreateTime,
				pAttr->dwCreateTimeNano,
				&pItem->ftCreateTime);
		if (pAttr->dwMask & SSH_FILEXFER_ATTR_OWNERGROUP)
		{
			pItem->strOwner = pAttr->strOwner;
			pItem->strGroup = pAttr->strGroup;
		}
		// deprecated
		if (pAttr->dwMask & SSH_FILEXFER_ATTR_UIDGID)
		{
			pItem->uUID = pAttr->uUserID;
			pItem->uGID = pAttr->uGroupID;
		}
	}
	else
	{
		if (pAttr->dwMask & SSH_FILEXFER_ATTR_ACMODTIME)
		{
			TimetToFileTime((time_t) pAttr->dwModifiedTime, &pItem->ftModifyTime);
			pItem->ftCreateTime = {};
		}
		if (pAttr->dwMask & SSH_FILEXFER_ATTR_UIDGID)
		{
			pItem->uUID = pAttr->uUserID;
			pItem->uGID = pAttr->uGroupID;
		}
	}
	if (pAttr->dwMask & SSH_FILEXFER_ATTR_SIZE)
		pItem->uliSize.QuadPart = pAttr->uliSize.QuadPart;
	else
		pItem->uliSize.QuadPart = 0;

	if (pAttr->bFileType == SSH_FILEXFER_TYPE_DIRECTORY)
	{
		pItem->type = fitypeDir;
		pItem->permissions.directory = pItem->permissions.dirCreatable =
			pItem->permissions.creatableInDir = pItem->permissions.listable = 1;
	}
	else
		pItem->type = fitypeFile;
	pItem->permissions.append = pItem->permissions.readable = pItem->permissions.writable =
		pItem->permissions.deletable = pItem->permissions.renameAllowed = 1;
}

extern "C" CFTPFileItem* __stdcall ParseSFTPData(CFTPDirectoryBase* pDirectory, ULONG uServerVersion, const CSFTPFileData* pFileData)
{
	CFTPFileItem* pRet;

	pRet = new CFTPFileItem(pDirectory);
	pRet->iIconIndex = -1;
	pRet->bWinAttr = false;
	pRet->strFileName = pFileData->strFileName;
	ParseSFTPAttributes(uServerVersion, pRet, &pFileData->attr);
	if (pDirectory)
		pDirectory->MakeUrl(pRet->strFileName, pRet->strUrl);
	return pRet;
}

///////////////////////////////////////////////////////////////////////////////

CFTPFileItem::CFTPFileItem(CFTPDirectoryBase* pParent)
	: CDispatchImplNoUnknownT(theApp.GetTypeInfo(IID_IEasySFTPFile))
	, CReferenceDelegationChild(pParent)
	, pTargetFile(NULL)
{
}

CFTPFileItem::~CFTPFileItem()
{
	if (pTargetFile)
		pTargetFile->Release();
}

STDMETHODIMP CFTPFileItem::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	*ppv = NULL;
	if (!IsEqualIID(riid, IID_IUnknown) && !IsEqualIID(riid, IID_IDispatch) &&
		!IsEqualIID(riid, IID_IEasySFTPFile))
		return E_NOINTERFACE;
	*ppv = static_cast<IEasySFTPFile*>(this);
	AddRef();
	return S_OK;
}

STDMETHODIMP CFTPFileItem::get_FileName(BSTR* pFileName)
{
	if (!pFileName)
		return E_POINTER;
	*pFileName = MyStringToBSTR(strFileName);
	return *pFileName ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CFTPFileItem::get_IsDirectory(VARIANT_BOOL* pbRet)
{
	if (!pbRet)
		return E_POINTER;
	*pbRet = IsDirectory() ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CFTPFileItem::get_IsHidden(VARIANT_BOOL* pbRet)
{
	if (!pbRet)
		return E_POINTER;
	*pbRet = IsHidden() ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CFTPFileItem::get_IsShortcut(VARIANT_BOOL* pbRet)
{
	if (!pbRet)
		return E_POINTER;
	*pbRet = IsShortcut() ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CFTPFileItem::get_CreateTime(DATE* pdRet)
{
	if (!pdRet)
		return E_POINTER;
	SYSTEMTIME st;
	if (!::FileTimeToSystemTime(&ftCreateTime, &st))
		*pdRet = 0;
	else if (!::SystemTimeToVariantTime(&st, pdRet))
		*pdRet = 0;
	return S_OK;
}

STDMETHODIMP CFTPFileItem::get_ModifyTime(DATE* pdRet)
{
	if (!pdRet)
		return E_POINTER;
	SYSTEMTIME st;
	if (!::FileTimeToSystemTime(&ftModifyTime, &st))
		*pdRet = 0;
	else if (!::SystemTimeToVariantTime(&st, pdRet))
		*pdRet = 0;
	return S_OK;
}

STDMETHODIMP CFTPFileItem::get_Size(hyper* piRet)
{
	if (!piRet)
		return E_POINTER;
	*piRet = static_cast<hyper>(uliSize.QuadPart);
	return S_OK;
}

STDMETHODIMP CFTPFileItem::get_Size32Bit(long* piRet)
{
	if (!piRet)
		return E_POINTER;
	if (uliSize.QuadPart > 0x7fffffff)
		*piRet = 0x7fffffff;
	else
		*piRet = static_cast<long>(uliSize.QuadPart);
	return S_OK;
}

STDMETHODIMP CFTPFileItem::get_Uid(long* piRet)
{
	if (!piRet)
		return E_POINTER;
	*piRet = static_cast<long>(uUID);
	return S_OK;
}

STDMETHODIMP CFTPFileItem::get_Gid(long* piRet)
{
	if (!piRet)
		return E_POINTER;
	*piRet = static_cast<long>(uGID);
	return S_OK;
}

STDMETHODIMP CFTPFileItem::get_OwnerName(BSTR* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = MyStringToBSTR(strOwner);
	return *pRet ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CFTPFileItem::get_GroupName(BSTR* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = MyStringToBSTR(strGroup);
	return *pRet ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CFTPFileItem::get_ItemType(BSTR* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = MyStringToBSTR(strType);
	return *pRet ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CFTPFileItem::get_Url(BSTR* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = MyStringToBSTR(strUrl);
	return *pRet ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CFTPFileItem::get_Directory(IEasySFTPDirectory** ppRet)
{
	if (!ppRet)
		return E_POINTER;
	auto* pParent = GetParent();
	if (!pParent)
	{
		*ppRet = NULL;
		return S_OK;
	}
	return pParent->QueryInterface(IID_IEasySFTPDirectory, reinterpret_cast<void**>(ppRet));
}

STDMETHODIMP CFTPFileItem::get_HasWinAttribute(VARIANT_BOOL* pbRet)
{
	if (!pbRet)
		return E_POINTER;
	*pbRet = bWinAttr ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CFTPFileItem::get_WinAttribute(long* piRet)
{
	if (!piRet)
		return E_POINTER;
	*piRet = bWinAttr ? static_cast<long>(dwAttributes) : 0;
	return S_OK;
}

STDMETHODIMP CFTPFileItem::get_UnixAttribute(long* piRet)
{
	if (!piRet)
		return E_POINTER;
	*piRet = bWinAttr ? 0 : static_cast<long>(nUnixMode);
	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

CFTPFileItemList::CFTPFileItemList(CFTPDirectoryBase* pDirectory)
	: CDispatchImplT(theApp.GetTypeInfo(IID_IEasySFTPFiles))
	, m_pDirectory(pDirectory)
{
	pDirectory->AddRef();
}

CFTPFileItemList::~CFTPFileItemList()
{
	m_pDirectory->Release();
}

STDMETHODIMP CFTPFileItemList::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	*ppv = NULL;
	if (!IsEqualIID(riid, IID_IUnknown) && !IsEqualIID(riid, IID_IDispatch) &&
		!IsEqualIID(riid, IID_IEasySFTPFiles))
		return E_NOINTERFACE;
	*ppv = static_cast<IEasySFTPFiles*>(this);
	AddRef();
	return S_OK;
}

STDMETHODIMP CFTPFileItemList::get_Item(VARIANT key, IEasySFTPFile** ppFile)
{
	if (!ppFile)
		return E_POINTER;
	if (key.vt == VT_BSTR)
	{
		CMyStringW str;
		MyBSTRToString(key.bstrVal, str);
		for (int i = 0; i < m_pDirectory->m_aFiles.GetCount(); ++i)
		{
			auto* p = m_pDirectory->m_aFiles.GetItem(i);
			if (p->strFileName.Compare(str) == 0)
			{
				return p->QueryInterface(IID_IEasySFTPFile, reinterpret_cast<void**>(ppFile));
			}
		}
		return DISP_E_BADINDEX;
	}
	else
	{
		ULONGLONG i = 0;
		switch (key.vt)
		{
		case VT_I1:
			if (key.cVal < 0)
				return DISP_E_BADINDEX;
			i = static_cast<ULONGLONG>(key.cVal);
			break;
		case VT_I2:
			if (key.iVal < 0)
				return DISP_E_BADINDEX;
			i = static_cast<ULONGLONG>(key.iVal);
			break;
		case VT_I4:
			if (key.lVal < 0)
				return DISP_E_BADINDEX;
			i = static_cast<ULONGLONG>(key.lVal);
			break;
		case VT_INT:
			if (key.intVal < 0)
				return DISP_E_BADINDEX;
			i = static_cast<ULONGLONG>(key.intVal);
			break;
		case VT_I8:
			if (key.llVal < 0)
				return DISP_E_BADINDEX;
			i = static_cast<ULONGLONG>(key.llVal);
			break;
		case VT_UI1:
			i = static_cast<ULONGLONG>(key.bVal);
			break;
		case VT_UI2:
			i = static_cast<ULONGLONG>(key.uiVal);
			break;
		case VT_UI4:
			i = static_cast<ULONGLONG>(key.ulVal);
			break;
		case VT_UINT:
			i = static_cast<ULONGLONG>(key.uintVal);
			break;
		case VT_UI8:
			i = static_cast<ULONGLONG>(key.ullVal);
			break;
		default:
			return DISP_E_TYPEMISMATCH;
		}
		if (i >= m_pDirectory->m_aFiles.GetCount())
			return DISP_E_BADINDEX;
		return m_pDirectory->m_aFiles.GetItem(static_cast<int>(i))->QueryInterface(IID_IEasySFTPFile, reinterpret_cast<void**>(ppFile));
	}
}

STDMETHODIMP CFTPFileItemList::get_Count(long* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = m_pDirectory->m_aFiles.GetCount();
	return S_OK;
}

STDMETHODIMP CFTPFileItemList::get__NewEnum(IUnknown** ppRet)
{
	if (!ppRet)
		return E_POINTER;
	*ppRet = new CEnum(this);
	return *ppRet ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CFTPFileItemList::CEnum::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	*ppv = NULL;
	if (!IsEqualIID(riid, IID_IUnknown) &&
		!IsEqualIID(riid, IID_IEnumVARIANT))
		return E_NOINTERFACE;
	*ppv = static_cast<IEnumVARIANT*>(this);
	AddRef();
	return S_OK;
}

STDMETHODIMP CFTPFileItemList::CEnum::Next(ULONG celt, VARIANT* rgelt, ULONG* pceltFetched)
{
	if (!rgelt)
		return E_POINTER;
	if (!celt)
		return S_OK;
	if (pceltFetched)
		*pceltFetched = 0;
	auto count = static_cast<ULONG>(m_pList->m_pDirectory->m_aFiles.GetCount());
	while (m_uPos < count)
	{
		CFTPFileItem* pItem = m_pList->m_pDirectory->m_aFiles.GetItem((int)(m_uPos++));
		rgelt->vt = VT_DISPATCH;
		rgelt->pdispVal = static_cast<IDispatch*>(pItem);
		pItem->AddRef();
		if (pceltFetched)
			(*pceltFetched)++;
		rgelt++;
		if (!--celt)
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CFTPFileItemList::CEnum::Skip(ULONG celt)
{
	if (!celt)
		return S_OK;
	auto count = static_cast<ULONG>(m_pList->m_pDirectory->m_aFiles.GetCount());
	if (m_uPos + celt > count)
	{
		m_uPos = count;
		return S_FALSE;
	}
	m_uPos += celt;
	return S_OK;
}

STDMETHODIMP CFTPFileItemList::CEnum::Reset()
{
	m_uPos = 0;
	return S_OK;
}

STDMETHODIMP CFTPFileItemList::CEnum::Clone(IEnumVARIANT** ppEnum)
{
	if (!ppEnum)
		return E_POINTER;
	CEnum* pEnum = new CEnum(m_pList);
	if (!pEnum)
		return E_OUTOFMEMORY;
	pEnum->m_uPos = m_uPos;
	*ppEnum = pEnum;
	return S_OK;
}
