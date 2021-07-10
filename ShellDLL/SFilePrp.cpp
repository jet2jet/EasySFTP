/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 SFilePrp.cpp - implementations of CServerFilePropertyDialog
 */

#include "StdAfx.h"
#include "ShellDLL.h"
#include "SFilePrp.h"

CServerFilePropertyDialog::CServerFilePropertyDialog(const CMyPtrArrayT<CFTPFileItem>& aFiles)
	: CMyDialog(IDD)
{
	for (int i = 0; i < aFiles.GetCount(); i++)
	{
		CFTPFileItem* pItem = aFiles.GetItem(i);
		pItem->AddRef();
		CServerFileAttrData* pAttr = new CServerFileAttrData();
		pAttr->pItem = pItem;
		pAttr->nUnixMode = pItem->nUnixMode;
		pAttr->strOwner = pItem->strOwner;
		pAttr->strGroup = pItem->strGroup;
		pAttr->uUID = pItem->uUID;
		pAttr->uGID = pItem->uGID;
		m_aAttrs.Add(pAttr);
	}
}

CServerFilePropertyDialog::~CServerFilePropertyDialog()
{
	for (int i = 0; i < m_aAttrs.GetCount(); i++)
	{
		CServerFileAttrData* pAttr = m_aAttrs.GetItem(i);
		pAttr->pItem->Release();
		delete pAttr;
	}
}

// in Folder.cpp
extern void __stdcall FileTimeToString(const FILETIME* pft, CMyStringW& ret);
extern void __stdcall FileSizeToString(ULARGE_INTEGER uli, CMyStringW& ret);

static void __stdcall Set3StateCheck(HWND hDlg, int nIDDlgItem, bool bIndeterminate, bool bChecked)
{
	if (bIndeterminate)
	{
		::SendDlgItemMessage(hDlg, nIDDlgItem, BM_SETSTYLE, (WPARAM) BS_AUTO3STATE, MAKELPARAM(FALSE, 0));
		::CheckDlgButton(hDlg, nIDDlgItem, BST_INDETERMINATE);
	}
	else
		::CheckDlgButton(hDlg, nIDDlgItem, bChecked ? BST_CHECKED : BST_UNCHECKED);
}

