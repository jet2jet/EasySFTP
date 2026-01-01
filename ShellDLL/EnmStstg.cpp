/*
 EasySFTP - Copyright (C) 2025 jet (ジェット)

 EnmStstg.cpp - implementation of CEnumFTPItemStatstg
 */

#include "stdafx.h"
#include "ShellDLL.h"
#include "EnmStstg.h"

 ////////////////////////////////////////////////////////////////////////////////

CEnumFTPItemStatstg::CEnumFTPItemStatstg(CDelegateMallocData* pMallocData,
	const CMyPtrArrayT<CFTPFileItem>& arrItems, bool bIsLockSupported,
	IUnknown* pUnkOuter)
	: m_pMallocData(pMallocData)
	, m_arrItems(arrItems)
	, m_uPos(0)
	, m_bIsLockSupported(bIsLockSupported)
	, m_pUnkOuter(pUnkOuter)
{
	m_pMallocData->AddRef();
	if (pUnkOuter)
		pUnkOuter->AddRef();
}

CEnumFTPItemStatstg::~CEnumFTPItemStatstg()
{
	if (m_pUnkOuter)
		m_pUnkOuter->Release();
	m_pMallocData->Release();
}

STDMETHODIMP CEnumFTPItemStatstg::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IEnumSTATSTG))
	{
		*ppv = this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP CEnumFTPItemStatstg::Next(ULONG celt, STATSTG* rgelt, ULONG* pceltFetched)
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
		rgelt->cbSize.QuadPart = pItem->uliSize.QuadPart;
		rgelt->pwcsName = DuplicateCoMemString(pItem->strFileName);
		rgelt->type = pItem->IsDirectory() ? STGTY_STORAGE : STGTY_STREAM;
		rgelt->mtime = pItem->ftModifyTime;
		rgelt->atime = {};
		rgelt->ctime = pItem->ftCreateTime;
		if (m_bIsLockSupported)
			rgelt->grfLocksSupported = LOCK_WRITE | LOCK_EXCLUSIVE;
		else
			rgelt->grfLocksSupported = 0;
		rgelt->clsid = CLSID_NULL;
		rgelt->grfStateBits = 0;

		if (pceltFetched)
			(*pceltFetched)++;
		rgelt++;
		if (!--celt)
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CEnumFTPItemStatstg::Skip(ULONG celt)
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

STDMETHODIMP CEnumFTPItemStatstg::Reset()
{
	m_uPos = 0;
	return S_OK;
}

STDMETHODIMP CEnumFTPItemStatstg::Clone(IEnumSTATSTG** ppEnum)
{
	CEnumFTPItemStatstg* pEnum = new CEnumFTPItemStatstg(m_pMallocData, m_arrItems, m_bIsLockSupported, m_pUnkOuter);
	pEnum->m_uPos = m_uPos;
	*ppEnum = pEnum;
	return S_OK;
}
