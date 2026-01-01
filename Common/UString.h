/*
 Copyright (C) 2011 jet (ジェット)

 UString.h - declarations of CMyStringW
 */

#ifndef __USTRING_H__
#define __USTRING_H__

#ifndef _WIN32
#define NO_LOADSTRING
#ifndef __stdcall
#define __stdcall
#endif
#ifndef WINAPI
#define WINAPI
#endif
#ifndef WINAPIV
#define WINAPIV
#endif
#define CONST const
#define FAR
typedef int BOOL;
#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif
#define CP_ACP 0
typedef char FAR* LPSTR;
typedef const char FAR* LPCSTR;
typedef wchar_t FAR* LPWSTR;
typedef const wchar_t FAR* LPCWSTR;
typedef BYTE FAR* LPBYTE;
typedef unsigned int UINT;
typedef unsigned short WORD, * PWORD, FAR* LPWORD;
typedef uint32_t DWORD, * PDWORD, FAR* LPDWORD;
#endif

typedef const BYTE FAR* LPCBYTE;

#ifndef __nullterminated
#define __nullterminated
#endif
#ifndef UNALIGNED
#if defined(_MSC_VER)
#if defined(_M_MRX000) || defined(_M_ALPHA) || defined(_M_PPC) || defined(_M_IA64) || defined(_M_AMD64)
#define ALIGNMENT_MACHINE
#define UNALIGNED __unaligned
#else
#undef ALIGNMENT_MACHINE
#define UNALIGNED
#endif
typedef __nullterminated WCHAR UNALIGNED *LPUWSTR, *PUWSTR;
typedef __nullterminated CONST WCHAR UNALIGNED *LPCUWSTR, *PCUWSTR;
#elif defined(__GNUC__)
#ifdef __x86_64__
#define ALIGNMENT_MACHINE
//#define UNALIGNED 
#define UNALIGNED
struct _UNALIGNED_WCHAR;
typedef struct _UNALIGNED_WCHAR UWCHAR;
struct _UNALIGNED_WCHAR
{
	__attribute__((packed)) wchar_t wch;

	inline _UNALIGNED_WCHAR() { }
	inline _UNALIGNED_WCHAR(const UWCHAR& w) : wch(w.wch) { }
	inline _UNALIGNED_WCHAR(wchar_t w) : wch(w) { }
	inline operator bool() const { return wch != 0; }
	inline bool operator !() const { return !wch; }
	inline operator wchar_t() const { return wch; }
	inline UWCHAR& operator = (const UWCHAR& w) { wch = w.wch; return *this; }
	inline UWCHAR& operator = (wchar_t w) { wch = w; return *this; }
	inline bool operator == (const UWCHAR& w) { return wch == w.wch; }
	inline bool operator == (wchar_t w) { return wch == w; }
	inline bool operator != (const UWCHAR& w) { return wch != w.wch; }
	inline bool operator != (wchar_t w) { return wch != w; }
	inline bool operator <= (const UWCHAR& w) { return wch <= w.wch; }
	inline bool operator <= (wchar_t w) { return wch <= w; }
	inline bool operator < (const UWCHAR& w) { return wch < w.wch; }
	inline bool operator < (wchar_t w) { return wch < w; }
	inline bool operator >= (const UWCHAR& w) { return wch >= w.wch; }
	inline bool operator >= (wchar_t w) { return wch >= w; }
	inline bool operator > (const UWCHAR& w) { return wch > w.wch; }
	inline bool operator > (wchar_t w) { return wch > w; }
};
typedef __nullterminated UWCHAR *LPUWSTR, *PUWSTR;
typedef __nullterminated CONST UWCHAR *LPCUWSTR, *PCUWSTR;
#else
#undef ALIGNMENT_MACHINE
#define UNALIGNED
typedef __nullterminated WCHAR *LPUWSTR, *PUWSTR;
typedef __nullterminated CONST WCHAR *LPCUWSTR, *PCUWSTR;
#endif
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// CMyStringW - a string class (Unicode)

