// CFactory.cpp: CClassFactory クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ShellDLL.h"
#include "CFactory.h"

//////////////////////////////////////////////////////////////////////
// CClassFactory

CClassFactory::CClassFactory()
{
	m_uRef = 1;
	m_pFunc = NULL;
}

CClassFactory::~CClassFactory()
{

}

STDMETHODIMP_(ULONG) CClassFactory::AddRef()
{
	return ++m_uRef;
}

STDMETHODIMP_(ULONG) CClassFactory::Release()
{
	ULONG u = --m_uRef;
	if (u == 0)
		delete this;
	return u;
}

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, void** ppv)
{
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IClassFactory))
	{
		*ppv = (IClassFactory*) this;
		AddRef();
		return S_OK;
	}
	*ppv = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP CClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv)
{
	if (m_pFunc == NULL)
		return E_FAIL;
	return (*m_pFunc)(this, pUnkOuter, riid, ppv);
}

STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
	if (fLock)
		theApp.m_uCFLock++;
	else if (theApp.m_uCFLock > 0)
		theApp.m_uCFLock--;
	return S_OK;
}
