/*
 EasySFTP - Copyright (C) 2025 Kuri-Applications

 StrList.cpp - implementation of CEasySFTPStringList
 */

#include "stdafx.h"
#include "StrList.h"

#include "ShellDLL.h"

CEasySFTPStringList::CEasySFTPStringList(CMyStringArrayW* pstrArray, IUnknown* pRef)
	: CDispatchImplT(theApp.GetTypeInfo(IID_IEasySFTPStringList))
	, m_pstrArray(pstrArray)
	, m_pRef(pRef)
{
	pRef->AddRef();
}

CEasySFTPStringList::~CEasySFTPStringList()
{
	m_pRef->Release();
}

STDMETHODIMP CEasySFTPStringList::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	*ppv = NULL;
	if (!IsEqualIID(riid, IID_IUnknown) && !IsEqualIID(riid, IID_IDispatch) &&
		!IsEqualIID(riid, IID_IEasySFTPStringList))
		return E_NOINTERFACE;
	*ppv = static_cast<IEasySFTPStringList*>(this);
	AddRef();
	return S_OK;
}

STDMETHODIMP CEasySFTPStringList::get_Item(long Index, BSTR* pString)
{
	if (!pString)
		return E_POINTER;
	if (Index < 0 || Index >= m_pstrArray->GetCount())
		return DISP_E_BADINDEX;
	CMyStringW str(m_pstrArray->GetItem(Index));
	*pString = MyStringToBSTR(str);
	return *pString ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CEasySFTPStringList::put_Item(long Index, BSTR String)
{
	if (Index < 0 || Index > m_pstrArray->GetCount())
		return DISP_E_BADINDEX;
	CMyStringW str;
	MyBSTRToString(String, str);
	return m_pstrArray->SetItem(Index, str) ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CEasySFTPStringList::get_Count(long* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = m_pstrArray->GetCount();
	return S_OK;
}

STDMETHODIMP CEasySFTPStringList::Add(BSTR String, long* pRet)
{
	CMyStringW str;
	MyBSTRToString(String, str);
	auto i = m_pstrArray->Add(str);
	if (i < 0)
		return E_OUTOFMEMORY;
	if (pRet)
		*pRet = i;
	return S_OK;
}

STDMETHODIMP CEasySFTPStringList::Remove(long Index)
{
	if (Index < 0 || Index >= m_pstrArray->GetCount())
		return DISP_E_BADINDEX;
	m_pstrArray->RemoveItem(Index);
	return S_OK;
}

STDMETHODIMP CEasySFTPStringList::FindIndex(BSTR String, long* pRet)
{
	CMyStringW str;
	MyBSTRToString(String, str);
	auto i = m_pstrArray->FindItem(str);
	if (pRet)
		*pRet = i;
	return S_OK;
}

STDMETHODIMP CEasySFTPStringList::get__NewEnum(IUnknown** ppRet)
{
	if (!ppRet)
		return E_POINTER;
	*ppRet = new CEnum(this);
	return *ppRet ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CEasySFTPStringList::CEnum::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	*ppv = NULL;
	if (!IsEqualIID(riid, IID_IUnknown) &&
		!IsEqualIID(riid, IID_IEnumVARIANT))
		return E_NOINTERFACE;
	*ppv = static_cast<IEnumVARIANT*>(this);
	AddRef();
	return S_OK;
}

STDMETHODIMP CEasySFTPStringList::CEnum::Next(ULONG celt, VARIANT* rgelt, ULONG* pceltFetched)
{
	if (!rgelt)
		return E_POINTER;
	if (!celt)
		return S_OK;
	if (pceltFetched)
		*pceltFetched = 0;
	auto count = static_cast<ULONG>(m_pList->m_pstrArray->GetCount());
	while (m_uPos < count)
	{
		CMyStringW str = m_pList->m_pstrArray->GetItem(static_cast<int>(m_uPos++));
		rgelt->bstrVal = MyStringToBSTR(str);
		if (!rgelt->bstrVal)
			return E_OUTOFMEMORY;
		rgelt->vt = VT_BSTR;
		if (pceltFetched)
			(*pceltFetched)++;
		rgelt++;
		if (!--celt)
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CEasySFTPStringList::CEnum::Skip(ULONG celt)
{
	if (!celt)
		return S_OK;
	auto count = static_cast<ULONG>(m_pList->m_pstrArray->GetCount());
	if (m_uPos + celt > count)
	{
		m_uPos = count;
		return S_FALSE;
	}
	m_uPos += celt;
	return S_OK;
}

STDMETHODIMP CEasySFTPStringList::CEnum::Reset()
{
	m_uPos = 0;
	return S_OK;
}

STDMETHODIMP CEasySFTPStringList::CEnum::Clone(IEnumVARIANT** ppEnum)
{
	if (!ppEnum)
		return E_POINTER;
	CEnum* pEnum = new CEnum(m_pList);
	if (!pEnum)
		return E_OUTOFMEMORY;
	pEnum->m_uPos = m_uPos;
	*ppEnum = pEnum;
	return S_OK;
}
