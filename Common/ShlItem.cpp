/*
 Copyright (C) 2025 jet (ジェット)

 ShlItem.cpp - implementations of utility functions for IShellItem
 */

#include "stdafx.h"
#include "ShlItem.h"

#include "IDList.h"

typedef HRESULT(STDAPICALLTYPE* T_SHCreateItemFromIDList)(__in PCIDLIST_ABSOLUTE pidl, __in REFIID riid, __deref_out void** ppv);
typedef HRESULT(STDAPICALLTYPE* T_SHCreateShellItem)(__in_opt PCIDLIST_ABSOLUTE pidlParent, __in_opt IShellFolder* psfParent, __in PCUITEMID_CHILD pidl, __out IShellItem** ppsi);
typedef HRESULT(STDAPICALLTYPE* T_SHCreateItemFromRelativeName)(_In_ IShellItem* psiParent, _In_ PCWSTR pszName, _In_opt_ IBindCtx* pbc, _In_ REFIID riid, _Outptr_ void** ppv);
static bool s_bSHItemFuncInitialized = false;
static T_SHCreateItemFromIDList s_pfnSHCreateItemFromIDList = NULL;
static T_SHCreateShellItem s_pfnSHCreateShellItem = NULL;
static T_SHCreateItemFromRelativeName s_pfnSHCreateItemFromRelativeName = NULL;

static void _InitShellItemApi()
{
	if (!s_bSHItemFuncInitialized)
	{
		HINSTANCE hInstShell32 = ::GetModuleHandle(_T("shell32.dll"));
		if (!hInstShell32)
			hInstShell32 = ::LoadLibrary(_T("shell32.dll"));
		if (hInstShell32)
		{
			s_pfnSHCreateItemFromIDList = (T_SHCreateItemFromIDList) ::GetProcAddress(hInstShell32, "SHCreateItemFromIDList");
			if (!s_pfnSHCreateItemFromIDList)
				s_pfnSHCreateShellItem = (T_SHCreateShellItem) ::GetProcAddress(hInstShell32, "SHCreateShellItem");
			s_pfnSHCreateItemFromRelativeName = (T_SHCreateItemFromRelativeName) ::GetProcAddress(hInstShell32, "SHCreateItemFromRelativeName");
		}
		s_bSHItemFuncInitialized = true;
	}
}

STDAPI MyCreateShellItem(PCIDLIST_ABSOLUTE pidl, IShellItem** ppItem)
{
	_InitShellItemApi();

	if (s_pfnSHCreateItemFromIDList)
		return s_pfnSHCreateItemFromIDList(pidl, IID_IShellItem, (void**)ppItem);
	if (s_pfnSHCreateShellItem)
		return s_pfnSHCreateShellItem(NULL, NULL, (PCUITEMID_CHILD)pidl, ppItem);
	return E_NOTIMPL;
}

STDAPI MyCreateShellItemOfFolder(IShellFolder* pFolder, IShellItem** ppItem)
{
	IPersistFolder2* pPersist;
	if (FAILED(pFolder->QueryInterface(IID_IPersistFolder2, reinterpret_cast<void**>(&pPersist))))
		return E_NOTIMPL;
	PIDLIST_ABSOLUTE pidlFolder = NULL;
	auto hr = pPersist->GetCurFolder(&pidlFolder);
	pPersist->Release();
	if (FAILED(hr))
		return hr;
	if (!pidlFolder)
		return E_OUTOFMEMORY;
	hr = MyCreateShellItem(pidlFolder, ppItem);
	::CoTaskMemFree(pidlFolder);
	return hr;
}

STDAPI MyCreateShellItemOnFolder(IShellFolder* pFolder, PCUIDLIST_RELATIVE pidl, IShellItem** ppItem)
{
	IPersistFolder2* pPersist;
	if (FAILED(pFolder->QueryInterface(IID_IPersistFolder2, reinterpret_cast<void**>(&pPersist))))
		return E_NOTIMPL;
	PIDLIST_ABSOLUTE pidlFolder = NULL;
	auto hr = pPersist->GetCurFolder(&pidlFolder);
	pPersist->Release();
	if (FAILED(hr))
		return hr;
	if (!pidlFolder)
		return E_OUTOFMEMORY;
	auto pidlFull = ::AppendItemIDList(pidlFolder, pidl);
	::CoTaskMemFree(pidlFolder);
	if (!pidlFull)
		return E_OUTOFMEMORY;
	hr = MyCreateShellItem(pidlFull, ppItem);
	::CoTaskMemFree(pidlFull);
	return hr;
}

STDAPI MyCreateShellItemFromRelativeName(IShellItem* pItemParent, LPCWSTR lpszRelativeName, IBindCtx* pb, IShellItem** ppItem)
{
	_InitShellItemApi();

	if (!s_pfnSHCreateItemFromRelativeName)
		return E_NOTIMPL;
	return s_pfnSHCreateItemFromRelativeName(pItemParent, lpszRelativeName, pb, IID_IShellItem, reinterpret_cast<void**>(ppItem));
}
