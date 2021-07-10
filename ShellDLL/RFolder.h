
#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Folder.h"
#include "Auth.h"
#include "Connect.h"

class CEasySFTPFolderRoot : public CFolderBase,
	public IDelegateFolder,
	public IEasySFTPRoot2,
	public IEasySFTPInternal
	//public IParentAndItem
{
public:
	CEasySFTPFolderRoot();
	~CEasySFTPFolderRoot();

public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	STDMETHOD(ParseDisplayName)(HWND hWnd, LPBC pbc, LPWSTR pszDisplayName,
		ULONG* pchEaten, PIDLIST_RELATIVE* ppidl, ULONG* pdwAttributes);
	STDMETHOD(EnumObjects)(HWND hWnd, SHCONTF grfFlags, IEnumIDList** ppenumIDList);
	STDMETHOD(BindToObject)(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void** ppv);
	STDMETHOD(BindToStorage)(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void** ppv);
	STDMETHOD(CompareIDs)(LPARAM lParam, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2);
	STDMETHOD(CreateViewObject)(HWND hWndOwner, REFIID riid, void** ppv);
	STDMETHOD(GetAttributesOf)(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, SFGAOF* rgfInOut);
	STDMETHOD(GetUIObjectOf)(HWND hWndOwner, UINT cidl, PCUITEMID_CHILD_ARRAY apidl,
		REFIID riid, UINT* rgfReserved, void** ppv);
	STDMETHOD(GetDisplayNameOf)(PCUITEMID_CHILD pidl, SHGDNF uFlags, STRRET* pName);
	STDMETHOD(SetNameOf)(HWND hWnd, PCUITEMID_CHILD pidl, LPCWSTR pszName, SHGDNF uFlags, PITEMID_CHILD* ppidlOut);

	STDMETHOD(GetDefaultSearchGUID)(GUID* pguid);
	STDMETHOD(EnumSearches)(IEnumExtraSearch** ppenum);
	STDMETHOD(GetDefaultColumn)(DWORD dwRes, ULONG* pSort, ULONG* pDisplay);
	STDMETHOD(GetDefaultColumnState)(UINT iColumn, SHCOLSTATEF* pcsFlags);
	STDMETHOD(GetDetailsEx)(PCUITEMID_CHILD pidl, const SHCOLUMNID* pscid, VARIANT* pv);
	STDMETHOD(GetDetailsOf)(PCUITEMID_CHILD pidl, UINT iColumn, SHELLDETAILS* psd);
	STDMETHOD(MapColumnToSCID)(UINT iColumn, SHCOLUMNID* pscid);

	// IPersist
public:
	STDMETHOD(GetClassID)(CLSID* pClassID);

	// IPersistFolder
public:
	//STDMETHOD(Initialize)(PCIDLIST_ABSOLUTE pidl);

//	// IPersistFolder2
//public:
//	STDMETHOD(GetCurFolder)(PIDLIST_ABSOLUTE FAR* ppidl);

	// IShellIcon
public:
	STDMETHOD(GetIconOf)(PCUITEMID_CHILD pidl, UINT flags, int* pIconIndex);

	// IDelegateFolder
public:
	STDMETHOD(SetItemAlloc)(IMalloc* pMalloc);

	// IShellItem (re-define)
public:
	STDMETHOD(GetParent)(IShellItem** ppsi);

	// IParentAndItem (re-define)
public:
	STDMETHOD(SetParentAndItem)(PCIDLIST_ABSOLUTE pidlParent, IShellFolder* psf, PCUITEMID_CHILD pidlChild);
	STDMETHOD(GetParentAndItem)(PIDLIST_ABSOLUTE* ppidlParent, IShellFolder** ppsf, PITEMID_CHILD* ppidlChild);

	// IEasySFTPRoot
public:
	//STDMETHOD(SetListener)(IEasySFTPListener* pListener);
	STDMETHOD(Connect)(VARIANT_BOOL bSFTP, HWND hWnd, const void* pvReserved,
		LPCWSTR lpszHostName, int nPort, IShellFolder FAR* FAR* ppFolder);
	STDMETHOD(QuickConnectDialog)(HWND hWndOwner, IShellFolder FAR* FAR* ppFolder);

	// IEasySFTPRoot2
public:
	STDMETHOD(GetDependencyLibraryInfo)(BSTR FAR* poutLibraryInfo);

	// IEasySFTPInternal
public:
	STDMETHOD(SetEmulateRegMode)(bool bEmulate);

	// CFolderBase
public:
	STDMETHOD_(void, UpdateItem)(PCUITEMID_CHILD pidlOld, PCUITEMID_CHILD pidlNew, LONG lEvent);
	STDMETHOD_(IShellFolder*, GetParentFolder)() { return m_pFolderParent; }

public:
	bool ConnectDialog(HWND hWndOwner, CUserInfo* pUser);
	// <0: error (only if pszAuthList != NULL)
	//  0: cancelled
	// >0: succeeded
	int DoRetryAuthentication(HWND hWndOwner, CUserInfo* pUser,
		bool bSFTPMode, const char* pszAuthList, bool bFirstAttempt);
	HRESULT _RetrieveDirectory(CFTPDirectoryRootBase* pRoot, LPCWSTR lpszPath, CFTPDirectoryBase** ppDirectory);
	HRESULT OpenWithConnectDialog(HWND hWndOwner, PCUITEMID_CHILD pidl, CFTPDirectoryRootBase** ppRoot);

protected:
	HRESULT _BindToObject(HWND hWndOwner, PCUITEMID_CHILD pidl, LPBC pbc, CUserInfo* pUser, CFTPDirectoryRootBase** ppRoot);

	friend class CEnumRootItemIDList;
	friend class CEasySFTPRootIcon;
	friend class CEasySFTPRootMenu;

	ULONG m_uRef;
	//PIDLIST_ABSOLUTE m_pidlMe;
	IShellFolder* m_pFolderParent;
	CConnectDialog m_dlgConnect;

public:
	//IEasySFTPListener* m_pListener;
};

