#pragma once

#include "Folder.h"
#include "TferStat.h"
#include "Transfer.h"

class CTransferDialog;
class CEasySFTPFolderRoot;

class CFTPDirectoryRootBase : public CFTPDirectoryT<IEasySFTPRootDirectory>,
	public CTransferDialogListener,
	public CTransferStatus
{
public:
	CFTPDirectoryRootBase(CDelegateMallocData* pMallocData, CFTPDirectoryItem* pItemMe, CEasySFTPFolderRoot* pParent);
	virtual ~CFTPDirectoryRootBase();

	virtual ULONG DetachAndRelease() override;

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override;
	STDMETHOD_(ULONG, AddRef)() override;
	STDMETHOD_(ULONG, Release)() override;

	// IProvideClassInfo
public:
	STDMETHOD(GetClassInfo)(ITypeInfo** ppTI) override;

	// abstract methods of CFTPDirectoryRootBase
public:
	STDMETHOD(GetFTPItemUIObjectOf)(HWND hWndOwner, CFTPDirectoryBase* pDirectory,
		const CMyPtrArrayT<CFTPFileItem>& aItems, CFTPDataObject** ppObject) = 0;
	STDMETHOD(SetFTPItemNameOf)(HWND hWnd, CFTPDirectoryBase* pDirectory,
		CFTPFileItem* pItem, LPCWSTR pszName, SHGDNF uFlags) = 0;
	STDMETHOD(DoDeleteFTPItems)(HWND hWndOwner, CFTPDirectoryBase* pDirectory,
		const CMyPtrArrayT<CFTPFileItem>& aItems);
	// lpszFileNames ::= L"<file-name>\0<file-name>\0 ... <file-name>\0\0"
	STDMETHOD(MoveFTPItems)(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszFromDir, LPCWSTR lpszFileNames);
	STDMETHOD(RenameFTPItem)(LPCWSTR lpszSrcFileName, LPCWSTR lpszNewFileName, CMyStringW* pstrMsg = NULL) = 0;
	STDMETHOD(UpdateFTPItemAttribute)(CFTPDirectoryBase* pDirectory, const CServerFileAttrData* pAttr, CMyStringW* pstrMsg = NULL) = 0;
	// sizeof(pabResults) == sizeof(bool) * aItems.GetCount()
	STDMETHOD(UpdateFTPItemAttributes)(HWND hWndOwner, CFTPDirectoryBase* pDirectory,
		const CMyPtrArrayT<CServerFileAttrData>& aAttrs, bool* pabResults);
	STDMETHOD(CreateFTPDirectory)(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszName) = 0;
	STDMETHOD(CreateShortcut)(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszName, LPCWSTR lpszLinkTo, bool bHardLink)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(CreateFTPItemStream)(CFTPDirectoryBase* pDirectory, CFTPFileItem* pItem, IStream** ppStream) = 0;
	STDMETHOD(WriteFTPItem)(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszName, IStream* pStream,
		void* pvObject, CTransferStatus* pStatus) = 0;
	//STDMETHOD(CreateInstance)(CFTPDirectoryItem* pItemMe, CFTPDirectoryBase* pParent, CFTPDirectoryRootBase* pRoot,
	//	LPCWSTR lpszDirectory, CFTPDirectoryBase** ppResult) = 0;
	//STDMETHOD(CreateViewObject)(HWND hWndOwner, REFIID riid, void** ppv);
	STDMETHOD(OpenFile)(CFTPDirectoryBase* pDirectory, LPCWSTR lpszName, DWORD grfMode, HANDLE* phFile, IEasySFTPFile** ppFile = NULL) = 0;
	STDMETHOD(ReadFile)(HANDLE hFile, void* outBuffer, DWORD dwSize, DWORD* pdwRead) = 0;
	STDMETHOD(WriteFile)(HANDLE hFile, const void* inBuffer, DWORD dwSize, DWORD* pdwWritten) = 0;
	STDMETHOD(SeekFile)(HANDLE hFile, LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer) = 0;
	STDMETHOD(StatFile)(HANDLE hFile, STATSTG* pStatstg, DWORD grfStatFlag) = 0;
	STDMETHOD(CloseFile)(HANDLE hFile) = 0;
	STDMETHOD(DuplicateFile)(HANDLE hFile, HANDLE* phFile) = 0;
	STDMETHOD(LockRegion)(HANDLE hFile, ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) = 0;
	STDMETHOD(UnlockRegion)(HANDLE hFile, ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) = 0;
	STDMETHOD(StatDirectory)(CFTPDirectoryBase* pDirectory, DWORD grfMode, STATSTG* pStatstg, DWORD grfStatFlag) = 0;
	STDMETHOD(SetFileTime)(LPCWSTR lpszFileName, const FILETIME* pctime, const FILETIME* patime, const FILETIME* pmtime) = 0;

	// IEasySFTPRootDirectory
public:
	STDMETHOD(get_IsUnixServer)(VARIANT_BOOL* pbRet) = 0;
	STDMETHOD(get_HostName)(BSTR* pRet);
	STDMETHOD(get_Port)(long* pRet);
	STDMETHOD(get_ConnectionMode)(EasySFTPConnectionMode* pRet);
	STDMETHOD(get_TextMode)(EasySFTPTextMode* pnTextMode);
	STDMETHOD(put_TextMode)(EasySFTPTextMode nTextMode);
	STDMETHOD(get_TransferMode)(EasySFTPTransferMode* pnTransferMode);
	STDMETHOD(put_TransferMode)(EasySFTPTransferMode nTransferMode);
	STDMETHOD(IsTextFile)(BSTR lpszFileName, VARIANT_BOOL* pbRet);

	STDMETHOD(Disconnect)() = 0;

	STDMETHOD(get_Connected)(VARIANT_BOOL* pbRet)
	{
		if (!pbRet)
			return E_POINTER;
		auto hr = IsConnected();
		if (FAILED(hr))
			return hr;
		*pbRet = hr == S_OK ? VARIANT_TRUE : VARIANT_FALSE;
		return S_OK;
	}
	STDMETHOD(get_Transferring)(VARIANT_BOOL* pbRet)
	{
		if (!pbRet)
			return E_POINTER;
		auto hr = IsTransferring();
		if (FAILED(hr))
			return hr;
		*pbRet = hr == S_OK ? VARIANT_TRUE : VARIANT_FALSE;
		return S_OK;
	}

	// IEasySFTPOldDirectory
public:
	STDMETHOD(GetHostInfo)(VARIANT_BOOL FAR* pbIsSFTP, int FAR* pnPort, BSTR FAR* pbstrHostName);
	STDMETHOD(GetTextMode)(LONG FAR* pnTextMode)
	{
		if (!pnTextMode)
			return E_POINTER;
		EasySFTPTextMode mode;
		auto hr = get_TextMode(&mode);
		if (FAILED(hr))
			return hr;
		*pnTextMode = static_cast<LONG>(mode);
		return S_OK;
	}
	STDMETHOD(SetTextMode)(LONG nTextMode)
	{
		return put_TextMode(static_cast<EasySFTPTextMode>(nTextMode));
	}
	STDMETHOD(GetTransferMode)(LONG FAR* pnTransferMode)
	{
		if (!pnTransferMode)
			return E_POINTER;
		EasySFTPTransferMode mode;
		auto hr = get_TransferMode(&mode);
		if (FAILED(hr))
			return hr;
		*pnTransferMode = static_cast<LONG>(mode);
		return S_OK;
	}
	STDMETHOD(SetTransferMode)(LONG nTransferMode)
	{
		return put_TransferMode(static_cast<EasySFTPTransferMode>(nTransferMode));
	}
	STDMETHOD(IsTextFile)(LPCWSTR lpszFileName)
	{
		CMyStringW strFile(lpszFileName);
		auto bstr = MyStringToBSTR(strFile);
		if (!bstr)
			return E_OUTOFMEMORY;
		VARIANT_BOOL b = VARIANT_FALSE;
		auto hr = IsTextFile(bstr, &b);
		::SysFreeString(bstr);
		return SUCCEEDED(hr) ? (b ? S_OK : S_FALSE) : hr;
	}
	//STDMETHOD(Disconnect)() = 0;
	STDMETHOD(IsConnected)() = 0;
	STDMETHOD(IsTransferring)() = 0;

public:
	virtual IShellFolder* GetParentFolder() override;
	virtual HRESULT SetParentFolder(IShellFolder* pFolder) override;

public:
	virtual void TransferCanceled()
	{
		m_bIsTransferCanceled = true;
	}

	virtual bool Connect(HWND hWnd, LPCWSTR lpszHostName, int nPort, IEasySFTPAuthentication* pUser) = 0;
	virtual EasySFTPConnectionMode GetProtocol(int* pnDefPort = NULL) const = 0;
	virtual LPCWSTR GetProtocolName() const = 0;
	virtual bool IsLockSupported() const = 0;
	virtual void PreShowPropertyDialog(CServerFilePropertyDialog* pDialog) { }
	// pDialog may be NULL
	// return: whether the protocol supports creation of shortcut (link)
	virtual bool PreShowCreateShortcutDialog(CLinkDialog* pDialog) { return false; }
	virtual void PreShowServerInfoDialog(CServerInfoDialog* pDialog) { }
	virtual bool ReceiveDirectory(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszDirectory, bool* pbReceived) = 0;
	virtual bool ValidateDirectory(LPCWSTR lpszParentDirectory, PCUIDLIST_RELATIVE pidlChild,
		CMyStringW& rstrRealPath) = 0;
	virtual CFTPFileItem* RetrieveFileItem(CFTPDirectoryBase* pDirectory, LPCWSTR lpszFileName) = 0;
	virtual ULONG GetServerVersion() { return 0; }
	virtual void SetChmodCommand(LPCWSTR lpszCommand) {}

	//void BeforeClipboardOperation(IDataObject* pObjectNew);
	//void AfterClipboardOperation(IDataObject* pObjectNew);
	void ShowServerInfoDialog(HWND hWndOwner);
	void OpenTransferDialogImpl(HWND hWndOwner);
	void CloseTransferDialogImpl();

	virtual void TransferCanceled(void* pvTransfer);
	virtual void TransferInProgress(void* pvObject, ULONGLONG uliPosition);
	virtual bool TransferIsCanceled(void* pvObject);

	void OnDisconnect();
	virtual HRESULT DoDeleteFileOrDirectory(HWND hWndOwner, CMyStringArrayW& astrMsgs, bool bIsDirectory, LPCWSTR lpszFile, CFTPDirectoryBase* pDirectory = NULL) = 0;
	HRESULT DoDeleteDirectoryRecursive(HWND hWndOwner, CMyStringArrayW& astrMsgs, LPCWSTR lpszName, CFTPDirectoryBase* pDirectory);

public:
	CMyStringW m_strHostName;
	CMyStringArrayW m_arrTextFileType;
	int m_nPort;
	BYTE m_bTextMode;
	char m_nServerCharset;
	// TRANSFER_MODE_XXX
	char m_nTransferMode;
	bool m_bUseSystemTextFileType;
	bool m_bAdjustRecvModifyTime;
	bool m_bAdjustSendModifyTime;
	bool m_bUseThumbnailPreview;
	bool m_bIsTransferCanceled;
	IDataObject* m_pObjectOnClipboard;
	CTransferDialog* m_pTransferDialog;
	CEasySFTPFolderRoot* m_pParent;
};

