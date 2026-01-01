/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 FileIcon.h - declaration of CFTPFileItemIcon
 */

#pragma once

class CFTPFileItemIcon : public IExtractIconW, public IExtractIconA
{
public:
	CFTPFileItemIcon(CFTPFileItem* pItem);
	CFTPFileItemIcon(LPCWSTR lpszFileName, bool bIsDirectory);
	~CFTPFileItemIcon();

	// IUnknown
public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	// IExtractIconW
public:
	STDMETHOD(GetIconLocation)(UINT uFlags, LPWSTR pszIconFile, UINT cchMax, int* piIndex, UINT* pwFlags);
	STDMETHOD(Extract)(LPCWSTR pszFile, UINT nIconIndex, HICON* phIconLarge, HICON* phIconSmall, UINT nIconSize);

	// IExtractIconA
public:
	STDMETHOD(GetIconLocation)(UINT uFlags, LPSTR pszIconFile, UINT cchMax, int* piIndex, UINT* pwFlags);
	STDMETHOD(Extract)(LPCSTR pszFile, UINT nIconIndex, HICON* phIconLarge, HICON* phIconSmall, UINT nIconSize);

protected:
	HRESULT DoExtract(bool bOpenIcon, HICON* phIconLarge, HICON* phIconSmall, UINT nIconSize);

	ULONG m_uRef;
	CMyStringW m_strFileName;
	bool m_bIsDirectory;
	int m_iIconIndex;
	int m_iOpenIconIndex;
	//CFTPFileItem* m_pItem;
};
