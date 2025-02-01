/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 FldrMenu.h - declarations of CFTPFileItemMenu and CFTPFileDirectoryMenu
 */

#pragma once

class CFTPDirectoryBase;

////////////////////////////////////////////////////////////////////////////////

#include "Transfer.h"

#define FFIM_FILETYPE_DIRECTORY    0
#define FFIM_FILETYPE_FILEITEM     1
#define FFIM_FILETYPE_COMPLEX      -1

class CFTPFileItemMenu : public IContextMenu,
	public IObjectWithSite,
	public CTransferDialogListener
{
public:
	CFTPFileItemMenu(CFTPDirectoryBase* pParent, PCIDLIST_ABSOLUTE pidlMe,
		IShellBrowser* pBrowser, const CMyPtrArrayT<CFTPFileItem>& aItems);
	~CFTPFileItemMenu();

	// IUnknown
public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	// IContextMenu
public:
	STDMETHOD(QueryContextMenu)(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
	STDMETHOD(InvokeCommand)(CMINVOKECOMMANDINFO* pici);
	STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uType, UINT* pReserved, LPSTR pszName, UINT cchMax);

	// IObjectWithSite
public:
	STDMETHOD(SetSite)(IUnknown* pUnkSite);
	STDMETHOD(GetSite)(REFIID riid, void** ppvSite);

	// CTransferDialogListener
public:
	virtual void TransferCanceled(void* pvTransfer);

protected:
	// bExtend == true: explore for directory, open as text for item
	void DoOpen(HWND hWndOwner, bool bExtend, DWORD dwHotKey);
	void DoCut(HWND hWndOwner) { DoCutCopy(hWndOwner, true); }
	void DoCopy(HWND hWndOwner) { DoCutCopy(hWndOwner, false); }
	void DoCutCopy(HWND hWndOwner, bool bCut);
	void DoDelete(HWND hWndOwner);
	void DoProperty(HWND hWndOwner);

	void DownloadAndOpenFiles(HWND hWndOwner, bool bAsText, const CMyPtrArrayT<CFTPFileItem>& aItems);

protected:
	ULONG m_uRef;
	CFTPDirectoryBase* m_pParent;
	PIDLIST_ABSOLUTE m_pidlMe;
	IShellBrowser* m_pBrowser;
	CMyPtrArrayT<CFTPFileItem> m_aItems;
	char m_nFileTypes;
	IUnknown* m_pUnkSite;

	CTransferDialog m_dlgTransfer;
	CMyPtrArrayT<CTransferDialog::CTransferItem> m_aTransfers;
};

////////////////////////////////////////////////////////////////////////////////

class CFTPFileDirectoryMenu : public IContextMenu,
	public IObjectWithSite
{
public:
	CFTPFileDirectoryMenu(CFTPDirectoryBase* pParent, PCIDLIST_ABSOLUTE pidlMe, IShellBrowser* pBrowser);
	~CFTPFileDirectoryMenu();

	// IUnknown
public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	// IContextMenu
public:
	STDMETHOD(QueryContextMenu)(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
	STDMETHOD(InvokeCommand)(CMINVOKECOMMANDINFO* pici);
	STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uType, UINT* pReserved, LPSTR pszName, UINT cchMax);

	// IObjectWithSite
public:
	STDMETHOD(SetSite)(IUnknown* pUnkSite);
	STDMETHOD(GetSite)(REFIID riid, void** ppvSite);

protected:
	UINT _QueryContextMenu(HMENU hMenuTarget, HMENU hMenuCurrent, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);

	void DoCreateFolder(HWND hWndOwner);
	void DoCreateShortcut(HWND hWndOwner);
	void DoProperty(HWND hWndOwner);
	void DoPaste(HWND hWndOwner);

protected:
	ULONG m_uRef;
	CFTPDirectoryBase* m_pParent;
	PIDLIST_ABSOLUTE m_pidlMe;
	IShellBrowser* m_pBrowser;
	IUnknown* m_pUnkSite;
};
