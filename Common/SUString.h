/*
 Copyright (C) 2010 Kuri-Applications

 SUString.h - declarations of _SecureStringW
 */

#pragma once

#include "UString.h"

class _SecureStringW
{
public:
	_SecureStringW();
	_SecureStringW(LPCWSTR lpszString);
	_SecureStringW(const CMyStringW& string);
	_SecureStringW(const _SecureStringW& string);
	~_SecureStringW();

	void Empty();
	bool IsEmpty() const { return !m_lpszBuffer; }
	void GetString(CMyStringW& string) const;
	void AppendToString(CMyStringW& string) const;
	bool GetStringFromWindowText(HWND hWnd);
	void SetStringToWindowText(HWND hWnd) const;

	void SetString(LPCWSTR lpszString);
	void SetString(const CMyStringW& string);
	void SetString(const _SecureStringW& string);
	void AppendChar(WCHAR wch);
	void AppendString(LPCWSTR lpszString);
	void AppendString(const CMyStringW& string);
	void AppendString(const _SecureStringW& string);

	_SecureStringW& operator = (LPCWSTR lpszString);
	_SecureStringW& operator = (const CMyStringW& string);
	_SecureStringW& operator = (const _SecureStringW& string);
	_SecureStringW& operator += (WCHAR wch);
	_SecureStringW& operator += (LPCWSTR lpszString);
	_SecureStringW& operator += (const CMyStringW& string);
	_SecureStringW& operator += (const _SecureStringW& string);

	static void __stdcall SecureEmptyString(CMyStringW& string);
	static void __stdcall SecureEmptyBuffer(LPVOID lpBuffer, size_t nSize);

private:
	LPWSTR m_lpszBuffer;
	size_t m_dwLengthX;
	BYTE m_nSeed;
};
