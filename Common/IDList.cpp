/*
 Copyright (C) 2010 Kuri-Applications

 IDList.cpp - implementations of utility functions for ITEMIDLIST
 */

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern "C" SIZE_T __stdcall GetItemIDListSize(PCUIDLIST_RELATIVE lpidl)
{
	SIZE_T nSize;
	if (!lpidl)
		return NULL;

	nSize = 0;
	while (lpidl->mkid.cb)
	{
		nSize += lpidl->mkid.cb;
		lpidl = (PCUIDLIST_RELATIVE) (((const BYTE*) lpidl) + lpidl->mkid.cb);
	}
	nSize += sizeof(lpidl->mkid.cb);
	return nSize;
}

extern "C" PIDLIST_RELATIVE __stdcall DuplicateItemIDList(PCUIDLIST_RELATIVE lpidl)
{
	PIDLIST_RELATIVE ret;
	SIZE_T nSize;

	if (!lpidl)
		return NULL;

	nSize = GetItemIDListSize(lpidl);
	ret = (PIDLIST_RELATIVE) ::CoTaskMemAlloc(nSize);
	if (!ret)
		return ret;
	memcpy(ret, lpidl, nSize);
	return ret;
}

extern "C" PIDLIST_ABSOLUTE __stdcall AppendItemIDList(PCUIDLIST_ABSOLUTE lpidlRoot, PCUIDLIST_RELATIVE lpNew)
{
	LPCITEMIDLIST lpLast;
	PIDLIST_ABSOLUTE ret;
	SIZE_T nSize, nSizeNew;

	nSize = 0;
	lpLast = lpidlRoot;
	while (lpLast->mkid.cb)
	{
		nSize += lpLast->mkid.cb;
		lpLast = (PCUIDLIST_ABSOLUTE) (((const BYTE*) lpLast) + lpLast->mkid.cb);
	}
	lpLast = lpNew;
	nSizeNew = nSize;
	while (lpLast->mkid.cb)
	{
		nSize += lpLast->mkid.cb;
		//nSizeNew += lpLast->mkid.cb;
		lpLast = (PCUIDLIST_ABSOLUTE) (((const BYTE*) lpLast) + lpLast->mkid.cb);
	}
	nSize += sizeof(lpLast->mkid.cb);
	ret = (PIDLIST_ABSOLUTE) ::CoTaskMemAlloc(nSize);
	if (!ret)
		return ret;
	//memcpy(ret, lpNew, nSizeNew);
	//memcpy(((LPBYTE) ret) + nSizeNew, lpidlRoot, nSize - nSizeNew);
	memcpy(ret, lpidlRoot, nSizeNew);
	memcpy(((LPBYTE) ret) + nSizeNew, lpNew, nSize - nSizeNew);
	return ret;
}

extern "C" PIDLIST_ABSOLUTE __stdcall RemoveOneChild(PCUIDLIST_ABSOLUTE lpidl)
{
	LPCITEMIDLIST lpLast, lpNext;
	PIDLIST_ABSOLUTE ret;
	SIZE_T nSize;

	if (!lpidl->mkid.cb)
		return NULL;

	nSize = 0;
	lpLast = NULL;
	lpNext = lpidl;
	do
	{
		if (lpLast)
			nSize += lpLast->mkid.cb;
		lpLast = lpNext;
		lpNext = (LPCITEMIDLIST) (((const BYTE*) lpLast) + lpLast->mkid.cb);
	} while (lpNext->mkid.cb);

	nSize += sizeof(lpLast->mkid.cb);
	ret = (PIDLIST_ABSOLUTE) ::CoTaskMemAlloc(nSize);
	if (!ret)
		return ret;
	memcpy(ret, lpidl, nSize - sizeof(lpLast->mkid.cb));
	*(LPWORD)(((LPBYTE) ret) + nSize - sizeof(lpLast->mkid.cb)) = 0;
	return ret;
}

extern "C" PITEMID_CHILD __stdcall GetChildItemIDList(PCUIDLIST_ABSOLUTE lpidl)
{
	PCUIDLIST_RELATIVE lpidlNow;
	PCUIDLIST_RELATIVE lpNext;
	PITEMID_CHILD ret;
	SIZE_T nSize;

	if (!lpidl->mkid.cb)
		return NULL;

	lpidlNow = lpidl;
	lpNext = lpidl;
	do
	{
		lpidlNow = (PCUIDLIST_ABSOLUTE) lpNext;
		lpNext = (PCUIDLIST_RELATIVE) (((const BYTE*) lpNext) + lpNext->mkid.cb);
	} while (lpNext->mkid.cb);
	nSize = sizeof(USHORT) + lpidlNow->mkid.cb;
	ret = (PITEMID_CHILD) ::CoTaskMemAlloc(nSize);
	memcpy(ret, lpidlNow, lpidlNow->mkid.cb);
	((LPITEMIDLIST) ((LPBYTE) ret + lpidlNow->mkid.cb))->mkid.cb = 0;

	return ret;
}

