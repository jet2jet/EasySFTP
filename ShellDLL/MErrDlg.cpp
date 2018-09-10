/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 MErrDlg.cpp - implementations of CMultipleErrorDialog
 */

#include "StdAfx.h"
#include "ShellDLL.h"
#include "MErrDlg.h"

CMultipleErrorDialog::CMultipleErrorDialog(const CMyStringArrayW& astrMessages)
	: CMyDialog(IDD)
	, m_astrMessages(astrMessages)
{
}

CMultipleErrorDialog::~CMultipleErrorDialog()
{
}

bool CMultipleErrorDialog::OnInitDialog(HWND hWndFocus)
{
	::SendDlgItemMessage(m_hWnd, IDC_MSG_ICON, STM_SETICON, (WPARAM) ::LoadIcon(NULL, IDC_HAND), 0);

	int i;
	size_t nLen = 0;
	LPWSTR lpwBuffer, lpw;
	for (i = 0; i < m_astrMessages.GetCount(); i++)
	{
		if (i > 0)
			nLen += 2;
		nLen += wcslen(m_astrMessages.GetItem(i));
	}
	lpw = lpwBuffer = (LPWSTR) malloc(sizeof(WCHAR) * (nLen + 1));
	if (lpwBuffer)
	{
		for (i = 0; i < m_astrMessages.GetCount(); i++)
		{
			if (i > 0)
			{
				*lpw++ = L'\r';
				*lpw++ = L'\n';
			}
			register LPWSTR lpwSrc = m_astrMessages.GetItem(i);
			nLen = wcslen(lpwSrc);
			memcpy(lpw, lpwSrc, sizeof(WCHAR) * nLen);
			lpw += nLen;
		}
		::MySetWindowTextW(::GetDlgItem(m_hWnd, IDC_ERROR_LIST), lpwBuffer);
		free(lpwBuffer);
	}
	return true;
}
