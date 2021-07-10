/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 Connect.h - implementations of CConnectDialog
 */

#include "StdAfx.h"
#include "ShellDLL.h"
#include "Connect.h"

#include "SSHCli.h"
#include "Pageant.h"
#include "WinOpSSH.h"

extern EVP_PKEY* __stdcall GetPrivateKey(FILE* pFileKey, const _SecureStringW& strPassword)
{
	 EVP_PKEY* ret;
	CMyStringW strBuffer;

	strPassword.GetString(strBuffer);
	ret = PEM_read_PrivateKey(pFileKey, NULL, NULL, strBuffer.IsEmpty() ? "" : (LPSTR) strBuffer);
	_SecureStringW::SecureEmptyString(strBuffer);
	strBuffer.Empty();
	return ret;
}

CConnectDialog::CConnectDialog(void)
	: CMyDialog(IDD1)
{
	m_bPasswordDialog = false;
	m_nPort = 21;
	m_bSFTPMode = false;
	m_pPKey = NULL;
	m_nAuthType = AUTHTYPE_PASSWORD;
}

CConnectDialog::~CConnectDialog(void)
{
	if (m_pPKey)
		EVP_PKEY_free(m_pPKey);
}

void CConnectDialog::SetDialogMode(bool bPasswordDialog)
{
	m_bPasswordDialog = bPasswordDialog;
	m_uID = bPasswordDialog ? IDD2 : IDD1;
}

bool CConnectDialog::OnInitDialog(HWND hWndFocus)
{
	if (!m_bPasswordDialog)
	{
		::SyncDialogData(m_hWnd, IDC_HOST_NAME, m_strHostName, false);
		::SyncDialogData(m_hWnd, IDC_PORT, m_nPort, false);
	}
	else if (!m_strMessage.IsEmpty())
		::SyncDialogData(m_hWnd, IDC_MESSAGE, m_strMessage, false);
	::SyncDialogData(m_hWnd, IDC_USER_NAME, m_strUserName, false);
	m_strPassword.SetStringToWindowText(::GetDlgItem(m_hWnd, IDC_PASSWORD));
	if (!m_bPasswordDialog)
		::SyncDialogData(m_hWnd, IDC_USE_SFTP, m_bSFTPMode, false);
	else
	{
		while (true)
		{
			if (m_bDisableAuthPassword && m_nAuthType == AUTHTYPE_PASSWORD)
				;
			else if (m_bDisableAuthPublicKey && m_nAuthType == AUTHTYPE_PUBLICKEY)
				;
			else if (m_bDisableAuthPublicKey && m_nAuthType == AUTHTYPE_PAGEANT)
				;
			else if (m_bDisableAuthPublicKey && m_nAuthType == AUTHTYPE_WINSSHAGENT)
				;
			else
				break;
			m_nAuthType++;
		}
	}
	::CheckRadioButton(m_hWnd, IDC_AUTH_PASSWORD, IDC_AUTH_PKEY, IDC_AUTH_PASSWORD + m_nAuthType - AUTHTYPE_PASSWORD);
	::SyncDialogData(m_hWnd, IDC_PKEY_FILE, m_strPKeyFileName, false);

	if (!m_bPasswordDialog)
		m_bDisableAuthPassword = m_bDisableAuthPublicKey = false;
	::EnableDlgItem(m_hWnd, IDC_AUTH_PASSWORD, m_bSFTPMode && !m_bDisableAuthPassword);
	::EnableDlgItem(m_hWnd, IDC_AUTH_PKEY, m_bSFTPMode && !m_bDisableAuthPublicKey);
	::EnableDlgItem(m_hWnd, IDC_AUTH_PAGEANT, m_bSFTPMode && !m_bDisableAuthPublicKey);
	::EnableDlgItem(m_hWnd, IDC_AUTH_WIN_SSHAGENT, m_bSFTPMode && !m_bDisableAuthPublicKey);
	::EnableDlgItem(m_hWnd, IDC_PKEY_FILE, m_bSFTPMode && !m_bDisableAuthPublicKey && (m_nAuthType == AUTHTYPE_PUBLICKEY));
	::EnableDlgItem(m_hWnd, IDC_PKEY_SEARCH, m_bSFTPMode && !m_bDisableAuthPublicKey && (m_nAuthType == AUTHTYPE_PUBLICKEY));

	if ((m_bPasswordDialog || !m_strHostName.IsEmpty()))
	{
		if (m_strUserName.IsEmpty())
			::SetDlgItemFocus(m_hWnd, IDC_USER_NAME);
		else if (m_strPassword.IsEmpty())
			::SetDlgItemFocus(m_hWnd, IDC_PASSWORD);
	}
	return false;
}

