/*
 EasySFTP - Copyright (C) 2025 Kuri-Applications

 Sync.cpp - implementations of synchronization between CFTPDirectoryBase and IShellFolder
 */

#include "stdafx.h"
#include "ShellDLL.h"
#include "Sync.h"

#include "Array.h"
#include "FileStrm.h"
#include "SyncUtil.h"
#include "unicode.h"
#include "UString.h"

#include "Folder.h"
#include "FoldRoot.h"

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

class CSyncTransferStatus : public CTransferStatus
{
public:
	virtual void TransferInProgress(void* pvObject, ULONGLONG uliPosition) override
	{
		if (!pvObject)
			return;
		m_pRoot->m_pTransferDialog->UpdateTransferItem(static_cast<CTransferDialog::CTransferItem*>(pvObject), uliPosition);
	}

	virtual bool TransferIsCanceled(void* pvObject)
	{
		return m_pRoot->m_bIsTransferCanceled || m_pRoot->IsConnected() != S_OK;
	}

	CFTPDirectoryRootBase* m_pRoot;
};

///////////////////////////////////////////////////////////////////////////////

static void _RegisterSyncTransfers(LPCWSTR lpszParentName, CMySimpleArray<FileData>& aFiles, CFTPDirectoryRootBase* pRoot)
{
	if (!pRoot->m_pTransferDialog)
		return;
	CMyStringW str;
	for (int i = 0; i < aFiles.GetCount(); ++i)
	{
		auto& data = aFiles.GetItemRef(i);

		str = data.lpwName;
		if (lpszParentName)
		{
			str.InsertChar(L'/', 0);
			str.InsertString(lpszParentName, 0);
		}

		if (data.bIsDirectory)
		{
			_RegisterSyncTransfers(str, *data.paChildren, pRoot);
		}
		else
		{
			ULONGLONG ullSize = 0;
			data.pFactory->GetFileSize(&ullSize);
			data.pvTransfer = pRoot->m_pTransferDialog->AddTransferItem(ullSize, str, NULL, true);
		}
	}
}