using CFTPDirectoryRoot = CFTPDirectoryRootBase;

////////////////////////////////////////////////////////////////////////////////

inline CFTPDirectoryRootBase* CFTPDirectoryBase::GetRoot()
{
	auto* p = this;
	while (p && !p->m_bIsRoot)
		p = p->m_pParent;
	return static_cast<CFTPDirectoryRootBase*>(p);
}

inline STDMETHODIMP CFTPDirectoryBase::GetHostInfo(VARIANT_BOOL FAR* pbIsSFTP, int FAR* pnPort, BSTR FAR* pbstrHostName)
{
	return GetRoot()->GetHostInfo(pbIsSFTP, pnPort, pbstrHostName);
}
inline STDMETHODIMP CFTPDirectoryBase::GetTextMode(LONG FAR* pnTextMode)
{
	return GetRoot()->GetTextMode(pnTextMode);
}
inline STDMETHODIMP CFTPDirectoryBase::SetTextMode(LONG nTextMode)
{
	return GetRoot()->SetTextMode(nTextMode);
}
inline STDMETHODIMP CFTPDirectoryBase::GetTransferMode(LONG FAR* pnTransferMode)
{
	return GetRoot()->GetTransferMode(pnTransferMode);
}
inline STDMETHODIMP CFTPDirectoryBase::SetTransferMode(LONG nTransferMode)
{
	return GetRoot()->SetTransferMode(nTransferMode);
}
inline STDMETHODIMP CFTPDirectoryBase::IsTextFile(LPCWSTR lpszFileName)
{
	return GetRoot()->IsTextFile(lpszFileName);
}
inline STDMETHODIMP CFTPDirectoryBase::Disconnect()
{
	return GetRoot()->Disconnect();
}
inline STDMETHODIMP CFTPDirectoryBase::IsConnected()
{
	return GetRoot()->IsConnected();
}
inline STDMETHODIMP CFTPDirectoryBase::IsTransferring()
{
	return GetRoot()->IsTransferring();
}
inline bool CFTPDirectoryBase::DoReceiveDirectory()
{
	return m_bDirReceived ? true : GetRoot()->ReceiveDirectory(m_hWndOwnerCache, this, m_strDirectory, &m_bDirReceived);
}
