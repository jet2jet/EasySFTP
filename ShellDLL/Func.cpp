/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 Func.cpp - implementations of some useful functions
 */

#include "stdafx.h"
#include "ShellDLL.h"
#include "Func.h"

///////////////////////////////////////////////////////////////////////////////

extern "C" int strcmplen(const char* string1, size_t str1len, const char* string2, size_t str2len)
{
	while (true)
	{
		if (!str1len)
			return str2len ? -1 : 0;
		else if (!str2len)
			return 1;
		register int r = (int) *string2 - (int) *string1;
		if (r)
			return r > 0 ? 1 : -1;
		string1++;
		string2++;
		str1len--;
		str2len--;
	}
}

extern "C" void __stdcall ConvertToLowerW(LPWSTR lpszString)
{
	while (*lpszString)
	{
		*lpszString = towlower(*lpszString);
		lpszString++;
	}
}

extern "C" void __stdcall ConvertToLowerLenW(LPWSTR lpszString, SIZE_T nLen)
{
	while (nLen--)
	{
		*lpszString = towlower(*lpszString);
		lpszString++;
	}
}

///////////////////////////////////////////////////////////////////////////////

void __stdcall _StartHook(CMyWindow* pWnd);
bool __stdcall _EndHook();

extern "C" int __stdcall MyMessageBoxW(HWND hWnd, LPCWSTR lpszText, LPCWSTR lpszCaption, UINT uType)
{
	CMyStringW strText, strCaption;
	if (HIWORD(lpszText) == 0 && lpszText != NULL)
	{
		strText.LoadString((UINT) LOWORD(lpszText));
		lpszText = strText;
	}
	if (lpszCaption == NULL)
		lpszCaption = (LPCWSTR) IDS_APP_TITLE;
	if (HIWORD(lpszCaption) == 0 && lpszCaption != NULL)
	{
		strCaption.LoadString((UINT) LOWORD(lpszCaption));
		lpszCaption = strCaption;
	}
	//if (!hWnd && theApp.m_pMainWnd)
	//	hWnd = theApp.m_pMainWnd->m_hWnd;
	_StartHook(NULL);
	int nRet = ::MessageBoxW(hWnd, lpszText, lpszCaption, uType);
	_EndHook();
	return nRet;
}

extern "C" bool __stdcall MyShellOpenW(HWND hWnd, LPCWSTR lpszFileName)
{
	HINSTANCE hInst;
	if (!MyIsExistFileW(lpszFileName))
		return false;
	hInst = ::ShellExecuteW(hWnd, NULL, lpszFileName, NULL, NULL, SW_SHOWNORMAL);
	if (PtrToInt(hInst) == 0)
	{
		CMyStringW str(lpszFileName);
		hInst = ::ShellExecuteA(hWnd, NULL, str, NULL, NULL, SW_SHOWNORMAL);
	}
	return PtrToInt(hInst) > 32;
}

extern "C" bool __stdcall MyShellExecuteWithParameterW(HWND hWnd, LPCWSTR lpszExecuteName, LPCWSTR lpszParameters)
{
	CMyStringW strFile;
	if (!::MySearchPathStringW(lpszExecuteName, strFile))
		return false;
	HINSTANCE hInst;
	hInst = ::ShellExecuteW(hWnd, NULL, strFile, lpszParameters, NULL, SW_SHOWNORMAL);
	if (PtrToInt(hInst) == 0)
	{
		CMyStringW str2(lpszParameters);
		hInst = ::ShellExecuteA(hWnd, NULL, strFile, str2, NULL, SW_SHOWNORMAL);
	}
	return PtrToInt(hInst) > 32;
}
