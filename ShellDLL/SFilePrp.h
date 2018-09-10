/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 SFilePrp.h - declarations of CServerFilePropertyDialog
 */

#pragma once

#include "FileList.h"

class CServerFilePropertyDialog : public CMyDialog
{
public:
	CServerFilePropertyDialog(const CMyPtrArrayT<CFTPFileItem>& aFiles);
	virtual ~CServerFilePropertyDialog();

	enum { IDD = IDD_SFILE_PROPERTY };

public:
	CMyStringW m_strDirectory;
	CMyPtrArrayT<CServerFileAttrData> m_aAttrs;
	// [inout] in: true if the owner can be changed / out: the owner was changed
	bool m_bChangeOwner;
	// [in] true if the owner value can be a string
	bool m_bSupportedName;
	// [inout] in: true if the attributes can be changed / out: some attributes were changed
	bool m_bChangeAttr;

protected:
	virtual bool OnInitDialog(HWND hWndFocus);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnOK(WPARAM wParam, LPARAM lParam);
};
