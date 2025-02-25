/*
 Copyright (C) 2025 Kuri-Applications

 SyncUtil.cpp - implementations of utility functions/structures/classes for synchronization
 */

#include "stdafx.h"
#include "SyncUtil.h"

#include <propkey.h>
#include "FileStrm.h"
#include "unicode.h"

///////////////////////////////////////////////////////////////////////////////

CFolderStreamFactory::CFolderStreamFactory(IShellFolder* pFolder, PCUITEMID_CHILD pidlItem)
	: m_pFolder(pFolder)
	, m_pidlItem(pidlItem)
	, m_ftTime{}
	, m_ullSize(0)
	, m_bRetrieved(false)
{
	pFolder->AddRef();
}

CFolderStreamFactory::~CFolderStreamFactory()
{
	m_pFolder->Release();
}

HRESULT CFolderStreamFactory::CreateStream(IStream** ppstm)
{
	return m_pFolder->BindToStorage(m_pidlItem, NULL, IID_IStream, reinterpret_cast<void**>(ppstm));
}

HRESULT CFolderStreamFactory::GetFileTime(FILETIME* pft)
{
	auto hr = DoStat();
	if (FAILED(hr))
		return hr;
	*pft = m_ftTime;
	return hr;
}

HRESULT CFolderStreamFactory::GetFileSize(ULONGLONG* pull)
{
	auto hr = DoStat();
	if (FAILED(hr))
		return hr;
	*pull = m_ullSize;
	return hr;
}

HRESULT CFolderStreamFactory::DoStat()
{
	if (m_bRetrieved)
		return S_OK;
	auto hr = _StatFromShellFolder(m_pFolder, m_pidlItem, &m_ftTime, &m_ullSize);
	if (SUCCEEDED(hr))
		m_bRetrieved = true;
	return hr;
}

///////////////////////////////////////////////////////////////////////////////

CFileSystemStreamFactory::CFileSystemStreamFactory(LPCWSTR lpszFileName)
	: m_strFileName(lpszFileName)
	, m_ftTime{}
	, m_ullSize(0)
	, m_bRetrieved(false)
{
}

HRESULT CFileSystemStreamFactory::CreateStream(IStream** ppstm)
{
	return MyOpenFileToStream(m_strFileName, false, ppstm);
}

HRESULT CFileSystemStreamFactory::GetFileTime(FILETIME* pft)
{
	auto hr = DoStat();
	if (FAILED(hr))
		return hr;
	*pft = m_ftTime;
	return hr;
}

HRESULT CFileSystemStreamFactory::GetFileSize(ULONGLONG* pull)
{
	auto hr = DoStat();
	if (FAILED(hr))
		return hr;
	*pull = m_ullSize;
	return hr;
}

