/*
 Copyright (C) 2011 Kuri-Applications

 AppClass.h - declarations of CMyApplication
 */

#pragma once

class CMyWindow;

class CMyThread
{
public:
	CMyThread();
	virtual ~CMyThread();

public:
	virtual int Run();
	virtual void PostEndThread() { }

	bool StartThread();
	bool IsThreadExited() const;
	bool TerminateThread(DWORD dwExitCode);
	bool IsWinThread() const { return m_bWinThread; }

public:
	DWORD m_dwThreadID;
	HANDLE m_hThread;

protected:
	CMyThread(bool bWinThread);
private:
	void CommonConstruct();

	CRITICAL_SECTION m_csThread;
	bool m_bWinThread;

	void EndThread();

	static UINT __stdcall ThreadProc(void* pv);
};

class DECLSPEC_NOVTABLE CMyWinThread : public CMyThread
{
public:
	CMyWinThread();

	virtual bool InitInstance() = 0;
	virtual int ExitInstance();
	virtual int Run();
	virtual bool OnIdle(long lCount);
	bool PumpMessage();

	MSG m_msg;
	CMyWindow* m_pMainWnd;
};

class DECLSPEC_NOVTABLE CMyApplication : public CMyWinThread
{
public:
	CMyApplication();
	virtual ~CMyApplication();

	HINSTANCE m_hInstance;
	LPTSTR m_lpCmdLine;
	int m_nCmdShow;

public:
	//virtual bool InitInstance() = 0;
};

class DECLSPEC_NOVTABLE CMyDLLApplication : public CMyThread
{
public:
	CMyDLLApplication();
	virtual ~CMyDLLApplication();

	HINSTANCE m_hInstance;

public:
	virtual bool InitInstance() = 0;
	virtual int ExitInstance();
};

CMyThread* WINAPI GetCurThread();
CMyApplication* WINAPI GetCurApp();
CMyDLLApplication* WINAPI GetCurDLLApp();
EXTERN_C HINSTANCE WINAPI MyGetCurrentInstance();

EXTERN_C int APIENTRY MyWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine, int nCmdShow);
EXTERN_C BOOL APIENTRY MyDllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved);
