/*
 Copyright (C) 2010 Kuri-Applications

 Splitter.cpp - implementations of Splitter window
 */

#include "stdafx.h"
#include "Splitter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define INITIAL_WIDTH             4

LRESULT CALLBACK SplitterProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////

extern "C" bool __stdcall InitSplitter(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.lpfnWndProc = (WNDPROC) SplitterProc;
#ifndef UNICODE
	wcex.lpszClassName = SPLITTER_CLASSA;
#else
	wcex.lpszClassName = SPLITTER_CLASSW;
#endif
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.hInstance = hInstance;
	return ::RegisterClassEx(&wcex) != NULL;
}

struct CSplitterData
{
	HWND hWnd;
	HCURSOR hCursor;
	short nWidth;
	bool bHorz;
};

static const TCHAR s_szSplitterProp[] = _T("SPD Property");

void __stdcall EnterTrack(CSplitterData* pData);
//void __stdcall CancelTrack(CSplitterData* pData);
void __stdcall MoveRectFromPoint(bool bHorz, LPRECT lprc, const POINT& pt);
void __stdcall DrawTrackRect(bool bHorz, HDC hDC, LPCRECT lprcWindow, const POINT& ptCursor, const POINT& ptBefore, HBRUSH hbr);

LRESULT CALLBACK SplitterProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CSplitterData* pData = (CSplitterData*) ::GetProp(hWnd, s_szSplitterProp);
	switch (message)
	{
		case WM_NCCREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
			pData = (CSplitterData*) malloc(sizeof(CSplitterData));
			pData->hWnd = hWnd;
			pData->bHorz = !(lpcs->style & SPS_VERTICAL);
			pData->nWidth = INITIAL_WIDTH;
			pData->hCursor = ::LoadCursor(NULL, pData->bHorz ? IDC_SIZENS : IDC_SIZEWE);
			::SetProp(hWnd, s_szSplitterProp, (HANDLE) pData);

			if (pData->bHorz)
				::MoveWindow(hWnd, lpcs->x, lpcs->y, lpcs->cx, pData->nWidth, FALSE);
			else
				::MoveWindow(hWnd, lpcs->x, lpcs->y, pData->nWidth, lpcs->cy, FALSE);
		}
		break;
		case WM_NCDESTROY:
		{
			::RemoveProp(hWnd, s_szSplitterProp);
			LRESULT lr = ::DefWindowProc(hWnd, message, wParam, lParam);
			::DestroyCursor(pData->hCursor);
			free(pData);
			return lr;
		}
		case WM_NCHITTEST:
			return (LRESULT) HTCLIENT;
		case WM_SETCURSOR:
		{
			if (LOWORD(lParam) == HTCLIENT)
			{
				::SetCursor(pData->hCursor);
				return (LRESULT) TRUE;
			}
		}
		break;

		case WM_LBUTTONDBLCLK:
		{
			NMHDR nmh;
			nmh.code = SPN_DBLCLK;
			nmh.hwndFrom = hWnd;
			nmh.idFrom = (UINT) ::GetWindowLong(hWnd, GWL_ID);
			if (::SendMessage(::GetParent(hWnd), WM_NOTIFY, (WPARAM) (nmh.idFrom), (LPARAM) &nmh))
				break;
			EnterTrack(pData);
			return 0;
		}
		case SPM_BEGINTRACK:
		case WM_LBUTTONDOWN:
		{
			NMHDR nmh;
			nmh.code = SPN_BEFORETRACK;
			nmh.hwndFrom = hWnd;
			nmh.idFrom = (UINT) ::GetWindowLong(hWnd, GWL_ID);
			if (::SendMessage(::GetParent(hWnd), WM_NOTIFY, (WPARAM) (nmh.idFrom), (LPARAM) &nmh))
				break;
			EnterTrack(pData);
			return 0;
		}

		case SPM_GETWIDTH:
			return (LRESULT) LongToPtr(MAKELONG(pData->nWidth, 0));
		case SPM_SETWIDTH:
		{
			RECT rc;
			short sh;
			if ((short) LOWORD(wParam) <= 0)
				return (LRESULT) LongToPtr(-1);
			sh = pData->nWidth;
			pData->nWidth = (short) LOWORD(wParam);
			::GetWindowRect(hWnd, &rc);
			::ScreenToClient(::GetParent(hWnd), (LPPOINT) &rc);
			::ScreenToClient(::GetParent(hWnd), ((LPPOINT) &rc) + 1);
			if (pData->bHorz)
				::MoveWindow(hWnd, rc.left, rc.top, rc.right - rc.left, pData->nWidth, TRUE);
			else
				::MoveWindow(hWnd, rc.left, rc.top, pData->nWidth, rc.bottom - rc.top, TRUE);
			return (LRESULT) LongToPtr(MAKELONG(sh, 0));
		}
		case SPM_GETCURSOR:
			return (LRESULT) pData->hCursor;
		case SPM_SETCURSOR:
		{
			::DestroyCursor(pData->hCursor);
			pData->hCursor = (HCURSOR) wParam;
			return 0;
		}
	}
	return ::DefWindowProc(hWnd, message, wParam, lParam);
}

