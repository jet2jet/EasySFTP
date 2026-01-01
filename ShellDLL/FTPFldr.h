
#pragma once

#include "RFolder.h"
#include "FTPConn.h"
#include "FoldRoot.h"

#define SVT_UNKNOWN           0
#define SVT_UNIX              1
#define SVT_DOS               2
#define SVT_WINDOWS           3

////////////////////////////////////////////////////////////////////////////////

class CSFTPFolderFTPDirectory : public CFTPDirectory
{
public:
	CSFTPFolderFTPDirectory(CDelegateMallocData* pMallocData,
		CFTPDirectoryItem* pItemMe,
		CFTPDirectoryBase* pParent,
		LPCWSTR lpszDirectory);
	virtual ~CSFTPFolderFTPDirectory();

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override { return CFTPDirectoryBase::QueryInterface(riid, ppv); }

public:
	//virtual HRESULT CreateStream(CFTPFileItem* pItem, IStream** ppStream);

	STDMETHOD(CreateInstance)(CFTPDirectoryItem* pItemMe, CFTPDirectoryBase* pParent,
		LPCWSTR lpszDirectory, CFTPDirectoryBase** ppResult) override;
	STDMETHOD_(void, UpdateItem)(CFTPFileItem* pOldItem, LPCWSTR lpszNewItem, LONG lEvent) override;

protected:
	CSFTPFolderFTPDirectory(CDelegateMallocData* pMallocData, CFTPDirectoryItem* pItemMe, ITypeInfo* pInfo);
};

class CSFTPFolderFTP : public CFTPDirectoryRoot
{
public:
	CSFTPFolderFTP(CDelegateMallocData* pMallocData, CFTPDirectoryItem* pItemMe, CEasySFTPFolderRoot* pFolderRoot, bool bIsFTPS);
	virtual ~CSFTPFolderFTP();

	// IUnknown
public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override { return CFTPDirectoryRootBase::QueryInterface(riid, ppv); }

	// CFTPDirectoryRootBase
public:
	STDMETHOD(GetFTPItemUIObjectOf)(HWND hWndOwner, CFTPDirectoryBase* pDirectory,
		const CMyPtrArrayT<CFTPFileItem>& aItems, CFTPDataObject** ppObject);
	STDMETHOD(SetFTPItemNameOf)(HWND hWnd, CFTPDirectoryBase* pDirectory,
		CFTPFileItem* pItem, LPCWSTR pszName, SHGDNF uFlags);
	STDMETHOD(RenameFTPItem)(LPCWSTR lpszSrcFileName, LPCWSTR lpszNewFileName, CMyStringW* pstrMsg);
	STDMETHOD(UpdateFTPItemAttribute)(CFTPDirectoryBase* pDirectory, const CServerFileAttrData* pAttr, CMyStringW* pstrMsg = NULL);
	STDMETHOD(CreateFTPDirectory)(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszName);
	STDMETHOD(CreateFTPItemStream)(CFTPDirectoryBase* pDirectory, CFTPFileItem* pItem, IStream** ppStream);
	STDMETHOD(WriteFTPItem)(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszName, IStream* pStream,
		void* pvObject, CTransferStatus* pStatus);
	STDMETHOD(CreateInstance)(CFTPDirectoryItem* pItemMe, CFTPDirectoryBase* pParent,
		LPCWSTR lpszDirectory, CFTPDirectoryBase** ppResult) override;

	STDMETHOD(get_IsUnixServer)(VARIANT_BOOL* pbRet);
	STDMETHOD(Disconnect)();
	STDMETHOD(IsConnected)() { return m_pConnection != NULL ? S_OK : S_FALSE; }
	STDMETHOD(IsTransferring)() { return m_dwTransferringCount > 0 ? S_OK : S_FALSE; }

	STDMETHOD(OpenFile)(CFTPDirectoryBase* pDirectory, LPCWSTR lpszName, DWORD grfMode, HANDLE* phFile, IEasySFTPFile** ppFile = NULL);
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

