/*
 Copyright (C) 2010 jet (ジェット)

 MyWindow.cpp - implementations of CMyWindow
 */

#include "StdAfx.h"
#include "MyWindow.h"

#include "AppClass.h"
#include "UString.h"

#ifdef _DEBUG
#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif
#endif

struct CMyWindowData
{
	HWND hWnd;
	CMyWindow* pWnd;
	CMyWindowData* pNext;

	void FreeAllData()
	{
		CMyWindowData* p, * p2;
		p2 = this;
		while (p2)
		{
			p = p2->pNext;
			free(p2);
			p2 = p;
		}
	}
};

typedef BOOL (WINAPI* T_GetMonitorInfoA)(HMONITOR hMonitor, LPMONITORINFO lpmi);
typedef HMONITOR (WINAPI* T_MonitorFromWindow)(HWND hwnd, DWORD dwFlags);
static T_GetMonitorInfoA s_pfnGetMonitorInfoA;
static T_MonitorFromWindow s_pfnMonitorFromWindow;
static bool s_bMonitorFuncInit = false;

static bool __stdcall _InitMonitorFunctions()
{
	if (!s_bMonitorFuncInit)
	{
		HINSTANCE hInstUser32 = ::GetModuleHandle(_T("user32.dll"));
		s_pfnGetMonitorInfoA = (T_GetMonitorInfoA) ::GetProcAddress(hInstUser32, "GetMonitorInfoA");
		s_pfnMonitorFromWindow = (T_MonitorFromWindow) ::GetProcAddress(hInstUser32, "MonitorFromWindow");
		s_bMonitorFuncInit = true;
	}
	return s_pfnGetMonitorInfoA != NULL;
}

EXTERN_C void WINAPI _GetCurrentMonitorRect(HWND hWndBase, LPRECT lpRect)
{
	if (_InitMonitorFunctions())
	{
		MONITORINFO mi;
		mi.cbSize = sizeof(mi);
		s_pfnGetMonitorInfoA(s_pfnMonitorFromWindow(hWndBase, MONITOR_DEFAULTTONEAREST), &mi);
		memcpy(lpRect, &mi.rcWork, sizeof(RECT));
	}
	else
		::GetWindowRect(::GetDesktopWindow(), lpRect);
}