static int __stdcall _PointToPos(bool bHorz, HWND hWndParent, const POINT& pt)
{
	POINT _pt;
	_pt.x = pt.x;
	_pt.y = pt.y;
	::ScreenToClient(hWndParent, &_pt);
	return (bHorz ? _pt.y : _pt.x);
}

static void __stdcall _PointFromPos(bool bHorz, HWND hWndParent, POINT& pt, int nPos)
{
	POINT _pt;
	_pt.x = _pt.y = nPos;
	::ClientToScreen(hWndParent, &_pt);
	if (bHorz)
		pt.y = _pt.y;
	else
		pt.x = _pt.x;
}

void __stdcall EnterTrack(CSplitterData* pData)
{
	RECT rcWindow;
	SPTRACKNOTIFY sptn;
	HBRUSH hbr;
	HWND hWndP = ::GetParent(pData->hWnd);
	sptn.hdr.hwndFrom = pData->hWnd;
	sptn.hdr.idFrom = (UINT) ::GetWindowLong(pData->hWnd, GWL_ID);
	sptn.hdr.code = SPN_GETBRUSH;
	sptn.nPos = 0;
	hbr = (HBRUSH) ::SendMessage(hWndP, WM_NOTIFY, (WPARAM) (sptn.hdr.idFrom), (LPARAM) &sptn);
	::GetWindowRect(pData->hWnd, &rcWindow);
	memcpy(&sptn.rcWindow, &rcWindow, sizeof(rcWindow));
	::GetClientRect(hWndP, &sptn.rcParent);

	HDC hDC;
	HWND hWndDesk = ::GetDesktopWindow();
	if (::LockWindowUpdate(hWndP))
		hDC = ::GetDCEx(hWndDesk, NULL, DCX_WINDOW | DCX_CACHE | DCX_LOCKWINDOWUPDATE);
	else
		hDC = ::GetDCEx(hWndDesk, NULL, DCX_WINDOW | DCX_CACHE);
	::SetCapture(pData->hWnd);

	MSG msg;
	POINT pt, ptBefore;
	bool bOK = false;
	bool bExit = false;
	::GetCursorPos(&pt);
	sptn.hdr.code = SPN_TRACKING;
	sptn.nPos = _PointToPos(pData->bHorz, hWndP, pt);
	if (::SendMessage(hWndP, WM_NOTIFY, (WPARAM) (sptn.hdr.idFrom), (LPARAM) &sptn))
        _PointFromPos(pData->bHorz, hWndP, pt, sptn.nPos);
	ptBefore.x = pt.x;
	ptBefore.y = pt.y;
	::DrawTrackRect(pData->bHorz, hDC, &rcWindow, pt, ptBefore, hbr);

	while (!bExit && ::GetMessage(&msg, NULL, 0, 0))
	{
		switch (msg.message)
		{
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
				bOK = true;
				bExit = true;
				break;
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
				bExit = true;
				break;
			case WM_MOUSEMOVE:
				::GetCursorPos(&pt);
				sptn.hdr.code = SPN_TRACKING;
				sptn.nPos = _PointToPos(pData->bHorz, hWndP, pt) - (pData->nWidth / 2);
				if (::SendMessage(hWndP, WM_NOTIFY, (WPARAM) (sptn.hdr.idFrom), (LPARAM) &sptn))
					_PointFromPos(pData->bHorz, hWndP, pt, sptn.nPos + (pData->nWidth / 2));
				if ((pData->bHorz && pt.y != ptBefore.y) ||
					(!pData->bHorz && pt.x != ptBefore.x))
				{
					//::DrawTrackRect(pData->bHorz, hDC, &rcWindow, ptBefore, hbr);
					::DrawTrackRect(pData->bHorz, hDC, &rcWindow, pt, ptBefore, hbr);
					ptBefore.x = pt.x;
					ptBefore.y = pt.y;
				}
				else
				{
					pt.x = ptBefore.x;
					pt.y = ptBefore.y;
				}
				break;
			case WM_KEYDOWN:
				if (msg.wParam == VK_ESCAPE ||
					msg.wParam == VK_LWIN ||
					msg.wParam == VK_RWIN)
				{
					bExit = true;
					break;
				}
			default:
				::DispatchMessage(&msg);
		}
	}

	//if ((pData->bHorz && pt.y != ptBefore.y) || (!pData->bHorz && pt.x != ptBefore.x))
		::DrawTrackRect(pData->bHorz, hDC, &rcWindow, pt, ptBefore, hbr);
	::ReleaseCapture();
	::ReleaseDC(hWndDesk, hDC);
	::LockWindowUpdate(NULL);

	sptn.hdr.code = SPN_TRACK;
	sptn.nPos = _PointToPos(pData->bHorz, hWndP, pt) - (pData->nWidth / 2);
	if (::SendMessage(hWndP, WM_NOTIFY, (WPARAM) (sptn.hdr.idFrom), (LPARAM) &sptn))
		bOK = false;
	if (bOK)
	{
		::MoveRectFromPoint(pData->bHorz, &rcWindow, pt);
		::ScreenToClient(hWndP, (LPPOINT) &rcWindow);
		::ScreenToClient(hWndP, ((LPPOINT) &rcWindow) + 1);
		::MoveWindow(pData->hWnd, rcWindow.left, rcWindow.top,
			rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, TRUE);
	}
}

