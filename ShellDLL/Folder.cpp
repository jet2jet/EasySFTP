/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 Folder.cpp - implementations of folder-helper classes
 */

#include "StdAfx.h"
#include "ShellDLL.h"
#include "Folder.h"

#include "EnmIDLst.h"
#include "EnmStstg.h"
#include "FileDisp.h"
#include "FileIcon.h"
#include "FileProp.h"
#include "FoldDrop.h"
#include "FoldRoot.h"
#include "FldrMenu.h"
#include "FTPStrm.h"

#include "RFolder.h"

static HMENU __stdcall GetSubMenuByID(HMENU hMenu, UINT uID)
{
	MENUITEMINFO mii;
	int nCount;

#ifdef _WIN64
	mii.cbSize = sizeof(mii);
#else
	mii.cbSize = MENUITEMINFO_SIZE_V1;
#endif
	mii.fMask = MIIM_ID | MIIM_SUBMENU;
	nCount = ::GetMenuItemCount(hMenu);
	for (int i = 0; i < nCount; i++)
	{
		::GetMenuItemInfo(hMenu, (UINT)i, TRUE, &mii);
		if (mii.wID == uID)
			return mii.hSubMenu;
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////

#define INITGUID
#include <propkeydef.h>
//DEFINE_PROPERTYKEY(PKEY_Permissions, 0xB725F130, 0x47EF, 0x101A, 0xA5, 0xF1, 0x02, 0x60, 0x8C, 0x9E, 0xEB, 0xAC, 13);
DEFINE_PROPERTYKEY(PKEY_FTPItemPermissions, 0x36089763, 0xb05f, 0x4ed7, 0xb7, 0xcb, 0xd2, 0x59, 0x7b, 0x21, 0xad, 0xa7, 2);
DEFINE_PROPERTYKEY(PKEY_FTPItemTransferType, 0x36089763, 0xb05f, 0x4ed7, 0xb7, 0xcb, 0xd2, 0x59, 0x7b, 0x21, 0xad, 0xa7, 3);
DEFINE_PROPERTYKEY(PKEY_FTPItemUID, 0x36089763, 0xb05f, 0x4ed7, 0xb7, 0xcb, 0xd2, 0x59, 0x7b, 0x21, 0xad, 0xa7, 4);
DEFINE_PROPERTYKEY(PKEY_FTPItemGID, 0x36089763, 0xb05f, 0x4ed7, 0xb7, 0xcb, 0xd2, 0x59, 0x7b, 0x21, 0xad, 0xa7, 5);

EXTERN_C const struct { UINT nID; const PROPERTYKEY& key; } s_columnIDMap[] = {
	{ IDS_HEAD_NAME, PKEY_ItemNameDisplay },
	{ IDS_HEAD_FILE_NAME, PKEY_FileName },
	{ IDS_HEAD_FILE_EXT, PKEY_FileExtension },
	{ IDS_HEAD_SIZE, PKEY_Size },
	{ IDS_HEAD_TYPE, PKEY_ItemTypeText},
	{ IDS_HEAD_CREATE_TIME, PKEY_DateCreated },
	{ IDS_HEAD_MODIFY_TIME, PKEY_DateModified },
	{ IDS_HEAD_PERMISSIONS, PKEY_FTPItemPermissions },
	{ IDS_HEAD_TRANSFER_TYPE, PKEY_FTPItemTransferType },
	{ IDS_HEAD_UID, PKEY_FTPItemUID },
	{ IDS_HEAD_GID, PKEY_FTPItemGID }
};

constexpr int _GetAvailablePropKeyCount()
{
	return sizeof(s_columnIDMap) / sizeof(s_columnIDMap[0]);
}

static UINT __stdcall _MyPropertyKeyToStringID(const PROPERTYKEY& key)
{
	for (int i = 0; i < _GetAvailablePropKeyCount(); i++)
	{
		if (IsEqualPropertyKey(s_columnIDMap[i].key, key))
			return s_columnIDMap[i].nID;
	}
	return 0;
}

//union SHFILEINFO_UNION
//{
//	SHFILEINFOA a;
//	SHFILEINFOW w;
//};

static DWORD __stdcall GetDummyFileAttribute(CFTPFileItem* pItem)
{
	if (pItem->bWinAttr)
		return pItem->dwAttributes;
	DWORD dwRet = 0;
	{
		register CFTPFileItem* p = pItem;
		while (p->pTargetFile)
			p = p->pTargetFile;
		if (p->type == fitypeDir)
			dwRet |= FILE_ATTRIBUTE_DIRECTORY;
		else
		{
			if (p->permissions.readable || !p->permissions.writable)
				dwRet |= FILE_ATTRIBUTE_READONLY;
		}
	}
	if (!pItem->strFileName.IsEmpty() && *((LPCWSTR)pItem->strFileName) == L'.')
		dwRet |= FILE_ATTRIBUTE_HIDDEN;
	return dwRet ? dwRet : FILE_ATTRIBUTE_NORMAL;
}

// also used in SFilePrp.cpp
extern void __stdcall FileTimeToString(const FILETIME* pft, CMyStringW& ret)
{
	FILETIME ft;
	SYSTEMTIME st;
	//CMyStringW str;
	::FileTimeToLocalFileTime(pft, &ft);
	::FileTimeToSystemTime(&ft, &st);

	ret.Format(L"%04hu/%02hu/%02hu %02hu:%02hu:%02hu",
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond);
}

static const WCHAR* s_szSizeType[] = {
	NULL,
	L"KB",
	L"MB",
	L"GB",
	L"TB",
	L"PB",
	L"EB",
	L"ZB",
	L"YB"
};

// also used in Transfer.cpp and SFilePrp.cpp
extern void __stdcall FileSizeToString(ULARGE_INTEGER uli, CMyStringW& ret)
{
	int t;
	t = 0;
	while (uli.QuadPart >= 2048 && t < 8)
	{
		//uli.QuadPart /= 1024;
		uli.QuadPart >>= 10;
		t++;
	}
	ret.Format(L"%u ", (UINT)uli.LowPart);
	if (t > 0)
		ret += s_szSizeType[t];
	else
	{
		CMyStringW s;
		s.LoadString(IDS_BYTE);
		ret += s;
	}
}

void __stdcall FillFileItemInfo(CFTPFileItem* pItem)
{
	SHFILEINFO_UNION sfi;
	DWORD dw;

	if (pItem->iIconIndex != -1)
		return;

	CFTPFileItem* p = pItem;
	while (p->pTargetFile)
		p = p->pTargetFile;
	CMyStringW str(p->strFileName);
	memset(&sfi.w, 0, sizeof(sfi.w));
	dw = GetDummyFileAttribute(pItem);
	if (!::SHGetFileInfoW(str, dw, &sfi.w, sizeof(sfi.w),
		SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_TYPENAME))
	{
		memset(&sfi.a, 0, sizeof(sfi.a));
		if (!::SHGetFileInfoA(str, dw, &sfi.a, sizeof(sfi.a),
			SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_TYPENAME))
			return;
		pItem->iIconIndex = sfi.a.iIcon;
		pItem->strType = sfi.a.szTypeName;
	}
	else
	{
		pItem->iIconIndex = sfi.w.iIcon;
		pItem->strType = sfi.w.szTypeName;
	}
	if (pItem->IsDirectory())
	{
		memset(&sfi.a, 0, sizeof(sfi.a));
		if (!::SHGetFileInfoA(str, dw, &sfi.a, sizeof(sfi.a),
			SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_OPENICON))
			return;
		pItem->iOpenIconIndex = sfi.a.iIcon;
	}
	else
		pItem->iOpenIconIndex = pItem->iIconIndex;
}

static void __stdcall FilePermissionsToString(int nPerm, CMyStringW& ret)
{
	if (!nPerm)
		ret.Empty();
	else
	{
		// S_IFLNK == S_IFCHR | S_IFREG
		if ((nPerm & S_IFLNK) == S_IFLNK)
			ret = L"l";
		// S_IFBLK == S_IFCHR | S_IFDIR
		else if ((nPerm & S_IFBLK) == S_IFBLK)
			ret = L"b";
		else if ((nPerm & S_IFCHR) == S_IFCHR)
			ret = L"c";
		else if ((nPerm & S_IFDIR) == S_IFDIR)
			ret = L"d";
		else
			ret = L"-";
		ret += (nPerm & S_IRUSR) ? L'r' : L'-';
		ret += (nPerm & S_IWUSR) ? L'w' : L'-';
		if (nPerm & S_ISUID)
			ret += (nPerm & S_IXUSR) ? L's' : L'S';
		else
			ret += (nPerm & S_IXUSR) ? L'x' : L'-';
		ret += (nPerm & S_IRGRP) ? L'r' : L'-';
		ret += (nPerm & S_IWGRP) ? L'w' : L'-';
		if (nPerm & S_ISGID)
			ret += (nPerm & S_IXGRP) ? L's' : L'S';
		else
			ret += (nPerm & S_IXGRP) ? L'x' : L'-';
		ret += (nPerm & S_IROTH) ? L'r' : L'-';
		ret += (nPerm & S_IWOTH) ? L'w' : L'-';
		if (nPerm & S_ISVTX)
			ret += (nPerm & S_IXOTH) ? L't' : L'T';
		else
			ret += (nPerm & S_IXOTH) ? L'x' : L'-';
	}
}

static void __stdcall FileAttributesToString(DWORD dwAttribute, CMyStringW& ret)
{
	ret.Empty();
	if (!(dwAttribute & FILE_ATTRIBUTE_DIRECTORY))
	{
		if (dwAttribute & FILE_ATTRIBUTE_ARCHIVE)
			ret += L'A';
		else
			ret += L'-';
		if (dwAttribute & FILE_ATTRIBUTE_SYSTEM)
			ret += L'S';
		else
			ret += L'-';
		if (dwAttribute & FILE_ATTRIBUTE_HIDDEN)
			ret += L'H';
		else
			ret += L'-';
		if (dwAttribute & FILE_ATTRIBUTE_READONLY)
			ret += L'R';
		else
			ret += L'-';
	}
}

HRESULT __stdcall _GetFileItemPropData(CFTPDirectoryBase* pDirectory, CFTPFileItem* p, const PROPERTYKEY& key, VARIANT* pv)
{
	switch (_MyPropertyKeyToStringID(key))
	{
	case IDS_HEAD_NAME:
	case IDS_HEAD_FILE_NAME:
		pv->vt = VT_BSTR;
		pv->bstrVal = ::SysAllocStringLen(p->strFileName, (UINT)p->strFileName.GetLength());
		if (!pv->bstrVal)
			return E_OUTOFMEMORY;
		break;
	case IDS_HEAD_FILE_EXT:
	{
		LPCWSTR lpw = wcsrchr(p->strFileName, L'.');
		if (!lpw)
			pv->vt = VT_EMPTY;
		else
		{
			pv->vt = VT_BSTR;
			pv->bstrVal = ::SysAllocStringLen(lpw, (UINT)wcslen(lpw));
			if (!pv->bstrVal)
				return E_OUTOFMEMORY;
		}
	}
	break;
	case IDS_HEAD_SIZE:
		pv->vt = VT_UI8;
		pv->ullVal = p->uliSize.QuadPart;
		break;
	case IDS_HEAD_TYPE:
		pv->vt = VT_BSTR;
		pv->bstrVal = ::SysAllocStringLen(p->strType, (UINT)p->strType.GetLength());
		if (!pv->bstrVal)
			return E_OUTOFMEMORY;
		break;
	case IDS_HEAD_CREATE_TIME:
	{
		SYSTEMTIME st;
		::FileTimeToSystemTime(&p->ftCreateTime, &st);
		::SystemTimeToVariantTime(&st, &pv->date);
		pv->vt = VT_DATE;
	}
	break;
	case IDS_HEAD_MODIFY_TIME:
	{
		SYSTEMTIME st;
		::FileTimeToSystemTime(&p->ftModifyTime, &st);
		::SystemTimeToVariantTime(&st, &pv->date);
		pv->vt = VT_DATE;
	}
	break;
	case IDS_HEAD_PERMISSIONS:
	{
		CMyStringW str;
		if (p->bWinAttr)
			::FileAttributesToString(p->dwAttributes, str);
		else
			::FilePermissionsToString(p->nUnixMode, str);
		pv->vt = VT_BSTR;
		pv->bstrVal = ::SysAllocStringLen(str, (UINT)str.GetLength());
		if (!pv->bstrVal)
			return E_OUTOFMEMORY;
	}
	break;
	case IDS_HEAD_TRANSFER_TYPE:
	{
		CMyStringW str;
		if (p->IsDirectory())
			str.LoadString(IDS_TYPE_DIRECTORY);
		else if (pDirectory->IsTextFile(p->strFileName) == S_OK)
			str.LoadString(IDS_TYPE_TEXT);
		else
			str.LoadString(IDS_TYPE_BINARY);
		pv->vt = VT_BSTR;
		pv->bstrVal = ::SysAllocStringLen(str, (UINT)str.GetLength());
		if (!pv->bstrVal)
			return E_OUTOFMEMORY;
	}
	break;
	case IDS_HEAD_UID:
	{
		CMyStringW str;
		if (!p->strOwner.IsEmpty())
			str = p->strOwner;
		else
			str.Format(L"%u", p->uUID);
		pv->vt = VT_BSTR;
		pv->bstrVal = ::SysAllocStringLen(str, (UINT)str.GetLength());
		if (!pv->bstrVal)
			return E_OUTOFMEMORY;
	}
	break;
	case IDS_HEAD_GID:
	{
		CMyStringW str;
		if (!p->strGroup.IsEmpty())
			str = p->strGroup;
		else
			str.Format(L"%u", p->uGID);
		pv->vt = VT_BSTR;
		pv->bstrVal = ::SysAllocStringLen(str, (UINT)str.GetLength());
		if (!pv->bstrVal)
			return E_OUTOFMEMORY;
	}
	break;
	default:
		if (IsEqualPropertyKey(key, PKEY_FindData))
		{
			SAFEARRAYBOUND sab;
			sab.cElements = sizeof(WIN32_FIND_DATAW);
			sab.lLbound = 0;
			auto* arr = ::SafeArrayCreate(VT_UI1, 1, &sab);
			if (!arr)
			{
				return E_OUTOFMEMORY;
			}
			WIN32_FIND_DATAW* buff;
			::SafeArrayAccessData(arr, reinterpret_cast<void**>(&buff));

			*buff = {};
			wcsncpy_s(buff->cFileName, p->strFileName, MAX_PATH);
			if (p->bWinAttr)
				buff->dwFileAttributes = p->dwAttributes;
			else
			{
				auto attr = p->nUnixMode;
				buff->dwFileAttributes = (attr & S_IFDIR) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
				if (buff->cFileName[0] == L'.')
					buff->dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN;
				if (attr & S_IFLNK)
					buff->dwFileAttributes |= FILE_ATTRIBUTE_REPARSE_POINT;
				if (attr & (S_IFBLK | S_IFSOCK | S_IFREG))
					buff->dwFileAttributes |= FILE_ATTRIBUTE_DEVICE;
			}
			buff->nFileSizeLow = p->uliSize.LowPart;
			buff->nFileSizeHigh = p->uliSize.HighPart;
			buff->ftLastWriteTime = p->ftModifyTime;
			buff->ftCreationTime = p->ftCreateTime;

			::SafeArrayUnaccessData(arr);
			pv->vt = VT_ARRAY | VT_UI1;
			pv->parray = arr;
			break;
		}
		else if (IsEqualPropertyKey(key, PKEY_PropList_InfoTip) ||
			IsEqualPropertyKey(key, PKEY_PropList_TileInfo) ||
			IsEqualPropertyKey(key, PKEY_PropList_FileOperationPrompt))
		{
			constexpr WCHAR strPropList[] = L"prop:System.ItemTypeText;System.Size;System.DateModified;";
			pv->vt = VT_BSTR;
			// std::extent<...>::value includes null char, so ignore from the length
			pv->bstrVal = ::SysAllocStringLen(strPropList, std::extent<decltype(strPropList)>::value - 1);
			if (!pv->bstrVal)
				return E_OUTOFMEMORY;
			break;
		}
#ifdef _DEBUG
		{
			CMyStringW str, str2;
			MyStringFromGUIDW(key.fmtid, str2);
			str.Format(L"[IPropertyStore::GetValue] unknown scid: %s, %d for file '%s'\n", str2.operator LPCWSTR(),
				key.pid, p->strFileName.operator LPCWSTR());
			OutputDebugStringW(str);
		}
#endif
		return E_INVALIDARG;
	}
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

CFTPDirectoryBase::CFTPDirectoryBase(
	CDelegateMallocData* pMallocData,
	CFTPDirectoryItem* pItemMe,
	CFTPDirectoryBase* pParent,
	CFTPDirectoryRootBase* pRoot,
	LPCWSTR lpszDirectory)
	: CFolderBase(pMallocData)
	, m_pItemMe(pItemMe)
	, m_pParent(pParent)
	, m_pRoot(pRoot)
	, m_strDirectory(lpszDirectory)
{
	pParent->AddRef();
	pRoot->AddRef();
	m_bIsRoot = false;

	CommonConstruct();
}

CFTPDirectoryBase::CFTPDirectoryBase(
	CDelegateMallocData* pMallocData,
	CFTPDirectoryItem* pItemMe)
	: CFolderBase(pMallocData)
	, m_pItemMe(pItemMe)
{
	m_pParent = NULL;
	m_pRoot = NULL;
	m_bIsRoot = true;

	CommonConstruct();
}

void CFTPDirectoryBase::CommonConstruct()
{
	m_uRef = 1;
	m_clsidThis = CLSID_NULL;
	m_grfMode = 0;
	m_grfStateBits = 0;
	m_bDirReceived = false;
	if (m_pItemMe)
	{
		m_pItemMe->AddRef();
		m_pItemMe->pDirectory = this;
	}

	::InitializeCriticalSection(&m_csRefs);
	::InitializeCriticalSection(&m_csFiles);
	CMyStringW str;
	str.Format(L"CFTPDirectoryBase::CommonConstruct() for (0x%p), count = 1\n",
		(void*)this);
	OutputDebugString(str);
}

CFTPDirectoryBase::~CFTPDirectoryBase()
{
	if (m_pParent)
		m_pParent->Release();
	if (m_pItemMe)
	{
		m_pItemMe->pDirectory = NULL;
		m_pItemMe->Release();
	}
	//#ifdef _DEBUG
	//	m_pParent = NULL;
	//	m_pItemMe = NULL;
	//#endif
	register int i = m_aDirectories.GetCount();
	while (i--)
	{
		CFTPDirectoryItem* p = m_aDirectories.GetItem(i);
		m_aDirectories.RemoveItem(i);
		//if (p->pDirectory)
		//	p->pDirectory->Release();
		//delete p;
		p->Release();
	}
	//#ifdef _DEBUG
	//	m_aDirectories.RemoveAll();
	//#endif
	::EnterCriticalSection(&m_csFiles);
	i = m_aFiles.GetCount();
	while (i--)
	{
		CFTPFileItem* pItem = m_aFiles.GetItem(i);
		pItem->Release();
	}
	m_aFiles.RemoveAll();
	::LeaveCriticalSection(&m_csFiles);
	if (!m_bIsRoot)
		m_pRoot->Release();
	//#ifdef _DEBUG
	//	m_pRoot = NULL;
	//#endif

	::DeleteCriticalSection(&m_csFiles);
	::DeleteCriticalSection(&m_csRefs);
}

STDMETHODIMP_(ULONG) CFTPDirectoryBase::AddRef()
{
	::EnterCriticalSection(&m_csRefs);
	//ULONG ret = (ULONG) ::InterlockedIncrement((LONG*) &m_uRef);
	ULONG ret = ++m_uRef;
	::LeaveCriticalSection(&m_csRefs);
	//#ifdef _DEBUG
	//	CMyStringW str;
	//	str.Format(L"CFTPDirectoryBase::AddRef() for '%s' (0x%p), count = %lu\n",
	//		(LPCWSTR) m_strDirectory, (void*) this, ret);
	//	OutputDebugString(str);
	//#endif
	return ret;
	//return (ULONG) ::InterlockedIncrement((LONG*) &m_uRef);
}

STDMETHODIMP_(ULONG) CFTPDirectoryBase::Release()
{
	::EnterCriticalSection(&m_csRefs);
	ULONG ret;
	//ret = (ULONG) ::InterlockedDecrement((LONG*) &m_uRef);
	ret = --m_uRef;
	if (!ret && m_pItemMe)
	{
		m_pItemMe->pDirectory = NULL;
		m_pItemMe->Release();
		m_pItemMe = NULL;
	}
	::LeaveCriticalSection(&m_csRefs);

	//#ifdef _DEBUG
	//	CMyStringW str;
	//	str.Format(L"CFTPDirectoryBase::Release() for '%s' (0x%p), count = %lu\n",
	//		(LPCWSTR) m_strDirectory, (void*) this, ret);
	//	OutputDebugString(str);
	//#endif
	if (!ret)
		delete this;
	return ret;
}

STDMETHODIMP CFTPDirectoryBase::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	if (IsEqualIID(riid, IID_IExtractIconA) || IsEqualIID(riid, IID_IExtractIconW) ||
		IsEqualIID(riid, IID_IDropTarget) || IsEqualIID(riid, IID_IContextMenu))
		return CreateViewObject(NULL, riid, ppv);
	else if (IsEqualIID(riid, IID_IThumbnailHandlerFactory))
	{
		*ppv = (IThumbnailHandlerFactory*)this;
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid, __uuidof(IShellFolderPropertyInformation)))
	{
		*ppv = (IShellFolderPropertyInformation*)this;
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid, IID_IEasySFTPDirectory))
	{
		*ppv = (IEasySFTPDirectory*)this;
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid, IID_IStorage))
	{
		*ppv = (IStorage*)this;
		AddRef();
		return S_OK;
	}
	return CFolderBase::QueryInterface(riid, ppv);
}