EXTERN_C void WINAPI _CenterWindow(HWND hWnd, HWND hWndBase, LPRECT lpRect)
{
	RECT rc, rcBase, rcDesk;
	int nw, nh;
	if (lpRect != NULL)
		memcpy(&rc, lpRect, sizeof(RECT));
	else
		::GetWindowRect(hWnd, &rc);
	{
		HWND hWndMonitorBase;
		if (hWndBase)
			hWndMonitorBase = hWndBase;
		else
		{
			CMyThread* pApp = GetCurThread();
			if (pApp && pApp->IsWinThread() && ((CMyWinThread*) pApp)->m_pMainWnd)
				hWndMonitorBase = ((CMyWinThread*) pApp)->m_pMainWnd->m_hWnd;
			else
			{
				hWndMonitorBase = ::GetActiveWindow();
				if (!hWndMonitorBase)
					hWndMonitorBase = hWnd;
			}
		}
		_GetCurrentMonitorRect(hWndMonitorBase, &rcDesk);
	}
	if (hWndBase == NULL)
	{
		hWndBase = ::GetDesktopWindow();
		rcBase = rcDesk;
	}
	else
		::GetWindowRect(hWndBase, &rcBase);
	nw = rc.right - rc.left;
	nh = rc.bottom - rc.top;
	int nX = ((rcBase.right - rcBase.left - nw) / 2);
	int nY = ((rcBase.bottom - rcBase.top - nh) / 2);
	rc.left = rcBase.left + nX;
	if (rc.left < rcDesk.left)
		rc.left = rcDesk.left;
	if (rc.left + nw > rcDesk.right)
		rc.left = rcDesk.right - rcDesk.left - nw;
	rc.top = rcBase.top + nY;
	if (rc.top < rcDesk.top)
		rc.top = rcDesk.top;
	if (rc.top + nh > rcDesk.bottom)
		rc.top = rcDesk.bottom - rcDesk.top - nh;
	rc.right = rc.left + nw;
	rc.bottom = rc.top + nh;
	if (lpRect != NULL)
		::memcpy(lpRect, &rc, sizeof(RECT));
	else
		::MoveWindow(hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
}

///////////////////////////////////////////////////////////////////////////////

static CMyWindowData* s_pWndData = NULL;
static DWORD s_dwTLSMessageData = 0;
static DWORD s_dwTLSHookData = 0;
static CRITICAL_SECTION s_csWndData{};
static bool s_bCsWndDataCreated = false;

void WINAPI AddWndHandle(HWND hWnd, CMyWindow* pWnd)
{
	if (!s_bCsWndDataCreated)
	{
		::InitializeCriticalSection(&s_csWndData);
		s_bCsWndDataCreated = true;
	}
	CMyWindowData* pData = (CMyWindowData*) malloc(sizeof(CMyWindowData));
	pData->hWnd = hWnd;
	pData->pWnd = pWnd;
	pData->pNext = NULL;
	::EnterCriticalSection(&s_csWndData);
	CMyWindowData* pFirst = s_pWndData;
	if (pFirst)
	{
		CMyWindowData* p = pFirst;
		while (p->pNext)
			p = p->pNext;
		p->pNext = pData;
	}
	else
		s_pWndData = pData;
	::LeaveCriticalSection(&s_csWndData);
}

void WINAPI RemoveWndHandle(HWND hWnd)
{
	if (!s_bCsWndDataCreated)
		return;

	CMyWindowData* pData, * p;
	::EnterCriticalSection(&s_csWndData);
	pData = s_pWndData;
	p = NULL;
	while (pData)
	{
		if (pData->hWnd == hWnd)
		{
			if (p)
				p->pNext = pData->pNext;
			else
				s_pWndData = pData->pNext;
			free(pData);
			break;
		}
		p = pData;
		pData = pData->pNext;
	}
	::LeaveCriticalSection(&s_csWndData);
}

CMyWindow* WINAPI FindWndHandle(HWND hWnd)
{
	if (!s_bCsWndDataCreated)
		return NULL;

	CMyWindowData* pData;
	::EnterCriticalSection(&s_csWndData);
	pData = s_pWndData;
	while (pData)
	{
		if (pData->hWnd == hWnd)
		{
			::LeaveCriticalSection(&s_csWndData);
			return pData->pWnd;
		}
		pData = pData->pNext;
	}
	::LeaveCriticalSection(&s_csWndData);
	return NULL;
}

extern "C" void WINAPI EndWindowData()
{
	if (s_bCsWndDataCreated)
	{
		::EnterCriticalSection(&s_csWndData);
		CMyWindowData* pData, * pData2;
		pData = s_pWndData;
		while (pData)
		{
			pData2 = pData;
			pData = pData->pNext;
			free(pData2);
		}
		s_pWndData = NULL;
		::LeaveCriticalSection(&s_csWndData);
		::DeleteCriticalSection(&s_csWndData);
	}

	if (s_dwTLSHookData)
		::TlsFree(s_dwTLSHookData);
	if (s_dwTLSMessageData)
		::TlsFree(s_dwTLSMessageData);
}

LRESULT CALLBACK MyWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CMyWindow* pWnd;
	pWnd = FindWndHandle(hWnd);
	if (!pWnd)
		return ::DefWindowProc(hWnd, message, wParam, lParam);
	if (message == WM_INITDIALOG)
		return pWnd->CMyWindow::CallDefWindowProc(message, wParam, lParam);
	return pWnd->_ProcessProc(message, wParam, lParam);
}

extern "C" WNDPROC WINAPI GetMyWndProc()
{
	return (WNDPROC) MyWndProc;
}

//static HHOOK s_hCreationHook = NULL;
//static CMyWindow* s_pCreate = NULL;
struct CHookData
{
	HHOOK hCreationHook;
	CMyWindow* pCreate;
};