LRESULT CConnectDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE_COMMAND(IDC_USE_SFTP, OnSFTPModeChecked);
	HANDLE_COMMAND(IDC_AUTH_PASSWORD, OnAuthTypeChecked);
	HANDLE_COMMAND(IDC_AUTH_PKEY, OnAuthTypeChecked);
	HANDLE_COMMAND(IDC_AUTH_PAGEANT, OnAuthTypeChecked);
	HANDLE_COMMAND(IDC_AUTH_WIN_SSHAGENT, OnAuthTypeChecked);
	HANDLE_COMMAND(IDC_PKEY_SEARCH, OnSearchPKeyFile);
	HANDLE_COMMAND(IDOK, OnOK);
	return CMyDialog::WindowProc(message, wParam, lParam);
}

LRESULT CConnectDialog::OnSFTPModeChecked(WPARAM wParam, LPARAM lParam)
{
	bool bSFTPMode;
	char nAuthType;
	::SyncDialogData(m_hWnd, IDC_USE_SFTP, bSFTPMode, true);
	if (::IsDlgButtonChecked(m_hWnd, IDC_AUTH_PASSWORD) == BST_CHECKED)
		nAuthType = AUTHTYPE_PASSWORD;
	else if (::IsDlgButtonChecked(m_hWnd, IDC_AUTH_PKEY) == BST_CHECKED)
		nAuthType = AUTHTYPE_PUBLICKEY;
	else if (::IsDlgButtonChecked(m_hWnd, IDC_AUTH_PAGEANT) == BST_CHECKED)
		nAuthType = AUTHTYPE_PAGEANT;
	else
		nAuthType = AUTHTYPE_PASSWORD;
	::EnableDlgItem(m_hWnd, IDC_AUTH_PASSWORD, bSFTPMode);
	::EnableDlgItem(m_hWnd, IDC_AUTH_PKEY, bSFTPMode);
	::EnableDlgItem(m_hWnd, IDC_PKEY_FILE, bSFTPMode && (nAuthType == AUTHTYPE_PUBLICKEY));
	::EnableDlgItem(m_hWnd, IDC_PKEY_SEARCH, bSFTPMode && (nAuthType == AUTHTYPE_PUBLICKEY));
	::SetDlgItemInt(m_hWnd, IDC_PORT, bSFTPMode ? 22 : 21, FALSE);
	::EnableDlgItem(m_hWnd, IDC_PASSWORD, !bSFTPMode || (nAuthType != AUTHTYPE_PAGEANT));
	return 0;
}

LRESULT CConnectDialog::OnAuthTypeChecked(WPARAM wParam, LPARAM lParam)
{
	bool bSFTPMode;
	char nAuthType;
	if (m_bPasswordDialog)
		bSFTPMode = m_bSFTPMode;
	else
		::SyncDialogData(m_hWnd, IDC_USE_SFTP, bSFTPMode, true);
	if (::IsDlgButtonChecked(m_hWnd, IDC_AUTH_PASSWORD) == BST_CHECKED)
		nAuthType = AUTHTYPE_PASSWORD;
	else if (::IsDlgButtonChecked(m_hWnd, IDC_AUTH_PKEY) == BST_CHECKED)
		nAuthType = AUTHTYPE_PUBLICKEY;
	else if (::IsDlgButtonChecked(m_hWnd, IDC_AUTH_PAGEANT) == BST_CHECKED)
		nAuthType = AUTHTYPE_PAGEANT;
	else if (::IsDlgButtonChecked(m_hWnd, IDC_AUTH_WIN_SSHAGENT) == BST_CHECKED)
		nAuthType = AUTHTYPE_WINSSHAGENT;
	::EnableDlgItem(m_hWnd, IDC_PKEY_FILE, bSFTPMode && (nAuthType == AUTHTYPE_PUBLICKEY));
	::EnableDlgItem(m_hWnd, IDC_PKEY_SEARCH, bSFTPMode && (nAuthType == AUTHTYPE_PUBLICKEY));
	::EnableDlgItem(m_hWnd, IDC_PASSWORD, !bSFTPMode || (nAuthType != AUTHTYPE_PAGEANT && nAuthType != AUTHTYPE_WINSSHAGENT));
	return 0;
}

LRESULT CConnectDialog::OnSearchPKeyFile(WPARAM wParam, LPARAM lParam)
{
	CMyStringW strFile;
	::SyncDialogData(m_hWnd, IDC_PKEY_FILE, strFile, true);
	if (theApp.FileDialog(true, strFile, this))
		::SyncDialogData(m_hWnd, IDC_PKEY_FILE, strFile, false);
	return 0;
}

