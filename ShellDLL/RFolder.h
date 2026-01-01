
#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Folder.h"
#include "Auth.h"
#include "Connect.h"

class CEasySFTPFolderRoot : public CFolderBase,
	public IDelegateFolder,
	public IPersistPropertyBag,
	public CDispatchImplNoUnknownT<IEasySFTPRoot2>,
	public IProvideClassInfo,
	public IEasySFTPOldRoot2,
	public IEasySFTPInternal
	//public IParentAndItem
{
public:
	CEasySFTPFolderRoot();
	~CEasySFTPFolderRoot();

public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override;
	STDMETHOD_(ULONG, AddRef)() override;
	STDMETHOD_(ULONG, Release)() override;

	virtual void* GetThisForDispatch() override
	{
		return static_cast<IEasySFTPRoot*>(this);
	}

	FORWARD_DISPATCH_IMPL_BASE_NO_UNKNOWN(CDispatchImplNoUnknownT)

	STDMETHOD(GetClassInfo)(ITypeInfo** ppTI) override;

	STDMETHOD(ParseDisplayName)(HWND hWnd, LPBC pbc, LPWSTR pszDisplayName,
		ULONG* pchEaten, PIDLIST_RELATIVE* ppidl, ULONG* pdwAttributes) override;
	STDMETHOD(EnumObjects)(HWND hWnd, SHCONTF grfFlags, IEnumIDList** ppenumIDList) override;
	STDMETHOD(BindToObject)(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void** ppv) override;
	STDMETHOD(BindToStorage)(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void** ppv) override;
	STDMETHOD(CompareIDs)(LPARAM lParam, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2) override;
	STDMETHOD(CreateViewObject)(HWND hWndOwner, REFIID riid, void** ppv) override;
	STDMETHOD(GetAttributesOf)(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, SFGAOF* rgfInOut) override;
	STDMETHOD(GetUIObjectOf)(HWND hWndOwner, UINT cidl, PCUITEMID_CHILD_ARRAY apidl,
		REFIID riid, UINT* rgfReserved, void** ppv) override;
	STDMETHOD(GetDisplayNameOf)(PCUITEMID_CHILD pidl, SHGDNF uFlags, STRRET* pName) override;
	STDMETHOD(SetNameOf)(HWND hWnd, PCUITEMID_CHILD pidl, LPCWSTR pszName, SHGDNF uFlags, PITEMID_CHILD* ppidlOut) override;

	STDMETHOD(GetDefaultSearchGUID)(GUID* pguid) override;
	STDMETHOD(EnumSearches)(IEnumExtraSearch** ppenum) override;
	STDMETHOD(GetDefaultColumn)(DWORD dwRes, ULONG* pSort, ULONG* pDisplay) override;
	STDMETHOD(GetDefaultColumnState)(UINT iColumn, SHCOLSTATEF* pcsFlags) override;
	STDMETHOD(GetDetailsEx)(PCUITEMID_CHILD pidl, const SHCOLUMNID* pscid, VARIANT* pv) override;
	STDMETHOD(GetDetailsOf)(PCUITEMID_CHILD pidl, UINT iColumn, SHELLDETAILS* psd) override;
	STDMETHOD(MapColumnToSCID)(UINT iColumn, SHCOLUMNID* pscid) override;

	// IPersist
public:
	STDMETHOD(GetClassID)(CLSID* pClassID) override;

	// IPersistFolder
public:
	//STDMETHOD(Initialize)(PCIDLIST_ABSOLUTE pidl);

//	// IPersistFolder2
//public:
//	STDMETHOD(GetCurFolder)(PIDLIST_ABSOLUTE FAR* ppidl);

	// IPersistPropertyBag
public:
	STDMETHOD(InitNew)() override;
	STDMETHOD(Load)(IPropertyBag* pPropBag, IErrorLog* pErrorLog) override;
	STDMETHOD(Save)(IPropertyBag* pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties) override;

	// IShellIcon
public:
	STDMETHOD(GetIconOf)(PCUITEMID_CHILD pidl, UINT flags, int* pIconIndex) override;

	// IDelegateFolder
public:
	STDMETHOD(SetItemAlloc)(IMalloc* pMalloc) override;

	// IEasySFTPRoot
public:
	STDMETHOD(get_Version)(BSTR* pRet) override;
	STDMETHOD(_Connect)(VARIANT_BOOL bSFTP, LONG_PTR hWnd, LONG_PTR pvReserved,
		BSTR lpszHostName, long nPort, IEasySFTPRootDirectory FAR* FAR* ppFolder) override;
	STDMETHOD(QuickConnectDialog)(LONG_PTR hWndOwner, IEasySFTPRootDirectory FAR* FAR* ppFolder) override;
	STDMETHOD(GetDependencyLibraryInfo)(BSTR FAR* poutLibraryInfo) override;
	STDMETHOD(Connect)(EasySFTPConnectionMode ConnectionMode, LONG_PTR hWnd, IEasySFTPAuthentication FAR* pAuth,
		BSTR lpszHostName, long nPort, VARIANT_BOOL bIgnoreFingerprint, IEasySFTPRootDirectory FAR* FAR* ppFolder) override;
	STDMETHOD(CreateAuthentication)(EasySFTPAuthenticationMode mode, BSTR bstrUserName, IEasySFTPAuthentication FAR* FAR* ppAuth) override;
	STDMETHOD(get_HostSettings)(IEasySFTPHostSettingList** ppList) override;
	STDMETHOD(CreateHostSetting)(IEasySFTPHostSetting** ppRet) override;
	STDMETHOD(ConnectFromSetting)(LONG_PTR hWnd, IEasySFTPHostSetting* pSetting, VARIANT_BOOL bIgnoreFingerprint, IEasySFTPDirectory** ppFolder) override;

	// IEasySFTPRoot2
public:
	STDMETHOD(ClearAllCredentials)() override;
	STDMETHOD(HasCredentials)(VARIANT_BOOL* pRet) override;
	STDMETHOD(AddLogger)(IEasySFTPLogger* Logger) override;
	STDMETHOD(RemoveLogger)(IEasySFTPLogger* Logger) override;

	// IEasySFTPOldRoot
public:
	//STDMETHOD(SetListener)(IEasySFTPListener* pListener);
	STDMETHOD(Connect)(VARIANT_BOOL bSFTP, HWND hWnd, const void* pvReserved,
		LPCWSTR lpszHostName, int nPort, IShellFolder FAR* FAR* ppFolder) override;
	STDMETHOD(QuickConnectDialog)(HWND hWndOwner, IShellFolder FAR* FAR* ppFolder) override;

	// IEasySFTPOldRoot2
public:
	//STDMETHOD(GetDependencyLibraryInfo)(BSTR FAR* poutLibraryInfo);

	// IEasySFTPInternal
public:
	STDMETHOD(SetEmulateRegMode)(bool bEmulate) override;

	// CFolderBase
public:
	STDMETHOD_(void, UpdateItem)(PCUITEMID_CHILD pidlOld, PCUITEMID_CHILD pidlNew, LONG lEvent) override;

public:
	bool ConnectDialog(HWND hWndOwner, IEasySFTPAuthentication* pUser);
	// <0: error (only if pszAuthList != NULL)
	//  0: cancelled
	// >0: succeeded
	int DoRetryAuthentication(HWND hWndOwner, IEasySFTPAuthentication* pUser,
		EasySFTPConnectionMode mode, const char* pszAuthList, bool bFirstAttempt);
	HRESULT _RetrieveDirectory(CFTPDirectoryRootBase* pRoot, LPCWSTR lpszPath, CFTPDirectoryBase** ppDirectory);
	HRESULT OpenWithConnectDialog(HWND hWndOwner, PCUITEMID_CHILD pidl, CFTPDirectoryRootBase** ppRoot);

	HRESULT AddHostSettings(CEasySFTPHostSetting* pSettings);
	HRESULT RemoveHostSettings(CEasySFTPHostSetting* pSettings);

protected:
	HRESULT _BindToObject(HWND hWndOwner, PCUITEMID_CHILD pidl, LPBC pbc, IEasySFTPAuthentication2* pUser, CFTPDirectoryRootBase** ppRoot);

	friend class CEnumRootItemIDList;
	friend class CEasySFTPRootIcon;
	friend class CEasySFTPRootMenu;

	ULONG m_uRef;
	//PIDLIST_ABSOLUTE m_pidlMe;
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