class CMyStringW
{
private:
	LPWSTR m_lpszData;
	LPSTR m_lpszLast;
	UINT m_uCodePage;

public:
	CMyStringW(wchar_t wch, size_t nCount);
	CMyStringW(char ch, size_t nCount, UINT uCodePage = CP_ACP);
	inline CMyStringW(wchar_t wch, int nCount) : CMyStringW(wch, static_cast<size_t>(nCount)) {}
	inline CMyStringW(char ch, int nCount, UINT uCodePage = CP_ACP) : CMyStringW(ch, static_cast<size_t>(nCount), uCodePage) {}
	CMyStringW(const CMyStringW& _string);
	CMyStringW(LPCSTR lpszString, UINT uCodePage = CP_ACP);
	CMyStringW(LPCWSTR lpszString);
#ifdef ALIGNMENT_MACHINE
	CMyStringW(LPCUWSTR lpszString);
#endif
#ifndef NO_LOADSTRING
	explicit CMyStringW(UINT uID);
#endif
private:
	struct _ConstructorWithVarArg {
		inline explicit _ConstructorWithVarArg()
		{
		}
	};
	WINAPIV CMyStringW(_ConstructorWithVarArg, LPCWSTR lpszString, ...);
	WINAPIV CMyStringW(_ConstructorWithVarArg, LPCSTR lpszString, ...);
#ifdef ALIGNMENT_MACHINE
	WINAPIV CMyStringW(_ConstructorWithVarArg, LPCUWSTR lpszString, ...);
#endif
#ifndef NO_LOADSTRING
	WINAPIV CMyStringW(_ConstructorWithVarArg, UINT uID, ...);
#endif
public:
	template <typename ...T>
	explicit inline WINAPIV CMyStringW(LPCWSTR lpszString, T... args)
		: CMyStringW(_ConstructorWithVarArg{}, lpszString, args...)
	{
	}
	template <typename ...T>
	explicit inline WINAPIV CMyStringW(LPCSTR lpszString, T... args)
		: CMyStringW(_ConstructorWithVarArg{}, lpszString, args...)
	{
	}
#ifdef ALIGNMENT_MACHINE
	template <typename ...T>
	explicit inline WINAPIV CMyStringW(LPCUWSTR lpszString, T... args)
		: CMyStringW(_ConstructorWithVarArg{}, lpszString, args...)
	{
	}
#endif
#ifndef NO_LOADSTRING
	template <typename ...T>
	explicit inline WINAPIV CMyStringW(UINT uID, T... args)
		: CMyStringW(_ConstructorWithVarArg{}, uID, args...)
	{
	}
#endif
	CMyStringW();
	~CMyStringW();

public:
	const CMyStringW& operator = (LPCWSTR lpszString);
#ifdef ALIGNMENT_MACHINE
	const CMyStringW& operator = (LPCUWSTR lpszString);
#endif
	const CMyStringW& operator = (LPCSTR lpszString);
	const CMyStringW& operator = (const CMyStringW& _string);

	CMyStringW operator + (LPCWSTR lpszString) const;
#ifdef ALIGNMENT_MACHINE
	CMyStringW operator + (LPCUWSTR lpszString) const;
#endif
	CMyStringW operator + (LPCSTR lpszString) const;
	CMyStringW operator + (const CMyStringW& _string) const;
	CMyStringW operator + (char ch) const;
	CMyStringW operator + (wchar_t ch) const;
	CMyStringW operator & (LPCWSTR lpszString) const;
#ifdef ALIGNMENT_MACHINE
	CMyStringW operator & (LPCUWSTR lpszString) const;
#endif
	CMyStringW operator & (LPCSTR lpszString) const;
	CMyStringW operator & (const CMyStringW& _string) const;
	CMyStringW operator & (char ch) const;
	CMyStringW operator & (wchar_t ch) const;
	const CMyStringW& operator += (LPCWSTR lpszString);
#ifdef ALIGNMENT_MACHINE
	const CMyStringW& operator += (LPCUWSTR lpszString);
#endif
	const CMyStringW& operator += (LPCSTR lpszString);
	const CMyStringW& operator += (const CMyStringW& _string);
	const CMyStringW& operator += (char ch);
	const CMyStringW& operator += (wchar_t ch);
	inline const CMyStringW& operator &= (LPCWSTR lpszString)
		{ return operator += (lpszString); }
#ifdef ALIGNMENT_MACHINE
	inline const CMyStringW& operator &= (LPCUWSTR lpszString)
		{ return operator += (lpszString); }
#endif
	inline const CMyStringW& operator &= (LPCSTR lpszString)
		{ return operator += (lpszString); }
	inline const CMyStringW& operator &= (const CMyStringW& _string)
		{ return operator += (_string); }
	inline const CMyStringW& operator &= (char ch)
		{ return operator += (ch); }
	inline const CMyStringW& operator &= (wchar_t ch)
		{ return operator += (ch); }

