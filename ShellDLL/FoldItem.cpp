/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 FoldItem.cpp - implementations of CFolderShellItem and folder-item-helper classes
 */

#include "stdafx.h"
#include "ShellDLL.h"
#include "Folder.h"
#include "FoldItem.h"

// in FoldBase.cpp
extern "C" SHGDNF __stdcall ConvertDisplayNameFlags(SIGDN flags);

////////////////////////////////////////////////////////////////////////////////

CFolderShellItem::CFolderShellItem(CFolderBase* pParent, PCUITEMID_CHILD pidlChild)
	: m_pParent(pParent)
{
	m_pidlChild = (PITEMID_CHILD) ::DuplicateItemIDList(pidlChild);
	pParent->AddRef();
}

CFolderShellItem::~CFolderShellItem()
{
	m_pParent->Release();
	::CoTaskMemFree(m_pidlChild);
}

STDMETHODIMP CFolderShellItem::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IShellItem))
	{
		*ppv = (IShellItem*) this;
		AddRef();
		return S_OK;
	}
	if (IsEqualIID(riid, IID_IPersistIDList))
	{
		*ppv = (IPersistIDList*) this;
		AddRef();
		return S_OK;
	}
	// NOTE: we must set NULL to *ppv, or causes 0xC000041D exception in Windows 7 (x64)
	*ppv = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CFolderShellItem::AddRef()
{
	return (ULONG) ::InterlockedIncrement((LONG*) &m_uRef);
}

STDMETHODIMP_(ULONG) CFolderShellItem::Release()
{
	ULONG ret;
	ret = (ULONG) ::InterlockedDecrement((LONG*) &m_uRef);

	if (!ret)
		delete this;
	return ret;
}

STDMETHODIMP CFolderShellItem::BindToHandler(IBindCtx* pbc, REFGUID bhid, REFIID riid, void** ppv)
{
	if (IsEqualGUID(bhid, BHID_SFObject))
		return m_pParent->BindToObject(m_pidlChild, pbc, riid, ppv);
	else if (IsEqualGUID(bhid, BHID_SFViewObject))
		return m_pParent->CreateViewObject(NULL, riid, ppv);
	else if (IsEqualGUID(bhid, BHID_SFUIObject) || IsEqualGUID(bhid, BHID_DataObject))
	{
		PCITEMID_CHILD pidlChild = m_pidlChild;
		return m_pParent->GetUIObjectOf(NULL, 1, &pidlChild, riid, NULL, ppv);
	}
	return E_NOTIMPL;
}

STDMETHODIMP CFolderShellItem::GetParent(IShellItem** ppsi)
{
	return m_pParent->QueryInterface(IID_IShellItem, (void**) ppsi);
}

STDMETHODIMP CFolderShellItem::GetDisplayName(SIGDN sigdnName, LPWSTR* ppszName)
{
	STRRET strret;
	HRESULT hr;

	if (!ppszName)
		return E_POINTER;

	strret.uType = STRRET_WSTR;
	hr = m_pParent->GetDisplayNameOf(m_pidlChild, ConvertDisplayNameFlags(sigdnName), &strret);
	if (FAILED(hr))
		return hr;
	switch (strret.uType)
	{
		case STRRET_WSTR:
			*ppszName = strret.pOleStr;
			break;
		case STRRET_CSTR:
		{
			CMyStringW str(strret.cStr);
			SIZE_T nSize = sizeof(WCHAR) * (str.GetLength() + 1);
			LPWSTR lpw = (LPWSTR) ::CoTaskMemAlloc(nSize);
			if (!lpw)
				return E_OUTOFMEMORY;
			memcpy(lpw, (LPCWSTR) str, nSize);
			*ppszName = lpw;
		}
		break;
		case STRRET_OFFSET:
		{
			CMyStringW str((LPCSTR) (((LPCBYTE) m_pidlChild) + strret.uOffset));
			SIZE_T nSize = sizeof(WCHAR) * (str.GetLength() + 1);
			LPWSTR lpw = (LPWSTR) ::CoTaskMemAlloc(nSize);
			if (!lpw)
				return E_OUTOFMEMORY;
			memcpy(lpw, (LPCWSTR) str, nSize);
			*ppszName = lpw;
		}
		break;
		default:
			return E_UNEXPECTED;
	}
	return S_OK;
}