LRESULT CConnectDialog::OnOK(WPARAM wParam, LPARAM lParam)
{
	CMyStringW strHost, strUser, strPKey;
	_SecureStringW strPassword;
	int nPort;
	bool bSFTPMode;
	char nAuthType;
	EVP_PKEY* pPKey;

	if (!m_bPasswordDialog)
	{
		::SyncDialogData(m_hWnd, IDC_HOST_NAME, strHost, true);
		if (strHost.IsEmpty())
		{
			::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_NO_HOST_NAME), NULL, MB_ICONEXCLAMATION);
			return 0;
		}
		::SyncDialogData(m_hWnd, IDC_PORT, nPort, true);
		if (!nPort || nPort > 65535)
		{
			::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_INVALID_PORT), NULL, MB_ICONEXCLAMATION);
			return 0;
		}
		::SyncDialogData(m_hWnd, IDC_USE_SFTP, bSFTPMode, true);
	}
	else
		bSFTPMode = m_bSFTPMode;
	::SyncDialogData(m_hWnd, IDC_USER_NAME, strUser, true);
	if (strUser.IsEmpty())
	{
		::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_NO_USER_NAME), NULL, MB_ICONEXCLAMATION);
		return 0;
	}
	// Do not use IsEmpty() here
	if (!strPassword.GetStringFromWindowText(::GetDlgItem(m_hWnd, IDC_PASSWORD)))
	{
		::MessageBeep(MB_ICONEXCLAMATION);
		return 0;
	}
	if (bSFTPMode)
	{
		if (::IsDlgButtonChecked(m_hWnd, IDC_AUTH_PASSWORD) == BST_CHECKED)
			nAuthType = AUTHTYPE_PASSWORD;
		else if (::IsDlgButtonChecked(m_hWnd, IDC_AUTH_PKEY) == BST_CHECKED)
			nAuthType = AUTHTYPE_PUBLICKEY;
		else if (::IsDlgButtonChecked(m_hWnd, IDC_AUTH_PAGEANT) == BST_CHECKED)
			nAuthType = AUTHTYPE_PAGEANT;
		else if (::IsDlgButtonChecked(m_hWnd, IDC_AUTH_WIN_SSHAGENT) == BST_CHECKED)
			nAuthType = AUTHTYPE_WINSSHAGENT;
		else
			nAuthType = AUTHTYPE_PASSWORD;
		if (nAuthType == AUTHTYPE_PUBLICKEY)
		{
			::SyncDialogData(m_hWnd, IDC_PKEY_FILE, strPKey, true);
			if (strPKey.IsEmpty())
			{
				::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_NO_PKEY_FILE_NAME), NULL, MB_ICONEXCLAMATION);
				return 0;
			}

			FILE* pFileKey;
			if (_wfopen_s(&pFileKey, strPKey, L"r") != 0)
			{
				if (fopen_s(&pFileKey, strPKey, "r") != 0)
				{
					::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_PKEY_FILE_NOT_FOUND), NULL, MB_ICONEXCLAMATION);
					return 0;
				}
			}
			pPKey = GetPrivateKey(pFileKey, strPassword);
			fclose(pFileKey);
			if (pPKey == NULL)
			{
				ULONG uErr = ERR_get_error();
				ERR_error_string_n(uErr, strHost.GetBufferA(MAX_PATH), MAX_PATH);
				strHost.ReleaseBufferA();
				strUser.Format(IDS_UNABLE_TO_LOAD_PKEY, (LPCWSTR) strHost);
				::MyMessageBoxW(m_hWnd, strUser, NULL, MB_ICONEXCLAMATION);
				return 0;
			}
		}
		else if (nAuthType == AUTHTYPE_PAGEANT)
		{
			if (!CPageantAgent::IsAvailable())
			{
				::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_PAGEANT_NOT_AVAILABLE), NULL, MB_ICONEXCLAMATION);
				return 0;
			}
			pPKey = NULL;
		}
		else if (nAuthType == AUTHTYPE_WINSSHAGENT)
		{
			if (!CWinOpenSSHAgent::IsAvailable())
			{
				::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_WINSSHAGENT_NOT_AVAILABLE), NULL, MB_ICONEXCLAMATION);
				return 0;
			}
			pPKey = NULL;
		}
		else
			pPKey = NULL;
	}
	else
	{
		nAuthType = AUTHTYPE_PASSWORD;
		pPKey = NULL;
	}

	if (!m_bPasswordDialog)
	{
		m_strHostName = strHost;
		m_nPort = nPort;
		m_bSFTPMode = bSFTPMode;
	}
	m_strUserName = strUser;
	m_strPassword = strPassword;
	m_nAuthType = nAuthType;
	if (m_pPKey)
		EVP_PKEY_free(m_pPKey);
	m_pPKey = pPKey;
	m_strPKeyFileName = strPKey;
	return CMyDialog::OnDefaultButton(wParam, lParam);
}
