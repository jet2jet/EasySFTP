/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 EasySFTP.cpp - declarations of CMainApplication and definitions for application
 */

#pragma once

#include "resource.h"

#include "AppClass.h"
#include "MyWindow.h"
#include "MyDialog.h"
#include "MyProp.h"
#include "MyFunc.h"
#include "Splitter.h"

#include "Unicode.h"
#include "UString.h"
#include "SUString.h"
#include "Array.h"
#include "KeyList.h"
#include "Func.h"
//#include "MySocket.h"
//#include "TextStrm.h"
//#include "FileStrm.h"
#include "Unknown.h"

#include "ESFTPFld.h"
#include "../ShellDLL/EasySFTP_h.h"

EXTERN_C BOOL MyFileIconInit(BOOL fRestoreCache);

DECLARE_INTERFACE_IID_(IEasySFTPInternal, IUnknown, "AD29C042-B9E3-4638-9DF6-D7DA5B8D0199")
{
	// *** IUnknown methods ***
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppv) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;

	// *** IEasySFTPInternal methods ***
	STDMETHOD(SetEmulateRegMode)(THIS_ bool bEmulate) PURE;
};

template <class T>
inline void __stdcall CallConstructor(T* ptr)
	{ new ((void*) ptr) T(); }

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// in RegHook.cpp
EXTERN_C bool __stdcall InitRegHook();
EXTERN_C void __stdcall TermRegHook();

// from MySocket.h
enum ServerCharset
{
	scsUTF8 = 0,
	scsShiftJIS,
	scsEUC
};

////////////////////////////////////////////////////////////////////////////////

union SHFILEINFO_UNION
{
	SHFILEINFOA a;
	SHFILEINFOW w;
};

////////////////////////////////////////////////////////////////////////////////

#define POPUP_POS_RETURNMODE            0
#define POPUP_POS_TRANSFER              1
//#define POPUP_POS_SERVERLISTVIEW        2
//#define POPUP_POS_SERVERLISTVIEW_NOSEL  3
//#define POPUP_POS_SERVERLISTVIEW_DROP   4
//
//#define DEFAULT_CHMOD_COMMAND      L"chmod"
//
//// generic invalid characters for server file name
//#define INVALID_SERVER_FILE_NAME_CHARS       L"*?\\/:;\"|<>"
//// allow wildcards
//#define INVALID_SERVER_FILE_NAME_CHARS_AW    L"\\/:;\"|<>"
//// allow unix path delimiter '/'
//#define INVALID_SERVER_FILE_NAME_CHARS_AP    L"*?\\:;\"|<>"
//// allow unix path delimiter '/' and wildcards
//#define INVALID_SERVER_FILE_NAME_CHARS_APW   L"\\:;\"|<>"
//// allow unix path delimiter '/' and windows path delimiter '\\'
//#define INVALID_SERVER_FILE_NAME_CHARS_APP   L"*?:;\"|<>"
//// allow unix path delimiter '/', windows path delimiter '\\', and wildcards
//#define INVALID_SERVER_FILE_NAME_CHARS_APPW  L":;\"|<>"

////////////////////////////////////////////////////////////////////////////////

struct CMRUStreamData
{
	PIDLIST_ABSOLUTE pidl;
	PBYTE pbData;
	SIZE_T nSize;
};

////////////////////////////////////////////////////////////////////////////////

class CMainApplication : public CMyApplication
{
public:
	CMainApplication();
	virtual ~CMainApplication();

	static const TCHAR s_szCFFTPData[];
	//static const TCHAR s_szCFFTPRenameFlag[];
	static const WCHAR s_szMainWndClass[];
	static const WCHAR s_szViewParentWndClass[];
	static const WCHAR s_szLocalViewStateFile[];
	static const WCHAR s_szServerViewStateFile[];

	enum
	{
		paramRegister = 0x0001,
		paramUnregister = 0x0002,
		paramNextIsLocalPath = 0x0004,
		paramNextIsServerPath = 0x0008
	};

public:
	virtual bool InitInstance();
	virtual int ExitInstance();
	virtual bool OnIdle(long lCount);

public:
	bool InitRegistryHook();
	bool InitSystemLibraries();
	bool InitEasySFTP();
	bool InitGraphics();
	bool InitWindowClasses();
	bool InitAppData();
	int ParseCommandLine();
	void CheckCommandParameter(LPCWSTR lpszParam, int& nCurrentStatus);

public:
	HIMAGELIST m_hImageListFileIcon;
	HIMAGELIST m_hImageListToolBar;
	HIMAGELIST m_hImageListToolBarL;
	HIMAGELIST m_hImageListAddrButtons;
	HIMAGELIST m_hImageListAddrButtonsL;
	HMENU m_hMenuPopup;
	//IStream* m_pStreamViewStateLocal;
	//IStream* m_pStreamViewStateServer;
	CMyPtrArrayT<CMRUStreamData> m_aLocalStreams;
	CMyPtrArrayT<CMRUStreamData> m_aServerStreams;

	CMyStringW m_strFirstLocalPath;
	CMyStringW m_strFirstServerPath;
	bool m_bExitWithRegister;
	bool m_bIsRegisterForSystem;
	bool m_bUnregisterOperation;
	bool m_bNoRestart;
	bool m_bNeedEmulationMode;
#ifndef _WIN64
	bool m_bIsWin9x;
#endif

