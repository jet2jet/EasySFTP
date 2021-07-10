/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 MWndConn.cpp - implementations of CMainWindow methods (using FTP/SFTP connections)
 */

#include "StdAfx.h"
#include "EasySFTP.h"
#include "MainWnd.h"

#include "IDList.h"

////////////////////////////////////////////////////////////////////////////////

void CMainWindow::DoHostConnect(bool bServer)
{
	if (bServer)
	{
		if (m_wndListViewServer.m_pDirectory)
			m_wndListViewServer.Refresh();
		else
			m_wndListViewServer.DoOpen(m_hWnd);
	}
	else
	{
		if (m_wndListViewLocal.m_pDirectory)
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
	if (bServer && !m_wndListViewServer.m_pDirectory)
		return;
	else if (!bServer && !m_wndListViewLocal.m_pDirectory)
		return;
	if (!bForce)
	{
		if (!CanDisconnect(bServer))
			return;
	}
	if (bServer && m_wndListViewServer.m_pDirectory)
	{
		m_wndListViewServer.m_pDirectory->Disconnect();
		UpdateServerFolderAbsolute(theApp.m_pidlEasySFTP);
	}
	else if (!bServer && m_wndListViewLocal.m_pDirectory)
	{
		m_wndListViewLocal.m_pDirectory->Disconnect();
		UpdateCurrentFolderAbsolute(theApp.m_pidlEasySFTP);
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
