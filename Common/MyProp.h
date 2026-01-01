/*
 Copyright (C) 2010 jet (ジェット)

 MyProp.h - declarations of CMyPropertySheet and CMyPropertyPage
 */

#pragma once

#include "MyDialog.h"
#include "Array.h"

class CMyPropertyPage;

class CMyPropertySheet : public CMyWindow
{
public:
	CMyPropertySheet();
	virtual ~CMyPropertySheet();

	void AddPage(CMyPropertyPage* pPage);

	HWND Create(bool bModal, LPCWSTR lpszTitle, HWND hWndParent);

public:
	DWORD m_dwPSHFlags;

	bool Apply();
	void CancelToClose();
	HWND GetCurrentPageHwnd() const;
	INT_PTR GetResult() const;
	HWND GetTabControl() const;
	int HwndToIndex(HWND hPageDlg) const;
	int IdToIndex(int iPageID) const;
	HWND IndexToHwnd(int iPageIndex) const;
	int IndexToId(int iPageIndex) const;
	CMyPropertyPage* IndexToPage(int iPageIndex) const;
	int PageToIndex(CMyPropertyPage* pPage) const;
	void PressButton(int iButton);
	bool QuerySiblings(WPARAM wParam, LPARAM lParam) const;
	void RebootSystem();
	bool RecalcPageSizes();
	void RestartWindows();
	bool SetCurSel(CMyPropertyPage* pPage);
	bool SetCurSel(int iPageIndex);
	bool SetCurSelByID(int iPageID);
	void SetFinishText(LPCTSTR lpszText);
	void SetHeaderSubTitle(int iPageIndex, LPCTSTR lpszHeaderSubTitle);
	void SetHeaderTitle(int iPageIndex, LPCTSTR lpszHeaderTitle);
	void SetTitle(bool bPropTitle, LPCTSTR lpszTitle);
	void SetWizButtons(DWORD dwFlags);

protected:
	CMySimpleArray<CMyPropertyPage*> m_arrPages;
};

class CMyPropertyPage : public CMyDialog
{
public:
	CMyPropertyPage(UINT uID);
	virtual ~CMyPropertyPage();

public:
	void SetModifiedFlag(bool bModified = true);
	CMyPropertySheet* GetParentSheet() const;

	DWORD m_dwPSPFlags;

protected:
	HPROPSHEETPAGE m_hPSP;

	virtual LPCWSTR GetPageTitle();
	virtual HPROPSHEETPAGE CreatePropPageHandle();

private:
	HGLOBAL m_hGlobal;
	HGLOBAL GetDialogResource();
	friend class CMyPropertySheet;
};

////////////////////////////////////////////////////////////////////////////////

inline bool CMyPropertySheet::Apply()
	{ return ::SendMessage(m_hWnd, PSM_APPLY, 0, 0) != 0; }
inline void CMyPropertySheet::CancelToClose()
	{ ::SendMessage(m_hWnd, PSM_CANCELTOCLOSE, 0, 0); }
inline HWND CMyPropertySheet::GetCurrentPageHwnd() const
	{ return (HWND) ::SendMessage(m_hWnd, PSM_GETCURRENTPAGEHWND, 0, 0); }
inline INT_PTR CMyPropertySheet::GetResult() const
	{ return (INT_PTR) ::SendMessage(m_hWnd, PSM_GETRESULT, 0, 0); }
inline HWND CMyPropertySheet::GetTabControl() const
	{ return (HWND) ::SendMessage(m_hWnd, PSM_GETTABCONTROL, 0, 0); }
inline int CMyPropertySheet::HwndToIndex(HWND hPageDlg) const
	{ return (int) (::SendMessage(m_hWnd, PSM_HWNDTOINDEX, (WPARAM) hPageDlg, 0)); }
inline int CMyPropertySheet::IdToIndex(int iPageID) const
	{ return (int) (::SendMessage(m_hWnd, PSM_IDTOINDEX, (WPARAM) IntToPtr(iPageID), 0)); }
inline HWND CMyPropertySheet::IndexToHwnd(int iPageIndex) const
	{ return (HWND) ::SendMessage(m_hWnd, PSM_INDEXTOHWND, (WPARAM) IntToPtr(iPageIndex), 0); }
inline int CMyPropertySheet::IndexToId(int iPageIndex) const
	{ return (int) (::SendMessage(m_hWnd, PSM_INDEXTOID, (WPARAM) IntToPtr(iPageIndex), 0)); }
