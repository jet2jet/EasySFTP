/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 FoldItem.h - declarations of CFolderShellItem and folder-item-helper classes
 */

#pragma once

class CFolderBase;

////////////////////////////////////////////////////////////////////////////////

class CFolderShellItem : public IShellItem,
	public IPersistIDList
{
public:
	CFolderShellItem(CFolderBase* pParent, PCUITEMID_CHILD pidlChild);
	virtual ~CFolderShellItem();

	// IUnknown
public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	// IShellItem
public:
	STDMETHOD(BindToHandler)(IBindCtx* pbc, REFGUID bhid, REFIID riid, void** ppv);
	STDMETHOD(GetParent)(IShellItem** ppsi);
	STDMETHOD(GetDisplayName)(SIGDN sigdnName, LPWSTR* ppszName);
	STDMETHOD(GetAttributes)(SFGAOF sfgaoMask, SFGAOF* psfgaoAttribs);
	STDMETHOD(Compare)(IShellItem* psi, SICHINTF hint, int* piOrder);

	// IPersist
public:
	STDMETHOD(GetClassID)(CLSID* pClassID);

	// IPersistIDList
public:
	STDMETHOD(GetIDList)(PIDLIST_ABSOLUTE* ppidl);
	STDMETHOD(SetIDList)(PCIDLIST_ABSOLUTE pidl);

protected:
	ULONG m_uRef;
	CFolderBase* m_pParent;
	PITEMID_CHILD m_pidlChild;
};

class CEnumFolderShellItem : public IEnumShellItems
{
public:
	CEnumFolderShellItem(CFolderBase* pTarget);
	virtual ~CEnumFolderShellItem();

	// IUnknown
public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	// IEnumShellItems
public:
	STDMETHOD(Next)(ULONG celt, IShellItem** rgelt, ULONG* pceltFetched);
	STDMETHOD(Skip)(ULONG celt);
	STDMETHOD(Reset)();
	STDMETHOD(Clone)(IEnumShellItems** ppEnum);

protected:
	ULONG m_uRef;
	CFolderBase* m_pTarget;
	IEnumIDList* m_pEnumIDList;

	HRESULT InitializeEnumIDList();
};
