/*
 Copyright (C) 2011 Kuri-Applications

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
class CUnknownImplT : public T, public CUnknownImplBase
{
public:
	CUnknownImplT() { }
	virtual ~CUnknownImplT() { }

	STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv)
		{ return InternalQueryInterface(riid, ppv); }
	STDMETHOD_(ULONG, AddRef)()
		{ return InternalAddRef(); }
	STDMETHOD_(ULONG, Release)()
		{ return InternalRelease(); }
};

typedef CUnknownImplT<IUnknown> CUnknownImpl;

class DECLSPEC_NOVTABLE CReferenceCountClassBase : public CUnknownImplBase
{
public:
	ULONG AddRef() { return InternalAddRef(); }
	ULONG Release() { return InternalRelease(); }
};

#define CReferenceCountClassBase_AddRef(p) (static_cast<CReferenceCountClassBase*>(p))->AddRef()
#define CReferenceCountClassBase_Release(p) (static_cast<CReferenceCountClassBase*>(p))->Release()
