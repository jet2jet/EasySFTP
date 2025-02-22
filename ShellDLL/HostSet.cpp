/*
 EasySFTP - Copyright (C) 2025 Kuri-Applications

 HostSet.cpp - implementation of CEasySFTPHostSetting
 */

#include "stdafx.h"
#include "ShellDLL.h"
#include "HostSet.h"

#include "StrList.h"
#include "RFolder.h"

CEasySFTPHostSetting::CEasySFTPHostSetting()
	: CDispatchImplT(theApp.GetTypeInfo(IID_IEasySFTPHostSetting))
{
}

CEasySFTPHostSetting::CEasySFTPHostSetting(const CEasySFTPHostSetting* pSettings)
	: CDispatchImplT(theApp.GetTypeInfo(IID_IEasySFTPHostSetting))
	, ConnectionMode(pSettings->ConnectionMode)
	, strDisplayName(pSettings->strDisplayName)
	, strHostName(pSettings->strHostName)
	, nPort(pSettings->nPort)
	//, strUserName(pSettings->strUserName)
	, strInitLocalPath(pSettings->strInitLocalPath)
	, strInitServerPath(pSettings->strInitServerPath)
	, bTextMode(pSettings->bTextMode)
	, nServerCharset(pSettings->nServerCharset)
	, nTransferMode(pSettings->nTransferMode)
	, bUseSystemTextFileType(pSettings->bUseSystemTextFileType)
	, bAdjustRecvModifyTime(pSettings->bAdjustRecvModifyTime)
	, bAdjustSendModifyTime(pSettings->bAdjustSendModifyTime)
	, strChmodCommand(pSettings->strChmodCommand)
{
	arrTextFileType.CopyArray(pSettings->arrTextFileType);
}

