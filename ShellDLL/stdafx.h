// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーから使用されていない部分を除外します。
#define STRICT_TYPED_ITEMIDS

#define EASYSFTP_SHELLDLL

// Windows ヘッダー ファイル:
#include <windows.h>

#include <commdlg.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <shldisp.h>
#include <commoncontrols.h>
#include <thumbcache.h>
#include <propkey.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <wspiapi.h>


#ifdef __IDataObjectAsyncCapability_INTERFACE_DEFINED__
typedef IDataObjectAsyncCapability IAsyncOperation;
#define IID_IAsyncOperation IID_IDataObjectAsyncCapability
#endif

// TODO: プログラムに必要な追加ヘッダーをここで参照してください。

// C ランタイム ヘッダー ファイル
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <wchar.h>
#include <math.h>
#include <limits.h>
#include <process.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#endif
#include <crtdbg.h>
#ifdef _DEBUG
#define DEBUG_NEW   new(_CLIENT_BLOCK, __FILE__, __LINE__)

//#define malloc(size) _malloc_dbg((size), _NORMAL_BLOCK, __FILE__, __LINE__)
//#define realloc(memory, size) _realloc_dbg((memory), (size), _NORMAL_BLOCK, __FILE__, __LINE__)
//#define free(memory) _free_dbg((memory), _NORMAL_BLOCK)
#endif

// OpenSSL
#include <openssl/opensslv.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/hmac.h>

// libssh2
#include <libssh2.h>
#include <libssh2_sftp.h>

#ifndef CDSIZEOF_STRUCT
#define CDSIZEOF_STRUCT(structname, member)      (((int)((LPBYTE)(&((structname*)0)->member) - ((LPBYTE)((structname*)0)))) + sizeof(((structname*)0)->member))
#endif

#define CDSIZEOF_STRUCT2(structname, afterMember) ((size_t)((LPBYTE)(&((structname*)0)->afterMember) - ((LPBYTE)((structname*)0))))

#if defined(__cplusplus) && !defined(NO_TEMPLATE_ROUNDUP)
template <class T> inline T __stdcall ROUNDUP(T val, T align)
	{ return align * ((T) ((val + align - 1) / align)); }
#else
#define ROUNDUP(val, align) \
	(align) * (((val) + (align) - 1) / (align))
#endif

#if (WINVER >= 0x0500)
#define MENUITEMINFO_SIZE_V1A          CDSIZEOF_STRUCT2(MENUITEMINFOA, hbmpItem)
#define MENUITEMINFO_SIZE_V1W          CDSIZEOF_STRUCT2(MENUITEMINFOW, hbmpItem)
#else
#define MENUITEMINFO_SIZE_V1A          sizeof(MENUITEMINFOW)
#define MENUITEMINFO_SIZE_V1W          sizeof(MENUITEMINFOA)
#endif // (WINVER >= 0x0500)

#ifdef UNICODE
#define MENUITEMINFO_SIZE_V1           MENUITEMINFO_SIZE_V1W
#else
#define MENUITEMINFO_SIZE_V1           MENUITEMINFO_SIZE_V1A
#endif // !UNICODE

#if (WINVER >= 0x0600)
#define NONCLIENTMETRICS_SIZE_V1   CDSIZEOF_STRUCT2(NONCLIENTMETRICS, iPaddedBorderWidth)
#else
#define NONCLIENTMETRICS_SIZE_V1   sizeof(NONCLIENTMETRICS)
#endif

// TODO: プログラムに必要な追加ヘッダーをここで参照してください。

#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
inline void* operator new (size_t, void* pv){ return pv; }
inline void operator delete (void*, void*) { }
#endif
