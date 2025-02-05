/*
 EasySFTP - Copyright (C) 2025 Kuri-Applications

 FileDisp.cpp - implementation of CFTPFileItemDisplayName
 */

#include "stdafx.h"
#include "ShellDLL.h"
#include "FileDisp.h"

 ///////////////////////////////////////////////////////////////////////////////

CFTPFileItemDisplayName::CFTPFileItemDisplayName(PIDLIST_ABSOLUTE pidl)
	: m_pidlMe(pidl)
{
}

CFTPFileItemDisplayName::~CFTPFileItemDisplayName()
{
	::CoTaskMemFree(m_pidlMe);
}

STDMETHODIMP CFTPFileItemDisplayName::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IRelatedItem) || IsEqualIID(riid, IID_IDisplayItem))
	{
		*ppv = static_cast<IDisplayItem*>(this);
		AddRef();
		return S_OK;
	}
	*ppv = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP CFTPFileItemDisplayName::GetItemIDList(PIDLIST_ABSOLUTE* ppidl)
{
	if (!ppidl)
		return E_POINTER;
	*ppidl = reinterpret_cast<PIDLIST_ABSOLUTE>(::DuplicateItemIDList(m_pidlMe));
	return *ppidl ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CFTPFileItemDisplayName::GetItem(IShellItem** ppsi)
{
	if (!ppsi)
		return E_POINTER;
	return ::MyCreateShellItem(m_pidlMe, ppsi);
}