inline CMyPropertyPage* CMyPropertySheet::IndexToPage(int iPageIndex) const
{
	HPROPSHEETPAGE hPage = (HPROPSHEETPAGE) ::SendMessage(m_hWnd, PSM_INDEXTOPAGE, (WPARAM) IntToPtr(iPageIndex), 0);
	if (!hPage)
		return NULL;

	HWND hWnd = IndexToHwnd(iPageIndex);
	CMyPropertyPage* pPage = (CMyPropertyPage*) CMyWindow::FromHandle(hWnd);
	if (!pPage || pPage->m_hPSP != hPage)
		return NULL;
	return pPage;
}
inline int CMyPropertySheet::PageToIndex(CMyPropertyPage* pPage) const
	{ if (!pPage || !pPage->m_hPSP) return -1;
	return (int) (::SendMessage(m_hWnd, PSM_PAGETOINDEX, (WPARAM) pPage->m_hPSP, 0)); }
inline void CMyPropertySheet::PressButton(int iButton)
	{ ::SendMessage(m_hWnd, PSM_PRESSBUTTON, (WPARAM) IntToPtr(iButton), 0); }
inline bool CMyPropertySheet::QuerySiblings(WPARAM wParam, LPARAM lParam) const
	{ return ::SendMessage(m_hWnd, PSM_QUERYSIBLINGS, wParam, lParam) != 0; }
inline void CMyPropertySheet::RebootSystem()
	{ ::SendMessage(m_hWnd, PSM_REBOOTSYSTEM, 0, 0); }
inline bool CMyPropertySheet::RecalcPageSizes()
	{ return ::SendMessage(m_hWnd, PSM_RECALCPAGESIZES, 0, 0) != 0; }
inline void CMyPropertySheet::RestartWindows()
	{ ::SendMessage(m_hWnd, PSM_RESTARTWINDOWS, 0, 0); }
inline bool CMyPropertySheet::SetCurSel(CMyPropertyPage* pPage)
	{ if (!pPage || !pPage->m_hPSP) return FALSE;
	return ::SendMessage(m_hWnd, PSM_SETCURSEL, 0, (LPARAM) pPage->m_hPSP) != 0; }
inline bool CMyPropertySheet::SetCurSel(int iPageIndex)
	{ return ::SendMessage(m_hWnd, PSM_SETCURSEL, (WPARAM) IntToPtr(iPageIndex), 0) != 0; }
inline bool CMyPropertySheet::SetCurSelByID(int iPageID)
	{ return ::SendMessage(m_hWnd, PSM_SETCURSELID, (WPARAM) IntToPtr(iPageID), 0) != 0; }
inline void CMyPropertySheet::SetFinishText(LPCTSTR lpszText)
	{ ::SendMessage(m_hWnd, PSM_SETFINISHTEXT, 0, (LPARAM) lpszText); }
inline void CMyPropertySheet::SetHeaderSubTitle(int iPageIndex, LPCTSTR lpszHeaderSubTitle)
	{ ::SendMessage(m_hWnd, PSM_SETHEADERSUBTITLE, (WPARAM) IntToPtr(iPageIndex), (LPARAM) lpszHeaderSubTitle); }
inline void CMyPropertySheet::SetHeaderTitle(int iPageIndex, LPCTSTR lpszHeaderTitle)
	{ ::SendMessage(m_hWnd, PSM_SETHEADERTITLE, (WPARAM) IntToPtr(iPageIndex), (LPARAM) lpszHeaderTitle); }
inline void CMyPropertySheet::SetTitle(bool bPropTitle, LPCTSTR lpszTitle)
	{ ::SendMessage(m_hWnd, PSM_SETTITLE, (WPARAM) UIntToPtr(bPropTitle ? PSH_PROPTITLE : 0), (LPARAM) lpszTitle); }
inline void CMyPropertySheet::SetWizButtons(DWORD dwFlags)
	{ ::SendMessage(m_hWnd, PSM_SETWIZBUTTONS, 0, (LPARAM) dwFlags); }

inline CMyPropertySheet* CMyPropertyPage::GetParentSheet() const
{
	if (!m_hWnd)
		return NULL;
	HWND hWnd = ::GetParent(m_hWnd);
	return (CMyPropertySheet*) CMyWindow::FromHandle(hWnd);
}
