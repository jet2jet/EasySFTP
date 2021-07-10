/*
 Copyright (C) 2010 Kuri-Applications

 MyDialog.cpp - implementations of CMyDialog
 */

#include "stdafx.h"
#include "MyDialog.h"

#include "MyFunc.h"
#include "unicode.h"
#include "AppClass.h"

#pragma pack(push, 1)

typedef struct _DLGTEMPLATEEX
{
	WORD dlgVer;
	WORD signature;
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	WORD cDlgItems;
	short x;
	short y;
	short cx;
	short cy;
} DLGTEMPLATEEX, FAR* LPDLGTEMPLATEEX;
typedef const DLGTEMPLATEEX FAR* LPCDLGTEMPLATEEX;

typedef struct _DLGITEMTEMPLATEEX
{
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	short x;
	short y;
	short cx;
	short cy;
	DWORD id;
} DLGITEMTEMPLATEEX, FAR* LPDLGITEMTEMPLATEEX;
typedef const DLGITEMTEMPLATEEX FAR* LPCDLGITEMTEMPLATEEX;

#pragma pack(pop)

typedef const BYTE FAR* LPCBYTE;
typedef const WORD FAR* LPCWORD;

void __stdcall _StartHook(CMyWindow* pWnd);
bool __stdcall _EndHook();

/////////////////////////////////////////////////////////////////////////////

extern "C" inline bool WINAPI _IsTemplateDialogEx(LPCDLGTEMPLATE pTemplate)
{
	return ((LPCDLGTEMPLATEEX) pTemplate)->signature == 0xFFFF;
}

extern "C" inline bool WINAPI _HasTemplateFont(LPCDLGTEMPLATE pTemplate)
{
	return ((_IsTemplateDialogEx(pTemplate) ?
		((LPCDLGTEMPLATEEX) pTemplate)->style : pTemplate->style) & DS_SETFONT) != 0;
}

extern "C" inline int _GetFontAttrSize(bool bDialogEx)
{
	return (int) sizeof(WORD) * (bDialogEx ? 3 : 1);
}

extern "C" inline LPCWSTR WINAPI _SkipString(LPCWSTR p)
{
	while (*p++);
	return p;
}
//
//extern "C" inline LPWSTR WINAPI _SkipString(LPWSTR p)
//{
//	while (*p++);
//	return p;
//}

extern "C" LPCBYTE WINAPI _GetFontSizeField(LPCDLGTEMPLATE pTemplate)
{
	LPCWORD pw;

	if (_IsTemplateDialogEx(pTemplate))
		pw = (LPCWORD) (((LPCDLGTEMPLATEEX) pTemplate) + 1);
	else
		pw = (LPCWORD) (pTemplate + 1);

	if (*pw == (WORD) -1)
		pw += 2;
	else
		pw = (LPCWORD) _SkipString((LPCWSTR) pw);

	if (*pw == (WORD) -1)
		pw += 2;
	else
		pw = (LPCWORD) _SkipString((LPCWSTR) pw);

	pw = (LPCWORD) _SkipString((LPCWSTR) pw);

	return (LPCBYTE) pw;
}

extern "C" UINT WINAPI _GetDlgTemplateSize(LPCDLGTEMPLATE pTemplate)
{
	bool bDialogEx;
	LPCBYTE pb;
	WORD nCtrl, cbExtraByte;

	bDialogEx = _IsTemplateDialogEx(pTemplate);
	pb = _GetFontSizeField(pTemplate);

	if (_HasTemplateFont(pTemplate))
	{
		pb += _GetFontAttrSize(bDialogEx);
		pb += sizeof(WCHAR) * (wcslen((LPWSTR) pb) + 1);
	}

	nCtrl = bDialogEx ? (WORD) ((LPCDLGTEMPLATEEX) pTemplate)->cDlgItems :
		(WORD) pTemplate->cdit;

	while (nCtrl--)
	{
		pb = (LPCBYTE) (((DWORD_PTR) pb + 3) & ~((DWORD_PTR) 3));

		pb += (bDialogEx ? sizeof(DLGITEMTEMPLATEEX) : sizeof(DLGITEMTEMPLATE));

		if (*((LPCWORD) pb) == (WORD) -1)
			pb += 2 * sizeof(WORD);
		else
			pb = (LPCBYTE) _SkipString((LPCWSTR) pb);

		if (*((LPCWORD) pb) == (WORD) -1)
			pb += 2 * sizeof(WORD);
		else
			pb = (LPCBYTE) _SkipString((LPCWSTR) pb);

		cbExtraByte = *((LPCWORD) pb);
		if (cbExtraByte != 0 && !bDialogEx)
			cbExtraByte -= 2;
		pb += sizeof(WORD) + cbExtraByte;
	}

	return (UINT) (pb - (LPCBYTE) pTemplate);
}

