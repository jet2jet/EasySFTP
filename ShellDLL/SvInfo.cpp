/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 SvInfo.cpp - implementations of CServerInfoDialog
 */

#include "StdAfx.h"
#include "ShellDLL.h"
#include "SvInfo.h"

CServerInfoDialog::CServerInfoDialog(void)
	: CMyDialog(IDD)
{
}

CServerInfoDialog::~CServerInfoDialog(void)
{
}

bool CServerInfoDialog::OnInitDialog(HWND hWndFocus)
{
	::MySetWindowTextW(::GetDlgItem(m_hWnd, IDC_INFO), m_strInfo);
	return true;
}
