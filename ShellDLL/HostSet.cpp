/*
 EasySFTP - Copyright (C) 2025 jet (ジェット)

 HostSet.cpp - implementation of CEasySFTPHostSetting
 */

#include "stdafx.h"
#include "ShellDLL.h"
#include "HostSet.h"

#include "Auth.h"
#include "StrList.h"
#include "RFolder.h"

#define CREDENTIAL_TYPE_INVALID         (static_cast<WORD>(0))
#define CREDENTIAL_TYPE_PASSWORD        (static_cast<WORD>(1))
#define CREDENTIAL_TYPE_PRIVATE_KEY     (static_cast<WORD>(2))

namespace
{

DWORD _GetCredPersistType(EasySFTPPassKeyStoreType type)
{
	return type == EasySFTPPassKeyStoreType::CurrentUser ? CRED_PERSIST_ENTERPRISE : CRED_PERSIST_LOCAL_MACHINE;
}

void _MakeCredTargetName(CMyStringW& rstrTarget, LPCWSTR lpszHostName)
{
	rstrTarget.Format(L"EasySFTP:cred_%s", lpszHostName);
}

void _FreeSecureMemory(void* buffer, size_t size)
{
	_SecureStringW::SecureEmptyBuffer(buffer, size);
	free(buffer);
}

HRESULT _WriteCredentialsRaw(LPCWSTR lpszHost, LPCWSTR lpszUserName, EasySFTPPassKeyStoreType type, const BYTE* pbBuffer, size_t bufferSize)
{
	CMyStringW strTarget;
	_MakeCredTargetName(strTarget, lpszHost);

	CREDENTIALW newCred{};
	newCred.Type = CRED_TYPE_GENERIC;
	newCred.CredentialBlob = const_cast<BYTE*>(pbBuffer);
	newCred.CredentialBlobSize = static_cast<DWORD>(bufferSize);
	newCred.TargetName = strTarget.GetBuffer();
	newCred.UserName = const_cast<LPWSTR>(lpszUserName);
	newCred.Persist = _GetCredPersistType(type);
	auto hr = ::CredWriteW(&newCred, 0) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	_SecureStringW::SecureEmptyBuffer(&newCred, sizeof(newCred));
	return hr;
}

HRESULT _WriteCredentialsPassword(LPCWSTR lpszHost, LPCWSTR lpszUserName, EasySFTPPassKeyStoreType type, BSTR Password)
{
	if (!lpszHost || !*lpszHost)
		return E_INVALIDARG;
	CMyStringW str;
	::MyBSTRToString(Password, str);

	size_t len = 0;
	auto* pUtf8 = str.AllocUTF8StringC(&len);
	if (!pUtf8)
	{
		_SecureStringW::SecureEmptyString(str);
		return E_OUTOFMEMORY;
	}
	auto* p = static_cast<BYTE*>(malloc(sizeof(WORD) + len));
	*reinterpret_cast<WORD*>(p) = CREDENTIAL_TYPE_PASSWORD;
	memcpy(p + sizeof(WORD), pUtf8, len);
	_FreeSecureMemory(pUtf8, len);
	_SecureStringW::SecureEmptyString(str);

	auto hr = _WriteCredentialsRaw(lpszHost, lpszUserName, type, p, sizeof(WORD) + len);
	_FreeSecureMemory(p, sizeof(WORD) + len);
	return hr;
}

HRESULT _WriteCredentialsPrivateKey(LPCWSTR lpszHost, LPCWSTR lpszUserName, EasySFTPPassKeyStoreType type, const CMyStringW& strPassword, const BYTE* pkeyFile, size_t len)
{
	if (!lpszHost || !*lpszHost || strPassword.GetLength() * sizeof(wchar_t) > LONG_MAX || len > LONG_MAX)
		return E_INVALIDARG;
	DWORD passLen = static_cast<DWORD>(sizeof(wchar_t) * strPassword.GetLength());
	size_t newLen = sizeof(WORD) + sizeof(DWORD) + passLen + sizeof(DWORD) + len;
	auto* pData = static_cast<BYTE*>(malloc(newLen));
	if (!pData)
	{
		return E_OUTOFMEMORY;
	}
	*reinterpret_cast<WORD UNALIGNED*>(pData) = CREDENTIAL_TYPE_PRIVATE_KEY;
	*reinterpret_cast<DWORD UNALIGNED*>(pData + sizeof(WORD)) = passLen;
	memcpy(pData + sizeof(WORD) + sizeof(DWORD), strPassword.operator LPCWSTR(), passLen);
	*reinterpret_cast<DWORD UNALIGNED*>(pData + sizeof(WORD) + sizeof(DWORD) + passLen) = static_cast<DWORD>(len);
	memcpy(pData + sizeof(WORD) + sizeof(DWORD) + passLen + sizeof(DWORD), pkeyFile, len);

	auto hr = _WriteCredentialsRaw(lpszHost, lpszUserName, type, pData, newLen);
	_FreeSecureMemory(pData, newLen);
	return hr;
}

HRESULT _CopyCredentials(LPCWSTR lpszHostFrom, LPCWSTR lpszHostTo, LPCWSTR lpszUserName)
{
	if (!lpszHostFrom || !*lpszHostFrom || !lpszHostTo || !*lpszHostTo || !lpszUserName)
	{
		return E_INVALIDARG;
	}

	CMyStringW strTargetFrom;
	_MakeCredTargetName(strTargetFrom, lpszHostFrom);

	PCREDENTIALW pcred;
	if (!::CredReadW(strTargetFrom, CRED_TYPE_GENERIC, 0, &pcred))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
	CMyStringW strTargetTo;
	_MakeCredTargetName(strTargetTo, lpszHostTo);

	CREDENTIALW newCred{};
	newCred.Type = pcred->Type;
	newCred.CredentialBlob = pcred->CredentialBlob;
	newCred.CredentialBlobSize = pcred->CredentialBlobSize;
	newCred.TargetName = strTargetTo.GetBuffer();
	newCred.UserName = const_cast<LPWSTR>(lpszUserName);
	newCred.Persist = pcred->Persist;
	auto hr = ::CredWriteW(&newCred, 0) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	::CredFree(pcred);
	_SecureStringW::SecureEmptyBuffer(&newCred, sizeof(newCred));
	return hr;
}

HRESULT _MoveCredentials(LPCWSTR lpszHostFrom, LPCWSTR lpszHostTo, LPCWSTR lpszUserName)
{
	auto hr = _CopyCredentials(lpszHostFrom, lpszHostTo, lpszUserName);
	if (FAILED(hr))
	{
		return hr;
	}

	CMyStringW strTargetFrom;
	_MakeCredTargetName(strTargetFrom, lpszHostFrom);
	return ::CredDeleteW(strTargetFrom, CRED_TYPE_GENERIC, 0) != 0 ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

HRESULT _DeleteCredentials(LPCWSTR lpszHost)
{
	if (!lpszHost || !*lpszHost)
		return S_OK;
	CMyStringW strTarget;
	_MakeCredTargetName(strTarget, lpszHost);
	return ::CredDeleteW(strTarget, CRED_TYPE_GENERIC, 0) != 0 ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

HRESULT _RenameCredentialsUserName(LPCWSTR lpszHost, LPCWSTR lpszUserName)
{
	if (!lpszHost || !*lpszHost || !lpszUserName)
	{
		return E_INVALIDARG;
	}

	CMyStringW strTargetFrom;
	_MakeCredTargetName(strTargetFrom, lpszHost);

	PCREDENTIALW pcred;
	if (!::CredReadW(strTargetFrom, CRED_TYPE_GENERIC, 0, &pcred))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
	pcred->UserName = const_cast<LPWSTR>(lpszUserName);

	auto hr = ::CredWriteW(pcred, 0) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	::CredFree(pcred);
	return hr;
}

HRESULT _ChangeCredentialsPersistType(LPCWSTR lpszHost, EasySFTPPassKeyStoreType type)
{
	if (!lpszHost || !*lpszHost)
	{
		return E_INVALIDARG;
	}

	CMyStringW strTargetFrom;
	_MakeCredTargetName(strTargetFrom, lpszHost);

	PCREDENTIALW pcred;
	if (!::CredReadW(strTargetFrom, CRED_TYPE_GENERIC, 0, &pcred))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
	pcred->Persist = _GetCredPersistType(type);

	auto hr = ::CredWriteW(pcred, 0) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	::CredFree(pcred);
	return hr;
}

PCREDENTIALW _GetCredientials(LPCWSTR lpszHost)
{
	if (!lpszHost || !*lpszHost)
	{
		return nullptr;
	}

	CMyStringW strTargetFrom;
	_MakeCredTargetName(strTargetFrom, lpszHost);

	PCREDENTIALW pcred;
	if (!::CredReadW(strTargetFrom, CRED_TYPE_GENERIC, 0, &pcred))
	{
		return nullptr;
	}

	auto result = CREDENTIAL_TYPE_INVALID;
	if (pcred->CredentialBlobSize >= sizeof(WORD))
	{
		auto* p = reinterpret_cast<WORD*>(pcred->CredentialBlob);
		result = *p;
	}
	if (result == CREDENTIAL_TYPE_INVALID)
	{
		::CredFree(pcred);
		return nullptr;
	}
	return pcred;
}

WORD _GetCredientialsType(LPCWSTR lpszHost)
{
	if (!lpszHost || !*lpszHost)
	{
		return CREDENTIAL_TYPE_INVALID;
	}

	CMyStringW strTargetFrom;
	_MakeCredTargetName(strTargetFrom, lpszHost);

	PCREDENTIALW pcred;
	if (!::CredReadW(strTargetFrom, CRED_TYPE_GENERIC, 0, &pcred))
	{
		return CREDENTIAL_TYPE_INVALID;
	}

	auto result = CREDENTIAL_TYPE_INVALID;
	if (pcred->CredentialBlobSize >= sizeof(WORD))
	{
		auto* p = reinterpret_cast<WORD*>(pcred->CredentialBlob);
		result = *p;
	}

	::CredFree(pcred);
	return result;
}

}

////////////////////////////////////////////////////////////////////////////////

CEasySFTPHostSetting::CEasySFTPHostSetting()
	: CDispatchImplT(theApp.GetTypeInfo(IID_IEasySFTPHostSetting2))
	, ConnectionMode(EasySFTPConnectionMode::FTP)
	, nPort(21)
	, bTextMode(0)
	, nServerCharset(scsUTF8)
	, nTransferMode(static_cast<char>(EasySFTPTransferMode::Auto))
	, bUseSystemTextFileType()
	, bAdjustRecvModifyTime()
	, bAdjustSendModifyTime()
	, bUseThumbnailPreview()
	, bAutoLogin()
	, AuthMode(EasySFTPAuthenticationMode::Password)
	, PassKeyStoreType(EasySFTPPassKeyStoreType::Local)
{
}

CEasySFTPHostSetting::CEasySFTPHostSetting(const CEasySFTPHostSetting* pSettings)
	: CDispatchImplT(theApp.GetTypeInfo(IID_IEasySFTPHostSetting2))
	, ConnectionMode(EasySFTPConnectionMode::FTP)
	, nPort(21)
	, bTextMode(0)
	, nServerCharset(scsUTF8)
	, nTransferMode(static_cast<char>(EasySFTPTransferMode::Auto))
	, bUseSystemTextFileType()
	, bAdjustRecvModifyTime()
	, bAdjustSendModifyTime()
	, bUseThumbnailPreview()
	, bAutoLogin()
	, AuthMode(EasySFTPAuthenticationMode::Password)
	, PassKeyStoreType(EasySFTPPassKeyStoreType::Local)
{
	Copy(pSettings);
}

void CEasySFTPHostSetting::Copy(const CEasySFTPHostSetting* pSettings)
{
	ConnectionMode = pSettings->ConnectionMode;
	strDisplayName = pSettings->strDisplayName;
	SetHostName(pSettings->strHostName);
	SetUserName(pSettings->strUserName);
	nPort = pSettings->nPort;
	//strUserName = pSettings->strUserName;
	strInitLocalPath = pSettings->strInitLocalPath;
	strInitServerPath = pSettings->strInitServerPath;
	bTextMode = pSettings->bTextMode;
	nServerCharset = pSettings->nServerCharset;
	nTransferMode = pSettings->nTransferMode;
	arrTextFileType.CopyArray(pSettings->arrTextFileType);
	bUseSystemTextFileType = pSettings->bUseSystemTextFileType;
	bAdjustRecvModifyTime = pSettings->bAdjustRecvModifyTime;
	bAdjustSendModifyTime = pSettings->bAdjustSendModifyTime;
	strChmodCommand = pSettings->strChmodCommand;
	//strTouchCommand = pSettings->strTouchCommand;
	bAutoLogin = pSettings->bAutoLogin;
	AuthMode = pSettings->AuthMode;
	PassKeyStoreType = pSettings->PassKeyStoreType;
}

STDMETHODIMP CEasySFTPHostSetting::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	*ppv = NULL;
	if (!IsEqualIID(riid, IID_IUnknown) && !IsEqualIID(riid, IID_IDispatch) &&
		!IsEqualIID(riid, IID_IEasySFTPHostSetting) &&
		!IsEqualIID(riid, IID_IEasySFTPHostSetting2))
		return E_NOINTERFACE;
	*ppv = static_cast<IEasySFTPHostSetting2*>(this);
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
	CMyStringW str;
	MyBSTRToString(Value, str);
	SetHostName(str);
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
		{
			CMyStringW str;
			MyBSTRToString(bstr, str);
			::SysFreeString(bstr);
			SetHostName(str);
		}
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

void CEasySFTPHostSetting::SetHostName(const CMyStringW& strHostName)
{
	if (this->strHostName.Compare(strHostName) == 0)
		return;
	if (strHostName.IsEmpty())
		_DeleteCredentials(this->strHostName);
	else
		_MoveCredentials(this->strHostName, strHostName, strUserName);
	this->strHostName = strHostName;
}

void CEasySFTPHostSetting::SetUserName(const CMyStringW& strUserName)
{
	if (!strHostName.IsEmpty())
		_RenameCredentialsUserName(strHostName, strUserName);
	this->strUserName = strUserName;
}

STDMETHODIMP CEasySFTPHostSetting::get_AutoLogin(VARIANT_BOOL* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = bAutoLogin ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::put_AutoLogin(VARIANT_BOOL Value)
{
	bAutoLogin = (Value != VARIANT_FALSE);
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::get_UserName(BSTR* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = MyStringToBSTR(strUserName);
	return *pRet ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CEasySFTPHostSetting::put_UserName(BSTR Value)
{
	CMyStringW str;
	MyBSTRToString(Value, str);
	SetUserName(str);
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::ClearCredentials()
{
	auto hr = _DeleteCredentials(strHostName);
	if (SUCCEEDED(hr))
	{
		strUserName.Empty();
		bAutoLogin = false;
		AuthMode = EasySFTPAuthenticationMode::Password;
	}
	return hr;
}

STDMETHODIMP CEasySFTPHostSetting::get_HasPassword(VARIANT_BOOL* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = _GetCredientialsType(strHostName) == CREDENTIAL_TYPE_PASSWORD ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::SetPassword(BSTR Password)
{
	if (!Password)
		return E_POINTER;
	if (strHostName.IsEmpty())
		return E_ACCESSDENIED;
	return _WriteCredentialsPassword(strHostName, strUserName, PassKeyStoreType, Password);
}

STDMETHODIMP CEasySFTPHostSetting::get_AuthenticationMode(EasySFTPAuthenticationMode* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = AuthMode;
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::put_AuthenticationMode(EasySFTPAuthenticationMode Value)
{
	AuthMode = Value;
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::get_HasPrivateKey(VARIANT_BOOL* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = _GetCredientialsType(strHostName) == CREDENTIAL_TYPE_PRIVATE_KEY ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::StorePrivateKeyFromFile(BSTR FileName, BSTR Password)
{
	if (!Password)
		return E_POINTER;
	if (strHostName.IsEmpty())
		return E_ACCESSDENIED;
	CMyStringW strPKey;
	MyBSTRToString(FileName, strPKey);
	auto hFile = MyCreateFileW(strPKey, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		LogWin32LastError(L"CreateFile");
		return HRESULT_FROM_WIN32(GetLastError());
	}
	LARGE_INTEGER liSize{};
	if (!GetFileSizeEx(hFile, &liSize))
	{
		LogWin32LastError(L"GetFileSizeEx");
		auto hr = HRESULT_FROM_WIN32(GetLastError());
		CloseHandle(hFile);
		return hr;
	}
	if (liSize.QuadPart == 0 || liSize.QuadPart > INT_MAX)
	{
		CloseHandle(hFile);
		return E_INVALIDARG;
	}
	auto hMapping = CreateFileMapping(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
	if (!hMapping)
	{
		LogWin32LastError(L"CreateFileMapping");
		auto hr = HRESULT_FROM_WIN32(GetLastError());
		CloseHandle(hFile);
		return hr;
	}
	auto pv = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
	if (!pv)
	{
		LogWin32LastError(L"MapViewOfFile");
		auto hr = HRESULT_FROM_WIN32(GetLastError());
		CloseHandle(hMapping);
		CloseHandle(hFile);
		return hr;
	}

	auto hr = StorePrivateKeyFromBinary(pv, static_cast<long>(liSize.LowPart), Password);

	UnmapViewOfFile(pv);
	CloseHandle(hMapping);
	CloseHandle(hFile);

	return hr;
}

STDMETHODIMP CEasySFTPHostSetting::StorePrivateKeyFromBinary(const void* buffer, long length, BSTR Password)
{
	if (!Password)
		return E_POINTER;
	if (length <= 0)
		return E_INVALIDARG;
	if (strHostName.IsEmpty())
		return E_ACCESSDENIED;

	auto* bio = BIO_new_mem_buf(buffer, static_cast<int>(length));

	CMyStringW str;
	MyBSTRToString(Password, str);
	auto* pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, str.IsEmpty() ? "" : str.operator LPSTR());
	BIO_free(bio);

	if (pkey == nullptr)
	{
		_SecureStringW::SecureEmptyString(str);
		auto err = ERR_peek_last_error();
		if (ERR_GET_LIB(err) != ERR_LIB_PEM)
		{
			return E_FAIL;
		}
		switch (ERR_GET_REASON(err))
		{
			case PEM_R_BAD_PASSWORD_READ:
				return ERROR_INVALID_PASSWORD;
			default:
				return E_INVALIDARG;
		}
	}
	EVP_PKEY_free(pkey);

	auto hr = _WriteCredentialsPrivateKey(strHostName, strUserName, PassKeyStoreType,
		str, static_cast<const BYTE*>(buffer), static_cast<size_t>(length));
	_SecureStringW::SecureEmptyString(str);

	return hr;
}

STDMETHODIMP CEasySFTPHostSetting::get_PassKeyStoreType(EasySFTPPassKeyStoreType* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = PassKeyStoreType;
	return S_OK;
}

STDMETHODIMP CEasySFTPHostSetting::put_PassKeyStoreType(EasySFTPPassKeyStoreType Value)
{
	if (_GetCredientialsType(strHostName) != CREDENTIAL_TYPE_INVALID)
	{
		auto hr = _ChangeCredentialsPersistType(strHostName, Value);
		if (FAILED(hr))
			return hr;
	}
	PassKeyStoreType = Value;
	return S_OK;
}

CAuthentication* CEasySFTPHostSetting::CreateAuthentication() const
{
	if (!bAutoLogin)
		return nullptr;

	auto* pAuth = new CAuthentication();
	pAuth->m_Mode = AuthMode;
	pAuth->m_strUserName = strUserName;

	if (AuthMode == EasySFTPAuthenticationMode::Password || AuthMode == EasySFTPAuthenticationMode::PrivateKey)
	{
		auto* pcred = _GetCredientials(strHostName);
		if (!pcred)
		{
			delete pAuth;
			return nullptr;
		}

		BYTE* pData = pcred->CredentialBlob;
		size_t nLen = static_cast<size_t>(pcred->CredentialBlobSize);
		if (*reinterpret_cast<WORD UNALIGNED*>(pData) == CREDENTIAL_TYPE_PASSWORD)
		{
			CMyStringW strTemp;
			strTemp.SetUTF8String(pData + sizeof(WORD), nLen - sizeof(WORD));
			pAuth->m_strPassword = strTemp;
			_SecureStringW::SecureEmptyString(strTemp);
		}
		else if (*reinterpret_cast<WORD UNALIGNED*>(pData) == CREDENTIAL_TYPE_PRIVATE_KEY)
		{
			CMyStringW strPassword;
			auto nPasswordLen = *reinterpret_cast<DWORD UNALIGNED*>(pData + sizeof(WORD));
			strPassword.SetString(
				reinterpret_cast<wchar_t UNALIGNED*>(pData + sizeof(WORD) + sizeof(DWORD)),
				nPasswordLen / sizeof(wchar_t));
			pAuth->m_strPassword = strPassword;
			_SecureStringW::SecureEmptyString(strPassword);
			auto nPKeyLen = *reinterpret_cast<DWORD UNALIGNED*>(pData + sizeof(WORD) + sizeof(DWORD) + nPasswordLen);
			pAuth->SetPrivateKeyBinary(pData + sizeof(WORD) + sizeof(DWORD) + nPasswordLen + sizeof(DWORD), static_cast<long>(nPKeyLen));
		}
		else
		{
			delete pAuth;
			return nullptr;
		}
	}
	return pAuth;
}

bool CEasySFTPHostSetting::HasCredentials()
{
	CMyStringW str;
	_MakeCredTargetName(str, L"*");
	DWORD dw = 0;
	PCREDENTIALW* ppcred;
	if (!::CredEnumerateW(str, 0, &dw, &ppcred))
		return false;

	::CredFree(ppcred);
	return dw > 0;
}

void CEasySFTPHostSetting::ClearAllCredentials()
{
	CMyStringW str;
	_MakeCredTargetName(str, L"*");
	DWORD dw = 0;
	PCREDENTIALW* ppcred;
	if (!::CredEnumerateW(str, 0, &dw, &ppcred))
		return;

	for (DWORD i = 0; i < dw; ++i)
	{
		::CredDeleteW(ppcred[i]->TargetName, CRED_TYPE_GENERIC, 0);
	}
	::CredFree(ppcred);
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
