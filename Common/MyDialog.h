/*
 Copyright (C) 2010 Kuri-Applications

 MyDialog.h - declarations of CMyDialog and some dialog functions
 */

#pragma once

#include "MyWindow.h"

extern "C" INT_PTR WINAPI ExDialogBoxParamA(HINSTANCE hInstance, LPCSTR lpszTemplate, HWND hWndOwner, DLGPROC pDialogProc, LPARAM lParam);
extern "C" INT_PTR WINAPI ExDialogBoxParamW(HINSTANCE hInstance, LPCWSTR lpszTemplate, HWND hWndOwner, DLGPROC pDialogProc, LPARAM lParam);
extern "C" HWND WINAPI ExCreateDialogParamA(HINSTANCE hInstance, LPCSTR lpszTemplate, HWND hWndOwner, DLGPROC pDialogProc, LPARAM lParam);
extern "C" HWND WINAPI ExCreateDialogParamW(HINSTANCE hInstance, LPCWSTR lpszTemplate, HWND hWndOwner, DLGPROC pDialogProc, LPARAM lParam);
#ifdef _UNICODE
#define ExDialogBoxParam      ExDialogBoxParamW
#define ExCreateDialogParam   ExCreateDialogParamW
#else
#define ExDialogBoxParam      ExDialogBoxParamA
#define ExCreateDialogParam   ExCreateDialogParamA
#endif

class CMyDialog : public CMyWindow
{
public:
	CMyDialog(UINT uID);

	INT_PTR ModalDialogA(HWND hWndParent);
	INT_PTR ModalDialogW(HWND hWndParent);
	HWND CreateA(HWND hWndParent);
	HWND CreateW(HWND hWndParent);

	virtual bool PreTranslateMessage(LPMSG lpMsg);

	virtual bool OnInitDialog(HWND hWndFocus) { return true; }

	HFONT m_hFontReplace;

protected:
	UINT m_uID;
	bool m_bModal;

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnDefaultButton(WPARAM wParam, LPARAM lParam);
};
