/*
 EasySFTP - Copyright (C) 2025 jet (ジェット)

 FileProp.cpp - implementation of CFTPFileItemPropertyStore
 */

#include "stdafx.h"
#include "ShellDLL.h"
#include "FileProp.h"

#include "FileList.h"
#include "Folder.h"

EXTERN_C const struct { UINT nID; const PROPERTYKEY& key; } s_columnIDMap[];

///////////////////////////////////////////////////////////////////////////////

CFTPFileItemPropertyStore::CFTPFileItemPropertyStore(CFTPDirectoryBase* pDirectory, CFTPFileItem* pItem)
	: m_pDirectory(pDirectory), m_pItem(pItem)
{
	pDirectory->AddRef();
	pItem->AddRef();
}

CFTPFileItemPropertyStore::~CFTPFileItemPropertyStore()
{
	m_pItem->Release();
	m_pDirectory->Release();
}

STDMETHODIMP CFTPFileItemPropertyStore::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IPropertyStore))
	{
		*ppv = static_cast<IPropertyStore*>(this);
		AddRef();
		return S_OK;
	}
	*ppv = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP CFTPFileItemPropertyStore::GetCount(DWORD* cProps)
{
	if (!cProps)
		return E_POINTER;
	*cProps = _GetAvailablePropKeyCount();
	return S_OK;
}

STDMETHODIMP CFTPFileItemPropertyStore::GetAt(DWORD iProp, PROPERTYKEY* pkey)
{
	if (!pkey)
		return E_POINTER;
	if (iProp >= static_cast<DWORD>(_GetAvailablePropKeyCount()))
		return E_INVALIDARG;
	*pkey = s_columnIDMap[iProp].key;
	return S_OK;
}

typedef HRESULT(STDMETHODCALLTYPE* T_VariantToPropVariant)(_In_ const VARIANT* pVar, _Out_ PROPVARIANT* pPropVar);
static T_VariantToPropVariant s_VariantToPropVariant = NULL;

STDMETHODIMP CFTPFileItemPropertyStore::GetValue(REFPROPERTYKEY key, PROPVARIANT* pv)
{
	if (!s_VariantToPropVariant)
	{
		auto hModule = ::GetModuleHandleA("propsys.dll");
		if (!hModule)
			return E_NOTIMPL;
		s_VariantToPropVariant = reinterpret_cast<T_VariantToPropVariant>(::GetProcAddress(hModule, "VariantToPropVariant"));
		if (!s_VariantToPropVariant)
			return E_NOTIMPL;
	}

	VARIANT v;
	auto hr = _GetFileItemPropData(m_pDirectory, m_pItem, key, &v);
	if (FAILED(hr))
		return hr;
	hr = s_VariantToPropVariant(&v, pv);
	::VariantClear(&v);
	return hr;
}

STDMETHODIMP CFTPFileItemPropertyStore::SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar)
{
	(void)key;
	(void)propvar;
	return STG_E_ACCESSDENIED;
}

STDMETHODIMP CFTPFileItemPropertyStore::Commit()
{
	return STG_E_ACCESSDENIED;
}