STDMETHODIMP CFTPDirectoryBase::EnumObjects(HWND hWnd, SHCONTF grfFlags, IEnumIDList** ppenumIDList)
{
	if (!ppenumIDList)
		return E_POINTER;
	if (!m_bDirReceived)
	{
		if (!(grfFlags & SHCONTF_FASTITEMS))
		{
			m_hWndOwnerCache = hWnd;
			m_pRoot->m_hWndOwnerCache = hWnd;
			if (!m_pRoot->ReceiveDirectory(hWnd, this, m_strDirectory, &m_bDirReceived))
			{
				*ppenumIDList = NULL;
				return S_FALSE;
			}
		}
	}

	*ppenumIDList = new CEnumFTPItemIDList(m_pMallocData, m_aFiles, grfFlags, (IShellFolder*)this);
	return S_OK;
}

STDMETHODIMP CFTPDirectoryBase::BindToObject(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void** ppv)
{
	if (!pidl || !ppv)
		return E_POINTER;
	//if (pbc)
	//	return E_UNEXPECTED;
	if (IsEqualIID(riid, IID_IExtractIconW) || IsEqualIID(riid, IID_IExtractIconA))
		return E_NOINTERFACE;

	if (IsEqualIID(riid, IID_IPropertyStore))
	{
		CFTPDirectoryBase* pDir;
		CFTPFileItem* p = GetFileItem(pidl, &pDir);
		if (!p)
			return E_INVALIDARG;
		auto* pStore = new CFTPFileItemPropertyStore(pDir, p);
		pDir->Release();
		if (!pStore)
			return E_OUTOFMEMORY;
		auto hr = pStore->QueryInterface(riid, ppv);
		pStore->Release();
		return hr;
	}

	if (IsEqualIID(riid, IID_IRelatedItem) || IsEqualIID(riid, IID_IDisplayItem))
	{
		CFTPDirectoryBase* pDir;
		CFTPFileItem* p = GetFileItem(pidl, &pDir);
		if (!p)
			return E_INVALIDARG;
		auto pidlAbsolute = ::AppendItemIDList(m_pidlMe, pidl);
		pDir->Release();
		if (!pidlAbsolute)
			return E_OUTOFMEMORY;
		auto* pItem = new CFTPFileItemDisplayName(pidlAbsolute);
		if (!pItem)
		{
			::CoTaskMemFree(pidlAbsolute);
			return E_OUTOFMEMORY;
		}
		auto hr = pItem->QueryInterface(riid, ppv);
		pItem->Release();
		return hr;
	}

	if (!IsEqualIID(riid, IID_IUnknown) &&
		!IsEqualIID(riid, IID_IShellFolder) &&
		!IsEqualIID(riid, IID_IShellFolder2) &&
		!IsEqualIID(riid, IID_IPersist) &&
		!IsEqualIID(riid, IID_IPersistFolder) &&
		!IsEqualIID(riid, IID_IPersistFolder2) &&
		!IsEqualIID(riid, IID_IPersistIDList) &&
		!IsEqualIID(riid, IID_IObjectWithSite) &&
		!IsEqualIID(riid, IID_IEasySFTPDirectory) &&
		!IsEqualIID(riid, IID_IStorage))
		return E_NOINTERFACE;

	CFTPDirectoryItem* pDirItem = GetAlreadyOpenedDirectory(pidl);
	if (pDirItem)
	{
		HRESULT hr = pDirItem->pDirectory->QueryInterface(riid, ppv);
		pDirItem->Release();
		return hr;
	}
	CMyStringW strRealPath;
	if (!m_pRoot->ValidateDirectory(m_strDirectory, pidl, strRealPath))
		return E_INVALIDARG;
	if (strRealPath.IsEmpty())
		return E_INVALIDARG;

	LPCWSTR lpw = strRealPath;
	CFTPDirectoryBase* pParent;
	if (*lpw == L'/')
	{
		pParent = m_pRoot;
		lpw++;
	}
	else
		pParent = this;
	CFTPDirectoryBase* pRet;
	HRESULT hr = pParent->OpenNewDirectory(lpw, &pRet);
	if (FAILED(hr))
		return hr;
	//{
	//	PIDLIST_ABSOLUTE pidlChild = ::AppendItemIDList(m_pidlMe, pidl);
	//	hr = pRet->Initialize(pidlChild);
	//	::CoTaskMemFree(pidlChild);
	//}
	hr = pRet->QueryInterface(riid, ppv);
	pRet->Release();

	return hr;
}

