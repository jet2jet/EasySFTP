/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 MainWnd.h - declarations of CMainWindow, etc.
 */

#pragma once

#include "AddrCBox.h"
#include "LFileVw.h"
//#include "SFileVw.h"
#include "CommndUI.h"

#include "SyncDlg.h"

#ifndef CWM_GETISHELLBROWSER
#define CWM_GETISHELLBROWSER  (WM_USER + 7)
#endif

#define MY_WM_BROWSE_VIEW     (WM_USER + 600)
//#define MY_WM_SOCKETMESSAGE   (WM_USER + 601)
//#define MY_WM_SENDQUEUE       (WM_USER + 602)
#define MY_WM_UPDATESETMENU   (WM_USER + 603)
#define MY_WM_CHANGENOTIFY    (WM_USER + 604)

#define TIMERID_KEEP_CONNECTION    101
#define KEEP_CONNECTION_TIME_SPAN  12000
#define TIMERID_TRANSFER_CHECK     102
#define TRANSFER_CHECK_TIME_SPAN   500

class CMainWindow :
	public CMyWindow,
	public IEasySFTPListener
{
public:
	CMainWindow();
	virtual ~CMainWindow();

	HWND CreateEx();
	virtual void PostNcDestroy();
	virtual bool PreTranslateMessage(LPMSG lpMsg);

	// called from CMainApplication
	bool OnIdle(long lCount);

	// IEasySFTPListener
public:
	STDMETHOD(ChangeLocalDirectory)(LPCWSTR lpszPath);

public:
	// IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
	STDMETHOD_(ULONG, AddRef)() { return InternalAddRef(); }
	STDMETHOD_(ULONG, Release)() { return InternalRelease(); }
	ULONG InternalAddRef();
	ULONG InternalRelease();

	class CBrowser : public CMyWindow,
		public IShellBrowser,
#ifdef _EASYSFTP_USE_ICOMMDLGBROWSER
		public ICommDlgBrowser,
#endif
		public IServiceProvider,
		//public IOleCommandTarget,
		public IInternetHostSecurityManager
	{
	public:
		CBrowser() { }

		virtual CMainWindow* This() const = 0;
		STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();

		// IOleWindow
		STDMETHOD(GetWindow)(HWND* phWnd);
		STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);

		// IShellBrowser
		STDMETHOD(InsertMenusSB)(HMENU hMenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
		STDMETHOD(SetMenuSB)(HMENU hMenuShared, HOLEMENU hOleMenuRes, HWND hWndActiveObject);
		STDMETHOD(RemoveMenusSB)(HMENU hMenuShared);
		STDMETHOD(SetStatusTextSB)(LPCWSTR pszStatusText);
		STDMETHOD(EnableModelessSB)(BOOL fEnable);
		STDMETHOD(TranslateAcceleratorSB)(LPMSG lpMsg, WORD wID);
		//STDMETHOD(BrowseObject)(PCUIDLIST_RELATIVE pidl, UINT wFlags);
		//STDMETHOD(GetViewStateStream)(DWORD grfMode, IStream** ppStrm);
		STDMETHOD(GetControlWindow)(UINT uID, HWND* phWnd);
		STDMETHOD(SendControlMsg)(UINT uID, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pRet);
		//STDMETHOD(QueryActiveShellView)(IShellView** ppshv);
		//STDMETHOD(OnViewWindowActive)(IShellView* pshv);
		STDMETHOD(SetToolbarItems)(LPTBBUTTONSB lpButtons, UINT nButtons, UINT uFlags);

		// IServiceProvider
		STDMETHOD(QueryService)(REFGUID guidService, REFIID riid, void FAR* FAR* ppvObject);

		// IInternetHostSecurityManager
		STDMETHOD(GetSecurityId)(BYTE* pbSecurityId, DWORD* pcbSecurityId, DWORD_PTR dwReserved);
		STDMETHOD(ProcessUrlAction)(DWORD dwAction, BYTE* pPolicy, DWORD cbPolicy, BYTE* pContext,
			DWORD cbContext, DWORD dwFlags, DWORD dwReserved);
		STDMETHOD(QueryCustomPolicy)(REFGUID guidKey, BYTE** ppPolicy, DWORD* pcbPolicy,
			BYTE* pContext, DWORD cbContext, DWORD dwReserved);

		//// IOleCommandTarget
		//STDMETHOD(QueryStatus)(const GUID* pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT* pCmdText);
		//STDMETHOD(Exec)(const GUID* pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANT* pvaIn, VARIANT* pvaOut);

	protected:
		LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	};

	class CBrowserLocal : public CBrowser
	{
	public:
		inline virtual CMainWindow* This() const
			{ return (CMainWindow*) (((DWORD_PTR) this) - (DWORD_PTR) offsetof(CMainWindow, m_xBrowserForLocal)); }

		// IShellBrowser
		STDMETHOD(BrowseObject)(PCUIDLIST_RELATIVE pidl, UINT wFlags);
		STDMETHOD(GetViewStateStream)(DWORD grfMode, IStream** ppStrm);
		STDMETHOD(QueryActiveShellView)(IShellView** ppshv);
		STDMETHOD(OnViewWindowActive)(IShellView* pshv);

#ifdef _EASYSFTP_USE_ICOMMDLGBROWSER
		// ICommDlgBrowser
		STDMETHOD(OnDefaultCommand)(IShellView* ppshv);
		STDMETHOD(OnStateChange)(IShellView* ppshv, ULONG uChange);
		STDMETHOD(IncludeObject)(IShellView* ppshv, PCUITEMID_CHILD pidl);
#endif
	} m_xBrowserForLocal;

	class CBrowserServer : public CBrowser
	{
	public:
		inline virtual CMainWindow* This() const
			{ return (CMainWindow*) (((DWORD_PTR) this) - (DWORD_PTR) offsetof(CMainWindow, m_xBrowserForServer)); }

		// only for CBrowserServer
		STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv)
		{
			if (IsEqualIID(riid, IID_IEasySFTPListener))
				return This()->QueryInterface(riid, ppv);
			return CBrowser::QueryInterface(riid, ppv);
		}

		// IShellBrowser
		STDMETHOD(BrowseObject)(PCUIDLIST_RELATIVE pidl, UINT wFlags);
		STDMETHOD(GetViewStateStream)(DWORD grfMode, IStream** ppStrm);
		STDMETHOD(QueryActiveShellView)(IShellView** ppshv);
		STDMETHOD(OnViewWindowActive)(IShellView* pshv);

#ifdef _EASYSFTP_USE_ICOMMDLGBROWSER
		// ICommDlgBrowser
		STDMETHOD(OnDefaultCommand)(IShellView* ppshv);
		STDMETHOD(OnStateChange)(IShellView* ppshv, ULONG uChange);
		STDMETHOD(IncludeObject)(IShellView* ppshv, PCUITEMID_CHILD pidl);
#endif
	} m_xBrowserForServer;

