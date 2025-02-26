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
#include "Sync.h"

#ifdef _DEBUG
#include "FTPFldr.h"
#include "SFTPFldr.h"
#endif

static HMENU __stdcall GetSubMenuByID(HMENU hMenu, UINT uID)
{
	MENUITEMINFOW mii;
	int nCount;

#ifdef _WIN64
	mii.cbSize = sizeof(mii);
#else
	mii.cbSize = MENUITEMINFO_SIZE_V1W;
#endif
	mii.fMask = MIIM_ID | MIIM_SUBMENU;
	nCount = ::GetMenuItemCount(hMenu);
	for (int i = 0; i < nCount; i++)
	{
		::MyGetMenuItemInfoW(hMenu, (UINT)i, TRUE, &mii);
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
		//while (p->pTargetFile)
		//	p = p->pTargetFile;

		auto attr = p->nUnixMode;
		dwRet = p->type == fitypeDir || (attr & S_IFDIR) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
		if (!pItem->strFileName.IsEmpty() && *((LPCWSTR)pItem->strFileName) == L'.')
			dwRet |= FILE_ATTRIBUTE_HIDDEN;
		if (attr & S_IFLNK)
			dwRet |= FILE_ATTRIBUTE_REPARSE_POINT;
		if (attr & (S_IFBLK | S_IFSOCK | S_IFREG))
			dwRet |= FILE_ATTRIBUTE_DEVICE;
		if (p->permissions.readable || !p->permissions.writable)
			dwRet |= FILE_ATTRIBUTE_READONLY;
	}
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
	if (p->iIconIndex == -1)
		FillFileItemInfo(p);
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
			buff->dwFileAttributes = GetDummyFileAttribute(p);
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

static HRESULT _PickFileNameFromVariant(const VARIANT& file, CMyStringW& str, CFTPDirectoryBase* pDirCheck = NULL)
{
	if (file.vt == VT_DISPATCH || file.vt == VT_UNKNOWN)
	{
		if (!file.punkVal)
			return DISP_E_TYPEMISMATCH;
		IEasySFTPFile* pFile;
		auto hr = file.punkVal->QueryInterface(IID_IEasySFTPFile, reinterpret_cast<void**>(&pFile));
		if (FAILED(hr))
			return DISP_E_TYPEMISMATCH;
		if (pDirCheck)
		{
			IEasySFTPDirectory* pDir;
			hr = pFile->get_Directory(&pDir);
			if (FAILED(hr))
				return E_INVALIDARG;
			pDir->Release();
			if (pDir != pDirCheck->GetThisDirectory())
				return E_INVALIDARG;
		}
		BSTR bstr;
		hr = pFile->get_FileName(&bstr);
		if (FAILED(hr))
			return hr;
		MyBSTRToString(bstr, str);
		::SysFreeString(bstr);
		return S_OK;
	}
	else if (file.vt == VT_BSTR)
	{
		if (!file.bstrVal)
			return E_INVALIDARG;
		MyBSTRToString(file.bstrVal, str);
		return S_OK;
	}
	else
	{
		return DISP_E_TYPEMISMATCH;
	}
}

////////////////////////////////////////////////////////////////////////////////

CFTPDirectoryBase::CFTPDirectoryBase(
	CDelegateMallocData* pMallocData,
	CFTPDirectoryItem* pItemMe,
	CFTPDirectoryBase* pParent,
	LPCWSTR lpszDirectory)
	: CFolderBase(pMallocData)
	, m_pParent(pParent)
	, m_pItemMe(pItemMe)
	, m_strDirectory(lpszDirectory)
{
	m_bIsRoot = false;

	CommonConstruct();
}

CFTPDirectoryBase::CFTPDirectoryBase(
	CDelegateMallocData* pMallocData,
	CFTPDirectoryItem* pItemMe,
	ITypeInfo* pInfo)
	: CFolderBase(pMallocData)
	, m_pParent(NULL)
	, m_pItemMe(pItemMe)
{
	m_bIsRoot = true;

	CommonConstruct();
}

constexpr const WCHAR* const s_pIgnoreNames[] = {
	L"CFTPDirectoryBase::AddRef",
	L"CFTPDirectoryBase::Release",
	L"CFTPDirectoryBase::CommonConstruct",
	L"CFTPDirectoryT<IEasySFTPDirectory>::AddRef",
	L"CFTPDirectoryT<IEasySFTPDirectory>::Release",
	L"CFTPDirectoryT<IEasySFTPRootDirectory>::AddRef",
	L"CFTPDirectoryT<IEasySFTPRootDirectory>::Release",
	L"CFTPDirectoryRootBase::AddRef",
	L"CFTPDirectoryRootBase::Release",
	L"CReferenceDelegationChild<CFTPDirectoryBase>::CReferenceDelegationChild<CFTPDirectoryBase>",
	L"CReferenceDelegationChild<CFTPDirectoryBase>::AddRef",
	L"CReferenceDelegationChild<CFTPDirectoryBase>::Release",
	L"CReferenceDelegationChild<CFTPDirectoryBase>::DetachParent",
	L"QueryInterface",
	L"IUnknown_SafeReleaseAndNullPtr",
	NULL,
};

void CFTPDirectoryBase::CommonConstruct()
{
	m_uRef = 1;
	m_bInDetachAndRelease = false;
	m_bPendingDelete = false;
	m_clsidThis = CLSID_NULL;
	m_grfMode = 0;
	m_grfStateBits = 0;
	m_bDirReceived = false;
	if (m_pItemMe)
	{
		m_pItemMe->AddRef();
	}

	ULONG retParent = 0;
	if (m_pParent)
		retParent = m_pParent->AddRef();

	::InitializeCriticalSection(&m_csFiles);
	::InitializeCriticalSection(&m_csRefs);
//#ifdef _DEBUG
//	CMyStringW str, str2;
//	GetCallerName(str2, s_pIgnoreNames);
//	if (m_pParent)
//	{
//		str.Format(L"CFTPDirectoryBase::CommonConstruct() for '%s' (0x%p) from '%s', count = 1 (parent 0x%p: %lu)\n",
//			(LPCWSTR)m_strDirectory, (void*)this, (LPCWSTR)str2, (void*)m_pParent, retParent);
//	}
//	else
//	{
//		str.Format(L"CFTPDirectoryBase::CommonConstruct() for '%s' (0x%p) from '%s', count = 1 (no parent)\n",
//			(LPCWSTR)m_strDirectory, (void*)this, (LPCWSTR)str2);
//	}
//	OutputDebugString(str);
//#endif
}

CFTPDirectoryBase::~CFTPDirectoryBase()
{
	if (m_pItemMe)
		m_pItemMe->Release();
	//#ifdef _DEBUG
	//	m_pParent = NULL;
	//	m_pItemMe = NULL;
	//#endif
	//#ifdef _DEBUG
	//	m_aDirectories.RemoveAll();
	//#endif
	//#ifdef _DEBUG
	//	m_pRoot = NULL;
	//#endif

	::DeleteCriticalSection(&m_csFiles);
	::DeleteCriticalSection(&m_csRefs);

	//if (m_pParent)
	//	m_pParent->Release();
}

bool CFTPDirectoryBase::DetachImpl()
{
	m_bInDetachAndRelease = true;
	auto i = m_aDirectories.GetCount();
	while (i--)
	{
		CFTPDirectoryItem* p = m_aDirectories.GetItem(i);
		m_aDirectories.RemoveItem(i);
		if (p->pDirectory)
			p->pDirectory->DetachAndRelease();
		p->Release();
	}
	::EnterCriticalSection(&m_csFiles);
	i = m_aFiles.GetCount();
	while (i--)
	{
		CFTPFileItem* pItem = m_aFiles.GetItem(i);
		pItem->DetachParent();
		pItem->Release();
	}
	m_aFiles.RemoveAll();
	::LeaveCriticalSection(&m_csFiles);

	if (m_pParent)
	{
		ULONG c = m_uRef;
		for (ULONG u = 0; u < c; ++u)
			m_pParent->Release();
		m_pParent = NULL;
	}

	m_bInDetachAndRelease = false;
	// see CFTPDirectoryBase::Release
	if (m_bPendingDelete)
	{
		delete this;
		return false;
	}
	else
		return true;
}

ULONG CFTPDirectoryBase::DetachAndRelease()
{
	if (!DetachImpl())
		return 0;
	return Release();
}

STDMETHODIMP_(ULONG) CFTPDirectoryBase::AddRef()
{
	::EnterCriticalSection(&m_csRefs);
	ULONG ret = ::InterlockedIncrement(&m_uRef);
	ULONG retParent = 0;
	if (m_pParent)
		retParent = m_pParent->AddRef();
	::LeaveCriticalSection(&m_csRefs);
//#ifdef _DEBUG
//	CMyStringW str, str2;
//	GetCallerName(str2, s_pIgnoreNames);
//	if (m_pParent)
//	{
//		str.Format(L"CFTPDirectoryBase::AddRef() for '%s' (0x%p), count = %lu (parent 0x%p: %lu) from '%s'\n",
//			(LPCWSTR)m_strDirectory, (void*)this, ret, (void*)m_pParent, retParent, (LPCWSTR)str2);
//	}
//	else
//	{
//		str.Format(L"CFTPDirectoryBase::AddRef() for '%s' (0x%p), count = %lu (no parent) from '%s'\n",
//			(LPCWSTR)m_strDirectory, (void*)this, ret, (LPCWSTR)str2);
//	}
//	OutputDebugString(str);
//#endif
	return ret;
}

STDMETHODIMP_(ULONG) CFTPDirectoryBase::Release()
{
	::EnterCriticalSection(&m_csRefs);
	ULONG ret;
	ULONG retParent = 0;
	if (!m_uRef)
		ret = 0;
	else
	{
		if (m_pParent)
			retParent = m_pParent->Release();
		ret = ::InterlockedDecrement(&m_uRef);
	}
	::LeaveCriticalSection(&m_csRefs);

//#ifdef _DEBUG
//	CMyStringW str, str2;
//	GetCallerName(str2, s_pIgnoreNames);
//	if (m_pParent)
//	{
//		str.Format(L"CFTPDirectoryBase::Release() for '%s' (0x%p), count = %lu (parent 0x%p: %lu) from '%s'\n",
//			(LPCWSTR)m_strDirectory, (void*)this, ret, (void*)m_pParent, retParent, (LPCWSTR)str2);
//	}
//	else
//	{
//		str.Format(L"CFTPDirectoryBase::Release() for '%s' (0x%p), count = %lu (no parent) from '%s'\n",
//			(LPCWSTR)m_strDirectory, (void*)this, ret, (LPCWSTR)str2);
//	}
//	OutputDebugString(str);
//
//	//if (m_bIsRoot)
//	//{
//	//	for (int i = 0; i < m_aDirectories.GetCount(); ++i)
//	//	{
//	//		auto* pChild = m_aDirectories.GetItem(i);
//	//		_ASSERT(pChild->pDirectory->m_uRef < ret + 1);
//	//	}
//	//}
//#endif
	if (!ret)
	{
		if (m_bInDetachAndRelease)
		{
			// This is a workaround for over-Release problem occurring when exiting Explorer process.
			// In some case reference count of the instance become fewer than expected, and
			// the count will be zero during DetachImpl(), so delay 'delete' to avoid unexpected memory access.
			m_bPendingDelete = true;
		}
		else
			delete this;
	}
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
		*ppv = static_cast<IThumbnailHandlerFactory*>(this);
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid, __uuidof(IShellFolderPropertyInformation)))
	{
		*ppv = static_cast<IShellFolderPropertyInformation*>(this);
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IDispatch) ||
		IsEqualIID(riid, IID_IEasySFTPDirectory))
	{
		*ppv = GetThisDirectory();
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid, IID_CFTPDirectoryBase))
	{
		*ppv = static_cast<CFTPDirectoryBase*>(this);
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid, IID_IEasySFTPDirectorySynchronization))
	{
		*ppv = static_cast<IEasySFTPDirectorySynchronization*>(this);
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid, IID_IEasySFTPOldDirectory))
	{
		*ppv = static_cast<IEasySFTPOldDirectory*>(this);
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid, IID_IStorage))
	{
		*ppv = static_cast<IStorage*>(this);
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid, IID_IProvideClassInfo))
	{
		*ppv = static_cast<IProvideClassInfo*>(this);
		AddRef();
		return S_OK;
	}
	return CFolderBase::QueryInterface(riid, ppv);
}

