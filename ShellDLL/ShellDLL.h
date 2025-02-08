/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 ShellDLL.h - declarations of CMainDLL and definitions for DLL
 */

#pragma once

#include "resource.h"

#include "MyFunc.h"
#include "Unicode.h"
#include "UString.h"
#include "SUString.h"
#include "Array.h"
#include "KeyList.h"
#include "Unknown.h"
#include "IDList.h"
#include "Func.h"
#include "TextStrm.h"
#include "MySocket.h"
#include "Lock.h"

#include "AppClass.h"
#include "MyWindow.h"
#include "MyDialog.h"

#include "ESFTPFld.h"

template <class T>
inline void __stdcall CallConstructor(T* ptr)
	{ new ((void*) ptr) T(); }

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DECLARE_INTERFACE_IID_(IEasySFTPInternal, IUnknown, "AD29C042-B9E3-4638-9DF6-D7DA5B8D0199")
{
	// *** IUnknown methods ***
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppv) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;

	// *** IEasySFTPInternal methods ***
	STDMETHOD(SetEmulateRegMode)(THIS_ bool bEmulate) PURE;
};

EXTERN_C const IID IID_IEasySFTPInternal;

#if !defined(NTDDI_WIN7) || (NTDDI_VERSION < NTDDI_WIN7)
EXTERN_C const GUID FAR SID_SInPlaceBrowser;
#endif

#ifndef CWM_GETISHELLBROWSER
#define CWM_GETISHELLBROWSER  (WM_USER + 7)
#endif

// CMF_DVFILE: reserved flags for IContextMenu::QueryContextMenu
//   - represents the menu is in 'File' menu (used by CDefView)
//   (see http://www.ureader.com/msg/1660149.aspx)
#ifndef CMF_DVFILE
#define CMF_DVFILE       0x10000
#endif

#include <pshpack1.h>
// for CSFTPCommandItem and CSFTPHostItem
#define SFTP_HOST_ITEM_SIGNATURE   0xE5C1
// for CSFTPFileItem
#define SFTP_FILE_ITEM_SIGNATURE   0xE5F1

struct CSFTPRootItem
{
	WORD cbSize;
	WORD wOuter;
	WORD cbInner;
	USHORT uSignature;
};

struct CSFTPCommandItem : public CSFTPRootItem
{
	WORD wID;
	WORD wReserved;
};

struct CSFTPHostItem : public CSFTPRootItem
{
	union
	{
		bool bSFTP;
		WORD wDummy;
	};
	WORD nPort;
	WCHAR wchHostName[1];
};

struct CSFTPFileItem
{
	WORD cbSize;
	WORD wOuter;
	WORD cbInner;
	USHORT uSignature;
	union
	{
		struct
		{
			WORD bHasAttribute : 1;
			WORD bIsDirectory : 1;
			WORD bIsHidden : 1;
			WORD bIsShortcut : 1;
		};
		WORD _padding;
	};
	WORD _padding2;
	WCHAR wchFileName[1];
};
#include <poppack.h>
//static_assert(offsetof(CSFTPFileItem, _padding2) == 10,
//	"Unexpected alignment");

#include "FileList.h"

void LogWin32LastError(const WCHAR* pszFuncName);

LPWSTR __stdcall DuplicateCoMemString(const CMyStringW& string);

STDAPI MyCreateShellItem(PCIDLIST_ABSOLUTE pidl, IShellItem** ppItem);

EXTERN_C PITEMID_CHILD __stdcall CreateRootCommandItem(IMalloc* pMalloc, WORD wID);
EXTERN_C PITEMID_CHILD __stdcall CreateHostItem(IMalloc* pMalloc, bool bSFTPMode, WORD nPort, LPCWSTR lpszHostName);
EXTERN_C PITEMID_CHILD __stdcall CreateFileItem(IMalloc* pMalloc, CFTPFileItem* pItem);
EXTERN_C PITEMID_CHILD __stdcall CreateDummyFileItem(IMalloc* pMalloc, LPCWSTR lpszFileName, bool bIsDirectory);
EXTERN_C PIDLIST_RELATIVE __stdcall CreateFullPathFileItem(IMalloc* pMalloc, LPCWSTR lpszFileName);
extern "C++" bool __stdcall PickupRootCommandItemID(PCUITEMID_CHILD pidlHostItem, WORD& rwID);
extern "C++" bool __stdcall PickupHostName(PCUITEMID_CHILD pidlHostItem, CMyStringW& rstrHostName);
extern "C++" bool __stdcall PickupFileName(PCUITEMID_CHILD pidlFileItem, CMyStringW& rstrFileName);