static LRESULT CALLBACK _CreationHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (!s_dwTLSHookData)
		return 0;

	CHookData FAR* pData = (CHookData FAR*) ::TlsGetValue(s_dwTLSHookData);
	if (!pData)
		return 0;

	LRESULT lr = ::CallNextHookEx(pData->hCreationHook, nCode, wParam, lParam);
	if (nCode == HCBT_CREATEWND)
	{
		HWND hWnd = (HWND) wParam;
		if (pData->pCreate)
		{
			WNDPROC pfn = (WNDPROC) ::GetWindowLongPtrW(hWnd, GWLP_WNDPROC);
			if (::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
				pfn = (WNDPROC) ::GetWindowLongPtrA(hWnd, GWLP_WNDPROC);
			if (pfn != (WNDPROC) MyWndProc)
			{
				pData->pCreate->Subclass(hWnd);
			}
			else
			{
				pData->pCreate->m_hWnd = hWnd;
				AddWndHandle(hWnd, pData->pCreate);
			}
			//s_pCreate = NULL;
		}

		TCHAR szClass[8];
		::GetClassName(hWnd, szClass, 8);
		if (_tcscmp(szClass, _T("#32770")) == 0)
		{
			LPCBT_CREATEWND lpcc = (LPCBT_CREATEWND) lParam;
			RECT rc = { lpcc->lpcs->x, lpcc->lpcs->y,
				lpcc->lpcs->x + lpcc->lpcs->cx, lpcc->lpcs->y + lpcc->lpcs->cy };
			_CenterWindow(hWnd, lpcc->lpcs->hwndParent, &rc);
			lpcc->lpcs->x = rc.left;
			lpcc->lpcs->y = rc.top;
			lpcc->lpcs->cx = rc.right - rc.left;
			lpcc->lpcs->cy = rc.bottom - rc.top;
		}
		::UnhookWindowsHookEx(pData->hCreationHook);
		::TlsSetValue(s_dwTLSHookData, NULL);
		free(pData);
		//s_hCreationHook = NULL;
	}
	return lr;
}

void __stdcall _StartHook(CMyWindow* pWnd)
{
	//if (s_hCreationHook)
	//	return;
	if (!s_dwTLSHookData)
		s_dwTLSHookData = ::TlsAlloc();
	CHookData FAR* pData = (CHookData FAR*) ::TlsGetValue(s_dwTLSHookData);
	if (pData)
		return;
	pData = (CHookData FAR*) malloc(sizeof(CHookData));
	::TlsSetValue(s_dwTLSHookData, pData);
	pData->pCreate = pWnd;
	pData->hCreationHook = ::SetWindowsHookEx(WH_CBT, (HOOKPROC) _CreationHookProc, MyGetCurrentInstance(), ::GetCurrentThreadId());
}

// returns true when a window was created successfully (in this case, the hook was removed)
bool __stdcall _EndHook()
{
	if (!s_dwTLSHookData)
		return true;

	CHookData FAR* pData = (CHookData FAR*) ::TlsGetValue(s_dwTLSHookData);
	if (!pData)
		return true;
	::TlsSetValue(s_dwTLSHookData, NULL);
	::UnhookWindowsHookEx(pData->hCreationHook);
	//s_hCreationHook = NULL;
	//s_pCreate = NULL;
	free(pData);
	return false;
}

CMyWindow::CMyWindow()
{
	m_hWnd = NULL;
	m_pfnWndProc = NULL;
	m_bUniWindow = false;
	m_bNeedDefaultForReflectMsg = false;
}

CMyWindow::~CMyWindow()
{
	if (m_hWnd)
		DestroyWindow();
}

LRESULT CMyWindow::_ProcessProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	return _ProcessProc2(false, message, wParam, lParam);
}

LRESULT CMyWindow::_ProcessProc2(bool bReflect, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (!s_dwTLSMessageData)
		s_dwTLSMessageData = ::TlsAlloc();

	UINT u = (UINT) PtrToLong(::TlsGetValue(s_dwTLSMessageData));
	::TlsSetValue(s_dwTLSMessageData, LongToPtr(MAKELONG(message, bReflect ? 1 : 0)));
	m_bInReflect = bReflect;
	LRESULT lr = WindowProc(message, wParam, lParam);
	if (message == WM_NCDESTROY)
	{
		RemoveWndHandle(m_hWnd);
		m_hWnd = NULL;

		CMyThread* pApp = GetCurThread();
		if (pApp && pApp->IsWinThread() && ((CMyWinThread*) pApp)->m_pMainWnd == this)
			((CMyWinThread*) pApp)->m_pMainWnd = NULL;
		PostNcDestroy();
	}
	::TlsSetValue(s_dwTLSMessageData, ULongToPtr(u));
	return lr;
}

LRESULT CMyWindow::Default(WPARAM wParam, LPARAM lParam)
{
	UINT uMsg = (UINT) PtrToLong(::TlsGetValue(s_dwTLSMessageData));
	return CallDefWindowProc(LOWORD(uMsg), wParam, lParam);
}

LRESULT CMyWindow::CallDefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT uMsg = (UINT) PtrToLong(::TlsGetValue(s_dwTLSMessageData));
	if (HIWORD(uMsg))
	{
		m_bNeedDefaultForReflectMsg = true;
		return 0;
	}
	return DefWindowProc(message, wParam, lParam);
}

