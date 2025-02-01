/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 Transfer.cpp - implementations of CTransferDialog
 */

#include "StdAfx.h"
#include "ShellDLL.h"
#include "Transfer.h"

#define LIST_PADDING          3
#define LIST_TEXT_MARGIN      4

CTransferDialog::CTransferDialog(CTransferDialogListener* pListener)
	: CMyDialog(IDD)
	, m_pListener(pListener)
	, m_himlSystemLarge(NULL)
{
	m_nRealItemCount = 0;
	m_pvCurItem = NULL;
	// default - set in OnInitDialog
	m_nXYPadding = -1;
	m_nCXButton = m_nCYButton = 0;
	m_nCXIcon = ::GetSystemMetrics(SM_CXICON);
	m_nCYIcon = ::GetSystemMetrics(SM_CYICON);
	m_nItemHeight = 0;
}

CTransferDialog::~CTransferDialog(void)
{
	int c = m_aItems.GetCount();
	while (c--)
		delete (CTransferItem*) m_aItems.GetItem(c);
}

static HIMAGELIST __stdcall RetrieveFileIcon(LPCWSTR lpszFileName, int* pnIconIndex, bool bVirtual)
{
	HIMAGELIST himl;
	SHFILEINFO_UNION sfi;
	UINT uFlags;

	memset(&sfi.w, 0, sizeof(sfi.w));
	uFlags = SHGFI_SYSICONINDEX | SHGFI_LARGEICON | SHGFI_TYPENAME;
	if (bVirtual)
		uFlags |= SHGFI_USEFILEATTRIBUTES;
	if (!(himl = (HIMAGELIST) ::SHGetFileInfoW(lpszFileName, FILE_ATTRIBUTE_NORMAL, &sfi.w, sizeof(sfi.w), uFlags)))
	{
		CMyStringW str(lpszFileName);
		memset(&sfi.a, 0, sizeof(sfi.a));
		if (!(himl = (HIMAGELIST) ::SHGetFileInfoA(str, FILE_ATTRIBUTE_NORMAL, &sfi.a, sizeof(sfi.a), uFlags)))
			*pnIconIndex = -1;
		else
			*pnIconIndex = sfi.a.iIcon;
	}
	else
	{
		*pnIconIndex = sfi.w.iIcon;
	}
	return himl;
}

CTransferDialog::CTransferItem* CTransferDialog::AddTransferItem(ULONGLONG uliMax, LPCWSTR lpszDestName, LPCWSTR lpszLocalFileName, bool bWaiting)
{
	HIMAGELIST himl;
	CTransferItem* pItem = new CTransferItem();
	pItem->uliCurrent = 0;
	pItem->uliMax = uliMax;
	pItem->strDestName = lpszDestName;
	if (lpszLocalFileName)
		pItem->strLocalFileName = lpszLocalFileName;
	pItem->dwStartTime = pItem->dwCurrentTime = GetTickCount();
	pItem->bWaiting = bWaiting;
	pItem->bFinished = false;
	pItem->bCanceled = false;

	if (lpszLocalFileName)
	{
		if (!MyIsExistFileW(lpszLocalFileName))
			lpszLocalFileName = NULL;
	}
	himl = RetrieveFileIcon(lpszLocalFileName ? lpszLocalFileName : lpszDestName,
		&pItem->iIconIndex, lpszLocalFileName == NULL);
	if (himl && !m_himlSystemLarge)
		m_himlSystemLarge = himl;

	m_aItems.InsertItem(0, pItem);
	m_nRealItemCount++;

	int i = (int) (::SendDlgItemMessage(m_hWnd, IDC_FILE_LIST, LB_INSERTSTRING, (WPARAM) (0), (LPARAM) pItem));
	::SendDlgItemMessage(m_hWnd, IDC_FILE_LIST, LB_SETITEMDATA, (WPARAM) (i), (LPARAM) pItem);
	if (m_nRealItemCount == 1)
		::SetTimer(m_hWnd, (UINT_PTR)(void*) this, 1000, NULL);
	::InvalidateRect(::GetDlgItem(m_hWnd, IDC_FILE_LIST), NULL, FALSE);

	UpdateWindowTitle();
	return pItem;
}

