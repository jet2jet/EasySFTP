/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 Connect.h - declarations of CConnectDialog
 */

#pragma once

class CConnectDialog : public CMyDialog
{
public:
	CConnectDialog();
	virtual ~CConnectDialog();

	enum { IDD1 = IDD_QUICK_CONNECT, IDD2 = IDD_PASSWORD };

public:
	void SetDialogMode(bool bPasswordDialog);

	CMyStringW m_strHostName;
	int m_nPort;
	CMyStringW m_strUserName;
	_SecureStringW m_strPassword;
	EasySFTPConnectionMode m_ConnectionMode;
	EasySFTPAuthenticationMode m_nAuthType;
	CMyStringW m_strPKeyFileName;
	EVP_PKEY* m_pPKey;

	CMyStringW m_strMessage;
	bool m_bDisableAuthPassword;
	bool m_bDisableAuthPublicKey;

	HRESULT SetToAuthentication(IEasySFTPAuthentication* pUser);

protected:
	bool m_bPasswordDialog;

	void UpdateConnectionMode(EasySFTPConnectionMode ConnectionMode);

	virtual bool OnInitDialog(HWND hWndFocus);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnConnectModeChanged(WPARAM wParam, LPARAM lParam);
	LRESULT OnAuthTypeChecked(WPARAM wParam, LPARAM lParam);
	LRESULT OnSearchPKeyFile(WPARAM wParam, LPARAM lParam);
	LRESULT OnOK(WPARAM wParam, LPARAM lParam);
};