	class CMRUStream : public IStream
	{
	public:
		CMRUStream(IStream* pStreamBase, CMRUStreamData* pData);
		~CMRUStream();

	public:
		STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject)
		{
			if (!ppvObject)
				return E_POINTER;
			if (IsEqualIID(riid, IID_IUnknown) ||
				IsEqualIID(riid, IID_ISequentialStream) ||
				IsEqualIID(riid, IID_IStream))
			{
				*ppvObject = (IStream*) this;
				AddRef();
				return S_OK;
			}
			*ppvObject = NULL;
			return E_NOINTERFACE;
		}
		STDMETHOD_(ULONG, AddRef)()
			{ return ++m_uRef; }
		STDMETHOD_(ULONG, Release)()
		{
			ULONG u = --m_uRef;
			if (!u)
				delete this;
			return u;
		}

		// ISequentialStream Interface
	public:
		STDMETHOD(Read)(void* pv, ULONG cb, ULONG* pcbRead)
			{ return m_pStream->Read(pv, cb, pcbRead); }
		STDMETHOD(Write)(void const* pv, ULONG cb, ULONG* pcbWritten)
			{ return m_pStream->Write(pv, cb, pcbWritten); }

		// IStream Interface
	public:
		STDMETHOD(SetSize)(ULARGE_INTEGER libNewSize)
			{ return m_pStream->SetSize(libNewSize); }
		STDMETHOD(CopyTo)(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten)
			{ return m_pStream->CopyTo(pstm, cb, pcbRead, pcbWritten); }
		STDMETHOD(Commit)(DWORD grfCommitFlags)
			{ return m_pStream->Commit(grfCommitFlags); }
		STDMETHOD(Revert)()
			{ return m_pStream->Revert(); }
		STDMETHOD(LockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
			{ return m_pStream->LockRegion(libOffset, cb, dwLockType); }
		STDMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
			{ return m_pStream->LockRegion(libOffset, cb, dwLockType); }
		STDMETHOD(Clone)(IStream** ppstm)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(Seek)(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer)
			{ return m_pStream->Seek(liDistanceToMove, dwOrigin, lpNewFilePointer); }
		STDMETHOD(Stat)(STATSTG* pStatstg, DWORD grfStatFlag)
			{ return m_pStream->Stat(pStatstg, grfStatFlag); }

	protected:
		ULONG m_uRef;
		CMRUStreamData* m_pData;
		IStream* m_pStream;
	};

	CMyStringW m_strINIFile;
	CMyStringW m_strTempPath;
	CMyStringW m_strLocalViewStateFile;
	CMyStringW m_strServerViewStateFile;
	CMyStringW m_strTitle;
	CMyStringW m_strFilter;

	UINT m_nCFShellIDList;
	//UINT m_nCFFileContents;
	//UINT m_nCFFileDescriptorA;
	//UINT m_nCFFileDescriptorW;
	//UINT m_nCFPerformedDropEffect;
	//UINT m_nCFPreferredDropEffect;
	//UINT m_nCFPasteSucceeded;
	//UINT m_nCFFTPData;
	////UINT m_nCFFTPRenameFlag;
	//CMySimpleArrayT<FORMATETC, const FORMATETC&> m_aFTPDataFormats;

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
	bool m_bUsePlacement;
	WINDOWPLACEMENT m_wpFrame;
	int m_nSplitterPos;

	//CMySimpleArray<CHostSettings*> m_aHostSettings;
	//CMySimpleArray<CKnownFingerPrint*> m_aKnownFingerPrints;
	//CMyStringArrayW m_arrDefTextFileType;
	//CMySimpleArray<void*> m_aObjectTransferring;
	//CMyStringKeyListW<UINT> m_mapIcon;
	IEasySFTPOldRoot* m_pEasySFTPRoot;
	PIDLIST_ABSOLUTE m_pidlEasySFTP;
	bool m_bEmulatingRegistry;
	bool m_bIsRegisteredAsUserClass;
	LONG m_uRefThread;
	IUnknown* m_pUnkThreadRef;

public:
	int GetImageListIconIndex(IExtractIconA* pIcon);
	int GetImageListIconIndex(IExtractIconW* pIcon);
	int GetImageListIconIndex(LPCWSTR lpszFileName, DWORD dwAttributes);
	bool FileDialog(bool bOpen, CMyStringW& rstrFileName, CMyWindow* pWndOwner);
	bool FolderDialog(CMyStringW& rstrDirectoryName, CMyWindow* pWndOwner);
	void GetTempFile(LPCWSTR lpszFileName, CMyStringW& rstrFullPath);
	void DoAutoComplete(HWND hWndEdit, IEnumString* pEnumString = NULL);
	HRESULT GetViewStateStream(bool bServer, PCIDLIST_ABSOLUTE pidlCurrent, DWORD grfMode, IStream** ppStream);

	bool CheckExternalApplications();
	void DoRegister();

protected:
	void LoadINISettings();
	void SaveINISettings();
};

extern CMainApplication theApp;