STDMETHODIMP CEasySFTPHostSetting::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	*ppv = NULL;
	if (!IsEqualIID(riid, IID_IUnknown) && !IsEqualIID(riid, IID_IDispatch) &&
		!IsEqualIID(riid, IID_IEasySFTPHostSetting))
		return E_NOINTERFACE;
	*ppv = static_cast<IEasySFTPHostSetting*>(this);
	AddRef();
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::get_Name(BSTR* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = MyStringToBSTR(strDisplayName);
	return *pRet ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CEasySFTPHostSetting::put_Name(BSTR Value)
{
	MyBSTRToString(Value, strDisplayName);
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::get_HostName(BSTR* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = MyStringToBSTR(strHostName);
	return *pRet ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CEasySFTPHostSetting::put_HostName(BSTR Value)
{
	MyBSTRToString(Value, strHostName);
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::get_ConnectionMode(EasySFTPConnectionMode* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = ConnectionMode;
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::put_ConnectionMode(EasySFTPConnectionMode Value)
{
	if (Value < EasySFTPConnectionMode::SFTP || Value > EasySFTPConnectionMode::FTPS)
		return E_INVALIDARG;
	ConnectionMode = Value;
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::get_Port(long* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = static_cast<long>(nPort);
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::put_Port(long Value)
{
	if (Value < 0 || Value > 65535)
		return E_INVALIDARG;
	nPort = Value;
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::get_InitLocalPath(BSTR* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = MyStringToBSTR(strInitLocalPath);
	return *pRet ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CEasySFTPHostSetting::put_InitLocalPath(BSTR Value)
{
	MyBSTRToString(Value, strInitLocalPath);
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::get_InitServerPath(BSTR* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = MyStringToBSTR(strInitServerPath);
	return *pRet ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CEasySFTPHostSetting::put_InitServerPath(BSTR Value)
{
	MyBSTRToString(Value, strInitServerPath);
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::get_TextMode(EasySFTPTextMode* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = static_cast<EasySFTPTextMode>(bTextMode);
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::put_TextMode(EasySFTPTextMode Value)
{
	auto i = static_cast<int>(Value);
	if (i < 0 || i > 0xFF)
		return E_INVALIDARG;
	bTextMode = static_cast<BYTE>(i);
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::get_ServerCharset(EasySFTPServerCharset* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = static_cast<EasySFTPServerCharset>(nServerCharset);
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::put_ServerCharset(EasySFTPServerCharset Value)
{
	if (Value < EasySFTPServerCharset::UTF8 || Value > EasySFTPServerCharset::EUC)
		return E_INVALIDARG;
	nServerCharset = static_cast<decltype(nServerCharset)>(Value);
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::get_TransferMode(EasySFTPTransferMode* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = static_cast<EasySFTPTransferMode>(nTransferMode);
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::put_TransferMode(EasySFTPTransferMode Value)
{
	if (Value < EasySFTPTransferMode::Auto || Value > EasySFTPTransferMode::Binary)
		return E_INVALIDARG;
	nTransferMode = static_cast<decltype(nTransferMode)>(Value);
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::get_TextFileType(IEasySFTPStringList** ppList)
{
	if (!ppList)
		return E_POINTER;
	*ppList = new CEasySFTPStringList(&arrTextFileType, this);
	return *ppList ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CEasySFTPHostSetting::get_UseSystemTextFileType(VARIANT_BOOL* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = bUseSystemTextFileType ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::put_UseSystemTextFileType(VARIANT_BOOL Value)
{
	bUseSystemTextFileType = (Value != VARIANT_FALSE);
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::get_AdjustRecvModifyTime(VARIANT_BOOL* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = bAdjustRecvModifyTime ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::put_AdjustRecvModifyTime(VARIANT_BOOL Value)
{
	bAdjustRecvModifyTime = (Value != VARIANT_FALSE);
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::get_AdjustSendModifyTime(VARIANT_BOOL* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = bAdjustSendModifyTime ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::put_AdjustSendModifyTime(VARIANT_BOOL Value)
{
	bAdjustSendModifyTime = (Value != VARIANT_FALSE);
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::get_UseThumbnailPreview(VARIANT_BOOL* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = bUseThumbnailPreview ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::put_UseThumbnailPreview(VARIANT_BOOL Value)
{
	bUseThumbnailPreview = (Value != VARIANT_FALSE);
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::get_ChmodCommand(BSTR* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = MyStringToBSTR(strChmodCommand);
	return *pRet ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CEasySFTPHostSetting::put_ChmodCommand(BSTR Value)
{
	MyBSTRToString(Value, strChmodCommand);
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::CopyFrom(IEasySFTPHostSetting* pSetting)
{
	HRESULT hr;
	BSTR bstr;
	do
	{
		hr = pSetting->get_Name(&bstr);
		if (FAILED(hr))
			break;
		MyBSTRToString(bstr, strDisplayName);
		::SysFreeString(bstr);
		hr = pSetting->get_HostName(&bstr);
		if (FAILED(hr))
			break;
		MyBSTRToString(bstr, strHostName);
		::SysFreeString(bstr);
		hr = pSetting->get_ConnectionMode(&ConnectionMode);
		if (FAILED(hr))
			break;
		static_assert(sizeof(long) == sizeof(int), "Unexpected");
		hr = pSetting->get_Port(reinterpret_cast<long*>(&nPort));
		if (FAILED(hr))
			break;
		hr = pSetting->get_InitLocalPath(&bstr);
		if (FAILED(hr))
			break;
		MyBSTRToString(bstr, strInitLocalPath);
		::SysFreeString(bstr);
		hr = pSetting->get_InitServerPath(&bstr);
		if (FAILED(hr))
			break;
		MyBSTRToString(bstr, strInitServerPath);
		::SysFreeString(bstr);
		{
			EasySFTPTextMode mode;
			hr = pSetting->get_TextMode(&mode);
			if (FAILED(hr))
				break;
			bTextMode = static_cast<BYTE>(mode);
		}
		{
			EasySFTPServerCharset charset;
			hr = pSetting->get_ServerCharset(&charset);
			if (FAILED(hr))
				break;
			nServerCharset = static_cast<char>(charset);
		}
		{
			EasySFTPTransferMode mode;
			hr = pSetting->get_TransferMode(&mode);
			if (FAILED(hr))
				break;
			nTransferMode = static_cast<char>(mode);
		}
		{
			arrTextFileType.RemoveAll();
			IEasySFTPStringList* pList;
			hr = pSetting->get_TextFileType(&pList);
			if (FAILED(hr))
				break;
			long c = 0;
			hr = pList->get_Count(&c);
			if (SUCCEEDED(hr))
			{
				CMyStringW str;
				for (long i = 0; i < c; ++i)
				{
					hr = pList->get_Item(i, &bstr);
					if (FAILED(hr))
						break;
					MyBSTRToString(bstr, str);
					arrTextFileType.Add(str);
				}
			}
			pList->Release();
			if (FAILED(hr))
				break;
		}
		{
			VARIANT_BOOL b;
			hr = pSetting->get_UseSystemTextFileType(&b);
			if (FAILED(hr))
				break;
			bUseSystemTextFileType = b != VARIANT_FALSE;
		}
		{
			VARIANT_BOOL b;
			hr = pSetting->get_AdjustRecvModifyTime(&b);
			if (FAILED(hr))
				break;
			bAdjustRecvModifyTime = b != VARIANT_FALSE;
		}
		{
			VARIANT_BOOL b;
			hr = pSetting->get_AdjustSendModifyTime(&b);
			if (FAILED(hr))
				break;
			bAdjustSendModifyTime = b != VARIANT_FALSE;
		}
		{
			VARIANT_BOOL b;
			hr = pSetting->get_UseThumbnailPreview(&b);
			if (FAILED(hr))
				break;
			bUseThumbnailPreview = b != VARIANT_FALSE;
		}
		hr = pSetting->get_ChmodCommand(&bstr);
		if (FAILED(hr))
			break;
		MyBSTRToString(bstr, strChmodCommand);
		::SysFreeString(bstr);
	} while (0);
	return hr;
}

///////////////////////////////////////////////////////////////////////////////

CEasySFTPHostSettingList::CEasySFTPHostSettingList(CEasySFTPFolderRoot* pParent)
	: CDispatchImplT(theApp.GetTypeInfo(IID_IEasySFTPHostSettingList))
	, m_pParent(pParent)
{
	pParent->AddRef();
}

CEasySFTPHostSettingList::~CEasySFTPHostSettingList()
{
	m_pParent->Release();
}

STDMETHODIMP CEasySFTPHostSettingList::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	*ppv = NULL;
	if (!IsEqualIID(riid, IID_IUnknown) && !IsEqualIID(riid, IID_IDispatch) &&
		!IsEqualIID(riid, IID_IEasySFTPHostSettingList))
		return E_NOINTERFACE;
	*ppv = static_cast<IEasySFTPHostSettingList*>(this);
	AddRef();
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSettingList::get_Item(long Index, IEasySFTPHostSetting** ppSetting)
{
	if (!ppSetting)
		return E_POINTER;
	if (Index < 0 || Index >= theApp.m_aHosts.GetCount())
		return DISP_E_BADINDEX;
	auto p = theApp.m_aHosts.GetItem(static_cast<int>(Index))->pSettings;
	*ppSetting = p;
	if (p)
		p->AddRef();
	return p ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CEasySFTPHostSettingList::get_Count(long* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = theApp.m_aHosts.GetCount();
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSettingList::Add(IEasySFTPHostSetting* pSetting, long* pRet)
{
	if (pRet)
		*pRet = -1;
	HRESULT hr;
	CEasySFTPHostSetting* pWrapped = new CEasySFTPHostSetting();
	if (!pWrapped)
		return E_OUTOFMEMORY;
	hr = pWrapped->CopyFrom(pSetting);
	if (SUCCEEDED(hr))
	{
		hr = m_pParent->AddHostSettings(pWrapped);
		if (SUCCEEDED(hr))
		{
			long found = -1;
			for (long i = 0; i < theApp.m_aHosts.GetCount(); ++i)
			{
				auto* p = theApp.m_aHosts.GetItem(i);
				if (p->pSettings->ConnectionMode == pWrapped->ConnectionMode &&
					p->pSettings->strDisplayName.Compare(pWrapped->strDisplayName) == 0 &&
					p->pSettings->strHostName.Compare(pWrapped->strHostName) == 0 &&
					p->pSettings->nPort == pWrapped->nPort)
				{
					found = i;
					break;
				}
			}
			if (found < 0)
				hr = E_FAIL;
			if (pRet)
				*pRet = found;
		}
	}
	pWrapped->Release();
	return hr;
}

STDMETHODIMP CEasySFTPHostSettingList::Remove(long Index)
{
	if (Index < 0 || Index >= theApp.m_aHosts.GetCount())
		return DISP_E_BADINDEX;
	auto* p = theApp.m_aHosts.GetItem(Index);
	return m_pParent->RemoveHostSettings(p->pSettings);
}

STDMETHODIMP CEasySFTPHostSettingList::FindHost(BSTR HostName, EasySFTPConnectionMode Mode, long Port, IEasySFTPHostSetting** ppRet)
{
	if (!ppRet)
		return E_POINTER;
	*ppRet = NULL;
	CMyStringW strHostName;
	MyBSTRToString(HostName, strHostName);
	for (long i = 0; i < theApp.m_aHosts.GetCount(); ++i)
	{
		auto* p = theApp.m_aHosts.GetItem(i);
		if (p->pSettings->ConnectionMode == Mode &&
			p->pSettings->strDisplayName.Compare(strHostName) == 0 &&
			p->pSettings->nPort == Port)
		{
			*ppRet = p->pSettings;
			p->pSettings->AddRef();
			return S_OK;
		}
	}
	return S_FALSE;
}

STDMETHODIMP CEasySFTPHostSettingList::get__NewEnum(IUnknown** ppRet)
{
	if (!ppRet)
		return E_POINTER;
	*ppRet = new CEnum();
	return *ppRet ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CEasySFTPHostSettingList::CEnum::QueryInterface(REFIID riid, void** ppv)
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

STDMETHODIMP CEasySFTPHostSettingList::CEnum::Next(ULONG celt, VARIANT* rgelt, ULONG* pceltFetched)
{
	if (!rgelt)
		return E_POINTER;
	if (!celt)
		return S_OK;
	if (pceltFetched)
		*pceltFetched = 0;
	auto count = static_cast<ULONG>(theApp.m_aHosts.GetCount());
	while (m_uPos < count)
	{
		auto* p = theApp.m_aHosts.GetItem(static_cast<int>(m_uPos++));
		rgelt->vt = VT_DISPATCH;
		rgelt->pdispVal = p->pSettings;
		p->pSettings->AddRef();
		if (pceltFetched)
			(*pceltFetched)++;
		rgelt++;
		if (!--celt)
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CEasySFTPHostSettingList::CEnum::Skip(ULONG celt)
{
	if (!celt)
		return S_OK;
	auto count = static_cast<ULONG>(theApp.m_aHosts.GetCount());
	if (m_uPos + celt > count)
	{
		m_uPos = count;
		return S_FALSE;
	}
	m_uPos += celt;
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSettingList::CEnum::Reset()
{
	m_uPos = 0;
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSettingList::CEnum::Clone(IEnumVARIANT** ppEnum)
{
	if (!ppEnum)
		return E_POINTER;
	CEnum* pEnum = new CEnum();
	if (!pEnum)
		return E_OUTOFMEMORY;
	pEnum->m_uPos = m_uPos;
	*ppEnum = pEnum;
	return S_OK;
}
