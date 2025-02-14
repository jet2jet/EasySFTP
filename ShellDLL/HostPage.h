/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 HostPage.h - declarations of CHostGeneralSettingPage
 */

#pragma once

#include "MyProp.h"

class CHostGeneralSettingPage : public CMyPropertyPage
{
public:
	CHostGeneralSettingPage(CEasySFTPHostSetting* pSettings, bool* pbResult);
	virtual ~CHostGeneralSettingPage();

	enum { IDD = IDD_PROP_HOST };

	bool m_bNoModeChange;

protected:
	CEasySFTPHostSetting* m_pSettings;
	bool* m_pbResult;

	virtual bool OnInitDialog(HWND hWndFocus);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnDefPortClicked(WPARAM wParam, LPARAM lParam);
	LRESULT OnConnectModeChanged(WPARAM wParam, LPARAM lParam);
	LRESULT OnLocalPathSearch(WPARAM wParam, LPARAM lParam);
	LRESULT OnApply(WPARAM wParam, LPARAM lParam);
	LRESULT OnKillActive(WPARAM wParam, LPARAM lParam);
};