	operator LPCWSTR() const;
	operator LPSTR();
	wchar_t operator [] (size_t uIndex) const;
	wchar_t& operator [] (size_t uIndex);

public:
	void Empty();
	size_t GetLength() const;
	size_t GetLengthA() const;
	bool IsEmpty() const;
#ifndef NO_LOADSTRING
	BOOL LoadString(UINT uID);
#endif
	BOOL WINAPIV Format(LPCSTR lpszString, ...);
	// no unaligned
	BOOL WINAPIV Format(LPCWSTR lpszString, ...);
	BOOL WINAPIV Format(const CMyStringW* pstrFormat, ...);
#ifndef NO_LOADSTRING
	BOOL WINAPIV Format(UINT uID, ...);
#endif
	int Compare(LPCSTR lpszString, bool bNoCase = false, size_t uLength = (size_t) -1) const;
	// no unaligned
	int Compare(LPCWSTR lpszString, bool bNoCase = false, size_t uLength = (size_t) -1) const;
	int Compare(const CMyStringW& strCompare, bool bNoCase = false) const;

	size_t AppendChar(char ch);
	size_t AppendChar(wchar_t wch);
	size_t AppendString(LPCSTR lpszString, size_t uLength = (size_t) -1);
	size_t AppendString(LPCWSTR lpszString, size_t uLength = (size_t) -1);
#ifdef ALIGNMENT_MACHINE
	size_t AppendString(LPCUWSTR lpszString, size_t uLength = (size_t) -1);
#endif
	size_t AppendString(const CMyStringW& strInsert);
	size_t InsertChar(char ch, size_t uPos);
	size_t InsertChar(wchar_t wch, size_t uPos);  // uPos is a position of Unicode string (not a byte offset)
	size_t InsertString(LPCSTR lpszString, size_t uPos, size_t uLength = (size_t) -1);
	size_t InsertString(LPCWSTR lpszString, size_t uPos, size_t uLength = (size_t) -1); // uPos is a position of Unicode string (not a byte offset)
#ifdef ALIGNMENT_MACHINE
	size_t InsertString(LPCUWSTR lpszString, size_t uPos, size_t uLength = (size_t) -1); // uPos is a position of Unicode string (not a byte offset)
#endif
	size_t InsertString(const CMyStringW& strInsert, size_t uPos);
	wchar_t DeleteChar(size_t uPos);
	bool DeleteString(size_t uStart, size_t uLength);

	LPWSTR GetBuffer(size_t uLength = 0);
	void ReleaseBuffer(size_t uNewLength = (size_t) -1);
	LPSTR GetBufferA(size_t uLength = 0);
	void ReleaseBufferA(BOOL bSetLength = TRUE, size_t uNewLength = (size_t) -1);
	void SetString(LPCSTR lpszString, size_t uLength = (size_t) -1);
	// lpszString can be the buffer which this CMyStringW owns
	void SetString(LPCWSTR lpszString, size_t uLength = (size_t) -1);
#ifdef ALIGNMENT_MACHINE
	// lpszString can be the buffer which this CMyStringW owns
	void SetString(LPCUWSTR lpszString, size_t uLength = (size_t) -1);
#endif
	void SetString(const CMyStringW& strData, size_t uLength = (size_t) -1);

	LPSTR AllocAnsiString() const;

	UINT GetCodePage() const;
	void SetCodePage(UINT uCodePage);

public:
	//// these funtions are extentions for this application

	void SetUTF8String(LPCBYTE lpBuffer, size_t dwByteLength);
	// not necessary to call free() (can't modify returned buffer)
	LPCBYTE AllocUTF8String(size_t* lpdwLength = NULL);
	// need to call free()
	LPBYTE AllocUTF8StringC(size_t* lpdwLength = NULL) const;
};

CMyStringW operator + (LPCWSTR lpszString, const CMyStringW& _string);
#ifdef ALIGNMENT_MACHINE
CMyStringW operator + (LPCUWSTR lpszString, const CMyStringW& _string);
#endif
CMyStringW operator + (LPCSTR lpszString, const CMyStringW& _string);
CMyStringW operator + (char ch, const CMyStringW& _string);
CMyStringW operator + (wchar_t ch, const CMyStringW& _string);

// backward-compability
#ifndef _CRT_DEPRECATE_TEXT
#ifdef __GNUC__
#define _CRT_DEPRECATE_TEXT(text) __attribute__((deprecated))
#else
#define _CRT_DEPRECATE_TEXT(text)
#endif
#endif
_CRT_DEPRECATE_TEXT("'_StringW' is deprecated; use 'CMyStringW' instead.")
typedef CMyStringW _StringW;

#endif // __USTRING_H__