bool WINAPI _GetDialogFont(LPCDLGTEMPLATE pTemplate, LPTSTR* lplpszFace, WORD* lpwFontSize)
{
	if (!_HasTemplateFont(pTemplate))
		return false;

	LPCBYTE pb = _GetFontSizeField(pTemplate);
	*lpwFontSize = *(LPCWORD) pb;
	pb += _GetFontAttrSize(_IsTemplateDialogEx(pTemplate));
	*lplpszFace = MyUnicodeToString((LPCWSTR) pb);

	return *lplpszFace != NULL;
}

extern "C" bool WINAPI _SetFontToTemplate(LPDLGTEMPLATE pTemplate, LPCTSTR lpFaceName, WORD nFontSize)
{
	DWORD dwTemplateSize;
	bool bHasFont;
	bool bDialogEx;
	int cbFontAttr;
	LPWSTR lpwFaceUnicode;
	int cbOld, cbNew;
	LPBYTE pb, pbNew;
	LPBYTE pOldControls, pNewControls;

	dwTemplateSize = _GetDlgTemplateSize(pTemplate);
	bHasFont = _HasTemplateFont(pTemplate);
	bDialogEx = _IsTemplateDialogEx(pTemplate);
	cbFontAttr = _GetFontAttrSize(bDialogEx);

	lpwFaceUnicode = MyStringToUnicode(lpFaceName);
	if (!lpwFaceUnicode)
		return false;

	if (bDialogEx)
		((LPDLGTEMPLATEEX) pTemplate)->style |= DS_SETFONT;
	else
		pTemplate->style |= DS_SETFONT;

	cbNew = cbFontAttr + (int) ((wcslen(lpwFaceUnicode) + 1) * sizeof(WCHAR));
	pbNew = (LPBYTE) lpwFaceUnicode;

	pb = (LPBYTE) _GetFontSizeField(pTemplate);
	cbOld = (int) (bHasFont ? cbFontAttr + 2 * (wcslen((LPWSTR) (pb + cbFontAttr)) + 1) : 0);

	pOldControls = (LPBYTE) (((DWORD_PTR) pb + cbOld + 3) & ~((DWORD_PTR) 3));
	pNewControls = (LPBYTE) (((DWORD_PTR) pb + cbNew + 3) & ~((DWORD_PTR) 3));

	WORD nCtrl = bDialogEx ? (WORD) ((LPDLGTEMPLATEEX) pTemplate)->cDlgItems : (WORD) pTemplate->cdit;

	if (cbNew != cbOld && nCtrl > 0)
		memmove(pNewControls, pOldControls, (size_t)(dwTemplateSize - (pOldControls - (LPBYTE) pTemplate)));

	*((LPWORD) pb) = nFontSize;
	memmove(pb + cbFontAttr, pbNew, cbNew - cbFontAttr);

	free(lpwFaceUnicode);

	return TRUE;
}

extern "C" bool WINAPI _SetDefaultFont(LPDLGTEMPLATE pTemplate, WORD wSize)
{
	LPCTSTR pszFace;
	LOGFONT lf;
	HFONT hFont;
	NONCLIENTMETRICS ncm;
	HDC hDC;

	ncm.cbSize = NONCLIENTMETRICS_SIZE_V1;
	if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, NONCLIENTMETRICS_SIZE_V1, &ncm, FALSE))
		memcpy(&lf, &ncm.lfMessageFont, sizeof(lf));
	else
	{
		hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
		if (hFont == NULL)
			hFont = (HFONT) GetStockObject(SYSTEM_FONT);
		if (hFont == NULL || GetObject(hFont, sizeof(LOGFONT), &lf) == 0)
			return false;
	}

	if (wSize == 0)
	{
		hDC = GetDC(NULL);
		if (hDC == NULL)
			return false;

		pszFace = lf.lfFaceName;
		if (lf.lfHeight < 0)
			lf.lfHeight = -lf.lfHeight;
		wSize = (WORD) MulDiv(lf.lfHeight, 72, GetDeviceCaps(hDC, LOGPIXELSY));
		ReleaseDC(NULL, hDC);
	}

	return _SetFontToTemplate(pTemplate, lf.lfFaceName, wSize);
}

