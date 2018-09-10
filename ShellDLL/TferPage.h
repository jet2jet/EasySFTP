/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 TferPage.h - declarations of CHostTransferPage
 */

#pragma once

#include "MyProp.h"

class CHostTransferPage : public CMyPropertyPage
{
public:
	CHostTransferPage(CHostSettings* pSettings, bool* pbResult);
	virtual ~CHostTransferPage(void);

	enum { IDD = IDD_PROP_TRANSFER };

protected:
	CHostSettings* m_pSettings;
	bool* m_pbResult;
	CMyStringArrayW m_arrTextFileType;

	virtual bool OnInitDialog(HWND hWndFocus);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnAddFileClicked(WPARAM wParam, LPARAM lParam);
	LRESULT OnDelFileClicked(WPARAM wParam, LPARAM lParam);
	LRESULT OnConnectModeChanged(WPARAM wParam, LPARAM lParam);
	LRESULT OnLocalPathSearch(WPARAM wParam, LPARAM lParam);
	LRESULT OnApply(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetActive(WPARAM wParam, LPARAM lParam);
};