void CTransferDialog::SetTransferItemSize(CTransferItem* pvItem, ULONGLONG uliMax)
{
	if (pvItem == NULL)
	{
		return;
	}
	pvItem->uliMax = uliMax;
	pvItem->uliCurrent = 0;
	pvItem->dwCurrentTime = GetTickCount();
	if (pvItem->bWaiting)
	{
		pvItem->bWaiting = false;
		pvItem->dwStartTime = pvItem->dwCurrentTime;
	}
}

void CTransferDialog::SetTransferItemLocalFileName(CTransferItem* pvItem, LPCWSTR lpszLocalFileName)
{
	if (pvItem == NULL)
	{
		return;
	}
	pvItem->strLocalFileName = lpszLocalFileName;
}

void CTransferDialog::UpdateTransferItem(CTransferItem* pvItem, ULONGLONG uliPosition)
{
	if (pvItem == NULL)
	{
		return;
	}
	pvItem->uliCurrent = uliPosition;
	pvItem->dwCurrentTime = GetTickCount();
	if (pvItem->bWaiting)
	{
		pvItem->bWaiting = false;
		pvItem->dwStartTime = pvItem->dwCurrentTime;
	}
}

void CTransferDialog::RemoveTransferItem(CTransferItem* pvItem, bool bCanceled)
{
	if (pvItem == NULL)
	{
		return;
	}
	pvItem->bFinished = true;
	pvItem->bCanceled = bCanceled;
	m_nRealItemCount--;
	if (MyIsExistFileW(pvItem->strLocalFileName))
		RetrieveFileIcon(pvItem->strLocalFileName, &pvItem->iIconIndex, false);

	if (m_nRealItemCount == 0)
	{
		::KillTimer(m_hWnd, (UINT_PTR)(void*) this);
		::InvalidateRect(::GetDlgItem(m_hWnd, IDC_FILE_LIST), NULL, FALSE);
	}
	UpdateWindowTitle();
}

void CTransferDialog::ClearAllItems()
{
	register int c = m_aItems.GetCount();
	while (c--)
	{
		register CTransferItem* p = (CTransferItem*) m_aItems.GetItem(c);
		if (p->bFinished || p->bCanceled)
		{
			::SendDlgItemMessage(m_hWnd, IDC_FILE_LIST, LB_DELETESTRING, (WPARAM) IntToPtr(c), 0);
			m_aItems.RemoveItem(c);
			delete p;
		}
	}
	UpdateWindowTitle();
}

int CTransferDialog::GetFinishedCount() const
{
	int ret = 0;
	for (int i = 0; i < m_aItems.GetCount(); i++)
	{
		register CTransferItem* p = (CTransferItem*) m_aItems.GetItem(i);
		if (p->bFinished || p->bCanceled)
			ret++;
	}
	return ret;
}

void CTransferDialog::UpdateWindowTitle()
{
	CMyStringW str(m_strTitle);
	if (m_aItems.GetCount())
	{
		CMyStringW str2;
		str2.Format(IDS_TRANSFER_COUNT, GetFinishedCount(), m_aItems.GetCount());
		str += L" - ";
		str += str2;
	}
	SetWindowTextW(str);
}

bool CTransferDialog::OnInitDialog(HWND hWndFocus)
{
	::MyGetWindowTextStringW(m_hWnd, m_strTitle);

	HMENU hSysMenu = ::GetSystemMenu(m_hWnd, FALSE);
	::DeleteMenu(hSysMenu, SC_MAXIMIZE, MF_BYCOMMAND);
	::DeleteMenu(hSysMenu, SC_MINIMIZE, MF_BYCOMMAND);
	::DeleteMenu(hSysMenu, SC_RESTORE, MF_BYCOMMAND);

	m_hMenuTransfer = ::GetSubMenu(theApp.m_hMenuPopup, POPUP_POS_TRANSFER);

	OnSize(0, 0);
	return true;
}

