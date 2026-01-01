/*
 EasySFTP - Copyright (C) 2025 jet (ジェット)

 EnmIDLst.cpp - implementation of CEnumFTPItemIDList
 */

#include "stdafx.h"
#include "ShellDLL.h"
#include "EnmIDLst.h"

 ////////////////////////////////////////////////////////////////////////////////

CEnumFTPItemIDList::CEnumFTPItemIDList(CDelegateMallocData* pMallocData,
	const CMyPtrArrayT<CFTPFileItem>& arrItems,
	SHCONTF grfFlags, IUnknown* pUnkOuter)
	: m_pMallocData(pMallocData)
	, m_arrItems(arrItems)
	, m_grfFlags(grfFlags)
	, m_uPos(0)
	, m_pUnkOuter(pUnkOuter)
{
	m_pMallocData->AddRef();
	if (pUnkOuter)
		pUnkOuter->AddRef();
}

CEnumFTPItemIDList::~CEnumFTPItemIDList()
{
	if (m_pUnkOuter)
		m_pUnkOuter->Release();
	m_pMallocData->Release();
}

STDMETHODIMP CEnumFTPItemIDList::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IEnumIDList))
	{
		*ppv = this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP CEnumFTPItemIDList::Next(ULONG celt, PITEMID_CHILD* rgelt, ULONG* pceltFetched)
{
	if (!rgelt)
		return E_POINTER;
	if (!celt)
		return S_OK;
	if (pceltFetched)
		*pceltFetched = 0;
	while (m_uPos < (ULONG)m_arrItems.GetCount())
	{
		CFTPFileItem* pItem = m_arrItems.GetItem((int)(m_uPos++));
		bool bFetch = false;
		if (pItem->IsDirectory())
			bFetch = ((m_grfFlags & SHCONTF_FOLDERS) != 0);
		else
			bFetch = ((m_grfFlags & SHCONTF_NONFOLDERS) != 0);
		if (bFetch && !(m_grfFlags & SHCONTF_INCLUDEHIDDEN))
			bFetch = !pItem->IsHidden();
		if (bFetch)
		{
			*rgelt = ::CreateFileItem(m_pMallocData->pMalloc, pItem);
			if (*rgelt && pceltFetched)
				(*pceltFetched)++;
			rgelt++;
			if (!--celt)
				return S_OK;
		}
	}
	return S_FALSE;
}

STDMETHODIMP CEnumFTPItemIDList::Skip(ULONG celt)
{
	if (!celt)
		return S_OK;
	if (m_uPos + celt > (ULONG)m_arrItems.GetCount())
	{
		m_uPos = (ULONG)m_arrItems.GetCount();
		return S_FALSE;
	}
	m_uPos += celt;
	return S_OK;
}

STDMETHODIMP CEnumFTPItemIDList::Reset()
{
	m_uPos = 0;
	return S_OK;
}

STDMETHODIMP CEnumFTPItemIDList::Clone(IEnumIDList** ppEnum)
{
	CEnumFTPItemIDList* pEnum = new CEnumFTPItemIDList(m_pMallocData, m_arrItems, m_grfFlags, m_pUnkOuter);
	pEnum->m_uPos = m_uPos;
	*ppEnum = pEnum;
	return S_OK;
}
