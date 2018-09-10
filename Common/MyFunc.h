/*
 Copyright (C) 2010 Kuri-Applications

 MyFunc.h - declarations of useful functions
 */

#ifndef __MYFUNC_H__
#define __MYFUNC_H__

#ifdef __CCL_H__
#ifndef IDS_APP_TITLE
#define IDS_APP_TITLE 101
#endif
#endif

#ifndef __UPDOWN_H__
#define CUDF_DECIMAL            0
#define CUDF_HEXADECIMAL        1
#define CUDF_HEXADECIMAL_UPPER  2
#define CUDF_OCTADECIMAL        3
#endif
#ifndef MAKECTYPE
#define MAKECTYPE(cudf, use_prefix)    ((cudf) | ((use_prefix) ? 0x10 : 0))
#endif

#ifdef __CCL_H__
void __stdcall SyncDialogData(HWND hWnd, int nID, CCCLString& rString, bool bGet);
#endif
void __stdcall SyncDialogData(HWND hWnd, int nID, int& nInt, bool bGet);
void __stdcall SyncDialogData(HWND hWnd, int nID, DWORD& dwDWord, bool bGet);
void __stdcall SyncDialogData(HWND hWnd, int nID, DWORD& dwDWord, bool bGet, int nMinCount, BYTE cType = MAKECTYPE(CUDF_HEXADECIMAL_UPPER, 1));
void __stdcall SyncDialogData(HWND hWnd, int nID, WORD& wWord, bool bGet);
void __stdcall SyncDialogData(HWND hWnd, int nID, BYTE& bByte, bool bGet);
void __stdcall SyncDialogData(HWND hWnd, int nID, DWORD& dw, DWORD dwBit, bool bGet);
void __stdcall SyncDialogData(HWND hWnd, int nID, BYTE& by, BYTE byBit, bool bGet);
void __stdcall SyncDialogData(HWND hWnd, int nID, bool& b, bool bGet);

// 以下の文字列関数の戻り値はすべて NULL 文字を含まないが、
// 必要なバッファのサイズは NULL 文字を含む

HANDLE __stdcall MyCreateFile(LPCTSTR lpszFileName, DWORD dwDesiredAccess, DWORD dwShareMode = 0,
	DWORD dwCreationDisposition = CREATE_ALWAYS, DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL);
HANDLE __stdcall MyOpenFile(LPCTSTR lpszFileName, DWORD dwDesiredAccess, DWORD dwShareMode = 0,
	DWORD dwCreationDisposition = OPEN_EXISTING, DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL);
