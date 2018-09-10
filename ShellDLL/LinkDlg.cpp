/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 LinkDlg.cpp - implementations of CLinkDialog
 */

#include "StdAfx.h"
#include "ShellDLL.h"
#include "LinkDlg.h"

CLinkDialog::CLinkDialog()
	: CMyDialog(IDD)
{
	m_bHardLink = false;
	m_bAllowHardLink = false;
}

CLinkDialog::~CLinkDialog()
{
}

LRESULT CLinkDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE_COMMAND(IDOK, OnOK);
	HANDLE_CONTROL(IDC_LINK_NAME, EN_CHANGE, OnLinkEditChange);
	HANDLE_CONTROL(IDC_FILE_NAME, EN_CHANGE, OnFileNameEditChange);
	return CMyDialog::WindowProc(message, wParam, lParam);
}

bool CLinkDialog::OnInitDialog(HWND hWndFocus)
{
	m_bFileNameChanged = true;
	::SyncDialogData(m_hWnd, IDC_CUR_DIR, m_strCurDir, false);
	::SyncDialogData(m_hWnd, IDC_LINK_NAME, m_strLinkTo, false);
	::SyncDialogData(m_hWnd, IDC_FILE_NAME, m_strFileName, false);
	::SyncDialogData(m_hWnd, IDC_HARD_LINK, m_bHardLink, false);
	::EnableDlgItem(m_hWnd, IDC_HARD_LINK, m_bAllowHardLink);
	if (m_strFileName.IsEmpty())
		m_bFileNameChanged = false;
	::SetDlgItemFocus(m_hWnd, IDC_LINK_NAME);
	return false;
}

LRESULT CLinkDialog::OnOK(WPARAM wParam, LPARAM lParam)
{
	CMyStringW strLinkTo, strFileName;
	::SyncDialogData(m_hWnd, IDC_LINK_NAME, strLinkTo, true);
	::SyncDialogData(m_hWnd, IDC_FILE_NAME, strFileName, true);
	if (strLinkTo.IsEmpty())
	{
		::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_NO_LINK_TO), NULL, MB_ICONEXCLAMATION);
		::SetDlgItemFocus(m_hWnd, IDC_LINK_NAME);
		return 0;
	}
	if (wcscspn(strLinkTo, INVALID_SERVER_FILE_NAME_CHARS_AP) < strLinkTo.GetLength())
	{
		::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_INVALID_FILE_NAME_CHAR), NULL, MB_ICONEXCLAMATION);
		::SetDlgItemFocus(m_hWnd, IDC_LINK_NAME);
		return 0;
	}
	if (strFileName.IsEmpty())
	{
		::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_NO_FILE_NAME), NULL, MB_ICONEXCLAMATION);
		::SetDlgItemFocus(m_hWnd, IDC_FILE_NAME);
		return 0;
	}
	if (wcscspn(strFileName, INVALID_SERVER_FILE_NAME_CHARS) < strFileName.GetLength())
	{
		::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_INVALID_FILE_NAME_CHAR), NULL, MB_ICONEXCLAMATION);
		::SetDlgItemFocus(m_hWnd, IDC_FILE_NAME);
		return 0;
	}

	m_strLinkTo = strLinkTo;
	m_strFileName = strFileName;
	if (m_bAllowHardLink)
		::SyncDialogData(m_hWnd, IDC_HARD_LINK, m_bHardLink, true);
	return CMyDialog::OnDefaultButton(wParam, lParam);
}

LRESULT CLinkDialog::OnLinkEditChange(WPARAM wParam, LPARAM lParam)
{
	if (!m_bFileNameChanged)
	{
		CMyStringW strLink, str;
		LPCWSTR lpw, lpw2;
		DWORD dwLen;
		::SyncDialogData(m_hWnd, IDC_LINK_NAME, strLink, true);
		lpw = strLink;
		lpw2 = wcsrchr(lpw, L'/');
		dwLen = lpw2 ? (DWORD)(((DWORD_PTR) lpw2 - (DWORD_PTR) lpw) / sizeof(WCHAR)) : 0;
		if (!lpw2 ||
			(m_strCurDir.GetLength() == dwLen &&
			m_strCurDir.Compare((LPCWSTR) strLink, false, dwLen) == 0))
		{
			str.LoadString(IDS_SHORTCUT_PREFIX);
			str += lpw2 ? lpw2 + 1 : lpw;
		}
		else
		{
			lpw2++;
			str = lpw2;
		}
		::SyncDialogData(m_hWnd, IDC_FILE_NAME, str, false);
		m_bFileNameChanged = false;
	}
	return 0;
}

LRESULT CLinkDialog::OnFileNameEditChange(WPARAM wParam, LPARAM lParam)
{
	m_bFileNameChanged = true;
	return 0;
}
