/*
 Copyright (C) 2025 Kuri-Applications

 Dispatch.h - declarations and implementations of CDispatchImplT
 */

#pragma once

#include "Unknown.h"

class CDispatchImplBase
{
public:
	CDispatchImplBase(ITypeInfo* pInfo) : m_pInfo(pInfo) { }
	virtual ~CDispatchImplBase() { m_pInfo->Release(); }

	virtual void* GetThisForDispatch() = 0;

	HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT* pctinfo)
	{
		if (!pctinfo)
			return E_POINTER;
		*pctinfo = 1;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo)
	{
		if (!ppTInfo)
			return E_POINTER;
		if (iTInfo != 0)
			return E_INVALIDARG;
		*ppTInfo = m_pInfo;
		m_pInfo->AddRef();
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId)
	{
		return ::DispGetIDsOfNames(m_pInfo, rgszNames, cNames, rgDispId);
	}

	HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams,
		VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
	{
		return ::DispInvoke(GetThisForDispatch(), m_pInfo, dispIdMember, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
	}

private:
	ITypeInfo* m_pInfo;
};

#define FORWARD_DISPATCH_IMPL_BASE_NO_UNKNOWN(base) \
	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) override{return base::GetTypeInfoCount(pctinfo);} \
	STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override{return base::GetTypeInfo(iTInfo, lcid, ppTInfo);} \
	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) override{return base::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);} \
	STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) override{return base::Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);}
#define FORWARD_DISPATCH_IMPL_BASE(base) \
	FORWARD_UNKNOWN_IMPL_BASE(base) \
	FORWARD_DISPATCH_IMPL_BASE_NO_UNKNOWN(base)


template <class T>
class DECLSPEC_NOVTABLE CDispatchImplNoUnknownT : public T, public CDispatchImplBase
{
public:
	CDispatchImplNoUnknownT(ITypeInfo* pInfo) : CDispatchImplBase(pInfo) { }
	virtual ~CDispatchImplNoUnknownT() { }

	virtual void* GetThisForDispatch()
	{
		return static_cast<T*>(this);
	}

	FORWARD_DISPATCH_IMPL_BASE_NO_UNKNOWN(CDispatchImplBase)
};

template <class T>
class DECLSPEC_NOVTABLE CDispatchImplT : public CUnknownImplT<T>, public CDispatchImplBase
{
public:
	CDispatchImplT(ITypeInfo* pInfo) : CDispatchImplBase(pInfo) { }
	virtual ~CDispatchImplT() { }

	virtual void* GetThisForDispatch()
	{
		return static_cast<T*>(this);
	}

	FORWARD_UNKNOWN_IMPL_T()
	FORWARD_DISPATCH_IMPL_BASE_NO_UNKNOWN(CDispatchImplBase)
};

#define FORWARD_DISPATCH_IMPL_T_NO_UNKNOWN() FORWARD_DISPATCH_IMPL_BASE_NO_UNKNOWN(CDispatchImplT)
#define FORWARD_DISPATCH_IMPL_T() FORWARD_DISPATCH_IMPL_BASE(CDispatchImplT)

class CDispatchImpl : public IDispatch, public CUnknownImpl, public CDispatchImplBase
{
public:
	CDispatchImpl(ITypeInfo* pInfo) : CDispatchImplBase(pInfo) { }
	virtual ~CDispatchImpl() { }

	FORWARD_UNKNOWN_IMPL()

	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) override
	{
		return CDispatchImplBase::GetTypeInfoCount(pctinfo);
	}

	STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override
	{
		return CDispatchImplBase::GetTypeInfo(iTInfo, lcid, ppTInfo);
	}

	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) override
	{
		return CDispatchImplBase::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);
	}

	STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams,
		VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) override
	{
		return CDispatchImplBase::Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
	}
};

#define FORWARD_DISPATCH_IMPL_NO_UNKNOWN() FORWARD_DISPATCH_IMPL_BASE_NO_UNKNOWN(CDispatchImpl)
#define FORWARD_DISPATCH_IMPL() FORWARD_DISPATCH_IMPL_BASE(CDispatchImpl)
