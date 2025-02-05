#pragma once

#include "Folder.h"
#include "TferStat.h"

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
		const CMyPtrArrayT<CFTPFileItem>& aItems);
	// lpszFileNames ::= L"<file-name>\0<file-name>\0 ... <file-name>\0\0"
	STDMETHOD(MoveFTPItems)(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszFromDir, LPCWSTR lpszFileNames);
	STDMETHOD(RenameFTPItem)(LPCWSTR lpszSrcFileName, LPCWSTR lpszNewFileName, CMyStringW* pstrMsg = NULL) = 0;
	// sizeof(pabResults) == sizeof(bool) * aItems.GetCount()
	STDMETHOD(UpdateFTPItemAttributes)(HWND hWndOwner, CFTPDirectoryBase* pDirectory,
		CServerFilePropertyDialog* pDialog, const CMyPtrArrayT<CServerFileAttrData>& aAttrs,
		bool* pabResults) = 0;
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
	STDMETHOD(SetFileTime)(CFTPDirectoryBase* pDirectory, LPCWSTR lpszFileName,
		const FILETIME* pftModifyTime) {
		return E_NOTIMPL;
	}

	STDMETHOD(GetHostInfo)(VARIANT_BOOL FAR* pbIsSFTP, int FAR* pnPort, BSTR FAR* pbstrHostName);
	STDMETHOD(GetTextMode)(LONG FAR* pnTextMode);
	STDMETHOD(SetTextMode)(LONG nTextMode);
	STDMETHOD(GetTransferMode)(LONG FAR* pnTransferMode);
	STDMETHOD(SetTransferMode)(LONG nTransferMode);
	STDMETHOD(IsTextFile)(LPCWSTR lpszFileName);
	STDMETHOD(Disconnect)() = 0;
	STDMETHOD(IsConnected)() = 0;
	STDMETHOD(IsTransferring)() = 0;

	STDMETHOD(OpenFile)(CFTPDirectoryBase* pDirectory, LPCWSTR lpszName, DWORD grfMode, HANDLE* phFile) = 0;
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

	virtual LPCWSTR GetProtocolName(int& nDefPort) const = 0;
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

	//void BeforeClipboardOperation(IDataObject* pObjectNew);
	//void AfterClipboardOperation(IDataObject* pObjectNew);
	void ShowServerInfoDialog(HWND hWndOwner);

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

////////////////////////////////////////////////////////////////////////////////

inline STDMETHODIMP CFTPDirectoryBase::GetHostInfo(VARIANT_BOOL FAR* pbIsSFTP, int FAR* pnPort, BSTR FAR* pbstrHostName)
{
	return m_pRoot->GetHostInfo(pbIsSFTP, pnPort, pbstrHostName);
}
inline STDMETHODIMP CFTPDirectoryBase::GetTextMode(LONG FAR* pnTextMode)
{
	return m_pRoot->GetTextMode(pnTextMode);
}
inline STDMETHODIMP CFTPDirectoryBase::SetTextMode(LONG nTextMode)
{
	return m_pRoot->SetTextMode(nTextMode);
}
inline STDMETHODIMP CFTPDirectoryBase::GetTransferMode(LONG FAR* pnTransferMode)
{
	return m_pRoot->GetTransferMode(pnTransferMode);
}
inline STDMETHODIMP CFTPDirectoryBase::SetTransferMode(LONG nTransferMode)
{
	return m_pRoot->SetTransferMode(nTransferMode);
}
inline STDMETHODIMP CFTPDirectoryBase::IsTextFile(LPCWSTR lpszFileName)
{
	return m_pRoot->IsTextFile(lpszFileName);
}
inline STDMETHODIMP CFTPDirectoryBase::Disconnect()
{
	return m_pRoot->Disconnect();
}
inline STDMETHODIMP CFTPDirectoryBase::IsConnected()
{
	return m_pRoot->IsConnected();
}
inline STDMETHODIMP CFTPDirectoryBase::IsTransferring()
{
	return m_pRoot->IsTransferring();
}
inline bool CFTPDirectoryBase::DoReceiveDirectory()
{
	return m_bDirReceived ? true : m_pRoot->ReceiveDirectory(m_hWndOwnerCache, this, m_strDirectory, &m_bDirReceived);
}
