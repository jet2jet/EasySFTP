/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 Folder.h - declarations of CFolderBase, CFTPDirectoryBase and folder-helper classes
 */

#pragma once

#include "FoldBase.h"
#include "DragData.h"
#include "SFilePrp.h"
#include "LinkDlg.h"
#include "SvInfo.h"
#include "RefDelg.h"

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

// in ShellDLL.cpp
ITypeInfo* GetTypeInfo(const GUID& guid);

////////////////////////////////////////////////////////////////////////////////

DECLARE_INTERFACE_IID_(IShellFolderPropertyInformation, IUnknown, "124bae2c-cb94-42cd-b5b8-4358789684ef")
{
	STDMETHOD(IsFastProperty)(THIS_ PCUITEMID_CHILD pidlChild, REFPROPERTYKEY pkey) PURE;
	STDMETHOD(GetFastProperties)(THIS_ PCUITEMID_CHILD pidlChild, REFIID riid, void** ppv) PURE;
};

////////////////////////////////////////////////////////////////////////////////

// struct CFTPDirectoryItem is defined in ShellDLL.h

class CFTPDirectoryBase : public CFolderBase,
	public IProvideClassInfo,
	public IStorage,
	public IThumbnailHandlerFactory,
	public IShellFolderPropertyInformation,
	public IEasySFTPDirectorySynchronization,
	public IEasySFTPOldDirectory//,
	//public IFTPDataObjectListener
{
	friend class CFTPDirectoryRootBase;
public:
	CFTPDirectoryBase(CDelegateMallocData* pMallocData,
		CFTPDirectoryItem* pItemMe,
		CFTPDirectoryBase* pParent,
		LPCWSTR lpszDirectory);
	virtual ~CFTPDirectoryBase();

	virtual ULONG DetachAndRelease();

	// IUnknown
public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override;
	STDMETHOD_(ULONG, AddRef)() override;
	STDMETHOD_(ULONG, Release)() override;

	// IProvideClassInfo
public:
	STDMETHOD(GetClassInfo)(ITypeInfo** ppTI) override;

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
#define STDMETHODNOVIRTUAL(n) COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE n
	STDMETHODNOVIRTUAL(get_Name)(BSTR* pRet);
	STDMETHODNOVIRTUAL(get_RootDirectory)(IEasySFTPRootDirectory** ppRootDirectory);
	STDMETHODNOVIRTUAL(OpenDirectory)(VARIANT file, IEasySFTPDirectory** ppRet);
	STDMETHODNOVIRTUAL(get_Files)(IEasySFTPFiles** ppFiles);
	STDMETHODNOVIRTUAL(OpenTransferDialog)(INT_PTR hWndOwner);
	STDMETHODNOVIRTUAL(CloseTransferDialog)();
	STDMETHODNOVIRTUAL(OpenFile)(BSTR lpszRelativeFileName, VARIANT_BOOL bIsWrite, EasySFTPTextMode nTextMode, IEasySFTPStream** ppStream);
	STDMETHODNOVIRTUAL(UploadFrom)(BSTR lpszDestinationRelativeName, BSTR lpszSourceLocalName, EasySFTPTextMode nTextMode);
	STDMETHODNOVIRTUAL(UploadFromStream)(BSTR lpszDestinationRelativeFileName, IUnknown* pStream, EasySFTPTextMode nTextMode);
	STDMETHODNOVIRTUAL(UploadFromDataObject)(IUnknown* pObject, EasySFTPTextMode nTextMode);
	STDMETHODNOVIRTUAL(DownloadTo)(VARIANT file, BSTR lpszTargetLocalName, EasySFTPTextMode nTextMode);
	STDMETHODNOVIRTUAL(DownloadToStream)(VARIANT file, IUnknown* pStream, EasySFTPTextMode nTextMode);
	STDMETHODNOVIRTUAL(UploadFiles)(SAFEARRAY* LocalFiles, EasySFTPTextMode nTextMode);
	STDMETHODNOVIRTUAL(DownloadFiles)(SAFEARRAY* RemoteFiles, BSTR bstrDestinationDirectory, EasySFTPTextMode nTextMode);
	STDMETHODNOVIRTUAL(Move)(VARIANT file, BSTR lpszTargetName);
	STDMETHODNOVIRTUAL(Remove)(VARIANT file);
	STDMETHODNOVIRTUAL(UpdateFileTime)(VARIANT file, DATE modifyTime, DATE createTime, DATE accessTime = 0);
	STDMETHODNOVIRTUAL(UpdateAttributes)(VARIANT file, long attr);
	STDMETHODNOVIRTUAL(CreateShortcut)(BSTR LinkName, BSTR TargetName);
	STDMETHODNOVIRTUAL(get_FullPath)(BSTR* pRet);
	STDMETHODNOVIRTUAL(get_Url)(BSTR* pRet);

	STDMETHODNOVIRTUAL(SynchronizeFrom)(LONG_PTR hWndOwner, BSTR bstrSourceDirectory, EasySFTPSynchronizeMode Flags);
	STDMETHODNOVIRTUAL(SynchronizeDirectoryFrom)(LONG_PTR hWndOwner, IEasySFTPDirectory* pSourceDirectory, EasySFTPSynchronizeMode Flags);
	STDMETHODNOVIRTUAL(SynchronizeFolderFrom)(LONG_PTR hWndOwner, IUnknown* pSourceShellFolder, EasySFTPSynchronizeMode Flags);
	STDMETHODNOVIRTUAL(SynchronizeTo)(LONG_PTR hWndOwner, BSTR bstrTargetDirectory, EasySFTPSynchronizeMode Flags);
	STDMETHODNOVIRTUAL(SynchronizeDirectoryTo)(LONG_PTR hWndOwner, IEasySFTPDirectory* pTargetDirectory, EasySFTPSynchronizeMode Flags);
	STDMETHODNOVIRTUAL(SynchronizeFolderTo)(LONG_PTR hWndOwner, IUnknown* pTargetShellFolder, EasySFTPSynchronizeMode Flags);

	// IEasySFTPOldDirectory
public:
	STDMETHOD(GetRootDirectory)(IEasySFTPOldDirectory** ppRootDirectory);
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
	HRESULT OpenStream2(const OLECHAR* pwcsName, DWORD grfMode, BYTE bTextMode, IStream** ppstm, IEasySFTPFile** ppFile = NULL);
	HRESULT DeleteFTPItem(CFTPFileItem* pItem);
	void AfterPaste(CFTPDataObject* pObject, DWORD dwEffects);
	void MakeUrl(const CMyStringW& strFileName, CMyStringW& rstrUrl, bool* pbLastIsNotDelimiter = NULL);

public:
	STDMETHOD(CreateInstance)(CFTPDirectoryItem* pItemMe, CFTPDirectoryBase* pParent,
		LPCWSTR lpszDirectory, CFTPDirectoryBase** ppResult) = 0;
	STDMETHOD(ParseDisplayName2)(PIDLIST_RELATIVE pidlParent, HWND hWnd, LPBC pbc,
		LPWSTR pszDisplayName, ULONG* pchEaten, PIDLIST_RELATIVE* ppidl, ULONG* pdwAttributes);
	STDMETHOD_(void, UpdateItem)(PCUITEMID_CHILD pidlOld, PCUITEMID_CHILD pidlNew, LONG lEvent);
	STDMETHOD_(void, UpdateItem)(CFTPFileItem* pOldItem, LPCWSTR lpszNewItem, LONG lEvent);

	inline CFTPDirectoryBase* GetParent() const { return m_pParent; }
	CFTPDirectoryRootBase* GetRoot();

public:
	CFTPDirectoryBase* m_pParent;
	CLSID m_clsidThis;
	ULONG m_uRef;
	DWORD m_grfMode;
	DWORD m_grfStateBits;
	bool m_bIsRoot;
	bool m_bInDetachAndRelease;
	bool m_bPendingDelete;
	// the absolute path
	CMyStringW m_strDirectory;
	CMyPtrArrayT<CFTPFileItem> m_aFiles;
	CMyPtrArrayT<CFTPDirectoryItem> m_aDirectories;

protected:
	CFTPDirectoryBase(CDelegateMallocData* pMallocData, CFTPDirectoryItem* pItemMe, ITypeInfo* pInfo);
	void CommonConstruct();
	bool DetachImpl();

public:
	CRITICAL_SECTION m_csFiles;
protected:
	CRITICAL_SECTION m_csRefs;
	CFTPDirectoryItem* m_pItemMe;
	bool m_bDirReceived;

public:
	CFTPFileItem* GetFileItem(LPCWSTR lpszName) const;
	CFTPFileItem* GetFileItem(LPCWSTR lpszRelativeName, CFTPDirectoryBase** ppParentDirectory);
	CFTPFileItem* GetFileItem(PCUIDLIST_RELATIVE pidlChild, CFTPDirectoryBase** ppParentDirectory);

public:
	// utility methods
	HRESULT OpenNewDirectory(LPCWSTR lpszRelativePath, CFTPDirectoryBase** ppDirectory);
	CFTPDirectoryItem* GetAlreadyOpenedDirectory(PCUIDLIST_RELATIVE pidlChild, PCUIDLIST_RELATIVE* ppidlNext);
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

	virtual IEasySFTPDirectory* GetThisDirectory() = 0;
};

