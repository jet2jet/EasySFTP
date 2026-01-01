
#include "StdAfx.h"
#include "EasySFTP.h"
#include "Option.h"

COptionDialog::COptionDialog()
	: CMyDialog(IDD_OPTION)
	, m_pRoot(nullptr)
{
}

COptionDialog::~COptionDialog()
{
	if (m_pRoot)
		m_pRoot->Release();
}

LRESULT COptionDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE_COMMAND(IDOK, OnOK);
	HANDLE_COMMAND(IDC_CLEAR_ALL_CREDENTIALS, OnClearAllCredentials);
	HANDLE_COMMAND(IDC_REGISTER, OnRegister);
	HANDLE_COMMAND(IDC_REGISTER_SYSTEM, OnRegister);
	return CMyDialog::WindowProc(message, wParam, lParam);
}

bool COptionDialog::OnInitDialog(HWND hWndFocus)
{
	if (!m_pRoot)
	{
		if (FAILED(theApp.m_pEasySFTPRoot->QueryInterface(IID_IEasySFTPRoot2, reinterpret_cast<void**>(&m_pRoot))))
			m_pRoot = nullptr;
	}
	bool bAnyCredentials = false;
	if (m_pRoot)
	{
		VARIANT_BOOL b = VARIANT_FALSE;
		m_pRoot->HasCredentials(&b);
		bAnyCredentials = b != VARIANT_FALSE;
	}
	EnableDlgItem(m_hWnd, IDC_CLEAR_ALL_CREDENTIALS, bAnyCredentials);

	CMyStringW strRegister;
	if (!theApp.m_bEmulatingRegistry)
	{
		strRegister.LoadString(IDS_UNREGISTER_BUTTON);
		MySetDlgItemTextW(m_hWnd, theApp.m_bIsRegisteredAsUserClass ? IDC_REGISTER : IDC_REGISTER_SYSTEM, strRegister);
		EnableDlgItem(m_hWnd, !theApp.m_bIsRegisteredAsUserClass ? IDC_REGISTER : IDC_REGISTER_SYSTEM, FALSE);
	}

	SendDlgItemMessage(m_hWnd, IDC_REGISTER_SYSTEM, BCM_SETSHIELD, 0, (LPARAM) TRUE);

	return false;
}

LRESULT COptionDialog::OnClearAllCredentials(WPARAM wParam, LPARAM lParam)
{
	if (!m_pRoot)
		return 0;

	if (::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_CLEAR_CREDENTIALS_CONFIRM), NULL,
		MB_ICONEXCLAMATION | MB_YESNO) != IDYES)
		return 0;
	m_pRoot->ClearAllCredentials();
	::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_CLEAR_CREDENTIALS_COMPLETE), NULL, MB_ICONINFORMATION);

	VARIANT_BOOL b = VARIANT_FALSE;
	m_pRoot->HasCredentials(&b);
	EnableDlgItem(m_hWnd, IDC_CLEAR_ALL_CREDENTIALS, b);

	return 0;
}

LRESULT COptionDialog::OnRegister(WPARAM wParam, LPARAM lParam)
{
	if (::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_RESTART_APP), NULL,
		MB_ICONINFORMATION | MB_OKCANCEL) != IDOK)
		return 0;

	::EndDialog(m_hWnd, (INT_PTR) LOWORD(wParam));
	return 0;
}

LRESULT COptionDialog::OnOK(WPARAM wParam, LPARAM lParam)
{
	return CMyDialog::OnDefaultButton(wParam, lParam);
}