STDMETHODIMP CFolderShellItem::GetAttributes(SFGAOF sfgaoMask, SFGAOF* psfgaoAttribs)
{
	SFGAOF attrs;
	if (!psfgaoAttribs)
		return E_POINTER;
	attrs = sfgaoMask;
	PCITEMID_CHILD pidlChild = m_pidlChild;
	HRESULT hr = m_pParent->GetAttributesOf(1, &pidlChild, &attrs);
	*psfgaoAttribs = attrs;
	return hr;
}

STDMETHODIMP CFolderShellItem::Compare(IShellItem* psi, SICHINTF hint, int* piOrder)
{
	HRESULT hr;
	PIDLIST_ABSOLUTE pidlItemAbsolute;
	PIDLIST_ABSOLUTE pidlParent;
	PITEMID_CHILD pidlItem;
	IPersistIDList* pPIDList;

	hr = psi->QueryInterface(IID_IPersistIDList, (void**) &pPIDList);
	if (FAILED(hr))
		return hr;
	hr = pPIDList->GetIDList(&pidlItemAbsolute);
	if (FAILED(hr))
	{
		pPIDList->Release();
		return hr;
	}
	pidlParent = ::RemoveOneChild(pidlItemAbsolute);
	pidlItem = ::GetChildItemIDList(pidlItemAbsolute);
	::CoTaskMemFree(pidlItemAbsolute);
	pPIDList->Release();
	if (!pidlParent || !pidlItem)
	{
		if (pidlParent)
			::CoTaskMemFree(pidlParent);
		if (pidlItem)
			::CoTaskMemFree(pidlItem);
		return E_OUTOFMEMORY;
	}

	if (!::IsEqualIDList(pidlParent, m_pParent->m_pidlMe))
	{
		::CoTaskMemFree(pidlParent);
		::CoTaskMemFree(pidlItem);
		if (piOrder)
			*piOrder = 1;
		return S_FALSE;
	}
	::CoTaskMemFree(pidlParent);

	switch (hint)
	{
		case SICHINT_DISPLAY:
			hr = m_pParent->CompareIDs(0, m_pidlChild, pidlItem);
			break;
		case SICHINT_CANONICAL:
			hr = m_pParent->CompareIDs(SHCIDS_CANONICALONLY, m_pidlChild, pidlItem);
			break;
		case static_cast<SICHINTF>(SICHINT_ALLFIELDS):
			hr = m_pParent->CompareIDs(SHCIDS_ALLFIELDS, m_pidlChild, pidlItem);
			break;
		default:
			hr = E_NOTIMPL;
	}
	::CoTaskMemFree(pidlItem);
	if (SUCCEEDED(hr))
	{
		if (piOrder)
			*piOrder = (int)(short) HRESULT_CODE(hr);
		hr = (HRESULT_CODE(hr) == 0 ? S_OK : S_FALSE);
	}
	return hr;
}

STDMETHODIMP CFolderShellItem::GetClassID(CLSID* pClassID)
{
	if (!pClassID)
		return E_POINTER;
	*pClassID = GUID_NULL;
	return S_OK;
}

