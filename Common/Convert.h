/*
 Copyright (C) 2011 jet (ジェット)

 Convert.h - declarations of functions for convertion between charsets
 */

#ifndef __CONVERT_H__
#define __CONVERT_H__

#ifndef _WIN32
#define __stdcall
#define FAR
typedef char FAR* LPSTR;
typedef const char FAR* LPCSTR;
typedef wchar_t FAR* LPWSTR;
typedef const wchar_t FAR* LPCWSTR;
typedef BYTE FAR* LPBYTE;
typedef unsigned short WORD, * PWORD, FAR* LPWORD;
typedef uint32_t DWORD, * PDWORD, FAR* LPDWORD;
#endif

typedef const BYTE FAR* LPCBYTE;

extern "C" size_t __stdcall UnicodeToUTF8(LPCWSTR lpszUnicode, size_t dwLength, LPBYTE lpBuffer, size_t dwBufferLength);
#ifdef _WIN32
extern "C" size_t __stdcall UnicodeToUTF8File(LPCWSTR lpszUnicode, size_t dwLength, HANDLE hFile, BOOL* pbResult = NULL);
extern "C" size_t __stdcall UnicodeFileToUTF8(HANDLE hFile, LPBYTE lpBuffer, size_t dwBufferLength);
#endif
extern "C" bool __stdcall IsUTF8Data(LPCBYTE lpszUTF8, size_t dwByteLength);
// 0: not enough length, -1: invalid data
extern "C" size_t __stdcall GetUTF8Length(LPCBYTE lpszUTF8, size_t dwMaxByteLength);
extern "C" size_t __stdcall CalcActualUTF8Length(LPCBYTE lpszUTF8, size_t dwMaxByteLength);
#ifdef _WIN32
extern "C" bool __stdcall IsUTF8File(HANDLE hFile);
#endif
extern "C" size_t __stdcall UTF8ToUnicode(LPCBYTE lpszUTF8, size_t dwByteLength, LPWSTR lpBuffer, size_t dwBufferLength);
// bEndian: 0x00: Big, 0x01: Little(8bit*2*2), 0x02: Little(16bit*2), 0x03: Little(8bit*4)
//   for 0x11223344:
//     bEndian == 0x00: 11 22 33 44 : usual Big-endian
//             == 0x01: 22 11 44 33 (reverse inside parentheses: (0x11 22)(33 44))
//             == 0x02: 33 44 11 22 (reverse 0x1122 and 0x3344)
//             == 0x03: 44 33 22 11 : usual Little-endian
enum EndianConstants
{
	endBigEndian = 0,
	endLittle822Endian,
	endLittle162Endian,
	endLittle84Endian,
	endLittleEndian = endLittle84Endian
};
#ifdef _WIN32
extern "C" size_t __stdcall UTF8ToUnicodeFile(LPCBYTE lpszUTF8, size_t dwByteLength, HANDLE hFile, BYTE bEndian);
extern "C" size_t __stdcall UTF8FileToUnicode(HANDLE hFile, LPWSTR lpBuffer, size_t dwBufferLength);
#endif

extern "C" size_t __stdcall CalcUTF32ToUnicodeLength(const char* pszBuffer, size_t nLen, size_t* pnReadLen, bool bBigEndian);
extern "C" size_t __stdcall UTF32ToUnicode(const char* pszBuffer, size_t nLen, size_t* pnReadLen,
	LPWSTR lpszBuffer, size_t nBufferLen, bool bBigEndian);

extern "C" bool __stdcall IsEUCChar(BYTE ch1, BYTE ch2);
extern "C" bool __stdcall IsEUCFirstChar(BYTE ch1);
extern "C" bool __stdcall IsShiftJISFirstChar(BYTE ch1);

extern "C" bool __stdcall EUCToShiftJISString(LPSTR lpBuffer, size_t dwBufferLength);
extern "C" bool __stdcall ShiftJISToEUCString(LPSTR lpBuffer, size_t dwBufferLength);

#ifdef _WIN32
extern "C" size_t __stdcall EUCFileToShiftJIS(HANDLE hFile, LPSTR lpBuffer, size_t dwBufferLength);
extern "C" size_t __stdcall ShiftJISToEUCFile(LPCSTR lpBuffer, size_t dwBufferLength, HANDLE hFile, BOOL* pbResult = NULL);
#endif

extern "C" inline bool __stdcall IsEUCChar(BYTE ch1, BYTE ch2)
{
	return (ch1 == 0x8E && ch2 >= 0xA0 && ch2 <= 0xDF) ||
		(ch1 >= 0xA1 && ch1 <= 0xFE && ch2 >= 0xA1 && ch2 <= 0xFE);
}

extern "C" inline bool __stdcall IsEUCFirstChar(BYTE ch1)
{
	return (ch1 == 0x8E) || (ch1 >= 0xA1 && ch1 <= 0xFE);
}

extern "C" inline bool __stdcall IsShiftJISFirstChar(BYTE ch1)
{
	//return (ch1 >= 0x81 && ch1 <= 0x9F) || (ch1 >= 0xE0 && ch1 <= 0xFC) ||
	//	(ch1 >= 0xA0 && ch1 <= 0xDF);
	return (ch1 >= 0x81 && ch1 <= 0xFC);
}

#endif // __CONVERT_H__