extern "C" bool __stdcall IsEqualIDList(PCUIDLIST_RELATIVE lpidl1, PCUIDLIST_RELATIVE lpidl2)
{
	while (true)
	{
		if (lpidl1->mkid.cb != lpidl2->mkid.cb)
			return false;
		if (!lpidl1->mkid.cb)
			break;
		if (memcmp(lpidl1, lpidl2, (size_t) lpidl1->mkid.cb) != 0)
			return false;
		lpidl1 = (PCUIDLIST_RELATIVE) (((const BYTE UNALIGNED*) lpidl1) + lpidl1->mkid.cb);
		lpidl2 = (PCUIDLIST_RELATIVE) (((const BYTE UNALIGNED*) lpidl2) + lpidl2->mkid.cb);
	}
	return true;
}

extern "C" bool __stdcall IsChildIDList(PCUIDLIST_RELATIVE lpidlParent, PCUIDLIST_RELATIVE lpidlChild)
{
	if (!lpidlParent->mkid.cb)
		return true;
	while (true)
	{
		if (!lpidlChild->mkid.cb)
			break;
		if (lpidlParent->mkid.cb != lpidlChild->mkid.cb)
			return false;
		if (!lpidlParent->mkid.cb)
			return false;
		if (memcmp(lpidlParent, lpidlChild, (size_t) lpidlParent->mkid.cb) != 0)
			return false;
		lpidlParent = (PCUIDLIST_RELATIVE) (((const BYTE UNALIGNED*) lpidlParent) + lpidlParent->mkid.cb);
		lpidlChild = (PCUIDLIST_RELATIVE) (((const BYTE UNALIGNED*) lpidlChild) + lpidlChild->mkid.cb);
	}
	return true;
}

EXTERN_C bool __stdcall IsMatchParentIDList(PCIDLIST_ABSOLUTE lpidlParent, PCIDLIST_ABSOLUTE lpidlTarget)
{
	register PCUIDLIST_RELATIVE pidlP = (PCUIDLIST_RELATIVE) lpidlParent;
	register PCUIDLIST_RELATIVE pidlT = (PCUIDLIST_RELATIVE) lpidlTarget;
	while (true)
	{
		if (!pidlP->mkid.cb)
			break;
		if (pidlP->mkid.cb != pidlT->mkid.cb)
			return false;
		if (memcmp(pidlP, pidlT, (size_t) pidlP->mkid.cb) != 0)
			return false;
		pidlP = (PCUIDLIST_RELATIVE) (((const BYTE UNALIGNED*) pidlP) + pidlP->mkid.cb);
		pidlT = (PCUIDLIST_RELATIVE) (((const BYTE UNALIGNED*) pidlT) + pidlT->mkid.cb);
	}
	return true;
}

EXTERN_C PCUIDLIST_RELATIVE __stdcall PickupRelativeIDList(PCIDLIST_ABSOLUTE lpidlParent, PCIDLIST_ABSOLUTE lpidlTarget)
{
	register PCUIDLIST_RELATIVE pidlP = (PCUIDLIST_RELATIVE) lpidlParent;
	register PCUIDLIST_RELATIVE pidlT = (PCUIDLIST_RELATIVE) lpidlTarget;
	while (true)
	{
		if (!pidlP->mkid.cb)
			break;
		if (pidlP->mkid.cb != pidlT->mkid.cb)
			return NULL;
		if (memcmp(pidlP, pidlT, (size_t) pidlP->mkid.cb) != 0)
			return NULL;
		pidlP = (PCUIDLIST_RELATIVE) (((const BYTE UNALIGNED*) pidlP) + pidlP->mkid.cb);
		pidlT = (PCUIDLIST_RELATIVE) (((const BYTE UNALIGNED*) pidlT) + pidlT->mkid.cb);
	}
	if (pidlT->mkid.cb)
		return ::DuplicateItemIDList(pidlT);
	return NULL;
}

EXTERN_C bool __stdcall IsMyComputerIDList(PCUIDLIST_ABSOLUTE lpidl)
{
	PIDLIST_ABSOLUTE lpidlComputer;
	HRESULT hr;

	hr = ::SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &lpidlComputer);
	if (FAILED(hr))
		return false;

	hr = (HRESULT) IsEqualIDList(lpidl, lpidlComputer);
	::CoTaskMemFree(lpidlComputer);
	return hr != 0;
}
