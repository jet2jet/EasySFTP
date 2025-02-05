/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 EnmIDLst.h - declaration of CEnumFTPItemIDList
 */

#pragma once

class CEnumFTPItemIDList : public CUnknownImplT<IEnumIDList>
{
public:
	CEnumFTPItemIDList(CDelegateMallocData* pMallocData, const CMyPtrArrayT<CFTPFileItem>& arrItems, SHCONTF grfFlags, IUnknown* pUnkOuter);
	virtual ~CEnumFTPItemIDList();

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);

	STDMETHOD(Next)(ULONG celt, PITEMID_CHILD* rgelt, ULONG* pceltFetched);
	STDMETHOD(Skip)(ULONG celt);
	STDMETHOD(Reset)();
	STDMETHOD(Clone)(IEnumIDList** ppEnum);

private:
	CDelegateMallocData* m_pMallocData;
	CMyPtrArrayT<CFTPFileItem> m_arrItems;
	SHCONTF m_grfFlags;
	ULONG m_uPos;
	IUnknown* m_pUnkOuter;
};
