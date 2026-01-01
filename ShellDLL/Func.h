/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 Func.cpp - declarations of some useful functions
 */

#pragma once

#include "UString.h"

extern "C" int strcmplen(const char* string1, size_t str1len, const char* string2, size_t str2len);
extern "C" inline int strcmplen1(const char* string1, size_t str1len, const char* string2)
	{ return strcmplen(string1, str1len, string2, strlen(string2)); }
extern "C" inline int strcmplen2(const char* string1, const char* string2, size_t str2len)
	{ return strcmplen(string1, strlen(string1), string2, str2len); }
extern "C" void __stdcall ConvertToLowerW(LPWSTR lpszString);
extern "C" void __stdcall ConvertToLowerLenW(LPWSTR lpszString, SIZE_T nLen);

extern "C" int __stdcall MyMessageBoxW(HWND hWnd, LPCWSTR lpszText, LPCWSTR lpszCaption, UINT uType);

extern "C" bool __stdcall MyShellOpenW(HWND hWnd, LPCWSTR lpszFileName);
extern "C" bool __stdcall MyShellExecuteWithParameterW(HWND hWnd, LPCWSTR lpszExecuteName, LPCWSTR lpszParameters);

extern "C++" void MakeRandomString(CMyStringW& rstrOutput);