public:
	ULONG m_uRef;
	HACCEL m_hAccel;
	HMENU m_hMenu;
	HMENU m_hMenuReturnMode;
	int m_nSplitterWidth;
	int m_nAddrButtonsWidth;
	int m_nToolBarHeight;
	int m_nStatusHeight;
	CMyStringW m_strToolTipTextKeep;
	CMyStringW m_strStatusText;
	HICON m_hIconSecure;
	HWND m_hWndFocusSaved;
	HWND m_hWndLastFocus;
	bool m_bWindowCreated;
	bool m_bNoRespondToDDE;
	bool m_bUpdateSetMenu;
	HMENU m_hMenuSet;
	HWND m_hWndViewForMenu;
	HFONT m_hFontWindow;
	UINT m_uIDChangeNotify;
	UINT m_uDpi;

	CMyWindow m_wndAddrButtons;
	CAddressComboBox m_wndAddress;
	CShellFolderFileView m_wndListViewLocal;
	CMyWindow m_wndSplitter;
	CMyWindow m_wndServerAddrButtons;
	//CVirtualAddressComboBox m_wndServerAddress;
	CAddressComboBox m_wndServerAddress;
	CShellFolderFileView m_wndListViewServer;
	CMyWindow m_wndToolBar;
	CMyWindow m_wndStatusBar;
	CSyncDetailDialog m_SyncDetailDialog;
	bool m_bLocalAddressSelChanged;
	bool m_bServerAddressSelChanged;

	UINT m_uLastStatusTextModeID;

	void UpdateCurrentFolder(PCUIDLIST_RELATIVE lpidl);
	HRESULT UpdateCurrentFolderAbsolute(PCUIDLIST_ABSOLUTE lpidl, IShellFolder* pFolder = NULL);
	void UpdateCurrentFolderAbsolute(LPCWSTR lpszPath);
	//void UpdateServerFolder(LPCWSTR lpszPath);
	void UpdateServerFolder(PCUIDLIST_RELATIVE lpidl);
	HRESULT UpdateServerFolderAbsolute(PCUIDLIST_ABSOLUTE lpidl, IShellFolder* pFolder = NULL);
	void UpdateServerFolderAbsolute(LPCWSTR lpszPath);
	void SetServerListenerToMe();
	void OnChangeServerFolderFailed();
	void NavigateParentFolder();
	void NavigateServerParentFolder();
	HRESULT QueryActiveShellView(bool bServer, IShellView** ppshv);
	void OnViewWindowActive(bool bServer, IShellView* pView);

	//void DoHostConnect();
	void DoHostConnect(bool bServer);
	void DoConnect();
	//void DoCloseConnection(bool bForce = false);
	void DoCloseConnection(bool bServer, bool bForce = false);
	//char DoRetryAuthentication(const char* pszAuthListMultiStr = NULL, bool bFirstAttempt = false);
	void DoDownload();
	void DoUpload();
	void DoDownloadAll();
	void DoUploadAll();
	void DoSyncLeftToRight();
	void DoSyncRightToLeft();
	void DoSyncDetail();

	void DeleteSelection();
	void ShowOption();

	void SetStatusText(LPCWSTR lpszStatusText);
	void SetStatusText(UINT uStatusID, LPCWSTR lpszStatusText);
	void SetStatusSecureIcon(bool bSecure);