extern "C" bool WINAPI _SetFontHandle(LPDLGTEMPLATE pTemplate, HFONT hFont)
{
	WORD wSize;
	LOGFONT lf;
	if (::GetObject(hFont, sizeof(LOGFONT), &lf) == 0)
		return false;

	HDC hDC = ::GetDC(NULL);
	if (lf.lfHeight < 0)
		lf.lfHeight = -lf.lfHeight;
	wSize = (WORD) MulDiv(lf.lfHeight, 72, GetDeviceCaps(hDC, LOGPIXELSY));
	::ReleaseDC(NULL, hDC);

	return _SetFontToTemplate(pTemplate, lf.lfFaceName, wSize);
}

/////////////////////////////////////////////////////////////////////////////

extern "C" HGLOBAL WINAPI _DuplicateDialogTemplate(HINSTANCE hInst, LPCTSTR lpszTemplate)
{
	HRSRC hr;
	HGLOBAL hTemplate;
	LPVOID lpv, lpv2;
	UINT uSize;
	HGLOBAL hGlobal;
	LPDLGTEMPLATE pt;
	LPTSTR lpFont;
	WORD wSize;

	hr = FindResource(hInst, lpszTemplate, RT_DIALOG);
	if (hr == NULL)
		return NULL;

	hTemplate = LoadResource(hInst, hr);
	lpv = LockResource(hTemplate);
	uSize = _GetDlgTemplateSize((LPDLGTEMPLATE) lpv);

	hGlobal = GlobalAlloc(GPTR, uSize + LF_FACESIZE * 2);
	if (hGlobal == NULL)
	{
		UnlockResource(hTemplate);
		FreeResource(hTemplate);
		return NULL;
	}

	lpv2 = GlobalLock(hGlobal);
	memcpy(lpv2, lpv, uSize);

	pt = (LPDLGTEMPLATE) lpv2;

	if (!_GetDialogFont(pt, &lpFont, &wSize))
	{
		GlobalUnlock(hGlobal);
		GlobalFree(hGlobal);
		UnlockResource(hTemplate);
		FreeResource(hTemplate);
		return NULL;
	}

	if (_tcsicmp(lpFont, _T("MS Shell Dlg")) == 0)
	{
		if (wSize == 8)
			wSize = 0;
		_SetDefaultFont(pt, wSize);
	}
	free(lpFont);

	GlobalUnlock(hGlobal);
	UnlockResource(hTemplate);
	FreeResource(hTemplate);
	return hGlobal;
}

extern "C" HGLOBAL WINAPI _DuplicateDialogTemplateA(HINSTANCE hInst, LPCSTR lpszTemplate)
{
#ifdef _UNICODE
	if (HIWORD(lpszTemplate))
	{
		CMyStringW str(lpszTemplate);
		return _DuplicateDialogTemplate(hInst, str);
	}
#endif
	return _DuplicateDialogTemplate(hInst, (LPCTSTR) lpszTemplate);
}

extern "C" HGLOBAL WINAPI _DuplicateDialogTemplateW(HINSTANCE hInst, LPCWSTR lpszTemplate)
{
#ifndef _UNICODE
	if (HIWORD(lpszTemplate))
	{
		CMyStringW str(lpszTemplate);
		return _DuplicateDialogTemplate(hInst, str);
	}
#endif
	return _DuplicateDialogTemplate(hInst, (LPCTSTR) lpszTemplate);
}

extern "C" INT_PTR CALLBACK _DefDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR) TRUE;
			}
		}
		break;
	}
	return (INT_PTR) FALSE;
}

extern "C" INT_PTR WINAPI ExDialogBoxParamA(HINSTANCE hInstance, LPCSTR lpszTemplate, HWND hWndOwner, DLGPROC pDialogProc, LPARAM lParam)
{
	HGLOBAL hGlobal;
	INT_PTR nRet;

	hGlobal = _DuplicateDialogTemplateA(hInstance, lpszTemplate);
	if (hGlobal == NULL)
		return (INT_PTR) -1;
	if (pDialogProc == NULL)
		pDialogProc = (DLGPROC) _DefDialogProc;

	_StartHook(NULL);
	nRet = DialogBoxIndirectParamA(hInstance, (LPCDLGTEMPLATE) hGlobal, hWndOwner, pDialogProc, lParam);
	_EndHook();

	GlobalFree(hGlobal);
	return nRet;
}

