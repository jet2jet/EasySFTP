/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 Folder.h - declarations of CFolderBase, CFTPDirectoryBase and folder-helper classes
 */

#pragma once

////////////////////////////////////////////////////////////////////////////////

// note: CFolderBase::GetDisplayNameOf must be implemented in case the 'pidl' is NULL
class CFolderBase : public IShellFolder2,//public IShellFolder3,
	public IPersistFolder2,
	public IShellItem,
	public IParentAndItem,
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

	// IShellItem
public:
	STDMETHOD(BindToHandler)(IBindCtx* pbc, REFGUID bhid, REFIID riid, void** ppv);
	STDMETHOD(GetParent)(IShellItem** ppsi);
	STDMETHOD(GetDisplayName)(SIGDN sigdnName, LPWSTR* ppszName);
	STDMETHOD(GetAttributes)(SFGAOF sfgaoMask, SFGAOF* psfgaoAttribs);
	STDMETHOD(Compare)(IShellItem* psi, SICHINTF hint, int* piOrder);

	// IParentAndItem
public:
	STDMETHOD(SetParentAndItem)(PCIDLIST_ABSOLUTE pidlParent, IShellFolder* psf, PCUITEMID_CHILD pidlChild);
	STDMETHOD(GetParentAndItem)(PIDLIST_ABSOLUTE* ppidlParent, IShellFolder** ppsf, PITEMID_CHILD* ppidlChild);

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

	STDMETHOD_(IShellFolder*, GetParentFolder)() { return m_pFolderParent; }

public:
	PIDLIST_ABSOLUTE m_pidlMe;
	CDelegateMallocData* m_pMallocData;
protected:
	IUnknown* m_pUnkSite;
	HWND m_hWndOwnerCache;
	CMyPtrArrayPtrT<HWND> m_ahWndViews;
	IShellFolder* m_pFolderParent;
	IShellItem* m_pItemParent;

	friend class CFolderShellItem;
};

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

class CEnumFTPItemIDList : public CUnknownImplT<IEnumIDList>
{
public:
	CEnumFTPItemIDList(CDelegateMallocData* pMallocData, const CMyPtrArrayT<CFTPFileItem>& arrItems, SHCONTF grfFlags, IUnknown* pUnkOuter);
	virtual ~CEnumFTPItemIDList();

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);

	STDMETHOD(Next)(ULONG celt, PITEMID_CHILD* rgelt, ULONG* pceltFetched);
	STDMETHOD(Skip)(ULONG celt);
	STDMETHOD(Reset)();
	STDMETHOD(Clone)(IEnumIDList** ppEnum);

private:
	CDelegateMallocData* m_pMallocData;
	CMyPtrArrayT<CFTPFileItem> m_arrItems;
	SHCONTF m_grfFlags;
	ULONG m_uPos;
	IUnknown* m_pUnkOuter;
};

////////////////////////////////////////////////////////////////////////////////

class __declspec(novtable) CTransferStatus
{
public:
	virtual void TransferInProgress(void* pvObject, ULONGLONG uliPosition) = 0;
	virtual bool TransferIsCanceled(void* pvObject) = 0;
};

////////////////////////////////////////////////////////////////////////////////

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

// struct CFTPDirectoryItem is defined in ShellDLL.h

class CFTPDirectoryBase : public CFolderBase,
	public IThumbnailHandlerFactory,
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

	//// IFTPDataObjectListener
public:
	HRESULT CreateStream(CFTPFileItem* pItem, IStream** ppStream);
	void DeleteFTPItem(CFTPFileItem* pItem);
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

protected:
	CFTPFileItem* GetFileItem(LPCWSTR lpszName) const;

