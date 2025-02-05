/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 Folder.h - declarations of CFolderBase, CFTPDirectoryBase and folder-helper classes
 */

#pragma once

#include "FoldBase.h"
#include "DragData.h"
#include "SFilePrp.h"
#include "LinkDlg.h"
#include "SvInfo.h"

#define PID_FTPITEM_BASE           200
#define STRING_ID_TO_MY_PID(u)     (PID_FTPITEM_BASE + (u) - IDS_HEAD_FILE_NAME + 1)
#define MY_PID_TO_STRING_ID(u)     (IDS_HEAD_FILE_NAME - 1 + (u) - PID_FTPITEM_BASE)
#define PID_FTPITEM_FILE_NAME      STRING_ID_TO_MY_PID(IDS_HEAD_FILE_NAME)
#define PID_FTPITEM_SIZE           STRING_ID_TO_MY_PID(IDS_HEAD_SIZE)
#define PID_FTPITEM_TYPE           STRING_ID_TO_MY_PID(IDS_HEAD_TYPE)
#define PID_FTPITEM_MODIFY_TIME    STRING_ID_TO_MY_PID(IDS_HEAD_MODIFY_TIME)
#define PID_FTPITEM_PERMISSIONS    STRING_ID_TO_MY_PID(IDS_HEAD_PERMISSIONS)
#define PID_FTPITEM_TRANSFER_TYPE  STRING_ID_TO_MY_PID(IDS_HEAD_TRANSFER_TYPE)
#define COUNT_OF_PID_FTPITEM       6

class CFTPDirectoryBase;
class CFTPDirectoryRootBase;

constexpr int _GetAvailablePropKeyCount();
HRESULT __stdcall _GetFileItemPropData(CFTPDirectoryBase* pDirectory, CFTPFileItem* p, const PROPERTYKEY& key, VARIANT* pv);
void __stdcall FillFileItemInfo(CFTPFileItem* pItem);

////////////////////////////////////////////////////////////////////////////////

DECLARE_INTERFACE_IID_(IShellFolderPropertyInformation, IUnknown, "124bae2c-cb94-42cd-b5b8-4358789684ef")
{
	STDMETHOD(IsFastProperty)(THIS_ PCUITEMID_CHILD pidlChild, REFPROPERTYKEY pkey) PURE;
	STDMETHOD(GetFastProperties)(THIS_ PCUITEMID_CHILD pidlChild, REFIID riid, void** ppv) PURE;
};

////////////////////////////////////////////////////////////////////////////////

//class CFolderViewWrapper : public IShellView3
//{
//public:
//	CFolderViewWrapper(IShellView* pView);
//	~CFolderViewWrapper();
//
//	// IUnknown
//public:
//	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
//	STDMETHOD_(ULONG, AddRef)();
//	STDMETHOD_(ULONG, Release)();
//
//	// IOleWindow
//public:
//	STDMETHOD(GetWindow)(HWND* phWnd);
//	STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);
//
//	// IShellView
//public:
//	STDMETHOD(TranslateAccelerator)(MSG* pmsg);
//	STDMETHOD(EnableModeless)(BOOL fEnable);
//	STDMETHOD(UIActivate)(UINT uState);
//	STDMETHOD(Refresh)(void);
//	STDMETHOD(CreateViewWindow)(IShellView* psvPrevious, LPCFOLDERSETTINGS pfs,
//		IShellBrowser* psb, RECT* prcView, HWND* phWnd);
//	STDMETHOD(DestroyViewWindow)(void);
//	STDMETHOD(GetCurrentInfo)(LPFOLDERSETTINGS pfs);
//	STDMETHOD(AddPropertySheetPages)(DWORD dwReserved, LPFNSVADDPROPSHEETPAGE pfn, LPARAM lParam);
//	STDMETHOD(SaveViewState)(void);
//	STDMETHOD(SelectItem)(PCUITEMID_CHILD pidlItem, SVSIF uFlags);
//	STDMETHOD(GetItemObject)(UINT uItem, REFIID riid, void** ppv);
//
//	// IShellView2
//public:
//	STDMETHOD(GetView)(SHELLVIEWID* pvid, ULONG uView);
//	STDMETHOD(CreateViewWindow2)(LPSV2CVW2_PARAMS lpParams);
//	STDMETHOD(HandleRename)(PCUITEMID_CHILD pidlNew);
//	STDMETHOD(SelectAndPositionItem)(PCUITEMID_CHILD pidlItem, UINT uFlags, POINT* ppt);
//
//	// IShellView3
//public:
//	STDMETHOD(CreateViewWindow3)(IShellBrowser* psbOwner, IShellView* psvPrev,
//		SV3CVW3_FLAGS dwViewFlags, FOLDERFLAGS dwMask, FOLDERFLAGS dwFlags,
//		FOLDERVIEWMODE fvMode, const SHELLVIEWID* pvid, const RECT* prcView,
//		HWND* phWndView);
//
//public:
//	inline IShellBrowser* GetBrowser() const { return m_pBrowser; }
//
//protected:
//	ULONG m_uRef;
//	union
//	{
//		IShellView* m_pView;
//		IShellView2* m_pView2;
//		IShellView3* m_pView3;
//	};
//	BYTE m_nAvailableVersion;
//	IShellBrowser* m_pBrowser;
//};

