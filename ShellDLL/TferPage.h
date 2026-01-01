/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 TferPage.h - declarations of CHostTransferPage
 */

#pragma once

#include "MyProp.h"

class CHostTransferPage : public CMyPropertyPage
{
public:
	CHostTransferPage(CEasySFTPHostSetting* pSettings, bool* pbResult);
	virtual ~CHostTransferPage(void);

	enum { IDD = IDD_PROP_TRANSFER };

protected:
	CEasySFTPHostSetting* m_pSettings;
	bool* m_pbResult;
	CMyStringArrayW m_arrTextFileType;

	virtual bool OnInitDialog(HWND hWndFocus);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnAddFileClicked(WPARAM wParam, LPARAM lParam);
	LRESULT OnDelFileClicked(WPARAM wParam, LPARAM lParam);
	LRESULT OnApply(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetActive(WPARAM wParam, LPARAM lParam);
};