public:
	// utility methods
	HRESULT OpenNewDirectory(LPCWSTR lpszRelativePath, CFTPDirectoryBase** ppDirectory);
	CFTPDirectoryItem* GetAlreadyOpenedDirectory(PCUIDLIST_RELATIVE pidlChild);
	inline void NotifyUpdate(LONG wEventId, PCUITEMID_CHILD pidlChild1, PCUITEMID_CHILD pidlChild2)
		{ CFolderBase::NotifyUpdate(wEventId, pidlChild1, pidlChild2); }
	void NotifyUpdate(LONG wEventId, LPCWSTR lpszFile1, LPCWSTR lpszFile2);

	void UpdateNewFile(LPCWSTR lpszFileName, bool bDirectory);
	void UpdateMoveFile(LPCWSTR lpszFromDir, LPCWSTR lpszFileName, bool bDirectory);
	void UpdateRenameFile(LPCWSTR lpszOldFileName, LPCWSTR lpszNewFileName, bool bDirectory);
	void UpdateFileAttrs(LPCWSTR lpszFileName, bool bDirectory);
	void UpdateRemoveFile(LPCWSTR lpszFileName, bool bDirectory);
	inline bool IsDirectoryReceived() const { return m_bDirReceived; }
	bool DoReceiveDirectory();
};

class CFTPDirectoryRootBase : public CFTPDirectoryBase
{
public:
	CFTPDirectoryRootBase(CDelegateMallocData* pMallocData, CFTPDirectoryItem* pItemMe);

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
	STDMETHOD(GetFTPItemUIObjectOf)(HWND hWndOwner, CFTPDirectoryBase* pDirectory,
		const CMyPtrArrayT<CFTPFileItem>& aItems, CFTPDataObject** ppObject) = 0;
	STDMETHOD(SetFTPItemNameOf)(HWND hWnd, CFTPDirectoryBase* pDirectory,
		CFTPFileItem* pItem, LPCWSTR pszName, SHGDNF uFlags) = 0;
	STDMETHOD(DoDeleteFTPItems)(HWND hWndOwner, CFTPDirectoryBase* pDirectory,
		const CMyPtrArrayT<CFTPFileItem>& aItems) = 0;
	// lpszFileNames ::= L"<file-name>\0<file-name>\0 ... <file-name>\0\0"
	STDMETHOD(MoveFTPItems)(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszFromDir, LPCWSTR lpszFileNames) = 0;
	// sizeof(pabResults) == sizeof(bool) * aItems.GetCount()
	STDMETHOD(UpdateFTPItemAttributes)(HWND hWndOwner, CFTPDirectoryBase* pDirectory,
		CServerFilePropertyDialog* pDialog, const CMyPtrArrayT<CServerFileAttrData>& aAttrs,
		bool* pabResults) = 0;
	STDMETHOD(CreateFTPDirectory)(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszName) = 0;
	STDMETHOD(CreateShortcut)(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszName, LPCWSTR lpszLinkTo, bool bHardLink)
		{ return E_NOTIMPL; }
	STDMETHOD(CreateFTPItemStream)(CFTPDirectoryBase* pDirectory, CFTPFileItem* pItem, IStream** ppStream) = 0;
	STDMETHOD(WriteFTPItem)(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszName, IStream* pStream,
		void* pvObject, CTransferStatus* pStatus) = 0;
	//STDMETHOD(CreateInstance)(CFTPDirectoryItem* pItemMe, CFTPDirectoryBase* pParent, CFTPDirectoryRootBase* pRoot,
	//	LPCWSTR lpszDirectory, CFTPDirectoryBase** ppResult) = 0;
	//STDMETHOD(CreateViewObject)(HWND hWndOwner, REFIID riid, void** ppv);
	STDMETHOD(SetFileTime)(CFTPDirectoryBase* pDirectory, LPCWSTR lpszFileName,
		const FILETIME* pftModifyTime) { return E_NOTIMPL; }

	STDMETHOD(GetHostInfo)(VARIANT_BOOL FAR* pbIsSFTP, int FAR* pnPort, BSTR FAR* pbstrHostName);
	STDMETHOD(GetTextMode)(LONG FAR* pnTextMode);
	STDMETHOD(SetTextMode)(LONG nTextMode);
	STDMETHOD(GetTransferMode)(LONG FAR* pnTransferMode);
	STDMETHOD(SetTransferMode)(LONG nTransferMode);
	STDMETHOD(IsTextFile)(LPCWSTR lpszFileName);
	STDMETHOD(Disconnect)() = 0;
	STDMETHOD(IsConnected)() = 0;
	STDMETHOD(IsTransferring)() = 0;