	virtual EasySFTPConnectionMode GetProtocol(int* pDefPort) const
	{
		if (pDefPort)
			*pDefPort = 21;
		return m_bIsFTPS ? EasySFTPConnectionMode::FTPS : EasySFTPConnectionMode::FTP;
	}
	virtual LPCWSTR GetProtocolName() const { return m_bIsFTPS ? L"ftps" : L"ftp"; }
	virtual bool IsLockSupported() const { return false; }
	virtual void PreShowPropertyDialog(CServerFilePropertyDialog* pDialog);
	virtual void PreShowServerInfoDialog(CServerInfoDialog* pDialog);
	virtual bool ReceiveDirectory(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszDirectory, bool* pbReceived);
	virtual bool ValidateDirectory(LPCWSTR lpszParentDirectory, PCUIDLIST_RELATIVE pidlChild,
		CMyStringW& rstrRealPath);
	virtual CFTPFileItem* RetrieveFileItem(CFTPDirectoryBase* pDirectory, LPCWSTR lpszFileName);
	virtual void SetChmodCommand(LPCWSTR lpszCommand)
	{
		m_strChmodCommand = lpszCommand;
	}

//	// CFTPMessageDispatcher
//public:
//	virtual void SendCommand(LPCWSTR lpszType, LPCWSTR lpszParam);
//	virtual void SecureSendCommand(LPCWSTR lpszType, const _SecureStringW& strParam);
//	virtual bool ReceiveMessage(CMyStringW& rstrMessage, int* pnCode);

public:
	bool Connect(HWND hWnd, LPCWSTR lpszHostName, int nPort, IEasySFTPAuthentication2* pUser);

protected:

	HWND m_hWndOwner;
	CFTPConnection* m_pConnection;
	UINT_PTR m_idTimer;
	IEasySFTPAuthentication* m_pUser;
	CMyStringW m_strServerInfo;
	CMyStringW m_strWelcomeMessage;
	int m_nServerSystemType;
	bool m_bIsFTPS;
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
	enum class ProcessLoginResult
	{
		InProgress = 0,
		Finish = 1,
		Cancel = 2,
		Failure = 3
	};

	void ShowFTPErrorMessage(int code, LPCWSTR lpszMessage);

	static void CALLBACK KeepConnectionTimerProc(UINT_PTR idEvent, LPARAM lParam);
	ProcessLoginResult DoProcessForLogin(CFTPConnection* pConnection, int code, CMyStringW& strMsg, const CMyStringW& strCommand, CWaitResponseData* pWait);
	// return true if main control connection is disconnected
	bool DisconnectImpl(CFTPConnection* pConnection);
	bool DoReceiveSocketCommon(CFTPConnection* pConnection, int& code, CMyStringW& strMsg, CMyStringW& strCommand, CWaitResponseData*& pWait,
		void (*pfnOnLoginFinished)(CFTPConnection* pConnection, void* pParam) = NULL, void* pParam = NULL);
	void DoReceiveSocket();
	void DoReceiveSocketPassiveControl(CFTPConnection* pConnection, CFTPWaitEstablishPassive* pEstablish = NULL);
	void StartAuth(CFTPConnection* pConnection);
	CFTPWaitEstablishPassive* StartPassive(CFTPPassiveMessage* pMessage);
	CFTPWaitPassive* PassiveStarted(CFTPWaitEstablishPassive* pWait, CFTPSocket* pSocket);
	void DoReceivePassive(CFTPWaitPassive* pPassive);
	CFTPFileItem* RetrieveFileItem2(CFTPDirectoryBase* pDirectory, LPCWSTR lpszFullPathName);
	virtual HRESULT DoDeleteFileOrDirectory(HWND hWndOwner, CMyStringArrayW& astrMsgs, bool bIsDirectory, LPCWSTR lpszFile, CFTPDirectoryBase* pDirectory = NULL);

public:
	bool WaitForReceive(bool* pbWaiting);
	bool WaitForReceiveEstablishPassive(bool* pbWaiting, CFTPWaitEstablishPassive* pPassive);
	bool WaitForReceivePassive(bool* pbWaiting, CFTPWaitPassive* pPassive, DWORD dwTimeoutMilliseconds = INFINITE);

protected:
	inline int DoRetryAuthentication(bool bFirstAttempt)
	{
		return m_pParent->DoRetryAuthentication(m_hWndOwner, m_pUser,
			m_bIsFTPS ? EasySFTPConnectionMode::FTPS : EasySFTPConnectionMode::FTP, NULL, bFirstAttempt);
	}

	class CFTPFileListingHandler : public CFTPFileListingListener
	{
	public:
		CFTPFileListingHandler(CSFTPFolderFTP* pRoot, CFTPDirectoryBase* pDirectory);
		~CFTPFileListingHandler();

		virtual bool ReceiveFileListing(CFTPSocket* pPassive, bool bMListing) override;
		virtual void FinishFileListing() override;

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