bool CServerFilePropertyDialog::OnInitDialog(HWND hWndFocus)
{
	CMyStringW str;
	CServerFileAttrData* pFirst;
	if (m_aAttrs.GetCount() == 1)
		pFirst = m_aAttrs.GetItem(0);
	else
		pFirst = NULL;
	if (!pFirst)
		str.LoadString(IDS_MULTI_SELECT);
	else
		str = pFirst->pItem->strFileName;
	::SyncDialogData(m_hWnd, IDC_FILE_NAME, str, false);
	::SyncDialogData(m_hWnd, IDC_DIRECTORY, m_strDirectory, false);
	if (!pFirst)
		str.LoadString(IDS_MULTI_SELECT);
	else
	{
		if (!pFirst->pItem->ftCreateTime.dwHighDateTime && !pFirst->pItem->ftCreateTime.dwLowDateTime)
			str.LoadString(IDS_UNAVAILABLE);
		else
			::FileTimeToString(&pFirst->pItem->ftCreateTime, str);
	}
	::SyncDialogData(m_hWnd, IDC_CREATION_TIME, str, false);
	if (!pFirst)
		str.LoadString(IDS_MULTI_SELECT);
	else
	{
		if (!pFirst->pItem->ftModifyTime.dwHighDateTime && !pFirst->pItem->ftModifyTime.dwLowDateTime)
			str.LoadString(IDS_UNAVAILABLE);
		else
			::FileTimeToString(&pFirst->pItem->ftModifyTime, str);
	}
	::SyncDialogData(m_hWnd, IDC_MODIFY_TIME, str, false);

	{
		ULARGE_INTEGER uli;
		uli.QuadPart = 0;
		for (int i = 0; i < m_aAttrs.GetCount(); i++)
			uli.QuadPart += m_aAttrs.GetItem(i)->pItem->uliSize.QuadPart;
		::FileSizeToString(uli, str);
		if (!pFirst)
		{
			CMyStringW str2;
			str2.LoadString(IDS_TOTAL);
			str.InsertString(str2, 0);
		}
		::SyncDialogData(m_hWnd, IDC_FILE_SIZE, str, false);
	}
	{
		UINT u;
		str.Empty();
		for (int i = 0; i < m_aAttrs.GetCount(); i++)
		{
			pFirst = m_aAttrs.GetItem(i);
			if (str.IsEmpty())
			{
				if (m_bSupportedName)
					str = pFirst->strOwner;
				else
				{
					u = pFirst->uUID;
					str.Format(L"%u", u);
				}
			}
			else
			{
				if (m_bSupportedName)
				{
					if (str.Compare(pFirst->strOwner, true) != 0)
					{
						str.Empty();
						break;
					}
				}
				else
				{
					if (u != pFirst->uUID)
					{
						str.Empty();
						break;
					}
				}
			}
		}
		::SyncDialogData(m_hWnd, IDC_OWNER, str, false);
		str.Empty();
		for (int i = 0; i < m_aAttrs.GetCount(); i++)
		{
			pFirst = m_aAttrs.GetItem(i);
			if (str.IsEmpty())
			{
				if (m_bSupportedName)
					str = pFirst->strGroup;
				else
				{
					u = pFirst->uGID;
					str.Format(L"%u", u);
				}
			}
			else
			{
				if (m_bSupportedName)
				{
					if (str.Compare(pFirst->strGroup, true) != 0)
					{
						str.Empty();
						break;
					}
				}
				else
				{
					if (u != pFirst->uGID)
					{
						str.Empty();
						break;
					}
				}
			}
		}
		::SyncDialogData(m_hWnd, IDC_GROUP, str, false);
		if (!m_bSupportedName)
		{
			register HWND h = ::GetDlgItem(m_hWnd, IDC_OWNER);
			::SetWindowLong(h, GWL_STYLE, ::GetWindowLong(h, GWL_STYLE) | ES_NUMBER);
			h = ::GetDlgItem(m_hWnd, IDC_GROUP);
			::SetWindowLong(h, GWL_STYLE, ::GetWindowLong(h, GWL_STYLE) | ES_NUMBER);
		}
	}
	if (!m_bChangeOwner)
	{
		::SendDlgItemMessage(m_hWnd, IDC_OWNER, EM_SETREADONLY, (WPARAM) TRUE, 0);
		::SendDlgItemMessage(m_hWnd, IDC_GROUP, EM_SETREADONLY, (WPARAM) TRUE, 0);
	}

	if (m_bChangeAttr)
	{
		UINT nMode;
		UINT nComplexMode = 0;
		nMode = (UINT) -1;
		for (int i = 0; i < m_aAttrs.GetCount(); i++)
		{
			pFirst = m_aAttrs.GetItem(i);
			register UINT nFMode = (UINT) pFirst->nUnixMode & 0777;
			if (nMode != (UINT) -1)
			{
				// 相違のある部分のフラグを加える
				if (nMode != nFMode)
					nComplexMode |= nMode ^ nFMode;
			}
			nMode = nFMode;
		}
		Set3StateCheck(m_hWnd, IDC_USER_READ, (nComplexMode & S_IRUSR) != 0, (nMode & S_IRUSR) != 0);
		Set3StateCheck(m_hWnd, IDC_USER_WRITE, (nComplexMode & S_IWUSR) != 0, (nMode & S_IWUSR) != 0);
		Set3StateCheck(m_hWnd, IDC_USER_EXECUTE, (nComplexMode & S_IXUSR) != 0, (nMode & S_IXUSR) != 0);
		Set3StateCheck(m_hWnd, IDC_GROUP_READ, (nComplexMode & S_IRGRP) != 0, (nMode & S_IRGRP) != 0);
		Set3StateCheck(m_hWnd, IDC_GROUP_WRITE, (nComplexMode & S_IWGRP) != 0, (nMode & S_IWGRP) != 0);
		Set3StateCheck(m_hWnd, IDC_GROUP_EXECUTE, (nComplexMode & S_IXGRP) != 0, (nMode & S_IXGRP) != 0);
		Set3StateCheck(m_hWnd, IDC_OTHER_READ, (nComplexMode & S_IROTH) != 0, (nMode & S_IROTH) != 0);
		Set3StateCheck(m_hWnd, IDC_OTHER_WRITE, (nComplexMode & S_IWOTH) != 0, (nMode & S_IWOTH) != 0);
		Set3StateCheck(m_hWnd, IDC_OTHER_EXECUTE, (nComplexMode & S_IXOTH) != 0, (nMode & S_IXOTH) != 0);
	}
	else
	{
		::EnableDlgItem(m_hWnd, IDC_USER_READ, FALSE);
		::EnableDlgItem(m_hWnd, IDC_USER_WRITE, FALSE);
		::EnableDlgItem(m_hWnd, IDC_USER_EXECUTE, FALSE);
		::EnableDlgItem(m_hWnd, IDC_GROUP_READ, FALSE);
		::EnableDlgItem(m_hWnd, IDC_GROUP_WRITE, FALSE);
		::EnableDlgItem(m_hWnd, IDC_GROUP_EXECUTE, FALSE);
		::EnableDlgItem(m_hWnd, IDC_OTHER_READ, FALSE);
		::EnableDlgItem(m_hWnd, IDC_OTHER_WRITE, FALSE);
		::EnableDlgItem(m_hWnd, IDC_OTHER_EXECUTE, FALSE);
	}

	return true;
}