LRESULT CTransferDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE_COMMAND(IDCANCEL, OnCloseButton);
	//HANDLE_COMMAND(IDC_REMOVE_ALL, OnRemoveAll);
	HANDLE_PROC_MESSAGE(WM_MEASUREITEM, OnMeasureItem);
	HANDLE_PROC_MESSAGE(WM_DRAWITEM, OnDrawItem);
	HANDLE_PROC_MESSAGE(WM_TIMER, OnTimer);
	HANDLE_PROC_MESSAGE(WM_CONTEXTMENU, OnContextMenu);
	HANDLE_PROC_MESSAGE(WM_INITMENUPOPUP, OnInitMenuPopup);
	HANDLE_PROC_MESSAGE(WM_SIZE, OnSize);
	HANDLE_PROC_MESSAGE(WM_GETMINMAXINFO, OnGetMinMaxInfo);
	return CMyDialog::WindowProc(message, wParam, lParam);
}

LRESULT CTransferDialog::OnCloseButton(WPARAM wParam, LPARAM lParam)
{
	//ShowWindow(m_hWnd, SW_HIDE);
	::EnableDlgItem(m_hWnd, IDCANCEL, FALSE);
	m_pListener->TransferCanceled(NULL);
	return 0;
}

LRESULT CTransferDialog::OnRemoveAll(WPARAM wParam, LPARAM lParam)
{
	ClearAllItems();
	return 0;
}

LRESULT CTransferDialog::OnMeasureItem(WPARAM wParam, LPARAM lParam)
{
	LPMEASUREITEMSTRUCT lpmis;
	HFONT hFont;
	HDC hDC;
	HGDIOBJ hgdi;
	TEXTMETRIC tm;
	int cy;

	lpmis = (LPMEASUREITEMSTRUCT) lParam;
	if (lpmis->CtlID == IDC_FILE_LIST)
	{
		//hFont = (HFONT) ::SendMessage(m_hWnd, WM_GETFONT, 0, 0);
		hFont = theApp.m_hFontWindow;
		hDC = ::GetDC(m_hWnd);
		hgdi = ::SelectObject(hDC, hFont);
		::GetTextMetrics(hDC, &tm);
		::SelectObject(hDC, hgdi);
		::ReleaseDC(m_hWnd, hDC);

		cy = tm.tmHeight * 2 + LIST_TEXT_MARGIN;
		if (cy < m_nCYIcon)
			cy = m_nCYIcon;
		lpmis->itemWidth = 0;
		m_nItemHeight = lpmis->itemHeight = cy + LIST_PADDING * 2;
	}
	return 0;
}

// in SFileVw.cpp
extern void __stdcall FileSizeToString(ULARGE_INTEGER uli, CMyStringW& ret);

static void __stdcall MyDrawRectString(HDC hDC, const RECT* lprc, const CMyStringW& text)
{
	//HRGN hRgn = ::CreateRectRgnIndirect(lprc);

	//::SelectClipRgn(hDC, hRgn);
	::TextOutW(hDC, lprc->left, lprc->top, text, (int) text.GetLength());
	//::SelectClipRgn(hDC, NULL);

	//::DeleteObject(hRgn);
}

// IDS_TRANSFER_RATE
//   %s<cur-size-string> / %s<max-size-string> - left %02d<mm>:%02d<ss> (rate: %s<size-per-sec> / sec.)

