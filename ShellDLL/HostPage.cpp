/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 HostPage.cpp - implementations of CHostGeneralSettingPage
 */

#include "StdAfx.h"
#include "ShellDLL.h"
#include "HostPage.h"

CHostGeneralSettingPage::CHostGeneralSettingPage(CEasySFTPHostSetting* pSettings, bool* pbResult)
	: CMyPropertyPage(IDD)
	, m_pSettings(pSettings)
	, m_pbResult(pbResult)
	, m_bNoModeChange(false)
{
	pSettings->AddRef();
}

CHostGeneralSettingPage::~CHostGeneralSettingPage()
{
	m_pSettings->Release();
}

LRESULT CHostGeneralSettingPage::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE_CONTROL(IDC_DEF_PORT, BN_CLICKED, OnDefPortClicked);
	HANDLE_CONTROL(IDC_CONNECT_MODE, CBN_SELCHANGE, OnConnectModeChanged);
	HANDLE_CONTROL(IDC_LOCAL_PATH_SEARCH, BN_CLICKED, OnLocalPathSearch);
	HANDLE_NOTIFY_CODE(PSN_APPLY, OnApply);
	HANDLE_NOTIFY_CODE(PSN_KILLACTIVE, OnKillActive);
	return CMyPropertyPage::WindowProc(message, wParam, lParam);
}

bool CHostGeneralSettingPage::OnInitDialog(HWND hWndFocus)
{
	CMyStringW str;
	int i;
	str.LoadString(IDS_CONNECTMODE_FTP);
	i = ::AddDlgComboBoxStringW(m_hWnd, IDC_CONNECT_MODE, str);
	::SendDlgItemMessage(m_hWnd, IDC_CONNECT_MODE, CB_SETITEMDATA, (WPARAM) IntToPtr(i), (LPARAM) IntToPtr(static_cast<int>(EasySFTPConnectionMode::FTP)));
	if (m_pSettings->ConnectionMode == EasySFTPConnectionMode::FTP)
		::SendDlgItemMessage(m_hWnd, IDC_CONNECT_MODE, CB_SETCURSEL, (WPARAM) IntToPtr(i), 0);
	str.LoadString(IDS_CONNECTMODE_SFTP);
	i = ::AddDlgComboBoxStringW(m_hWnd, IDC_CONNECT_MODE, str);
	::SendDlgItemMessage(m_hWnd, IDC_CONNECT_MODE, CB_SETITEMDATA, (WPARAM) IntToPtr(i), (LPARAM) IntToPtr(static_cast<int>(EasySFTPConnectionMode::SFTP)));
	if (m_pSettings->ConnectionMode == EasySFTPConnectionMode::SFTP)
		::SendDlgItemMessage(m_hWnd, IDC_CONNECT_MODE, CB_SETCURSEL, (WPARAM) IntToPtr(i), 0);
	if (m_bNoModeChange)
		::EnableDlgItem(m_hWnd, IDC_CONNECT_MODE, m_bNoModeChange);
	str.LoadString(IDS_CONNECTMODE_FTPS);
	i = ::AddDlgComboBoxStringW(m_hWnd, IDC_CONNECT_MODE, str);
	::SendDlgItemMessage(m_hWnd, IDC_CONNECT_MODE, CB_SETITEMDATA, (WPARAM) IntToPtr(i), (LPARAM) IntToPtr(static_cast<int>(EasySFTPConnectionMode::FTPS)));
	if (m_pSettings->ConnectionMode == EasySFTPConnectionMode::FTPS)
		::SendDlgItemMessage(m_hWnd, IDC_CONNECT_MODE, CB_SETCURSEL, (WPARAM) IntToPtr(i), 0);

	::SyncDialogData(m_hWnd, IDC_NAME, m_pSettings->strDisplayName, false);
	::SyncDialogData(m_hWnd, IDC_HOST_NAME, m_pSettings->strHostName, false);
	::SyncDialogData(m_hWnd, IDC_PORT, m_pSettings->nPort, false);
	{
		register int nDefPort = m_pSettings->ConnectionMode == EasySFTPConnectionMode::SFTP ? 22 : 21;
		::EnableDlgItem(m_hWnd, IDC_PORT, m_pSettings->nPort != nDefPort);
		::CheckDlgButton(m_hWnd, IDC_DEF_PORT, m_pSettings->nPort == nDefPort ? BST_CHECKED : BST_UNCHECKED);
	}
	//::SyncDialogData(m_hWnd, IDC_USER_NAME, m_pSettings->strUserName, false);
	::SyncDialogData(m_hWnd, IDC_LOCAL_PATH, m_pSettings->strInitLocalPath, false);
	::SyncDialogData(m_hWnd, IDC_SERVER_PATH, m_pSettings->strInitServerPath, false);
	::SyncDialogData(m_hWnd, IDC_USE_THUMBNAIL, m_pSettings->bUseThumbnailPreview, false);

	*m_pbResult = false;

	return true;
}

