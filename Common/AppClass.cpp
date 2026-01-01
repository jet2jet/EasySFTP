/*
 Copyright (C) 2011 jet (ジェット)

 AppClass.cpp - implementations of CMyApplication and WinMain
 */

#include "StdAfx.h"
#include "AppClass.h"

#include "MyWindow.h"

#include <process.h>

static struct CCurrentAppData
{
	CMyApplication* pMainApp;
	CMyDLLApplication* pDLLApp;
} s_app;

extern "C" void WINAPI EndWindowData();

EXTERN_C int APIENTRY MyWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine, int nCmdShow)
{
	if (!s_app.pMainApp)
		return -1;
	s_app.pMainApp->m_dwThreadID = ::GetCurrentThreadId();
	s_app.pMainApp->m_hThread = ::GetCurrentThread();
	s_app.pMainApp->m_hInstance = hInstance;
	s_app.pMainApp->m_lpCmdLine = lpCmdLine;
	s_app.pMainApp->m_nCmdShow = nCmdShow;
	if (!s_app.pMainApp->InitInstance())
		return s_app.pMainApp->ExitInstance();
	int ret = s_app.pMainApp->Run();
	::EndWindowData();
	return ret;
}

EXTERN_C BOOL APIENTRY MyDllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			if (!s_app.pDLLApp)
				return FALSE;
			s_app.pDLLApp->m_hInstance = hInstance;
			if (!s_app.pDLLApp->InitInstance())
			{
				s_app.pDLLApp->ExitInstance();
				return FALSE;
			}
		}
		break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			s_app.pDLLApp->ExitInstance();
			::EndWindowData();
			break;
	}
	return TRUE;
}

CMyThread* WINAPI GetCurThread()
{
	return s_app.pMainApp ? (CMyThread*) s_app.pMainApp : (CMyThread*) s_app.pDLLApp;
}

CMyApplication* WINAPI GetCurApp()
{
	return s_app.pMainApp;
}

CMyDLLApplication* WINAPI GetCurDLLApp()
{
	return s_app.pDLLApp;
}

EXTERN_C HINSTANCE WINAPI MyGetCurrentInstance()
{
	if (s_app.pMainApp)
		return s_app.pMainApp->m_hInstance;
	else if (s_app.pDLLApp)
		return s_app.pDLLApp->m_hInstance;
	else
		return NULL;
}

////////////////////////////////////////////////////////////////////////////////

CMyThread::CMyThread()
{
	m_bWinThread = false;
	CommonConstruct();
}

CMyThread::CMyThread(bool bWinThread)
{
	m_bWinThread = bWinThread;
	CommonConstruct();
}

void CMyThread::CommonConstruct()
{
	m_dwThreadID = 0;
	m_hThread = NULL;
	::InitializeCriticalSection(&m_csThread);
}

CMyThread::~CMyThread()
{
	EndThread();
	::DeleteCriticalSection(&m_csThread);
}

int CMyThread::Run()
{
	return 0;
}

UINT __stdcall CMyThread::ThreadProc(void* pv)
{
	UINT uRet;
	CMyThread* pThread = (CMyThread*) pv;
	uRet = (UINT) pThread->Run();
	::EnterCriticalSection(&pThread->m_csThread);
	bool bNeedEnd = (pThread->m_hThread != NULL);
	if (bNeedEnd)
		pThread->EndThread();
	::LeaveCriticalSection(&pThread->m_csThread);
	if (bNeedEnd)
		pThread->PostEndThread();
	_endthreadex(uRet);
	return uRet;
}

bool CMyThread::StartThread()
{
	::EnterCriticalSection(&m_csThread);
	if (m_hThread)
	{
		::LeaveCriticalSection(&m_csThread);
		return false;
	}
	::LeaveCriticalSection(&m_csThread);

	HANDLE h;
	h = (HANDLE) _beginthreadex(NULL, 0, CMyThread::ThreadProc, this,
		CREATE_SUSPENDED, (UINT*) &m_dwThreadID);
	if (!h)
	{
		PostEndThread();
		return false;
	}
	m_hThread = h;
	::ResumeThread(h);
	return true;
}

bool CMyThread::IsThreadExited() const
{
	return m_hThread == NULL;
}

bool CMyThread::TerminateThread(DWORD dwExitCode)
{
	::EnterCriticalSection(&m_csThread);
	if (!m_hThread)
	{
		::LeaveCriticalSection(&m_csThread);
		return true;
	}
	if (m_dwThreadID == ::GetCurrentThreadId() || !::TerminateThread(m_hThread, dwExitCode))
	{
		::LeaveCriticalSection(&m_csThread);
		return false;
	}
	EndThread();
	::LeaveCriticalSection(&m_csThread);
	PostEndThread();
	return true;
}

void CMyThread::EndThread()
{
	if (m_hThread)
		::CloseHandle(m_hThread);
	m_hThread = NULL;
	m_dwThreadID = 0;
}

////////////////////////////////////////////////////////////////////////////////

CMyWinThread::CMyWinThread()
	: CMyThread(true)
{
	memset(&m_msg, 0, sizeof(m_msg));
	m_msg.message = WM_QUIT;
	m_msg.wParam = (WPARAM) -1;
	m_msg.lParam = 0;
	m_pMainWnd = NULL;
}

int CMyWinThread::ExitInstance()
{
	return (int) m_msg.wParam;
}

int CMyWinThread::Run()
{
	// メイン メッセージ ループ:
	long c;
	bool bExit = false;
	while (!bExit)
	{
		if (!m_pMainWnd)
			return ExitInstance();

		c = 0;
		// キューにメッセージが無い間 OnIdle を呼び続ける
		while (!::PeekMessage(&m_msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!OnIdle(c++))
				break;
		}
		// キューにメッセージがある限り、それらを処理する
		// (OnIdle が false を返して戻った場合、GetMessage で待機する)
		while (true)
		{
			if (!PumpMessage())
			{
				bExit = true;
				break;
			}
			if (!::PeekMessage(&m_msg, NULL, 0, 0, PM_NOREMOVE))
				break;
		}
	}
	return ExitInstance();
}

bool CMyWinThread::OnIdle(long lCount)
{
	return false;
}

bool CMyWinThread::PumpMessage()
{
	//if (!::PeekMessage(&m_msg, NULL, 0, 0, PM_NOREMOVE))
	//	return true;
	if (!::GetMessage(&m_msg, NULL, 0, 0))
		return false;
	CMyWindow* pWnd = CMyWindow::FromHandle(m_msg.hwnd);
	if (pWnd && pWnd != m_pMainWnd && pWnd->PreTranslateMessage(&m_msg))
		return true;
	CMyWindow* pWnd2 = CMyWindow::FromHandle(::GetActiveWindow());
	if (pWnd2 && pWnd2 != m_pMainWnd && pWnd2 != pWnd && pWnd2->PreTranslateMessage(&m_msg))
		return true;
	if (!m_pMainWnd || !m_pMainWnd->PreTranslateMessage(&m_msg))
	{
		::TranslateMessage(&m_msg);
		::DispatchMessage(&m_msg);
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////

CMyApplication::CMyApplication()
{
	if (!s_app.pMainApp)
		s_app.pMainApp = this;
}

CMyApplication::~CMyApplication()
{
	m_hThread = NULL;
}

////////////////////////////////////////////////////////////////////////////////

CMyDLLApplication::CMyDLLApplication()
{
	if (!s_app.pDLLApp)
		s_app.pDLLApp = this;
}

CMyDLLApplication::~CMyDLLApplication()
{
	m_hThread = NULL;
}