STDMETHODIMP CFTPDirectoryBase::BindToStorage(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void** ppv)
{
	if (!pidl || !ppv)
		return E_POINTER;
	//if (pbc)
	//	return E_UNEXPECTED;
	if (!IsEqualIID(riid, IID_IUnknown) &&
		!IsEqualIID(riid, IID_IStorage) &&
		!IsEqualIID(riid, IID_ISequentialStream) &&
		!IsEqualIID(riid, IID_IStream))
		return E_NOINTERFACE;

	CFTPDirectoryItem* pDirItem = GetAlreadyOpenedDirectory(pidl);
	if (pDirItem)
	{
		HRESULT hr = pDirItem->pDirectory->QueryInterface(riid, ppv);
		pDirItem->Release();
		return hr;
	}

	CFTPDirectoryBase* pDir;
	CFTPFileItem* p = GetFileItem(pidl, &pDir);
	if (!p)
		return E_INVALIDARG;

	if (p->IsDirectory())
	{
		CFTPDirectoryBase* pChildDir;
		auto hr = pDir->OpenNewDirectory(p->strFileName, &pChildDir);
		pDir->Release();
		if (FAILED(hr))
		{
			return hr;
		}
		return pChildDir->QueryInterface(riid, ppv);
	}
	else
	{
		if (!IsEqualIID(riid, IID_IStream) && !IsEqualIID(riid, IID_ISequentialStream))
		{
			pDir->Release();
			return E_NOINTERFACE;
		}
		IStream* pstm;
		auto hr = pDir->OpenStream(p->strFileName, NULL, STGM_READ, 0, &pstm);
		pDir->Release();
		if (FAILED(hr))
			return hr;
		hr = pstm->QueryInterface(riid, ppv);
		pstm->Release();
		return hr;
	}
}

STDMETHODIMP CFTPDirectoryBase::CompareIDs(LPARAM lParam, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2)
{
	if (!pidl1 || !pidl2)
		return E_POINTER;

	int nColumn;
	SHCOLUMNID scid;
	HRESULT hr;
	if (lParam & SHCIDS_ALLFIELDS)
		nColumn = -1;
	else
	{
		nColumn = (lParam & SHCIDS_COLUMNMASK);
		hr = MapColumnToSCID(nColumn, &scid);
		if (FAILED(hr))
			return hr;
	}

	CMyStringW str1, str2;
	if (!pidl1->mkid.cb)
		return pidl2->mkid.cb ? MAKE_HRESULT(0, 0, (unsigned short)(short)-1) : MAKE_HRESULT(0, 0, 0);
	else if (!pidl2->mkid.cb)
		return MAKE_HRESULT(0, 0, 1);
	if (!PickupFileName((PCUITEMID_CHILD)pidl1, str1))
		return E_INVALIDARG;
	if (!PickupFileName((PCUITEMID_CHILD)pidl2, str2))
		return E_INVALIDARG;

	CFTPFileItem* pItem1;
	CFTPFileItem* pItem2;
	int r = 0;
	UINT uID = _MyPropertyKeyToStringID(scid);
	//if (nColumn == -1 || scid.pid != PID_FTPITEM_FILE_NAME)
	if (nColumn == -1 || (uID != IDS_HEAD_NAME && uID != IDS_HEAD_FILE_NAME && uID != IDS_HEAD_FILE_EXT))
	{
		if (!m_bDirReceived)
		{
			if (!m_pRoot->ReceiveDirectory(m_hWndOwnerCache, this, m_strDirectory, &m_bDirReceived))
				return E_UNEXPECTED;
		}
		pItem1 = GetFileItem(str1);
		pItem2 = GetFileItem(str2);
		if (!pItem1 || !pItem2)
			return E_INVALIDARG;
	}
	else
	{
		if (m_bDirReceived)
		{
			pItem1 = GetFileItem(str1);
			pItem2 = GetFileItem(str2);
			//if (!pItem1 || !pItem2)
			//	return E_INVALIDARG;
		}
		else
			pItem1 = pItem2 = NULL;
	}
	if (pItem1 && pItem2)
	{
		bool bDir1 = pItem1->IsDirectory();
		bool bDir2 = pItem2->IsDirectory();
		if (bDir1 != bDir2)
		{
			if (bDir1)
				r = -1;
			//else if (IsShortcutItem(pItem1) && pItem1->IsDirectory())
			//	return pItem2->type == fitypeDir ? 1 : -1;
			else //if (bDir2)
				r = 1;
		}
	}

	if (r == 0)
	{
		if (nColumn == -1)
			//scid.pid = PID_FTPITEM_FILE_NAME;
			uID = IDS_HEAD_NAME;
		while (true)
		{
			switch (uID)
			{
			case IDS_HEAD_NAME:
			case IDS_HEAD_FILE_NAME:
				if (pItem1 && pItem2)
					r = pItem1->strFileName.Compare(pItem2->strFileName);
				else
					r = str1.Compare(str2);
				break;
			case IDS_HEAD_FILE_EXT:
			{
				LPCWSTR lpw1, lpw2;
				if (pItem1 && pItem2)
				{
					lpw1 = pItem1->strFileName.IsEmpty() ? NULL : wcsrchr(pItem1->strFileName, L'.');
					lpw2 = pItem2->strFileName.IsEmpty() ? NULL : wcsrchr(pItem2->strFileName, L'.');
				}
				else
				{
					lpw1 = str1.IsEmpty() ? NULL : wcsrchr(str1, L'.');
					lpw2 = str2.IsEmpty() ? NULL : wcsrchr(str2, L'.');
				}
				if (!lpw1)
					r = (lpw2 ? -1 : 0);
				else if (!lpw2)
					r = 1;
				else
					r = wcscmp(lpw1, lpw2);
			}
			break;
			case IDS_HEAD_SIZE:
				if (pItem1->uliSize.QuadPart < pItem2->uliSize.QuadPart)
					r = -1;
				else if (pItem1->uliSize.QuadPart > pItem2->uliSize.QuadPart)
					r = 1;
				else
					r = 0;
				break;
			case IDS_HEAD_TYPE:
				FillFileItemInfo(pItem1);
				FillFileItemInfo(pItem2);
				r = pItem1->strType.Compare(pItem2->strType);
				break;
			case IDS_HEAD_CREATE_TIME:
				r = ::CompareFileTime(&pItem1->ftCreateTime, &pItem2->ftCreateTime);
				break;
			case IDS_HEAD_MODIFY_TIME:
				r = ::CompareFileTime(&pItem1->ftModifyTime, &pItem2->ftModifyTime);
				break;
			case IDS_HEAD_PERMISSIONS:
				r = 0;
				break;
			case IDS_HEAD_UID:
				if (pItem1->strOwner.IsEmpty() && pItem1->uUID)
					str1.Format(L"%u", pItem1->uUID);
				else
					str1 = pItem1->strOwner;
				if (pItem2->strOwner.IsEmpty() && pItem2->uUID)
					str2.Format(L"%u", pItem2->uUID);
				else
					str2 = pItem2->strOwner;
				r = str1.Compare(str2);
				break;
			case IDS_HEAD_GID:
				if (pItem1->strGroup.IsEmpty() && pItem1->uGID)
					str1.Format(L"%u", pItem1->uGID);
				else
					str1 = pItem1->strGroup;
				if (pItem2->strGroup.IsEmpty() && pItem2->uGID)
					str2.Format(L"%u", pItem2->uGID);
				else
					str2 = pItem2->strGroup;
				r = str1.Compare(str2);
				break;
			case IDS_HEAD_TRANSFER_TYPE:
				if (pItem1->IsDirectory())
					r = (pItem2->IsDirectory() ? 0 : -1);
				else if (pItem2->IsDirectory())
					r = 1;
				else if (IsTextFile(pItem1->strFileName))
					r = (IsTextFile(pItem2->strFileName) ? 0 : -1);
				else if (IsTextFile(pItem2->strFileName))
					r = 1;
				else
					r = 0;
				break;
			}
			if (nColumn == -1)
			{
				if (r != 0)
					break;
				uID++;
				if (uID > IDS_HEAD_GID)
					break;
			}
			else
			{
				if (r != 0 || uID == IDS_HEAD_NAME)
					break;
				uID = IDS_HEAD_NAME;
			}
		}
	}

	if (r != 0)
	{
		r = (r > 0 ? 1 : -1);
		// we must cast to 'unsigned short'
		return MAKE_HRESULT(0, 0, (unsigned short)(short)r);
	}

	if (!((PCUIDLIST_RELATIVE)(((DWORD_PTR)pidl1) + pidl1->mkid.cb))->mkid.cb)
	{
		return (!((PCUIDLIST_RELATIVE)(((DWORD_PTR)pidl1) + pidl1->mkid.cb))->mkid.cb) ?
			MAKE_HRESULT(0, 0, 0) : MAKE_HRESULT(0, 0, (unsigned short)(short)-1);
	}
	else if (!((PCUIDLIST_RELATIVE)(((DWORD_PTR)pidl1) + pidl1->mkid.cb))->mkid.cb)
		return MAKE_HRESULT(0, 0, 1);

	IShellFolder* pChild;
	hr = BindToObject(pidl1, NULL, IID_IShellFolder, (void**)&pChild);
	if (FAILED(hr))
		return hr;
	pidl1 = (PCUIDLIST_RELATIVE)(((DWORD_PTR)pidl1) + pidl1->mkid.cb);
	pidl2 = (PCUIDLIST_RELATIVE)(((DWORD_PTR)pidl2) + pidl2->mkid.cb);
	hr = pChild->CompareIDs(lParam, pidl1, pidl2);
	pChild->Release();
	return hr;
}

STDMETHODIMP CFTPDirectoryBase::CreateViewObject(HWND hWndOwner, REFIID riid, void** ppv)
{
	if (IsEqualIID(riid, IID_IExtractIconA) || IsEqualIID(riid, IID_IExtractIconW))
	{
		CFTPFileItem* pItem = new CFTPFileItem();
		pItem->bWinAttr = false;
		pItem->nUnixMode = S_IFDIR;
		pItem->strFileName = m_pItemMe->strName;
		pItem->iIconIndex = -1;
		CFTPFileItemIcon* pIcon = new CFTPFileItemIcon(pItem);
		HRESULT hr = pIcon->QueryInterface(riid, ppv);
		pIcon->Release();
		pItem->Release();
		return hr;
	}
	else if (IsEqualIID(riid, IID_IDropTarget))
	{
		CFTPDropHandler* pDrop = new CFTPDropHandler(this, hWndOwner);
		*ppv = (IDropTarget*)pDrop;
		return S_OK;
	}
	else if (IsEqualIID(riid, IID_IContextMenu))
	{
		IShellBrowser* pBrowser = GetShellBrowser(hWndOwner);
		CFTPFileDirectoryMenu* pMenu = new CFTPFileDirectoryMenu(this, m_pidlMe, pBrowser);
		if (pBrowser)
			pBrowser->Release();
		*ppv = (IContextMenu*)pMenu;
		return S_OK;
	}
	return CFolderBase::CreateViewObject(hWndOwner, riid, ppv);
}

STDMETHODIMP CFTPDirectoryBase::GetAttributesOf(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, SFGAOF* rgfInOut)
{
	if (!rgfInOut)
		return E_POINTER;
	if (!cidl)
	{
		*rgfInOut = 0;
		return S_OK;
	}
	if (!apidl)
		return E_POINTER;
	SFGAOF ret, f, req;
	req = *rgfInOut;
	if (req & SFGAO_FILESYSTEM)
	{
		*rgfInOut = (req &= ~SFGAO_FILESYSTEM);
		if (!req)
			return S_OK;
	}
	CMyStringW str;
	for (UINT u = 0; u < cidl; u++)
	{
		if (!::PickupFileName(apidl[u], str))
			return E_INVALIDARG;
		f = SFGAO_CANCOPY | SFGAO_CANMOVE | SFGAO_CANDELETE | SFGAO_CANRENAME | SFGAO_HASPROPSHEET;
		if ((req & (SFGAO_BROWSABLE | SFGAO_FOLDER | SFGAO_DROPTARGET | SFGAO_HASSUBFOLDER | SFGAO_HIDDEN | SFGAO_LINK | SFGAO_HASSTORAGE | SFGAO_STREAM)) != 0)
		{
			CFTPFileItem* p;
			//CSFTPFileItem UNALIGNED* pData = (CSFTPFileItem UNALIGNED*) apidl[u];
			//if (pData->bHasAttribute)
			//{
			//	if (pData->bIsDirectory)
			//		f |= SFGAO_BROWSABLE | SFGAO_FOLDER | SFGAO_DROPTARGET | SFGAO_HASSUBFOLDER;
			//	if (pData->bIsHidden)
			//		f |= SFGAO_HIDDEN;
			//	if (pData->bIsShortcut)
			//		f |= SFGAO_LINK;
			//}
			//else

			CSFTPFileItem UNALIGNED* pFItem = (CSFTPFileItem UNALIGNED*) apidl[u];
			if ((req & ~(SFGAO_BROWSABLE | SFGAO_FOLDER | SFGAO_DROPTARGET | SFGAO_HASSUBFOLDER | SFGAO_HASSTORAGE | SFGAO_STREAM | (pFItem->bHasAttribute ? (SFGAO_HIDDEN | SFGAO_LINK) : 0))) == 0)
			{
				// fast retrieve
				if (pFItem->bIsDirectory)
					f |= SFGAO_BROWSABLE | SFGAO_FOLDER | SFGAO_DROPTARGET | SFGAO_HASSUBFOLDER | SFGAO_HASSTORAGE;
				else
					f |= SFGAO_STREAM;
				if (pFItem->bHasAttribute)
				{
					if (pFItem->bIsHidden)
						f |= SFGAO_HIDDEN;
					if (pFItem->bIsShortcut)
						f |= SFGAO_LINK;
				}
			}
			else
			{
				if (!m_bDirReceived)
				{
					if (!m_hWndOwnerCache)
						m_hWndOwnerCache = m_pRoot->m_hWndOwnerCache;
					if (!m_pRoot->ReceiveDirectory(m_hWndOwnerCache, this, m_strDirectory, &m_bDirReceived))
						return E_UNEXPECTED;
				}
				p = GetFileItem(str);
				if (!p)
					return E_INVALIDARG;
				if (p->IsDirectory())
					f |= SFGAO_BROWSABLE | SFGAO_FOLDER | SFGAO_DROPTARGET | SFGAO_HASSUBFOLDER | SFGAO_HASSTORAGE;
				else
					f |= SFGAO_STREAM;
				if (p->IsHidden())
					f |= SFGAO_HIDDEN;
				if (p->IsShortcut())
					f |= SFGAO_LINK;
			}
		}
		if (!u)
			ret = f;
		else
			ret &= f;
	}
	*rgfInOut &= ret;
	return S_OK;
}