LRESULT CHostGeneralSettingPage::OnDefPortClicked(WPARAM wParam, LPARAM lParam)
{
	register BOOL b = (::IsDlgButtonChecked(m_hWnd, IDC_DEF_PORT) == BST_CHECKED);
	::EnableDlgItem(m_hWnd, IDC_PORT, !b);
	if (b)
	{
		int i = (int) (::SendDlgItemMessage(m_hWnd, IDC_CONNECT_MODE, CB_GETCURSEL, 0, 0));
		if (i != CB_ERR)
			i = (int) (::SendDlgItemMessage(m_hWnd, IDC_CONNECT_MODE, CB_GETITEMDATA, (WPARAM) IntToPtr(i), 0));
		::SetDlgItemInt(m_hWnd, IDC_PORT, (i == static_cast<int>(EasySFTPConnectionMode::SFTP)) ? 22 : 21, FALSE);
	}
	return 0;
}

LRESULT CHostGeneralSettingPage::OnLocalPathSearch(WPARAM wParam, LPARAM lParam)
{
	CMyStringW strDir;
	::SyncDialogData(m_hWnd, IDC_LOCAL_PATH, strDir, true);
	if (theApp.FolderDialog(strDir, this))
		::SyncDialogData(m_hWnd, IDC_LOCAL_PATH, strDir, false);
	return 0;
}

LRESULT CHostGeneralSettingPage::OnConnectModeChanged(WPARAM wParam, LPARAM lParam)
{
	register BOOL b = (::IsDlgButtonChecked(m_hWnd, IDC_DEF_PORT) == BST_CHECKED);
	if (b)
	{
		int i = (int) (::SendDlgItemMessage(m_hWnd, IDC_CONNECT_MODE, CB_GETCURSEL, 0, 0));
		if (i != CB_ERR)
			i = (int) (::SendDlgItemMessage(m_hWnd, IDC_CONNECT_MODE, CB_GETITEMDATA, (WPARAM) IntToPtr(i), 0));
		::SetDlgItemInt(m_hWnd, IDC_PORT, (i == static_cast<int>(EasySFTPConnectionMode::SFTP)) ? 22 : 21, FALSE);
	}
	return 0;
}

LRESULT CHostGeneralSettingPage::OnApply(WPARAM wParam, LPARAM lParam)
{
	CMyStringW strName;
	::SyncDialogData(m_hWnd, IDC_NAME, strName, true);
	if (strName.IsEmpty())
	{
		::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_NO_SET_NAME), NULL, MB_ICONEXCLAMATION);
		GetParentSheet()->SetCurSel(this);
		return PSNRET_INVALID_NOCHANGEPAGE;
	}
	CMyStringW strHost;
	::SyncDialogData(m_hWnd, IDC_HOST_NAME, strHost, true);
	if (strHost.IsEmpty())
	{
		::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_NO_HOST_NAME), NULL, MB_ICONEXCLAMATION);
		GetParentSheet()->SetCurSel(this);
		return PSNRET_INVALID_NOCHANGEPAGE;
	}
	//CMyStringW strUserName;
	//::SyncDialogData(m_hWnd, IDC_USER_NAME, strUserName, true);
	//if (strUserName.IsEmpty())
	//{
	//	::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_NO_USER_NAME), NULL, MB_ICONEXCLAMATION);
	//	GetParentSheet()->SetCurSel(this);
	//	return PSNRET_INVALID_NOCHANGEPAGE;
	//}

	int nPort;
	EasySFTPConnectionMode mode;
	::SyncDialogData(m_hWnd, IDC_PORT, nPort, true);
	int i = (int) (::SendDlgItemMessage(m_hWnd, IDC_CONNECT_MODE, CB_GETCURSEL, 0, 0));
	if (i != CB_ERR)
		i = (int) (::SendDlgItemMessage(m_hWnd, IDC_CONNECT_MODE, CB_GETITEMDATA, (WPARAM) IntToPtr(i), 0));
	if (i < 0)
		mode = EasySFTPConnectionMode::FTP;
	else
		mode = static_cast<EasySFTPConnectionMode>(i);

	m_pSettings->strDisplayName = strName;
	m_pSettings->strHostName = strHost;
	//m_pSettings->strUserName = strUserName;
	::SyncDialogData(m_hWnd, IDC_LOCAL_PATH, m_pSettings->strInitLocalPath, true);
	::SyncDialogData(m_hWnd, IDC_SERVER_PATH, m_pSettings->strInitServerPath, true);
	::SyncDialogData(m_hWnd, IDC_USE_THUMBNAIL, m_pSettings->bUseThumbnailPreview, true);
	if (!m_bNoModeChange)
		m_pSettings->ConnectionMode = mode;
	m_pSettings->nPort = nPort;
	*m_pbResult = true;

	return PSNRET_NOERROR;
}

LRESULT CHostGeneralSettingPage::OnKillActive(WPARAM wParam, LPARAM lParam)
{
	// informs the current mode (FTP or SFTP) to another pages
	int i = (int) (::SendDlgItemMessage(m_hWnd, IDC_CONNECT_MODE, CB_GETCURSEL, 0, 0));
	if (i != CB_ERR)
		i = (int) (::SendDlgItemMessage(m_hWnd, IDC_CONNECT_MODE, CB_GETITEMDATA, (WPARAM) IntToPtr(i), 0));
	if (i < 0)
		m_pSettings->ConnectionMode = EasySFTPConnectionMode::FTP;
	else
		m_pSettings->ConnectionMode = static_cast<EasySFTPConnectionMode>(i);
	return FALSE;
}