STDAPI MyCreateThumbnailProviderFromFileName(LPCWSTR lpszFileName, IThumbnailProvider** ppProvider);

class CDelegateMallocData : public CUnknownImpl
{
public:
	CDelegateMallocData() : pMalloc(NULL) { }
	virtual ~CDelegateMallocData() { if (pMalloc) pMalloc->Release(); }
	IMalloc* pMalloc;
};

////////////////////////////////////////////////////////////////////////////////

union SHFILEINFO_UNION
{
	SHFILEINFOA a;
	SHFILEINFOW w;
};

////////////////////////////////////////////////////////////////////////////////

#define KEEP_CONNECTION_TIME_SPAN  12000
// milliseconds
#define WAIT_RECEIVE_TIME          10000

#define DEFAULT_CHMOD_COMMAND      L"site chmod"

// generic invalid characters for server file name
#define INVALID_SERVER_FILE_NAME_CHARS       L"*?\\/:;\"|<>"
// allow wildcards
#define INVALID_SERVER_FILE_NAME_CHARS_AW    L"\\/:;\"|<>"
// allow unix path delimiter '/'
#define INVALID_SERVER_FILE_NAME_CHARS_AP    L"*?\\:;\"|<>"
// allow unix path delimiter '/' and wildcards
#define INVALID_SERVER_FILE_NAME_CHARS_APW   L"\\:;\"|<>"
// allow unix path delimiter '/' and windows path delimiter '\\'
#define INVALID_SERVER_FILE_NAME_CHARS_APP   L"*?:;\"|<>"
// allow unix path delimiter '/', windows path delimiter '\\', and wildcards
#define INVALID_SERVER_FILE_NAME_CHARS_APPW  L":;\"|<>"

struct CHostSettings
{
	bool bSFTPMode;
	CMyStringW strDisplayName;
	CMyStringW strHostName;
	int nPort;
	//CMyStringW strUserName;
	CMyStringW strInitLocalPath;
	CMyStringW strInitServerPath;
	BYTE bTextMode;
	char nServerCharset;
	char nTransferMode;
	CMyStringArrayW arrTextFileType;
	bool bUseSystemTextFileType;
	bool bAdjustRecvModifyTime;
	bool bAdjustSendModifyTime;
	bool bUseThumbnailPreview;
	CMyStringW strChmodCommand;
	//CMyStringW strTouchCommand;

	CHostSettings() { }
	CHostSettings(const CHostSettings& settings)
		: bSFTPMode(settings.bSFTPMode)
		, strDisplayName(settings.strDisplayName)
		, strHostName(settings.strHostName)
		, nPort(settings.nPort)
		//, strUserName(settings.strUserName)
		, strInitLocalPath(settings.strInitLocalPath)
		, strInitServerPath(settings.strInitServerPath)
		, bTextMode(settings.bTextMode)
		, nServerCharset(settings.nServerCharset)
		, nTransferMode(settings.nTransferMode)
		, bUseSystemTextFileType(settings.bUseSystemTextFileType)
		, bAdjustRecvModifyTime(settings.bAdjustRecvModifyTime)
		, bAdjustSendModifyTime(settings.bAdjustSendModifyTime)
		, strChmodCommand(settings.strChmodCommand)
		//, strTouchCommand(settings.strTouchCommand)
	{
		arrTextFileType.CopyArray(settings.arrTextFileType);
	}
	void Copy(const CHostSettings& settings)
	{
		bSFTPMode = settings.bSFTPMode;
		strDisplayName = settings.strDisplayName;
		strHostName = settings.strHostName;
		nPort = settings.nPort;
		//strUserName = settings.strUserName;
		strInitLocalPath = settings.strInitLocalPath;
		strInitServerPath = settings.strInitServerPath;
		bTextMode = settings.bTextMode;
		nServerCharset = settings.nServerCharset;
		nTransferMode = settings.nTransferMode;
		arrTextFileType.CopyArray(settings.arrTextFileType);
		bUseSystemTextFileType = settings.bUseSystemTextFileType;
		bAdjustRecvModifyTime = settings.bAdjustRecvModifyTime;
		bAdjustSendModifyTime = settings.bAdjustSendModifyTime;
		strChmodCommand = settings.strChmodCommand;
		//strTouchCommand = settings.strTouchCommand;
	}
};

