/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 MErrDlg.h - declarations of CMultipleErrorDialog
 */

#pragma once

#include "MyDialog.h"
#include "Array.h"

class CMultipleErrorDialog :
	public CMyDialog
{
public:
	CMultipleErrorDialog(const CMyStringArrayW& astrMessages);
	virtual ~CMultipleErrorDialog();

	enum { IDD = IDD_FILENAME };

protected:
	virtual bool OnInitDialog(HWND hWndFocus);

	CMyStringArrayW m_astrMessages;
};
