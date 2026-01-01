/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 MWndConn.cpp - implementations of CMainWindow methods (using FTP/SFTP connections)
 */

#include "StdAfx.h"
#include "EasySFTP.h"
#include "MainWnd.h"

#include "CallSync.h"
#include "IDList.h"

////////////////////////////////////////////////////////////////////////////////

void CMainWindow::DoHostConnect(bool bServer)
{
	if (bServer)
	{
		if (m_wndListViewServer.m_pRootDirectory)
			m_wndListViewServer.Refresh();
		else
			m_wndListViewServer.DoOpen(m_hWnd);
	}
	else
	{
		if (m_wndListViewLocal.m_pRootDirectory)
			m_wndListViewLocal.Refresh();
		else
			m_wndListViewLocal.DoOpen(m_hWnd);
	}
}

void CMainWindow::DoConnect()
{
	IShellFolder* pFolder;
	HRESULT hr = theApp.m_pEasySFTPRoot->QuickConnectDialog(m_hWnd, &pFolder);
	if (hr == S_OK)
	{
		IPersistFolder2* pPFolder;
		hr = pFolder->QueryInterface(IID_IPersistFolder2, (void**) &pPFolder);
		if (SUCCEEDED(hr))
		{
			PIDLIST_ABSOLUTE pidl;
			hr = pPFolder->GetCurFolder(&pidl);
			if (SUCCEEDED(hr))
			{
				UpdateServerFolderAbsolute(pidl);
				::CoTaskMemFree(pidl);
			}
			pPFolder->Release();
		}
		pFolder->Release();
	}
	//else if (FAILED(hr))
	//	::MyMessageBoxW(m_hWnd, MAKEINTRESOURCEW(IDS_FAILED_TO_CONNECT), NULL, MB_ICONEXCLAMATION);
}

//void CMainWindow::DoCloseConnection(bool bForce)
void CMainWindow::DoCloseConnection(bool bServer, bool bForce)
{
	if (bServer && !m_wndListViewServer.m_pRootDirectory)
		return;
	else if (!bServer && !m_wndListViewLocal.m_pRootDirectory)
		return;
	if (!bForce)
	{
		if (!CanDisconnect(bServer))
			return;
	}
	if (bServer && m_wndListViewServer.m_pRootDirectory)
	{
		auto pDirectory = m_wndListViewServer.m_pRootDirectory;
		// change folder before disconnect to prevent from handling MY_WM_CHANGENOTIFY
		UpdateServerFolderAbsolute(theApp.m_pidlEasySFTP);
		pDirectory->Disconnect();
	}
	else if (!bServer && m_wndListViewLocal.m_pRootDirectory)
	{
		auto pDirectory = m_wndListViewLocal.m_pRootDirectory;
		// change folder before disconnect to prevent from handling MY_WM_CHANGENOTIFY
		UpdateCurrentFolderAbsolute(theApp.m_pidlEasySFTP);
		pDirectory->Disconnect();
	}
}

static HRESULT __stdcall EmulateDragDrop(HWND hWndOwner, IShellFolder* pFolder, IDataObject* pObject,
	DWORD grfKeyState, DWORD dwEffect, DWORD* pdwEffectResult)
{
	HRESULT hr;
	IDropTarget* pDropTarget;
	hr = pFolder->CreateViewObject(hWndOwner, IID_IDropTarget, (void**) &pDropTarget);
	if (FAILED(hr))
		return E_UNEXPECTED;
	DWORD dwE = dwEffect;
	POINTL pt = { -1, -1 };
	hr = pDropTarget->DragEnter(pObject, grfKeyState, pt, &dwE);
	if (FAILED(hr))
	{
		pDropTarget->Release();
		return E_UNEXPECTED;
	}
	dwE = dwEffect;
	hr = pDropTarget->Drop(pObject, grfKeyState, pt, &dwEffect);
	pDropTarget->Release();
	if (pdwEffectResult)
		*pdwEffectResult = dwEffect;
	return hr;
}

void CMainWindow::DoDownload()
{
	HRESULT hr;
	IDataObject* pObject;
	hr = m_wndListViewServer.m_pView->GetItemObject(SVGIO_SELECTION, IID_IDataObject, (void**) &pObject);
	if (SUCCEEDED(hr))
	{
		EmulateDragDrop(m_hWnd, m_wndListViewLocal.m_pFolder, pObject, MK_CONTROL | MK_LBUTTON, DROPEFFECT_COPY, NULL);
		pObject->Release();
		//SendObject(DROPEFFECT_COPY, pObject, &hr, m_wndServerAddress.m_strDirectory);
		//return;
	}
}

void CMainWindow::DoUpload()
{
	HRESULT hr;
	IDataObject* pObject;
	hr = m_wndListViewLocal.m_pView->GetItemObject(SVGIO_SELECTION, IID_IDataObject, (void**) &pObject);
	if (SUCCEEDED(hr))
	{
		EmulateDragDrop(m_hWnd, m_wndListViewServer.m_pFolder, pObject, MK_CONTROL | MK_LBUTTON, DROPEFFECT_COPY, NULL);
		pObject->Release();
		//SendObject(DROPEFFECT_COPY, pObject, &hr, m_wndServerAddress.m_strDirectory);
		//return;
	}
}

