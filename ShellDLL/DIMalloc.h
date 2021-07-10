
#pragma once

class CDelegateItemIDMalloc : public IMalloc
{
public:
	CDelegateItemIDMalloc();
	~CDelegateItemIDMalloc();

	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	STDMETHOD_(void*, Alloc)(SIZE_T cb);
	STDMETHOD_(void*, Realloc)(void* pv, SIZE_T cb);
	STDMETHOD_(void, Free)(void* pv);
	STDMETHOD_(SIZE_T, GetSize)(void* pv);
	STDMETHOD_(int, DidAlloc)(void* pv);
	STDMETHOD_(void, HeapMinimize)(void);

private:
	ULONG m_cRef;
	IMalloc* m_pMalloc;
};