LRESULT CTransferDialog::OnDrawItem(WPARAM wParam, LPARAM lParam)
{
	LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam;

	if (lpdis->CtlID != IDC_FILE_LIST)
		return 0;

	{
		CTransferItem* pItem = lpdis->itemID == -1 ? NULL : (CTransferItem*) lpdis->itemData;
		RECT rc;
		TEXTMETRIC tm;
		int nBkMode;
		COLORREF crText, crBack;
		HGDIOBJ hgdiFont;

		hgdiFont = ::SelectObject(lpdis->hDC, theApp.m_hFontWindow);
		::GetTextMetrics(lpdis->hDC, &tm);
		nBkMode = ::SetBkMode(lpdis->hDC, OPAQUE);

		memcpy(&rc, &lpdis->rcItem, sizeof(rc));
		if (lpdis->itemAction & (ODA_DRAWENTIRE | ODA_FOCUS))
		{
			if (lpdis->itemState & ODS_FOCUS)
			{
				::DrawFocusRect(lpdis->hDC, &rc);
				rc.left++;
				rc.top++;
				rc.right--;
				rc.bottom--;
			}
			else if (lpdis->itemAction & ODA_FOCUS)
				::DrawFocusRect(lpdis->hDC, &rc);
		}
		if (lpdis->itemState & ODS_SELECTED)
		{
			if (lpdis->itemAction & (ODA_DRAWENTIRE | ODA_SELECT))
				::FillRect(lpdis->hDC, &rc, ::GetSysColorBrush(COLOR_HIGHLIGHT));
			crText = ::SetTextColor(lpdis->hDC, ::GetSysColor(COLOR_HIGHLIGHTTEXT));
			crBack = ::SetBkColor(lpdis->hDC, ::GetSysColor(COLOR_HIGHLIGHT));
		}
		else
		{
			if (lpdis->itemAction & (ODA_DRAWENTIRE | ODA_SELECT))
				::FillRect(lpdis->hDC, &rc, ::GetSysColorBrush(COLOR_WINDOW));
			crText = ::SetTextColor(lpdis->hDC, ::GetSysColor(COLOR_WINDOWTEXT));
			crBack = ::SetBkColor(lpdis->hDC, ::GetSysColor(COLOR_WINDOW));
		}

		if (pItem)
		{
			//if (m_himlSystemLarge && pItem->iIconIndex != -1)
			//{
			//	int y = rc.top + ((rc.bottom - rc.top) - m_nCYIcon) / 2;
			//	::ImageList_Draw(m_himlSystemLarge, pItem->iIconIndex, lpdis->hDC, LIST_PADDING, y,
			//		(lpdis->itemState & ODS_SELECTED) ? ILD_SELECTED | ILD_TRANSPARENT : ILD_TRANSPARENT);
			//}

			//rc.bottom = lpdis->rcItem.bottom / 2;
			rc.bottom = rc.top + (rc.bottom - rc.top) / 2;
			rc.bottom -= LIST_TEXT_MARGIN / 2;
			rc.top = rc.bottom - tm.tmHeight;
			rc.left = LIST_PADDING + m_nCXIcon + LIST_TEXT_MARGIN;
			rc.right = lpdis->rcItem.right - LIST_PADDING;
			MyDrawRectString(lpdis->hDC, &rc, pItem->strDestName);

			CMyStringW strTransfer;
			if (pItem->bFinished && pItem->bCanceled)
				strTransfer.LoadString(IDS_CANCELED);
			else if (pItem->bWaiting)
				strTransfer.LoadString(IDS_WAITING);
			else
			{
				CMyStringW strSizeMax;
				if (pItem->bFinished)
				{
					ULARGE_INTEGER uli;
					uli.QuadPart = pItem->uliCurrent;
					FileSizeToString(uli, strSizeMax);
					strTransfer.Format(IDS_FINISHED, (LPCWSTR) strSizeMax);
				}
				else
				{
					CMyStringW strSizeCur, strSizeRate;
					DWORD dwPassTime = pItem->dwCurrentTime - pItem->dwStartTime;
					{
						ULARGE_INTEGER uli;
						uli.QuadPart = pItem->uliCurrent;
						FileSizeToString(uli, strSizeCur);
						if (pItem->uliMax == -1)
							strSizeMax.LoadString(IDS_UNKNOWN_SIZE);
						else
						{
							uli.QuadPart = pItem->uliMax;
							FileSizeToString(uli, strSizeMax);
						}
						if (dwPassTime < 1000)
							uli.QuadPart = 0;
						else
							uli.QuadPart = pItem->uliCurrent / (dwPassTime / 1000);
						FileSizeToString(uli, strSizeRate);

						// 残り時間を計算
						if (!uli.QuadPart)
							uli.QuadPart = 1024;
						uli.QuadPart = ((pItem->uliMax - pItem->uliCurrent) * 1000 / uli.QuadPart);
						if (uli.HighPart != 0)
							dwPassTime = 0xFFFFFFFF;
						else
							dwPassTime = uli.LowPart;
					}
					{
						register int nMinute, nSecond;
						nMinute = (int) (dwPassTime / 60000);
						nSecond = (int) ((dwPassTime - ((DWORD) nMinute * 60000)) / 1000);
						strTransfer.Format(IDS_TRANSFER_RATE, (LPCWSTR) strSizeCur, (LPCWSTR) strSizeMax,
							nMinute, nSecond, (LPCWSTR) strSizeRate);
					}
				}
			}
			rc.top = rc.bottom + LIST_TEXT_MARGIN;
			rc.bottom = rc.top + tm.tmHeight;
			MyDrawRectString(lpdis->hDC, &rc, strTransfer);
		}

		::SetBkMode(lpdis->hDC, nBkMode);
		::SelectObject(lpdis->hDC, hgdiFont);
	}
	return 0;
}

