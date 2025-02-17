#pragma once

// note: CFolderBase::GetDisplayNameOf must be implemented in case the 'pidl' is NULL
class DECLSPEC_NOVTABLE CFolderBase : public IShellFolder2,//public IShellFolder3,
	public IPersistFolder2,
	public IPersistIDList,
	public IObjectWithSite,
	public IShellIcon,
	public IShellFolderViewCB
{
public:
	CFolderBase(CDelegateMallocData* pMallocData);
	virtual ~CFolderBase();

public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
	STDMETHOD_(ULONG, AddRef)() = 0;
	STDMETHOD_(ULONG, Release)() = 0;
	void OnAddRef();
	void OnRelease();

	STDMETHOD(ParseDisplayName)(HWND hWnd, LPBC pbc, LPWSTR pszDisplayName,
		ULONG* pchEaten, PIDLIST_RELATIVE* ppidl, ULONG* pdwAttributes);
	//STDMETHOD(EnumObjects)(HWND hWnd, SHCONTF grfFlags, IEnumIDList** ppenumIDList) = 0;
	//STDMETHOD(BindToObject)(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void** ppv) = 0;
	//STDMETHOD(BindToStorage)(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void** ppv) = 0;
	//STDMETHOD(CompareIDs)(LPARAM lParam, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2) = 0;
	STDMETHOD(CreateViewObject)(HWND hWndOwner, REFIID riid, void** ppv);
	//STDMETHOD(GetAttributesOf)(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, SFGAOF* rgfInOut) = 0;
	//STDMETHOD(GetUIObjectOf)(HWND hWndOwner, UINT cidl, PCUITEMID_CHILD_ARRAY apidl,
	//	REFIID riid, UINT* rgfReserved, void** ppv) = 0;
	//STDMETHOD(GetDisplayNameOf)(PCUITEMID_CHILD pidl, SHGDNF uFlags, STRRET* pName) = 0;
	//STDMETHOD(SetNameOf)(HWND hWnd, PCUITEMID_CHILD pidl, LPCWSTR pszName, SHGDNF uFlags, PITEMID_CHILD* ppidlOut) = 0;

	//STDMETHOD(GetDefaultSearchGUID)(GUID* pguid);
	//STDMETHOD(EnumSearches)(IEnumExtraSearch** ppenum);
	//STDMETHOD(GetDefaultColumn)(DWORD dwRes, ULONG* pSort, ULONG* pDisplay);
	//STDMETHOD(GetDefaultColumnState)(UINT iColumn, SHCOLSTATEF* pcsFlags);
	//STDMETHOD(GetDetailsEx)(PCUITEMID_CHILD pidl, const SHCOLUMNID* pscid, VARIANT* pv);
	//STDMETHOD(GetDetailsOf)(PCUITEMID_CHILD pidl, UINT iColumn, SHELLDETAILS* psd);
	//STDMETHOD(MapColumnToSCID)(UINT iColumn, SHCOLUMNID* pscid);

	// IPersist
public:
	STDMETHOD(GetClassID)(CLSID* pClassID);

	// IPersistFolder
public:
	STDMETHOD(Initialize)(PCIDLIST_ABSOLUTE pidl);

	// IPersistFolder2
public:
	STDMETHOD(GetCurFolder)(PIDLIST_ABSOLUTE FAR* ppidl);

	// IPersistIDList
public:
	STDMETHOD(GetIDList)(PIDLIST_ABSOLUTE* ppidl);
	STDMETHOD(SetIDList)(PCIDLIST_ABSOLUTE pidl);

	// IObjectWithSite
public:
	STDMETHOD(SetSite)(IUnknown* pUnkSite);
	STDMETHOD(GetSite)(REFIID riid, void** ppvSite);

	// IShellIcon
public:
	//STDMETHOD(GetIconOf)(PCUITEMID_CHILD pidl, UINT flags, int* pIconIndex);

	// IShellFolderViewCB
public:
	STDMETHOD(MessageSFVCB)(UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	inline HWND GetHwndOwnerCache() const { return m_hWndOwnerCache; }
	inline IMalloc* GetDelegateMalloc() const { return m_pMallocData->pMalloc; }

	// utility methods
	void NotifyUpdate(LONG wEventId, PCUITEMID_CHILD pidlChild1, PCUITEMID_CHILD pidlChild2);
	void DefViewNotifyUpdate(LONG wEventId, PCUITEMID_CHILD pidlChild1, PCUITEMID_CHILD pidlChild2);
	IShellBrowser* GetShellBrowser(HWND hWndOwner);

protected:
	STDMETHOD(ParseDisplayName2)(PIDLIST_RELATIVE pidlParent, HWND hWnd, LPBC pbc,
		LPWSTR pszDisplayName, ULONG* pchEaten, PIDLIST_RELATIVE* ppidl, ULONG* pdwAttributes);

	STDMETHOD_(void, UpdateItem)(PCUITEMID_CHILD pidlOld, PCUITEMID_CHILD pidlNew, LONG lEvent)
	{ }

	virtual HRESULT InitializeParent() = 0;
	virtual IShellFolder* GetParentFolder() = 0;
	virtual HRESULT SetParentFolder(IShellFolder* pFolder) = 0;

public:
	PIDLIST_ABSOLUTE m_pidlMe;
	CDelegateMallocData* m_pMallocData;
protected:
	IUnknown* m_pUnkSite;
	HWND m_hWndOwnerCache;
	CMyPtrArrayPtrT<HWND> m_ahWndViews;
};