struct CKnownFingerPrint
{
	CMyStringW strHostName;
	BYTE* pFingerPrint;
	size_t nFingerPrintLen;
};

class CFTPDirectoryBase;

struct CFTPDirectoryItem : public CReferenceCountClassBase
{
	CMyStringW strName;
	CFTPDirectoryBase* pDirectory;
};

struct CHostFolderData
{
	bool bSFTPMode;
	// pDirItem->strName is used for host name
	// pDirItem->pDirectory must be CFTPDirectoryRootBase*
	CFTPDirectoryItem* pDirItem;
	int nPort;
	CHostSettings* pSettings;
};

////////////////////////////////////////////////////////////////////////////////

#define IML_INDEX_EASYSFTP    0
#define IML_INDEX_NETDRIVE    1
#define IML_INDEX_NEWHOST     2

#define POPUP_POS_TRANSFER    0
#define POPUP_POS_DROP        1

#define CXMENU_POPUP_HOST     0
#define CXMENU_POPUP_FILEITEM 1
#define CXMENU_POPUP_FILEDIR  2
#define CXMENU_POPUP_ROOTHELP 3

struct CGeneralSettings
{
//	int nSplitterPos;
//	WINDOWPLACEMENT wpFrame;
//	bool bUsePlacement;
//#ifdef _EASYSFTP_USE_VIEWSTATE_STREAM
//	IStream* pStreamViewState;
//#endif

	inline void Construct()
	{
//#ifdef _EASYSFTP_USE_VIEWSTATE_STREAM
//		pStreamViewState = NULL;
//#endif
	}

	inline void Destruct()
	{
//#ifdef _EASYSFTP_USE_VIEWSTATE_STREAM
//		if (pStreamViewState)
//			pStreamViewState->Release();
//#endif
	}

	inline CGeneralSettings() { Construct(); }
	inline ~CGeneralSettings() { Destruct(); }
};

class CEasySFTPFolderRoot;

typedef void (CALLBACK* PFNEASYSFTPTIMERPROC)(UINT_PTR idTimer, LPARAM lParam);

class CMainDLL : public CMyDLLApplication
{
public:
	CMainDLL();

	static const TCHAR s_szCFFTPData[];
	static const WCHAR s_szHostINIFile[];

	virtual bool InitInstance();
	virtual int ExitInstance();
	// currently does nothing
	bool MyPumpMessage();
	// processes the messages: if WM_QUIT has come, then return false without removing WM_QUIT from the queue
	bool MyPumpMessage2();
	bool CheckQueueMessage();

	static void __stdcall EmptyKnownFingerPrints(CMyPtrArrayT<CKnownFingerPrint>& aKnownFingerPrints);
	static void __stdcall EmptyHostSettings(CMyPtrArrayT<CHostSettings>& aHostSettings);
	void LoadINISettings(
		CGeneralSettings* pSettings,
		CMyPtrArrayT<CKnownFingerPrint>* paKnownFingerPrints,
		CMyPtrArrayT<CHostSettings>* paHostSettings
		);
	void SaveINISettings(
		CGeneralSettings* pSettings,
		CMyPtrArrayT<CKnownFingerPrint>* paKnownFingerPrints,
		CMyPtrArrayT<CHostSettings>* paHostSettings
		);

	//inline void LoadINISettings() { LoadINISettings(m_settings, m_aKnownFingerPrints, m_aHostSettings); }
	// nCommand: 0 -- update, 1 -- add, 2 -- delete, -1 -- reload and merge only
	void UpdateHostSettings(const CHostSettings* pOldSettings, const CHostSettings* pNewSettings, char nCommand, int nCount = 1);
	//inline void SaveINISettings() { SaveINISettings(m_settings, m_aKnownFingerPrints, m_aHostSettings); }
	// update m_aHosts
	void MergeHostSettings(const CMyPtrArrayT<CHostSettings>& aHostSettings);

