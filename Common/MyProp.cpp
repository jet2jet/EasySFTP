/*
 Copyright (C) 2010 Kuri-Applications

 MyProp.cpp - implementations of CMyPropertySheet and CMyPropertyPage
 */

#include "StdAfx.h"
#include "MyProp.h"

#include "AppClass.h"
#include "UString.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void __stdcall _StartHook(CMyWindow* pWnd);
bool __stdcall _EndHook();

extern "C" HGLOBAL WINAPI _DuplicateDialogTemplate(HINSTANCE hInst, LPCTSTR lpszTemplate);
extern "C" bool WINAPI _SetFontHandle(LPDLGTEMPLATE pTemplate, HFONT hFont);
extern "C" INT_PTR CALLBACK _WrapperDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

////////////////////////////////////////////////////////////////////////////////

#ifndef PROPSHEETHEADER_V1_SIZE
#define PROPSHEETHEADERA_V1_SIZE   CDSIZEOF_STRUCT(PROPSHEETHEADERA, pfnCallback)
#define PROPSHEETHEADERW_V1_SIZE   CDSIZEOF_STRUCT(PROPSHEETHEADERW, pfnCallback)
#define PROPSHEETHEADER_V1_SIZE    CDSIZEOF_STRUCT(PROPSHEETHEADER, pfnCallback)
#endif

#ifndef PROPSHEETPAGE_V1_SIZE
#define PROPSHEETPAGEA_V1_SIZE     CDSIZEOF_STRUCT(PROPSHEETPAGEA, pcRefParent)
#define PROPSHEETPAGEW_V1_SIZE     CDSIZEOF_STRUCT(PROPSHEETPAGEW, pcRefParent)
#define PROPSHEETPAGE_V1_SIZE      CDSIZEOF_STRUCT(PROPSHEETPAGE, pcRefParent)
#endif

LRESULT CALLBACK MyPropPageCallbackA(HWND hWnd, UINT message, PROPSHEETPAGEA* pPSP)
{
	switch (message)
	{
		case PSPCB_CREATE:
			_StartHook((CMyPropertyPage*) pPSP->lParam);
			return TRUE;
		case PSPCB_RELEASE:
			_EndHook();
			break;
	}
	return 0;
}

LRESULT CALLBACK MyPropPageCallbackW(HWND hWnd, UINT message, PROPSHEETPAGEW* pPSP)
{
	switch (message)
	{
		case PSPCB_CREATE:
			_StartHook((CMyPropertyPage*) pPSP->lParam);
			return TRUE;
		case PSPCB_RELEASE:
			_EndHook();
			break;
	}
	return 0;
}

CMyPropertySheet::CMyPropertySheet()
{
	m_dwPSHFlags = 0;
}

CMyPropertySheet::~CMyPropertySheet()
{
	int i;
	for (i = 0; i < m_arrPages.GetCount(); i++)
		delete m_arrPages.GetItem(i);
	m_arrPages.RemoveAll();
}

void CMyPropertySheet::AddPage(CMyPropertyPage* pPage)
{
	m_arrPages.Add(pPage);
}

HWND CMyPropertySheet::Create(bool bModal, LPCWSTR lpszTitle, HWND hWndParent)
{
	PROPSHEETHEADERW psh;
	HPROPSHEETPAGE* lphpsp;
	CMyPropertyPage* pPage;
	HWND hWnd;
	int n;

	memset(&psh, 0, sizeof(psh));
	psh.dwSize = PROPSHEETHEADERW_V1_SIZE;

	if (bModal)
		psh.dwFlags = 0;
	else
		psh.dwFlags = PSH_MODELESS;
	psh.dwFlags |= m_dwPSHFlags;
	psh.dwFlags &= ~PSH_PROPSHEETPAGE;
	psh.nPages = m_arrPages.GetCount();
	psh.phpage = (lphpsp = (HPROPSHEETPAGE*) malloc(sizeof(HPROPSHEETPAGE) * psh.nPages));

	for (n = 0; n < (int) psh.nPages; n++)
	{
		pPage = m_arrPages.GetItem(n);
		lphpsp[n] = pPage->CreatePropPageHandle();
		if (!lphpsp[n])
		{
			while (n--)
			{
				pPage = m_arrPages.GetItem(n);
				::DestroyPropertySheetPage(lphpsp[n]);
				pPage->m_hPSP = NULL;
			}
			free(lphpsp);
			return bModal ? (HWND) -1 : NULL;
		}
		pPage->m_hPSP = lphpsp[n];
	}
	psh.pszCaption = lpszTitle;
	psh.hwndParent = hWndParent;

	_StartHook(this);
	hWnd = (HWND) ::PropertySheetW(&psh);
	if ((bModal && hWnd == (HWND) -1) || (!bModal && !hWnd))
	{
		PROPSHEETHEADERA pshA;
		CMyStringW str;
		if (lpszTitle)
			str = lpszTitle;
		memset(&pshA, 0, sizeof(pshA));
		pshA.dwSize = PROPSHEETHEADERA_V1_SIZE;

		pshA.dwFlags = psh.dwFlags;
		pshA.nPages = psh.nPages;
		pshA.phpage = psh.phpage;
		pshA.pszCaption = str;
		pshA.hwndParent = psh.hwndParent;
		hWnd = (HWND) ::PropertySheetA(&pshA);
	}

	free(lphpsp);
	if (!_EndHook())
		return bModal ? (HWND) -1 : NULL;

	return hWnd;
}