HRESULT CFileSystemStreamFactory::DoStat()
{
	if (m_bRetrieved)
		return S_OK;
	HANDLE h = MyCreateFileW(m_strFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (!h || h == INVALID_HANDLE_VALUE)
		return HRESULT_FROM_WIN32(GetLastError());
	auto hr = S_OK;
	if (!::GetFileTime(h, NULL, NULL, &m_ftTime))
		hr = HRESULT_FROM_WIN32(GetLastError());
	else
	{
		ULARGE_INTEGER uli;
		SetLastError(ERROR_SUCCESS);
		uli.LowPart = ::GetFileSize(h, &uli.HighPart);
		if (uli.LowPart == 0xFFFFFFFF && GetLastError() != ERROR_SUCCESS)
			hr = HRESULT_FROM_WIN32(GetLastError());
		else
		{
			m_ullSize = uli.QuadPart;
			m_bRetrieved = true;
		}
	}
	::CloseHandle(h);
	return hr;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT _StatFromShellFolder(IShellFolder* pParent, PCUITEMID_CHILD pidlChild, FILETIME* pft, ULONGLONG* pull)
{
	IShellFolder2* pFolder2;
	HRESULT hr = E_FAIL;
	if (SUCCEEDED(pParent->QueryInterface(IID_IShellFolder2, reinterpret_cast<void**>(&pFolder2))))
	{
		VARIANT v{};
		if (pft)
		{
			hr = pFolder2->GetDetailsEx(pidlChild, &PKEY_DateModified, &v);
			if (FAILED(hr))
				hr = pFolder2->GetDetailsEx(pidlChild, &PKEY_ItemDate, &v);
			if (SUCCEEDED(hr))
			{
				if (v.vt != VT_DATE)
					hr = E_FAIL;
				else
				{
					SYSTEMTIME st;
					::VariantTimeToSystemTime(v.date, &st);
					::SystemTimeToFileTime(&st, pft);
				}
				::VariantClear(&v);
			}
			if (FAILED(hr))
				*pft = {};
		}
		if (pull)
		{
			hr = pFolder2->GetDetailsEx(pidlChild, &PKEY_Size, &v);
			if (SUCCEEDED(hr))
			{
				if (v.vt != VT_UI8)
					hr = E_FAIL;
				else
					*pull = v.ullVal;
				::VariantClear(&v);
			}
			if (FAILED(hr))
				*pull = 0;
		}
		pFolder2->Release();
	}
	if (FAILED(hr))
	{
		IStorage* pStorage;
		if (SUCCEEDED(pParent->BindToStorage(pidlChild, NULL, IID_IStorage, reinterpret_cast<void**>(&pStorage))))
		{
			STATSTG stat;
			hr = pStorage->Stat(&stat, STATFLAG_NONAME);
			pStorage->Release();
			if (SUCCEEDED(hr))
			{
				if (pft)
					*pft = stat.mtime;
				if (pull)
					*pull = stat.cbSize.QuadPart;
			}
		}
	}
	return hr;
}

HRESULT _MakeStreamFactory(IShellFolder* pFolder, PCUITEMID_CHILD pidlChild, CStreamFactory** ppFactory)
{
	SFGAOF rgf = SFGAO_FOLDER | SFGAO_STREAM | SFGAO_FILESYSTEM;
	auto hr = pFolder->GetAttributesOf(1, &pidlChild, &rgf);
	if (FAILED(hr))
		return hr;

	if (rgf & SFGAO_FOLDER)
	{
		*ppFactory = NULL;
		return S_FALSE;
	}
	else if (rgf & SFGAO_STREAM)
	{
		CStreamFactory* pFactory = new CFolderStreamFactory(pFolder, pidlChild);
		if (!pFactory)
			return E_OUTOFMEMORY;
		*ppFactory = pFactory;
	}
	else if (rgf & SFGAO_FILESYSTEM)
	{
		STRRET strret{};
		strret.uType = STRRET_WSTR;
		hr = pFolder->GetDisplayNameOf(pidlChild, SHGDN_FORPARSING | SHGDN_NORMAL, &strret);
		if (FAILED(hr))
			return hr;
		CMyStringW str;
		switch (strret.uType)
		{
		case STRRET_WSTR:
			str = strret.pOleStr;
			::CoTaskMemFree(strret.pOleStr);
			break;
		case STRRET_CSTR:
			str = strret.cStr;
			break;
		case STRRET_OFFSET:
			str = (reinterpret_cast<LPCSTR>(reinterpret_cast<LPCBYTE>(pidlChild) + strret.uOffset));
			break;
		default:
			break;
		}
		if (str.IsEmpty())
			return E_FAIL;
		CStreamFactory* pFactory = new CFileSystemStreamFactory(str);
		if (!pFactory)
			return E_OUTOFMEMORY;
		*ppFactory = pFactory;
	}
	else
		return E_NOTIMPL;
	return S_OK;
}

HRESULT _GatherFilesFromFolder(IShellFolder* pFolder, HWND hWndOwner, bool bIncludeHidden, CMySimpleArray<FileData>& aFiles)
{
	IEnumIDList* pEnum = NULL;
	auto hr = pFolder->EnumObjects(reinterpret_cast<HWND>(hWndOwner),
		SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | (bIncludeHidden ? (SHCONTF_INCLUDEHIDDEN | SHCONTF_INCLUDESUPERHIDDEN) : 0),
		&pEnum);
	if (FAILED(hr))
	{
		return hr;
	}
	if (hr == S_FALSE || !pEnum)
	{
		return E_NOTIMPL;
	}
	while (true)
	{
		PITEMID_CHILD pidlChild = NULL;
		{
			ULONG u = 0;
			hr = pEnum->Next(1, &pidlChild, &u);
			if (u == 0)
				break;
		}

		LPWSTR lpwName;
		{
			STRRET strret{};
			strret.uType = STRRET_WSTR;
			hr = pFolder->GetDisplayNameOf(pidlChild, SHGDN_FORPARSING | SHGDN_INFOLDER, &strret);
			if (FAILED(hr))
			{
				::CoTaskMemFree(pidlChild);
				continue;
			}
			CMyStringW str;
			switch (strret.uType)
			{
			case STRRET_WSTR:
				str = strret.pOleStr;
				::CoTaskMemFree(strret.pOleStr);
				break;
			case STRRET_CSTR:
				str = strret.cStr;
				break;
			case STRRET_OFFSET:
				str = (reinterpret_cast<LPCSTR>(reinterpret_cast<LPCBYTE>(pidlChild) + strret.uOffset));
				break;
			default:
				::CoTaskMemFree(pidlChild);
				continue;
			}
			lpwName = _wcsdup(str);
		}

		CStreamFactory* pFactory;
		hr = _MakeStreamFactory(pFolder, pidlChild, &pFactory);
		if (FAILED(hr))
		{
			free(lpwName);
			::CoTaskMemFree(pidlChild);
			continue;
		}
		if (hr == S_FALSE)
		{
			// is directory
			IShellFolder* pChild = NULL;
			hr = pFolder->BindToObject(pidlChild, NULL, IID_IShellFolder, reinterpret_cast<void**>(&pChild));
			if (SUCCEEDED(hr) && pChild)
			{
				CMySimpleArray<FileData>* pChildren = new CMySimpleArray<FileData>();
				if (pChildren)
				{
					FileData d;
					d.paChildren = pChildren;
					d.lpwName = lpwName;
					d.pidlItem = pidlChild;
					d.pvTransfer = NULL;
					d.bIsDirectory = true;
					lpwName = NULL;
					pidlChild = NULL;
					aFiles.Add(d);
					hr = _GatherFilesFromFolder(pChild, hWndOwner, bIncludeHidden, *pChildren);
				}
				pChild->Release();
			}
		}
		else
		{
			FileData d;
			d.pFactory = pFactory;
			d.lpwName = lpwName;
			d.pidlItem = pidlChild;
			d.pvTransfer = NULL;
			d.bIsDirectory = false;
			lpwName = NULL;
			pidlChild = NULL;
			aFiles.Add(d);
		}
		if (lpwName)
			free(lpwName);
		if (pidlChild)
			::CoTaskMemFree(pidlChild);
	}
	pEnum->Release();
	return hr;
}

HRESULT _GatherFileNames(IShellFolder* pFolder, HWND hWndOwner, bool bIncludeHidden, CMyStringArrayW& aFileNames, CMySimpleArray<bool>* paIsDirectoryArray)
{
	IEnumIDList* pEnum;
	auto hr = pFolder->EnumObjects(reinterpret_cast<HWND>(hWndOwner),
		SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | (bIncludeHidden ? (SHCONTF_INCLUDEHIDDEN | SHCONTF_INCLUDESUPERHIDDEN) : 0),
		&pEnum);
	if (FAILED(hr))
	{
		return hr;
	}
	while (true)
	{
		PITEMID_CHILD pidlChild = NULL;
		{
			ULONG u = 0;
			hr = pEnum->Next(1, &pidlChild, &u);
			if (u == 0)
				break;
		}

		{
			STRRET strret{};
			strret.uType = STRRET_WSTR;
			hr = pFolder->GetDisplayNameOf(pidlChild, SHGDN_FORPARSING | SHGDN_INFOLDER, &strret);
			if (FAILED(hr))
			{
				::CoTaskMemFree(pidlChild);
				continue;
			}
			CMyStringW str;
			switch (strret.uType)
			{
				case STRRET_WSTR:
					str = strret.pOleStr;
					::CoTaskMemFree(strret.pOleStr);
					break;
				case STRRET_CSTR:
					str = strret.cStr;
					break;
				case STRRET_OFFSET:
					str = (reinterpret_cast<LPCSTR>(reinterpret_cast<LPCBYTE>(pidlChild) + strret.uOffset));
					break;
				default:
					::CoTaskMemFree(pidlChild);
					continue;
			}
			SFGAOF rgf = SFGAO_FOLDER;
			hr = pFolder->GetAttributesOf(1, &pidlChild, &rgf);
			if (FAILED(hr))
			{
				rgf = 0;
				hr = S_OK;
			}
			aFileNames.Add(str);
			if (paIsDirectoryArray)
				paIsDirectoryArray->Add((rgf & SFGAO_FOLDER) != 0);
		}
		if (pidlChild)
			::CoTaskMemFree(pidlChild);
	}
	pEnum->Release();
	return hr;
}

void _ReleaseFiles(CMySimpleArray<FileData>& aFiles)
{
	for (int i = 0; i < aFiles.GetCount(); ++i)
	{
		auto& data = aFiles.GetItemRef(i);
		if (data.bIsDirectory)
		{
			_ReleaseFiles(*data.paChildren);
			delete data.paChildren;
		}
		else
			delete data.pFactory;
		::CoTaskMemFree(data.pidlItem);
		free(data.lpwName);
	}
	aFiles.RemoveAll();
}