STDMETHODIMP CFTPDirectoryBase::GetUIObjectOf(HWND hWndOwner, UINT cidl, PCUITEMID_CHILD_ARRAY apidl,
	REFIID riid, UINT* rgfReserved, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	// NOTE: we must set NULL to *ppv, or causes 0xC000041D exception in Windows 7 (x64)
	*ppv = NULL;
	if (!cidl)
		return E_INVALIDARG;
	if (!apidl)
		return E_POINTER;

	bool bIsIcon;
	if (IsEqualIID(riid, IID_IExtractIconA) ||
		IsEqualIID(riid, IID_IExtractIconW))
		bIsIcon = true;
	else
	{
		bIsIcon = false;
		if (!IsEqualIID(riid, IID_IDataObject) &&
			!IsEqualIID(riid, IID_IContextMenu) &&
			!IsEqualIID(riid, IID_IDropTarget) &&
			!IsEqualIID(riid, IID_IThumbnailProvider))
		{
#ifdef _DEBUG
			//if (!IsEqualIID(riid, IID_IQueryInfo))
			{
				CMyStringW str;
				str.Format(L"CFTPDirectoryBase::GetUIObjectOf: unknown interface: {%08lX-%04hX-%04hX-%02X%02X-%02X%02X%02X%02X%02X%02X}\n",
					riid.Data1, riid.Data2, riid.Data3, (UINT)riid.Data4[0], (UINT)riid.Data4[1],
					(UINT)riid.Data4[2], (UINT)riid.Data4[3], (UINT)riid.Data4[4], (UINT)riid.Data4[5],
					(UINT)riid.Data4[6], (UINT)riid.Data4[7]);
				OutputDebugString(str);
			}
#endif
			return E_NOINTERFACE;
		}
	}

	CMyPtrArrayT<CFTPFileItem> aItems;
	CMyStringW str;
	for (UINT u = 0; u < cidl; u++)
	{
		if (!::PickupFileName(apidl[u], str))
			return E_INVALIDARG;
		if (!m_bDirReceived && !bIsIcon)
		{
			m_hWndOwnerCache = hWndOwner;
			m_pRoot->m_hWndOwnerCache = hWndOwner;
			if (!m_pRoot->ReceiveDirectory(hWndOwner, this, m_strDirectory, &m_bDirReceived))
				return E_UNEXPECTED;
		}
		if (m_bDirReceived)
		{
			CFTPFileItem* p = GetFileItem(str);
			if (!p)
				return E_INVALIDARG;
			aItems.Add(p);
		}
	}

	HRESULT hr = E_NOINTERFACE;

	if (IsEqualIID(riid, IID_IExtractIconA) || IsEqualIID(riid, IID_IExtractIconW))
	{
		if (cidl != 1)
			return E_INVALIDARG;
		CFTPFileItemIcon* pIcon;
		if (m_bDirReceived)
			pIcon = new CFTPFileItemIcon(aItems.GetItem(0));
		else
		{
			CSFTPFileItem UNALIGNED* pFItem = (CSFTPFileItem UNALIGNED*) apidl[0];
			str = pFItem->wchFileName;
			pIcon = new CFTPFileItemIcon(str, pFItem->bIsDirectory != 0);
		}
		hr = pIcon->QueryInterface(riid, ppv);
		pIcon->Release();
	}
	else if (IsEqualIID(riid, IID_IDropTarget))
	{
		if (aItems.GetCount() != 1)
			return E_INVALIDARG;
		if (!aItems.GetItem(0)->IsDirectory())
			return E_INVALIDARG;
		CFTPDirectoryBase* pDir;
		hr = OpenNewDirectory(aItems.GetItem(0)->strFileName, &pDir);
		if (SUCCEEDED(hr))
		{
			hr = pDir->CreateViewObject(hWndOwner, IID_IDropTarget, ppv);
			pDir->Release();
		}
	}
	else if (IsEqualIID(riid, IID_IContextMenu))
	{
		IShellBrowser* pBrowser = GetShellBrowser(hWndOwner);
		CFTPFileItemMenu* pMenu = new CFTPFileItemMenu(this, m_pidlMe, pBrowser, aItems);
		if (pBrowser)
			pBrowser->Release();
		hr = pMenu->QueryInterface(riid, ppv);
		pMenu->Release();
	}
	else if (IsEqualIID(riid, IID_IThumbnailProvider))
	{
		if (!m_pRoot->m_bUseThumbnailPreview)
			return E_NOINTERFACE;
		if (aItems.GetCount() != 1)
			return E_INVALIDARG;
		if (aItems.GetItem(0)->IsDirectory())
			return E_INVALIDARG;
		IThumbnailProvider* pProvider;
		hr = MyCreateThumbnailProviderFromFileName(aItems.GetItem(0)->strFileName, &pProvider);
		if (FAILED(hr))
			return hr;
		IInitializeWithStream* pInitStream;
		hr = pProvider->QueryInterface(IID_IInitializeWithStream, (void**)&pInitStream);
		if (SUCCEEDED(hr))
		{
			IStream* pStream;
			hr = CreateStream(aItems.GetItem(0), &pStream);
			if (SUCCEEDED(hr))
			{
				hr = pInitStream->Initialize(pStream, STGM_READ);
				if (SUCCEEDED(hr))
					hr = pProvider->QueryInterface(riid, ppv);
				pStream->Release();
			}
			pInitStream->Release();
		}
		pProvider->Release();
		return hr;
	}

	if (SUCCEEDED(hr))
		return hr;

	CFTPDataObject* pObj;
	AddRef();
	hr = m_pRoot->GetFTPItemUIObjectOf(hWndOwner, this, aItems, &pObj);
	Release();
	if (SUCCEEDED(hr))
	{
		hr = pObj->QueryInterface(riid, ppv);
		pObj->Release();
		//if (SUCCEEDED(hr))
		//	return hr;
	}

	return hr;
}

STDMETHODIMP CFTPDirectoryBase::GetDisplayNameOf(PCUITEMID_CHILD pidl, SHGDNF uFlags, STRRET* pName)
{
	CMyStringW str;
	CFTPFileItem* p;
	if (pidl)
	{
		if (!::PickupFileName(pidl, str))
			return E_INVALIDARG;
		if (m_bDirReceived)
		{
			p = GetFileItem(str);
			if (!p)
				return E_INVALIDARG;
		}
		else
			p = NULL;
	}
	else
		p = NULL;
	str.Empty();
	switch (uFlags & 0x0FFF)
	{
	case SHGDN_NORMAL:
		//if (pidl && !p)
		//	return E_INVALIDARG;
		if (uFlags & (SHGDN_FORPARSING | SHGDN_FORADDRESSBAR))
		{
			CMyStringW str2;
			int nDefPort;
			str = m_pRoot->GetProtocolName(nDefPort);
			str += L"://";
			str += m_pRoot->m_strHostName;
			if (m_pRoot->m_nPort != nDefPort)
			{
				str2.Format(L"%d", m_pRoot->m_nPort);
				str += L':';
				str += str2;
			}
			//str2.Empty();
			//CFTPDirectoryBase* pd = this;
			//while (pd)
			//{
			//	if (pd == (CFTPDirectoryBase*) m_pRoot)
			//		break;
			//	str2.InsertString(pd->m_strDirectory, 0);
			//	str2.InsertChar(L'/', 0);
			//	pd = pd->m_pParent;
			//}
			//str += str2;
			str += m_strDirectory;
			if (((LPCWSTR)m_strDirectory)[m_strDirectory.GetLength() - 1] != L'/')
				str += L'/';
			bool bLastIsNotDelimiter = false;
			if (p)
			{
				str += p->strFileName;
				bLastIsNotDelimiter = true;
			}
			else if (pidl)
			{
				if (!::PickupFileName(pidl, str2))
					return E_INVALIDARG;
				str += str2;
				bLastIsNotDelimiter = true;
			}
			else
				break;
			while (true)
			{
				pidl = (PCUITEMID_CHILD)(((DWORD_PTR)pidl) + pidl->mkid.cb);
				if (!pidl->mkid.cb)
					break;
				if (!::PickupFileName(pidl, str2))
					return E_INVALIDARG;
				if (bLastIsNotDelimiter)
					str += L'/';
				else
					bLastIsNotDelimiter = true;
				str += str2;
			}
			break;
		}
		//else
		//{
		//	//CMyStringW str2;
		//	//str2.Empty();
		//	//CFTPDirectoryBase* pd = this;
		//	//while (pd)
		//	//{
		//	//	if (pd == (CFTPDirectoryBase*) m_pRoot)
		//	//		break;
		//	//	str2.InsertString(pd->m_strDirectory, 0);
		//	//	str2.InsertChar(L'/', 0);
		//	//	pd = pd->m_pParent;
		//	//}
		//	//str += str2;
		//	//str += L'/';
		//	str = m_strDirectory;
		//	if (p)
		//	{
		//		if (((LPCWSTR) m_strDirectory)[m_strDirectory.GetLength() - 1] != L'/')
		//			str += L'/';
		//		str += p->strFileName;
		//	}
		//}
		//break;
	case SHGDN_INFOLDER:
		if (!pidl)
		{
			if (this == (CFTPDirectoryBase*)m_pRoot)
				str = m_pRoot->m_strHostName;
			else
				str = m_strDirectory;
		}
		else if (p)
			str = p->strFileName;
		else if (!::PickupFileName(pidl, str))
			return E_INVALIDARG;
		break;
	default:
		return E_NOTIMPL;
	}

	SIZE_T nSize = sizeof(WCHAR) * (str.GetLength() + 1);
	pName->uType = STRRET_WSTR;
	pName->pOleStr = (LPWSTR) ::CoTaskMemAlloc(nSize);
	memcpy(pName->pOleStr, (LPCWSTR)str, nSize);
	return S_OK;
}

STDMETHODIMP CFTPDirectoryBase::SetNameOf(HWND hWnd, PCUITEMID_CHILD pidl, LPCOLESTR pszName, SHGDNF uFlags, PITEMID_CHILD* ppidlOut)
{
	if (!pszName)
		return E_POINTER;
	if (!pidl)
		return E_INVALIDARG;

	CMyStringW str;
	CFTPFileItem* p;
	if (!::PickupFileName(pidl, str))
	{
		if (ppidlOut)
			*ppidlOut = NULL;
		return E_INVALIDARG;
	}
	p = GetFileItem(str);
	if (!p)
	{
		if (ppidlOut)
			*ppidlOut = NULL;
		return E_INVALIDARG;
	}

	AddRef();
	HRESULT hr = m_pRoot->SetFTPItemNameOf(hWnd, this, p, pszName, uFlags);
	if (SUCCEEDED(hr))
	{
		UpdateRenameFile(p->strFileName, pszName, p->IsDirectory());
		PITEMID_CHILD pidlNewChild;
		p->strFileName = pszName;
		pidlNewChild = ::CreateFileItem(m_pMallocData->pMalloc, p);
		if (pidlNewChild)
		{
			//NotifyUpdate(p->IsDirectory() ? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM, pidl, pidlNewChild);
			if (ppidlOut)
				*ppidlOut = pidlNewChild;
		}
	}
	else
	{
		if (ppidlOut)
			*ppidlOut = NULL;
	}
	Release();

	return hr;
}

STDMETHODIMP CFTPDirectoryBase::GetDefaultSearchGUID(GUID* pguid)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFTPDirectoryBase::EnumSearches(IEnumExtraSearch** ppenum)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFTPDirectoryBase::GetDefaultColumn(DWORD dwRes, ULONG* pSort, ULONG* pDisplay)
{
	if (!pSort || !pDisplay)
		return E_POINTER;
	*pSort = *pDisplay = 0;
	return S_OK;
}