LRESULT CMyWindow::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (m_pfnWndProc)
	{
		if (m_bUniWindow)
			return ::CallWindowProcW(m_pfnWndProc, m_hWnd, message, wParam, lParam);
		else
			return ::CallWindowProcA(m_pfnWndProc, m_hWnd, message, wParam, lParam);
	}
	if (m_bUniWindow)
		return ::DefWindowProcW(m_hWnd, message, wParam, lParam);
	else
		return ::DefWindowProcA(m_hWnd, message, wParam, lParam);
}

void CMyWindow::PostNcDestroy() { }

CMyWindow* WINAPI CMyWindow::FromHandle(HWND hWnd)
{
	return FindWndHandle(hWnd);
}

HWND CMyWindow::CreateExA(DWORD dwExStyle, LPCSTR lpszClassName, LPCSTR lpszWindowName,
	DWORD dwStyle, int x, int y, int cx, int cy,
	HWND hWndParent, HMENU hMenuOrID, HINSTANCE hInstance, LPVOID lpParam)
{
	if (!hInstance)
		hInstance = MyGetCurrentInstance();
	_StartHook(this);
	HWND hWndRet;
	m_bUniWindow = false;
	hWndRet = ::CreateWindowExA(dwExStyle, lpszClassName, lpszWindowName, dwStyle,
		x, y, cx, cy, hWndParent, hMenuOrID, hInstance, lpParam);
	if (!_EndHook())
	{
		PostNcDestroy();
		return NULL;
	}
	return hWndRet;
}

HWND CMyWindow::CreateExW(DWORD dwExStyle, LPCWSTR lpszClassName, LPCWSTR lpszWindowName,
	DWORD dwStyle, int x, int y, int cx, int cy,
	HWND hWndParent, HMENU hMenuOrID, HINSTANCE hInstance, LPVOID lpParam)
{
	if (!hInstance)
		hInstance = MyGetCurrentInstance();
	_StartHook(this);
	HWND hWndRet;
	m_bUniWindow = true;
	hWndRet = ::CreateWindowExW(dwExStyle, lpszClassName, lpszWindowName, dwStyle,
		x, y, cx, cy, hWndParent, hMenuOrID, hInstance, lpParam);
	if (!hWndRet && ::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
	{
		LPCSTR lpC, lpW;
		CMyStringW strC, strW;
		if (!lpszClassName)
			lpC = NULL;
		else
		{
			strC = lpszClassName;
			lpC = strC;
		}
		if (!lpszWindowName)
			lpW = NULL;
		else
		{
			strW = lpszWindowName;
			lpW = strW;
		}
		m_bUniWindow = false;
		hWndRet = ::CreateWindowExA(dwExStyle, lpC, lpW, dwStyle,
			x, y, cx, cy, hWndParent, hMenuOrID, hInstance, lpParam);
	}
	if (!_EndHook())
	{
		PostNcDestroy();
		return NULL;
	}
	return hWndRet;
}

BOOL CMyWindow::DestroyWindow()
{
	if (!m_hWnd)
		return TRUE;
	return ::DestroyWindow(m_hWnd);
}

bool CMyWindow::Attach(HWND hWnd)
{
	if (m_hWnd || !hWnd)
		return false;
	m_hWnd = hWnd;
	m_bUniWindow = (::IsWindowUnicode(hWnd) != 0);
	m_pfnWndProc = NULL;
	AddWndHandle(hWnd, this);
	return true;
}

HWND CMyWindow::Detach()
{
	if (!m_hWnd || m_pfnWndProc)
		return NULL;

	RemoveWndHandle(m_hWnd);
	register HWND hWnd = m_hWnd;
	m_hWnd = NULL;
	return hWnd;
}

bool CMyWindow::Subclass(HWND hWnd)
{
	if (m_hWnd || !hWnd)
		return false;
	m_hWnd = hWnd;
	m_bUniWindow = (::IsWindowUnicode(hWnd) != 0);
	if (m_bUniWindow)
	{
		m_pfnWndProc = (WNDPROC) ::SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)(WNDPROC) MyWndProc);
		if (!m_pfnWndProc && ::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
		{
			m_bUniWindow = false;
			m_pfnWndProc = (WNDPROC) ::SetWindowLongPtrA(hWnd, GWLP_WNDPROC, (LONG_PTR)(WNDPROC) MyWndProc);
		}
	}
	else
		m_pfnWndProc = (WNDPROC) ::SetWindowLongPtrA(hWnd, GWLP_WNDPROC, (LONG_PTR)(WNDPROC) MyWndProc);
	if (!m_pfnWndProc)
	{
		m_hWnd = NULL;
		return false;
	}
	AddWndHandle(hWnd, this);
	return true;
}

