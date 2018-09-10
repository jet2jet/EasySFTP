/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 IDList.h - declarations of utility functions for ITEMIDLIST
 */

#pragma once

EXTERN_C inline bool __stdcall IsRootIDList(PCUIDLIST_ABSOLUTE lpidl)
	{ return lpidl ? !lpidl->mkid.cb : false; }
#define IsDesktopIDList IsRootIDList
EXTERN_C SIZE_T __stdcall GetItemIDListSize(PCUIDLIST_RELATIVE lpidl);
EXTERN_C PIDLIST_RELATIVE __stdcall DuplicateItemIDList(PCUIDLIST_RELATIVE lpidl);
EXTERN_C PIDLIST_ABSOLUTE __stdcall AppendItemIDList(PCUIDLIST_ABSOLUTE lpidlRoot, PCUIDLIST_RELATIVE lpNew);
EXTERN_C PIDLIST_ABSOLUTE __stdcall RemoveOneChild(PCUIDLIST_ABSOLUTE lpidl);
EXTERN_C PITEMID_CHILD __stdcall GetChildItemIDList(PCUIDLIST_ABSOLUTE lpidl);
EXTERN_C bool __stdcall IsEqualIDList(PCUIDLIST_RELATIVE lpidl1, PCUIDLIST_RELATIVE lpidl2);
EXTERN_C bool __stdcall IsChildIDList(PCUIDLIST_RELATIVE lpidlParent, PCUIDLIST_RELATIVE lpidlChild);
EXTERN_C bool __stdcall IsMatchParentIDList(PCIDLIST_ABSOLUTE lpidlParent, PCIDLIST_ABSOLUTE lpidlTarget);
EXTERN_C PIDLIST_RELATIVE __stdcall PickupRelativeIDList(PCIDLIST_ABSOLUTE lpidlParent, PCIDLIST_ABSOLUTE lpidlTarget);
EXTERN_C inline bool __stdcall IsSingleIDList(PCUIDLIST_RELATIVE lpidl)
{
	return lpidl ?
		(lpidl->mkid.cb && !((PCUIDLIST_RELATIVE) (((const BYTE*) lpidl) + lpidl->mkid.cb))->mkid.cb) :
		false;
}

EXTERN_C bool __stdcall IsMyComputerIDList(PCUIDLIST_ABSOLUTE lpidl);
