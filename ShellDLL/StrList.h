/*
 EasySFTP - Copyright (C) 2025 jet (ジェット)

 StrList.h - declaration of CEasySFTPStringList
 */

#pragma once

#include "Dispatch.h"
#include "EasySFTP_h.h"
#include "Array.h"

class CEasySFTPStringList : public CDispatchImplT<IEasySFTPStringList>
{
public:
	CEasySFTPStringList(CMyStringArrayW* pstrArray, IUnknown* pRef);
	virtual ~CEasySFTPStringList();

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override;

	STDMETHOD(get_Item)(long Index, BSTR* pString) override;
	STDMETHOD(put_Item)(long Index, BSTR String) override;
	STDMETHOD(get_Count)(long* pRet) override;
	STDMETHOD(Add)(BSTR String, long* pRet) override;
	STDMETHOD(Remove)(long Index) override;
	STDMETHOD(FindIndex)(BSTR String, long* pRet) override;
	STDMETHOD(get__NewEnum)(IUnknown** ppRet) override;

private:
	class CEnum : public CUnknownImplT<IEnumVARIANT>
	{
	public:
		CEnum(CEasySFTPStringList* pList)
			: m_pList(pList)
			, m_uPos(0)
		{
			pList->AddRef();
		}
		virtual ~CEnum()
		{
			m_pList->Release();
		}

		STDMETHOD(QueryInterface)(REFIID riid, void** ppv);

		STDMETHOD(Next)(ULONG celt, VARIANT* rgelt, ULONG* pceltFetched) override;
		STDMETHOD(Skip)(ULONG celt) override;
		STDMETHOD(Reset)() override;
		STDMETHOD(Clone)(IEnumVARIANT** ppEnum) override;

	private:
		CEasySFTPStringList* m_pList;
		ULONG m_uPos;
	};

	CMyStringArrayW* m_pstrArray;
	IUnknown* m_pRef;
};
