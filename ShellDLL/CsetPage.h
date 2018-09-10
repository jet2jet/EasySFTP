/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 CsetPage.h - declarations of CHostCharsetPage
 */

#pragma once

#include "MyProp.h"

class CHostCharsetPage : public CMyPropertyPage
{
public:
	CHostCharsetPage(CHostSettings* pSettings, bool* pbResult);
	virtual ~CHostCharsetPage();

	enum { IDD = IDD_PROP_CHARSET };

protected:
	CHostSettings* m_pSettings;
	bool* m_pbResult;

	virtual bool OnInitDialog(HWND hWndFocus);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnApply(WPARAM wParam, LPARAM lParam);
};
