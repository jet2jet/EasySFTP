
#include "StdAfx.h"
#include "DIMalloc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/****************************************************\
    Constructor
\****************************************************/
CDelegateItemIDMalloc::CDelegateItemIDMalloc() : m_cRef(1)
{
	::CoGetMalloc(1, &m_pMalloc);
}

/****************************************************\
    Destructor
\****************************************************/
CDelegateItemIDMalloc::~CDelegateItemIDMalloc()
{
	m_pMalloc->Release();
}
 
STDMETHODIMP_(ULONG) CDelegateItemIDMalloc::AddRef()
{
	return ++m_cRef;
}

STDMETHODIMP_(ULONG) CDelegateItemIDMalloc::Release()
{
	register ULONG u = --m_cRef;
	if (!u)
		delete this;
	return u;
}

STDMETHODIMP CDelegateItemIDMalloc::QueryInterface(REFIID riid, void** ppvObj)
{
	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IMalloc))
	{
		*ppvObj = (IMalloc*) this;
		AddRef();
		return S_OK;
	}

	*ppvObj = NULL;
	return E_NOINTERFACE;
}

/*****************************************************************************
 *	IMalloc::Alloc
 *****************************************************************************/

STDMETHODIMP_(LPVOID) CDelegateItemIDMalloc::Alloc(SIZE_T cb)
{
	WORD cbActualSize = (WORD) (sizeof(DELEGATEITEMID) - 1 + cb);
	PDELEGATEITEMID pidl = (PDELEGATEITEMID) m_pMalloc->Alloc(cbActualSize + sizeof(WORD));

	if (pidl)
	{
		pidl->cbSize = cbActualSize;
		pidl->wOuter = 0x6646;          // "Ff"
		pidl->cbInner = (WORD) cb;
		*(WORD *)&(((BYTE*) pidl)[cbActualSize]) = 0;
	}

	return pidl;
}

/*****************************************************************************
 *	IMalloc::Realloc
 *****************************************************************************/

STDMETHODIMP_(LPVOID) CDelegateItemIDMalloc::Realloc(void* pv, SIZE_T cb)
{
	return m_pMalloc->Realloc(pv, cb);
}

/*****************************************************************************
 *	IMalloc::Free
 *****************************************************************************/

STDMETHODIMP_(void) CDelegateItemIDMalloc::Free(void* pv)
{
	m_pMalloc->Free(pv);
}

/*****************************************************************************
 *	IMalloc::GetSize
 *****************************************************************************/

STDMETHODIMP_(SIZE_T) CDelegateItemIDMalloc::GetSize(void* pv)
{
	return m_pMalloc->GetSize(pv);
}

/*****************************************************************************
 *	IMalloc::DidAlloc
 *****************************************************************************/

STDMETHODIMP_(int) CDelegateItemIDMalloc::DidAlloc(void* pv)
{
    return m_pMalloc->DidAlloc(pv);
}

/*****************************************************************************
 *	IMalloc::HeapMinimize
 *****************************************************************************/

STDMETHODIMP_(void) CDelegateItemIDMalloc::HeapMinimize(void) 
{
	m_pMalloc->HeapMinimize();
}
