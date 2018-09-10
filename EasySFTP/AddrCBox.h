/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 AddrCBox.cpp - declarations of CAddressComboBox and CVirtualAddressComboBox
 */

#pragma once

class CAddressComboBox :
	public CMyWindow
{
public:
	CAddressComboBox(void);
	virtual ~CAddressComboBox(void);

	HWND Create(int x, int y, int cx, int cy, HWND hWndParent, UINT uID);

	void ChangeCurrentFolder(PCIDLIST_ABSOLUTE lpidl);
	void NotifyChange(WPARAM wParam, LPARAM lParam);
	PCIDLIST_ABSOLUTE GetSelectedFolder() const;
	PCIDLIST_ABSOLUTE FindItemFromDisplayName(LPCWSTR lpszDisplayName) const;
	void RestoreTextBox();
	// return true if the parent window should operate some command
	bool HandleEndEdit(int iWhy, bool bChanged, LPCWSTR lpszText, HWND hWndNextFocus);

public:
	IShellFolder* m_pFolderRoot;
	IShellFolder* m_pFolder;
	HIMAGELIST m_himlSystemSmall;

protected:
	CMyStringW m_strRealPath;
	bool m_bUseDisplayName;
	bool m_bUnicode;
	int m_nCurSel;

	void FillData(void* pData) const;
	void DeleteData(void* pData) const;
	void UpdateRealPath(LPCWSTR lpszRealPath);
	void* GetItemData(int nIndex) const;

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnDeleteItem(WPARAM wParam, LPARAM lParam);
	LRESULT OnGetDispInfoA(WPARAM wParam, LPARAM lParam);
	LRESULT OnGetDispInfoW(WPARAM wParam, LPARAM lParam);
	LRESULT OnBeginEdit(WPARAM wParam, LPARAM lParam);
};
