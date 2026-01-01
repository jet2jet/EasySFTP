/*
 EasySFTP - Copyright (C) 2025 jet (ジェット)

 FileIcon.cpp - implementation of CFTPFileItemIcon
 */

#include "stdafx.h"
#include "ShellDLL.h"
#include "FileIcon.h"

#include "Folder.h"

static void __stdcall GetDummyFileItemIcon(LPCWSTR lpszFileName, bool bIsDirectory, int& iIconIndex, int& iOpenIconIndex)
{
	SHFILEINFOA sfiA;
	DWORD dw;

	CMyStringW str(lpszFileName);
	memset(&sfiA, 0, sizeof(sfiA));
	dw = bIsDirectory ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
	if (!::SHGetFileInfoA(str, dw, &sfiA, sizeof(sfiA),
		SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON))
		return;
	iIconIndex = sfiA.iIcon;
	if (bIsDirectory)
	{
		memset(&sfiA, 0, sizeof(sfiA));
		if (!::SHGetFileInfoA(str, dw, &sfiA, sizeof(sfiA),
			SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_OPENICON))
			return;
		iOpenIconIndex = sfiA.iIcon;
	}
	else
		iOpenIconIndex = iIconIndex;
}

////////////////////////////////////////////////////////////////////////////////

CFTPFileItemIcon::CFTPFileItemIcon(CFTPFileItem* pItem)
	: m_uRef(1)
{
	while (pItem->pTargetFile)
		pItem = pItem->pTargetFile;
	if (pItem->iIconIndex == -1)
		FillFileItemInfo(pItem);
	m_strFileName = pItem->strFileName;
	m_bIsDirectory = pItem->IsDirectory();
	m_iIconIndex = pItem->iIconIndex;
	m_iOpenIconIndex = pItem->iOpenIconIndex;
}

CFTPFileItemIcon::CFTPFileItemIcon(LPCWSTR lpszFileName, bool bIsDirectory)
	: m_uRef(1)
{
	m_strFileName = lpszFileName;
	m_bIsDirectory = bIsDirectory;
	GetDummyFileItemIcon(lpszFileName, bIsDirectory, m_iIconIndex, m_iOpenIconIndex);
}

CFTPFileItemIcon::~CFTPFileItemIcon()
{
}

