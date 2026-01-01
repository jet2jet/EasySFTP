/*
 EasySFTP - Copyright (C) 2025 jet (ジェット)

 SyncDlg.h - declaration of CSyncDetailDialog
 */

#pragma once

#include "BitFlags.h"

class CSyncDetailDialog : public CMyDialog
{
public:
	CSyncDetailDialog();

	CMyStringW m_strLeft;
	CMyStringW m_strRight;

	bool m_bIsLeftToRight;
	EasySFTPSynchronizeModeFlags m_Flags;

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual bool OnInitDialog(HWND hWndFocus);
	LRESULT OnSelChange(WPARAM wParam, LPARAM lParam);
	LRESULT OnOK(WPARAM wParam, LPARAM lParam);
};