extern "C" INT_PTR WINAPI ExDialogBoxParamW(HINSTANCE hInstance, LPCWSTR lpszTemplate, HWND hWndOwner, DLGPROC pDialogProc, LPARAM lParam)
{
	HGLOBAL hGlobal;
	INT_PTR nRet;

	hGlobal = _DuplicateDialogTemplateW(hInstance, lpszTemplate);
	if (hGlobal == NULL)
		return (INT_PTR) -1;
	if (pDialogProc == NULL)
		pDialogProc = (DLGPROC) _DefDialogProc;

	_StartHook(NULL);
	nRet = DialogBoxIndirectParamW(hInstance, (LPCDLGTEMPLATE) hGlobal, hWndOwner, pDialogProc, lParam);
	if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
		nRet = DialogBoxIndirectParamA(hInstance, (LPCDLGTEMPLATE) hGlobal, hWndOwner, pDialogProc, lParam);
	_EndHook();

	GlobalFree(hGlobal);
	return nRet;
}

extern "C" HWND WINAPI ExCreateDialogParamA(HINSTANCE hInstance, LPCSTR lpszTemplate, HWND hWndOwner, DLGPROC pDialogProc, LPARAM lParam)
{
	HGLOBAL hGlobal;
	HWND hRet;

	hGlobal = _DuplicateDialogTemplateA(hInstance, lpszTemplate);
	if (hGlobal == NULL)
		return NULL;
	if (pDialogProc == NULL)
		pDialogProc = (DLGPROC) _DefDialogProc;

	_StartHook(NULL);
	hRet = CreateDialogIndirectParamA(hInstance, (LPCDLGTEMPLATE) hGlobal, hWndOwner, pDialogProc, lParam);
	_EndHook();

	GlobalFree(hGlobal);
	return hRet;
}

extern "C" HWND WINAPI ExCreateDialogParamW(HINSTANCE hInstance, LPCWSTR lpszTemplate, HWND hWndOwner, DLGPROC pDialogProc, LPARAM lParam)
{
	HGLOBAL hGlobal;
	HWND hRet;

	hGlobal = _DuplicateDialogTemplateW(hInstance, lpszTemplate);
	if (hGlobal == NULL)
		return NULL;
	if (pDialogProc == NULL)
		pDialogProc = (DLGPROC) _DefDialogProc;

	_StartHook(NULL);
	hRet = CreateDialogIndirectParamW(hInstance, (LPCDLGTEMPLATE) hGlobal, hWndOwner, pDialogProc, lParam);
	if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
		hRet = CreateDialogIndirectParamA(hInstance, (LPCDLGTEMPLATE) hGlobal, hWndOwner, pDialogProc, lParam);
	_EndHook();

	GlobalFree(hGlobal);
	return hRet;
}

////////////////////////////////////////////////////////////////////////////////

extern "C" INT_PTR CALLBACK _WrapperDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CMyDialog* pDlg = (CMyDialog*) CMyWindow::FromHandle(hDlg);
	if (!pDlg)
		return 0;
	if (message == WM_INITDIALOG)
		return (INT_PTR) (pDlg->OnInitDialog((HWND) wParam) ? TRUE : FALSE);
	return 0;
}

CMyDialog::CMyDialog(UINT uID)
	: m_uID(uID)
	, m_bModal(false)
	, m_hFontReplace(NULL)
{
}

INT_PTR CMyDialog::ModalDialogA(HWND hWndParent)
{
	HGLOBAL hGlobal;
	INT_PTR nRet;
	HINSTANCE hInst = MyGetCurrentInstance();

	hGlobal = _DuplicateDialogTemplate(hInst, MAKEINTRESOURCE(m_uID));
	if (hGlobal == NULL)
		return (INT_PTR) -1;

	if (m_hFontReplace != NULL)
	{
		LPVOID lpv = ::GlobalLock(hGlobal);
		::_SetFontHandle((LPDLGTEMPLATE) lpv, m_hFontReplace);
		::GlobalUnlock(hGlobal);
	}

	m_bModal = true;
	_StartHook(this);
	m_bUniWindow = false;
	nRet = ::DialogBoxIndirectParamA(hInst, (LPCDLGTEMPLATE) hGlobal,
		hWndParent, (DLGPROC) _WrapperDialogProc, 0);
	_EndHook();

	GlobalFree(hGlobal);
	return nRet;
}

