/*
 EasySFTP - Copyright (C) 2025 Kuri-Applications

 CallSync.cpp - implementations of synchronization between IShellFolder's
 */

#include "stdafx.h"
#include "EasySFTP.h"
#include "CallSync.h"

#include "ShlItem.h"
#include "SyncUtil.h"

#include <process.h>

///////////////////////////////////////////////////////////////////////////////

static HRESULT _PushOperations(IShellFolder* pFolderFrom, IShellItem* pItemTo, IShellFolder* pFolderTo, CMySimpleArray<FileData>& aFiles, IFileOperation* pOperation, IBindCtx* pbDummyDir, HWND hWndOwner, EasySFTPSynchronizeModeFlags Flags)
{
	CMyStringArrayW aExistingFiles;
	CMySimpleArray<bool> aExistingFilesIsDirectory;

	aExistingFiles.SetCaseSensitive(false);

	auto hr = S_OK;
	if (pFolderTo)
	{
		hr = _GatherFileNames(pFolderTo, hWndOwner, !(Flags & EasySFTPSynchronizeMode::SyncExcludeHidden), aExistingFiles, &aExistingFilesIsDirectory);
		if (FAILED(hr))
			return hr;
	}

	for (int i = 0; i < aFiles.GetCount(); ++i)
	{
		auto& data = aFiles.GetItemRef(i);

		HRESULT hr2 = S_OK;
		if (data.bIsDirectory)
		{
			CMyStringW str;
			IShellItem* pChildItemTo = NULL;
			{
				auto j = aExistingFiles.FindItem(data.lpwName);
				if (j >= 0)
				{
					hr2 = ::MyCreateShellItemFromRelativeName(pItemTo, data.lpwName, NULL, &pChildItemTo);
					if (!aExistingFilesIsDirectory.GetItem(j))
					{
						hr2 = pOperation->DeleteItem(pChildItemTo, NULL);
						pChildItemTo->Release();
						pChildItemTo = NULL;
					}
				}
			}
			IShellFolder* pFolderFromChild = NULL;
			hr2 = pFolderFrom->BindToObject(data.pidlItem, NULL, IID_IShellFolder, reinterpret_cast<void**>(&pFolderFromChild));
			if (SUCCEEDED(hr2) && !pFolderFromChild)
				hr2 = E_FAIL;
			if (SUCCEEDED(hr2))
			{
				bool bIsDummyItem = false;
				if (pChildItemTo == NULL)
				{
					bIsDummyItem = true;
					hr2 = MyCreateShellItemFromRelativeName(pItemTo, data.lpwName, pbDummyDir, &pChildItemTo);
				}
				if (SUCCEEDED(hr2))
				{
					IShellFolder* pFolderToChild = NULL;
					if (!bIsDummyItem)
					{
						hr2 = pChildItemTo->BindToHandler(NULL, BHID_SFObject, IID_IShellFolder, reinterpret_cast<void**>(&pFolderToChild));
						if (SUCCEEDED(hr2) && !pFolderToChild)
							hr2 = E_FAIL;
					}
					if (SUCCEEDED(hr2))
					{
						hr2 = _PushOperations(pFolderFromChild, pChildItemTo, pFolderToChild, *data.paChildren, pOperation, pbDummyDir, hWndOwner, Flags);
						if (pFolderToChild)
							pFolderToChild->Release();
					}
					pChildItemTo->Release();
				}
				pFolderFromChild->Release();
			}
		}
		else
		{
			IShellItem* pItem;
			hr2 = ::MyCreateShellItemOnFolder(pFolderFrom, data.pidlItem, &pItem);
			if (SUCCEEDED(hr2))
			{
				hr2 = pOperation->CopyItem(pItem, pItemTo, data.lpwName, NULL);
				pItem->Release();
			}
		}
		// remove name from list
		{
			auto i = aExistingFiles.FindItem(data.lpwName);
			if (i >= 0)
				aExistingFiles.RemoveItem(i);
		}
		if (SUCCEEDED(hr))
			hr = hr2;
	}

	if (SUCCEEDED(hr) && (Flags & EasySFTPSynchronizeMode::SyncDeleteIfNotExist))
	{
		HRESULT hr2 = S_OK;
		for (int i = 0; i < aExistingFiles.GetCount(); ++i)
		{
			IShellItem* pChildItemTo = NULL;
			hr2 = ::MyCreateShellItemFromRelativeName(pItemTo, aExistingFiles.GetItem(i), NULL, &pChildItemTo);
			if (SUCCEEDED(hr2) && pChildItemTo)
			{
				hr2 = pOperation->DeleteItem(pChildItemTo, NULL);
				pChildItemTo->Release();
			}
			if (SUCCEEDED(hr))
				hr = hr2;
		}
	}

	return hr;
}