STDMETHODIMP CFTPFileItemIcon::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;

	// NOTE: we must set NULL to *ppv, or causes 0xC000041D exception in Windows 7 (x64)
	*ppv = NULL;

	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IExtractIconW))
	{
		*ppv = (IExtractIconW*)this;
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid, IID_IExtractIconA))
	{
		*ppv = (IExtractIconA*)this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CFTPFileItemIcon::AddRef()
{
	return (ULONG) ::InterlockedIncrement((LONG*)&m_uRef);
}

STDMETHODIMP_(ULONG) CFTPFileItemIcon::Release()
{
	ULONG u = (ULONG) ::InterlockedDecrement((LONG*)&m_uRef);
	if (!u)
		delete this;
	return u;
}

STDMETHODIMP CFTPFileItemIcon::GetIconLocation(UINT uFlags, LPWSTR pszIconFile, UINT cchMax, int* piIndex, UINT* pwFlags)
{
	if (!piIndex)
		return E_POINTER;
	if (cchMax > 0)
	{
		if (!pszIconFile)
			return E_POINTER;
		UINT dw = (UINT)m_strFileName.GetLength();
		if (cchMax >= dw + 1)
			cchMax = dw + 1;
		memcpy(pszIconFile, (LPCWSTR)m_strFileName, sizeof(WCHAR) * (cchMax - 1));
		pszIconFile[cchMax - 1] = 0;
	}
	if ((uFlags & GIL_OPENICON) && m_bIsDirectory)
		*piIndex = 1;
	else
		*piIndex = 0;
	if (pwFlags)
		*pwFlags = GIL_NOTFILENAME;
	return S_OK;
}

static HRESULT __stdcall _ExtractIconImpl(int iIndex, WORD iconSize, HICON* phIcon)
{
	if (iconSize <= 16)
		*phIcon = ::ImageList_GetIcon(theApp.m_himlSysIconSmall, iIndex, ILD_TRANSPARENT);
	else if (iconSize <= 32)
		*phIcon = ::ImageList_GetIcon(theApp.m_himlSysIconLarge, iIndex, ILD_TRANSPARENT);
	else if (iconSize <= 48)
	{
		if (!theApp.m_pimlSysIconExtraLarge)
		{
			*phIcon = NULL;
			return E_NOTIMPL;
		}
		HRESULT hr = theApp.m_pimlSysIconExtraLarge->GetIcon(iIndex, ILD_TRANSPARENT, phIcon);
		if (FAILED(hr))
			return hr;
	}
	else if (iconSize <= 256)
	{
		if (!theApp.m_pimlSysIconJumbo)
		{
			*phIcon = NULL;
			return E_NOTIMPL;
		}
		HRESULT hr = theApp.m_pimlSysIconJumbo->GetIcon(iIndex, ILD_TRANSPARENT, phIcon);
		if (FAILED(hr))
			return hr;
	}
	else
	{
		*phIcon = NULL;
		return E_NOTIMPL;
	}
	if (!*phIcon)
		return E_OUTOFMEMORY;
	return S_OK;
}

HRESULT CFTPFileItemIcon::DoExtract(bool bOpenIcon, HICON* phIconLarge, HICON* phIconSmall, UINT nIconSize)
{
	int iIndex = bOpenIcon ? m_iOpenIconIndex : m_iIconIndex;
	if (phIconLarge)
	{
		auto hr = _ExtractIconImpl(iIndex, LOWORD(nIconSize), phIconLarge);
		if (FAILED(hr))
			return hr;
	}
	if (phIconSmall)
	{
		auto hr = _ExtractIconImpl(iIndex, HIWORD(nIconSize), phIconSmall);
		if (FAILED(hr))
		{
			if (phIconLarge)
			{
				::DestroyIcon(*phIconLarge);
				*phIconLarge = NULL;
			}
			return hr;
		}
	}
	return S_OK;
}

STDMETHODIMP CFTPFileItemIcon::Extract(LPCWSTR pszFile, UINT nIconIndex, HICON* phIconLarge, HICON* phIconSmall, UINT nIconSize)
{
	if (!pszFile)
		return E_POINTER;
	if (m_strFileName.Compare(pszFile) != 0)
		return E_INVALIDARG;
	return DoExtract(nIconIndex == 1, phIconLarge, phIconSmall, nIconSize);
}

STDMETHODIMP CFTPFileItemIcon::GetIconLocation(UINT uFlags, LPSTR pszIconFile, UINT cchMax, int* piIndex, UINT* pwFlags)
{
	if (!piIndex)
		return E_POINTER;
	if (cchMax > 0)
	{
		if (!pszIconFile)
			return E_POINTER;
		UINT dw = (UINT)m_strFileName.GetLengthA();
		if (cchMax >= dw + 1)
			cchMax = dw + 1;
		memcpy(pszIconFile, (LPCSTR)m_strFileName, sizeof(CHAR) * (cchMax - 1));
		pszIconFile[cchMax - 1] = 0;
	}
	if ((uFlags & GIL_OPENICON) && m_bIsDirectory)
		*piIndex = 1;
	else
		*piIndex = 0;
	if (pwFlags)
		*pwFlags = 0;
	return S_OK;
}

STDMETHODIMP CFTPFileItemIcon::Extract(LPCSTR pszFile, UINT nIconIndex, HICON* phIconLarge, HICON* phIconSmall, UINT nIconSize)
{
	if (!pszFile)
		return E_POINTER;
	if (m_strFileName.Compare(pszFile) != 0)
		return E_INVALIDARG;
	return DoExtract(nIconIndex == 1, phIconLarge, phIconSmall, nIconSize);
}