INT_PTR CMyDialog::ModalDialogW(HWND hWndParent)
{
	HGLOBAL hGlobal;
	INT_PTR nRet;
	HINSTANCE hInst = MyGetCurrentInstance();

	hGlobal = _DuplicateDialogTemplate(hInst, MAKEINTRESOURCE(m_uID));
	if (hGlobal == NULL)
		return (INT_PTR) -1;

	if (m_hFontReplace != NULL)
	{
		LPVOID lpv = ::GlobalLock(hGlobal);
		::_SetFontHandle((LPDLGTEMPLATE) lpv, m_hFontReplace);
		::GlobalUnlock(hGlobal);
	}

	m_bModal = true;
	_StartHook(this);
	m_bUniWindow = true;
	nRet = ::DialogBoxIndirectParamW(hInst, (LPCDLGTEMPLATE) hGlobal,
		hWndParent, (DLGPROC) _WrapperDialogProc, 0);
	if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
	{
		m_bUniWindow = false;
		nRet = ::DialogBoxIndirectParamA(hInst, (LPCDLGTEMPLATE) hGlobal,
			hWndParent, (DLGPROC) _WrapperDialogProc, 0);
	}
	_EndHook();

	GlobalFree(hGlobal);
	return nRet;
}

HWND CMyDialog::CreateA(HWND hWndParent)
{
	HGLOBAL hGlobal;
	HWND hRet;
	HINSTANCE hInst = MyGetCurrentInstance();

	hGlobal = _DuplicateDialogTemplate(hInst, MAKEINTRESOURCE(m_uID));
	if (hGlobal == NULL)
		return NULL;

	if (m_hFontReplace != NULL)
	{
		LPVOID lpv = ::GlobalLock(hGlobal);
		::_SetFontHandle((LPDLGTEMPLATE) lpv, m_hFontReplace);
		::GlobalUnlock(hGlobal);
	}

	m_bModal = false;
	_StartHook(this);
	m_bUniWindow = false;
	hRet = ::CreateDialogIndirectParamA(hInst, (LPCDLGTEMPLATE) hGlobal,
		hWndParent, (DLGPROC) _WrapperDialogProc, 0);
	_EndHook();

	GlobalFree(hGlobal);
	return hRet;
}

HWND CMyDialog::CreateW(HWND hWndParent)
{
	HGLOBAL hGlobal;
	HWND hRet;
	HINSTANCE hInst = MyGetCurrentInstance();

	hGlobal = _DuplicateDialogTemplate(hInst, MAKEINTRESOURCE(m_uID));
	if (hGlobal == NULL)
		return NULL;

	if (m_hFontReplace != NULL)
	{
		LPVOID lpv = ::GlobalLock(hGlobal);
		::_SetFontHandle((LPDLGTEMPLATE) lpv, m_hFontReplace);
		::GlobalUnlock(hGlobal);
	}

	m_bModal = false;
	_StartHook(this);
	m_bUniWindow = true;
	hRet = CreateDialogIndirectParamW(hInst, (LPCDLGTEMPLATE) hGlobal,
		hWndParent, _WrapperDialogProc, 0);
	if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
	{
		m_bUniWindow = false;
		hRet = ::CreateDialogIndirectParamA(hInst, (LPCDLGTEMPLATE) hGlobal,
			hWndParent, (DLGPROC) _WrapperDialogProc, 0);
	}
	_EndHook();

	GlobalFree(hGlobal);
	return hRet;
}

bool CMyDialog::PreTranslateMessage(LPMSG lpMsg)
{
	if (m_bUniWindow)
		return (::IsDialogMessageW(m_hWnd, lpMsg) != 0);
	else
		return (::IsDialogMessageA(m_hWnd, lpMsg) != 0);
}

LRESULT CMyDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE_COMMAND(IDOK, OnDefaultButton);
	HANDLE_COMMAND(IDCANCEL, OnDefaultButton);
	return CMyWindow::WindowProc(message, wParam, lParam);
}

LRESULT CMyDialog::OnDefaultButton(WPARAM wParam, LPARAM lParam)
{
	if (m_bModal)
		::EndDialog(m_hWnd, (INT_PTR) LOWORD(wParam));
	else
		DestroyWindow();
	return 0;
}
