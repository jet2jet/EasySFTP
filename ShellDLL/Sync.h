/*
 EasySFTP - Copyright (C) 2025 Kuri-Applications

 Sync.h - declarations of synchronization between CFTPDirectoryBase and IShellFolder
 */

#pragma once

#include "EasySFTP_h.h"

class CFTPDirectoryBase;

EXTERN_C HRESULT EasySFTPSynchronizeFrom(IShellFolder* pFolderFrom, CFTPDirectoryBase* pDirectoryTo, HWND hWndOwner, EasySFTPSynchronizeModeFlags Flags);
EXTERN_C HRESULT EasySFTPSynchronizeTo(CFTPDirectoryBase* pDirectoryFrom, IShellFolder* pFolderTo, HWND hWndOwner, EasySFTPSynchronizeModeFlags Flags);