static HRESULT _PerformSync(IShellFolder* pFolderFrom, IShellFolder* pFolderTo, HWND hWndOwner, EasySFTPSynchronizeModeFlags Flags)
{
	CMySimpleArray<FileData> aFiles;

	// gather files
	auto hr = _GatherFilesFromFolder(pFolderFrom, hWndOwner, !(Flags & EasySFTPSynchronizeMode::SyncExcludeHidden), aFiles);
	if (FAILED(hr))
		return hr;

	if (aFiles.GetCount() == 0)
		return S_FALSE;

	IBindCtx* pb;
	hr = ::CreateBindCtx(0, &pb);
	if (FAILED(hr))
	{
		_ReleaseFiles(aFiles);
		return hr;
	}
	CDummyFileSystemBindData* pBind = new CDummyFileSystemBindData();
	if (!pBind)
	{
		pb->Release();
		_ReleaseFiles(aFiles);
		return hr;
	}
	hr = pb->RegisterObjectParam(STR_FILE_SYS_BIND_DATA, pBind);
	pBind->Release();
	if (FAILED(hr))
	{
		pb->Release();
		_ReleaseFiles(aFiles);
		return hr;
	}

	IShellItem* pItemTo = NULL;
	hr = ::MyCreateShellItemOfFolder(pFolderTo, &pItemTo);
	if (FAILED(hr))
	{
		pb->Release();
		_ReleaseFiles(aFiles);
		return hr;
	}

	IFileOperation* pOperation;
	hr = ::CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_INPROC_SERVER, IID_IFileOperation, reinterpret_cast<void**>(&pOperation));
	if (FAILED(hr))
	{
		pItemTo->Release();
		pb->Release();
		_ReleaseFiles(aFiles);
		return hr;
	}

	hr = pOperation->SetOperationFlags(FOF_NOCONFIRMATION | FOF_ALLOWUNDO | FOF_NOCONFIRMMKDIR | ((Flags & EasySFTPSynchronizeMode::SyncCopyOld) ? 0 : FOFX_KEEPNEWERFILE));

	hr = _PushOperations(pFolderFrom, pItemTo, pFolderTo, aFiles, pOperation, pb, hWndOwner, Flags);

	pItemTo->Release();
	pb->Release();
	_ReleaseFiles(aFiles);

	if (SUCCEEDED(hr))
		hr = pOperation->PerformOperations();

	pOperation->Release();

	return hr;
}

struct SyncThreadData
{
	HWND hWndOwner;
	IShellFolder* pFolderFrom;
	IShellFolder* pFolderTo;
	EasySFTPSynchronizeModeFlags Flags;
};

static unsigned __stdcall _SyncThreadProc(SyncThreadData* pData)
{
	if (FAILED(::OleInitialize(NULL)))
	{
		_endthreadex(-1);
		return -1;
	}
	_PerformSync(pData->pFolderFrom, pData->pFolderTo, pData->hWndOwner, pData->Flags);

	// cleanup
	pData->pFolderFrom->Release();
	pData->pFolderTo->Release();
	delete pData;
	::OleUninitialize();
	_endthreadex(0);
	return 0;
}

