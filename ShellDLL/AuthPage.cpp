/*
 EasySFTP - Copyright (C) 2025 jet (ジェット)

 AuthPage.cpp - implementations of CHostAuthPage
 */

#include "StdAfx.h"
#include "ShellDLL.h"
#include "AuthPage.h"

#include "HostPage.h"
#include "SSHCli.h"

CHostAuthPage::CHostAuthPage(CEasySFTPHostSetting* pSettings, CHostGeneralSettingPage* pHostSettingPage, bool* pbResult)
	: CMyPropertyPage(IDD)
	, m_pSettings(pSettings)
	, m_pHostSettingPage(pHostSettingPage)
	, m_pbResult(pbResult)
{
	pSettings->AddRef();
}

CHostAuthPage::~CHostAuthPage()
{
	m_pSettings->Release();
}

void CHostAuthPage::UpdateControlEnable()
{
	auto bAutoLogin = false;
	::SyncDialogData(m_hWnd, IDC_USE_AUTO_AUTH, bAutoLogin, true);
	char nAuthType;
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
	::EnableDlgItem(m_hWnd, IDC_USER_NAME, bAutoLogin);
	::EnableDlgItem(m_hWnd, IDC_PASSWORD, bAutoLogin && (nAuthType == AUTHTYPE_PASSWORD || nAuthType == AUTHTYPE_PUBLICKEY));
	::EnableDlgItem(m_hWnd, IDC_STORE_TYPE, bAutoLogin && (nAuthType == AUTHTYPE_PASSWORD || nAuthType == AUTHTYPE_PUBLICKEY));
	::EnableDlgItem(m_hWnd, IDC_AUTH_PASSWORD, bAutoLogin);
	::EnableDlgItem(m_hWnd, IDC_AUTH_PKEY, bAutoLogin);
	::EnableDlgItem(m_hWnd, IDC_AUTH_PAGEANT, bAutoLogin);
	::EnableDlgItem(m_hWnd, IDC_AUTH_WIN_SSHAGENT, bAutoLogin);
	::EnableDlgItem(m_hWnd, IDC_PKEY_FILE, bAutoLogin && (nAuthType == AUTHTYPE_PUBLICKEY));
	::EnableDlgItem(m_hWnd, IDC_PKEY_SEARCH, bAutoLogin && (nAuthType == AUTHTYPE_PUBLICKEY));
}

bool CHostAuthPage::ConfirmPasswordChanged()
{
	if (m_strDummyPKey.IsEmpty() || m_strDummyPassword.IsEmpty())
		return true;
	if (::IsDlgButtonChecked(m_hWnd, IDC_AUTH_PKEY) == BST_CHECKED)
	{
		CMyStringW strPassword;
		CMyStringW strPKey;
		::SyncDialogData(m_hWnd, IDC_PASSWORD, strPassword, true);
		::SyncDialogData(m_hWnd, IDC_PKEY_FILE, strPKey, true);
		if (m_strDummyPassword.Compare(strPassword) != 0 && m_strDummyPKey.Compare(strPKey) == 0)
		{
			if (::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_CHANGE_PASSWORD_WITH_PRIVATE_KEY), nullptr, MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
			{
				m_strDummyPKey.Empty();
				m_strDummyPassword.Empty();
				strPKey.Empty();
				::SyncDialogData(m_hWnd, IDC_PKEY_FILE, strPKey, false);
			}
			else
			{
				::SyncDialogData(m_hWnd, IDC_PASSWORD, m_strDummyPassword, false);
				::SetDlgItemFocus(m_hWnd, IDC_PASSWORD);
				return false;
			}
		}
	}
	return true;
}

LRESULT CHostAuthPage::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE_COMMAND(IDC_USE_AUTO_AUTH, OnAutoLoginChecked);
	HANDLE_COMMAND(IDC_AUTH_PASSWORD, OnAuthTypeChecked);
	HANDLE_COMMAND(IDC_AUTH_PKEY, OnAuthTypeChecked);
	HANDLE_COMMAND(IDC_AUTH_PAGEANT, OnAuthTypeChecked);
	HANDLE_COMMAND(IDC_AUTH_WIN_SSHAGENT, OnAuthTypeChecked);
	HANDLE_CONTROL(IDC_PASSWORD, EN_KILLFOCUS, OnPasswordKillFocus);
	HANDLE_COMMAND(IDC_PKEY_SEARCH, OnSearchPKeyFile);
	HANDLE_NOTIFY_CODE(PSN_APPLY, OnApply);
	return CMyPropertyPage::WindowProc(message, wParam, lParam);
}

