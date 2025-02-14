/*
 EasySFTP - Copyright (C) 2025 Kuri-Applications

 HostSet.h - declaration of CEasySFTPHostSetting
 */

#pragma once

#include "Dispatch.h"
#include "EasySFTP_h.h"
#include "Array.h"

class CEasySFTPFolderRoot;

 ///////////////////////////////////////////////////////////////////////////////

class CEasySFTPHostSetting : public CDispatchImplT<IEasySFTPHostSetting>
{
public:
	EasySFTPConnectionMode ConnectionMode;
	CMyStringW strDisplayName;
	CMyStringW strHostName;
	int nPort;
	//CMyStringW strUserName;
	CMyStringW strInitLocalPath;
	CMyStringW strInitServerPath;
	BYTE bTextMode;
	char nServerCharset;
	char nTransferMode;
	CMyStringArrayW arrTextFileType;
	bool bUseSystemTextFileType;
	bool bAdjustRecvModifyTime;
	bool bAdjustSendModifyTime;
	bool bUseThumbnailPreview;
	CMyStringW strChmodCommand;
	//CMyStringW strTouchCommand;

	CEasySFTPHostSetting();
	CEasySFTPHostSetting(const CEasySFTPHostSetting* pSettings);
	void Copy(const CEasySFTPHostSetting* pSettings)
	{
		ConnectionMode = pSettings->ConnectionMode;
		strDisplayName = pSettings->strDisplayName;
		strHostName = pSettings->strHostName;
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
	}

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override;

	STDMETHOD(get_Name)(BSTR* pRet) override;
	STDMETHOD(put_Name)(BSTR Value) override;
	STDMETHOD(get_HostName)(BSTR* pRet) override;
	STDMETHOD(put_HostName)(BSTR Value) override;
	STDMETHOD(get_ConnectionMode)(EasySFTPConnectionMode* pRet) override;
	STDMETHOD(put_ConnectionMode)(EasySFTPConnectionMode Value) override;
	STDMETHOD(get_Port)(long* pRet) override;
	STDMETHOD(put_Port)(long Value) override;
	STDMETHOD(get_InitLocalPath)(BSTR* pRet) override;
	STDMETHOD(put_InitLocalPath)(BSTR Value) override;
	STDMETHOD(get_InitServerPath)(BSTR* pRet) override;
	STDMETHOD(put_InitServerPath)(BSTR Value) override;
	STDMETHOD(get_TextMode)(EasySFTPTextMode* pRet) override;
	STDMETHOD(put_TextMode)(EasySFTPTextMode Value) override;
	STDMETHOD(get_ServerCharset)(EasySFTPServerCharset* pRet) override;
	STDMETHOD(put_ServerCharset)(EasySFTPServerCharset Value) override;
	STDMETHOD(get_TransferMode)(EasySFTPTransferMode* pRet) override;
	STDMETHOD(put_TransferMode)(EasySFTPTransferMode Value) override;
	STDMETHOD(get_TextFileType)(IEasySFTPStringList** ppList) override;
	STDMETHOD(get_UseSystemTextFileType)(VARIANT_BOOL* pRet) override;
	STDMETHOD(put_UseSystemTextFileType)(VARIANT_BOOL Value) override;
	STDMETHOD(get_AdjustRecvModifyTime)(VARIANT_BOOL* pRet) override;
	STDMETHOD(put_AdjustRecvModifyTime)(VARIANT_BOOL Value) override;
	STDMETHOD(get_AdjustSendModifyTime)(VARIANT_BOOL* pRet) override;
	STDMETHOD(put_AdjustSendModifyTime)(VARIANT_BOOL Value) override;
	STDMETHOD(get_UseThumbnailPreview)(VARIANT_BOOL* pRet) override;
	STDMETHOD(put_UseThumbnailPreview)(VARIANT_BOOL Value) override;
	STDMETHOD(get_ChmodCommand)(BSTR* pRet) override;
	STDMETHOD(put_ChmodCommand)(BSTR Value) override;
	STDMETHOD(CopyFrom)(IEasySFTPHostSetting* pSetting) override;
};

///////////////////////////////////////////////////////////////////////////////

class CEasySFTPHostSettingList : public CDispatchImplT<IEasySFTPHostSettingList>
{
public:
	CEasySFTPHostSettingList(CEasySFTPFolderRoot* pParent);
	virtual ~CEasySFTPHostSettingList();

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override;

	STDMETHOD(get_Item)(long Index, IEasySFTPHostSetting** ppSetting) override;
	STDMETHOD(get_Count)(long* pRet) override;
	STDMETHOD(Add)(IEasySFTPHostSetting* pSetting, long* pRet) override;
	STDMETHOD(Remove)(long Index) override;
	STDMETHOD(FindHost)(BSTR HostName, EasySFTPConnectionMode Mode, long Port, IEasySFTPHostSetting** ppRet) override;
	STDMETHOD(get__NewEnum)(IUnknown** ppRet) override;

private:
	class CEnum : public CUnknownImplT<IEnumVARIANT>
	{
	public:
		CEnum()
			: m_uPos(0)
		{
		}
		virtual ~CEnum()
		{
		}

		STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override;

		STDMETHOD(Next)(ULONG celt, VARIANT* rgelt, ULONG* pceltFetched) override;
		STDMETHOD(Skip)(ULONG celt) override;
		STDMETHOD(Reset)() override;
		STDMETHOD(Clone)(IEnumVARIANT** ppEnum) override;

	private:
		ULONG m_uPos;
	};

	CEasySFTPFolderRoot* m_pParent;
};
