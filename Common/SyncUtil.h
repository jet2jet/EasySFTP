/*
 Copyright (C) 2025 Kuri-Applications

 SyncUtil.h - declarations of utility functions/structures/classes for synchronization
 */

#pragma once

#include "Array.h"
#include "UString.h"

 ///////////////////////////////////////////////////////////////////////////////

class __declspec(novtable) CStreamFactory
{
public:
	virtual ~CStreamFactory() {}
	virtual HRESULT CreateStream(IStream** ppstm) = 0;
	virtual HRESULT GetFileTime(FILETIME* pft) = 0;
	virtual HRESULT GetFileSize(ULONGLONG* pull) = 0;
};

class CFolderStreamFactory : public CStreamFactory
{
public:
	CFolderStreamFactory(IShellFolder* pFolder, PCUITEMID_CHILD pidlItem);
	virtual ~CFolderStreamFactory();

	virtual HRESULT CreateStream(IStream** ppstm) override;
	virtual HRESULT GetFileTime(FILETIME* pft) override;
	virtual HRESULT GetFileSize(ULONGLONG* pull) override;

private:
	HRESULT DoStat();

	IShellFolder* m_pFolder;
	PCUITEMID_CHILD m_pidlItem;
	FILETIME m_ftTime;
	ULONGLONG m_ullSize;
	bool m_bRetrieved;
};

class CFileSystemStreamFactory : public CStreamFactory
{
public:
	CFileSystemStreamFactory(LPCWSTR lpszFileName);
	virtual ~CFileSystemStreamFactory() {}

	virtual HRESULT CreateStream(IStream** ppstm) override;
	virtual HRESULT GetFileTime(FILETIME* pft) override;
	virtual HRESULT GetFileSize(ULONGLONG* pull) override;

private:
	HRESULT DoStat();

	CMyStringW m_strFileName;
	FILETIME m_ftTime;
	ULONGLONG m_ullSize;
	bool m_bRetrieved;
};

struct FileData
{
	union
	{
		CStreamFactory* pFactory; // available only if bIsDirectory != true
		CMySimpleArray<FileData>* paChildren; // available only if bIsDirectory == true
	};
	LPWSTR lpwName;
	PITEMID_CHILD pidlItem;
	void* pvTransfer;
	bool bIsDirectory;
};

///////////////////////////////////////////////////////////////////////////////

HRESULT _StatFromShellFolder(IShellFolder* pParent, PCUITEMID_CHILD pidlChild, FILETIME* pft, ULONGLONG* pull);
HRESULT _MakeStreamFactory(IShellFolder* pFolder, PCUITEMID_CHILD pidlChild, CStreamFactory** ppFactory);
HRESULT _GatherFilesFromFolder(IShellFolder* pFolder, HWND hWndOwner, bool bIncludeHidden, CMySimpleArray<FileData>& aFiles);
HRESULT _GatherFileNames(IShellFolder* pFolder, HWND hWndOwner, bool bIncludeHidden, CMyStringArrayW& aFileNames, CMySimpleArray<bool>* paIsDirectoryArray = NULL);
void _ReleaseFiles(CMySimpleArray<FileData>& aFiles);