bool CHostAuthPage::OnInitDialog(HWND hWndFocus)
{
	VARIANT_BOOL bHasPassword = VARIANT_FALSE;
	VARIANT_BOOL bHasPrivateKey = VARIANT_FALSE;
	m_pSettings->get_HasPassword(&bHasPassword);
	m_pSettings->get_HasPrivateKey(&bHasPrivateKey);
	if (bHasPassword || bHasPrivateKey)
		MakeRandomString(m_strDummyPassword);
	else
		m_strDummyPassword.Empty();
	::CheckDlgButton(m_hWnd, IDC_USE_AUTO_AUTH, m_pSettings->bAutoLogin);
	::SyncDialogData(m_hWnd, IDC_USER_NAME, m_pSettings->strUserName, false);
	::SyncDialogData(m_hWnd, IDC_PASSWORD, m_strDummyPassword, false);

	m_strDummyPKey.Empty();

	{
		EasySFTPPassKeyStoreType nStoreType = EasySFTPPassKeyStoreType::Local;
		m_pSettings->get_PassKeyStoreType(&nStoreType);
		CMyStringW str;
		int i;
		str.LoadString(IDS_STORE_TYPE_LOCAL);
		i = ::AddDlgComboBoxStringW(m_hWnd, IDC_STORE_TYPE, str);
		::SendDlgItemMessage(m_hWnd, IDC_STORE_TYPE, CB_SETITEMDATA, (WPARAM)IntToPtr(i), (LPARAM)IntToPtr(static_cast<int>(EasySFTPPassKeyStoreType::Local)));
		if (nStoreType == EasySFTPPassKeyStoreType::Local)
			::SendDlgItemMessage(m_hWnd, IDC_STORE_TYPE, CB_SETCURSEL, (WPARAM)IntToPtr(i), 0);
		str.LoadString(IDS_STORE_TYPE_USER);
		i = ::AddDlgComboBoxStringW(m_hWnd, IDC_STORE_TYPE, str);
		::SendDlgItemMessage(m_hWnd, IDC_STORE_TYPE, CB_SETITEMDATA, (WPARAM)IntToPtr(i), (LPARAM)IntToPtr(static_cast<int>(EasySFTPPassKeyStoreType::CurrentUser)));
		if (nStoreType == EasySFTPPassKeyStoreType::CurrentUser)
			::SendDlgItemMessage(m_hWnd, IDC_STORE_TYPE, CB_SETCURSEL, (WPARAM)IntToPtr(i), 0);
	}

	EasySFTPAuthenticationMode nAuthType = EasySFTPAuthenticationMode::None;
	m_pSettings->get_AuthenticationMode(&nAuthType);
	::CheckRadioButton(m_hWnd, IDC_AUTH_PASSWORD, IDC_AUTH_WIN_SSHAGENT,
		IDC_AUTH_PASSWORD + static_cast<int>(nAuthType) - static_cast<int>(EasySFTPAuthenticationMode::Password));
	{
		CMyStringW str;
		if (bHasPrivateKey)
		{
			m_strDummyPKey.LoadString(IDS_DATA_SET);
			str = m_strDummyPKey;
		}
		::SyncDialogData(m_hWnd, IDC_PKEY_FILE, str, false);
	}
	UpdateControlEnable();

	*m_pbResult = false;

	return true;
}

LRESULT CHostAuthPage::OnAutoLoginChecked(WPARAM wParam, LPARAM lParam)
{
	UpdateControlEnable();
	SetModifiedFlag();
	return 0;
}

LRESULT CHostAuthPage::OnAuthTypeChecked(WPARAM wParam, LPARAM lParam)
{
	UpdateControlEnable();
	SetModifiedFlag();
	ConfirmPasswordChanged();
	return 0;
}

