/*
 EasySFTP - Copyright (C) 2025 jet (ジェット)

 SyncDlg.cpp - implementation of CSyncDetailDialog
 */

#include "stdafx.h"
#include "EasySFTP.h"
#include "SyncDlg.h"

constexpr auto LEFT_CHAR = L"←";
constexpr auto RIGHT_CHAR = L"→";

CSyncDetailDialog::CSyncDetailDialog()
	: CMyDialog(IDD_SYNC_DETAIL)
{
	m_bIsLeftToRight = true;
	m_Flags = EasySFTPSynchronizeMode::SyncNormal;
}

LRESULT CSyncDetailDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE_COMMAND(IDOK, OnOK);
	HANDLE_CONTROL(IDC_DIRECTION, CBN_SELCHANGE, OnSelChange);
	return CMyDialog::WindowProc(message, wParam, lParam);
}

bool CSyncDetailDialog::OnInitDialog(HWND hWndFocus)
{
	CMyStringW str;
	str.LoadString(IDS_LEFT_TO_RIGHT);
	::AddDlgComboBoxStringW(m_hWnd, IDC_DIRECTION, str);
	str.LoadString(IDS_RIGHT_TO_LEFT);
	::AddDlgComboBoxStringW(m_hWnd, IDC_DIRECTION, str);

	::MySetDlgItemTextW(m_hWnd, IDC_LEFT_PATH, m_strLeft);
	::MySetDlgItemTextW(m_hWnd, IDC_RIGHT_PATH, m_strRight);
	::MySetDlgItemTextW(m_hWnd, IDC_DIRECTION_LABEL, m_bIsLeftToRight ? RIGHT_CHAR : LEFT_CHAR);
	::SendDlgItemMessage(m_hWnd, IDC_DIRECTION, CB_SETCURSEL, static_cast<WPARAM>(m_bIsLeftToRight ? 0 : 1), 0);
	::CheckDlgButton(m_hWnd, IDC_EXCLUDE_HIDDEN, (m_Flags & EasySFTPSynchronizeMode::SyncExcludeHidden) ? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(m_hWnd, IDC_DELETE_NON_EXISTENCE, (m_Flags & EasySFTPSynchronizeMode::SyncDeleteIfNotExist) ? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(m_hWnd, IDC_COPY_OLD_FILES, (m_Flags & EasySFTPSynchronizeMode::SyncCopyOld) ? BST_CHECKED : BST_UNCHECKED);

	return false;
}

LRESULT CSyncDetailDialog::OnSelChange(WPARAM wParam, LPARAM lParam)
{
	auto i = static_cast<int>(::SendDlgItemMessage(m_hWnd, IDC_DIRECTION, CB_GETCURSEL, 0, 0));
	m_bIsLeftToRight = i == 0;
	::MySetDlgItemTextW(m_hWnd, IDC_DIRECTION_LABEL, m_bIsLeftToRight ? RIGHT_CHAR : LEFT_CHAR);
	return 0;
}

LRESULT CSyncDetailDialog::OnOK(WPARAM wParam, LPARAM lParam)
{
	auto i = static_cast<int>(::SendDlgItemMessage(m_hWnd, IDC_DIRECTION, CB_GETCURSEL, 0, 0));
	m_bIsLeftToRight = i == 0;

	m_Flags = EasySFTPSynchronizeMode::SyncNormal;
	if (::IsDlgButtonChecked(m_hWnd, IDC_EXCLUDE_HIDDEN) == BST_CHECKED)
		m_Flags |= EasySFTPSynchronizeMode::SyncExcludeHidden;
	if (::IsDlgButtonChecked(m_hWnd, IDC_DELETE_NON_EXISTENCE) == BST_CHECKED)
		m_Flags |= EasySFTPSynchronizeMode::SyncDeleteIfNotExist;
	if (::IsDlgButtonChecked(m_hWnd, IDC_COPY_OLD_FILES) == BST_CHECKED)
		m_Flags |= EasySFTPSynchronizeMode::SyncCopyOld;
	return CMyDialog::OnDefaultButton(wParam, lParam);
}
