/*
 Copyright (C) 2026 jet (ジェット)

 OS.cpp - implementations for OS-related functions
 */

#include "stdafx.h"
#include "OS.h"

static DWORD s_dwPlatformId = 0;

EXTERN_C bool IsUnicodeAvailableOS()
{
	if (s_dwPlatformId == 0)
	{
		OSVERSIONINFO vi{};
		vi.dwOSVersionInfoSize = sizeof(vi);
#pragma warning(push)
#pragma warning(disable:4996 28159)
		if (!::GetVersionExA(&vi))
#pragma warning(pop)
			return false;
		s_dwPlatformId = vi.dwPlatformId;
	}
	return (s_dwPlatformId != VER_PLATFORM_WIN32_WINDOWS);
}

EXTERN_C DWORD MyGetTick32()
{
#pragma warning(push)
#pragma warning(disable:28159)
	// If the caller ensures 32-bit value of tick, we should use GetTickCount safely
	return GetTickCount();
#pragma warning(pop)
}
