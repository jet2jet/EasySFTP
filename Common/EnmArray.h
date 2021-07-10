/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 EnmArray.h - declarations of CEnumArrayT
 */

#pragma once

#include "Array.h"
#include "Unknown.h"

template <class I, class T, class ARG_T>
class CEnumArrayT : public CUnknownImplT<I>
{
public:
	CEnumArrayT(CMySimpleArrayT<T, ARG_T>* pArray);

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);

	STDMETHOD(Next)(ULONG celt, T* rgelt, ULONG* pceltFetched);
	STDMETHOD(Skip)(ULONG celt);
	STDMETHOD(Reset)();
	STDMETHOD(Clone)(I** ppEnum);

protected:
	CMySimpleArrayT<T, ARG_T>* m_pArray;
	ULONG m_uPos;
};

template <class I, class T>
class CEnumArray : public CEnumArrayT<I, T, T>
{
public:
	CEnumArray(CMySimpleArrayT<T, T>* pArray) : CEnumArrayT<I, T, T>(pArray) { }
};

template <class I, class T, class ARG_T>
CEnumArrayT<I, T, ARG_T>::CEnumArrayT(CMySimpleArrayT<T, ARG_T>* pArray)
	: m_pArray(pArray)
	, m_uPos(0)
{
}

template <class I, class T, class ARG_T>
STDMETHODIMP CEnumArrayT<I, T, ARG_T>::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, __uuidof(I)))
	{
		*ppv = this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

template <class I, class T, class ARG_T>
STDMETHODIMP CEnumArrayT<I, T, ARG_T>::Next(ULONG celt, T* rgelt, ULONG* pceltFetched)
{
	if (!rgelt)
		return E_POINTER;
	if (!celt)
		return S_OK;
	if (pceltFetched)
		*pceltFetched = 0;
	while (m_uPos < (ULONG) m_pArray->GetCount())
	{
		if (pceltFetched)
			(*pceltFetched)++;
		*rgelt++ = m_pArray->GetItemRef((int) m_uPos++);
		if (!--celt)
			return S_OK;
	}
	return S_FALSE;
}

template <class I, class T, class ARG_T>
STDMETHODIMP CEnumArrayT<I, T, ARG_T>::Skip(ULONG celt)
{
	if (!celt)
		return S_OK;
	if (m_uPos + celt > (ULONG) m_pArray->GetCount())
	{
		m_uPos = (ULONG) m_pArray->GetCount();
		return S_FALSE;
	}
	m_uPos += celt;
	return S_OK;
}

template <class I, class T, class ARG_T>
STDMETHODIMP CEnumArrayT<I, T, ARG_T>::Reset()
{
	m_uPos = 0;
	return S_OK;
}

template <class I, class T, class ARG_T>
STDMETHODIMP CEnumArrayT<I, T, ARG_T>::Clone(I** ppEnum)
{
	CEnumArrayT<I, T, ARG_T>* pEnum = new CEnumArrayT(m_pArray);
	pEnum->m_uPos = m_uPos;
	*ppEnum = pEnum;
	return S_OK;
}