STDMETHODIMP CFTPDirectoryBase::GetDefaultColumnState(UINT iColumn, SHCOLSTATEF* pcsFlags)
{
	if (!pcsFlags)
		return E_POINTER;

	SHCOLUMNID scid;
	HRESULT hr = MapColumnToSCID(iColumn, &scid);
	if (FAILED(hr))
		return hr;

	switch (_MyPropertyKeyToStringID(scid))
	{
	case IDS_HEAD_NAME:
	case IDS_HEAD_TYPE:
	case IDS_HEAD_PERMISSIONS:
	case IDS_HEAD_TRANSFER_TYPE:
		*pcsFlags = SHCOLSTATE_TYPE_STR | SHCOLSTATE_ONBYDEFAULT;
		break;
	case IDS_HEAD_SIZE:
		*pcsFlags = SHCOLSTATE_TYPE_INT | SHCOLSTATE_ONBYDEFAULT;
		break;
	case IDS_HEAD_MODIFY_TIME:
		*pcsFlags = SHCOLSTATE_TYPE_DATE | SHCOLSTATE_ONBYDEFAULT;
		break;
	case IDS_HEAD_FILE_NAME:
	case IDS_HEAD_FILE_EXT:
	case IDS_HEAD_GID:
	case IDS_HEAD_UID:
		*pcsFlags = SHCOLSTATE_TYPE_STR;
		break;
	case IDS_HEAD_CREATE_TIME:
		*pcsFlags = SHCOLSTATE_TYPE_DATE;
		break;
	default:
		return E_INVALIDARG;
	}

	return S_OK;
}

STDMETHODIMP CFTPDirectoryBase::GetDetailsEx(PCUITEMID_CHILD pidl, const SHCOLUMNID* pscid, VARIANT* pv)
{
	//if (!IsEqualGUID(pscid->fmtid, GUID_FTPItemColumn))
	//	return E_INVALIDARG;

	CMyStringW str;
	if (!::PickupFileName(pidl, str))
		return E_INVALIDARG;
	CFTPFileItem* p = GetFileItem(str);
	if (!p)
		return E_INVALIDARG;

	return _GetFileItemPropData(this, p, *pscid, pv);
}

STDMETHODIMP CFTPDirectoryBase::GetDetailsOf(PCUITEMID_CHILD pidl, UINT iColumn, SHELLDETAILS* psd)
{
	if (!psd)
		return E_POINTER;

	SHCOLUMNID scid;
	HRESULT hr = MapColumnToSCID(iColumn, &scid);
	if (FAILED(hr))
		return hr;

	UINT uID = _MyPropertyKeyToStringID(scid);
	switch (uID)
	{
	case IDS_HEAD_NAME:
	case IDS_HEAD_FILE_NAME:
		psd->fmt = LVCFMT_LEFT;
		break;
	case IDS_HEAD_FILE_EXT:
		psd->fmt = LVCFMT_LEFT;
		psd->cxChar = 10;
		break;
	case IDS_HEAD_SIZE:
		psd->fmt = LVCFMT_RIGHT;
		psd->cxChar = 10;
		break;
	case IDS_HEAD_TYPE:
		psd->fmt = LVCFMT_LEFT;
		break;
	case IDS_HEAD_CREATE_TIME:
	case IDS_HEAD_MODIFY_TIME:
		psd->fmt = LVCFMT_LEFT;
		psd->cxChar = 20;
		break;
	case IDS_HEAD_PERMISSIONS:
		psd->fmt = LVCFMT_LEFT;
		psd->cxChar = 10;
		break;
	case IDS_HEAD_TRANSFER_TYPE:
		psd->fmt = LVCFMT_LEFT;
		psd->cxChar = 12;
		break;
	case IDS_HEAD_UID:
	case IDS_HEAD_GID:
		psd->fmt = LVCFMT_LEFT;
		psd->cxChar = 10;
		break;
	default:
		return E_INVALIDARG;
	}

	CMyStringW str;
	if (!pidl)
		str.LoadString(uID);
	else
	{
		VARIANT v;
		hr = GetDetailsEx(pidl, &scid, &v);
		if (FAILED(hr))
			return hr;
		if (uID == IDS_HEAD_SIZE)
			::FileSizeToString(*((ULARGE_INTEGER*)&v.ullVal), str);
		else
		{
			switch (v.vt)
			{
			case VT_EMPTY:
				str.Empty();
				break;
			case VT_BSTR:
				str = v.bstrVal;
				break;
			case VT_DATE:
			{
				SYSTEMTIME st;
				::VariantTimeToSystemTime(v.date, &st);
				str.Format(L"%04hu/%02hu/%02hu %02hu:%02hu:%02hu",
					st.wYear, st.wMonth, st.wDay,
					st.wHour, st.wMinute, st.wSecond);
			}
			break;
			default:
#ifdef _DEBUG
			{
				str.Format(L"We must handle vt type = %d\n", (int)v.vt);
				OutputDebugString(str);
			}
#endif
			str.Empty();
			break;
			}
		}
		::VariantClear(&v);
	}
	psd->str.uType = STRRET_WSTR;
	psd->str.pOleStr = DuplicateCoMemString(str);
	if (!psd->str.pOleStr)
		return E_OUTOFMEMORY;

	return S_OK;
}

STDMETHODIMP CFTPDirectoryBase::MapColumnToSCID(UINT iColumn, SHCOLUMNID* pscid)
{
	if (!pscid)
		return E_POINTER;
	if (iColumn >= static_cast<UINT>(_GetAvailablePropKeyCount()))
		return E_FAIL;
	*pscid = s_columnIDMap[iColumn].key;
	return S_OK;
}


STDMETHODIMP CFTPDirectoryBase::GetIconOf(PCUITEMID_CHILD pidl, UINT flags, int* pIconIndex)
{
	if (!pIconIndex)
		return E_POINTER;

	CMyStringW str;
	if (!::PickupFileName(pidl, str))
		return E_INVALIDARG;
	if (!m_bDirReceived)
	{
		//CSFTPFileItem UNALIGNED* pf = (CSFTPFileItem UNALIGNED*) pidl;
		//if (pf->bHasAttribute && pf->bIsDirectory)
		//{
		//	SHFILEINFO_UNION sfi;
		//	DWORD dw;
		//	UINT u;
		//	memset(&sfi.w, 0, sizeof(sfi.w));
		//	dw = FILE_ATTRIBUTE_DIRECTORY;
		//	u = SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_LARGEICON;
		//	if (flags & GIL_OPENICON)
		//		u |= SHGFI_OPENICON;
		//	if (pf->bIsHidden)
		//		dw |= FILE_ATTRIBUTE_HIDDEN;
		//	if (!::SHGetFileInfoW(str, dw, &sfi.w, sizeof(sfi.w), u))
		//	{
		//		memset(&sfi.a, 0, sizeof(sfi.a));
		//		if (!::SHGetFileInfoA(str, dw, &sfi.a, sizeof(sfi.a), u))
		//			return E_FAIL;
		//		*pIconIndex = sfi.a.iIcon;
		//	}
		//	else
		//	{
		//		*pIconIndex = sfi.w.iIcon;
		//	}
		//	return S_OK;
		//}
		return S_FALSE;
	}
	CFTPFileItem* p = GetFileItem(str);
	if (!p)
		return E_INVALIDARG;
	if (p->iIconIndex == -1)
		FillFileItemInfo(p);
	*pIconIndex = (flags & GIL_OPENICON) ? p->iOpenIconIndex : p->iIconIndex;
	return S_OK;
}


