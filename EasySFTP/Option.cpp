
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
	return CMyDialog::WindowProc(message, wParam, lParam);
}

bool COptionDialog::OnInitDialog(HWND hWndFocus)
{
	CMyStringW strRegister;
	strRegister.LoadString(theApp.m_bEmulatingRegistry
		? IDS_REGISTER_BUTTON : IDS_UNREGISTER_BUTTON);
	MySetDlgItemTextW(m_hWnd, IDC_REGISTER, strRegister);

	SendDlgItemMessage(m_hWnd, IDC_REGISTER, BCM_SETSHIELD, 0, (LPARAM) TRUE);

	return false;
}

LRESULT COptionDialog::OnRegister(WPARAM wParam, LPARAM lParam)
{
	if (::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_RESTART_APP), NULL,
		MB_ICONINFORMATION | MB_OKCANCEL) != IDOK)
		return 0;

	::EndDialog(m_hWnd, IDC_REGISTER);
	return 0;
}

LRESULT COptionDialog::OnOK(WPARAM wParam, LPARAM lParam)
{
	return CMyDialog::OnDefaultButton(wParam, lParam);
}
