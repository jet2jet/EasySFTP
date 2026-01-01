/*
 EasySFTP - Copyright (C) 2025 jet (ジェット)

 CallSync.h - declarations of synchronization between IShellFolder's
 */

#pragma once

#include "Unknown.h"

// used for creating IShellItem of uncreated directory
class CDummyFileSystemBindData : public CUnknownImplT<IFileSystemBindData>
{
public:
	CDummyFileSystemBindData() {}
	virtual ~CDummyFileSystemBindData() override {}

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override
	{
		if (!ppv)
			return E_POINTER;
		if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IFileSystemBindData))
		{
			*ppv = static_cast<IFileSystemBindData*>(this);
			AddRef();
			return S_OK;
		}
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	STDMETHOD(SetFindData)(const WIN32_FIND_DATAW* pfd) override
	{
		// do nothing
		return S_OK;
	}
	STDMETHOD(GetFindData)(WIN32_FIND_DATAW* pfd) override
	{
		*pfd = {};
		// is a directory
		pfd->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
		return S_OK;
	}
};

EXTERN_C HRESULT EasySFTPSynchronizeAuto(IShellFolder* pFolderFrom, IShellFolder* pFolderTo, HWND hWndOwner, EasySFTPSynchronizeModeFlags Flags);
EXTERN_C bool IsSynchronizationSupported(IShellFolder* pFolderFrom, IShellFolder* pFolderTo);