LRESULT CHostAuthPage::OnPasswordKillFocus(WPARAM wParam, LPARAM lParam)
{
	ConfirmPasswordChanged();
	return 0;
}

LRESULT CHostAuthPage::OnSearchPKeyFile(WPARAM wParam, LPARAM lParam)
{
	CMyStringW strFile;
	::SyncDialogData(m_hWnd, IDC_PKEY_FILE, strFile, true);
	if (strFile.Compare(m_strDummyPKey) == 0)
		strFile.Empty();
	if (theApp.FileDialog(true, strFile, this))
	{
		::SyncDialogData(m_hWnd, IDC_PKEY_FILE, strFile, false);
		SetModifiedFlag();
	}
	return 0;
}

LRESULT CHostAuthPage::OnApply(WPARAM wParam, LPARAM lParam)
{
	// Host name is necessary for storing credentials
	CMyStringW strHost;
	CMyStringW strOldHost(m_pSettings->strHostName), strOldUser(m_pSettings->strUserName);
	auto oldStoreType = m_pSettings->PassKeyStoreType;
	{
		::SyncDialogData(m_pHostSettingPage->GetSafeHwnd(), IDC_HOST_NAME, strHost, true);
		if (strHost.IsEmpty())
		{
			::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_NO_HOST_NAME), NULL, MB_ICONEXCLAMATION);
			GetParentSheet()->SetCurSel(m_pHostSettingPage);
			return PSNRET_INVALID_NOCHANGEPAGE;
		}
	}

	bool bAutoLogin = false;
	CMyStringW strUserName, strPassword;
	::SyncDialogData(m_hWnd, IDC_USE_AUTO_AUTH, bAutoLogin, true);
	::SyncDialogData(m_hWnd, IDC_USER_NAME, strUserName, true);
	::SyncDialogData(m_hWnd, IDC_PASSWORD, strPassword, true);
	EasySFTPPassKeyStoreType nStoreType;
	{
		int i = (int)(::SendDlgItemMessage(m_hWnd, IDC_STORE_TYPE, CB_GETCURSEL, 0, 0));
		if (i != CB_ERR)
			i = (int)(::SendDlgItemMessage(m_hWnd, IDC_STORE_TYPE, CB_GETITEMDATA, (WPARAM)IntToPtr(i), 0));
		if (i < 0)
			nStoreType = EasySFTPPassKeyStoreType::Local;
		else
			nStoreType = static_cast<EasySFTPPassKeyStoreType>(i);
	}
	EasySFTPAuthenticationMode nAuthType;
	{
		if (::IsDlgButtonChecked(m_hWnd, IDC_AUTH_PASSWORD) == BST_CHECKED)
			nAuthType = EasySFTPAuthenticationMode::Password;
		else if (::IsDlgButtonChecked(m_hWnd, IDC_AUTH_PKEY) == BST_CHECKED)
			nAuthType = EasySFTPAuthenticationMode::PrivateKey;
		else if (::IsDlgButtonChecked(m_hWnd, IDC_AUTH_PAGEANT) == BST_CHECKED)
			nAuthType = EasySFTPAuthenticationMode::Pageant;
		else if (::IsDlgButtonChecked(m_hWnd, IDC_AUTH_WIN_SSHAGENT) == BST_CHECKED)
			nAuthType = EasySFTPAuthenticationMode::WinOpenSSH;
		else
			nAuthType = EasySFTPAuthenticationMode::Password;
	}
	if (bAutoLogin)
	{
		if (strUserName.IsEmpty())
		{
			_SecureStringW::SecureEmptyString(strPassword);
			::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_NO_USER_NAME), NULL, MB_ICONEXCLAMATION);
			GetParentSheet()->SetCurSel(this);
			return PSNRET_INVALID_NOCHANGEPAGE;
		}

		CMyStringW strPKey;
		if (nAuthType == EasySFTPAuthenticationMode::PrivateKey)
		{
			::SyncDialogData(m_hWnd, IDC_PKEY_FILE, strPKey, true);
			if (m_strDummyPKey.IsEmpty() || strPKey.Compare(m_strDummyPKey) != 0)
			{
				if (strPKey.IsEmpty())
				{
					_SecureStringW::SecureEmptyString(strPassword);
					::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_NO_PKEY_FILE_NAME), NULL, MB_ICONEXCLAMATION);
					GetParentSheet()->SetCurSel(this);
					return PSNRET_INVALID_NOCHANGEPAGE;
				}
				if (!m_strDummyPassword.IsEmpty() && m_strDummyPassword.Compare(strPassword) == 0)
				{
					_SecureStringW::SecureEmptyString(strPassword);
					::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_NO_PASSWORD), NULL, MB_ICONEXCLAMATION);
					GetParentSheet()->SetCurSel(this);
					return PSNRET_INVALID_NOCHANGEPAGE;
				}
				m_pSettings->SetHostAndUserName(strHost, strUserName);
				m_pSettings->PassKeyStoreType = nStoreType;
				BSTR bstrPKeyFile = ::MyStringToBSTR(strPKey);
				BSTR bstrPassword = ::MyStringToBSTR(strPassword);
				auto hr = m_pSettings->StorePrivateKeyFromFile(bstrPKeyFile, bstrPassword);
				if (FAILED(hr))
				{
					ULONG uErr = ERR_get_error();
					::SysFreeString(bstrPKeyFile);
					_SecureStringW::SecureEmptyBStr(bstrPassword);
					::SysFreeString(bstrPassword);
					_SecureStringW::SecureEmptyString(strPassword);
					m_pSettings->PassKeyStoreType = oldStoreType;
					m_pSettings->SetHostAndUserName(strOldHost, strOldUser);

					ERR_error_string_n(uErr, strHost.GetBufferA(MAX_PATH), MAX_PATH);
					strHost.ReleaseBufferA();
					strOldHost.Format(IDS_UNABLE_TO_LOAD_PKEY, (LPCWSTR)strHost);
					::MyMessageBoxW(m_hWnd, strOldHost, NULL, MB_ICONEXCLAMATION);
					GetParentSheet()->SetCurSel(this);
					return PSNRET_INVALID_NOCHANGEPAGE;
				}
				::SysFreeString(bstrPKeyFile);
				_SecureStringW::SecureEmptyBStr(bstrPassword);
				::SysFreeString(bstrPassword);
			}
		}
		else if (nAuthType == EasySFTPAuthenticationMode::Password)
		{
			if (strPassword.Compare(m_strDummyPassword) != 0)
			{
				BSTR bstrPassword = ::MyStringToBSTR(strPassword);
				m_pSettings->PassKeyStoreType = nStoreType;
				m_pSettings->SetHostAndUserName(strHost, strUserName);
				auto hr = m_pSettings->SetPassword(bstrPassword);
				if (FAILED(hr))
				{
					_SecureStringW::SecureEmptyBStr(bstrPassword);
					::SysFreeString(bstrPassword);
					_SecureStringW::SecureEmptyString(strPassword);
					m_pSettings->PassKeyStoreType = oldStoreType;
					m_pSettings->SetHostAndUserName(strOldHost, strOldUser);

					MyGetErrorMessageString(hr, strOldHost);
					::MyMessageBoxW(m_hWnd, strOldHost, NULL, MB_ICONEXCLAMATION);
					GetParentSheet()->SetCurSel(this);
					return PSNRET_INVALID_NOCHANGEPAGE;
				}
				_SecureStringW::SecureEmptyBStr(bstrPassword);
				::SysFreeString(bstrPassword);
			}
		}
		else
		{
			m_pSettings->ClearCredentials();
			m_pSettings->strUserName = strUserName;
		}
	}
	else
	{
		strUserName.Empty();
		nAuthType = EasySFTPAuthenticationMode::Password;
		nStoreType = EasySFTPPassKeyStoreType::Local;
		m_pSettings->ClearCredentials();
		m_pSettings->strUserName.Empty();
	}
	m_pSettings->bAutoLogin = bAutoLogin;
	m_pSettings->AuthMode = nAuthType;
	m_pSettings->PassKeyStoreType = nStoreType;

	_SecureStringW::SecureEmptyString(strPassword);

	*m_pbResult = true;
	return PSNRET_NOERROR;
}
