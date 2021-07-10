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

extern "C" int __stdcall MyMessageBoxW(HWND hWnd, LPCWSTR lpszText, LPCWSTR lpszCaption, UINT uType);

extern "C" bool __stdcall MyShellOpenW(HWND hWnd, LPCWSTR lpszFileName);

#ifdef _DEBUG
extern "C" void __stdcall OutputDebugFTPMessage(int code, LPCWSTR msg);
#else
extern "C" inline void __stdcall OutputDebugFTPMessage(int code, LPCWSTR msg) { }
#endif
