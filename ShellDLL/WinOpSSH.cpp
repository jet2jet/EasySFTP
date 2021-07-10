#include "stdafx.h"
#include "ShellDLL.h"
#include "WinOpSSH.h"

// ConvertEndian
#include "ExBuffer.h"

#define WIN_OPENSSH_AGENT_PIPE _T("\\\\.\\pipe\\openssh-ssh-agent")

bool CWinOpenSSHAgent::IsAvailable()
{
	DWORD dwStartTick = ::GetTickCount() + 1000;
	while (true)
	{
		auto h = ::CreateFile(WIN_OPENSSH_AGENT_PIPE, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, 0, NULL);
		if (h && h != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(h);
			return true;
		}
		if (::GetLastError() != ERROR_PIPE_BUSY)
		{
			return false;
		}
		if (::GetTickCount() >= dwStartTick)
		{
			return false;
		}
		::WaitNamedPipe(WIN_OPENSSH_AGENT_PIPE, 100);
	}
	// unreachable
}

bool CWinOpenSSHAgent::Query(const void* dataSend, size_t dataSendSize, void** dataReceived, size_t* dataReceivedSize)
{
	HANDLE h;
	DWORD dwStartTick = ::GetTickCount() + 1000;
	while (true)
	{
		h = ::CreateFile(WIN_OPENSSH_AGENT_PIPE, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, 0, NULL);
		if (h && h != INVALID_HANDLE_VALUE)
		{
			break;
		}
		if (::GetLastError() != ERROR_PIPE_BUSY)
		{
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
		::CloseHandle(h);
		return false;
	}

	DWORD dw = 0;
	if (!::WriteFile(h, dataSend, static_cast<DWORD>(dataSendSize), &dw, NULL))
	{
		::CloseHandle(h);
		return false;
	}
	DWORD dwLenCE = 0;
	if (!::ReadFile(h, &dwLenCE, 4, &dw, NULL))
	{
		::CloseHandle(h);
		return false;
	}
	auto dwLen = ::ConvertEndian(dwLenCE);
	auto buff = static_cast<LPBYTE>(malloc(sizeof(DWORD) + sizeof(BYTE) * dwLen));
	if (!buff)
	{
		::CloseHandle(h);
		return false;
	}
	*reinterpret_cast<DWORD*>(buff) = dwLenCE;
	if (!::ReadFile(h, buff + sizeof(DWORD), sizeof(BYTE) * dwLen, &dw, NULL))
	{
		::CloseHandle(h);
		return false;
	}

	*dataReceived = buff;
	*dataReceivedSize = sizeof(DWORD) + dw;
	return true;
}