#ifdef __CCL_H__
int __stdcall MyMessageBox(HWND hWnd, LPCTSTR lpszText, LPCTSTR lpszCaption = NULL, UINT uType = MB_OK);
#endif
extern "C" BOOL __stdcall EnableDlgItem(HWND hDlg, int nIDDlgItem, BOOL bEnable);
extern "C" int __stdcall GetDlgItemTextLen(HWND hDlg, int nIDDlgItem);
extern "C" HWND __stdcall SetDlgItemFocus(HWND hDlg, int nIDDlgItem);
extern "C" BOOL __stdcall IsExistFile(LPCTSTR lpszFileName);
extern "C" int __stdcall MyGetFullPathA(LPCSTR lpszPath, LPCSTR lpszFile, LPSTR lpszBuffer, int nMaxLen);
extern "C" int __stdcall MyGetFullPathW(LPCWSTR lpszPath, LPCWSTR lpszFile, LPWSTR lpszBuffer, int nMaxLen);
extern "C" LPSTR __stdcall MyGetFullPath2A(LPCSTR lpszPath, LPCSTR lpszFile);
extern "C" LPWSTR __stdcall MyGetFullPath2W(LPCWSTR lpszPath, LPCWSTR lpszFile);
extern "C" int __stdcall MyGetFileTitleA(LPCSTR lpszFile, LPSTR lpszBuffer, int nMaxLen);
extern "C" int __stdcall MyGetFileTitleW(LPCWSTR lpszFile, LPWSTR lpszBuffer, int nMaxLen);
extern "C" int __stdcall MyGetModuleFileTitleA(HINSTANCE hInstance, LPSTR lpszBuffer, int nMaxLen);
extern "C" int __stdcall MyGetModuleFileTitleW(HINSTANCE hInstance, LPWSTR lpszBuffer, int nMaxLen);
extern "C" bool __stdcall MyIsBackSlashA(CHAR ch);
extern "C" bool __stdcall MyIsBackSlashW(WCHAR ch);
extern "C" LPSTR __stdcall MyFindBackSlashA(LPCSTR lpszString);
extern "C" LPWSTR __stdcall MyFindBackSlashW(LPCWSTR lpszString);
extern "C" LPSTR __stdcall MyFindBackSlashReverseA(LPCSTR lpszString);
extern "C" LPWSTR __stdcall MyFindBackSlashReverseW(LPCWSTR lpszString);
extern "C" LPSTR __stdcall MyFindReturnA(LPCSTR lpszString);
extern "C" LPWSTR __stdcall MyFindReturnW(LPCWSTR lpszString);
// pszPath から "..\" のような部分を取り除いて正しいパスに変換
// ((void*) pszPath == (void*) pszOutput でも正常に動作)
extern "C" void __stdcall MyRemoveDotsFromPathA(LPCSTR pszPath, LPSTR pszOutput);
extern "C" void __stdcall MyRemoveDotsFromPathW(LPCWSTR pszPath, LPWSTR pszOutput);
// lpszRelativePathName と lpszDirectory (ディレクトリパスでありファイル名ではない)
// を比較して、絶対パスを作成 (実際は MyGetFullPath + MyRemoveDotsFromPath)
extern "C" int __stdcall MyGetAbsolutePathA(LPCSTR lpszRelativePathName, LPCSTR lpszDirectory, LPSTR lpszBuffer, int nMaxLen);
extern "C" int __stdcall MyGetAbsolutePathW(LPCWSTR lpszRelativePathName, LPCWSTR lpszDirectory, LPWSTR lpszBuffer, int nMaxLen);
extern "C" LPSTR __stdcall MyGetAbsolutePath2A(LPCSTR lpszRelativePathName, LPCSTR lpszDirectory);
extern "C" LPWSTR __stdcall MyGetAbsolutePath2W(LPCWSTR lpszRelativePathName, LPCWSTR lpszDirectory);
// lpszFullPathName と lpszDirectory (ディレクトリパスでありファイル名ではない)
// を比較して、相対パスを作成
extern "C" int __stdcall MyGetRelativePathA(LPCSTR lpszFullPathName, LPCSTR lpszDirectory, LPSTR lpszBuffer, int nMaxLen);
extern "C" int __stdcall MyGetRelativePathW(LPCWSTR lpszFullPathName, LPCWSTR lpszDirectory, LPWSTR lpszBuffer, int nMaxLen);
extern "C" LPSTR __stdcall MyGetRelativePath2A(LPCSTR lpszFullPathName, LPCSTR lpszDirectory);
extern "C" LPWSTR __stdcall MyGetRelativePath2W(LPCWSTR lpszFullPathName, LPCWSTR lpszDirectory);
// 現在のディレクトリを使って絶対パスを作成
extern "C" int __stdcall MyMakeFullPathFromCurDirA(LPCSTR lpszPathName, LPSTR lpszBuffer, int nMaxLen);
extern "C" int __stdcall MyMakeFullPathFromCurDirW(LPCWSTR lpszPathName, LPWSTR lpszBuffer, int nMaxLen);
extern "C" LPSTR __stdcall MyMakeFullPathFromCurDir2A(LPCSTR lpszPathName);
extern "C" LPWSTR __stdcall MyMakeFullPathFromCurDir2W(LPCWSTR lpszPathName);
// 探せる場所からファイルを見つけて絶対パスを作成
extern "C" int __stdcall MySearchPath(LPCTSTR lpszPathName, LPTSTR lpszBuffer, int nMaxLen);
extern "C" DWORD __stdcall MyGetLongPathNameA(LPCSTR lpszPath, LPSTR lpszBuffer, DWORD dwMaxLen);
// ※ Windows NT 系のみ
extern "C" DWORD __stdcall MyGetLongPathNameW(LPCWSTR lpszPath, LPWSTR lpszBuffer, DWORD dwMaxLen);
extern "C" bool __stdcall CompareFilePathA(LPCSTR lpszPath1, LPCSTR lpszPath2);
extern "C" bool __stdcall CompareFilePathLenA(LPCSTR lpszPath1, LPCSTR lpszPath2, int nLen);
// ※ Windows NT 系のみ
extern "C" bool __stdcall CompareFilePathW(LPCWSTR lpszPath1, LPCWSTR lpszPath2);
// ※ Windows NT 系のみ
extern "C" bool __stdcall CompareFilePathLenW(LPCWSTR lpszPath1, LPCWSTR lpszPath2, int nLen);
LPSTR __stdcall MyFindStringNoCaseA(LPSTR lpszTarget, LPSTR lpszFind);
LPCSTR __stdcall MyFindStringNoCaseA(LPCSTR lpszTarget, LPCSTR lpszFind);
LPWSTR __stdcall MyFindStringNoCaseW(LPWSTR lpszTarget, LPWSTR lpszFind);
LPCWSTR __stdcall MyFindStringNoCaseW(LPCWSTR lpszTarget, LPCWSTR lpszFind);
#ifdef _UNICODE
#define MyGetFullPath             MyGetFullPathW
#define MyGetFullPath2            MyGetFullPath2W
#define MyGetFileTitle            MyGetFileTitleW
#define MyGetModuleFileTitle      MyGetModuleFileTitleW
#define MyIsBackSlash             MyIsBackSlashW
#define MyFindBackSlash           MyFindBackSlashW
#define MyFindBackSlashReverse    MyFindBackSlashReverseW
#define MyFindReturn              MyFindReturnW
#define MyRemoveDotsFromPath      MyRemoveDotsFromPathW
#define MyGetAbsolutePath         MyGetAbsolutePathW
#define MyGetAbsolutePath2        MyGetAbsolutePath2W
#define MyGetRelativePath         MyGetRelativePathW
#define MyGetRelativePath2        MyGetRelativePath2W
#define MyMakeFullPathFromCurDir  MyMakeFullPathFromCurDirW
#define MyMakeFullPathFromCurDir2 MyMakeFullPathFromCurDir2W
#define MyGetLongPathName         MyGetLongPathNameW
#define MyFindStringNoCase        MyFindStringNoCaseW
#else
#define MyGetFullPath             MyGetFullPathA
#define MyGetFullPath2            MyGetFullPath2A
#define MyGetFileTitle            MyGetFileTitleA
#define MyGetModuleFileTitle      MyGetModuleFileTitleA
#define MyIsBackSlash             MyIsBackSlashA
#define MyFindBackSlash           MyFindBackSlashA
#define MyFindBackSlashReverse    MyFindBackSlashReverseA
#define MyFindReturn              MyFindReturnA
#define MyRemoveDotsFromPath      MyRemoveDotsFromPathA
#define MyGetAbsolutePath         MyGetAbsolutePathA
#define MyGetAbsolutePath2        MyGetAbsolutePath2A
#define MyGetRelativePath         MyGetRelativePathA
#define MyGetRelativePath2        MyGetRelativePath2A
#define MyMakeFullPathFromCurDir  MyMakeFullPathFromCurDirA
#define MyMakeFullPathFromCurDir2 MyMakeFullPathFromCurDir2A
#define MyGetLongPathName         MyGetLongPathNameA
#define MyFindStringNoCase        MyFindStringNoCaseA
#endif
#ifdef __CCL_H__
void __stdcall MyGetFullPathString(LPCTSTR lpszPath, LPCTSTR lpszFile, CCCLString& rstrBuffer);
int __stdcall MyGetFileTitleString(LPCTSTR lpszFile, CCCLString& rstrFileTitle);
void __stdcall GetWindowTextString(HWND hWnd, CCCLString& rstrBuffer);
void __stdcall GetDlgItemTextString(HWND hDlg, int nIDDlgItem, CCCLString& rstrBuffer);
void __stdcall MyRemoveDotsFromPathString(CCCLString& rstrPath);
void __stdcall MyGetAbsolutePathString(LPCTSTR lpszRelativePathName, LPCTSTR lpszDirectory, CCCLString& rstrBuffer);
void __stdcall MyGetRelativePathString(LPCTSTR lpszFullPathName, LPCTSTR lpszDirectory, CCCLString& rstrBuffer);
void __stdcall MyGetLongPathNameString(LPCTSTR lpszPath, CCCLString& rstrBuffer);
void __stdcall MyMakeFullPathFromCurDirString(LPCTSTR lpszPathName, CCCLString& rstrBuffer);
void __stdcall MySearchPathString(LPCTSTR lpszPathName, CCCLString& rstrBuffer);
#endif