LRESULT CTransferDialog::OnTimer(WPARAM wParam, LPARAM lParam)
{
	if ((void*) wParam == (void*) this)
	{
		//RECT rc;
		//::SendDlgItemMessage(m_hWnd, IDC_FILE_LIST, LB_GETITEMRECT, (WPARAM) IntToPtr(i), (LPARAM) &rc);
		//::InvalidateRect(::GetDlgItem(m_hWnd, IDC_FILE_LIST), &rc, FALSE);
		::InvalidateRect(::GetDlgItem(m_hWnd, IDC_FILE_LIST), NULL, FALSE);
		return 0;
	}
	return Default(wParam, lParam);
}

LRESULT CTransferDialog::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	if ((HWND) wParam == ::GetDlgItem(m_hWnd, IDC_FILE_LIST))
	{
		POINT pt;
		int nCurSel;
		pt.x = (int)(short) LOWORD(lParam);
		pt.y = (int)(short) HIWORD(lParam);
		if (pt.x != -1 && pt.y != -1)
		{
			::ScreenToClient(::GetDlgItem(m_hWnd, IDC_FILE_LIST), &pt);
			nCurSel = (int) (::SendDlgItemMessage(m_hWnd, IDC_FILE_LIST, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y)));
			if (HIWORD(nCurSel))
				return 0;
			nCurSel = (int)(short) LOWORD(nCurSel);
			if (nCurSel == LB_ERR)
				return 0;
			::SendMessage((HWND) wParam, LB_SETCURSEL, (WPARAM) (nCurSel), 0);
		}
		else
			nCurSel = (int) (::SendMessage((HWND) wParam, LB_GETCURSEL, 0, 0));
		if (nCurSel != LB_ERR)
		{
			CTransferItem* p;
			p = (CTransferItem*) m_aItems.GetItem(nCurSel);
			if (pt.x == -1 && pt.y == -1)
			{
				RECT rc;
				::SendMessage((HWND) wParam, LB_GETITEMRECT, (WPARAM) (nCurSel), (LPARAM)(LPRECT) &rc);
				pt.x = rc.left + 2;
				pt.y = rc.top + 2;
				::ClientToScreen((HWND) wParam, &pt);
			}
			else
			{
				// 本来 pt はスクリーン座標のはずだが、なぜかクライアント座標が入っているので
				// 互換性のためにも位置を取得し直す
				::GetCursorPos(&pt);
			}
			m_pvCurItem = p;
			UINT uCmd = (UINT) ::TrackPopupMenu(m_hMenuTransfer, TPM_RETURNCMD, pt.x, pt.y, 0, m_hWnd, NULL);
			m_pvCurItem = NULL;
			switch (uCmd)
			{
				case ID_TRANSFER_OPEN_LOCAL:
					MyShellOpenW(m_hWnd, p->strLocalFileName);
					break;
				case ID_TRANSFER_CANCEL:
					m_pListener->TransferCanceled(p);
					break;
			}
		}
		return 0;
	}
	return Default(wParam, lParam);
}

