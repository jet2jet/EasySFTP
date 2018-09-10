/*
 Copyright (C) 2010 Kuri-Applications

 unicode.h - declarations of utility functions for Unicode
 */

#ifndef __UNICODE_H__
#define __UNICODE_H__

LPWSTR __stdcall AnsiToUnicode(LPCSTR lpszString, UINT uLen);

///////////////////////////////////////////////////////////////////////////////

//#include "Convert.h"
#include "UString.h"

EXTERN_C __out HANDLE WINAPI MyFindFirstFileW(__in LPCWSTR lpFileName, __out LPWIN32_FIND_DATAW lpFindFileData);
EXTERN_C BOOL WINAPI MyFindNextFileW(__in  HANDLE hFindFile, __out LPWIN32_FIND_DATAW lpFindFileData);

EXTERN_C HANDLE WINAPI MyCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess,
	DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
EXTERN_C BOOL WINAPI MyCreateDirectoryW(LPCWSTR lpszDirectoryName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
EXTERN_C BOOL WINAPI MyRemoveDirectoryW(LPCWSTR lpPathName);
EXTERN_C BOOL WINAPI MyDeleteFileW(LPCWSTR lpszFileName);

EXTERN_C BOOL WINAPI MyIsEmptyDirectoryW(LPCWSTR lpszPathName);
// return: <0 means error, 0 means some files were not deleted, >0 means succeeded
EXTERN_C int WINAPI MyRemoveDirectoryRecursiveW(LPCWSTR lpPathName);

extern "C" bool __stdcall MyIsExistFileW(LPCWSTR lpszFileName);
extern "C" bool __stdcall MyIsDirectoryW(LPCWSTR lpszFileName);
extern "C" int __stdcall MySearchPathW(LPCWSTR lpszPathName, LPWSTR lpszBuffer, int nMaxLen);

///////////////////////////////////////////////////////////////////////////////

void __stdcall MyGetFullPathStringW(LPCWSTR lpszPath, LPCWSTR lpszFile, CMyStringW& rstrBuffer);
int __stdcall MyGetFileTitleStringW(LPCWSTR lpszFile, CMyStringW& rstrFileTitle);
void __stdcall MyRemoveDotsFromPathStringW(CMyStringW& rstrPath);
void __stdcall MyGetAbsolutePathStringW(LPCWSTR lpszRelativePathName, LPCWSTR lpszDirectory, CMyStringW& rstrBuffer);
void __stdcall MyGetRelativePathStringW(LPCWSTR lpszFullPathName, LPCWSTR lpszDirectory, CMyStringW& rstrBuffer);
void __stdcall MyGetLongPathNameStringW(LPCWSTR lpszPath, CMyStringW& rstrBuffer);
void __stdcall MyMakeFullPathFromCurDirStringW(LPCWSTR lpszPathName, CMyStringW& rstrBuffer);
bool __stdcall MySearchPathStringW(LPCWSTR lpszPathName, CMyStringW& rstrBuffer);
void __stdcall MyReplaceStringW(CMyStringW& rstrText, LPCWSTR lpszFind, LPCWSTR lpszReplace);
#if defined(GUID_DEFINED) && defined(__LPGUID_DEFINED__) && defined(_REFGUID_DEFINED)
void __stdcall MyStringFromGUIDW(REFGUID rguid, CMyStringW& rstrRet);
#endif

#ifdef _OBJBASE_H_
BSTR __stdcall MyStringToBSTR(
#ifndef OLE2ANSI
	const
#endif
	CMyStringW& string);
void __stdcall MyBSTRToString(BSTR bstr, CMyStringW& rstrString);
#endif

#ifdef _INC_SHELLAPI
EXTERN_C int __stdcall MyDragQueryFileCount(HDROP hDrop);
void __stdcall MyDragQueryFileStringW(HDROP hDrop, UINT iFile, CMyStringW& rstrFile);
EXTERN_C void __stdcall MyDragFinish(HDROP hDrop);
#endif

EXTERN_C ATOM WINAPI MyAddAtomW(LPCWSTR lpString);
EXTERN_C ATOM WINAPI MyFindAtomW(LPCWSTR lpString);
EXTERN_C ATOM WINAPI MyDeleteAtom(ATOM nAtom);

///////////////////////////////////////////////////////////////////////////////

void __stdcall _GetDlgRichTextUnicodeText(HWND hDlg, int nIDDlgItem, CMyStringW& rString);
void __stdcall _SetDlgRichTextUnicodeText(HWND hDlg, int nIDDlgItem, LPCWSTR lpszString);
void __stdcall MyGetWindowTextStringW(HWND hWnd, CMyStringW& rString);
extern "C" bool __stdcall MySetWindowTextW(HWND hWnd, LPCWSTR lpszString);
extern "C" bool __stdcall MySetDlgItemTextW(HWND hDlg, int nIDDlgItem, LPCWSTR lpszString);
void __stdcall MyGetDlgUnicodeText(HWND hDlg, int nIDDlgItem, CMyStringW& rString);
void __stdcall SyncDialogData(HWND hWnd, int nID, CMyStringW& rString, bool bGet);
extern "C" int __stdcall AddDlgListBoxStringW(HWND hWnd, int nID, LPCWSTR lpszString);
extern "C" int __stdcall InsertDlgListBoxStringW(HWND hWnd, int nID, int nIndex, LPCWSTR lpszString);
extern "C" int __stdcall AddDlgComboBoxStringW(HWND hWnd, int nID, LPCWSTR lpszString);

#endif // __UNICODE_H__