extern "C" void __stdcall GetTimeDateString(LPTSTR lpszBuffer, DWORD dwMaxLen);

extern "C" HFONT __stdcall MyCreateBoldFont(HFONT hFontNormal);

int __stdcall FormatByteStringExA(LPSTR lpszBuffer, int nBufferLen, LPCVOID lpData, DWORD dwSize);
int __stdcall FormatByteStringExW(LPWSTR lpszBuffer, int nBufferLen, LPCVOID lpData, DWORD dwSize);
DWORD __stdcall UnformatByteStringExA(LPCSTR lpszBuffer, LPVOID lpBuffer, DWORD dwSize);
DWORD __stdcall UnformatByteStringExW(LPCWSTR lpszBuffer, LPVOID lpBuffer, DWORD dwSize);
bool __stdcall GetDWordFromStringA(LPCSTR lpszString, DWORD& dwRet);
bool __stdcall GetDWordFromStringW(LPCWSTR lpszString, DWORD& dwRet);
void __stdcall GetStringFromDWordA(long nValue, LPSTR lpszBuffer, int nMinCount, BYTE cType);
void __stdcall GetStringFromDWordW(long nValue, LPWSTR lpszBuffer, int nMinCount, BYTE cType);
#ifdef _UNICODE
#define FormatByteStringEx        FormatByteStringExW
#define UnformatByteStringEx      UnformatByteStringExW
#define GetDWordFromString        GetDWordFromStringW
#define GetStringFromDWord        GetStringFromDWordW
#else
#define FormatByteStringEx        FormatByteStringExA
#define UnformatByteStringEx      UnformatByteStringExA
#define GetDWordFromString        GetDWordFromStringA
#define GetStringFromDWord        GetStringFromDWordA
#endif

