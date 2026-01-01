/*
 EasySFTP - Copyright (C) 2026 jet (ジェット)

 Logger.cpp - implementations of IEasySFTPLogger
 */

#include "stdafx.h"
#include "EasySFTP.h"
#include "Logger.h"

#include "MainWnd.h"

STDMETHODIMP CEasySFTPLogger::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	if (!IsEqualIID(riid, IID_IUnknown) && !IsEqualIID(riid, IID_IEasySFTPLogger))
		return E_NOINTERFACE;
	*ppv = static_cast<IEasySFTPLogger*>(this);
	AddRef();
	return S_OK;
}

STDMETHODIMP CEasySFTPLogger::Log(EasySFTPLogLevel Level, BSTR Message, long HResult)
{
#ifdef _DEBUG
	{
		CMyStringW str;
		::MyBSTRToString(Message, str);
		str.InsertString(L"[EasySFTP] ", 0);
		str += L'\n';
		OutputDebugString(str);
	}
#endif
	if (Level == EasySFTPLogLevel::Debug)
		return S_OK;
	auto* pWnd = static_cast<CMainWindow*>(theApp.m_pMainWnd);
	if (!pWnd)
		return S_OK;
	pWnd->SetStatusText(Message);
	return S_OK;
}
