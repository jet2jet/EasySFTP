/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 INIFile.h - declarations of functions for INI file
 */

#ifndef __INIFILE_H__
#define __INIFILE_H__

DECLARE_HANDLE(HINIFILE);

// INI ファイルをファイルハンドルから読み込む (すべて UTF-8 で)

HINIFILE __stdcall MyLoadINIFileA(LPCSTR lpszFileName, bool bUseComment);
HINIFILE __stdcall MyLoadINIFileW(LPCWSTR lpszFileName, bool bUseComment);
#ifdef _UNICODE
#define MyLoadINIFile    MyLoadINIFileW
#else
#define MyLoadINIFile    MyLoadINIFileA
#endif
// if lpszSection is NULL, then searches global keys
// the result of MyGetProfileSectionW must be released by MyEndReadProfileSectionW
// hINIFile can be NULL, then this function returns NULL
PVOID __stdcall MyGetProfileSectionW(HINIFILE hINIFile, LPCWSTR lpszSection);
// if lpSectionBuffer is NULL, then this function must returns lpszDefault
// if returns non-NULL, then the caller must release the buffer by using 'free()'
LPWSTR __stdcall MyGetProfileStringW(PVOID lpSectionBuffer, LPCWSTR lpszKey, LPCWSTR lpszDefault = NULL);
int __stdcall MyGetProfileIntW(PVOID lpSectionBuffer, LPCWSTR lpszKey, int nDefault = 0);
DWORD __stdcall MyGetProfileDWordW(PVOID lpSectionBuffer, LPCWSTR lpszKey, DWORD dwDefault = 0);
DWORD __stdcall MyGetProfileBinaryW(PVOID lpSectionBuffer, LPCWSTR lpszKey, LPVOID lpBuffer, DWORD dwSize);
bool __stdcall MyGetProfileBooleanW(PVOID lpSectionBuffer, LPCWSTR lpszKey, bool bDefault = false);
LPWSTR __stdcall MyGetNextProfileStringW(PVOID lpSectionBuffer, int nIndex, LPWSTR* lplpszValue);
// if lpSectionBuffer is NULL, then this function does nothing
void __stdcall MyEndReadProfileSectionW(PVOID lpSectionBuffer);
// if hINIFile is NULL, then this function does nothing
void __stdcall MyCloseINIFile(HINIFILE hINIFile);

// INI ファイルになるような書き込みを行う
// MyWriteINISectionW -> MyWriteINIValueW -> MyWriteINIValueW -> ... (-> MyWriteCRLFW -> MyWriteINISectionW -> ...)
void __stdcall MyWriteStringW(HANDLE hFile, LPCWSTR lpszString);
void __stdcall MyWriteCRLFW(HANDLE hFile);
void __stdcall MyWriteStringLineW(HANDLE hFile, LPCWSTR lpszString);
void __stdcall MyWriteINISectionW(HANDLE hFile, LPCWSTR lpszSectionName);
void __stdcall MyWriteINIValueW(HANDLE hFile, LPCWSTR lpszKey, LPCWSTR lpszValue);
void __stdcall MyWriteINIValueW(HANDLE hFile, LPCWSTR lpszKey, int nValue);
// cType: CUDF_XXXX
void __stdcall MyWriteINIValueW(HANDLE hFile, LPCWSTR lpszKey, DWORD dwValue, BYTE cType);
void __stdcall MyWriteINIValueW(HANDLE hFile, LPCWSTR lpszKey, LPCVOID lpBuffer, DWORD dwSize);

#endif //__INIFILE_H__
