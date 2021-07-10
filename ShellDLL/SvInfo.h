/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 SvInfo.h - declarations of CServerInfoDialog
 */

#pragma once

#include "MyDialog.h"

class CServerInfoDialog :
	public CMyDialog
{
public:
	CServerInfoDialog(void);
	virtual ~CServerInfoDialog(void);

	enum { IDD = IDD_SERVER_INFO };

	CMyStringW m_strInfo;

protected:
	virtual bool OnInitDialog(HWND hWndFocus);
};