STDMETHODIMP CFTPDirectoryBase::MessageSFVCB(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case 17:   // SFVM_LISTREFRESHED
		if (wParam)
		{
			::EnterCriticalSection(&m_csFiles);
			int i;
			i = m_aFiles.GetCount();
			while (i--)
				m_aFiles.GetItem(i)->Release();
			m_aFiles.RemoveAll();
			m_bDirReceived = false;
			::LeaveCriticalSection(&m_csFiles);
		}
		return S_OK;
	case SFVM_MERGEMENU:
	{
		LPQCMINFO lpqi = (LPQCMINFO)lParam;
		HMENU hMenuToAdd = GetSubMenuByID(lpqi->hmenu, FCIDM_MENU_HELP);
		if (hMenuToAdd)
		{
			int i, nCount;
			UINT indexMenu;
			MENUITEMINFO mii;
			UINT uMaxID;
			CMyStringW str;

#ifdef _WIN64
			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_ID | MIIM_STATE;
#else
			mii.cbSize = MENUITEMINFO_SIZE_V1;
			mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
#endif
			uMaxID = lpqi->idCmdFirst;
			//indexMenu = lpqi->indexMenu;
			indexMenu = 0;

			HMENU h = ::GetSubMenu(theApp.m_hMenuContext, CXMENU_POPUP_ROOTHELP);
			nCount = ::GetMenuItemCount(h);
			for (i = 0; i < nCount; i++)
			{
				mii.cch = MAX_PATH;
#ifdef _UNICODE
				mii.dwTypeData = str.GetBufferW(MAX_PATH);
#else
				mii.dwTypeData = str.GetBufferA(MAX_PATH);
#endif
				::GetMenuItemInfo(h, (UINT)i, TRUE, &mii);
				mii.wID = (WORD)((UINT)mii.wID - ID_ROOT_IDBASE + lpqi->idCmdFirst);
				if (uMaxID < (UINT)mii.wID)
					uMaxID = (UINT)mii.wID;
				::InsertMenuItem(hMenuToAdd, indexMenu++, TRUE, &mii);
			}
			//mii.fMask = MIIM_TYPE;
			//mii.fType = MFT_SEPARATOR;
			//::InsertMenuItem(hMenuToAdd, indexMenu++, TRUE, &mii);
			lpqi->idCmdFirst = uMaxID - lpqi->idCmdFirst + 1;
	}
		return S_OK;
	}
	case SFVM_INVOKECOMMAND:
	{
		UINT uID = ((UINT)wParam) + ID_ROOT_IDBASE;
		switch (uID)
		{
		case ID_ROOT_SERVER_INFO:
		{
			m_pRoot->ShowServerInfoDialog(m_pRoot->m_hWndOwnerCache);
		}
		return S_OK;
		}
	}
	break;
}
	return CFolderBase::MessageSFVCB(uMsg, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFTPDirectoryBase::GetThumbnailHandler(PCUITEMID_CHILD pidl, LPBC pbc, REFIID riid, void** ppv)
{
	return GetUIObjectOf(NULL, 1, &pidl, riid, NULL, ppv);
}

///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFTPDirectoryBase::IsFastProperty(PCUITEMID_CHILD pidlChild, REFPROPERTYKEY pkey)
{
	(void)pidlChild;
	return _MyPropertyKeyToStringID(pkey) != 0 ? S_OK : S_FALSE;
}

// in propsys.dll
typedef HRESULT(STDMETHODCALLTYPE* T_PSCreatePropertyKeyStore)(PROPERTYKEY*, int, REFIID riid, void** ppv);

STDMETHODIMP CFTPDirectoryBase::GetFastProperties(PCUITEMID_CHILD pidlChild, REFIID riid, void** ppv)
{
	(void)pidlChild;
	auto hModule = ::GetModuleHandleA("propsys.dll");
	if (!hModule)
		return E_NOTIMPL;
	auto pfnPSCreatePropertyKeyStore = (T_PSCreatePropertyKeyStore) ::GetProcAddress(hModule, "PSCreatePropertyKeyStore");
	if (!pfnPSCreatePropertyKeyStore)
		return E_NOTIMPL;

	IPropertyKeyStore* pStore;
	auto hr = pfnPSCreatePropertyKeyStore(NULL, 0, __uuidof(IPropertyKeyStore), reinterpret_cast<void**>(&pStore));
	if (FAILED(hr))
		return hr;
	for (int i = 0; i < _GetAvailablePropKeyCount(); i++)
	{
		hr = pStore->AppendKey(s_columnIDMap[i].key);
		if (FAILED(hr))
		{
			pStore->Release();
			return hr;
		}
	}
	hr = pStore->QueryInterface(riid, ppv);
	pStore->Release();
	return hr;
}

///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFTPDirectoryBase::CreateStream(const OLECHAR* pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStream** ppstm)
{
	auto* pStream = new CFTPFileStream(m_pRoot);
	if (!pStream)
		return E_OUTOFMEMORY;
	HANDLE handle;
	auto hr = m_pRoot->OpenFile(this, pwcsName, grfMode, &handle);
	if (FAILED(hr))
	{
		delete pStream;
		return hr;
	}
	pStream->SetHandle(handle);
	*ppstm = pStream;
	return S_OK;
}

STDMETHODIMP CFTPDirectoryBase::OpenStream(const OLECHAR* pwcsName, void* reserved1, DWORD grfMode, DWORD reserved2, IStream** ppstm)
{
	auto* pStream = new CFTPFileStream(m_pRoot);
	if (!pStream)
		return E_OUTOFMEMORY;
	HANDLE handle;
	auto hr = m_pRoot->OpenFile(this, pwcsName, grfMode, &handle);
	if (FAILED(hr))
	{
		delete pStream;
		return hr;
	}
	pStream->SetHandle(handle);
	*ppstm = pStream;
	return S_OK;
}

STDMETHODIMP CFTPDirectoryBase::CreateStorage(const OLECHAR* pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStorage** ppstg)
{
	if (!pwcsName || !ppstg)
		return E_POINTER;
	if (grfMode & (STGM_SHARE_DENY_READ | STGM_SHARE_DENY_WRITE | STGM_SHARE_EXCLUSIVE | STGM_PRIORITY | STGM_CONVERT | STGM_TRANSACTED | STGM_NOSCRATCH | STGM_NOSNAPSHOT | STGM_SIMPLE | STGM_DIRECT_SWMR | STGM_DELETEONRELEASE))
		return STG_E_INVALIDFLAG;
	auto hr = m_pRoot->CreateFTPDirectory(NULL, this, pwcsName);
	if (FAILED(hr))
		return hr;
	CFTPDirectoryBase* pDir;
	hr = OpenNewDirectory(pwcsName, &pDir);
	if (FAILED(hr))
		return hr;
	pDir->m_grfMode = grfMode;
	*ppstg = pDir;
	return hr;
}

STDMETHODIMP CFTPDirectoryBase::OpenStorage(const OLECHAR* pwcsName, IStorage* pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage** ppstg)
{
	if (!pwcsName || !ppstg)
		return E_POINTER;
	if (pstgPriority)
		return E_INVALIDARG;
	if (grfMode & (STGM_SHARE_DENY_READ | STGM_SHARE_DENY_WRITE | STGM_SHARE_EXCLUSIVE | STGM_PRIORITY | STGM_CONVERT | STGM_TRANSACTED | STGM_NOSCRATCH | STGM_NOSNAPSHOT | STGM_SIMPLE | STGM_DIRECT_SWMR | STGM_DELETEONRELEASE))
		return STG_E_INVALIDFLAG;
	HRESULT hr;
	if (grfMode & STGM_CREATE)
	{
		hr = m_pRoot->CreateFTPDirectory(NULL, this, pwcsName);
		if (FAILED(hr))
			return hr;
	}
	CFTPDirectoryBase* pDir;
	hr = OpenNewDirectory(pwcsName, &pDir);
	if (FAILED(hr))
		return hr;
	pDir->m_grfMode = grfMode;
	*ppstg = pDir;
	return hr;
}

STDMETHODIMP CFTPDirectoryBase::CopyTo(DWORD ciidExclude, const IID* rgiidExclude, SNB snbExclude, IStorage* pstgDest)
{
	bool excludeFile = false;
	bool excludeDirectory = false;
	if (rgiidExclude != NULL)
	{
		if (ciidExclude == 0)
		{
			excludeFile = true;
			excludeDirectory = true;
		}
		else
		{
			for (DWORD i = 0; i < ciidExclude; ++i)
			{
				if (IsEqualIID(rgiidExclude[i], IID_IStream))
					excludeFile = true;
				else if (IsEqualIID(rgiidExclude[i], IID_IStorage))
					excludeDirectory = true;
			}
		}
	}

	if (!m_bDirReceived)
	{
		if (!m_pRoot->ReceiveDirectory(m_hWndOwnerCache, this, m_strDirectory, &m_bDirReceived))
		{
			// nothing to be copied
			return S_OK;
		}
	}

	for (auto i = 0; i < m_aFiles.GetCount(); ++i)
	{
		auto* pFile = m_aFiles.GetItem(i);
		if (pFile->IsDirectory() && excludeDirectory)
			continue;
		if (!pFile->IsDirectory() && excludeFile)
			continue;
		if (snbExclude != NULL)
		{
			auto isExcluded = false;
			for (auto snbExcludePointer = snbExclude; *snbExcludePointer; ++snbExcludePointer)
			{
				if (pFile->strFileName.Compare(*snbExcludePointer) == 0)
				{
					isExcluded = true;
					break;
				}
			}
			if (isExcluded)
				continue;
		}

		auto hr = CopyFileItemToStorage(pFile, ciidExclude, rgiidExclude, snbExclude, pstgDest);
		if (FAILED(hr))
			return hr;
	}
	return S_OK;
}

STDMETHODIMP CFTPDirectoryBase::MoveElementTo(const OLECHAR* pwcsName, IStorage* pstgDest, const OLECHAR* pwcsNewName, DWORD grfFlags)
{
	IEasySFTPDirectory* pFTPDir;
	if (SUCCEEDED(pstgDest->QueryInterface(&pFTPDir)))
	{
		VARIANT_BOOL bIsSFTP = VARIANT_FALSE;
		int nPort = 0;
		BSTR hostName = NULL;
		if (SUCCEEDED(pFTPDir->GetHostInfo(&bIsSFTP, &nPort, &hostName)))
		{
			int nPortTemp = 0;
			auto isSFTP = _wcsicmp(m_pRoot->GetProtocolName(nPortTemp), L"sftp") == 0;
			auto isSameHost = nPortTemp == nPort && m_pRoot->m_strHostName.Compare(hostName, true) == 0 &&
				isSFTP;
			::SysFreeString(hostName);
			if (isSameHost)
			{
				CFTPDirectoryBase* pDir = static_cast<CFTPDirectoryBase*>(pFTPDir);
				CMyStringW strFromName = m_strDirectory;
				if (((LPCWSTR)strFromName)[strFromName.GetLength() - 1] != L'/')
					strFromName += L'/';
				strFromName += pwcsName;
				CMyStringW strToName = pDir->m_strDirectory;
				if (((LPCWSTR)strToName)[strToName.GetLength() - 1] != L'/')
					strToName += L'/';
				strToName = pwcsNewName;
				auto hr = m_pRoot->RenameFTPItem(strFromName, strToName);
				pFTPDir->Release();
				return hr;
			}
		}
	}

	if (!m_bDirReceived)
	{
		if (!m_pRoot->ReceiveDirectory(m_hWndOwnerCache, this, m_strDirectory, &m_bDirReceived))
		{
			return STG_E_FILENOTFOUND;
		}
	}

	auto* pFile = GetFileItem(pwcsName);
	if (!pFile)
		return STG_E_FILENOTFOUND;

	auto hr = CopyFileItemToStorage(pFile, 0, NULL, NULL, pstgDest);
	if (FAILED(hr))
		return hr;

	CMyStringArrayW astrMsgs;
	return m_pRoot->DoDeleteFileOrDirectory(NULL, astrMsgs, pFile->IsDirectory(), pFile->strFileName, this);
}

STDMETHODIMP CFTPDirectoryBase::Commit(DWORD)
{
	return S_OK;
}

STDMETHODIMP CFTPDirectoryBase::Revert()
{
	return S_OK;
}

STDMETHODIMP CFTPDirectoryBase::EnumElements(DWORD reserved1, void* reserved2, DWORD reserved3, IEnumSTATSTG** ppenum)
{
	if (!ppenum)
		return E_POINTER;
	*ppenum = NULL;
	if (!m_bDirReceived)
	{
		if (!m_pRoot->ReceiveDirectory(m_hWndOwnerCache, this, m_strDirectory, &m_bDirReceived))
		{
			return STG_E_FILENOTFOUND;
		}
	}
	auto* pEnum = new CEnumFTPItemStatstg(m_pMallocData, m_aFiles, m_pRoot->IsLockSupported(), static_cast<IShellFolder*>(this));
	if (!pEnum)
	{
		return E_OUTOFMEMORY;
	}
	*ppenum = pEnum;
	return S_OK;
}

STDMETHODIMP CFTPDirectoryBase::DestroyElement(const OLECHAR* pwcsName)
{
	if (!pwcsName)
		return E_POINTER;
	if (!m_bDirReceived)
	{
		if (!m_pRoot->ReceiveDirectory(m_hWndOwnerCache, this, m_strDirectory, &m_bDirReceived))
		{
			return STG_E_FILENOTFOUND;
		}
	}

	auto pSub = wcschr(pwcsName, L'/');
	if (pSub)
	{
		CMyStringW strDir;
		strDir.SetString(pwcsName, pSub - pwcsName);
		pwcsName = pSub;
		auto* pItem = GetFileItem(strDir);
		if (!pItem)
			return STG_E_PATHNOTFOUND;
		CFTPDirectoryBase* pDir;
		auto hr = OpenNewDirectory(strDir, &pDir);
		if (FAILED(hr))
			return hr;
		hr = pDir->DestroyElement(pwcsName);
		pDir->Release();
		return hr;
	}

	auto* pItem = GetFileItem(pwcsName);
	if (!pItem)
		return STG_E_PATHNOTFOUND;
	return DeleteFTPItem(pItem);
}

STDMETHODIMP CFTPDirectoryBase::RenameElement(const OLECHAR* pwcsOldName, const OLECHAR* pwcsNewName)
{
	CMyStringW strFromName = m_strDirectory;
	if (((LPCWSTR)strFromName)[strFromName.GetLength() - 1] != L'/')
		strFromName += L'/';
	strFromName += pwcsOldName;
	CMyStringW strToName = m_strDirectory;
	if (((LPCWSTR)strToName)[strToName.GetLength() - 1] != L'/')
		strToName += L'/';
	strToName += pwcsNewName;
	return m_pRoot->RenameFTPItem(strFromName, strToName);
}

STDMETHODIMP CFTPDirectoryBase::SetElementTimes(const OLECHAR* pwcsName, const FILETIME* pctime, const FILETIME* patime, const FILETIME* pmtime)
{
	CMyStringW strFromName = m_strDirectory;
	if (((LPCWSTR)strFromName)[strFromName.GetLength() - 1] != L'/')
		strFromName += L'/';
	strFromName += pwcsName;
	return m_pRoot->SetFileTime(strFromName, pctime, patime, pmtime);
}

STDMETHODIMP CFTPDirectoryBase::SetClass(REFCLSID clsid)
{
	m_clsidThis = clsid;
	return S_OK;
}

STDMETHODIMP CFTPDirectoryBase::SetStateBits(DWORD grfStateBits, DWORD grfMask)
{
	m_grfStateBits &= ~grfMask;
	m_grfStateBits |= grfStateBits & grfMask;
	return S_OK;
}

STDMETHODIMP CFTPDirectoryBase::Stat(STATSTG* pstatstg, DWORD grfStatFlag)
{
	return m_pRoot->StatDirectory(this, m_grfMode, pstatstg, grfStatFlag);
}

///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFTPDirectoryBase::GetRootDirectory(IEasySFTPDirectory** ppRootDirectory)
{
	return m_pRoot->QueryInterface(IID_IEasySFTPDirectory, (void**)ppRootDirectory);
}

///////////////////////////////////////////////////////////////////////////////

HRESULT CFTPDirectoryBase::CreateStream(CFTPFileItem* pItem, IStream** ppStream)
{
	HRESULT hr = m_pRoot->CreateFTPItemStream(this, pItem, ppStream);
	if (FAILED(hr))
		return hr;
	if (!TEXTMODE_IS_NO_CONVERTION(m_pRoot->m_bTextMode))
	{
		hr = IsTextFile(pItem->strFileName);
		if (SUCCEEDED(hr) && hr == S_OK)
		{
			IStream* pstm2;
			hr = MyCreateTextStream(*ppStream, TEXTMODE_FOR_RECV(m_pRoot->m_bTextMode), &pstm2);
			(*ppStream)->Release();
			if (FAILED(hr))
			{
				*ppStream = NULL;
				return hr;
			}
			*ppStream = pstm2;
		}
	}
	return S_OK;
}

HRESULT CFTPDirectoryBase::DeleteFTPItem(CFTPFileItem* pItem)
{
	CMyPtrArrayT<CFTPFileItem> aItems;
	aItems.Add(pItem);
	pItem->AddRef();
	auto hr = m_pRoot->DoDeleteFTPItems(m_hWndOwnerCache, this, aItems);
	if (FAILED(hr))
	{
		pItem->Release();
		return hr;
	}
	//PITEMID_CHILD pidl = ::CreateFileItem(m_pMallocData->pMalloc, pItem);
	//if (!pidl)
	//	return;
	//NotifyUpdate(pItem->IsDirectory() ? SHCNE_RMDIR : SHCNE_DELETE, pidl, NULL);
	//::CoTaskMemFree(pidl);
	UpdateRemoveFile(pItem->strFileName, pItem->IsDirectory());
	pItem->Release();
	return S_OK;
}

void CFTPDirectoryBase::AfterPaste(CFTPDataObject* pObject, DWORD dwEffects)
{
	theApp.CheckClipboardForMoved(pObject, dwEffects);
}

STDMETHODIMP CFTPDirectoryBase::ParseDisplayName2(PIDLIST_RELATIVE pidlParent,
	HWND hWnd,
	LPBC pbc,
	LPWSTR pszDisplayName,
	ULONG* pchEaten,
	PIDLIST_RELATIVE* ppidl,
	ULONG* pdwAttributes)
{
	//bool bSFTPMode;
	ULONG uEaten;
	if (!ppidl)
		return E_POINTER;
	m_hWndOwnerCache = hWnd;
	*ppidl = NULL;

	LPWSTR lpw;
	CMyStringW strName;
	PIDLIST_RELATIVE pidlCurrent;

	pidlCurrent = NULL;
	uEaten = 0;
	while (true)
	{
		if (*pszDisplayName == L'/')
		{
			pszDisplayName++;
			uEaten++;
		}
		if (!*pszDisplayName)
			break;
		lpw = wcschr(pszDisplayName, L'/');
		if (lpw)
		{
			if (lpw == pszDisplayName)
				continue;
			strName.SetString(pszDisplayName, (DWORD)((DWORD_PTR)lpw - (DWORD_PTR)pszDisplayName) / sizeof(WCHAR));
		}
		else
			strName = pszDisplayName;

		{
			pszDisplayName += strName.GetLength();
			uEaten += (ULONG)strName.GetLength();

			PITEMID_CHILD pidlChild;
			CFTPFileItem* pItem = NULL;
			if (m_bDirReceived)
				pItem = GetFileItem(strName);
			if (pItem)
				pidlChild = ::CreateFileItem(m_pMallocData->pMalloc, pItem);
			else
				pidlChild = ::CreateDummyFileItem(m_pMallocData->pMalloc, strName, *pszDisplayName == L'/');
			if (!pidlChild)
			{
				::CoTaskMemFree(pidlCurrent);
				return E_OUTOFMEMORY;
			}
			PIDLIST_RELATIVE pidl2;
			if (!pidlCurrent && !pidlParent)
				pidl2 = pidlChild;
			else
			{
				pidl2 = (PIDLIST_RELATIVE) ::AppendItemIDList(
					(PCUIDLIST_ABSOLUTE)(pidlCurrent ? pidlCurrent : pidlParent),
					(PCUIDLIST_RELATIVE)pidlChild);
				::CoTaskMemFree(pidlChild);
				if (pidlCurrent)
					::CoTaskMemFree(pidlCurrent);
				if (!pidl2)
					return E_OUTOFMEMORY;
			}
			if (pItem)
			{
				CFTPDirectoryBase* pDir;
				HRESULT hr = OpenNewDirectory(strName, &pDir);
				if (SUCCEEDED(hr))
				{
					ULONG u = 0;
					hr = pDir->ParseDisplayName2(pidl2, hWnd, pbc, pszDisplayName, &u, ppidl, pdwAttributes);
					pDir->Release();
					if (SUCCEEDED(hr))
					{
						uEaten += u;
						::CoTaskMemFree(pidl2);
						if (pdwAttributes)
						{
							//hr = GetAttributesOf(1, (PCUITEMID_CHILD_ARRAY) &pidlCurrent, pdwAttributes);
							*pdwAttributes &= SFGAO_FOLDER;
						}
						if (pchEaten)
							*pchEaten = uEaten;
						return hr;
					}
				}
			}
			pidlCurrent = pidl2;
		}
	}
	if (pdwAttributes)
	{
		//hr = GetAttributesOf(1, (PCUITEMID_CHILD_ARRAY) &pidlCurrent, pdwAttributes);
		*pdwAttributes &= SFGAO_FOLDER;
	}
	if (pchEaten)
		*pchEaten = uEaten;
	if (!pidlCurrent)
		pidlCurrent = (PIDLIST_RELATIVE) ::DuplicateItemIDList((PCUIDLIST_ABSOLUTE)pidlParent);
	*ppidl = pidlCurrent;
	return S_OK;
}

STDMETHODIMP_(void) CFTPDirectoryBase::UpdateItem(PCUITEMID_CHILD pidlOld, PCUITEMID_CHILD pidlNew, LONG lEvent)
{
	CFTPFileItem* pItem = NULL;
	CMyStringW str;
	if (pidlOld)
	{
		if (!::PickupFileName(pidlOld, str))
			return;
		pItem = GetFileItem(str);
		if (!pItem)
			return;
	}
	str.Empty();
	if (pidlNew)
	{
		if (!::PickupFileName(pidlNew, str))
			return;
	}
	UpdateItem(pItem, str, lEvent);
}

STDMETHODIMP_(void) CFTPDirectoryBase::UpdateItem(CFTPFileItem* pOldItem, LPCWSTR lpszNewItem, LONG lEvent)
{
	switch (lEvent)
	{
	case SHCNE_CREATE:
	case SHCNE_MKDIR:
		break;
	case SHCNE_RENAMEITEM:
	case SHCNE_RENAMEFOLDER:
		pOldItem->strFileName = lpszNewItem;
		break;
	case SHCNE_UPDATEITEM:
	case SHCNE_UPDATEDIR:
		break;
	case SHCNE_DELETE:
	case SHCNE_RMDIR:
	{
		int i = m_aFiles.FindItem(pOldItem);
		m_aFiles.RemoveItem(i);
		pOldItem->Release();
	}
	break;
	}
}


HRESULT CFTPDirectoryBase::OpenNewDirectory(LPCWSTR lpszRelativePath, CFTPDirectoryBase** ppDirectory)
{
	CMyStringW str;
	LPCWSTR lp = wcschr(lpszRelativePath, L'/');
	if (!lp)
	{
		str = lpszRelativePath;
		lpszRelativePath = NULL;
	}
	else
	{
		str.SetString(lpszRelativePath, (DWORD)(((DWORD_PTR)lp - (DWORD_PTR)lpszRelativePath) / sizeof(WCHAR)));
		lpszRelativePath = lp + 1;
		if (!*lpszRelativePath)
			lpszRelativePath = NULL;
	}
	CFTPFileItem* pItem = NULL;
	if (m_bDirReceived)
	{
		pItem = GetFileItem(str);
		if (pItem)
		{
			if (!pItem->IsDirectory())
				return E_INVALIDARG;
		}
	}

	CFTPDirectoryItem* pDirItem = NULL;
	bool bFound = false;
	for (int i = 0; i < m_aDirectories.GetCount(); i++)
	{
		pDirItem = m_aDirectories.GetItem(i);
		if (pDirItem->strName.Compare(str) == 0)
		{
			bFound = true;
			break;
		}
	}
	if (!bFound)
	{
		pDirItem = new CFTPDirectoryItem();
		pDirItem->pDirectory = NULL;
		pDirItem->strName = str;
	}
	if (!pDirItem->pDirectory)
	{
		PITEMID_CHILD pidlItem;
		if (pItem)
			pidlItem = ::CreateFileItem(m_pRoot->m_pMallocData->pMalloc, pItem);
		else
			pidlItem = ::CreateDummyFileItem(m_pRoot->m_pMallocData->pMalloc, str, true);
		if (!pidlItem)
		{
			pDirItem->pDirectory = NULL;
			if (!bFound)
				pDirItem->Release();
			return E_OUTOFMEMORY;
		}
		//((CSFTPFileItem*) pidlItem)->bHasAttribute = true;
		//((CSFTPFileItem*) pidlItem)->bIsDirectory = true;
		if (m_strDirectory.GetLength() != 1 || *((LPCWSTR)m_strDirectory) != L'/')
			str.InsertChar(L'/', 0);
		str.InsertString(m_strDirectory, 0);
		HRESULT hr = CreateInstance(pDirItem, this, m_pRoot, str, &pDirItem->pDirectory);
		if (FAILED(hr))
		{
			::CoTaskMemFree(pidlItem);
			pDirItem->pDirectory = NULL;
			if (!bFound)
				pDirItem->Release();
			return E_OUTOFMEMORY;
		}
		{
			PIDLIST_ABSOLUTE pidlChild = ::AppendItemIDList(m_pidlMe, pidlItem);
			hr = pDirItem->pDirectory->Initialize(pidlChild);
			::CoTaskMemFree(pidlChild);
			::CoTaskMemFree(pidlItem);
			if (FAILED(hr))
			{
				pDirItem->pDirectory->Release();
				pDirItem->pDirectory = NULL;
				if (!bFound)
					pDirItem->Release();
				return E_OUTOFMEMORY;
			}
		}
	}
	else
		pDirItem->pDirectory->AddRef();

	if (lpszRelativePath)
	{
		HRESULT hr = pDirItem->pDirectory->OpenNewDirectory(lpszRelativePath, ppDirectory);
		pDirItem->pDirectory->Release();
		if (SUCCEEDED(hr))
		{
			if (!bFound)
				m_aDirectories.Add(pDirItem);
		}
		else
		{
			//pDirItem->pDirectory = NULL;
			if (!bFound)
				pDirItem->Release();
		}
		return hr;
	}
	else
	{
		*ppDirectory = pDirItem->pDirectory;
		//pDirItem->pDirectory->AddRef();
		if (!bFound)
			m_aDirectories.Add(pDirItem);
		return S_OK;
	}
}

CFTPDirectoryItem* CFTPDirectoryBase::GetAlreadyOpenedDirectory(PCUIDLIST_RELATIVE pidlChild)
{
	CMyStringW str;
	if (!::PickupFileName((PCUITEMID_CHILD)pidlChild, str))
		return NULL;
	pidlChild = (PCUIDLIST_RELATIVE)(((DWORD_PTR)pidlChild) + pidlChild->mkid.cb);
	for (int i = 0; i < m_aDirectories.GetCount(); i++)
	{
		CFTPDirectoryItem* pDirItem = m_aDirectories.GetItem(i);
		if (pDirItem->strName.Compare(str) == 0)
		{
			if (!pDirItem->pDirectory)
				return NULL;
			if (pidlChild->mkid.cb)
				return pDirItem->pDirectory->GetAlreadyOpenedDirectory(pidlChild);
			else
			{
				pDirItem->AddRef();
				return pDirItem;
			}
		}
	}
	return NULL;
}

CFTPFileItem* CFTPDirectoryBase::GetFileItem(LPCWSTR lpszName) const
{
	for (int i = 0; i < m_aFiles.GetCount(); i++)
	{
		CFTPFileItem* p = m_aFiles.GetItem(i);
		if (p->strFileName.Compare(lpszName) == 0)
			return p;
	}
	return NULL;
}

CFTPFileItem* CFTPDirectoryBase::GetFileItem(LPCWSTR lpszRelativeName, CFTPDirectoryBase** ppParentDirectory)
{
	if (ppParentDirectory)
		*ppParentDirectory = NULL;
	if (*lpszRelativeName == L'/')
	{
		if (this != m_pRoot)
		{
			return m_pRoot->GetFileItem(lpszRelativeName + 1, ppParentDirectory);
		}
		++lpszRelativeName;
	}
	auto nextToken = wcschr(lpszRelativeName, L'/');
	if (!nextToken)
	{
		auto* p = GetFileItem(lpszRelativeName);
		if (p && ppParentDirectory)
		{
			*ppParentDirectory = this;
			AddRef();
		}
		return p;
	}
	CMyStringW str;
	str.SetString(lpszRelativeName, nextToken - lpszRelativeName);
	lpszRelativeName = nextToken + 1;
	for (int i = 0; i < m_aDirectories.GetCount(); i++)
	{
		CFTPDirectoryItem* pDirItem = m_aDirectories.GetItem(i);
		if (pDirItem->strName.Compare(str) == 0)
		{
			if (!pDirItem->pDirectory)
				return NULL;
			return pDirItem->pDirectory->GetFileItem(lpszRelativeName, ppParentDirectory);
		}
	}
	return NULL;
}

CFTPFileItem* CFTPDirectoryBase::GetFileItem(PCUIDLIST_RELATIVE pidlChild, CFTPDirectoryBase** ppParentDirectory)
{
	if (ppParentDirectory)
		*ppParentDirectory = NULL;
	CMyStringW str;
	if (!::PickupFileName((PCUITEMID_CHILD)pidlChild, str))
		return NULL;
	pidlChild = (PCUIDLIST_RELATIVE)(((DWORD_PTR)pidlChild) + pidlChild->mkid.cb);
	if (!pidlChild->mkid.cb)
	{
		auto* p = GetFileItem(str);
		if (p && ppParentDirectory)
		{
			*ppParentDirectory = this;
			AddRef();
		}
		return p;
	}
	for (int i = 0; i < m_aDirectories.GetCount(); i++)
	{
		CFTPDirectoryItem* pDirItem = m_aDirectories.GetItem(i);
		if (pDirItem->strName.Compare(str) == 0)
		{
			if (!pDirItem->pDirectory)
				return NULL;
			return pDirItem->pDirectory->GetFileItem(pidlChild, ppParentDirectory);
		}
	}
	return NULL;
}

void CFTPDirectoryBase::NotifyUpdate(LONG wEventId, LPCWSTR lpszFile1, LPCWSTR lpszFile2)
{
	PIDLIST_RELATIVE pidlR1 = lpszFile1 ? ::CreateFullPathFileItem(m_pMallocData->pMalloc, lpszFile1) : NULL;
	PIDLIST_RELATIVE pidlR2 = lpszFile2 ? ::CreateFullPathFileItem(m_pMallocData->pMalloc, lpszFile2) : NULL;
	PIDLIST_ABSOLUTE pidl1 = pidlR1 ? ::AppendItemIDList(*lpszFile1 == L'/' ? m_pRoot->m_pidlMe : m_pidlMe, pidlR1) : NULL;
	PIDLIST_ABSOLUTE pidl2 = pidlR2 ? ::AppendItemIDList(*lpszFile2 == L'/' ? m_pRoot->m_pidlMe : m_pidlMe, pidlR2) : NULL;
	theApp.MyChangeNotify(wEventId, SHCNF_IDLIST | SHCNF_NOTIFYRECURSIVE | SHCNF_FLUSHNOWAIT, pidl1, pidl2);
	if (pidl1)
		::CoTaskMemFree(pidl1);
	if (pidl2)
		::CoTaskMemFree(pidl2);
	// In registry-emulating mode, the view cannot receive the change notify.
	// So we will update our own views manually.
	if (theApp.m_bEmulateRegMode)
	{
		// pidlR1 and pidlR2 are not actual PITEMID_CHILDren, so
		// we need to parse id-lists to pass real PITEMID_CHILDren to their parents
		CFTPDirectoryBase* pParent = lpszFile1 && *lpszFile1 == L'/' ? m_pRoot : this;
		PCUIDLIST_RELATIVE pidlUR1 = pidlR1;
		PCUIDLIST_RELATIVE pidlUR2 = pidlR2;
		CMyStringW str;
		pParent->AddRef();
		// only 2 events that use both pidlR1 and pidlR2
		if (wEventId == SHCNE_RENAMEFOLDER || wEventId == SHCNE_RENAMEITEM)
		{
			// pick up the last same parent
			while (pidlUR1 && pidlUR2 && pidlUR1->mkid.cb &&
				pidlUR1->mkid.cb == pidlUR2->mkid.cb &&
				memcmp(pidlUR1, pidlUR2, (size_t)pidlUR1->mkid.cb) == 0)
			{
				PickupFileName((PCUITEMID_CHILD)pidlUR1, str);
				CFTPDirectoryBase* pDir2;
				if (FAILED(pParent->OpenNewDirectory(str, &pDir2)))
				{
					pParent->Release();
					pParent = NULL;
					break;
				}
				pParent->Release();
				pParent = pDir2;
				pidlUR1 = (PCUIDLIST_RELATIVE)(((DWORD_PTR)pidlUR1) + pidlUR1->mkid.cb);
				pidlUR2 = (PCUIDLIST_RELATIVE)(((DWORD_PTR)pidlUR2) + pidlUR2->mkid.cb);
			}
		}
		if (pParent)
		{
			// if pidlUR1 and pidlUR2 are the actual PCUITEMID_CHILDren,
			// we can just pass them to pParent->DefViewNotifyUpdate
			if ((wEventId == SHCNE_RENAMEFOLDER || wEventId == SHCNE_RENAMEITEM) &&
				pidlUR1->mkid.cb && pidlUR2->mkid.cb &&
				!(((PCUIDLIST_RELATIVE)(((DWORD_PTR)pidlUR1) + pidlUR1->mkid.cb))->mkid.cb) &&
				!(((PCUIDLIST_RELATIVE)(((DWORD_PTR)pidlUR2) + pidlUR2->mkid.cb))->mkid.cb))
			{
				pParent->DefViewNotifyUpdate(wEventId, (PCUITEMID_CHILD)pidlUR1, (PCUITEMID_CHILD)pidlUR2);
			}
			else
			{
				// picks up and call the method to the last parent of pidlUR1
				if (pidlUR1)
				{
					CFTPDirectoryBase* pDir2 = pParent;
					pDir2->AddRef();
					while (((PCUIDLIST_RELATIVE)(((DWORD_PTR)pidlUR1) + pidlUR1->mkid.cb))->mkid.cb)
					{
						PickupFileName((PCUITEMID_CHILD)pidlUR1, str);
						CFTPDirectoryBase* pDir3;
						if (FAILED(pDir2->OpenNewDirectory(str, &pDir3)))
						{
							pDir2->Release();
							pDir2 = NULL;
							break;
						}
						pDir2->Release();
						pDir2 = pDir3;
						pidlUR1 = (PCUIDLIST_RELATIVE)(((DWORD_PTR)pidlUR1) + pidlUR1->mkid.cb);
					}
					if (pDir2)
					{
						LONG l = wEventId;
						// change event type to the nearest one because
						// the parents of pidlUR1 and pidlUR2 are not same and
						// we cannot pass pidlUR2 to the parent of pidlUR1
						if (l == SHCNE_RENAMEFOLDER)
							l = SHCNE_RMDIR;
						else if (l == SHCNE_RENAMEITEM)
							l = SHCNE_DELETE;
						pDir2->DefViewNotifyUpdate(l, (PCUITEMID_CHILD)pidlUR1, NULL);
						pDir2->Release();
					}
				}
				// picks up and call the method to the last parent of pidlUR2
				if (pidlUR2)
				{
					CFTPDirectoryBase* pDir2 = pParent;
					pDir2->AddRef();
					while (((PCUIDLIST_RELATIVE)(((DWORD_PTR)pidlUR2) + pidlUR2->mkid.cb))->mkid.cb)
					{
						PickupFileName((PCUITEMID_CHILD)pidlUR2, str);
						CFTPDirectoryBase* pDir3;
						if (FAILED(pDir2->OpenNewDirectory(str, &pDir3)))
						{
							pDir2->Release();
							pDir2 = NULL;
							break;
						}
						pDir2->Release();
						pDir2 = pDir3;
						pidlUR2 = (PCUIDLIST_RELATIVE)(((DWORD_PTR)pidlUR2) + pidlUR2->mkid.cb);
					}
					if (pDir2)
					{
						LONG l = wEventId;
						// same as the case of pidlUR1, but
						// the order of arguments is a little different
						if (l == SHCNE_RENAMEFOLDER || l == SHCNE_RENAMEITEM)
						{
							if (l == SHCNE_RENAMEFOLDER)
								l = SHCNE_MKDIR;
							else if (l == SHCNE_RENAMEITEM)
								l = SHCNE_CREATE;
							pidlUR1 = pidlUR2;
							pidlUR2 = NULL;
						}
						else
							pidlUR1 = NULL;
						pDir2->DefViewNotifyUpdate(l, (PCUITEMID_CHILD)pidlUR1, (PCUITEMID_CHILD)pidlUR2);
						pDir2->Release();
					}
				}
				//DefViewNotifyUpdate(wEventId, (PCUITEMID_CHILD) pidlR1, (PCUITEMID_CHILD) pidlR2);
			}
			pParent->Release();
		}
	}
	if (pidlR1)
		::CoTaskMemFree(pidlR1);
	if (pidlR2)
		::CoTaskMemFree(pidlR2);
}

void CFTPDirectoryBase::UpdateNewFile(LPCWSTR lpszFileName, bool bDirectory)
{
	CFTPFileItem* p, * p2;
	// ignore if the file is subitem
	if (wcschr(lpszFileName, L'/'))
	{
		return;
	}
	p = m_pRoot->RetrieveFileItem(this, lpszFileName);
	if (p)
	{
		::EnterCriticalSection(&m_csFiles);
		p2 = GetFileItem(lpszFileName);
		if (p2)
		{
			PITEMID_CHILD pidl1, pidl2;
			int i = m_aFiles.FindItem(p2);
			m_aFiles.SetItem(i, p);
			::LeaveCriticalSection(&m_csFiles);
			pidl1 = CreateFileItem(m_pMallocData->pMalloc, p2);
			p2->Release();
			pidl2 = CreateFileItem(m_pMallocData->pMalloc, p);
			NotifyUpdate(bDirectory ? SHCNE_UPDATEDIR : SHCNE_UPDATEITEM, pidl1, pidl2);
			m_pMallocData->pMalloc->Free(pidl1);
			m_pMallocData->pMalloc->Free(pidl2);
			return;
		}
		else
		{
			m_aFiles.Add(p);
			::LeaveCriticalSection(&m_csFiles);
		}
	}
	NotifyUpdate(bDirectory ? SHCNE_MKDIR : SHCNE_CREATE, lpszFileName, NULL);
}

void CFTPDirectoryBase::UpdateMoveFile(LPCWSTR lpszFromDir, LPCWSTR lpszFileName, bool bDirectory, LPCWSTR lpszNewFileName)
{
	if (!lpszNewFileName)
		lpszNewFileName = lpszFileName;
	CMyStringW strFP(lpszFromDir), strTP(m_strDirectory);
	if (m_strDirectory.Compare(lpszFromDir) != 0)
	{
		CFTPFileItem* p = m_pRoot->RetrieveFileItem(this, lpszNewFileName);
		if (p)
		{
			::EnterCriticalSection(&m_csFiles);
			m_aFiles.Add(p);
			::LeaveCriticalSection(&m_csFiles);
		}
		CFTPDirectoryBase* pDir;
		if (SUCCEEDED(m_pRoot->OpenNewDirectory(lpszFromDir + 1, &pDir)))
		{
			for (auto i = 0; i < pDir->m_aFiles.GetCount(); ++i)
			{
				CFTPFileItem* p = pDir->m_aFiles.GetItem(i);
				if (p->strFileName.Compare(lpszFileName) == 0)
				{
					pDir->m_aFiles.RemoveItem(i);
					p->Release();
					break;
				}
			}
			pDir->Release();
		}
	}
	else
	{
		::EnterCriticalSection(&m_csFiles);
		CFTPFileItem* p = GetFileItem(lpszFileName);
		if (p)
			p->strFileName = lpszNewFileName;
		::LeaveCriticalSection(&m_csFiles);
	}
	if (strFP.IsEmpty() || ((LPCWSTR)strFP)[strFP.GetLength() - 1] != L'/')
		strFP += L'/';
	if (strTP.IsEmpty() || ((LPCWSTR)strTP)[strTP.GetLength() - 1] != L'/')
		strTP += L'/';
	strFP += lpszFileName;
	strTP += lpszNewFileName;
	NotifyUpdate(bDirectory ? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM, strFP, strTP);
}

void CFTPDirectoryBase::UpdateRenameFile(LPCWSTR lpszOldFileName, LPCWSTR lpszNewFileName, bool bDirectory)
{
	CMyStringW strFP(lpszOldFileName);
	//CMyStringW strFP(m_strDirectory), strTP(m_strDirectory);
	::EnterCriticalSection(&m_csFiles);
	CFTPFileItem* p = GetFileItem(lpszOldFileName);
	if (p)
		p->strFileName = lpszNewFileName;
	::LeaveCriticalSection(&m_csFiles);
	//if (m_strDirectory.IsEmpty() || ((LPCWSTR) m_strDirectory)[m_strDirectory.GetLength() - 1] != L'/')
	//{
	//	strFP += L'/';
	//	strTP += L'/';
	//}
	//strFP += lpszOldFileName;
	//strTP += lpszNewFileName;
	//NotifyUpdate(bDirectory ? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM, strFP, strTP);
	NotifyUpdate(bDirectory ? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM, strFP, lpszNewFileName);
}

void CFTPDirectoryBase::UpdateFileAttrs(LPCWSTR lpszFileName, bool bDirectory)
{
	CMyStringW str(lpszFileName);
	::EnterCriticalSection(&m_csFiles);
	for (int i = 0; i < m_aFiles.GetCount(); i++)
	{
		CFTPFileItem* p = m_aFiles.GetItem(i);
		if (p->strFileName.Compare(lpszFileName) == 0)
		{
			CFTPFileItem* p2 = m_pRoot->RetrieveFileItem(this, lpszFileName);
			if (p2)
			{
				m_aFiles.SetItem(i, p2);
				p->Release();
			}
			break;
		}
	}
	::LeaveCriticalSection(&m_csFiles);
	NotifyUpdate(bDirectory ? SHCNE_UPDATEDIR : SHCNE_UPDATEITEM, str, NULL);
}

void CFTPDirectoryBase::UpdateRemoveFile(LPCWSTR lpszFileName, bool bDirectory)
{
	CMyStringW str(lpszFileName);
	::EnterCriticalSection(&m_csFiles);
	for (int i = 0; i < m_aFiles.GetCount(); i++)
	{
		CFTPFileItem* p = m_aFiles.GetItem(i);
		if (p->strFileName.Compare(lpszFileName) == 0)
		{
			m_aFiles.RemoveItem(i);
			p->Release();
			break;
		}
	}
	::LeaveCriticalSection(&m_csFiles);
	NotifyUpdate(bDirectory ? SHCNE_RMDIR : SHCNE_DELETE, str, NULL);
}

void CFTPDirectoryBase::RemoveAllFiles()
{
	::EnterCriticalSection(&m_csFiles);
	int i;
	i = m_aFiles.GetCount();
	while (i--)
		m_aFiles.GetItem(i)->Release();
	m_aFiles.RemoveAll();
	::LeaveCriticalSection(&m_csFiles);
	i = m_aDirectories.GetCount();
	while (i--)
	{
		CFTPDirectoryItem* pItem = m_aDirectories.GetItem(i);
		m_aDirectories.RemoveItem(i);
		if (pItem->pDirectory)
		{
			pItem->pDirectory->RemoveAllFiles();
			//pItem->pDirectory->Release();
		}
		if (!pItem->strName.IsEmpty())
		{
			PITEMID_CHILD pidlChild = ::CreateDummyFileItem(m_pMallocData->pMalloc, pItem->strName, pItem->pDirectory != NULL);
			if (pidlChild)
			{
				NotifyUpdate(SHCNE_RMDIR, pidlChild, NULL);
				m_pMallocData->pMalloc->Free(pidlChild);
			}
		}
		pItem->Release();
	}
	//m_aDirectories.RemoveAll();
	m_bDirReceived = false;
}

HRESULT CFTPDirectoryBase::CopyFileItemToStorage(CFTPFileItem* pFile, DWORD ciidExclude, const IID* rgiidExclude, SNB snbExclude, IStorage* pstgDest)
{
	if (pFile->IsDirectory())
	{
		CFTPDirectoryBase* pDirectory;
		auto hr = OpenNewDirectory(pFile->strFileName, &pDirectory);
		if (FAILED(hr))
			return hr;

		IStorage* pstgChild;
		hr = pstgDest->CreateStorage(pFile->strFileName, STGM_CREATE, 0, 0, &pstgChild);
		if (FAILED(hr))
		{
			pDirectory->Release();
			return hr;
		}
		hr = pDirectory->CopyTo(ciidExclude, rgiidExclude, snbExclude, pstgChild);
		pstgChild->Release();
		pDirectory->Release();
		if (FAILED(hr))
			return hr;
	}
	else
	{
		IStream* pStream;
		auto hr = OpenStream(pFile->strFileName, NULL, STGM_READ, 0, &pStream);
		if (FAILED(hr))
			return hr;

		IStream* pStreamChild;
		hr = pstgDest->CreateStream(pFile->strFileName, STGM_CREATE | STGM_WRITE, 0, 0, &pStreamChild);
		if (FAILED(hr))
		{
			pStream->Release();
			return hr;
		}
		hr = pStream->CopyTo(pStreamChild, pFile->uliSize, NULL, NULL);
		pStreamChild->Release();
		pStream->Release();
		if (FAILED(hr))
			return hr;
	}
	return S_OK;
}