	virtual LPCWSTR GetProtocolName(int& nDefPort) const = 0;
	virtual void PreShowPropertyDialog(CServerFilePropertyDialog* pDialog) { }
	// pDialog may be NULL
	// return: whether the protocol supports creation of shortcut (link)
	virtual bool PreShowCreateShortcutDialog(CLinkDialog* pDialog) { return false; }
	virtual void PreShowServerInfoDialog(CServerInfoDialog* pDialog) { }
	virtual bool ReceiveDirectory(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszDirectory, bool* pbReceived) = 0;
	virtual bool ValidateDirectory(LPCWSTR lpszParentDirectory, PCUIDLIST_RELATIVE pidlChild,
		CMyStringW& rstrRealPath) = 0;
	virtual CFTPFileItem* RetrieveFileItem(CFTPDirectoryBase* pDirectory, LPCWSTR lpszFileName) = 0;

	//void BeforeClipboardOperation(IDataObject* pObjectNew);
	//void AfterClipboardOperation(IDataObject* pObjectNew);
	void ShowServerInfoDialog(HWND hWndOwner);

protected:
	virtual HRESULT DoDeleteFileOrDirectory(HWND hWndOwner, CMyStringArrayW& astrMsgs, bool bIsDirectory, LPCWSTR lpszFile, CFTPDirectoryBase* pDirectory = NULL) = 0;
	HRESULT DoDeleteDirectoryRecursive(HWND hWndOwner, CMyStringArrayW& astrMsgs, LPCWSTR lpszName, CFTPDirectoryBase* pDirectory);

public:
	CMyStringW m_strHostName;
	int m_nPort;
	BYTE m_bTextMode;
	char m_nServerCharset;
	// TRANSFER_MODE_XXX
	char m_nTransferMode;
	CMyStringArrayW m_arrTextFileType;
	bool m_bUseSystemTextFileType;
	bool m_bAdjustRecvModifyTime;
	bool m_bAdjustSendModifyTime;
	bool m_bUseThumbnailPreview;
	IDataObject* m_pObjectOnClipboard;
};

class CFTPFileItemIcon : public IExtractIconW, public IExtractIconA
{
public:
	CFTPFileItemIcon(CFTPFileItem* pItem);
	CFTPFileItemIcon(LPCWSTR lpszFileName, bool bIsDirectory);
	~CFTPFileItemIcon();

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
	HRESULT DoExtract(bool bOpenIcon, HICON* phIconLarge, HICON* phIconSmall, UINT nIconSize);

	ULONG m_uRef;
	CMyStringW m_strFileName;
	bool m_bIsDirectory;
	int m_iIconIndex;
	int m_iOpenIconIndex;
	//CFTPFileItem* m_pItem;
};

////////////////////////////////////////////////////////////////////////////////

inline STDMETHODIMP CFTPDirectoryBase::GetHostInfo(VARIANT_BOOL FAR* pbIsSFTP, int FAR* pnPort, BSTR FAR* pbstrHostName)
	{ return m_pRoot->GetHostInfo(pbIsSFTP, pnPort, pbstrHostName); }
inline STDMETHODIMP CFTPDirectoryBase::GetTextMode(LONG FAR* pnTextMode)
	{ return m_pRoot->GetTextMode(pnTextMode); }
inline STDMETHODIMP CFTPDirectoryBase::SetTextMode(LONG nTextMode)
	{ return m_pRoot->SetTextMode(nTextMode); }
inline STDMETHODIMP CFTPDirectoryBase::GetTransferMode(LONG FAR* pnTransferMode)
	{ return m_pRoot->GetTransferMode(pnTransferMode); }
inline STDMETHODIMP CFTPDirectoryBase::SetTransferMode(LONG nTransferMode)
	{ return m_pRoot->SetTransferMode(nTransferMode); }
inline STDMETHODIMP CFTPDirectoryBase::IsTextFile(LPCWSTR lpszFileName)
	{ return m_pRoot->IsTextFile(lpszFileName); }
inline STDMETHODIMP CFTPDirectoryBase::Disconnect()
	{ return m_pRoot->Disconnect(); }
inline STDMETHODIMP CFTPDirectoryBase::IsConnected()
	{ return m_pRoot->IsConnected(); }
inline STDMETHODIMP CFTPDirectoryBase::IsTransferring()
	{ return m_pRoot->IsTransferring(); }
inline bool CFTPDirectoryBase::DoReceiveDirectory()
	{ return m_bDirReceived ? true : m_pRoot->ReceiveDirectory(m_hWndOwnerCache, this, m_strDirectory, &m_bDirReceived); }