void __stdcall MoveRectFromPoint(bool bHorz, LPRECT lprc, const POINT& pt)
{
	int n;
	if (bHorz)
	{
		n = lprc->bottom - lprc->top;
		lprc->top = pt.y - n / 2;
		lprc->bottom = lprc->top + n;
	}
	else
	{
		n = lprc->right - lprc->left;
		lprc->left = pt.x - n / 2;
		lprc->right = lprc->left + n;
	}
}

void __stdcall DrawTrackRect(bool bHorz, HDC hDC, LPCRECT lprcWindow, const POINT& ptCursor, const POINT& ptBefore, HBRUSH hbr)
{
	RECT rc;
	HRGN hRgn;
	memcpy(&rc, lprcWindow, sizeof(rc));
	MoveRectFromPoint(bHorz, &rc, ptCursor);
	hRgn = ::CreateRectRgnIndirect(&rc);

	HGDIOBJ hgdi;
	if (hbr)
		hgdi = ::SelectObject(hDC, hbr);
	if (ptBefore.x != ptCursor.x || ptBefore.y != ptCursor.y)
	{
		RECT rc2;
		HRGN hRgn2;
		memcpy(&rc2, lprcWindow, sizeof(rc2));
		MoveRectFromPoint(bHorz, &rc2, ptBefore);
		hRgn2 = ::CreateRectRgnIndirect(&rc2);
		::CombineRgn(hRgn, hRgn, hRgn2, RGN_XOR);
		::DeleteObject(hRgn2);
	}
	::SelectClipRgn(hDC, hRgn);
	::GetClipBox(hDC, &rc);
	::PatBlt(hDC, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, PATINVERT);
	::SelectClipRgn(hDC, NULL);
	::DeleteObject(hRgn);
	if (hbr)
		::SelectObject(hDC, hgdi);
}