STDMETHODIMP CFolderShellItem::GetIDList(PIDLIST_ABSOLUTE* ppidl)
{
	if (!ppidl)
		return E_POINTER;
	*ppidl = ::AppendItemIDList(m_pParent->m_pidlMe, m_pidlChild);
	return *ppidl ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CFolderShellItem::SetIDList(PCIDLIST_ABSOLUTE pidl)
{
	return E_ACCESSDENIED;
}


CEnumFolderShellItem::CEnumFolderShellItem(CFolderBase* pTarget)
	: m_pTarget(pTarget)
{
	pTarget->AddRef();
	m_pEnumIDList = NULL;
}

CEnumFolderShellItem::~CEnumFolderShellItem()
{
	if (m_pEnumIDList)
		m_pEnumIDList->Release();
	m_pTarget->Release();
}

STDMETHODIMP CEnumFolderShellItem::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IEnumShellItems))
	{
		*ppv = (IEnumShellItems*) this;
		AddRef();
		return S_OK;
	}
	// NOTE: we must set NULL to *ppv, or causes 0xC000041D exception in Windows 7 (x64)
	*ppv = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CEnumFolderShellItem::AddRef()
{
	return (ULONG) ::InterlockedIncrement((LONG*) &m_uRef);
}

STDMETHODIMP_(ULONG) CEnumFolderShellItem::Release()
{
	ULONG ret;
	ret = (ULONG) ::InterlockedDecrement((LONG*) &m_uRef);

	if (!ret)
		delete this;
	return ret;
}

STDMETHODIMP CEnumFolderShellItem::Next(ULONG celt, IShellItem** rgelt, ULONG* pceltFetched)
{
	HRESULT hr;
	hr = InitializeEnumIDList();
	if (FAILED(hr))
		return hr;
	PITEMID_CHILD* pidls = (PITEMID_CHILD*) malloc(sizeof(PITEMID_CHILD) * celt);
	if (!pidls)
		return E_OUTOFMEMORY;
	ULONG uc = 0;
	hr = m_pEnumIDList->Next(celt, pidls, &uc);
	if (FAILED(hr))
	{
		free(pidls);
		return hr;
	}
	for (ULONG u = 0; u < uc; u++)
	{
		rgelt[u] = new CFolderShellItem(m_pTarget, pidls[u]);
		::CoTaskMemFree(pidls[u]);
		if (!rgelt[u])
		{
			for (ULONG u2 = u + 1; u2 < uc; u2++)
				::CoTaskMemFree(pidls[u2]);
			while (u--)
			{
				rgelt[u]->Release();
				rgelt[u] = NULL;
			}
			return E_OUTOFMEMORY;
		}
	}
	if (pceltFetched)
		*pceltFetched = uc;
	return hr;
}

STDMETHODIMP CEnumFolderShellItem::Skip(ULONG celt)
{
	HRESULT hr;
	hr = InitializeEnumIDList();
	if (FAILED(hr))
		return hr;
	hr = m_pEnumIDList->Skip(celt);
	return hr;
}

STDMETHODIMP CEnumFolderShellItem::Reset()
{
	if (m_pEnumIDList)
		return m_pEnumIDList->Reset();
	return S_OK;
}

STDMETHODIMP CEnumFolderShellItem::Clone(IEnumShellItems** ppEnum)
{
	if (!ppEnum)
		return E_POINTER;
	CEnumFolderShellItem* p = new CEnumFolderShellItem(m_pTarget);
	if (!p)
		return E_OUTOFMEMORY;
	if (m_pEnumIDList)
	{
		HRESULT hr = m_pEnumIDList->Clone(&p->m_pEnumIDList);
		if (FAILED(hr))
		{
			p->m_pEnumIDList = NULL;
			p->Release();
			return hr;
		}
	}
	*ppEnum = p;
	return S_OK;
}

#ifndef SHCONTF_INCLUDESUPERHIDDEN
#define SHCONTF_INCLUDESUPERHIDDEN 0x10000
#endif

HRESULT CEnumFolderShellItem::InitializeEnumIDList()
{
	if (m_pEnumIDList)
		return S_OK;
	HRESULT hr = m_pTarget->EnumObjects(NULL,
		SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_INCLUDESUPERHIDDEN,
		&m_pEnumIDList);
	if (FAILED(hr))
		m_pEnumIDList = NULL;
	else if (!m_pEnumIDList)
		hr = E_FAIL;
	return hr;
}
