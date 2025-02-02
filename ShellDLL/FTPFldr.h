
#pragma once

#include "RFolder.h"
#include "FTPConn.h"

#define SVT_UNKNOWN           0
#define SVT_UNIX              1
#define SVT_DOS               2
#define SVT_WINDOWS           3

////////////////////////////////////////////////////////////////////////////////

class CSFTPFolderFTPDirectory : public CFTPDirectoryBase
{
public:
	CSFTPFolderFTPDirectory(CDelegateMallocData* pMallocData,
		CFTPDirectoryItem* pItemMe,
		CFTPDirectoryBase* pParent,
		CFTPDirectoryRootBase* pRoot,
		LPCWSTR lpszDirectory);
	virtual ~CSFTPFolderFTPDirectory();

public:
	//virtual HRESULT CreateStream(CFTPFileItem* pItem, IStream** ppStream);

	STDMETHOD(CreateInstance)(CFTPDirectoryItem* pItemMe, CFTPDirectoryBase* pParent, CFTPDirectoryRootBase* pRoot,
		LPCWSTR lpszDirectory, CFTPDirectoryBase** ppResult);
	STDMETHOD_(void, UpdateItem)(CFTPFileItem* pOldItem, LPCWSTR lpszNewItem, LONG lEvent);

protected:
	CSFTPFolderFTPDirectory(CDelegateMallocData* pMallocData, CFTPDirectoryItem* pItemMe);
};

class CSFTPFolderFTP : public CFTPDirectoryRootBase//,
	//public CFTPMessageDispatcher
{
public:
	CSFTPFolderFTP(CDelegateMallocData* pMallocData, CFTPDirectoryItem* pItemMe, CEasySFTPFolderRoot* pFolderRoot);
	virtual ~CSFTPFolderFTP();

	// IUnknown
public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv) { return CFTPDirectoryRootBase::QueryInterface(riid, ppv); }
	STDMETHOD_(ULONG, AddRef)() { return CFTPDirectoryRootBase::AddRef(); }
	STDMETHOD_(ULONG, Release)() { return CFTPDirectoryRootBase::Release(); }

	// CFTPDirectoryRootBase