////////////////////////////////////////////////////////////////////////////////

CMyPropertyPage::CMyPropertyPage(UINT uID)
	: CMyDialog(uID)
{
	m_hPSP = NULL;
	m_hGlobal = NULL;
	m_dwPSPFlags = 0;
}

CMyPropertyPage::~CMyPropertyPage()
{
	if (m_hGlobal)
		GlobalFree(m_hGlobal);
}

LPCWSTR CMyPropertyPage::GetPageTitle()
{
	return NULL;
}

HGLOBAL CMyPropertyPage::GetDialogResource()
{
	HINSTANCE hInst = MyGetCurrentInstance();

	if (m_hGlobal)
		GlobalFree(m_hGlobal);

	m_hGlobal = ::_DuplicateDialogTemplate(hInst, MAKEINTRESOURCE(m_uID));

	if (m_hFontReplace != NULL)
	{
		LPVOID lpv = ::GlobalLock(m_hGlobal);
		::_SetFontHandle((LPDLGTEMPLATE) lpv, m_hFontReplace);
		::GlobalUnlock(m_hGlobal);
	}
	return m_hGlobal;
}

HPROPSHEETPAGE CMyPropertyPage::CreatePropPageHandle()
{
	HPROPSHEETPAGE hPSP;
	PROPSHEETPAGEW psp;

	memset(&psp, 0, sizeof(psp));
	psp.dwSize = PROPSHEETPAGEW_V1_SIZE;
	psp.dwFlags = PSP_DLGINDIRECT | PSP_USECALLBACK | m_dwPSPFlags;
	psp.pResource = (LPCDLGTEMPLATE) GetDialogResource();
	psp.pfnDlgProc = (DLGPROC) _WrapperDialogProc;
	psp.pfnCallback = (LPFNPSPCALLBACKW) MyPropPageCallbackW;
	psp.lParam = (LPARAM) this;
	if (psp.pszTitle = GetPageTitle())
		psp.dwFlags |= PSP_USETITLE;
	if (!(hPSP = ::CreatePropertySheetPageW(&psp)))
	{
		PROPSHEETPAGEA pspA;
		memset(&pspA, 0, sizeof(pspA));
		pspA.dwSize = PROPSHEETPAGEA_V1_SIZE;
		pspA.dwFlags = psp.dwFlags;
		pspA.pResource = psp.pResource;
		pspA.pfnDlgProc = psp.pfnDlgProc;
		pspA.pfnCallback = (LPFNPSPCALLBACKA) MyPropPageCallbackA;
		pspA.lParam = psp.lParam;
		CMyStringW str;
		if (psp.pszTitle)
		{
			str = psp.pszTitle;
			pspA.pszTitle = str;
		}
		hPSP = ::CreatePropertySheetPageA(&pspA);
	}
	return hPSP;
}

void CMyPropertyPage::SetModifiedFlag(bool bModified)
{
	if (!m_hWnd)
		return;
	HWND hWnd = ::GetParent(m_hWnd);
	::SendMessage(hWnd, bModified ? PSM_CHANGED : PSM_UNCHANGED, (WPARAM) m_hWnd, 0);
}