////////////////////////////////////////////////////////////////////////////////

// struct CFTPDirectoryItem is defined in ShellDLL.h

class CFTPDirectoryBase : public CFolderBase,
	public IStorage,
	public IThumbnailHandlerFactory,
	public IShellFolderPropertyInformation,
	public IEasySFTPDirectory//,
	//public IFTPDataObjectListener
{
public:
	CFTPDirectoryBase(CDelegateMallocData* pMallocData,
		CFTPDirectoryItem* pItemMe,
		CFTPDirectoryBase* pParent,
		CFTPDirectoryRootBase* pRoot,
		LPCWSTR lpszDirectory);
	virtual ~CFTPDirectoryBase();

	// IUnknown
public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	// IShellFolder
public:
	//STDMETHOD(ParseDisplayName)(HWND hWnd, LPBC pbc, LPWSTR pszDisplayName,
	//	ULONG* pchEaten, PIDLIST_RELATIVE* ppidl, ULONG* pdwAttributes);
	STDMETHOD(EnumObjects)(HWND hWnd, SHCONTF grfFlags, IEnumIDList** ppenumIDList);
	STDMETHOD(BindToObject)(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void** ppv);
	STDMETHOD(BindToStorage)(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void** ppv);
	STDMETHOD(CompareIDs)(LPARAM lParam, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2);
	STDMETHOD(CreateViewObject)(HWND hWndOwner, REFIID riid, void** ppv);
	STDMETHOD(GetAttributesOf)(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, SFGAOF* rgfInOut);
	STDMETHOD(GetUIObjectOf)(HWND hWndOwner, UINT cidl, PCUITEMID_CHILD_ARRAY apidl,
		REFIID riid, UINT* rgfReserved, void** ppv);
	STDMETHOD(GetDisplayNameOf)(PCUITEMID_CHILD pidl, SHGDNF uFlags, STRRET* pName);
	STDMETHOD(SetNameOf)(HWND hWnd, PCUITEMID_CHILD pidl, LPCOLESTR pszName, SHGDNF uFlags, PITEMID_CHILD* ppidlOut);

	STDMETHOD(GetDefaultSearchGUID)(GUID* pguid);
	STDMETHOD(EnumSearches)(IEnumExtraSearch** ppenum);
	STDMETHOD(GetDefaultColumn)(DWORD dwRes, ULONG* pSort, ULONG* pDisplay);
	STDMETHOD(GetDefaultColumnState)(UINT iColumn, SHCOLSTATEF* pcsFlags);
	STDMETHOD(GetDetailsEx)(PCUITEMID_CHILD pidl, const SHCOLUMNID* pscid, VARIANT* pv);
	STDMETHOD(GetDetailsOf)(PCUITEMID_CHILD pidl, UINT iColumn, SHELLDETAILS* psd);
	STDMETHOD(MapColumnToSCID)(UINT iColumn, SHCOLUMNID* pscid);

	// IShellIcon
public:
	STDMETHOD(GetIconOf)(PCUITEMID_CHILD pidl, UINT flags, int* pIconIndex);

	// IShellFolderViewCB
public:
	STDMETHOD(MessageSFVCB)(UINT uMsg, WPARAM wParam, LPARAM lParam);

	// IThumbnailHandlerFactory
public:
	STDMETHOD(GetThumbnailHandler)(PCUITEMID_CHILD pidl, LPBC pbc, REFIID riid, void** ppv);

	// IShellFolderPropertyInformation
public:
	STDMETHOD(IsFastProperty)(PCUITEMID_CHILD pidlChild, REFPROPERTYKEY pkey);
	STDMETHOD(GetFastProperties)(PCUITEMID_CHILD pidlChild, REFIID riid, void** ppv);

	// IEasySFTPDirectory
public:
	STDMETHOD(GetRootDirectory)(IEasySFTPDirectory** ppRootDirectory);
	STDMETHOD(GetHostInfo)(VARIANT_BOOL FAR* pbIsSFTP, int FAR* pnPort, BSTR FAR* pbstrHostName);
	STDMETHOD(GetTextMode)(LONG FAR* pnTextMode);
	STDMETHOD(SetTextMode)(LONG nTextMode);
	STDMETHOD(GetTransferMode)(LONG FAR* pnTransferMode);
	STDMETHOD(SetTransferMode)(LONG nTransferMode);
	STDMETHOD(IsTextFile)(LPCWSTR lpszFileName);
	STDMETHOD(Disconnect)();
	STDMETHOD(IsConnected)();
	STDMETHOD(IsTransferring)();

	// IStorage
public:
	STDMETHOD(CreateStream)(const OLECHAR* pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStream** ppstm);
	STDMETHOD(OpenStream)(const OLECHAR* pwcsName, void* reserved1, DWORD grfMode, DWORD reserved2, IStream** ppstm);
	STDMETHOD(CreateStorage)(const OLECHAR* pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStorage** ppstg);
	STDMETHOD(OpenStorage)(const OLECHAR* pwcsName, IStorage* pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage** ppstg);
	STDMETHOD(CopyTo)(DWORD ciidExclude, const IID* rgiidExclude, SNB snbExclude, IStorage* pstgDest);
	STDMETHOD(MoveElementTo)(const OLECHAR* pwcsName, IStorage* pstgDest, const OLECHAR* pwcsNewName, DWORD grfFlags);
	STDMETHOD(Commit)(DWORD grfCommitFlags);
	STDMETHOD(Revert)();
	STDMETHOD(EnumElements)(DWORD reserved1, void* reserved2, DWORD reserved3, IEnumSTATSTG** ppenum);
	STDMETHOD(DestroyElement)(const OLECHAR* pwcsName);
	STDMETHOD(RenameElement)(const OLECHAR* pwcsOldName, const OLECHAR* pwcsNewName);
	STDMETHOD(SetElementTimes)(const OLECHAR* pwcsName, const FILETIME* pctime, const FILETIME* patime, const FILETIME* pmtime);
	STDMETHOD(SetClass)(REFCLSID clsid);
	STDMETHOD(SetStateBits)(DWORD grfStateBits, DWORD grfMask);
	STDMETHOD(Stat)(STATSTG* pstatstg, DWORD grfStatFlag);

	//// IFTPDataObjectListener
public:
	HRESULT CreateStream(CFTPFileItem* pItem, IStream** ppStream);
	HRESULT DeleteFTPItem(CFTPFileItem* pItem);
	void AfterPaste(CFTPDataObject* pObject, DWORD dwEffects);

public:
	STDMETHOD(CreateInstance)(CFTPDirectoryItem* pItemMe, CFTPDirectoryBase* pParent, CFTPDirectoryRootBase* pRoot,
		LPCWSTR lpszDirectory, CFTPDirectoryBase** ppResult) = 0;
	STDMETHOD(ParseDisplayName2)(PIDLIST_RELATIVE pidlParent, HWND hWnd, LPBC pbc,
		LPWSTR pszDisplayName, ULONG* pchEaten, PIDLIST_RELATIVE* ppidl, ULONG* pdwAttributes);
	STDMETHOD_(void, UpdateItem)(PCUITEMID_CHILD pidlOld, PCUITEMID_CHILD pidlNew, LONG lEvent);
	STDMETHOD_(void, UpdateItem)(CFTPFileItem* pOldItem, LPCWSTR lpszNewItem, LONG lEvent);
	STDMETHOD_(IShellFolder*, GetParentFolder)() { return m_pParent; }

public:
	CFTPDirectoryBase* m_pParent;
	CFTPDirectoryRootBase* m_pRoot;
	CLSID m_clsidThis;
	DWORD m_grfMode;
	DWORD m_grfStateBits;
	bool m_bIsRoot;
	// the absolute path
	CMyStringW m_strDirectory;
	CMyPtrArrayT<CFTPFileItem> m_aFiles;
	CMyPtrArrayT<CFTPDirectoryItem> m_aDirectories;

protected:
	CFTPDirectoryBase(CDelegateMallocData* pMallocData, CFTPDirectoryItem* pItemMe);
	void CommonConstruct();
	void RemoveAllFiles();

	CRITICAL_SECTION m_csRefs;
	ULONG m_uRef;
public:
	CRITICAL_SECTION m_csFiles;
protected:
	CFTPDirectoryItem* m_pItemMe;
	bool m_bDirReceived;

public:
	CFTPFileItem* GetFileItem(LPCWSTR lpszName) const;
	CFTPFileItem* GetFileItem(LPCWSTR lpszRelativeName, CFTPDirectoryBase** ppParentDirectory);
	CFTPFileItem* GetFileItem(PCUIDLIST_RELATIVE pidlChild, CFTPDirectoryBase** ppParentDirectory);

public:
	// utility methods
	HRESULT OpenNewDirectory(LPCWSTR lpszRelativePath, CFTPDirectoryBase** ppDirectory);
	CFTPDirectoryItem* GetAlreadyOpenedDirectory(PCUIDLIST_RELATIVE pidlChild);
	inline void NotifyUpdate(LONG wEventId, PCUITEMID_CHILD pidlChild1, PCUITEMID_CHILD pidlChild2)
	{
		CFolderBase::NotifyUpdate(wEventId, pidlChild1, pidlChild2);
	}
	void NotifyUpdate(LONG wEventId, LPCWSTR lpszFile1, LPCWSTR lpszFile2);

	void UpdateNewFile(LPCWSTR lpszFileName, bool bDirectory);
	void UpdateMoveFile(LPCWSTR lpszFromDir, LPCWSTR lpszFileName, bool bDirectory, LPCWSTR lpszNewFileName = NULL);
	void UpdateRenameFile(LPCWSTR lpszOldFileName, LPCWSTR lpszNewFileName, bool bDirectory);
	void UpdateFileAttrs(LPCWSTR lpszFileName, bool bDirectory);
	void UpdateRemoveFile(LPCWSTR lpszFileName, bool bDirectory);
	inline bool IsDirectoryReceived() const { return m_bDirReceived; }
	bool DoReceiveDirectory();
	inline bool IsDirectoryItem(LPCWSTR lpszFileName) const
	{
		auto* pItem = GetFileItem(lpszFileName);
		return pItem != NULL && pItem->IsDirectory();
	}

	HRESULT CopyFileItemToStorage(CFTPFileItem* pFile, DWORD ciidExclude, const IID* rgiidExclude, SNB snbExclude, IStorage* pstgDest);
};