	inline CHostFolderData* FindHostFolderData(bool bSFTP, LPCWSTR lpszHost, int nPort)
	{
		::EnterCriticalSection(&m_csHosts);
		CHostFolderData* pRet = FindHostFolderDataUnsafe(bSFTP, lpszHost, nPort);
		::LeaveCriticalSection(&m_csHosts);
		return pRet;
	}
	CHostFolderData* FindHostFolderDataUnsafe(bool bSFTP, LPCWSTR lpszHost, int nPort);

	bool FileDialog(bool bOpen, CMyStringW& rstrFileName, CMyWindow* pWndOwner);
	bool FolderDialog(CMyStringW& rstrDirectoryName, CMyWindow* pWndOwner);
	void MultipleErrorMsgBox(HWND hWndOwner, const CMyStringArrayW& astrMessages);

	UINT_PTR RegisterTimer(DWORD dwSpan, PFNEASYSFTPTIMERPROC pfnTimerProc, LPARAM lParam);
	void UnregisterTimer(UINT_PTR idTimer);

	void PlaceClipboardData(IDataObject* pObject);
	bool CheckClipboardForMoved(IDataObject* pObject, DWORD dwEffects);

	void MyChangeNotify(LONG wEventId, UINT uFlags, PIDLIST_ABSOLUTE pidl1, PIDLIST_ABSOLUTE pidl2);

	void AddReference(CEasySFTPFolderRoot* pRoot);
	void RemoveReference(CEasySFTPFolderRoot* pRoot);

	void GetTemporaryFileName(LPCWSTR lpszFileName, CMyStringW& rstrResult);
	// set the file as 'retreived from foreign source'
	void SetAttachmentLock(LPCWSTR lpszFileName, LPCWSTR lpszURL);

public:
	MSG m_msg;

	CMyStringW m_strINIFile;
	CMyStringW m_strTitle;
	CMyStringW m_strFilter;
	CMyStringW m_strTempDirectory;

	bool m_bEmulateRegMode;
	CGeneralSettings m_settings;

	IImageList* m_pimlSysIconJumbo;
	IImageList* m_pimlSysIconExtraLarge;
	HIMAGELIST m_himlSysIconLarge;
	HIMAGELIST m_himlSysIconSmall;
	HIMAGELIST m_himlIconJumbo;
	HIMAGELIST m_himlIconExtraLarge;
	HIMAGELIST m_himlIconLarge;
	HIMAGELIST m_himlIconSmall;
	enum
	{
		iconIndexSmall = 0,
		iconIndexLarge,
		iconIndexExtraLarge,
		iconIndexJumbo,
		iconIndexMaxCount
	};
	int m_iEasySFTPIconIndex[iconIndexMaxCount];
	int m_iNetDriveIconIndex[iconIndexMaxCount];
	int m_iNewHostIconIndex[iconIndexMaxCount];

	HFONT m_hFontWindow;
	HMENU m_hMenuPopup;
	HMENU m_hMenuContext;

	HWND m_hWndTimer;

	union
	{
		OPENFILENAMEA m_ofnA;
		OPENFILENAMEW m_ofnW;
	};
	union
	{
		BROWSEINFOA m_biA;
		BROWSEINFOW m_biW;
	};
	bool m_bUseOFNUnicode;
	bool m_bUseBIUnicode;

	//CMyPtrArrayT<CHostSettings> m_aHostSettings;
	//CMyPtrArrayT<CKnownFingerPrint> m_aKnownFingerPrints;
	CMyStringArrayW m_arrDefTextFileType;
	CMyStringArrayW m_arrTempFileDirectories;

	// lock count for Class factory
	ULONG m_uCFLock;

	CMyPtrArray m_arrTimers;

	CRITICAL_SECTION m_csHosts;
	CMyPtrArrayT<CHostFolderData> m_aHosts;

	UINT m_nCFShellIDList;
	UINT m_nCFFileContents;
	UINT m_nCFFileDescriptorA;
	UINT m_nCFFileDescriptorW;
	UINT m_nCFPerformedDropEffect;
	UINT m_nCFPreferredDropEffect;
	UINT m_nCFPasteSucceeded;
	UINT m_nCFFTPData;
	CMySimpleArrayT<FORMATETC, const FORMATETC&> m_aFTPDataFormats;
	// no AddRef()/Release()
	IDataObject* m_pObjectOnClipboard;

	bool m_bEnableRootRefs;
	CMyPtrArrayT<CEasySFTPFolderRoot> m_aRootRefs;
	CRITICAL_SECTION m_csRootRefs;
};

extern CMainDLL theApp;