void CMainWindow::DoDownloadAll()
{
	HRESULT hr;
	IDataObject* pObject;
	hr = m_wndListViewServer.m_pView->GetItemObject(SVGIO_ALLVIEW, IID_IDataObject, (void**) &pObject);
	if (SUCCEEDED(hr))
	{
		EmulateDragDrop(m_hWnd, m_wndListViewLocal.m_pFolder, pObject, MK_CONTROL | MK_LBUTTON, DROPEFFECT_COPY, NULL);
		pObject->Release();
		//SendObject(DROPEFFECT_COPY, pObject, &hr, m_wndServerAddress.m_strDirectory);
		//return;
	}
}

void CMainWindow::DoUploadAll()
{
	HRESULT hr;
	IDataObject* pObject;
	hr = m_wndListViewLocal.m_pView->GetItemObject(SVGIO_ALLVIEW, IID_IDataObject, (void**) &pObject);
	if (SUCCEEDED(hr))
	{
		EmulateDragDrop(m_hWnd, m_wndListViewServer.m_pFolder, pObject, MK_CONTROL | MK_LBUTTON, DROPEFFECT_COPY, NULL);
		pObject->Release();
		//SendObject(DROPEFFECT_COPY, pObject, &hr, m_wndServerAddress.m_strDirectory);
		//return;
	}
}

void CMainWindow::DoSyncLeftToRight()
{
	::EasySFTPSynchronizeAuto(m_wndListViewLocal.m_pFolder, m_wndListViewServer.m_pFolder, m_hWnd, EasySFTPSynchronizeMode::SyncNormal);
}

void CMainWindow::DoSyncRightToLeft()
{
	::EasySFTPSynchronizeAuto(m_wndListViewServer.m_pFolder, m_wndListViewLocal.m_pFolder, m_hWnd, EasySFTPSynchronizeMode::SyncNormal);
}

static HRESULT _GetPath(IShellFolder* pRoot, PCIDLIST_ABSOLUTE pidl, CMyStringW& rstr)
{
	PIDLIST_ABSOLUTE pidlParent = NULL;
	PITEMID_CHILD pidlChild = NULL;
	if (!IsDesktopIDList(pidl))
	{
		pidlParent = ::RemoveOneChild(pidl);
		if (!pidlParent)
			return E_OUTOFMEMORY;
		pidlChild = ::GetChildItemIDList(pidl);
		if (!pidlChild)
		{
			::CoTaskMemFree(pidlParent);
			return E_OUTOFMEMORY;
		}
	}
	HRESULT hr;
	STRRET strret;
	strret.uType = STRRET_WSTR;
	if (!pidlParent || !pidlChild)
	{
		hr = pRoot->GetDisplayNameOf(NULL, SHGDN_NORMAL | SHGDN_FORPARSING | SHGDN_FORADDRESSBAR, &strret);
	}
	else
	{
		IShellFolder* pParent;
		hr = pRoot->BindToObject(pidlParent, NULL, IID_IShellFolder, reinterpret_cast<void**>(&pParent));
		if (SUCCEEDED(hr))
		{
			hr = pParent->GetDisplayNameOf(pidlChild, SHGDN_NORMAL | SHGDN_FORPARSING | SHGDN_FORADDRESSBAR, &strret);
			pParent->Release();
		}
	}
	if (FAILED(hr))
		return hr;

	switch (strret.uType)
	{
		case STRRET_WSTR:
			rstr = strret.pOleStr;
			::CoTaskMemFree(strret.pOleStr);
			break;
		case STRRET_CSTR:
			rstr = strret.cStr;
			break;
		case STRRET_OFFSET:
			rstr = (LPCSTR)(((LPCBYTE)pidlChild) + strret.uOffset);
			break;
	}
	if (pidlChild)
		::CoTaskMemFree(pidlChild);
	if (pidlParent)
		::CoTaskMemFree(pidlParent);
	return S_OK;
}

void CMainWindow::DoSyncDetail()
{
	_GetPath(m_wndAddress.m_pFolderRoot, m_wndListViewLocal.m_lpidlAbsoluteMe, m_SyncDetailDialog.m_strLeft);
	_GetPath(m_wndAddress.m_pFolderRoot, m_wndListViewServer.m_lpidlAbsoluteMe, m_SyncDetailDialog.m_strRight);
	if (m_SyncDetailDialog.ModalDialogW(m_hWnd) == IDOK)
	{
		auto* pFolderFrom = m_SyncDetailDialog.m_bIsLeftToRight ? m_wndListViewLocal.m_pFolder : m_wndListViewServer.m_pFolder;
		auto* pFolderTo = m_SyncDetailDialog.m_bIsLeftToRight ? m_wndListViewServer.m_pFolder : m_wndListViewLocal.m_pFolder;
		::EasySFTPSynchronizeAuto(pFolderFrom, pFolderTo, m_hWnd, m_SyncDetailDialog.m_Flags);
	}
}
