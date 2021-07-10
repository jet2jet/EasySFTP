/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 TferPage.cpp - implementations of CHostTransferPage
 */

#include "StdAfx.h"
#include "ShellDLL.h"
#include "TferPage.h"

#include "FNameDlg.h"

CHostTransferPage::CHostTransferPage(CHostSettings* pSettings, bool* pbResult)
	: CMyPropertyPage(IDD)
	, m_pSettings(pSettings)
	, m_pbResult(pbResult)
{
}

CHostTransferPage::~CHostTransferPage(void)
{
}

LRESULT CHostTransferPage::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE_CONTROL(IDC_ADD_FILE, BN_CLICKED, OnAddFileClicked);
	HANDLE_CONTROL(IDC_DEL_FILE, BN_CLICKED, OnDelFileClicked);
	HANDLE_NOTIFY_CODE(PSN_APPLY, OnApply);
	HANDLE_NOTIFY_CODE(PSN_SETACTIVE, OnSetActive);
	return CMyPropertyPage::WindowProc(message, wParam, lParam);
}

bool CHostTransferPage::OnInitDialog(HWND hWndFocus)
{
	//CMyStringW str;
	int i;

	::CheckRadioButton(m_hWnd, IDC_FILE_AUTO, IDC_FILE_BINARY, IDC_FILE_AUTO + m_pSettings->nTransferMode);
	m_arrTextFileType.CopyArray(m_pSettings->arrTextFileType);
	m_arrTextFileType.SetCaseSensitive(true);
	for (i = 0; i < m_arrTextFileType.GetCount(); i++)
	{
		LPWSTR lpw = m_arrTextFileType.GetItem(i);
		int iIndex = ::AddDlgListBoxStringW(m_hWnd, IDC_TEXTFILE_PATTERN, lpw);
		::SendDlgItemMessage(m_hWnd, IDC_TEXTFILE_PATTERN, LB_SETITEMDATA, (WPARAM) (iIndex), (LPARAM) lpw);
	}
	::SyncDialogData(m_hWnd, IDC_ADD_SYSTEM, m_pSettings->bUseSystemTextFileType, false);
	::SyncDialogData(m_hWnd, IDC_KEEP_RECV_MODIFY_TIME, m_pSettings->bAdjustRecvModifyTime, false);
	::SyncDialogData(m_hWnd, IDC_KEEP_SEND_MODIFY_TIME, m_pSettings->bAdjustSendModifyTime, false);

	*m_pbResult = false;

	return true;
}

LRESULT CHostTransferPage::OnAddFileClicked(WPARAM wParam, LPARAM lParam)
{
	CMyStringW str;
	if (::FileNameDialog(str, m_hWnd))
	{
		if (m_arrTextFileType.FindItem(str) < 0)
		{
			int i = m_arrTextFileType.Add(str);
			LPWSTR lpw = m_arrTextFileType.GetItem(i);
			int iIndex = ::AddDlgListBoxStringW(m_hWnd, IDC_TEXTFILE_PATTERN, lpw);
			::SendDlgItemMessage(m_hWnd, IDC_TEXTFILE_PATTERN, LB_SETITEMDATA, (WPARAM) (iIndex), (LPARAM) lpw);
			::SendDlgItemMessage(m_hWnd, IDC_TEXTFILE_PATTERN, LB_SETCURSEL, (WPARAM) (iIndex), 0);
		}
	}
	return 0;
}

LRESULT CHostTransferPage::OnDelFileClicked(WPARAM wParam, LPARAM lParam)
{
	int iIndex = (int) (::SendDlgItemMessage(m_hWnd, IDC_TEXTFILE_PATTERN, LB_GETCURSEL, 0, 0));
	if (iIndex != LB_ERR)
	{
		LPWSTR lpw = (LPWSTR) ::SendDlgItemMessage(m_hWnd, IDC_TEXTFILE_PATTERN, LB_GETITEMDATA, (WPARAM) (iIndex), 0);
		int i = m_arrTextFileType.FindItem(lpw);
		if (i >= 0)
		{
			::SendDlgItemMessage(m_hWnd, IDC_TEXTFILE_PATTERN, LB_DELETESTRING, (WPARAM) (iIndex), 0);
			m_arrTextFileType.RemoveItem(i);
			if (iIndex >= m_arrTextFileType.GetCount())
				iIndex--;
			if (iIndex >= 0)
				::SendDlgItemMessage(m_hWnd, IDC_TEXTFILE_PATTERN, LB_SETCURSEL, (WPARAM) (iIndex), 0);
		}
	}
	return 0;
}

LRESULT CHostTransferPage::OnApply(WPARAM wParam, LPARAM lParam)
{
	if (::IsDlgButtonChecked(m_hWnd, IDC_FILE_AUTO) == BST_CHECKED)
		m_pSettings->nTransferMode = TRANSFER_MODE_AUTO;
	else if (::IsDlgButtonChecked(m_hWnd, IDC_FILE_TEXT) == BST_CHECKED)
		m_pSettings->nTransferMode = TRANSFER_MODE_TEXT;
	else if (::IsDlgButtonChecked(m_hWnd, IDC_FILE_BINARY) == BST_CHECKED)
		m_pSettings->nTransferMode = TRANSFER_MODE_BINARY;
	m_pSettings->arrTextFileType.CopyArray(m_arrTextFileType);
	::SyncDialogData(m_hWnd, IDC_ADD_SYSTEM, m_pSettings->bUseSystemTextFileType, true);
	::SyncDialogData(m_hWnd, IDC_KEEP_RECV_MODIFY_TIME, m_pSettings->bAdjustRecvModifyTime, true);
	::SyncDialogData(m_hWnd, IDC_KEEP_SEND_MODIFY_TIME, m_pSettings->bAdjustSendModifyTime, true);
	*m_pbResult = true;

	return PSNRET_NOERROR;
}

LRESULT CHostTransferPage::OnSetActive(WPARAM wParam, LPARAM lParam)
{
	// disables IDC_KEEP_SEND_MODIFY_TIME if the mode is FTP
	::EnableDlgItem(m_hWnd, IDC_KEEP_SEND_MODIFY_TIME, m_pSettings->bSFTPMode);
	return 0;
}
