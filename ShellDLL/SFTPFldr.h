
#pragma once

#include "SFTPChan.h"
#include "SSHCli.h"
#include "SFTPStrm.h"

#include "RFolder.h"
#include "FoldRoot.h"

#include "MsgData.h"

class CSFTPFolderSFTPDirectory;
class CSFTPFolderSFTP;

struct CSFTPWaitData
{
	ULONG uMsgID;
	int nType;
};

struct CSFTPWaitDirectoryData;

struct CSFTPWaitConfirm : public CWaitResponseData
{
	inline CSFTPWaitConfirm() : CWaitResponseData(WRD_CONFIRM) { }
	inline CSFTPWaitConfirm(int nWaitType) : CWaitResponseData(nWaitType) { }
	int nResult;
	CMyStringW strMessage;
};

struct CSFTPWaitAttrData : public CSFTPWaitConfirm
{
	inline CSFTPWaitAttrData() : CSFTPWaitConfirm(WRD_FTPWAITATTR) { fileData.dwMask = 0; }
	~CSFTPWaitAttrData() { if (fileData.dwMask & SSH_FILEXFER_ATTR_EXTENDED) free(fileData.aExAttrs); }
	CMyStringW strFileName;
	CMyStringW strRemoteDirectory;
	enum
	{
		// -1: End (used for type 3, and represents the result has come)
		typeEnd = -1,
		// 0: Normal
		typeNormal = 0,
		// 1: Read link
		typeReadLink = 1,
		// 2: Retrieve file list for receive files
		typeRetrieveFile = 2,
		// 3: Real path
		typeRealPath = 3
	};
	char nType;
	union
	{
		CFTPFileItem* pItem;

		//CRecvDirectoryData* pDirData;
	};
	CSFTPWaitDirectoryData* pWaitDir;
	CSFTPFileAttribute fileData;
	union
	{
		// base link-file data (the first link file)
		CFTPFileItem* pLinkFrom;

		HSFTPHANDLE hDirHandle;
	};
};

struct CSFTPWaitReadData : public CSFTPWaitConfirm
{
	inline CSFTPWaitReadData() : CSFTPWaitConfirm(WRD_READDATA), outBuffer(NULL), bufferCapacity(0), readBytes(0), bIsEOF(false){}

	void* outBuffer;
	DWORD bufferCapacity;
	DWORD readBytes;
	bool bIsEOF;
};

struct CSFTPSendFileData
{
	bool bForSend;
	CMyStringW strFile;
	IStream* pStream;
	HSFTPHANDLE hFile;
	ULONGLONG uliOffset;
	FILETIME ftFileTime;
	//CSyncSendRecvFile* pSendFile;
	bool bCanceled;
};

struct CSFTPWaitDirectoryData : public CWaitResponseData
{
	inline CSFTPWaitDirectoryData() : CWaitResponseData(WRD_DIRECTORY), nStep(stepFinished), bResult(false), nStatus(SSH_FX_OK) { }
	enum
	{
		stepFinished = 0,
		stepRetrieveHandle = 1,
		stepRetrieveFiles = 2
	};
	char nStep;
	bool bResult;
	int nStatus;
	HSFTPHANDLE hSFTPHandle;
	DWORD dwReadLinkCount;
	CFTPDirectoryBase* pDirectory;
};

struct CSFTPWaitFileHandle : public CSFTPWaitConfirm
{
	inline CSFTPWaitFileHandle() : CSFTPWaitConfirm(WRD_FILEHANDLE) { }
	HSFTPHANDLE hSFTPHandle;
};

struct CSFTPWaitSetStat : public CSFTPWaitConfirm
{
	inline CSFTPWaitSetStat() : CSFTPWaitConfirm(WRD_SETSTAT) { }
	CMyStringW strFileName;
	//bool* pbResult;
};

////////////////////////////////////////////////////////////////////////////////

class CSFTPFolderSFTPDirectory : public CFTPDirectory
{
public:
	CSFTPFolderSFTPDirectory(CDelegateMallocData* pMallocData,
		CFTPDirectoryItem* pItemMe,
		CFTPDirectoryBase* pParent,
		LPCWSTR lpszDirectory);
	virtual ~CSFTPFolderSFTPDirectory();

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv) { return CFTPDirectoryBase::QueryInterface(riid, ppv); }

public:
	STDMETHOD(CreateInstance)(CFTPDirectoryItem* pItemMe, CFTPDirectoryBase* pParent,
		LPCWSTR lpszDirectory, CFTPDirectoryBase** ppResult) override;
	STDMETHOD_(void, UpdateItem)(CFTPFileItem* pOldItem, LPCWSTR lpszNewItem, LONG lEvent) override;