HWND CMyWindow::Unsubclass()
{
	if (!m_hWnd || !m_pfnWndProc)
		return NULL;

	if (m_bUniWindow)
		::SetWindowLongPtrW(m_hWnd, GWLP_WNDPROC, (LONG_PTR) m_pfnWndProc);
	else
		::SetWindowLongPtrA(m_hWnd, GWLP_WNDPROC, (LONG_PTR) m_pfnWndProc);
	m_pfnWndProc = NULL;
	RemoveWndHandle(m_hWnd);
	register HWND hWnd = m_hWnd;
	m_hWnd = NULL;
	return hWnd;
}

bool CMyWindow::PreTranslateMessage(LPMSG lpMsg)
{
	return false;
}

bool WINAPI CMyWindow::ReflectMessage(CMyWindow* pWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	LRESULT lr;
	lr = pWnd->_ProcessProc2(true, message, wParam, lParam);
	if (!pWnd->m_bNeedDefaultForReflectMsg)
	{
		*pResult = lr;
		return true;
	}
	return false;
}

bool WINAPI CMyWindow::ReflectControl(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (lParam)
	{
		CMyWindow* pWndReflect = CMyWindow::FromHandle((HWND) lParam);
		if (pWndReflect)
			return ReflectMessage(pWndReflect, WM_COMMAND, wParam, lParam, pResult);
	}
	return false;
}

bool WINAPI CMyWindow::ReflectNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	LPNMHDR lpnmh = (LPNMHDR) lParam;
	if (lpnmh && lpnmh->hwndFrom)
	{
		CMyWindow* pWndReflect = CMyWindow::FromHandle(((LPNMHDR) lParam)->hwndFrom);
		if (pWndReflect)
			return ReflectMessage(pWndReflect, WM_NOTIFY, wParam, lParam, pResult);
	}
	return false;
}

bool WINAPI CMyWindow::ReflectMeasureItem(CMyWindow* pWnd, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	if ((int) (wParam) == ::GetDlgCtrlID(pWnd->m_hWnd))
		return ReflectMessage(pWnd, WM_MEASUREITEM, wParam, lParam, pResult);
	return false;
}

bool WINAPI CMyWindow::ReflectDrawItem(CMyWindow* pWnd, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	if ((int) (wParam) == ::GetDlgCtrlID(pWnd->m_hWnd))
		return ReflectMessage(pWnd, WM_DRAWITEM, wParam, lParam, pResult);
	return false;
}

bool WINAPI CMyWindow::ReflectCompareItem(CMyWindow* pWnd, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	if ((int) (wParam) == ::GetDlgCtrlID(pWnd->m_hWnd))
		return ReflectMessage(pWnd, WM_COMPAREITEM, wParam, lParam, pResult);
	return false;
}

bool WINAPI CMyWindow::ReflectVKeyToItem(CMyWindow* pWnd, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	if (lParam && (HWND) lParam == pWnd->m_hWnd)
		return ReflectMessage(pWnd, WM_VKEYTOITEM, wParam, lParam, pResult);
	return false;
}

bool WINAPI CMyWindow::ReflectDeleteItem(CMyWindow* pWnd, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	if ((int) (wParam) == ::GetDlgCtrlID(pWnd->m_hWnd))
		return ReflectMessage(pWnd, WM_DELETEITEM, wParam, lParam, pResult);
	return false;
}

bool WINAPI CMyWindow::ReflectCharToItem(CMyWindow* pWnd, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	if (lParam && (HWND) lParam == pWnd->m_hWnd)
		return ReflectMessage(pWnd, WM_CHARTOITEM, wParam, lParam, pResult);
	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool CMyWindow::SetWindowTextW(LPCWSTR lpszString)
{
	BOOL b;
	if (m_bUniWindow)
		b = ::SetWindowTextW(m_hWnd, lpszString);
	if (!m_bUniWindow || ::GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
	{
		CMyStringW str(lpszString);
		return (::SetWindowTextA(m_hWnd, str) != 0);
	}
	return (b != 0);
}

bool CMyWindow::IsWindowUnicode() const
{
	if (!m_hWnd)
		return false;
	return m_bUniWindow;
}
