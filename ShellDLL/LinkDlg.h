/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 LinkDlg.h - declarations of CLinkDialog
 */

#pragma once

class CLinkDialog : public CMyDialog
{
public:
	CLinkDialog();
	virtual ~CLinkDialog();

	enum { IDD = IDD_NEWLINK };

public:
	CMyStringW m_strCurDir;
	CMyStringW m_strLinkTo;
	CMyStringW m_strFileName;
	bool m_bHardLink;
	bool m_bAllowHardLink;

protected:
	bool m_bFileNameChanged;

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual bool OnInitDialog(HWND hWndFocus);
	LRESULT OnOK(WPARAM wParam, LPARAM lParam);
	LRESULT OnLinkEditChange(WPARAM wParam, LPARAM lParam);
	LRESULT OnFileNameEditChange(WPARAM wParam, LPARAM lParam);
};
