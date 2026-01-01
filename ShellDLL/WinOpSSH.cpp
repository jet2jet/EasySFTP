#include "stdafx.h"
#include "ShellDLL.h"
#include "WinOpSSH.h"

#include <AclAPI.h>

// ConvertEndian
#include "ExBuffer.h"

#define WIN_OPENSSH_AGENT_PIPE_NAME "openssh-ssh-agent"
#define __W(x) L##x
#define _W(x) __W(x)
#define WIN_OPENSSH_AGENT_PIPE_NAME_W _W(WIN_OPENSSH_AGENT_PIPE_NAME)
#define WIN_OPENSSH_AGENT_PIPE _T("\\\\.\\pipe\\") _T(WIN_OPENSSH_AGENT_PIPE_NAME)

typedef VOID(WINAPI* T_BuildExplicitAccessWithNameW)(_Inout_ PEXPLICIT_ACCESS_W pExplicitAccess, _In_opt_ LPWSTR pTrusteeName, _In_ DWORD AccessPermissions, _In_ ACCESS_MODE AccessMode, _In_ DWORD Inheritance);
typedef DWORD(WINAPI* T_SetEntriesInAclW)(_In_ ULONG cCountOfExplicitEntries, _In_reads_opt_(cCountOfExplicitEntries) PEXPLICIT_ACCESS_W  pListOfExplicitEntries, _In_opt_ PACL OldAcl, _Out_ PACL* NewAcl);
typedef DWORD(WINAPI* T_GetSecurityInfo)(_In_ HANDLE handle, _In_ SE_OBJECT_TYPE ObjectType, _In_ SECURITY_INFORMATION SecurityInfo, _Out_opt_ PSID* ppsidOwner, _Out_opt_ PSID* ppsidGroup, _Out_opt_ PACL* ppDacl, _Out_opt_ PACL* ppSacl, _Out_opt_ PSECURITY_DESCRIPTOR* ppSecurityDescriptor);
typedef DWORD(WINAPI* T_SetSecurityInfo)(_In_ HANDLE handle, _In_ SE_OBJECT_TYPE ObjectType, _In_ SECURITY_INFORMATION SecurityInfo, _In_opt_ PSID psidOwner, _In_opt_ PSID psidGroup, _In_opt_ PACL pDacl, _In_opt_ PACL pSacl);

static bool s_bInitAclAPI = false;
static T_BuildExplicitAccessWithNameW s_BuildExplicitAccessWithNameW = NULL;
static T_SetEntriesInAclW s_SetEntriesInAclW = NULL;
static T_GetSecurityInfo s_GetSecurityInfo = NULL;
static T_SetSecurityInfo s_SetSecurityInfo = NULL;

static bool InitAclAPI()
{
	if (!s_bInitAclAPI)
	{
		auto hModule = ::GetModuleHandle(_T("advapi32.dll"));
		if (!hModule)
			hModule = ::LoadLibrary(_T("advapi32.dll"));
		if (hModule)
		{
			s_BuildExplicitAccessWithNameW = reinterpret_cast<T_BuildExplicitAccessWithNameW>(::GetProcAddress(hModule, "BuildExplicitAccessWithNameW"));
			s_SetEntriesInAclW = reinterpret_cast<T_SetEntriesInAclW>(::GetProcAddress(hModule, "SetEntriesInAclW"));
			s_GetSecurityInfo = reinterpret_cast<T_GetSecurityInfo>(::GetProcAddress(hModule, "GetSecurityInfo"));
			s_SetSecurityInfo = reinterpret_cast<T_SetSecurityInfo>(::GetProcAddress(hModule, "SetSecurityInfo"));
		}
	}
	return s_SetSecurityInfo != NULL;
}

bool CWinOpenSSHAgent::IsAvailable()
{
	WIN32_FIND_DATAW wfd;
	auto h = ::MyFindFirstFileW(L"\\\\.\\pipe\\*", &wfd);
	if (!h || h == INVALID_HANDLE_VALUE)
		return false;
	auto found = false;
	while (true)
	{
		if (_wcsnicmp(wfd.cFileName, WIN_OPENSSH_AGENT_PIPE_NAME_W, MAX_PATH) == 0)
		{
			found = true;
			break;
		}
		if (!::MyFindNextFileW(h, &wfd))
			break;
	}
	::FindClose(h);
	return found;
}