EXTERN_C HRESULT EasySFTPSynchronize(IShellFolder* pFolderFrom, IShellFolder* pFolderTo, HWND hWndOwner, EasySFTPSynchronizeModeFlags Flags)
{
	pFolderFrom->AddRef();
	pFolderTo->AddRef();

	if (Flags & EasySFTPSynchronizeMode::SyncNoAsync)
	{
		auto hr = _PerformSync(pFolderFrom, pFolderTo, hWndOwner, Flags);
		pFolderTo->Release();
		pFolderFrom->Release();
		return hr;
	}
	else
	{
		auto* pData = new SyncThreadData();
		if (!pData)
		{
			pFolderTo->Release();
			pFolderFrom->Release();
			return E_OUTOFMEMORY;
		}
		pData->hWndOwner = hWndOwner;
		pData->pFolderFrom = pFolderFrom;
		pData->pFolderTo = pFolderTo;
		pData->Flags = Flags;

		UINT u = 0;
		auto h = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, reinterpret_cast<unsigned(__stdcall*)(void*)>(_SyncThreadProc), pData, 0, &u));
		if (!h || h == INVALID_HANDLE_VALUE)
		{
			pFolderTo->Release();
			pFolderFrom->Release();
			delete pData;
			return E_FAIL;
		}
		::CloseHandle(h);

		// performed in another thread
		return S_OK;
	}
}

///////////////////////////////////////////////////////////////////////////////

EXTERN_C HRESULT EasySFTPSynchronizeAuto(IShellFolder* pFolderFrom, IShellFolder* pFolderTo, HWND hWndOwner, EasySFTPSynchronizeModeFlags Flags)
{
	IEasySFTPDirectorySynchronization* pDir;
	if (SUCCEEDED(pFolderTo->QueryInterface(IID_IEasySFTPDirectorySynchronization, reinterpret_cast<void**>(&pDir))))
	{
		auto hr = pDir->SynchronizeFolderFrom(reinterpret_cast<LONG_PTR>(hWndOwner), pFolderFrom, Flags);
		pDir->Release();
		return hr;
	}
	if (SUCCEEDED(pFolderFrom->QueryInterface(IID_IEasySFTPDirectorySynchronization, reinterpret_cast<void**>(&pDir))))
	{
		auto hr = pDir->SynchronizeFolderTo(reinterpret_cast<LONG_PTR>(hWndOwner), pFolderTo, Flags);
		pDir->Release();
		return hr;
	}
	return EasySFTPSynchronize(pFolderFrom, pFolderTo, hWndOwner, Flags);
}

static bool s_bFileOpCLSIDChecked = false;
static bool s_bFileOpCLSIDRegistered = false;

EXTERN_C bool IsSynchronizationSupported(IShellFolder* pFolderFrom, IShellFolder* pFolderTo)
{
	IEasySFTPDirectorySynchronization* pDir;
	if (SUCCEEDED(pFolderTo->QueryInterface(IID_IEasySFTPDirectorySynchronization, reinterpret_cast<void**>(&pDir))))
	{
		pDir->Release();
		return true;
	}
	if (SUCCEEDED(pFolderFrom->QueryInterface(IID_IEasySFTPDirectorySynchronization, reinterpret_cast<void**>(&pDir))))
	{
		pDir->Release();
		return true;
	}

	if (!s_bFileOpCLSIDChecked)
	{
		CMyStringW str;
		MyStringFromGUIDW(CLSID_FileOperation, str);
		str.InsertString(L"CLSID\\", 0);
		HKEY hKey;
		if (::RegOpenKeyEx(HKEY_CLASSES_ROOT, str, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
		{
			s_bFileOpCLSIDRegistered = true;
			::RegCloseKey(hKey);
		}
		s_bFileOpCLSIDChecked = true;
	}

	if (!s_bFileOpCLSIDRegistered)
		return false;

	return true;
}
