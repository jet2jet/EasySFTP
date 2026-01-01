/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 FNameDlg.cpp - implementations of FileNameDialog and CFileNameDialog
 */

#include "StdAfx.h"
#include "ShellDLL.h"
#include "FNameDlg.h"

#include "MyDialog.h"

class CFileNameDialog : public CMyDialog
{
public:
	CFileNameDialog();
	virtual ~CFileNameDialog();

	enum { IDD = IDD_FILENAME };

	CMyStringW m_strFileName;
	bool m_bNoPath;
	bool m_bAllowWildcard;

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual bool OnInitDialog(HWND hWndFocus);
	LRESULT OnOK(WPARAM wParam, LPARAM lParam);
};

bool __stdcall FileNameDialog(CMyStringW& rstrFileName, HWND hWndParent, bool bNoPath, bool bAllowWildcard)
{
	CFileNameDialog dlg;
	dlg.m_strFileName = rstrFileName;
	dlg.m_bNoPath = bNoPath;
	dlg.m_bAllowWildcard = bAllowWildcard;
	if (dlg.ModalDialogW(hWndParent) == IDOK)
	{
		rstrFileName = dlg.m_strFileName;
		return true;
	}
	return false;
}

CFileNameDialog::CFileNameDialog()
	: CMyDialog(IDD)
{
}

CFileNameDialog::~CFileNameDialog()
{
}

LRESULT CFileNameDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE_COMMAND(IDOK, OnOK);
	return CMyDialog::WindowProc(message, wParam, lParam);
}

bool CFileNameDialog::OnInitDialog(HWND hWndFocus)
{
	::SyncDialogData(m_hWnd, IDC_FILE_NAME, m_strFileName, false);
	return true;
}

LRESULT CFileNameDialog::OnOK(WPARAM wParam, LPARAM lParam)
{
	::SyncDialogData(m_hWnd, IDC_FILE_NAME, m_strFileName, true);
	LPCWSTR lp;
	if (m_bNoPath)
		lp = m_bAllowWildcard ? INVALID_SERVER_FILE_NAME_CHARS_AW : INVALID_SERVER_FILE_NAME_CHARS;
	else
		lp = m_bAllowWildcard ? INVALID_SERVER_FILE_NAME_CHARS_APPW : INVALID_SERVER_FILE_NAME_CHARS_APP;
	if (wcscspn(m_strFileName, lp) < m_strFileName.GetLength())
	{
		::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_INVALID_FILE_NAME_CHAR), NULL, MB_ICONEXCLAMATION);
		return 0;
	}
	return CMyDialog::OnDefaultButton(wParam, lParam);
}
