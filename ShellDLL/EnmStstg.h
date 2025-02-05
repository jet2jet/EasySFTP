/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 EnmStstg.h - declaration of CEnumFTPItemStatstg
 */

#pragma once

#include "FileList.h"

class CEnumFTPItemStatstg : public CUnknownImplT<IEnumSTATSTG>
{
public:
	CEnumFTPItemStatstg(CDelegateMallocData* pMallocData, const CMyPtrArrayT<CFTPFileItem>& arrItems, bool bIsLockSupported, IUnknown* pUnkOuter);
	virtual ~CEnumFTPItemStatstg();

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);

	STDMETHOD(Next)(ULONG celt, STATSTG* rgelt, ULONG* pceltFetched);
	STDMETHOD(Skip)(ULONG celt);
	STDMETHOD(Reset)();
	STDMETHOD(Clone)(IEnumSTATSTG** ppEnum);

private:
	CDelegateMallocData* m_pMallocData;
	CMyPtrArrayT<CFTPFileItem> m_arrItems;
	ULONG m_uPos;
	bool m_bIsLockSupported;
	IUnknown* m_pUnkOuter;
};
