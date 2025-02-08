
#include "StdAfx.h"
#include "EasySFTP.h"
#include "Option.h"

COptionDialog::COptionDialog()
	: CMyDialog(IDD_OPTION)
{
}

LRESULT COptionDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE_COMMAND(IDOK, OnOK);
	HANDLE_COMMAND(IDC_REGISTER, OnRegister);
	HANDLE_COMMAND(IDC_REGISTER_SYSTEM, OnRegister);
	return CMyDialog::WindowProc(message, wParam, lParam);
}

bool COptionDialog::OnInitDialog(HWND hWndFocus)
{
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
