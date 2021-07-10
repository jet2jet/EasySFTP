/*
 Copyright (C) 2010 Kuri-Applications

 MyWindow.h - declarations of CMyWindow and some macros for implementation
 */

#pragma once

extern "C" WNDPROC WINAPI GetMyWndProc();

class CMyWindow
{
public:
	CMyWindow();
	virtual ~CMyWindow();

public:
	static CMyWindow* WINAPI FromHandle(HWND hWnd);

	HWND CreateExA(DWORD dwExStyle, LPCSTR lpszClassName, LPCSTR lpszWindowName,
		DWORD dwStyle, int x, int y, int cx, int cy,
		HWND hWndParent, HMENU hMenuOrID, HINSTANCE hInstance = NULL, LPVOID lpParam = NULL);
	HWND CreateExW(DWORD dwExStyle, LPCWSTR lpszClassName, LPCWSTR lpszWindowName,
		DWORD dwStyle, int x, int y, int cx, int cy,
		HWND hWndParent, HMENU hMenuOrID, HINSTANCE hInstance = NULL, LPVOID lpParam = NULL);
	BOOL DestroyWindow();

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) { return Default(wParam, lParam); }

	LRESULT Default(WPARAM wParam, LPARAM lParam);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();
	// return false if TranslateMessage/DispatchMessage is needed
	virtual bool PreTranslateMessage(LPMSG lpMsg);
	static bool WINAPI ReflectMessage(CMyWindow* pWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	static bool WINAPI ReflectControl(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	static bool WINAPI ReflectNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	static bool WINAPI ReflectMeasureItem(CMyWindow* pWnd, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	static bool WINAPI ReflectDrawItem(CMyWindow* pWnd, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	static bool WINAPI ReflectCompareItem(CMyWindow* pWnd, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	static bool WINAPI ReflectVKeyToItem(CMyWindow* pWnd, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	static bool WINAPI ReflectDeleteItem(CMyWindow* pWnd, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	static bool WINAPI ReflectCharToItem(CMyWindow* pWnd, WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	bool Attach(HWND hWnd);
	HWND Detach();
	bool Subclass(HWND hWnd);
	HWND Unsubclass();

	operator HWND() const { return m_hWnd; }
	HWND GetSafeHwnd() const { return this ? m_hWnd : NULL; }
	bool SetWindowTextW(LPCWSTR lpszString);
	bool IsWindowUnicode() const;

public:
	HWND m_hWnd;

protected:
	WNDPROC m_pfnWndProc;
	bool m_bUniWindow;
	bool m_bInReflect;
	bool m_bNeedDefaultForReflectMsg;

	friend LRESULT CALLBACK MyWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT _ProcessProc(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT _ProcessProc2(bool bReflect, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CallDefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};

#define HANDLE_PROC_MESSAGE(h_msg, h_proc) \
	if (!m_bInReflect && message == h_msg) return h_proc(wParam, lParam)
#define HANDLE_COMMAND(hc_id, hc_proc) \
	if (!m_bInReflect && message == WM_COMMAND && LOWORD(wParam) == hc_id) \
		return hc_proc(wParam, lParam)
#define HANDLE_CONTROL(hc_id, hc_code, hc_proc) \
	if (!m_bInReflect && message == WM_COMMAND && LOWORD(wParam) == hc_id && HIWORD(wParam) == hc_code) \
		return hc_proc(wParam, lParam)
#define HANDLE_NOTIFY(hc_id, hn_code, hn_proc) \
	if (!m_bInReflect && message == WM_NOTIFY && (UINT) (wParam) == hc_id && ((LPNMHDR) lParam)->code == hn_code) \
		return hn_proc(wParam, lParam)
#define HANDLE_NOTIFY_WND(hc_hwnd, hn_code, hn_proc) \
	if (!m_bInReflect && message == WM_NOTIFY && ((LPNMHDR) lParam)->hwndFrom == hc_hwnd && ((LPNMHDR) lParam)->code == hn_code) \
		return hn_proc(wParam, lParam)
#define HANDLE_NOTIFY_CODE(hn_code, hn_proc) \
	if (!m_bInReflect && message == WM_NOTIFY && ((LPNMHDR) lParam)->code == hn_code) \
		return hn_proc(wParam, lParam)
#define HANDLE_REFLECT_MESSAGE(hr_msg, hr_proc) \
	if (m_bInReflect && message == hr_msg) return hr_proc(wParam, lParam)
#define HANDLE_REFLECT_CONTROL(hr_code, hr_proc) \
	if (m_bInReflect && message == WM_COMMAND && LOWORD(wParam) == hr_code) \
		return hr_proc(wParam, lParam)
#define HANDLE_REFLECT_NOTIFY(hr_code, hr_proc) \
	if (m_bInReflect && message == WM_NOTIFY && ((LPNMHDR) lParam)->code == hr_code) \
		return hr_proc(wParam, lParam)
#define REFLECT_CONTROL(r_id) \
	if (message == WM_COMMAND && lParam && LOWORD(wParam) == r_id) \
	{ \
		LRESULT lrrf; \
		if (ReflectControl(wParam, lParam, &lrrf)) return lrrf; \
	}
#define REFLECT_CONTROL_WND(r_hwnd) \
	if (message == WM_COMMAND && lParam && (HWND) lParam == r_hwnd) \
	{ \
		LRESULT lrrf; \
		if (ReflectControl(wParam, lParam, &lrrf)) return lrrf; \
	}
#define REFLECT_NOTIFY(r_id) \
	if (message == WM_NOTIFY && lParam && ((LPNMHDR) lParam)->idFrom == r_id) \
	{ \
		LRESULT lrrf; \
		if (ReflectNotify(wParam, lParam, &lrrf)) return lrrf; \
	}
#define REFLECT_NOTIFY_WND(r_hwnd) \
	if (message == WM_NOTIFY && lParam && ((LPNMHDR) lParam)->hwndFrom == r_hwnd) \
	{ \
		LRESULT lrrf; \
		if (ReflectNotify(wParam, lParam, &lrrf)) return lrrf; \
	}
#define REFLECT_MESSAGE(r_msg, r_pwnd) \
	if (message == r_msg) \
	{ \
		LRESULT lrrf; \
		if (ReflectMessage((r_pwnd), wParam, lParam, &lrrf)) return lrrf; \
	}
#define REFLECT_MEASUREITEM(r_pwnd) \
	if (message == WM_MEASUREITEM) \
	{ \
		LRESULT lrrf; \
		if (ReflectMeasureItem((r_pwnd), wParam, lParam, &lrrf)) return lrrf; \
	}
#define REFLECT_DRAWITEM(r_pwnd) \
	if (message == WM_DRAWITEM) \
	{ \
		LRESULT lrrf; \
		if (ReflectDrawItem((r_pwnd), wParam, lParam, &lrrf)) return lrrf; \
	}
#define REFLECT_COMPAREITEM(r_pwnd) \
	if (message == WM_COMPAREITEM) \
	{ \
		LRESULT lrrf; \
		if (ReflectCompareItem((r_pwnd), wParam, lParam, &lrrf)) return lrrf; \
	}
#define REFLECT_VKEYTOITEM(r_pwnd) \
	if (message == WM_VKEYTOITEM) \
	{ \
		LRESULT lrrf; \
		if (ReflectVKeyToItem((r_pwnd), wParam, lParam, &lrrf)) return lrrf; \
	}
#define REFLECT_DELETEITEM(r_pwnd) \
	if (message == WM_DELETEITEM) \
	{ \
		LRESULT lrrf; \
		if (ReflectDeleteItem((r_pwnd), wParam, lParam, &lrrf)) return lrrf; \
	}
#define REFLECT_CHARTOITEM(r_pwnd) \
	if (message == WM_CHARTOITEM) \
	{ \
		LRESULT lrrf; \
		if (ReflectCharToItem((r_pwnd), wParam, lParam, &lrrf)) return lrrf; \
	}