CWinOpenSSHAgent::~CWinOpenSSHAgent()
{
	if (m_pACLKeep != NULL)
	{
		// restore DACL
		s_SetSecurityInfo(::GetCurrentProcess(), SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, m_pACLKeep, NULL);
	}
	if (m_pSecurityDescriptor != NULL)
	{
		::LocalFree(m_pSecurityDescriptor);
	}
	if (m_hPipe != INVALID_HANDLE_VALUE)
		::CloseHandle(m_hPipe);
}

bool CWinOpenSSHAgent::Query(const void* dataSend, size_t dataSendSize, void** dataReceived, size_t* dataReceivedSize)
{
	HANDLE h = m_hPipe;
	if (h == INVALID_HANDLE_VALUE)
	{
		// To access my process, especially Explorer.exe, from ssh-agent.exe,
		// we must allow ssh-agent.exe to access my process.
		// (Explorer.exe subprocess may deny from SYSTEM by default, causing
		// 'ssh-agent: error: cannot retrieve client impersonation token')
		if (InitAclAPI())
		{
			auto hProcess = ::GetCurrentProcess();
			// get and keep previous DACL
			s_GetSecurityInfo(hProcess, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, &m_pACLKeep, NULL, &m_pSecurityDescriptor);
			// make explicit access granting SYSTEM for PROCESS_QUERY_INFORMATION | PROCESS_DUP_HANDLE
			EXPLICIT_ACCESSW access = { 0 };
			s_BuildExplicitAccessWithNameW(&access, L"SYSTEM", SYNCHRONIZE | PROCESS_QUERY_INFORMATION | PROCESS_DUP_HANDLE, SET_ACCESS, NO_INHERITANCE);
			// rewrite ACL object
			PACL pACL = NULL;
			s_SetEntriesInAclW(1, &access, m_pACLKeep, &pACL);
			if (pACL != NULL)
			{
				// update DACL
				auto r = s_SetSecurityInfo(hProcess, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, pACL, NULL);
				if (r != ERROR_SUCCESS)
				{
					SetLastError(r);
					LogWin32LastError(L"SetSecurityInfo");
				}
				::LocalFree(pACL);
			}
		}

		DWORD dwStartTick = ::GetTickCount() + 1000;
		while (true)
		{
			h = ::CreateFile(WIN_OPENSSH_AGENT_PIPE, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL, OPEN_EXISTING, 0, NULL);
			if (h && h != INVALID_HANDLE_VALUE)
			{
				theApp.Log(EasySFTPLogLevel::Debug, L"open OpenSSH Agent pipe", S_OK);
				break;
			}
			if (::GetLastError() != ERROR_PIPE_BUSY)
			{
				LogWin32LastError(L"CreateFile");
				return false;
			}
			if (::GetTickCount() >= dwStartTick)
			{
				return false;
			}
			::WaitNamedPipe(WIN_OPENSSH_AGENT_PIPE, 100);
		}
		if (!::SetHandleInformation(h, HANDLE_FLAG_INHERIT, 0))
		{
			LogWin32LastError(L"SetHandleInformation");
			::CloseHandle(h);
			return false;
		}
		m_hPipe = h;
	}

	DWORD dw = 0;
	if (!::WriteFile(h, dataSend, static_cast<DWORD>(dataSendSize), &dw, NULL))
	{
		LogWin32LastError(L"WriteFile");
		return false;
	}
	if (dw < static_cast<DWORD>(dataSendSize))
	{
		return false;
	}
	DWORD dwLenCE = 0;
	if (!::ReadFile(h, &dwLenCE, 4, &dw, NULL))
	{
		LogWin32LastError(L"ReadFile");
		return false;
	}
	auto dwLen = ::ConvertEndian(dwLenCE);
	auto buff = static_cast<LPBYTE>(malloc(sizeof(DWORD) + sizeof(BYTE) * dwLen));
	if (!buff)
	{
		return false;
	}
	*reinterpret_cast<DWORD*>(buff) = dwLenCE;
	if (!::ReadFile(h, buff + sizeof(DWORD), sizeof(BYTE) * dwLen, &dw, NULL))
	{
		LogWin32LastError(L"ReadFile(2)");
		return false;
	}

	*dataReceived = buff;
	*dataReceivedSize = sizeof(DWORD) + dw;
	return true;
}
