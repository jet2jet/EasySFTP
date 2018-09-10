/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 LFileVw.h - declarations of CShellFolderFileView
 */

#pragma once

class CShellFolderFileView : public CMyWindow
{
public:
	CShellFolderFileView(void);
	virtual ~CShellFolderFileView(void);

	HWND Create(IShellBrowser* pBrowser, HWND hWndParent);
	HWND Create(PCIDLIST_ABSOLUTE lpItemID, IShellFolder* pFolder, IShellBrowser* pBrowser, HWND hWndParent);

public:
	IShellBrowser* m_pBrowser;
	IShellFolder* m_pFolder;
	IShellView* m_pView;
	PIDLIST_ABSOLUTE m_lpidlAbsoluteMe;
	FOLDERSETTINGS m_fs;
	IEasySFTPDirectory* m_pDirectory;

	bool m_bFocused;
	bool m_bReplacing;

	void OnFocusView(bool bFocus);

	HRESULT ReplaceView(PCUIDLIST_RELATIVE lpItemID);
	HRESULT ReplaceViewAbsolute(PCUIDLIST_ABSOLUTE lpItemID);
	HRESULT ReplaceViewAbsolute(PCUIDLIST_ABSOLUTE lpItemID, IShellFolder* pFolder);
	void Refresh();
	bool SendCommandString(HWND hWndBrowser, UINT uItem, LPCWSTR lpszCommand);
	void DoCreateNewFolder(HWND hWndBrowser);
	void DoOpen(HWND hWndBrowser);
	void DoCreateShortcut(HWND hWndBrowser);
	void DoCopy(HWND hWndBrowser, bool bCut);
	void DoPaste(HWND hWndBrowser);
	void DoDelete(HWND hWndBrowser);
	void DoRename(HWND hWndBrowser);
	void DoProperty(HWND hWndBrowser);
	void DoSelectAll(HWND hWndBrowser);
	void DoInvertSelection(HWND hWndBrowser);

	int GetSelectionCount();
	PIDLIST_ABSOLUTE GetSelectedItem(int iIndex);
	PIDLIST_ABSOLUTE* GetAllSelection(int* pnCount);

	void ReleaseAll();

protected:
	HRESULT ReplaceView(IShellFolder* pFolder);
};