static HRESULT _PerformSyncFromInner(IShellFolder* pParent, CFTPDirectoryBase* pDirectoryTo, HWND hWndOwner, CMySimpleArray<FileData>& aFiles, CFTPDirectoryRootBase* pRoot, CSyncTransferStatus* pStatus, EasySFTPSynchronizeModeFlags Flags)
{
	HRESULT hr = S_OK;
	CMyStringArrayW aExistingFiles;
	hr = _GatherFileNames(pDirectoryTo, hWndOwner, Flags, aExistingFiles);
	if (FAILED(hr))
		return hr;
	for (int i = 0; i < aFiles.GetCount(); ++i)
	{
		auto& data = aFiles.GetItemRef(i);

		HRESULT hr2 = S_OK;
		if (data.bIsDirectory)
		{
			IShellFolder* pChild = NULL;
			hr2 = pParent->BindToObject(data.pidlItem, NULL, IID_IShellFolder, reinterpret_cast<void**>(&pChild));
			if (SUCCEEDED(hr2))
				hr2 = pRoot->CreateFTPDirectory(hWndOwner, pDirectoryTo, data.lpwName);
			else
				pChild = NULL;
			if (SUCCEEDED(hr2))
			{
				CFTPDirectoryBase* pDir;
				hr2 = pDirectoryTo->OpenNewDirectory(data.lpwName, &pDir);
				if (SUCCEEDED(hr2))
				{
					hr2 = _PerformSyncFromInner(pChild, pDir, hWndOwner, *data.paChildren, pRoot, pStatus, Flags);
					pDir->Release();
				}
			}
			if (pChild)
				pChild->Release();
		}
		else
		{
			bool bSkip = false;
			FILETIME ft{};
			hr2 = data.pFactory->GetFileTime(&ft);
			if (SUCCEEDED(hr2) && !(Flags & EasySFTPSynchronizeMode::SyncCopyOld))
			{
				auto* pItem = pDirectoryTo->GetFileItem(data.lpwName);
				if (pItem)
				{
					bSkip = ::CompareFileTime(&pItem->ftModifyTime, &ft) >= 0;
				}
			}
			if (bSkip)
			{
				pRoot->m_pTransferDialog->RemoveTransferItem(static_cast<CTransferDialog::CTransferItem*>(data.pvTransfer));
			}
			else
			{
				IStream* pstm = NULL;
				hr2 = data.pFactory->CreateStream(&pstm);
				if (SUCCEEDED(hr2))
				{
					if (!TEXTMODE_IS_NO_CONVERTION(pRoot->m_bTextMode))
					{
						IStream* pStream2;
						hr = pRoot->IsTextFile(data.lpwName);
						hr = MyCreateTextStream(pstm,
							hr == S_OK ?
							TEXTMODE_FOR_SEND_LOCAL_STREAM(pRoot->m_bTextMode) : 0,
							&pStream2);
						if (SUCCEEDED(hr2))
						{
							pstm->Release();
							pstm = pStream2;
						}
					}

					hr2 = pRoot->WriteFTPItem(hWndOwner, pDirectoryTo, data.lpwName, pstm, data.pvTransfer, pStatus);
					if (SUCCEEDED(hr2) && pRoot->m_bAdjustSendModifyTime)
					{
						pDirectoryTo->SetElementTimes(data.lpwName, NULL, NULL, &ft);
					}
					pstm->Release();
				}
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
		if (pStatus->TransferIsCanceled(NULL))
		{
			if (SUCCEEDED(hr))
				hr = E_ABORT;
			break;
		}
	}

	if (SUCCEEDED(hr) && (Flags & EasySFTPSynchronizeMode::SyncDeleteIfNotExist))
	{
		for (int i = 0; i < aExistingFiles.GetCount(); ++i)
		{
			auto hr2 = pDirectoryTo->DestroyElement(aExistingFiles.GetItem(i));
			if (SUCCEEDED(hr))
				hr = hr2;
		}
	}
	return hr;
}

static HRESULT _PerformSyncFrom(IShellFolder* pFolderFrom, CFTPDirectoryBase* pDirectoryTo, HWND hWndOwner, EasySFTPSynchronizeModeFlags Flags)
{
	CMySimpleArray<FileData> aFiles;

	// gather files
	auto hr = _GatherFilesFromFolder(pFolderFrom, hWndOwner, !(Flags & EasySFTPSynchronizeMode::SyncExcludeHidden), aFiles);
	if (FAILED(hr))
		return hr;

	if (aFiles.GetCount() == 0)
		return S_FALSE;

	auto* pRoot = pDirectoryTo->GetRoot();
	if (!(Flags & EasySFTPSynchronizeMode::SyncNoProgress))
	{
		pRoot->OpenTransferDialogImpl(hWndOwner);
		_RegisterSyncTransfers(NULL, aFiles, pRoot);
	}

	{
		CSyncTransferStatus status;
		status.m_pRoot = pRoot;

		hr = _PerformSyncFromInner(pFolderFrom, pDirectoryTo, hWndOwner, aFiles, pRoot, &status, Flags);
	}

	if (!(Flags & EasySFTPSynchronizeMode::SyncNoProgress))
	{
		pRoot->CloseTransferDialogImpl();
	}

	// cleanup
	_ReleaseFiles(aFiles);
	return hr;
}

struct SyncFromThreadData
{
	HWND hWndOwner;
	IShellFolder* pFolderFrom;
	CFTPDirectoryBase* pDirectoryTo;
	EasySFTPSynchronizeModeFlags Flags;
};

static unsigned __stdcall _SyncFromThreadProc(SyncFromThreadData* pData)
{
	if (FAILED(::OleInitialize(NULL)))
	{
		_endthreadex(-1);
		return -1;
	}
	_PerformSyncFrom(pData->pFolderFrom, pData->pDirectoryTo, pData->hWndOwner, pData->Flags);

	// cleanup
	pData->pFolderFrom->Release();
	pData->pDirectoryTo->Release();
	delete pData;
	::OleUninitialize();
	_endthreadex(0);
	return 0;
}

EXTERN_C HRESULT EasySFTPSynchronizeFrom(IShellFolder* pFolderFrom, CFTPDirectoryBase* pDirectoryTo, HWND hWndOwner, EasySFTPSynchronizeModeFlags Flags)
{
	if (Flags & EasySFTPSynchronizeMode::SyncNoAsync)
	{
		pDirectoryTo->AddRef();
		pFolderFrom->AddRef();
		auto hr = _PerformSyncFrom(pFolderFrom, pDirectoryTo, hWndOwner, Flags);
		pDirectoryTo->Release();
		pFolderFrom->Release();
		return hr;
	}
	else
	{
		auto* pData = new SyncFromThreadData();
		if (!pData)
			return E_OUTOFMEMORY;
		pData->hWndOwner = hWndOwner;
		pData->pFolderFrom = pFolderFrom;
		pData->pDirectoryTo = pDirectoryTo;
		pData->Flags = Flags;
		pDirectoryTo->AddRef();
		pFolderFrom->AddRef();

		UINT u = 0;
		auto h = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, reinterpret_cast<unsigned(__stdcall*)(void*)>(_SyncFromThreadProc), pData, 0, &u));
		if (!h || h == INVALID_HANDLE_VALUE)
		{
			pDirectoryTo->Release();
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

#define BUFFER_SIZE 32768

static HRESULT _PerformSyncToInner(CFTPDirectoryBase* pParent, IShellFolder* pFolderTo, HWND hWndOwner, CMySimpleArray<FileData>& aFiles, CFTPDirectoryRootBase* pRoot, CSyncTransferStatus* pStatus, EasySFTPSynchronizeModeFlags Flags, void* pvBuffer)
{
	HRESULT hr = S_OK;
	CMyStringArrayW aExistingFiles;
	hr = _GatherFileNames(pFolderTo, hWndOwner, Flags, aExistingFiles);
	if (FAILED(hr))
		return hr;
	IStorage* pStorageFolderTo = NULL;
	for (int i = 0; i < aFiles.GetCount(); ++i)
	{
		auto& data = aFiles.GetItemRef(i);

		HRESULT hr2 = S_OK;
		if (data.bIsDirectory)
		{
			CFTPDirectoryBase* pChild = NULL;
			IStorage* pstgChild = NULL;
			hr2 = pParent->OpenNewDirectory(data.lpwName, &pChild);
			if (SUCCEEDED(hr2))
			{
				if (!pStorageFolderTo)
				{
					hr2 = pFolderTo->QueryInterface(IID_IStorage, reinterpret_cast<void**>(&pStorageFolderTo));
					if (FAILED(hr2))
						hr2 = E_NOTIMPL;
				}
				if (SUCCEEDED(hr2) && pStorageFolderTo)
					hr2 = pStorageFolderTo->CreateStorage(data.lpwName, STGM_CREATE, 0, 0, &pstgChild);
			}
			else
				pChild = NULL;
			if (SUCCEEDED(hr2) && pstgChild)
			{
				IShellFolder* pDir;
				hr2 = pstgChild->QueryInterface(IID_IShellFolder, reinterpret_cast<void**>(&pDir));
				if (SUCCEEDED(hr2))
				{
					hr2 = _PerformSyncToInner(pChild, pDir, hWndOwner, *data.paChildren, pRoot, pStatus, Flags, pvBuffer);
					pDir->Release();
				}
				pstgChild->Release();
			}
			if (pChild)
				pChild->Release();
		}
		else
		{
			bool bSkip = false;
			FILETIME ft2{};
			hr2 = data.pFactory->GetFileTime(&ft2);
			if (SUCCEEDED(hr2) && !(Flags & EasySFTPSynchronizeMode::SyncCopyOld))
			{
				PITEMID_CHILD pidlExisting;
				hr2 = pFolderTo->ParseDisplayName(hWndOwner, NULL, data.lpwName, NULL, reinterpret_cast<PIDLIST_RELATIVE*>(&pidlExisting), NULL);
				if (FAILED(hr2))
					hr2 = S_OK;
				else
				{
					FILETIME ft;
					hr2 = _StatFromShellFolder(pFolderTo, pidlExisting, &ft, NULL);
					::CoTaskMemFree(pidlExisting);
					if (SUCCEEDED(hr2))
					{
						bSkip = ::CompareFileTime(&ft, &ft2) >= 0;
					}
				}
			}
			if (bSkip)
			{
				pRoot->m_pTransferDialog->RemoveTransferItem(static_cast<CTransferDialog::CTransferItem*>(data.pvTransfer));
			}
			else
			{
				hr2 = S_OK;
				if (!pStorageFolderTo)
				{
					hr2 = pFolderTo->QueryInterface(IID_IStorage, reinterpret_cast<void**>(&pStorageFolderTo));
					if (FAILED(hr2))
						hr2 = E_NOTIMPL;
				}
				if (SUCCEEDED(hr2) && pStorageFolderTo)
				{
					IStream* pstm = NULL;
					hr2 = data.pFactory->CreateStream(&pstm);
					if (SUCCEEDED(hr2))
					{
						if (!TEXTMODE_IS_NO_CONVERTION(pRoot->m_bTextMode))
						{
							hr2 = pRoot->IsTextFile(data.lpwName);
							if (SUCCEEDED(hr2) && hr2 == S_OK)
							{
								IStream* pstm2;
								hr2 = MyCreateTextStream(pstm, TEXTMODE_FOR_RECV(pRoot->m_bTextMode), &pstm2);
								if (SUCCEEDED(hr2))
								{
									pstm->Release();
									pstm = pstm2;
								}
							}
						}

						IStream* pstmDest = NULL;
						hr2 = pStorageFolderTo->CreateStream(data.lpwName, STGM_CREATE | STGM_WRITE, 0, 0, &pstmDest);
						if (SUCCEEDED(hr2))
						{
							bool bOK = false;
							ULONGLONG uliPos = 0;
							while (true)
							{
								while (theApp.CheckQueueMessage())
								{
									if (!theApp.MyPumpMessage2())
									{
										pRoot->m_bIsTransferCanceled = true;
										hr2 = E_ABORT;
										break;
									}
								}
								if (pRoot->TransferIsCanceled(NULL))
								{
									hr2 = E_ABORT;
									break;
								}
								ULONG ul = 0, ul2 = 0;
								hr2 = pstm->Read(pvBuffer, BUFFER_SIZE, &ul);
								if (FAILED(hr2))
									break;
								if (!ul)
								{
									bOK = true;
									break;
								}
								hr2 = pstmDest->Write(pvBuffer, ul, &ul2);
								if (FAILED(hr2) || ul != ul2)
									break;
								uliPos += ul2;
								pStatus->TransferInProgress(data.pvTransfer, uliPos);
							}
							pstmDest->Release();
							if (SUCCEEDED(hr2) && pRoot->m_bAdjustRecvModifyTime)
								pStorageFolderTo->SetElementTimes(data.lpwName, NULL, NULL, &ft2);
						}
						pstm->Release();
					}
				}
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
		if (pStatus->TransferIsCanceled(NULL))
		{
			if (SUCCEEDED(hr))
				hr = E_ABORT;
			break;
		}
	}

	if (SUCCEEDED(hr) && (Flags & EasySFTPSynchronizeMode::SyncDeleteIfNotExist))
	{
		HRESULT hr2 = S_OK;
		if (!pStorageFolderTo)
		{
			hr2 = pFolderTo->QueryInterface(IID_IStorage, reinterpret_cast<void**>(&pStorageFolderTo));
			if (FAILED(hr2))
				hr2 = E_NOTIMPL;
		}
		if (SUCCEEDED(hr2) && pStorageFolderTo)
		{
			for (int i = 0; i < aExistingFiles.GetCount(); ++i)
			{
				hr2 = pStorageFolderTo->DestroyElement(aExistingFiles.GetItem(i));
				if (SUCCEEDED(hr))
					hr = hr2;
			}
		}
		if (SUCCEEDED(hr))
			hr = hr2;
	}

	if (pStorageFolderTo)
		pStorageFolderTo->Release();

	return hr;
}

static HRESULT _PerformSyncTo(CFTPDirectoryBase* pDirectoryFrom, IShellFolder* pFolderTo, HWND hWndOwner, EasySFTPSynchronizeModeFlags Flags)
{
	CMySimpleArray<FileData> aFiles;

	// gather files
	auto hr = _GatherFilesFromFolder(pDirectoryFrom, hWndOwner, !(Flags & EasySFTPSynchronizeMode::SyncExcludeHidden), aFiles);
	if (FAILED(hr))
		return hr;

	if (aFiles.GetCount() == 0)
		return S_FALSE;

	auto* pvBuffer = malloc(BUFFER_SIZE);
	if (!pvBuffer)
	{
		_ReleaseFiles(aFiles);
		return E_OUTOFMEMORY;
	}

	auto* pRoot = pDirectoryFrom->GetRoot();
	if (!(Flags & EasySFTPSynchronizeMode::SyncNoProgress))
	{
		pRoot->OpenTransferDialogImpl(hWndOwner);
		_RegisterSyncTransfers(NULL, aFiles, pRoot);
	}

	{
		CSyncTransferStatus status;
		status.m_pRoot = pRoot;

		hr = _PerformSyncToInner(pDirectoryFrom, pFolderTo, hWndOwner, aFiles, pRoot, &status, Flags, pvBuffer);
	}

	if (!(Flags & EasySFTPSynchronizeMode::SyncNoProgress))
	{
		pRoot->CloseTransferDialogImpl();
	}

	// cleanup
	_ReleaseFiles(aFiles);
	free(pvBuffer);
	return hr;
}

struct SyncToThreadData
{
	HWND hWndOwner;
	CFTPDirectoryBase* pDirectoryFrom;
	IShellFolder* pFolderTo;
	EasySFTPSynchronizeModeFlags Flags;
};

static unsigned __stdcall _SyncToThreadProc(SyncToThreadData* pData)
{
	if (FAILED(::OleInitialize(NULL)))
	{
		_endthreadex(-1);
		return -1;
	}
	_PerformSyncTo(pData->pDirectoryFrom, pData->pFolderTo, pData->hWndOwner, pData->Flags);

	// cleanup
	pData->pDirectoryFrom->Release();
	pData->pFolderTo->Release();
	delete pData;
	::OleUninitialize();
	_endthreadex(0);
	return 0;
}

EXTERN_C HRESULT EasySFTPSynchronizeTo(CFTPDirectoryBase* pDirectoryFrom, IShellFolder* pFolderTo, HWND hWndOwner, EasySFTPSynchronizeModeFlags Flags)
{
	if (Flags & EasySFTPSynchronizeMode::SyncNoAsync)
	{
		pDirectoryFrom->AddRef();
		pFolderTo->AddRef();
		auto hr = _PerformSyncTo(pDirectoryFrom, pFolderTo, hWndOwner, Flags);
		pFolderTo->Release();
		pDirectoryFrom->Release();
		return hr;
	}
	else
	{
		auto* pData = new SyncToThreadData();
		if (!pData)
			return E_OUTOFMEMORY;
		pData->hWndOwner = hWndOwner;
		pData->pDirectoryFrom = pDirectoryFrom;
		pData->pFolderTo = pFolderTo;
		pData->Flags = Flags;
		pFolderTo->AddRef();
		pDirectoryFrom->AddRef();

		UINT u = 0;
		auto h = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, reinterpret_cast<unsigned(__stdcall*)(void*)>(_SyncToThreadProc), pData, 0, &u));
		if (!h || h == INVALID_HANDLE_VALUE)
		{
			pFolderTo->Release();
			pDirectoryFrom->Release();
			delete pData;
			return E_FAIL;
		}
		::CloseHandle(h);

		// performed in another thread
		return S_OK;
	}
}