template <class T>
class CFTPDirectoryT : public CFTPDirectoryBase, public CMultipleDispatchImplBase, public T
{
private:
	const CMultipleDispatchImplBase::Entry s_DispatchEntries[3] = {
		{ ::GetTypeInfo(IID_IEasySFTPDirectory), &IID_IEasySFTPDirectory },
		{ ::GetTypeInfo(IID_IEasySFTPDirectorySynchronization), &IID_IEasySFTPDirectorySynchronization },
		{ NULL, NULL }
	};
public:
	CFTPDirectoryT(CDelegateMallocData* pMallocData,
		CFTPDirectoryItem* pItemMe,
		CFTPDirectoryBase* pParent,
		LPCWSTR lpszDirectory)
		: CFTPDirectoryBase(pMallocData, pItemMe, pParent, lpszDirectory)
		, CMultipleDispatchImplBase(s_DispatchEntries)
	{
		static_assert(std::is_base_of<IEasySFTPDirectory, T>::value, "T is not derived from IEasySFTPDirectory");
	}
protected:
	CFTPDirectoryT(CDelegateMallocData* pMallocData, CFTPDirectoryItem* pItemMe, ITypeInfo* pInfo)
		: CFTPDirectoryBase(pMallocData, pItemMe, pInfo)
		, CMultipleDispatchImplBase(s_DispatchEntries) {
		const_cast<CMultipleDispatchImplBase::Entry&>(s_DispatchEntries[0]).pInfo = pInfo;
	}

public:
	virtual ~CFTPDirectoryT() {}