STDMETHODIMP CFTPDirectoryBase::GetClassInfo(ITypeInfo** ppTI)
{
	return theApp.m_pTypeLib->GetTypeInfoOfGuid(CLSID_EasySFTPDirectory, ppTI);
}

STDMETHODIMP CFTPDirectoryBase::EnumObjects(HWND hWnd, SHCONTF grfFlags, IEnumIDList** ppenumIDList)
{
	if (!ppenumIDList)
		return E_POINTER;
	if (!m_bDirReceived)
	{
		if (!(grfFlags & SHCONTF_FASTITEMS))
		{
			auto* pRoot = GetRoot();
			m_hWndOwnerCache = hWnd;
			pRoot->m_hWndOwnerCache = hWnd;
			if (!pRoot->ReceiveDirectory(hWnd, this, m_strDirectory, &m_bDirReceived))
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

	if (IsEqualIID(riid, IID_IShellItem) || IsEqualIID(riid, IID_IShellItem2))
	{
		::EnterCriticalSection(&m_csPidlMe);
		auto pidlAbs = ::AppendItemIDList(m_pidlMe, pidl);
		::LeaveCriticalSection(&m_csPidlMe);
		if (!pidlAbs)
			return E_OUTOFMEMORY;
		IShellItem* pItem = NULL;
		auto hr = ::MyCreateShellItem(pidlAbs, &pItem);
		if (FAILED(hr))
			return hr;
		if (!pItem)
			return E_UNEXPECTED;
		hr = pItem->QueryInterface(riid, ppv);
		pItem->Release();
		return hr;
	}

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
		::EnterCriticalSection(&m_csPidlMe);
		auto pidlAbsolute = ::AppendItemIDList(m_pidlMe, pidl);
		::LeaveCriticalSection(&m_csPidlMe);
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
		!IsEqualIID(riid, IID_IEasySFTPOldDirectory) &&
		!IsEqualIID(riid, IID_IStorage))
		return E_NOINTERFACE;

	PCUIDLIST_RELATIVE pidlNext = NULL;
	auto* pRoot = GetRoot();
	CFTPDirectoryItem* pDirItem = GetAlreadyOpenedDirectory(pidl, &pidlNext);
	CMyStringW strRealPath;
	if (pDirItem)
	{
		// strRealPath includes the child item only, so use BindToObject later with pidlNext
		strRealPath = pDirItem->strRealPath;
	}
	else
	{
		if (!pRoot->ValidateDirectory(m_strDirectory, pidl, strRealPath))
			return E_INVALIDARG;
		if (strRealPath.IsEmpty())
			return E_INVALIDARG;
	}

	LPCWSTR lpw = strRealPath;
	CFTPDirectoryBase* pParent;
	if (*lpw == L'/')
	{
		pParent = pRoot;
		lpw++;
	}
	else
		pParent = this;
	CFTPDirectoryBase* pRet;
	HRESULT hr = pParent->OpenNewDirectory(lpw, &pRet);
	if (FAILED(hr))
		return hr;
	if (pidlNext)
		hr = pRet->BindToObject(pidlNext, pbc, riid, ppv);
	else
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
		hr = pChildDir->QueryInterface(riid, ppv);
		pChildDir->Release();
		return hr;
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
			if (!GetRoot()->ReceiveDirectory(m_hWndOwnerCache, this, m_strDirectory, &m_bDirReceived))
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
				else if (IsTextFile(pItem1->strFileName) == S_OK)
					r = (IsTextFile(pItem2->strFileName) == S_OK ? 0 : -1);
				else if (IsTextFile(pItem2->strFileName) == S_OK)
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
		CFTPFileItem* pItem = new CFTPFileItem(this);
		pItem->bWinAttr = false;
		pItem->nUnixMode = S_IFDIR;
		pItem->strFileName = m_pItemMe->strName;
		MakeUrl(CMyStringW(), pItem->strUrl);
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
		::EnterCriticalSection(&m_csPidlMe);
		CFTPFileDirectoryMenu* pMenu = new CFTPFileDirectoryMenu(this, m_pidlMe, pBrowser);
		::LeaveCriticalSection(&m_csPidlMe);
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
					auto* pRoot = GetRoot();
					if (!m_hWndOwnerCache)
						m_hWndOwnerCache = pRoot->m_hWndOwnerCache;
					if (!pRoot->ReceiveDirectory(m_hWndOwnerCache, this, m_strDirectory, &m_bDirReceived))
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
	auto* pRoot = GetRoot();
	for (UINT u = 0; u < cidl; u++)
	{
		if (!::PickupFileName(apidl[u], str))
			return E_INVALIDARG;
		if (!m_bDirReceived && !bIsIcon)
		{
			m_hWndOwnerCache = hWndOwner;
			pRoot->m_hWndOwnerCache = hWndOwner;
			if (!pRoot->ReceiveDirectory(hWndOwner, this, m_strDirectory, &m_bDirReceived))
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
		::EnterCriticalSection(&m_csPidlMe);
		CFTPFileItemMenu* pMenu = new CFTPFileItemMenu(this, m_pidlMe, pBrowser, aItems);
		::LeaveCriticalSection(&m_csPidlMe);
		if (pBrowser)
			pBrowser->Release();
		hr = pMenu->QueryInterface(riid, ppv);
		pMenu->Release();
	}
	else if (IsEqualIID(riid, IID_IThumbnailProvider))
	{
		if (!pRoot->m_bUseThumbnailPreview)
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
	hr = pRoot->GetFTPItemUIObjectOf(hWndOwner, this, aItems, &pObj);
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
			bool bLastIsNotDelimiter = false;
			if (p)
			{
				str2 = p->strFileName;
			}
			else if (pidl)
			{
				if (!::PickupFileName(pidl, str2))
					return E_INVALIDARG;
			}
			MakeUrl(str2, str, &bLastIsNotDelimiter);
			while (pidl != NULL)
			{
				pidl = (PCUITEMID_CHILD)(((DWORD_PTR)pidl) + pidl->mkid.cb);
				if (!pidl->mkid.cb)
					break;
				if (!::PickupFileName(pidl, str2))
					return E_INVALIDARG;
				if (!bLastIsNotDelimiter)
					str += L'/';
				else
					bLastIsNotDelimiter = true;
				str += str2;
			}
			break;
		}
		// fall through
	case SHGDN_INFOLDER:
		if (!pidl)
		{
			if (m_bIsRoot)
				str = static_cast<CFTPDirectoryRootBase*>(this)->m_strHostName;
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
	if (!pName->pOleStr)
		return E_OUTOFMEMORY;
	if (str.IsEmpty())
		*pName->pOleStr = 0;
	else
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
	HRESULT hr = GetRoot()->SetFTPItemNameOf(hWnd, this, p, pszName, uFlags);
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
			{
				auto* p = m_aFiles.GetItem(i);
				p->DetachParent();
				p->Release();
			}
			m_aFiles.RemoveAll();
			m_bDirReceived = false;
			::LeaveCriticalSection(&m_csFiles);
			i = m_aDirectories.GetCount();
			while (i--)
			{
				auto* p = m_aDirectories.GetItem(i);
				if (p->pDirectory)
					p->pDirectory->DetachAndRelease();
				p->Release();
			}
			m_aDirectories.RemoveAll();
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
			MENUITEMINFOW mii;
			UINT uMaxID;
			CMyStringW str;

#ifdef _WIN64
			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_ID | MIIM_STATE;
#else
			mii.cbSize = MENUITEMINFO_SIZE_V1W;
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
				mii.dwTypeData = str.GetBuffer(MAX_PATH);
				::MyGetMenuItemInfoW(h, (UINT)i, TRUE, &mii);
				mii.wID = (WORD)((UINT)mii.wID - ID_ROOT_IDBASE + lpqi->idCmdFirst);
				if (uMaxID < (UINT)mii.wID)
					uMaxID = (UINT)mii.wID;
				::MyInsertMenuItemW(hMenuToAdd, indexMenu++, TRUE, &mii);
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
			auto* pRoot = GetRoot();
			pRoot->ShowServerInfoDialog(pRoot->m_hWndOwnerCache);
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
typedef _Check_return_ HRESULT(STDMETHODCALLTYPE* T_PSCreatePropertyKeyStore)(_In_opt_ PROPERTYKEY*, _In_ int, _In_ REFIID riid, _COM_Outptr_ _At_(*ppv, _Post_readable_size_(_Inexpressible_(varies))) void** ppv);

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

STDMETHODIMP CFTPDirectoryBase::get_Name(BSTR* pRet)
{
	if (!pRet)
		return E_POINTER;
	auto* p = wcsrchr(m_strDirectory, L'/');
	if (!p)
		p = m_strDirectory;
	else
		++p;
	*pRet = ::SysAllocString(p);
	return *pRet ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CFTPDirectoryBase::get_RootDirectory(IEasySFTPRootDirectory** ppRootDirectory)
{
	if (!ppRootDirectory)
		return E_POINTER;
	return GetRoot()->QueryInterface(IID_IEasySFTPRootDirectory, reinterpret_cast<void**>(ppRootDirectory));
}

STDMETHODIMP CFTPDirectoryBase::OpenDirectory(VARIANT file, IEasySFTPDirectory** ppRet)
{
	if (!ppRet)
		return E_POINTER;
	CMyStringW str;
	auto hr = _PickFileNameFromVariant(file, str, this);
	if (FAILED(hr))
		return hr;
	CFTPDirectoryBase* pDir = NULL;
	hr = OpenNewDirectory(str, &pDir);
	if (FAILED(hr))
		return hr;
	hr = pDir->QueryInterface(IID_IEasySFTPDirectory, reinterpret_cast<void**>(ppRet));
	pDir->Release();
	return hr;
}

STDMETHODIMP CFTPDirectoryBase::get_Files(IEasySFTPFiles** ppFiles)
{
	if (!ppFiles)
		return E_POINTER;
	if (!m_bDirReceived)
	{
		if (!GetRoot()->ReceiveDirectory(m_hWndOwnerCache, this, m_strDirectory, &m_bDirReceived))
		{
			// nothing to be retrieved; do nothing
		}
	}
	*ppFiles = new CFTPFileItemList(this);
	return *ppFiles ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CFTPDirectoryBase::OpenTransferDialog(INT_PTR hWndOwner)
{
	GetRoot()->OpenTransferDialogImpl(reinterpret_cast<HWND>(hWndOwner));
	return S_OK;
}

STDMETHODIMP CFTPDirectoryBase::CloseTransferDialog()
{
	GetRoot()->CloseTransferDialogImpl();
	return S_OK;
}

STDMETHODIMP CFTPDirectoryBase::OpenFile(BSTR lpszRelativeFileName, VARIANT_BOOL bIsWrite, EasySFTPTextMode nTextMode, IEasySFTPStream** ppStream)
{
	if (!ppStream)
		return E_POINTER;
	if (!lpszRelativeFileName)
		return E_INVALIDARG;
	IStream* pStream = NULL;
	IEasySFTPFile* pFile = NULL;
	auto* pRoot = GetRoot();
	auto hr = OpenStream2(lpszRelativeFileName,
		bIsWrite ? STGM_WRITE : STGM_READ,
		!TEXTMODE_IS_NO_CONVERTION(nTextMode) ? static_cast<BYTE>(nTextMode) :
			pRoot->IsTextFile(lpszRelativeFileName) == S_OK ? pRoot->m_bTextMode : 0,
		&pStream,
		&pFile);
	if (FAILED(hr))
		return hr;
	STATSTG statstg = {};
	if (FAILED(pStream->Stat(&statstg, STATFLAG_NONAME)))
		statstg.cbSize.QuadPart = 0;
	CTransferDialog::CTransferItem* pTransfer = NULL;
	if (pRoot->m_pTransferDialog)
	{
		pTransfer = pRoot->m_pTransferDialog->AddTransferItem(statstg.cbSize.QuadPart, lpszRelativeFileName);
	}
	*ppStream = new CEasySFTPStream(pStream, pFile, pRoot->m_pTransferDialog, pTransfer);
	pStream->Release();
	pFile->Release();
	return *ppStream ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CFTPDirectoryBase::UploadFrom(BSTR lpszDestinationRelativeName, BSTR lpszSourceLocalName, EasySFTPTextMode nTextMode)
{
	auto* pFileToken = wcsrchr(lpszDestinationRelativeName, L'/');
	if (pFileToken && pFileToken[1] == 0)
		return E_INVALIDARG;
	CFTPDirectoryBase* pDir = NULL;
	HRESULT hr;
	if (pFileToken)
	{
		CMyStringW strDir;
		strDir.SetString(lpszDestinationRelativeName, pFileToken - lpszDestinationRelativeName);
		lpszDestinationRelativeName = pFileToken + 1;
		hr = OpenNewDirectory(strDir, &pDir);
		if (FAILED(hr))
			return hr;
	}
	else
	{
		pDir = this;
		AddRef();
	}
	auto* pRoot = GetRoot();
	auto* pOperation = new CFTPDropHandlerOperation(pDir, NULL, NULL, pRoot->m_pTransferDialog, true);
	pDir->Release();
	if (!pOperation)
		return E_OUTOFMEMORY;
	BYTE bTextMode = !TEXTMODE_IS_NO_CONVERTION(nTextMode) ? static_cast<BYTE>(nTextMode) : pRoot->m_bTextMode;
	hr = pOperation->RetrieveFileNameSingle(lpszSourceLocalName, NULL, lpszDestinationRelativeName, false, bTextMode);
	delete pOperation;
	return hr;
}

STDMETHODIMP CFTPDirectoryBase::UploadFromStream(BSTR lpszDestinationRelativeFileName, IUnknown* pStream_, EasySFTPTextMode nTextMode)
{
	auto* pFileToken = wcsrchr(lpszDestinationRelativeFileName, L'/');
	if (pFileToken && pFileToken[1] == 0)
		return E_INVALIDARG;
	CFTPDirectoryBase* pDir = NULL;
	IStream* pStream = NULL;
	HRESULT hr = pStream_->QueryInterface(IID_IStream, reinterpret_cast<void**>(&pStream));
	if (FAILED(hr))
		return DISP_E_TYPEMISMATCH;
	if (pFileToken)
	{
		CMyStringW strDir;
		strDir.SetString(lpszDestinationRelativeFileName, pFileToken - lpszDestinationRelativeFileName);
		lpszDestinationRelativeFileName = pFileToken + 1;
		hr = OpenNewDirectory(strDir, &pDir);
		if (FAILED(hr))
		{
			pStream->Release();
			return hr;
		}
	}
	else
	{
		pDir = this;
		AddRef();
	}
	auto* pRoot = GetRoot();
	auto* pOperation = new CFTPDropHandlerOperation(pDir, NULL, NULL, pRoot->m_pTransferDialog, true);
	pDir->Release();
	if (!pOperation)
	{
		pStream->Release();
		return E_OUTOFMEMORY;
	}
	BYTE bTextMode = !TEXTMODE_IS_NO_CONVERTION(nTextMode) ? static_cast<BYTE>(nTextMode) : pRoot->m_bTextMode;
	hr = pOperation->RetrieveFileNameSingle(NULL, pStream, lpszDestinationRelativeFileName, false, bTextMode);
	delete pOperation;
	pStream->Release();
	return hr;
}

STDMETHODIMP CFTPDirectoryBase::UploadFromDataObject(IUnknown* pObject_, EasySFTPTextMode nTextMode)
{
	IDataObject* pObject;
	auto hr = pObject_->QueryInterface(IID_IDataObject, reinterpret_cast<void**>(&pObject));
	if (FAILED(hr))
		return hr;
	auto* pRoot = GetRoot();
	auto* pOperation = new CFTPDropHandlerOperation(this, NULL, pObject, pRoot->m_pTransferDialog, true);
	if (!pOperation)
	{
		pObject->Release();
		return E_OUTOFMEMORY;
	}
	BYTE bTextMode = !TEXTMODE_IS_NO_CONVERTION(nTextMode) ? static_cast<BYTE>(nTextMode) : pRoot->m_bTextMode;
	hr = pOperation->RetrieveFileContents(pObject, bTextMode);
	delete pOperation;
	pObject->Release();
	return hr;
}

STDMETHODIMP CFTPDirectoryBase::DownloadTo(VARIANT file, BSTR lpszTargetLocalName, EasySFTPTextMode nTextMode)
{
	CMyStringW str;
	auto hr = _PickFileNameFromVariant(file, str, this);
	if (FAILED(hr))
		return hr;
	CFTPDirectoryBase* pDir;
	auto* pItem = GetFileItem(str, &pDir);
	if (!pItem)
		return STG_E_FILENOTFOUND;
	IStream* pSource = NULL;
	auto* pRoot = GetRoot();
	hr = pRoot->CreateFTPItemStream(pDir, pItem, &pSource);
	pDir->Release();
	if (FAILED(hr))
		return hr;
	BYTE bTextMode = !TEXTMODE_IS_NO_CONVERTION(nTextMode) ? static_cast<BYTE>(nTextMode) : pRoot->m_bTextMode;
	IStream* pDest;
	hr = MyOpenTextFileToStream(lpszTargetLocalName, true, bTextMode, &pDest);
	if (FAILED(hr))
	{
		pSource->Release();
		return hr;
	}
	auto* pvObject = pRoot->m_pTransferDialog ? pRoot->m_pTransferDialog->AddTransferItem(pItem->uliSize.QuadPart, lpszTargetLocalName) : NULL;
	hr = CopyStream(pSource, pDest, pvObject, pRoot);
	if (pvObject && pRoot->m_pTransferDialog)
		pRoot->m_pTransferDialog->RemoveTransferItem(pvObject, FAILED(hr));
	pDest->Release();
	pSource->Release();
	return hr;
}

STDMETHODIMP CFTPDirectoryBase::DownloadToStream(VARIANT file, IUnknown* pStream_, EasySFTPTextMode nTextMode)
{
	CMyStringW str;
	auto hr = _PickFileNameFromVariant(file, str, this);
	if (FAILED(hr))
		return hr;
	CFTPDirectoryBase* pDir;
	auto* pItem = GetFileItem(str, &pDir);
	if (!pItem)
		return STG_E_FILENOTFOUND;
	IStream* pStream = NULL;
	hr = pStream_->QueryInterface(IID_IStream, reinterpret_cast<void**>(&pStream));
	if (FAILED(hr))
		return DISP_E_TYPEMISMATCH;
	auto* pRoot = GetRoot();
	IStream* pSource = NULL;
	hr = pRoot->CreateFTPItemStream(pDir, pItem, &pSource);
	pDir->Release();
	if (FAILED(hr))
	{
		pStream->Release();
		return hr;
	}
	BYTE bTextMode = !TEXTMODE_IS_NO_CONVERTION(nTextMode) ? static_cast<BYTE>(nTextMode) : pRoot->m_bTextMode;
	IStream* pDest;
	if (!TEXTMODE_IS_NO_CONVERTION(bTextMode) && pRoot->IsTextFile(str))
	{
		hr = MyCreateTextStream(pStream, TEXTMODE_FOR_RECV(bTextMode), &pDest);
		pStream->Release();
		if (FAILED(hr))
		{
			pSource->Release();
			return hr;
		}
	}
	else
	{
		pDest = pStream;
	}
	auto* pvObject = pRoot->m_pTransferDialog ? pRoot->m_pTransferDialog->AddTransferItem(pItem->uliSize.QuadPart, str) : NULL;
	hr = CopyStream(pSource, pDest, pvObject, pRoot);
	if (pvObject && pRoot->m_pTransferDialog)
		pRoot->m_pTransferDialog->RemoveTransferItem(pvObject, FAILED(hr));
	pDest->Release();
	pSource->Release();
	return hr;
}

STDMETHODIMP CFTPDirectoryBase::UploadFiles(SAFEARRAY* LocalFiles, EasySFTPTextMode nTextMode)
{
	HRESULT hr;
	VARTYPE vt;
	hr = SafeArrayGetVartype(LocalFiles, &vt);
	if (FAILED(hr))
		return hr;
	if (vt != VT_BSTR)
		return DISP_E_TYPEMISMATCH;
	if (LocalFiles->cDims != 1)
		return E_INVALIDARG;
	void* pvData;
	hr = SafeArrayAccessData(LocalFiles, &pvData);
	if (FAILED(hr))
		return hr;
	CMyStringArrayW astrFiles;
	auto c = LocalFiles->cbElements;
	while (c--)
	{
		auto bstr = *reinterpret_cast<BSTR*&>(pvData)++;
		if (!bstr || !::SysStringLen(bstr))
		{
			hr = E_INVALIDARG;
			break;
		}
		if (!::MyIsExistFileW(bstr))
		{
			hr = STG_E_FILENOTFOUND;
			break;
		}
		astrFiles.Add(bstr);
	}
	SafeArrayUnaccessData(LocalFiles);
	if (FAILED(hr))
	{
		return hr;
	}
	auto* pRoot = GetRoot();
	auto* pOperation = new CFTPDropHandlerOperation(this, NULL, NULL, pRoot->m_pTransferDialog, true);
	if (!pOperation)
	{
		return E_OUTOFMEMORY;
	}
	BYTE bTextMode = !TEXTMODE_IS_NO_CONVERTION(nTextMode) ? static_cast<BYTE>(nTextMode) : pRoot->m_bTextMode;
	hr = pOperation->RetrieveFileNameMultiple(astrFiles, false, bTextMode);
	delete pOperation;
	return hr;
}

STDMETHODIMP CFTPDirectoryBase::DownloadFiles(SAFEARRAY* RemoteFiles, BSTR bstrDestinationDirectory, EasySFTPTextMode nTextMode)
{
	HRESULT hr;
	VARTYPE vt;
	hr = SafeArrayGetVartype(RemoteFiles, &vt);
	if (FAILED(hr))
		return hr;
	if (RemoteFiles->cDims != 1)
		return E_INVALIDARG;
	void* pvData;
	hr = SafeArrayAccessData(RemoteFiles, &pvData);
	if (FAILED(hr))
		return hr;
	ULONG c = RemoteFiles->cbElements;
	CMyStringArrayW aFiles;
	hr = S_OK;
	while (c--)
	{
		if (vt == VT_BSTR)
		{
			auto bstr = *(reinterpret_cast<BSTR*&>(pvData)++);
			if (bstr == NULL || bstr[0] == 0)
			{
				hr = E_INVALIDARG;
				break;
			}
			aFiles.Add(bstr);
		}
		else if (vt == VT_UNKNOWN || vt == VT_DISPATCH)
		{
			auto pUnk = *(reinterpret_cast<IUnknown**&>(pvData)++);
			if (pUnk == NULL)
			{
				hr = E_INVALIDARG;
				break;
			}
			IEasySFTPFile* pFile;
			hr = pUnk->QueryInterface(IID_IEasySFTPFile, reinterpret_cast<void**>(&pFile));
			if (FAILED(hr))
			{
				hr = DISP_E_TYPEMISMATCH;
				break;
			}
			IEasySFTPDirectory* pDir;
			hr = pFile->get_Directory(&pDir);
			if (FAILED(hr))
			{
				pFile->Release();
				break;
			}
			pDir->Release();
			if (pDir != GetThisDirectory())
			{
				pFile->Release();
				hr = E_INVALIDARG;
				break;
			}
			hr = STG_E_FILENOTFOUND;
			for (int i = 0; i < m_aFiles.GetCount(); ++i)
			{
				auto* p = m_aFiles.GetItem(i);
				if (p == pFile)
				{
					hr = S_OK;
					break;
				}
			}
			if (FAILED(hr))
			{
				pFile->Release();
				break;
			}
			BSTR bstr;
			hr = pFile->get_FileName(&bstr);
			pFile->Release();
			if (FAILED(hr))
				break;
			aFiles.Add(bstr);
			::SysFreeString(bstr);
		}
		else if (vt == VT_VARIANT)
		{
			auto& v = *(reinterpret_cast<VARIANT*&>(pvData)++);
			CMyStringW str;
			hr = _PickFileNameFromVariant(v, str, this);
			if (FAILED(hr))
				break;
			aFiles.Add(str);
		}
		else
		{
			hr = DISP_E_TYPEMISMATCH;
			break;
		}
	}
	SafeArrayUnaccessData(RemoteFiles);
	if (FAILED(hr))
		return hr;

	CMyStringW strDir;
	MyBSTRToString(bstrDestinationDirectory, strDir);
	auto* pRoot = GetRoot();
	auto bTextMode = !TEXTMODE_IS_NO_CONVERTION(nTextMode) ? static_cast<BYTE>(nTextMode) : pRoot->m_bTextMode;
	CMyPtrArrayT<CTransferDialog::CTransferItem> aTransfers;
	for (int i = 0; i < aFiles.GetCount(); ++i)
	{
		CFTPDirectoryBase* pDir;
		auto* pItem = GetFileItem(aFiles.GetItem(i), &pDir);
		if (!pItem)
			return STG_E_FILENOTFOUND;
		pDir->Release();
		CMyStringW strLocalName;
		MyGetAbsolutePathStringW(pItem->strFileName, strDir, strLocalName);
		auto* pvObject = pRoot->m_pTransferDialog ? pRoot->m_pTransferDialog->AddTransferItem(pItem->uliSize.QuadPart, strLocalName) : NULL;
		aTransfers.Add(pvObject);
	}
	int done = -1;
	for (int i = 0; i < aFiles.GetCount(); ++i)
	{
		CFTPDirectoryBase* pDir;
		auto* pItem = GetFileItem(aFiles.GetItem(i), &pDir);
		if (!pItem)
		{
			hr = STG_E_FILENOTFOUND;
			break;
		}
		IStream* pSource = NULL;
		hr = pRoot->CreateFTPItemStream(pDir, pItem, &pSource);
		pDir->Release();
		if (FAILED(hr))
			break;
		CMyStringW strLocalName;
		MyGetAbsolutePathStringW(pItem->strFileName, strDir, strLocalName);
		IStream* pDest;
		hr = MyOpenTextFileToStream(strLocalName, true, bTextMode, &pDest);
		if (FAILED(hr))
		{
			pSource->Release();
			break;
		}
		auto* pvObject = aTransfers.GetItem(i);
		hr = CopyStream(pSource, pDest, pvObject, pRoot);
		if (pvObject && pRoot->m_pTransferDialog)
			pRoot->m_pTransferDialog->RemoveTransferItem(pvObject, FAILED(hr));
		pDest->Release();
		pSource->Release();
		done = i;
		if (FAILED(hr))
			break;
	}
	for (++done; done < aTransfers.GetCount(); ++done)
	{
		auto* pvObject = aTransfers.GetItem(done);
		if (pvObject && pRoot->m_pTransferDialog)
			pRoot->m_pTransferDialog->RemoveTransferItem(pvObject, true);
	}
	return hr;
}

STDMETHODIMP CFTPDirectoryBase::Move(VARIANT file, BSTR lpszTargetName)
{
	CMyStringW str;
	auto hr = _PickFileNameFromVariant(file, str, this);
	if (FAILED(hr))
		return hr;
	return RenameElement(str, lpszTargetName);
}

STDMETHODIMP CFTPDirectoryBase::Remove(VARIANT file)
{
	CMyStringW str;
	auto hr = _PickFileNameFromVariant(file, str, this);
	if (FAILED(hr))
		return hr;
	CFTPDirectoryBase* pDir;
	auto* pItem = GetFileItem(str, &pDir);
	if (!pItem)
		return STG_E_FILENOTFOUND;
	hr = pDir->DeleteFTPItem(pItem);
	pDir->Release();
	return hr;
}

STDMETHODIMP CFTPDirectoryBase::UpdateFileTime(VARIANT file, DATE modifyTime, DATE createTime, DATE accessTime)
{
	CMyStringW str;
	auto hr = _PickFileNameFromVariant(file, str, this);
	if (FAILED(hr))
		return hr;
	CFTPDirectoryBase* pDir;
	auto* pItem = GetFileItem(str, &pDir);
	if (!pItem)
		return STG_E_FILENOTFOUND;
	FILETIME ftModify = {}, ftCreate = {};
	if (modifyTime != 0)
	{
		SYSTEMTIME st;
		if (!VariantTimeToSystemTime(modifyTime, &st) || !SystemTimeToFileTime(&st, &ftModify))
			return E_INVALIDARG;
	}
	if (createTime != 0)
	{
		SYSTEMTIME st;
		if (!VariantTimeToSystemTime(createTime, &st) || !SystemTimeToFileTime(&st, &ftCreate))
			return E_INVALIDARG;
	}
	str = pDir->m_strDirectory;
	if (str.operator LPCWSTR()[str.GetLength() - 1] != L'/')
		str += L'/';
	str += pItem->strFileName;
	hr = GetRoot()->SetFileTime(str, &ftCreate, NULL, &ftModify);
	pDir->Release();
	return hr;
}

STDMETHODIMP CFTPDirectoryBase::UpdateAttributes(VARIANT file, long attr)
{
	CMyStringW str;
	auto hr = _PickFileNameFromVariant(file, str, this);
	if (FAILED(hr))
		return hr;
	CFTPDirectoryBase* pDir;
	auto* pItem = GetFileItem(str, &pDir);
	if (!pItem)
		return STG_E_FILENOTFOUND;
	pDir->Release();
	return hr;
}

STDMETHODIMP CFTPDirectoryBase::CreateShortcut(BSTR LinkName, BSTR TargetName)
{
	return GetRoot()->CreateShortcut(NULL, this, LinkName, TargetName, false);
}

STDMETHODIMP CFTPDirectoryBase::get_FullPath(BSTR* pRet)
{
	if (!pRet)
		return E_POINTER;
	*pRet = MyStringToBSTR(m_strDirectory);
	return *pRet ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CFTPDirectoryBase::get_Url(BSTR* pRet)
{
	if (!pRet)
		return E_POINTER;
	CMyStringW str;
	MakeUrl(CMyStringW(), str);
	*pRet = MyStringToBSTR(str);
	return *pRet ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CFTPDirectoryBase::SynchronizeFrom(LONG_PTR hWndOwner, BSTR bstrSourceDirectory, EasySFTPSynchronizeMode Flags)
{
	IShellFolder* pFolder;
	auto hr = ::SHGetDesktopFolder(&pFolder);
	if (FAILED(hr))
		return hr;
	PIDLIST_RELATIVE pidl = NULL;
	hr = pFolder->ParseDisplayName(reinterpret_cast<HWND>(hWndOwner), NULL, bstrSourceDirectory, NULL, &pidl, NULL);
	if (FAILED(hr))
	{
		pFolder->Release();
		return hr;
	}
	if (!pidl)
	{
		pFolder->Release();
		return E_UNEXPECTED;
	}
	IShellFolder* pChild;
	hr = pFolder->BindToObject(pidl, NULL, IID_IShellFolder, reinterpret_cast<void**>(&pChild));
	::CoTaskMemFree(pidl);
	pFolder->Release();
	if (FAILED(hr))
		return hr;
	hr = SynchronizeFolderFrom(hWndOwner, pChild, Flags);
	pChild->Release();
	return hr;
}

STDMETHODIMP CFTPDirectoryBase::SynchronizeDirectoryFrom(LONG_PTR hWndOwner, IEasySFTPDirectory* pSourceDirectory, EasySFTPSynchronizeMode Flags)
{
	return SynchronizeFolderFrom(hWndOwner, pSourceDirectory, Flags);
}

STDMETHODIMP CFTPDirectoryBase::SynchronizeFolderFrom(LONG_PTR hWndOwner, IUnknown* pSourceShellFolder, EasySFTPSynchronizeMode Flags)
{
	if (!pSourceShellFolder)
		return E_INVALIDARG;
	IShellFolder* pFolder = NULL;
	auto hr = pSourceShellFolder->QueryInterface(IID_IShellFolder, reinterpret_cast<void**>(&pFolder));
	if (FAILED(hr))
		return DISP_E_TYPEMISMATCH;

	hr = EasySFTPSynchronizeFrom(pFolder, this, reinterpret_cast<HWND>(hWndOwner), Flags);

	pFolder->Release();
	return hr;
}

STDMETHODIMP CFTPDirectoryBase::SynchronizeTo(LONG_PTR hWndOwner, BSTR bstrTargetDirectory, EasySFTPSynchronizeMode Flags)
{
	IShellFolder* pFolder;
	auto hr = ::SHGetDesktopFolder(&pFolder);
	if (FAILED(hr))
		return hr;
	PIDLIST_RELATIVE pidl = NULL;
	hr = pFolder->ParseDisplayName(reinterpret_cast<HWND>(hWndOwner), NULL, bstrTargetDirectory, NULL, &pidl, NULL);
	if (FAILED(hr))
	{
		pFolder->Release();
		return hr;
	}
	if (!pidl)
	{
		pFolder->Release();
		return E_UNEXPECTED;
	}
	IShellFolder* pChild;
	hr = pFolder->BindToObject(pidl, NULL, IID_IShellFolder, reinterpret_cast<void**>(&pChild));
	::CoTaskMemFree(pidl);
	pFolder->Release();
	if (FAILED(hr))
		return hr;
	hr = SynchronizeFolderTo(hWndOwner, pChild, Flags);
	pChild->Release();
	return hr;
}

STDMETHODIMP CFTPDirectoryBase::SynchronizeDirectoryTo(LONG_PTR hWndOwner, IEasySFTPDirectory* pTargetDirectory, EasySFTPSynchronizeMode Flags)
{
	return SynchronizeFolderFrom(hWndOwner, pTargetDirectory, Flags);
}

STDMETHODIMP CFTPDirectoryBase::SynchronizeFolderTo(LONG_PTR hWndOwner, IUnknown* pTargetShellFolder, EasySFTPSynchronizeMode Flags)
{
	if (!pTargetShellFolder)
		return E_INVALIDARG;
	IShellFolder* pFolder = NULL;
	auto hr = pTargetShellFolder->QueryInterface(IID_IShellFolder, reinterpret_cast<void**>(&pFolder));
	if (FAILED(hr))
		return DISP_E_TYPEMISMATCH;

	hr = EasySFTPSynchronizeTo(this, pFolder, reinterpret_cast<HWND>(hWndOwner), Flags);

	pFolder->Release();
	return hr;
}

///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFTPDirectoryBase::CreateStream(const OLECHAR* pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStream** ppstm)
{
	auto* pRoot = GetRoot();
	return OpenStream2(pwcsName, grfMode, pRoot->IsTextFile(pwcsName) == S_OK ? pRoot->m_bTextMode : 0, ppstm);
}

STDMETHODIMP CFTPDirectoryBase::OpenStream(const OLECHAR* pwcsName, void* reserved1, DWORD grfMode, DWORD reserved2, IStream** ppstm)
{
	auto* pRoot = GetRoot();
	return OpenStream2(pwcsName, grfMode, pRoot->IsTextFile(pwcsName) == S_OK ? pRoot->m_bTextMode : 0, ppstm);
}

STDMETHODIMP CFTPDirectoryBase::CreateStorage(const OLECHAR* pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStorage** ppstg)
{
	if (!pwcsName || !ppstg)
		return E_POINTER;
	if (grfMode & (STGM_SHARE_DENY_READ | STGM_SHARE_DENY_WRITE | STGM_SHARE_EXCLUSIVE | STGM_PRIORITY | STGM_CONVERT | STGM_TRANSACTED | STGM_NOSCRATCH | STGM_NOSNAPSHOT | STGM_SIMPLE | STGM_DIRECT_SWMR | STGM_DELETEONRELEASE))
		return STG_E_INVALIDFLAG;
	auto* pRoot = GetRoot();
	if ((grfMode & STGM_CREATE) == STGM_FAILIFTHERE)
	{
		if (pRoot->IsDirectoryExists(NULL, this, pwcsName) == S_OK)
			return STG_E_FILEALREADYEXISTS;
	}
	auto hr = pRoot->CreateFTPDirectory(NULL, this, pwcsName);
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
		hr = GetRoot()->CreateFTPDirectory(NULL, this, pwcsName);
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
		if (!GetRoot()->ReceiveDirectory(m_hWndOwnerCache, this, m_strDirectory, &m_bDirReceived))
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
	auto* pMyRoot = GetRoot();
	IEasySFTPDirectory* pFTPDir;
	if (SUCCEEDED(pstgDest->QueryInterface(IID_IEasySFTPDirectory, reinterpret_cast<void**>(&pFTPDir))))
	{
		IEasySFTPRootDirectory* pRoot;
		if (SUCCEEDED(pFTPDir->get_RootDirectory(&pRoot)))
		{
			EasySFTPConnectionMode mode = EasySFTPConnectionMode::SFTP;
			long nPort = 0;
			BSTR hostName = NULL;
			if (SUCCEEDED(pRoot->get_ConnectionMode(&mode)) && SUCCEEDED(pRoot->get_HostName(&hostName)) &&
				SUCCEEDED(pRoot->get_Port(&nPort)))
			{
				auto isSameHost = pMyRoot->m_nPort == nPort &&
					pMyRoot->m_strHostName.Compare(hostName, true) == 0 &&
					pMyRoot->GetProtocol() == mode;
				::SysFreeString(hostName);
				BSTR bstrDir;
				if (isSameHost && SUCCEEDED(pFTPDir->get_FullPath(&bstrDir)))
				{
					CMyStringW strFromName = m_strDirectory;
					if (((LPCWSTR)strFromName)[strFromName.GetLength() - 1] != L'/')
						strFromName += L'/';
					strFromName += pwcsName;
					CMyStringW strToName;
					MyBSTRToString(bstrDir, strToName);
					::SysFreeString(bstrDir);
					if (((LPCWSTR)strToName)[strToName.GetLength() - 1] != L'/')
						strToName += L'/';
					strToName += pwcsNewName;
					auto hr = pMyRoot->RenameFTPItem(strFromName, strToName);
					pFTPDir->Release();
					return hr;
				}
			}
			pRoot->Release();
		}
		pFTPDir->Release();
	}

	if (!m_bDirReceived)
	{
		if (!pMyRoot->ReceiveDirectory(m_hWndOwnerCache, this, m_strDirectory, &m_bDirReceived))
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
	if (pFile->IsDirectory())
	{
		hr = pMyRoot->DoDeleteDirectoryRecursive(NULL, astrMsgs, pFile->strFileName, this);
		if (FAILED(hr))
			return hr;
	}
	return pMyRoot->DoDeleteFileOrDirectory(NULL, astrMsgs, pFile->IsDirectory(), pFile->strFileName, this);
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
	auto* pRoot = GetRoot();
	if (!m_bDirReceived)
	{
		if (!pRoot->ReceiveDirectory(m_hWndOwnerCache, this, m_strDirectory, &m_bDirReceived))
		{
			return STG_E_FILENOTFOUND;
		}
	}
	auto* pEnum = new CEnumFTPItemStatstg(m_pMallocData, m_aFiles, pRoot->IsLockSupported(), static_cast<IShellFolder*>(this));
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
		if (!GetRoot()->ReceiveDirectory(m_hWndOwnerCache, this, m_strDirectory, &m_bDirReceived))
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
	CMyStringW strFromName;
	if (*pwcsOldName != L'/')
	{
		strFromName = m_strDirectory;
		if (((LPCWSTR)strFromName)[strFromName.GetLength() - 1] != L'/')
			strFromName += L'/';
	}
	strFromName += pwcsOldName;
	CMyStringW strToName;
	if (*pwcsNewName != L'/')
	{
		strToName = m_strDirectory;
		if (((LPCWSTR)strToName)[strToName.GetLength() - 1] != L'/')
			strToName += L'/';
	}
	strToName += pwcsNewName;
	return GetRoot()->RenameFTPItem(strFromName, strToName);
}

STDMETHODIMP CFTPDirectoryBase::SetElementTimes(const OLECHAR* pwcsName, const FILETIME* pctime, const FILETIME* patime, const FILETIME* pmtime)
{
	CMyStringW strFromName = m_strDirectory;
	if (((LPCWSTR)strFromName)[strFromName.GetLength() - 1] != L'/')
		strFromName += L'/';
	strFromName += pwcsName;
	return GetRoot()->SetFileTime(strFromName, pctime, patime, pmtime);
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
	return GetRoot()->StatDirectory(this, m_grfMode, pstatstg, grfStatFlag);
}

///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFTPDirectoryBase::GetRootDirectory(IEasySFTPOldDirectory** ppRootDirectory)
{
	return GetRoot()->QueryInterface(IID_IEasySFTPOldDirectory, (void**)ppRootDirectory);
}

///////////////////////////////////////////////////////////////////////////////

HRESULT CFTPDirectoryBase::CreateStream(CFTPFileItem* pItem, IStream** ppStream)
{
	auto* pRoot = GetRoot();
	HRESULT hr = pRoot->CreateFTPItemStream(this, pItem, ppStream);
	if (FAILED(hr))
		return hr;
	if (!TEXTMODE_IS_NO_CONVERTION(pRoot->m_bTextMode))
	{
		hr = IsTextFile(pItem->strFileName);
		if (SUCCEEDED(hr) && hr == S_OK)
		{
			IStream* pstm2;
			hr = MyCreateTextStream(*ppStream, TEXTMODE_FOR_RECV(pRoot->m_bTextMode), &pstm2);
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

HRESULT CFTPDirectoryBase::OpenStream2(const OLECHAR* pwcsName, DWORD grfMode, BYTE bTextMode, IStream** ppstm, IEasySFTPFile** ppFile)
{
	auto* pRoot = GetRoot();
	auto* pStream = new CFTPFileStream(pRoot);
	if (!pStream)
		return E_OUTOFMEMORY;
	HANDLE handle;
	IEasySFTPFile* pFile = NULL;
	auto hr = pRoot->OpenFile(this, pwcsName, grfMode, &handle, &pFile);
	if (FAILED(hr))
	{
		delete pStream;
		return hr;
	}
	pStream->SetHandle(handle);
	BYTE bMode = 0;
	if (grfMode & (STGM_READWRITE | STGM_WRITE))
		bMode = TEXTMODE_FOR_SEND(bTextMode);
	if (grfMode & (STGM_READWRITE | STGM_READ))
		bMode |= TEXTMODE_FOR_RECV(bTextMode);
	if (!TEXTMODE_IS_NO_CONVERTION(bMode))
	{
		hr = MyCreateTextStream(pStream, bMode, ppstm);
		pStream->Release();
	}
	else
	{
		*ppstm = pStream;
		hr = S_OK;
	}
	if (SUCCEEDED(hr) && ppFile)
	{
		*ppFile = pFile;
	}
	else
	{
		pFile->Release();
	}
	return hr;
}

HRESULT CFTPDirectoryBase::DeleteFTPItem(CFTPFileItem* pItem)
{
	CMyPtrArrayT<CFTPFileItem> aItems;
	aItems.Add(pItem);
	pItem->AddRef();
	auto hr = GetRoot()->DoDeleteFTPItems(m_hWndOwnerCache, this, aItems);
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

void CFTPDirectoryBase::MakeUrl(const CMyStringW& strFileName, CMyStringW& rstrUrl, bool* pbLastIsNotDelimiter)
{
	auto* pRoot = GetRoot();
	CMyStringW strHost(pRoot->m_strHostName); // avoid multithreaded-free for LPCSTR
	CMyStringW str2;
	int nDefPort = 0;
	pRoot->GetProtocol(&nDefPort);
	rstrUrl = pRoot->GetProtocolName();
	rstrUrl += L"://";
	GetHostNameForUrl(strHost, str2);
	rstrUrl += str2;
	if (pRoot->m_nPort != nDefPort)
	{
		str2.Format(L"%d", pRoot->m_nPort);
		rstrUrl += L':';
		rstrUrl += str2;
	}
	rstrUrl += m_strDirectory;
	if (((LPCWSTR)m_strDirectory)[m_strDirectory.GetLength() - 1] != L'/')
		rstrUrl += L'/';
	if (pbLastIsNotDelimiter)
		*pbLastIsNotDelimiter = !strFileName.IsEmpty();
	if (!strFileName.IsEmpty())
		rstrUrl += strFileName;
}

STDMETHODIMP CFTPDirectoryBase::ParseDisplayName2(PIDLIST_RELATIVE pidlParent,
	HWND hWnd,
	LPBC pbc,
	LPWSTR pszDisplayName,
	ULONG* pchEaten,
	PIDLIST_RELATIVE* ppidl,
	ULONG* pdwAttributes)
{
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
			{
				bool bIsDirectory = false;
				for (int i = 0; i < m_aDirectories.GetCount(); ++i)
				{
					if (m_aDirectories[i]->strName.Compare(strName) == 0)
					{
						bIsDirectory = true;
						break;
					}
				}
				pidlChild = ::CreateDummyFileItem(m_pMallocData->pMalloc, strName, bIsDirectory || *pszDisplayName == L'/');
			}
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
		pidlCurrent = pidlParent ? (PIDLIST_RELATIVE) ::DuplicateItemIDList((PCUIDLIST_ABSOLUTE)pidlParent) : ::MakeNullIDList();
	*ppidl = pidlCurrent;
	return *ppidl ? S_OK : E_OUTOFMEMORY;
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
		pOldItem->DetachParent();
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
		pDirItem->strName = str;
		pDirItem->strRealPath = m_strDirectory;
		if ((m_strDirectory.operator LPCWSTR())[m_strDirectory.GetLength() - 1] != L'/')
			pDirItem->strRealPath += L'/';
		pDirItem->strRealPath += str;
	}
	CFTPDirectoryBase* pDirectory = NULL;
	if (pDirItem->pDirectory)
		pDirectory = pDirItem->pDirectory;
	else
	{
		auto* pRoot = GetRoot();
		PITEMID_CHILD pidlItem;
		if (pItem)
			pidlItem = ::CreateFileItem(pRoot->m_pMallocData->pMalloc, pItem);
		else
			pidlItem = ::CreateDummyFileItem(pRoot->m_pMallocData->pMalloc, str, true);
		if (!pidlItem)
		{
			if (!bFound)
				pDirItem->Release();
			return E_OUTOFMEMORY;
		}
		if (m_strDirectory.GetLength() != 1 || *((LPCWSTR)m_strDirectory) != L'/')
			str.InsertChar(L'/', 0);
		str.InsertString(m_strDirectory, 0);
		HRESULT hr = CreateInstance(pDirItem, this, str, &pDirectory);
		if (FAILED(hr))
		{
			::CoTaskMemFree(pidlItem);
			if (!bFound)
				pDirItem->Release();
			return E_OUTOFMEMORY;
		}
		{
			::EnterCriticalSection(&m_csPidlMe);
			PIDLIST_ABSOLUTE pidlChild = ::AppendItemIDList(m_pidlMe, pidlItem);
			::LeaveCriticalSection(&m_csPidlMe);
			hr = pDirectory->Initialize(pidlChild);
			::CoTaskMemFree(pidlChild);
			::CoTaskMemFree(pidlItem);
			if (FAILED(hr))
			{
				pDirectory->Release();
				if (!bFound)
					pDirItem->Release();
				return E_OUTOFMEMORY;
			}
		}
		pDirectory->SetHwndOwnerCache(GetHwndOwnerCache());
		pDirItem->pDirectory = pDirectory;
	}

	if (lpszRelativePath)
	{
		HRESULT hr = pDirectory->OpenNewDirectory(lpszRelativePath, ppDirectory);
		if (SUCCEEDED(hr))
		{
			if (!bFound)
				m_aDirectories.Add(pDirItem);
		}
		else
		{
			if (!bFound)
			{
				pDirectory->Release();
				pDirItem->Release();
			}
		}
		return hr;
	}
	else
	{
		*ppDirectory = pDirectory;
		pDirectory->AddRef();
		if (!bFound)
			m_aDirectories.Add(pDirItem);
		return S_OK;
	}
}

CFTPDirectoryItem* CFTPDirectoryBase::GetAlreadyOpenedDirectory(PCUIDLIST_RELATIVE pidlChild, PCUIDLIST_RELATIVE* ppidlNext)
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
			if (pDirItem->pDirectory && pidlChild->mkid.cb)
				return pDirItem->pDirectory->GetAlreadyOpenedDirectory(pidlChild, ppidlNext);
			*ppidlNext = pidlChild->mkid.cb ? pidlChild : NULL;
			return pDirItem;
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
		auto* pRoot = GetRoot();
		if (this != pRoot)
		{
			return pRoot->GetFileItem(lpszRelativeName + 1, ppParentDirectory);
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
			CFTPDirectoryBase* pDir;
			auto hr = OpenNewDirectory(str, &pDir);
			if (FAILED(hr))
				return NULL;
			auto* p = pDir->GetFileItem(lpszRelativeName, ppParentDirectory);
			pDir->Release();
			return p;
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
			CFTPDirectoryBase* pDir;
			auto hr = OpenNewDirectory(str, &pDir);
			if (FAILED(hr))
				return NULL;
			auto* p = pDir->GetFileItem(pidlChild, ppParentDirectory);
			pDir->Release();
			return p;
		}
	}
	return NULL;
}

void CFTPDirectoryBase::NotifyUpdate(LONG wEventId, LPCWSTR lpszFile1, LPCWSTR lpszFile2)
{
	auto* pRoot = GetRoot();
	PIDLIST_RELATIVE pidlR1 = lpszFile1 ? ::CreateFullPathFileItem(m_pMallocData->pMalloc, lpszFile1) : NULL;
	PIDLIST_RELATIVE pidlR2 = lpszFile2 ? ::CreateFullPathFileItem(m_pMallocData->pMalloc, lpszFile2) : NULL;
	PIDLIST_ABSOLUTE pidl1;
	PIDLIST_ABSOLUTE pidl2;
	if (!pidlR1)
		pidl1 = NULL;
	else if (*lpszFile1 == L'/')
	{
		::EnterCriticalSection(&pRoot->m_csPidlMe);
		pidl1 = ::AppendItemIDList(pRoot->m_pidlMe, pidlR1);
		::LeaveCriticalSection(&pRoot->m_csPidlMe);
	}
	else
	{
		::EnterCriticalSection(&m_csPidlMe);
		pidl1 = ::AppendItemIDList( m_pidlMe, pidlR1);
		::LeaveCriticalSection(&m_csPidlMe);
	}
	if (!pidlR2)
		pidl2 = NULL;
	else if (*lpszFile2 == L'/')
	{
		::EnterCriticalSection(&pRoot->m_csPidlMe);
		pidl2 = ::AppendItemIDList(pRoot->m_pidlMe, pidlR2);
		::LeaveCriticalSection(&pRoot->m_csPidlMe);
	}
	else
	{
		::EnterCriticalSection(&m_csPidlMe);
		pidl2 = ::AppendItemIDList( m_pidlMe, pidlR2);
		::LeaveCriticalSection(&m_csPidlMe);
	}
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
		CFTPDirectoryBase* pParent = lpszFile1 && *lpszFile1 == L'/' ? pRoot : this;
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
	p = GetRoot()->RetrieveFileItem(this, lpszFileName);
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
	bool isExtensionChanged = false;
	if (!bDirectory)
	{
		auto* pExtOld = wcsrchr(lpszFileName, L'.');
		auto* pExtNew = wcsrchr(lpszNewFileName, L'.');
		if (!pExtOld)
			isExtensionChanged = (pExtNew != NULL);
		else if (!pExtNew)
			isExtensionChanged = true;
		else
			isExtensionChanged = _wcsicmp(pExtOld, pExtNew) != 0;
	}
	CMyStringW strFP(lpszFromDir), strTP(m_strDirectory);
	if (m_strDirectory.Compare(lpszFromDir) != 0)
	{
		auto* pRoot = GetRoot();
		CFTPFileItem* p = pRoot->RetrieveFileItem(this, lpszNewFileName);
		if (p)
		{
			::EnterCriticalSection(&m_csFiles);
			m_aFiles.Add(p);
			::LeaveCriticalSection(&m_csFiles);
		}
		CFTPDirectoryBase* pDir;
		if (SUCCEEDED(pRoot->OpenNewDirectory(lpszFromDir + 1, &pDir)))
		{
			for (auto i = 0; i < pDir->m_aFiles.GetCount(); ++i)
			{
				CFTPFileItem* p = pDir->m_aFiles.GetItem(i);
				if (p->strFileName.Compare(lpszFileName) == 0)
				{
					pDir->m_aFiles.RemoveItem(i);
					p->DetachParent();
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
		{
			p->strFileName = lpszNewFileName;
			if (isExtensionChanged)
			{
				p->iIconIndex = -1;
				FillFileItemInfo(p);
			}
		}
		::LeaveCriticalSection(&m_csFiles);
	}
	if (strFP.IsEmpty() || ((LPCWSTR)strFP)[strFP.GetLength() - 1] != L'/')
		strFP += L'/';
	if (strTP.IsEmpty() || ((LPCWSTR)strTP)[strTP.GetLength() - 1] != L'/')
		strTP += L'/';
	strFP += lpszFileName;
	strTP += lpszNewFileName;
	NotifyUpdate(bDirectory ? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM, strFP, strTP);
	if (isExtensionChanged)
		NotifyUpdate(SHCNE_UPDATEITEM, strTP, NULL);
}

void CFTPDirectoryBase::UpdateRenameFile(LPCWSTR lpszOldFileName, LPCWSTR lpszNewFileName, bool bDirectory)
{
	bool isExtensionChanged = false;
	if (!bDirectory)
	{
		auto* pExtOld = wcsrchr(lpszOldFileName, L'.');
		auto* pExtNew = wcsrchr(lpszNewFileName, L'.');
		if (!pExtOld)
			isExtensionChanged = (pExtNew != NULL);
		else if (!pExtNew)
			isExtensionChanged = true;
		else
			isExtensionChanged = _wcsicmp(pExtOld, pExtNew) != 0;
	}
	CMyStringW strFP(lpszOldFileName);
	//CMyStringW strFP(m_strDirectory), strTP(m_strDirectory);
	::EnterCriticalSection(&m_csFiles);
	CFTPFileItem* p = GetFileItem(lpszOldFileName);
	if (p)
	{
		p->strFileName = lpszNewFileName;
		if (isExtensionChanged)
		{
			p->iIconIndex = -1;
			FillFileItemInfo(p);
		}
	}
	::LeaveCriticalSection(&m_csFiles);
	NotifyUpdate(bDirectory ? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM, strFP, lpszNewFileName);
	if (isExtensionChanged)
		NotifyUpdate(SHCNE_UPDATEITEM, lpszNewFileName, NULL);
}

void CFTPDirectoryBase::UpdateFileAttrs(LPCWSTR lpszFileName, bool bDirectory)
{
	CMyStringW str(lpszFileName);
	auto* pRoot = GetRoot();
	::EnterCriticalSection(&m_csFiles);
	for (int i = 0; i < m_aFiles.GetCount(); i++)
	{
		CFTPFileItem* p = m_aFiles.GetItem(i);
		if (p->strFileName.Compare(lpszFileName) == 0)
		{
			CFTPFileItem* p2 = pRoot->RetrieveFileItem(this, lpszFileName);
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
			p->DetachParent();
			p->Release();
			break;
		}
	}
	::LeaveCriticalSection(&m_csFiles);
	if (bDirectory)
	{
		for (int i = 0; i < m_aDirectories.GetCount(); i++)
		{
			auto* pDirItem = m_aDirectories.GetItem(i);
			if (pDirItem->strName.Compare(str) == 0)
			{
				m_aDirectories.RemoveItem(i);
				if (pDirItem->pDirectory)
					pDirItem->pDirectory->DetachAndRelease();
				pDirItem->Release();
				break;
			}
		}
	}
	NotifyUpdate(bDirectory ? SHCNE_RMDIR : SHCNE_DELETE, str, NULL);
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
