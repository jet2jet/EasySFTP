// CFactory.h: CClassFactory クラスのインターフェイス

#pragma once

class CClassFactory;

typedef HRESULT (STDMETHODCALLTYPE* CLASS_CREATE_FUNC)(CClassFactory*, IUnknown*, REFIID, void**);

struct _CLASS_ENTRY
{
	const CLSID* pclsid;
	CLASS_CREATE_FUNC pFunc;
};

#define BEGIN_CLASS_ENTRY(data_name) \
	static const _CLASS_ENTRY data_name[] = {
#define CLASS_ENTRY(clsid, class) \
	{ &clsid, (CLASS_CREATE_FUNC) class::FactoryCreate },
#define END_CLASS_ENTRY() \
	{ &CLSID_NULL, NULL } };

class CClassFactory : public IClassFactory  
{
public:
	CClassFactory();
	virtual ~CClassFactory();

public:
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);

	STDMETHOD(CreateInstance)(IUnknown* pUnkOuter, REFIID riid, void** ppv);
	STDMETHOD(LockServer)(BOOL fLock);

public:
	CLASS_CREATE_FUNC m_pFunc;

protected:
	ULONG m_uRef;
};