////////////////////////////////////////////////////////////////////////////////

class CEasySFTPRootIcon : public IExtractIconW, public IExtractIconA
{
public:
	CEasySFTPRootIcon(int iIndex);
	~CEasySFTPRootIcon();

	// IUnknown
public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	// IExtractIconW
public:
	STDMETHOD(GetIconLocation)(UINT uFlags, LPWSTR pszIconFile, UINT cchMax, int* piIndex, UINT* pwFlags);
	STDMETHOD(Extract)(LPCWSTR pszFile, UINT nIconIndex, HICON* phIconLarge, HICON* phIconSmall, UINT nIconSize);

	// IExtractIconA
public:
	STDMETHOD(GetIconLocation)(UINT uFlags, LPSTR pszIconFile, UINT cchMax, int* piIndex, UINT* pwFlags);
	STDMETHOD(Extract)(LPCSTR pszFile, UINT nIconIndex, HICON* phIconLarge, HICON* phIconSmall, UINT nIconSize);

protected:
	HRESULT DoExtract(HICON* phIconLarge, HICON* phIconSmall, UINT nIconSize);

	ULONG m_uRef;
	int m_iIndex;
};

////////////////////////////////////////////////////////////////////////////////

class CEasySFTPRootMenu : public IContextMenu,
	public IObjectWithSite
{
public:
	CEasySFTPRootMenu(CEasySFTPFolderRoot* pRoot, IShellBrowser* pBrowser, PCIDLIST_ABSOLUTE pidlMe, PCUITEMID_CHILD_ARRAY apidl, int nCount);
	~CEasySFTPRootMenu();

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
	void DoAdd(HWND hWndOwner);
	void DoProperty(HWND hWndOwner);
	void DoDelete(HWND hWndOwner);

	PIDLIST_ABSOLUTE RetrieveDefaultDirectory(HWND hWndOwner, PCUIDLIST_RELATIVE pidl, LPCWSTR* lplpszLocalDirectory);

protected:
	ULONG m_uRef;
	CEasySFTPFolderRoot* m_pRoot;
	IShellBrowser* m_pBrowser;
	PIDLIST_ABSOLUTE m_pidlMe;
	PIDLIST_RELATIVE* m_apidl;
	int m_nCount;
	IUnknown* m_pUnkSite;
	WORD m_wIDCommand;
};