public:
	STDMETHOD(GetFTPItemUIObjectOf)(HWND hWndOwner, CFTPDirectoryBase* pDirectory,
		const CMyPtrArrayT<CFTPFileItem>& aItems, CFTPDataObject** ppObject);
	STDMETHOD(SetFTPItemNameOf)(HWND hWnd, CFTPDirectoryBase* pDirectory,
		CFTPFileItem* pItem, LPCWSTR pszName, SHGDNF uFlags);
	STDMETHOD(DoDeleteFTPItems)(HWND hWndOwner, CFTPDirectoryBase* pDirectory,
		const CMyPtrArrayT<CFTPFileItem>& aItems);
	STDMETHOD(RenameFTPItem)(LPCWSTR lpszSrcFileName, LPCWSTR lpszNewFileName, CMyStringW* pstrMsg);
	STDMETHOD(UpdateFTPItemAttributes)(HWND hWndOwner, CFTPDirectoryBase* pDirectory,
		CServerFilePropertyDialog* pDialog, const CMyPtrArrayT<CServerFileAttrData>& aAttrs, bool* pabResults);
	STDMETHOD(CreateFTPDirectory)(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszName);
	STDMETHOD(CreateFTPItemStream)(CFTPDirectoryBase* pDirectory, CFTPFileItem* pItem, IStream** ppStream);
	STDMETHOD(WriteFTPItem)(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszName, IStream* pStream,
		void* pvObject, CTransferStatus* pStatus);
	STDMETHOD(CreateInstance)(CFTPDirectoryItem* pItemMe, CFTPDirectoryBase* pParent, CFTPDirectoryRootBase* pRoot,
		LPCWSTR lpszDirectory, CFTPDirectoryBase** ppResult);

	STDMETHOD(Disconnect)();
	STDMETHOD(IsConnected)() { return m_pConnection != NULL ? S_OK : S_FALSE; }
	STDMETHOD(IsTransferring)() { return m_dwTransferringCount > 0 ? S_OK : S_FALSE; }

	STDMETHOD(OpenFile)(CFTPDirectoryBase* pDirectory, LPCWSTR lpszName, DWORD grfMode, HANDLE* phFile);
	STDMETHOD(ReadFile)(HANDLE hFile, void* outBuffer, DWORD dwSize, DWORD* pdwRead);
	STDMETHOD(WriteFile)(HANDLE hFile, const void* inBuffer, DWORD dwSize, DWORD* pdwWritten);
	STDMETHOD(SeekFile)(HANDLE hFile, LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer);
	STDMETHOD(StatFile)(HANDLE hFile, STATSTG* pStatstg, DWORD grfStatFlag);
	STDMETHOD(CloseFile)(HANDLE hFile);
	STDMETHOD(DuplicateFile)(HANDLE hFile, HANDLE* phFile);
	STDMETHOD(LockRegion)(HANDLE hFile, ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
	STDMETHOD(UnlockRegion)(HANDLE hFile, ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
	STDMETHOD(StatDirectory)(CFTPDirectoryBase* pDirectory, DWORD grfMode, STATSTG* pStatstg, DWORD grfStatFlag);
	STDMETHOD(SetFileTime)(LPCWSTR lpszFileName, const FILETIME* pctime, const FILETIME* patime, const FILETIME* pmtime);

	STDMETHOD_(IShellFolder*, GetParentFolder)() { return m_pFolderRoot; }

	virtual LPCWSTR GetProtocolName(int& nDefPort) const { nDefPort = 21; return L"ftp"; }
	virtual bool IsLockSupported() const { return false; }
	virtual void PreShowPropertyDialog(CServerFilePropertyDialog* pDialog);
	virtual void PreShowServerInfoDialog(CServerInfoDialog* pDialog);
	virtual bool ReceiveDirectory(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszDirectory, bool* pbReceived);
	virtual bool ValidateDirectory(LPCWSTR lpszParentDirectory, PCUIDLIST_RELATIVE pidlChild,
		CMyStringW& rstrRealPath);
	virtual CFTPFileItem* RetrieveFileItem(CFTPDirectoryBase* pDirectory, LPCWSTR lpszFileName);

//	// CFTPMessageDispatcher
//public:
//	virtual void SendCommand(LPCWSTR lpszType, LPCWSTR lpszParam);
//	virtual void SecureSendCommand(LPCWSTR lpszType, const _SecureStringW& strParam);
//	virtual bool ReceiveMessage(CMyStringW& rstrMessage, int* pnCode);

public:
	bool Connect(HWND hWnd, LPCWSTR lpszHostName, int nPort, CUserInfo* pUser);

protected:
	CEasySFTPFolderRoot* m_pFolderRoot;

	HWND m_hWndOwner;
	CFTPConnection* m_pConnection;
	UINT_PTR m_idTimer;
	CUserInfo* m_pUser;
	CMyStringW m_strServerInfo;
	CMyStringW m_strWelcomeMessage;
	bool m_bFirstReceive;
	bool m_bLoggingIn;
	int m_nServerSystemType;
	char m_nYearFollows;      // used for DOS system type
	bool m_bY2KProblem;
	DWORD m_dwTransferringCount;
	CRITICAL_SECTION m_csSocket;
	CMyPtrArrayT<CWaitResponseData> m_aWaitResponse;
	CMyPtrArrayT<CFTPWaitPassive> m_aWait150Messages;
	CMyPtrArrayT<CFTPWaitPassive> m_aWaitPassives;

	void IncrementTransferCount();
	void DecrementTransferCount();

public:
	CMyStringW m_strChmodCommand;

protected:
	void ShowFTPErrorMessage(int code, LPCWSTR lpszMessage);

	static void CALLBACK KeepConnectionTimerProc(UINT_PTR idEvent, LPARAM lParam);
	void OnFTPSocketReceive();
	void _OnFTPSocketReceiveThreadUnsafe();
	void DoReceiveSocket();
	CFTPWaitEstablishPassive* StartPassive(CFTPPassiveMessage* pMessage);
	CFTPWaitPassive* PassiveStarted(CFTPWaitEstablishPassive* pWait, CTextSocket* pSocket);
	void DoReceivePassive(CFTPWaitPassive* pPassive);
	CFTPFileItem* RetrieveFileItem2(LPCWSTR lpszFullPathName);
	virtual HRESULT DoDeleteFileOrDirectory(HWND hWndOwner, CMyStringArrayW& astrMsgs, bool bIsDirectory, LPCWSTR lpszFile, CFTPDirectoryBase* pDirectory = NULL);

public:
	bool WaitForReceive(bool* pbWaiting, CFTPWaitPassive* pPassive = NULL);

protected:
	void AddFileDataIfNeed(LPCWSTR lpszPickupFileName, LPCWSTR lpszAttributes);

	inline int DoRetryAuthentication(bool bFirstAttempt)
		{ return m_pFolderRoot->DoRetryAuthentication(m_hWndOwner, m_pUser, false, NULL, bFirstAttempt); }

	class CFTPFileListingHandler : public CFTPFileListingListener
	{
	public:
		CFTPFileListingHandler(CSFTPFolderFTP* pRoot, CFTPDirectoryBase* pDirectory);
		~CFTPFileListingHandler();

		virtual bool ReceiveFileListing(CTextSocket* pPassive, bool bMListing);
		virtual void FinishFileListing();

	private:
		CSFTPFolderFTP* m_pRoot;
		CFTPDirectoryBase* m_pDirectory;
		char m_nServerSystemType;
	};

	class CFTPHandleData
	{
	public:
		CFTPDirectoryBase* pDirectory;
		CFTPPassiveMessage* pMessage;
		CFTPWaitPassive* pPassive;
		CFTPFileItem* pItem;
		ULONGLONG offset;
		STATSTG statstg;
		DWORD grfMode;
		DWORD dwRefCount;
	};

	friend class CFTPStream;
};