LRESULT CServerFilePropertyDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE_COMMAND(IDOK, OnOK);
	return CMyDialog::WindowProc(message, wParam, lParam);
}

LRESULT CServerFilePropertyDialog::OnOK(WPARAM wParam, LPARAM lParam)
{
	UINT nMode, nComplexMode;
	int i;
	UINT uUID, uGID;
	CMyStringW strOwner, strGroup;
	if (m_bChangeOwner)
	{
		::SyncDialogData(m_hWnd, IDC_OWNER, strOwner, true);
		::SyncDialogData(m_hWnd, IDC_GROUP, strGroup, true);
		{
			DWORD dw;
			bool bNoEmpty = (m_aAttrs.GetCount() == 1);
			if ((bNoEmpty && strOwner.IsEmpty()) ||
				(!strOwner.IsEmpty() && !m_bSupportedName && !::GetDWordFromStringW(strOwner, dw)))
			{
				::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(m_bSupportedName ? IDS_NO_OWNER : IDS_NO_OWNER_ID), NULL, MB_ICONEXCLAMATION);
				::SetDlgItemFocus(m_hWnd, IDC_OWNER);
				return 0;
			}
			else if (!strOwner.IsEmpty() && !m_bSupportedName)
				uUID = (UINT) dw;
			if ((bNoEmpty && strGroup.IsEmpty()) ||
				(!strGroup.IsEmpty() && !m_bSupportedName && !::GetDWordFromStringW(strGroup, dw)))
			{
				::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(m_bSupportedName ? IDS_NO_GROUP : IDS_NO_GROUP_ID), NULL, MB_ICONEXCLAMATION);
				::SetDlgItemFocus(m_hWnd, IDC_GROUP);
				return 0;
			}
			else if (!strGroup.IsEmpty() && !m_bSupportedName)
				uGID = (UINT) dw;
		}
	}
	if (m_bChangeAttr)
	{
		nMode = nComplexMode = 0;
		switch (::IsDlgButtonChecked(m_hWnd, IDC_USER_READ))
		{
			case BST_INDETERMINATE: nComplexMode |= S_IRUSR; break;
			case BST_CHECKED: nMode |= S_IRUSR; break;
		}
		switch (::IsDlgButtonChecked(m_hWnd, IDC_USER_WRITE))
		{
			case BST_INDETERMINATE: nComplexMode |= S_IWUSR; break;
			case BST_CHECKED: nMode |= S_IWUSR; break;
		}
		switch (::IsDlgButtonChecked(m_hWnd, IDC_USER_EXECUTE))
		{
			case BST_INDETERMINATE: nComplexMode |= S_IXUSR; break;
			case BST_CHECKED: nMode |= S_IXUSR; break;
		}
		switch (::IsDlgButtonChecked(m_hWnd, IDC_GROUP_READ))
		{
			case BST_INDETERMINATE: nComplexMode |= S_IRGRP; break;
			case BST_CHECKED: nMode |= S_IRGRP; break;
		}
		switch (::IsDlgButtonChecked(m_hWnd, IDC_GROUP_WRITE))
		{
			case BST_INDETERMINATE: nComplexMode |= S_IWGRP; break;
			case BST_CHECKED: nMode |= S_IWGRP; break;
		}
		switch (::IsDlgButtonChecked(m_hWnd, IDC_GROUP_EXECUTE))
		{
			case BST_INDETERMINATE: nComplexMode |= S_IXGRP; break;
			case BST_CHECKED: nMode |= S_IXGRP; break;
		}
		switch (::IsDlgButtonChecked(m_hWnd, IDC_OTHER_READ))
		{
			case BST_INDETERMINATE: nComplexMode |= S_IROTH; break;
			case BST_CHECKED: nMode |= S_IROTH; break;
		}
		switch (::IsDlgButtonChecked(m_hWnd, IDC_OTHER_WRITE))
		{
			case BST_INDETERMINATE: nComplexMode |= S_IWOTH; break;
			case BST_CHECKED: nMode |= S_IWOTH; break;
		}
		switch (::IsDlgButtonChecked(m_hWnd, IDC_OTHER_EXECUTE))
		{
			case BST_INDETERMINATE: nComplexMode |= S_IXOTH; break;
			case BST_CHECKED: nMode |= S_IXOTH; break;
		}
	}

	bool bOChanged = false, bAChanged = false;
	for (i = 0; i < m_aAttrs.GetCount(); i++)
	{
		CServerFileAttrData* pItem = m_aAttrs.GetItem(i);
		if (m_bChangeOwner)
		{
			if (m_bSupportedName)
			{
				if (!strOwner.IsEmpty() && pItem->strOwner.Compare(strOwner, true) != 0)
				{
					bOChanged = true;
					pItem->strOwner = strOwner;
				}
				if (!strGroup.IsEmpty() && pItem->strGroup.Compare(strGroup, true) != 0)
				{
					bOChanged = true;
					pItem->strGroup = strGroup;
				}
			}
			else
			{
				if (!strOwner.IsEmpty() && pItem->uUID != uUID)
				{
					bOChanged = true;
					pItem->uUID = uUID;
				}
				if (!strGroup.IsEmpty() && pItem->uGID != uGID)
				{
					bOChanged = true;
					pItem->uGID = uGID;
				}
			}
		}
		if (m_bChangeAttr)
		{
			if (nComplexMode != 0777)
			{
				register UINT u = (UINT) pItem->nUnixMode;
				u &= nComplexMode | ~0777;
				u |= nMode;
				if ((UINT) pItem->nUnixMode != u)
				{
					bAChanged = true;
					pItem->nUnixMode = (int) u;
				}
			}
		}
	}
	m_bChangeOwner = bOChanged;
	m_bChangeAttr = bAChanged;
	return CMyDialog::OnDefaultButton(wParam, lParam);
}