protected:
	CSFTPFolderSFTPDirectory(CDelegateMallocData* pMallocData, CFTPDirectoryItem* pItemMe, ITypeInfo* pInfo);
};

class CSFTPFolderSFTP : public CFTPDirectoryRoot,
	public CSFTPChannelListener,
	public CSSH2FingerPrintHandler,
	public CPumpMessageProcessor
{
public:
	friend class CSFTPFolderSFTPDirectory;

	CSFTPFolderSFTP(CDelegateMallocData* pMallocData, CFTPDirectoryItem* pItemMe, CEasySFTPFolderRoot* pFolderRoot);
	virtual ~CSFTPFolderSFTP();

	// IUnknown
public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv) { return CFTPDirectoryRootBase::QueryInterface(riid, ppv); }

#ifdef _DEBUG
	STDMETHOD(CompareIDs)(LPARAM lParam, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2)
	{
		return CFTPDirectoryRootBase::CompareIDs(lParam, pidl1, pidl2);
	}
#endif

	// CFTPDirectoryRootBase
public:
	STDMETHOD(GetFTPItemUIObjectOf)(HWND hWndOwner, CFTPDirectoryBase* pDirectory,
		const CMyPtrArrayT<CFTPFileItem>& aItems, CFTPDataObject** ppObject);
	STDMETHOD(SetFTPItemNameOf)(HWND hWnd, CFTPDirectoryBase* pDirectory,
		CFTPFileItem* pItem, LPCWSTR pszName, SHGDNF uFlags);
	STDMETHOD(RenameFTPItem)(LPCWSTR lpszSrcFileName, LPCWSTR lpszNewFileName, CMyStringW* pstrMsg);
	STDMETHOD(UpdateFTPItemAttribute)(CFTPDirectoryBase* pDirectory, const CServerFileAttrData* pAttr, CMyStringW* pstrMsg = NULL);
	STDMETHOD(CreateFTPDirectory)(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszName);
	STDMETHOD(CreateShortcut)(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszName, LPCWSTR lpszLinkTo, bool bHardLink);
	STDMETHOD(CreateFTPItemStream)(CFTPDirectoryBase* pDirectory, CFTPFileItem* pItem, IStream** ppStream);
	STDMETHOD(WriteFTPItem)(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszName, IStream* pStream,
		void* pvObject, CTransferStatus* pStatus);
	STDMETHOD(CreateInstance)(CFTPDirectoryItem* pItemMe, CFTPDirectoryBase* pParent,
		LPCWSTR lpszDirectory, CFTPDirectoryBase** ppResult) override;

	STDMETHOD(get_IsUnixServer)(VARIANT_BOOL* pbRet);
	STDMETHOD(Disconnect)();
	STDMETHOD(IsConnected)() { return m_pClient != NULL ? S_OK : S_FALSE; }
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
			*pDefPort = 22;
		return EasySFTPConnectionMode::SFTP;
	}
	virtual LPCWSTR GetProtocolName() const { return L"sftp"; }
	virtual bool IsLockSupported() const { return m_pChannel && m_pChannel->GetServerVersion() >= 6; }
	virtual void PreShowPropertyDialog(CServerFilePropertyDialog* pDialog);
	// pDialog may be NULL
	// return: whether the protocol supports creation of shortcut (link)
	virtual bool PreShowCreateShortcutDialog(CLinkDialog* pDialog);
	virtual void PreShowServerInfoDialog(CServerInfoDialog* pDialog);
	virtual bool ReceiveDirectory(HWND hWndOwner, CFTPDirectoryBase* pDirectory, LPCWSTR lpszDirectory, bool* pbReceived);
	virtual bool ValidateDirectory(LPCWSTR lpszParentDirectory, PCUIDLIST_RELATIVE pidlChild,
		CMyStringW& rstrRealPath);
	virtual CFTPFileItem* RetrieveFileItem(CFTPDirectoryBase* pDirectory, LPCWSTR lpszFileName);
	virtual ULONG GetServerVersion() { return m_pChannel ? m_pChannel->GetServerVersion() : 0; }

	// CSFTPChannelListener