extern "C" bool __stdcall MyMatchWildcardA(LPCSTR lpszTarget, LPCSTR lpszPattern);
extern "C" bool __stdcall MyMatchWildcardW(LPCWSTR lpszTarget, LPCWSTR lpszPattern);
extern "C" bool __stdcall MyMatchWildcardLenA(LPCSTR lpszTarget, size_t nTargetLen, LPCSTR lpszPattern, size_t nPatternLen);
extern "C" bool __stdcall MyMatchWildcardLenW(LPCWSTR lpszTarget, size_t nTargetLen, LPCWSTR lpszPattern, size_t nPatternLen);
#ifdef _UNICODE
#define MyMatchWildcard           MyMatchWildcardW
#define MyMatchWildcardLen        MyMatchWildcardLenW
#else
#define MyMatchWildcard           MyMatchWildcardA
#define MyMatchWildcardLen        MyMatchWildcardLenA
#endif

extern "C" int __stdcall MyCopyStringLenA(LPSTR lpszBuffer, LPCSTR lpszString, int nMaxLen);
extern "C" int __stdcall MyCopyStringLenW(LPWSTR lpszBuffer, LPCWSTR lpszString, int nMaxLen);
extern "C" int __stdcall MyCopyAnsiFromUnicode(LPCWSTR lpszString, LPSTR lpBuffer, int nMaxLen);
extern "C" int __stdcall MyCopyAnsiToUnicode(LPCSTR lpszString, LPWSTR lpBuffer, int nMaxLen);
extern "C" LPWSTR __stdcall MyCopyAnsiToUnicodeRealloc(LPCSTR lpszString, LPWSTR lpBuffer);
extern "C" LPWSTR __stdcall MyCopyUnicodeToUnicodeRealloc(LPCWSTR lpszString, LPWSTR lpBuffer);
extern "C" LPSTR __stdcall MyUnicodeToAnsiString(LPCWSTR lpszString);
extern "C" LPWSTR __stdcall MyAnsiStringToUnicode(LPCSTR lpszString);
extern "C" int __stdcall MyGetAnsiStringLenAsUnicode(LPCSTR lpszString);
#ifdef _UNICODE
#define MyCopyStringLen           MyCopyStringLenW
#define MyCopyFromUnicode(lpszString, lpBuffer, nMaxLen) \
	MyCopyStringLenW(lpBuffer, lpszString, nMaxLen)
