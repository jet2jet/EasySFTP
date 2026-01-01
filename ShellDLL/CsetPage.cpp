/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 CsetPage.cpp - implementations of CHostCharsetPage
 */

#include "StdAfx.h"
#include "ShellDLL.h"
#include "CsetPage.h"

CHostCharsetPage::CHostCharsetPage(CEasySFTPHostSetting* pSettings, bool* pbResult)
	: CMyPropertyPage(IDD)
	, m_pSettings(pSettings)
	, m_pbResult(pbResult)
{
	pSettings->AddRef();
}

CHostCharsetPage::~CHostCharsetPage()
{
	m_pSettings->Release();
}

LRESULT CHostCharsetPage::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE_NOTIFY_CODE(PSN_APPLY, OnApply);
	return CMyPropertyPage::WindowProc(message, wParam, lParam);
}

bool CHostCharsetPage::OnInitDialog(HWND hWndFocus)
{
	int i;
	BYTE b;

	b = (m_pSettings->bTextMode & TEXTMODE_LOCAL_MASK);
	if (b == TEXTMODE_LOCAL_CR)
		i = IDC_LOCAL_CR;
	else if (b == TEXTMODE_LOCAL_LF)
		i = IDC_LOCAL_LF;
	else //if (b == TEXTMODE_LOCAL_CRLF)
		i = IDC_LOCAL_CRLF;
	::CheckRadioButton(m_hWnd, IDC_LOCAL_CRLF, IDC_LOCAL_LF, i);
	b = (m_pSettings->bTextMode & TEXTMODE_SERVER_MASK);
	if (b == TEXTMODE_SERVER_CR)
		i = IDC_SERVER_CR;
	else if (b == TEXTMODE_SERVER_LF)
		i = IDC_SERVER_LF;
	else //if (b == TEXTMODE_SERVER_CRLF)
		i = IDC_SERVER_CRLF;
	::CheckRadioButton(m_hWnd, IDC_SERVER_CRLF, IDC_SERVER_LF, i);

	i = ::AddDlgComboBoxStringW(m_hWnd, IDC_SERVER_FILE_CHARSET, L"UTF-8");
	::SendDlgItemMessage(m_hWnd, IDC_SERVER_FILE_CHARSET, CB_SETITEMDATA, (WPARAM) IntToPtr(i), (LPARAM) scsUTF8);
	if (m_pSettings->nServerCharset == scsUTF8)
		::SendDlgItemMessage(m_hWnd, IDC_SERVER_FILE_CHARSET, CB_SETCURSEL, (WPARAM) IntToPtr(i), 0);
	i = ::AddDlgComboBoxStringW(m_hWnd, IDC_SERVER_FILE_CHARSET, L"Shift-JIS");
	::SendDlgItemMessage(m_hWnd, IDC_SERVER_FILE_CHARSET, CB_SETITEMDATA, (WPARAM) IntToPtr(i), (LPARAM) scsShiftJIS);
	if (m_pSettings->nServerCharset == scsShiftJIS)
		::SendDlgItemMessage(m_hWnd, IDC_SERVER_FILE_CHARSET, CB_SETCURSEL, (WPARAM) IntToPtr(i), 0);
	i = ::AddDlgComboBoxStringW(m_hWnd, IDC_SERVER_FILE_CHARSET, L"EUC-JP");
	::SendDlgItemMessage(m_hWnd, IDC_SERVER_FILE_CHARSET, CB_SETITEMDATA, (WPARAM) IntToPtr(i), (LPARAM) scsEUC);
	if (m_pSettings->nServerCharset == scsEUC)
		::SendDlgItemMessage(m_hWnd, IDC_SERVER_FILE_CHARSET, CB_SETCURSEL, (WPARAM) IntToPtr(i), 0);

	*m_pbResult = false;

	return true;
}

LRESULT CHostCharsetPage::OnApply(WPARAM wParam, LPARAM lParam)
{
	BYTE bTextMode;
	char nCharset;
	if (::IsDlgButtonChecked(m_hWnd, IDC_LOCAL_CRLF) == BST_CHECKED)
		bTextMode = TEXTMODE_LOCAL_CRLF;
	else if (::IsDlgButtonChecked(m_hWnd, IDC_LOCAL_CR) == BST_CHECKED)
		bTextMode = TEXTMODE_LOCAL_CR;
	else if (::IsDlgButtonChecked(m_hWnd, IDC_LOCAL_LF) == BST_CHECKED)
		bTextMode = TEXTMODE_LOCAL_LF;
	if (::IsDlgButtonChecked(m_hWnd, IDC_SERVER_CRLF) == BST_CHECKED)
		bTextMode |= TEXTMODE_SERVER_CRLF;
	else if (::IsDlgButtonChecked(m_hWnd, IDC_SERVER_CR) == BST_CHECKED)
		bTextMode |= TEXTMODE_SERVER_CR;
	else if (::IsDlgButtonChecked(m_hWnd, IDC_SERVER_LF) == BST_CHECKED)
		bTextMode |= TEXTMODE_SERVER_LF;

	int i = (int) (::SendDlgItemMessage(m_hWnd, IDC_SERVER_FILE_CHARSET, CB_GETCURSEL, 0, 0));
	if (i != CB_ERR)
		nCharset = (char)(int) (::SendDlgItemMessage(m_hWnd, IDC_SERVER_FILE_CHARSET, CB_GETITEMDATA, (WPARAM) IntToPtr(i), 0));
	else
		nCharset = scsUTF8;

	m_pSettings->bTextMode = bTextMode;
	m_pSettings->nServerCharset = nCharset;

	*m_pbResult = true;
	return PSNRET_NOERROR;
}