public:
	virtual void ChannelOpenFailure(CSSHChannel* pChannel, int nReason);
	virtual void ChannelError(CSSHChannel* pChannel, int nReason);
	virtual void ChannelOpened(CSSHChannel* pChannel);
	virtual void ChannelClosed(CSSHChannel* pChannel);
	virtual void ChannelExitStatus(CSSHChannel* pChannel, int nExitCode);
	virtual void ChannelConfirm(CSSHChannel* pChannel, bool bSucceeded, int nReason);
	virtual void ChannelDataReceived(CSSHChannel* pChannel, const void* pvData, size_t nSize)
	{ }
	virtual void SFTPOpened(CSFTPChannel* pChannel);
	virtual void SFTPConfirm(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
		int nStatus, const CMyStringW& strMessage);
	virtual void SFTPFileHandle(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
		HSFTPHANDLE hSFTP);
	virtual void SFTPReceiveFileName(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
		const CSFTPFileData* aFiles, int nCount);
	virtual void SFTPReceiveData(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
		const void* pvData, size_t nLen, const bool* pbEOF);
	virtual void SFTPReceiveAttributes(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
		const CSFTPFileAttribute& attrs);
	virtual void SFTPReceiveStatVFS(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
		const struct sftp_statvfs& statvfs);

	// CFingerPrintHandler
public:
	virtual bool __stdcall CheckFingerPrint(const BYTE* pFingerPrint, size_t nLen);

	// CPumpMessageProcessor
public:
	virtual HRESULT PumpSocketAndMessage(DWORD dwWaitTime = 0xFFFFFFFF);

public:
	virtual bool Connect(HWND hWnd, LPCWSTR lpszHostName, int nPort, IEasySFTPAuthentication2* pUser);
	//void Disconnect();

public:
	enum class Phase : UINT
	{
		First = 0,
		Handshake,
		Authenticating,
		Authenticated,
		WaitingLoggedIn,
		LoggedIn
	};
public:
	//CMyStringW m_strHostName;
	//int m_nPort;
	char m_nServerCharset;

	bool m_bMyUserInfo;
	IEasySFTPAuthentication2* m_pUser;
	CSSH2Client* m_pClient;
	Phase m_phase;
	bool m_bFirstAuthenticate;
	CSFTPChannel* m_pChannel;
	//ULONG m_uDirMsg;
	//CMyStringW m_strReceivingDirPath;
	//HSFTPHANDLE m_hDirFile;
	CMyKeyList<CWaitResponseData*, ULONG> m_listWaitResponse;
	//CMyKeyList<CSFTPSendFileData*, ULONG> m_listSFTPSendFile;
	//CRITICAL_SECTION m_csListSendFile;
	DWORD m_dwTransferringCount;
	//CRITICAL_SECTION m_csTransferringCount;
	//UINT m_uSFTPDirChangeMsgID;
	//UINT m_uSFTPCreateEditLabelMsgID;

	void IncrementTransferCount();
	void DecrementTransferCount();

protected:
	//ULONG m_uRef;
	HWND m_hWndOwner;
	UINT_PTR m_idTimer;
	CRITICAL_SECTION m_csSocket;
	CRITICAL_SECTION m_csReceive;
	bool m_bNextLoop;
	bool m_bReconnect;
	static void CALLBACK KeepConnectionTimerProc(UINT_PTR idEvent, LPARAM lParam);
	void OnSFTPSocketReceive(bool isSocketReceived);
	// return 0 for success, non-zero for failure (if its value is not (UINT) -1, it means message id)
	UINT _OnSFTPSocketReceiveThreadUnsafe(bool isSocketReceived);
	void DoReceiveSocket();
	inline int DoRetryAuthentication(const char* pszAuthList, bool bFirstAttempt)
	{
		return m_pParent->DoRetryAuthentication(m_hWndOwner, m_pUser, EasySFTPConnectionMode::SFTP, pszAuthList, bFirstAttempt);
	}
	void DoNextReadDirectory(CSFTPWaitDirectoryData* pData);
	virtual HRESULT DoDeleteFileOrDirectory(HWND hWndOwner, CMyStringArrayW& astrMsgs, bool bIsDirectory, LPCWSTR lpszFile, CFTPDirectoryBase* pDirectory = NULL);
	HRESULT _StatFile(HANDLE hFile, STATSTG* pStatstg, DWORD grfStatFlag);

	class CSFTPStreamCounter : public IUnknown
	{
	public:
		inline CSFTPFolderSFTP* This() const
		{
			return (CSFTPFolderSFTP*)(((DWORD_PTR)this) - (DWORD_PTR)offsetof(CSFTPFolderSFTP, m_xStreamCounter));
		}
		STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv) { return E_NOINTERFACE; }
		STDMETHOD_(ULONG, AddRef)()
		{
			This()->IncrementTransferCount();
			return 2;
		}
		STDMETHOD_(ULONG, Release)()
		{
			This()->DecrementTransferCount();
			return 1;
		}
	} m_xStreamCounter;

	class CSFTPHandleData
	{
	public:
		CMyStringW strName;
		HSFTPHANDLE hSFTP;
		ULONGLONG offset;
		STATSTG statstg;
		DWORD grfMode;
		DWORD dwRefCount;
		bool bIsEOF;
	};
};
