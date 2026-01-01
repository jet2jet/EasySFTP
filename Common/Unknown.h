/*
 Copyright (C) 2011 jet (ジェット)

 Unknown.h - declarations and implementations of CUnknownImplT
 */

#pragma once

class DECLSPEC_NOVTABLE CUnknownImplBase
{
protected:
	CUnknownImplBase() { m_uRef = 1; }
	virtual ~CUnknownImplBase() { }

public:
	HRESULT InternalQueryInterface(REFIID riid, void FAR* FAR* ppv) { return E_NOINTERFACE; }
	ULONG InternalAddRef() { return (ULONG) InterlockedIncrement((LONG*) &m_uRef); }
	ULONG InternalRelease()
	{
		ULONG u = (ULONG) InterlockedDecrement((LONG*) &m_uRef);
		if (!u)
		{
			delete this;
			return 0;
		}
		return u;
	}

protected:
	ULONG m_uRef;
};

template <class T>
class DECLSPEC_NOVTABLE CUnknownImplT : public T, public CUnknownImplBase
{
public:
	CUnknownImplT() { }
	virtual ~CUnknownImplT() { }

	STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv) = 0;
	STDMETHOD_(ULONG, AddRef)() override
		{ return InternalAddRef(); }
	STDMETHOD_(ULONG, Release)() override
		{ return InternalRelease(); }
};

#define FORWARD_UNKNOWN_IMPL_BASE(base) \
	STDMETHOD_(ULONG, AddRef)() override { return base::AddRef(); } \
	STDMETHOD_(ULONG, Release)() override { return base::Release(); }
#define FORWARD_UNKNOWN_IMPL_T() FORWARD_UNKNOWN_IMPL_BASE(CUnknownImplT)

class CUnknownImpl : public virtual IUnknown, public CUnknownImplBase
{
public:
	STDMETHOD_(ULONG, AddRef)() override
		{ return InternalAddRef(); }
	STDMETHOD_(ULONG, Release)() override
		{ return InternalRelease(); }
};

#define FORWARD_UNKNOWN_IMPL() FORWARD_UNKNOWN_IMPL_BASE(CUnknownImpl)

class DECLSPEC_NOVTABLE CReferenceCountClassBase : public CUnknownImplBase
{
public:
	ULONG AddRef() { return InternalAddRef(); }
	ULONG Release() { return InternalRelease(); }
};

#define CReferenceCountClassBase_AddRef(p) (static_cast<CReferenceCountClassBase*>(p))->AddRef()
#define CReferenceCountClassBase_Release(p) (static_cast<CReferenceCountClassBase*>(p))->Release()
