/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 Transfer.h - declarations of CTransferDialog
 */

#pragma once

#include "UString.h"
#include "Array.h"

class __declspec(novtable) CTransferDialogListener
{
public:
	virtual void TransferCanceled(void* pvTransfer) = 0;
};

class CTransferDialog :
	public CMyDialog
{
public:
	CTransferDialog(CTransferDialogListener* pListener);
	virtual ~CTransferDialog(void);

	enum { IDD = IDD_TRANSFER };

	struct CTransferItem
	{
		ULONGLONG uliCurrent;
		ULONGLONG uliMax;
		ULONGLONG dwStartTime;
		ULONGLONG dwCurrentTime;
		CMyStringW strDestName;
		CMyStringW strLocalFileName;
		int iIconIndex;
		bool bWaiting;
		bool bFinished;
		bool bCanceled;
	};

public:
	CTransferItem* AddTransferItem(ULONGLONG uliMax, LPCWSTR lpszDestName, LPCWSTR lpszLocalFileName = NULL, bool bWaiting = false);
	void SetTransferItemSize(CTransferItem* pvItem, ULONGLONG uliMax);
	void SetTransferItemLocalFileName(CTransferItem* pvItem, LPCWSTR lpszLocalFileName);
	void UpdateTransferItem(CTransferItem* pvItem, ULONGLONG uliPosition);
	void RemoveTransferItem(CTransferItem* pvItem, bool bCanceled = false);
	void ClearAllItems();

protected:
	CTransferDialogListener* m_pListener;
	CMyStringW m_strTitle;
	HIMAGELIST m_himlSystemLarge;
	CMyPtrArray m_aItems;
	int m_nRealItemCount;
	HMENU m_hMenuTransfer;
	void* m_pvCurItem;
	int m_nCXIcon, m_nCYIcon;
	int m_nXYPadding, m_nCXButton, m_nCYButton, m_nItemHeight;

	int GetFinishedCount() const;
	void UpdateWindowTitle();

	virtual bool OnInitDialog(HWND hWndFocus);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnCloseButton(WPARAM wParam, LPARAM lParam);
	LRESULT OnRemoveAll(WPARAM wParam, LPARAM lParam);
	LRESULT OnMeasureItem(WPARAM wParam, LPARAM lParam);
	LRESULT OnDrawItem(WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnInitMenuPopup(WPARAM wParam, LPARAM lParam);
	LRESULT OnSize(WPARAM wParam, LPARAM lParam);
	LRESULT OnGetMinMaxInfo(WPARAM wParam, LPARAM lParam);
};
