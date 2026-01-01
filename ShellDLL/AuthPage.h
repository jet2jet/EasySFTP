/*
 EasySFTP - Copyright (C) 2025 jet (ジェット)

 AuthPage.h - declarations of CHostAuthPage
 */

#pragma once

#include "MyProp.h"

class CHostGeneralSettingPage;

class CHostAuthPage : public CMyPropertyPage
{
public:
	CHostAuthPage(CEasySFTPHostSetting* pSettings, CHostGeneralSettingPage* pHostSettingPage, bool* pbResult);
	virtual ~CHostAuthPage();

	enum { IDD = IDD_PROP_AUTH };

protected:
	CEasySFTPHostSetting* m_pSettings;
	CHostGeneralSettingPage* m_pHostSettingPage;
	bool* m_pbResult;
	CMyStringW m_strDummyPassword;
	CMyStringW m_strDummyPKey;

	void UpdateControlEnable();
	bool ConfirmPasswordChanged();

	virtual bool OnInitDialog(HWND hWndFocus);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnAutoLoginChecked(WPARAM wParam, LPARAM lParam);
	LRESULT OnAuthTypeChecked(WPARAM wParam, LPARAM lParam);
	LRESULT OnPasswordKillFocus(WPARAM wParam, LPARAM lParam);
	LRESULT OnSearchPKeyFile(WPARAM wParam, LPARAM lParam);
	LRESULT OnApply(WPARAM wParam, LPARAM lParam);
};