	FORWARD_UNKNOWN_IMPL_BASE(CFTPDirectoryBase)

	FORWARD_DISPATCH_IMPL_BASE_NO_UNKNOWN(CMultipleDispatchImplBase)

public:
	STDMETHOD(get_Name)(BSTR* pRet) override { return CFTPDirectoryBase::get_Name(pRet); }
	STDMETHOD(get_RootDirectory)(IEasySFTPRootDirectory** ppRootDirectory) override { return CFTPDirectoryBase::get_RootDirectory( ppRootDirectory); }
	STDMETHOD(OpenDirectory)(VARIANT file, IEasySFTPDirectory** ppRet) override { return CFTPDirectoryBase::OpenDirectory(file, ppRet); }
	STDMETHOD(get_Files)(IEasySFTPFiles** ppFiles) override { return CFTPDirectoryBase::get_Files( ppFiles); }
	STDMETHOD(OpenTransferDialog)(LONG_PTR hWndOwner) override { return CFTPDirectoryBase::OpenTransferDialog(hWndOwner); }
	STDMETHOD(CloseTransferDialog)() override { return CFTPDirectoryBase::CloseTransferDialog(); }
	STDMETHOD(OpenFile)(BSTR lpszRelativeFileName, VARIANT_BOOL bIsWrite, EasySFTPTextMode nTextMode, IEasySFTPStream** ppStream) override { return CFTPDirectoryBase::OpenFile(lpszRelativeFileName, bIsWrite, nTextMode, ppStream); }
	STDMETHOD(UploadFrom)(BSTR lpszDestinationRelativeName, BSTR lpszSourceLocalName, EasySFTPTextMode nTextMode) override { return CFTPDirectoryBase::UploadFrom(lpszDestinationRelativeName, lpszSourceLocalName, nTextMode); }
	STDMETHOD(UploadFromStream)(BSTR lpszDestinationRelativeFileName, IUnknown* pStream, EasySFTPTextMode nTextMode) override { return CFTPDirectoryBase::UploadFromStream(lpszDestinationRelativeFileName, pStream, nTextMode); }
	STDMETHOD(UploadFromDataObject)(IUnknown* pObject, EasySFTPTextMode nTextMode) override { return CFTPDirectoryBase::UploadFromDataObject(pObject, nTextMode); }
	STDMETHOD(DownloadTo)(VARIANT file, BSTR lpszTargetLocalName, EasySFTPTextMode nTextMode) override { return CFTPDirectoryBase::DownloadTo(file, lpszTargetLocalName, nTextMode); }
	STDMETHOD(DownloadToStream)(VARIANT file, IUnknown* pStream, EasySFTPTextMode nTextMode) override { return CFTPDirectoryBase::DownloadToStream(file, pStream, nTextMode); }
	STDMETHOD(UploadFiles)(SAFEARRAY* LocalFiles, EasySFTPTextMode nTextMode) override { return CFTPDirectoryBase::UploadFiles(LocalFiles, nTextMode); }
	STDMETHOD(DownloadFiles)(SAFEARRAY* RemoteFiles, BSTR bstrDestinationDirectory, EasySFTPTextMode nTextMode) override { return CFTPDirectoryBase::DownloadFiles(RemoteFiles, bstrDestinationDirectory, nTextMode); }
	STDMETHOD(Move)(VARIANT file, BSTR lpszTargetName) override { return CFTPDirectoryBase::Move(file, lpszTargetName); }
	STDMETHOD(Remove)(VARIANT file) override { return CFTPDirectoryBase::Remove(file); }
	STDMETHOD(UpdateFileTime)(VARIANT file, DATE modifyTime, DATE createTime, DATE accessTime = 0) override { return CFTPDirectoryBase::UpdateFileTime(file, modifyTime, createTime, accessTime); }
	STDMETHOD(UpdateAttributes)(VARIANT file, long attr) override { return CFTPDirectoryBase::UpdateAttributes(file, attr); }
	STDMETHOD(CreateShortcut)(BSTR LinkName, BSTR TargetName) override { return CFTPDirectoryBase::CreateShortcut(LinkName, TargetName); }
	STDMETHOD(get_FullPath)(BSTR* pRet) override { return CFTPDirectoryBase::get_FullPath(pRet); }
	STDMETHOD(get_Url)(BSTR* pRet) override { return CFTPDirectoryBase::get_Url(pRet); }
	STDMETHOD(SynchronizeFrom)(LONG_PTR hWndOwner, BSTR bstrSourceDirectory, EasySFTPSynchronizeMode Flags) override { return CFTPDirectoryBase::SynchronizeFrom(hWndOwner, bstrSourceDirectory, Flags); }
	STDMETHOD(SynchronizeDirectoryFrom)(LONG_PTR hWndOwner, IEasySFTPDirectory* pSourceDirectory, EasySFTPSynchronizeMode Flags) override { return CFTPDirectoryBase::SynchronizeDirectoryFrom(hWndOwner, pSourceDirectory, Flags); }
	STDMETHOD(SynchronizeFolderFrom)(LONG_PTR hWndOwner, IUnknown* pSourceShellFolder, EasySFTPSynchronizeMode Flags) override { return CFTPDirectoryBase::SynchronizeFolderFrom(hWndOwner, pSourceShellFolder, Flags); }
	STDMETHOD(SynchronizeTo)(LONG_PTR hWndOwner, BSTR bstrTargetDirectory, EasySFTPSynchronizeMode Flags) override { return CFTPDirectoryBase::SynchronizeTo(hWndOwner, bstrTargetDirectory, Flags); }
	STDMETHOD(SynchronizeDirectoryTo)(LONG_PTR hWndOwner, IEasySFTPDirectory* pTargetDirectory, EasySFTPSynchronizeMode Flags) override { return CFTPDirectoryBase::SynchronizeDirectoryTo(hWndOwner, pTargetDirectory, Flags); }
	STDMETHOD(SynchronizeFolderTo)(LONG_PTR hWndOwner, IUnknown* pTargetShellFolder, EasySFTPSynchronizeMode Flags) override { return CFTPDirectoryBase::SynchronizeFolderTo(hWndOwner, pTargetShellFolder, Flags); }

	virtual IEasySFTPDirectory* GetThisDirectory() override { return this; }
	virtual void* GetThisForDispatch(REFIID riid) override
	{
		if (IsEqualIID(riid, IID_IEasySFTPDirectory))
			return static_cast<IEasySFTPDirectory*>(this);
		if (IsEqualIID(riid, IID_IEasySFTPDirectorySynchronization))
			return static_cast<IEasySFTPDirectorySynchronization*>(this);
		return NULL;
	}
};

using CFTPDirectory = CFTPDirectoryT<IEasySFTPDirectory>;