#define MyCopyToUnicode(lpszString, lpBuffer, nMaxLen) \
	MyCopyStringLenW(lpBuffer, lpszString, nMaxLen)
#define MyCopyToUnicode2          MyCopyUnicodeToUnicodeRealloc
#define MyAnsiToString            MyAnsiStringToUnicode
#define MyStringToAnsi            MyUnicodeToAnsiString
#define MyUnicodeToString         _wcsdup
#define MyStringToUnicode         _wcsdup
#define MyGetStringLenAsUnicode   wcslen
#else
#define MyCopyStringLen           MyCopyStringLenA
#define MyCopyFromUnicode(lpszString, lpBuffer, nMaxLen) \
	MyCopyAnsiFromUnicode(lpszString, lpBuffer, nMaxLen)
#define MyCopyToUnicode(lpszString, lpBuffer, nMaxLen) \
	MyCopyAnsiToUnicode(lpszString, lpBuffer, nMaxLen)
#define MyCopyToUnicode2          MyCopyAnsiToUnicodeRealloc
#define MyAnsiToString            _strdup
#define MyStringToAnsi            _strdup
#define MyUnicodeToString         MyUnicodeToAnsiString
#define MyStringToUnicode         MyAnsiStringToUnicode
#define MyGetStringLenAsUnicode   MyGetAnsiStringLenAsUnicode
#endif
#ifdef __CCL_H__
void __stdcall ReplaceString(CCCLString& rstrText, LPCTSTR lpszFind, LPCTSTR lpszReplace);
bool __stdcall MyGetEnvironmentVariableString(LPCTSTR lpName, CCCLString& strBuffer);
#endif

extern "C" BOOL __stdcall FillSolidRect(HDC hDC, const RECT* lpRect, COLORREF crColor);

#if defined(GUID_DEFINED) && defined(__LPGUID_DEFINED__) && defined(_REFGUID_DEFINED)
extern "C" bool __stdcall MyGUIDFromStringA(LPCSTR lpszString, LPGUID lpGuid);
extern "C" bool __stdcall MyGUIDFromStringW(LPCWSTR lpszString, LPGUID lpGuid);
extern "C" bool __stdcall MyIsNullGUID(REFGUID rguid);
#ifdef _UNICODE
#define MyGUIDFromString          MyGUIDFromStringW
#else
#define MyGUIDFromString          MyGUIDFromStringA
#endif
#endif

#endif // __MYFUNC_H__