LRESULT CTransferDialog::OnInitMenuPopup(WPARAM wParam, LPARAM lParam)
{
	Default(wParam, lParam);

	if ((HMENU) wParam == m_hMenuTransfer)
	{
		register CTransferItem* p = (CTransferItem*) m_pvCurItem;
		::EnableMenuItem(m_hMenuTransfer, ID_TRANSFER_OPEN_LOCAL,
			p != NULL && ::MyIsExistFileW(p->strLocalFileName) ? MF_ENABLED : MF_GRAYED);
		::EnableMenuItem(m_hMenuTransfer, ID_TRANSFER_CANCEL,
			p != NULL && !p->bCanceled && !p->bFinished ? MF_ENABLED : MF_GRAYED);
	}

	return 0;
}

LRESULT CTransferDialog::OnSize(WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	::GetClientRect(m_hWnd, &rc);

	if (m_nXYPadding == -1)
	{
		if (::GetDlgItem(m_hWnd, IDC_FILE_LIST))
		{
			RECT rcList;
			::GetWindowRect(::GetDlgItem(m_hWnd, IDC_FILE_LIST), &rcList);
			::ScreenToClient(m_hWnd, (LPPOINT) &rcList);
			//::ScreenToClient(m_hWnd, ((LPPOINT) &rcList) + 1);
			m_nXYPadding = rcList.left;
		}
	}
	if (!m_nCXButton)
	{
		if (::GetDlgItem(m_hWnd, IDCANCEL))
		{
			RECT rcButton;
			::GetWindowRect(::GetDlgItem(m_hWnd, IDCANCEL), &rcButton);
			m_nCXButton = rcButton.right - rcButton.left;
			m_nCYButton = rcButton.bottom - rcButton.top;
		}
	}

	::MoveWindow(::GetDlgItem(m_hWnd, IDC_FILE_LIST),
		m_nXYPadding,
		m_nXYPadding,
		rc.right - m_nXYPadding * 2,
		rc.bottom - m_nXYPadding * 3 - m_nCYButton,
		TRUE);
	::MoveWindow(::GetDlgItem(m_hWnd, IDCANCEL),
		rc.right - m_nXYPadding - m_nCXButton,
		rc.bottom - m_nXYPadding - m_nCYButton,
		m_nCXButton,
		m_nCYButton,
		TRUE);

	return 0;
}

LRESULT CTransferDialog::OnGetMinMaxInfo(WPARAM wParam, LPARAM lParam)
{
	Default(wParam, lParam);

	LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
	RECT rc = { 0, 0, m_nXYPadding * 2 + m_nCXButton, m_nXYPadding * 3 + m_nCYButton + m_nItemHeight * 2 };
	::AdjustWindowRectEx(&rc, ::GetWindowLong(m_hWnd, GWL_STYLE), FALSE, ::GetWindowLong(m_hWnd, GWL_EXSTYLE));
	lpmmi->ptMinTrackSize.x = rc.right - rc.left;
	lpmmi->ptMinTrackSize.y = rc.bottom - rc.top;
	return 0;
}