protected:
	void ShowAboutDialog();
	void OnResize();
	void UpdateUIItem(CCommandUIItem* pUIItem);
	void UpdateFonts();
	void UpdateToolBarEnable();
	void UpdateToolBarIcons();
	void UpdateStatusParts();
	void UpdateFileSelection();
	void UpdateViewStatus(HWND hWndFocus);
	void CheckFileTypes(CShellFolderFileView* pViewCur, CShellFolderFileView* pViewOther,
		PIDLIST_ABSOLUTE* ppidlItems, int nCount, int& nTextCount, int& nDirCount);

	void _SetTransferMode(LONG nTransferMode);
	void _SetTextMode(bool bServer, LONG nTextMode);

	bool CanConnect(bool bServer);
	bool CanDisconnect(bool bServer);

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
	LRESULT OnClose(WPARAM wParam, LPARAM lParam);
	LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);
	LRESULT OnSize(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnInitMenuPopup(WPARAM wParam, LPARAM lParam);
	LRESULT OnMenuSelect(WPARAM wParam, LPARAM lParam);
	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnActivate(WPARAM wParam, LPARAM lParam);
	LRESULT OnSettingChange(WPARAM wParam, LPARAM lParam);
	LRESULT OnDpiChanged(WPARAM wParam, LPARAM lParam);
	LRESULT OnToolTipDispInfoA(WPARAM wParam, LPARAM lParam);
	LRESULT OnToolTipDispInfoW(WPARAM wParam, LPARAM lParam);
	LRESULT OnToolBarDropDown(WPARAM wParam, LPARAM lParam);
	void OnLocalAddressTextReturn(LPCWSTR lpszText);
	LRESULT OnLocalAddressEndEditA(WPARAM wParam, LPARAM lParam);
	LRESULT OnLocalAddressEndEditW(WPARAM wParam, LPARAM lParam);
	LRESULT OnLocalAddressSelChange(WPARAM wParam, LPARAM lParam);
	LRESULT OnLocalAddressCloseUp(WPARAM wParam, LPARAM lParam);
	void OnServerAddressTextReturn(LPCWSTR lpszText);
	LRESULT OnServerAddressEndEditA(WPARAM wParam, LPARAM lParam);
	LRESULT OnServerAddressEndEditW(WPARAM wParam, LPARAM lParam);
	LRESULT OnServerAddressSelChange(WPARAM wParam, LPARAM lParam);
	LRESULT OnServerAddressCloseUp(WPARAM wParam, LPARAM lParam);
	//LRESULT OnServerListViewDblClick(WPARAM wParam, LPARAM lParam);
	//LRESULT OnServerListViewReturn(WPARAM wParam, LPARAM lParam);
	//LRESULT OnServerListViewBeginLabelEdit(WPARAM wParam, LPARAM lParam);
	//LRESULT OnServerListViewEndLabelEdit(WPARAM wParam, LPARAM lParam);
	//LRESULT OnSocketMessage(WPARAM wParam, LPARAM lParam);
	//LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
	//LRESULT OnSendQueue(WPARAM wParam, LPARAM lParam);
	LRESULT OnBrowseView(WPARAM wParam, LPARAM lParam);
	LRESULT OnUpdateSetMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnChangeNotify(WPARAM wParam, LPARAM lParam);

	LRESULT OnSplitterTrack(WPARAM wParam, LPARAM lParam);
	LRESULT OnSplitterTracking(WPARAM wParam, LPARAM lParam);

	LRESULT OnDDEInitialize(WPARAM wParam, LPARAM lParam);
	LRESULT OnDDETerminate(WPARAM wParam, LPARAM lParam);
	LRESULT OnDDEExecute(WPARAM wParam, LPARAM lParam);
};
