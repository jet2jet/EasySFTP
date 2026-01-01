/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 Main.cpp - registering EasySFTP.dll with an administrator priviledges
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <tchar.h>

#include "MyFunc.h"
#include "UString.h"

#include "resource.h"

// in EasySFTP.dll
STDAPI EasySFTPRegisterServer(bool bForUser);
STDAPI EasySFTPUnregisterServer(bool bForUser);

static bool __stdcall _IsElevated()
{
	auto hProcess = ::GetCurrentProcess();
	HANDLE hToken;
	if (!::OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
		return false;
	DWORD dw = 0, dwLen = 0;
	if (!::GetTokenInformation(hToken, TokenElevation, &dw, sizeof(dw), &dwLen) || dwLen != sizeof(dw))
	{
		::CloseHandle(hToken);
		return false;
	}
	::CloseHandle(hToken);
	return dw != 0;
}

EXTERN_C int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
	HRESULT hr;
	bool bUnregister = false;
	for (int i = 1; i < __argc; i++)
	{
		LPCTSTR lp = __targv[i];
		if (*lp == _T('-') || *lp == _T('/'))
		{
			lp++;
			if (_tcsicmp(lp, _T("u")) == 0 ||
				_tcsicmp(lp, _T("unregister")) == 0 ||
				_tcsicmp(lp, _T("unregserver")) == 0)
				bUnregister = true;
			else if (_tcsicmp(lp, _T("register")) == 0 ||
				_tcsicmp(lp, _T("regserver")) == 0)
				bUnregister = false;
		}
	}

	bool bForUser = !_IsElevated();
	hr = (bUnregister ? EasySFTPUnregisterServer(bForUser) : EasySFTPRegisterServer(bForUser));

	CMyStringW strMsg, strCaption;
	strCaption.LoadString(IDS_APP_TITLE);
	if (FAILED(hr))
	{
		CMyStringW strBuf;
		LPVOID lpMsgBuf;
		::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						hr,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPTSTR) &lpMsgBuf,
						0,
						NULL);
		strBuf = (LPCTSTR) lpMsgBuf;
		::LocalFree(lpMsgBuf);
		strMsg.Format(IDS_ERROR, hr, (LPCWSTR) strBuf);
		::MessageBoxW(NULL, strMsg, strCaption, MB_ICONEXCLAMATION);

		return 1;
	}
	else
	{
		strMsg.LoadString(bUnregister ? IDS_UNREGISTER_SUCCEEDED : IDS_REGISTER_SUCCEEDED);
		::MessageBoxW(NULL, strMsg, strCaption, MB_ICONINFORMATION);

		return 0;
	}
}
