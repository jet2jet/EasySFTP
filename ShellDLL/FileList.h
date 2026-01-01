/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 FileList.h - declarations for file listings
 */

#pragma once

#include "UString.h"
#include "Unknown.h"
#include "Dispatch.h"
#include "RefDelg.h"
#include "EasySFTP_h.h"

#ifndef S_IFMT
#define S_IFMT	_S_IFMT
#endif
#define S_IFSOCK	0140000
#define S_IFLNK	0120000
#ifndef S_IFREG
#define S_IFREG	_S_IFREG /*0100000*/
#endif
#define S_IFBLK	0060000
#ifndef S_IFDIR
#define S_IFDIR	_S_IFDIR /*0040000*/
#endif
#ifndef S_IFCHR
#define S_IFCHR	_S_IFCHR /*0020000*/
#endif
#ifndef S_IFIFO
#define S_IFIFO _S_IFIFO /*0010000*/
#endif
#define S_ISUID	0004000
#define S_ISGID	0002000
#define S_ISVTX	0001000
#define S_IRWXU	00700
#define S_IRUSR	_S_IREAD /*00400*/
#define S_IWUSR	_S_IWRITE /*00200*/
#define S_IXUSR	_S_IEXEC /*00100*/
#define S_IRWXG	00070
#define S_IRGRP	00040
#define S_IWGRP	00020
#define S_IXGRP	00010
#define S_IRWXO	00007
#define S_IROTH	00004
#define S_IWOTH	00002
#define S_IXOTH	00001

enum FileItemType
{
	fitypeFile = 0,
	fitypeDir = 1,
	fitypeCurDir = 2,
	fitypeParentDir = 3
};

#include <pshpack1.h>

struct CPermissionData
{
	unsigned append : 1;          // 'a'
	unsigned creatableInDir : 1;  // 'c'
	unsigned deletable : 1;       // 'd'
	unsigned directory : 1;       // 'e'
	unsigned renameAllowed : 1;   // 'f'
	unsigned listable : 1;        // 'l' -- can use LIST, NLST, MLSD
	unsigned dirCreatable : 1;    // 'm'
	unsigned purgable : 1;        // 'p'
	unsigned readable : 1;        // 'r'
	unsigned writable : 1;        // 'w'
};

#include <poppack.h>

class CFTPFileItem;
class CFTPDirectoryBase;
struct CSFTPFileData;
struct CSFTPFileAttribute;

extern "C" void __stdcall TimetToFileTime(time_t t, LPFILETIME pft);
extern "C" void __stdcall Time64AndNanoToFileTime(ULONGLONG uliTime64, DWORD dwNano, LPFILETIME pft);
extern "C" void __stdcall FileTimeToTimet(time_t * pt, const FILETIME * pft);
extern "C" void __stdcall FileTimeToTime64AndNano(ULONGLONG * puliTime64, DWORD * pdwNano, const FILETIME * pft);

extern "C" CFTPFileItem* __stdcall ParseUnixFileList(CFTPDirectoryBase* pDirectory, LPCWSTR lpszString);
extern "C" CFTPFileItem* __stdcall PickupUnixFileList(CFTPDirectoryBase* pDirectory, LPCWSTR lpszString, LPCWSTR lpszFileName, CFTPFileItem * pItem);
extern "C" CFTPFileItem* __stdcall ParseDOSFileList(CFTPDirectoryBase* pDirectory, LPCWSTR lpszString, char* pnYearFollows, bool* pbY2KProblem);
extern "C" CFTPFileItem* __stdcall PickupDOSFileList(CFTPDirectoryBase* pDirectory, LPCWSTR lpszString, LPCWSTR lpszFileName, CFTPFileItem * pItem, char* pnYearFollows, bool* pbY2KProblem);
extern "C" CFTPFileItem* __stdcall ParseMLSxData(CFTPDirectoryBase* pDirectory, LPCWSTR lpszString);
extern "C" CFTPFileItem* __stdcall ParseMLSxDataEx(CFTPDirectoryBase* pDirectory, LPCWSTR lpszString, CFTPFileItem * pItem);
extern "C" void __stdcall ParseSFTPAttributes(ULONG uServerVersion, CFTPFileItem * pItem, const CSFTPFileAttribute * pAttr);
extern "C" CFTPFileItem* __stdcall ParseSFTPData(CFTPDirectoryBase* pDirectory, ULONG uServerVersion, const CSFTPFileData * pFileData);

class CFTPFileItem : public CDispatchImplNoUnknownT<IEasySFTPFile>,
	public CReferenceDelegationChild<CFTPDirectoryBase>
{
public:
	CFTPFileItem(CFTPDirectoryBase* pParent);
	virtual ~CFTPFileItem();

	virtual void* GetThisForDispatch() override { return static_cast<IEasySFTPFile*>(this); }

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override;
	STDMETHOD_(ULONG, AddRef)() override { return CReferenceDelegationChild::AddRef(); }
	STDMETHOD_(ULONG, Release)() override { return CReferenceDelegationChild::Release(); }
	STDMETHOD(get_FileName)(BSTR* pFileName) override;
	STDMETHOD(get_IsDirectory)(VARIANT_BOOL* pbRet) override;
	STDMETHOD(get_IsHidden)(VARIANT_BOOL* pbRet) override;
	STDMETHOD(get_IsShortcut)(VARIANT_BOOL* pbRet) override;
	STDMETHOD(get_CreateTime)(DATE* pdRet) override;
	STDMETHOD(get_ModifyTime)(DATE* pdRet) override;
	STDMETHOD(get_Size)(hyper* piRet) override;
	STDMETHOD(get_Size32Bit)(long* piRet) override;
	STDMETHOD(get_Uid)(long* piRet) override;
	STDMETHOD(get_Gid)(long* piRet) override;
	STDMETHOD(get_OwnerName)(BSTR* pRet) override;
	STDMETHOD(get_GroupName)(BSTR* pRet) override;
	STDMETHOD(get_ItemType)(BSTR* pRet) override;
	STDMETHOD(get_Url)(BSTR* pRet) override;
	STDMETHOD(get_Directory)(IEasySFTPDirectory** ppRet) override;
	STDMETHOD(get_HasWinAttribute)(VARIANT_BOOL* pbRet) override;
	STDMETHOD(get_WinAttribute)(long* piRet) override;
	STDMETHOD(get_UnixAttribute)(long* piRet) override;

	inline bool IsDirectory() const
	{
		return pTargetFile ? pTargetFile->IsDirectory() : (type == fitypeDir);
	}
	inline bool IsHidden() const
	{
		if (bWinAttr)
			return (dwAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
		return (!strFileName.IsEmpty() && *((LPCWSTR)strFileName) == L'.');
	}
	inline bool IsShortcut() const
	{
		if (bWinAttr)
			return pTargetFile != NULL;
		return (nUnixMode & S_IFLNK) == S_IFLNK;
	}


public:
	CMyStringW strFileName;
	CMyStringW strUrl;
	// only for link, and only used in FTP mode
	CMyStringW strTargetFile;
	// only for link
	CFTPFileItem* pTargetFile;
	int type;
	CPermissionData permissions;
	bool bWinAttr;
	union {
		int nUnixMode;
		DWORD dwAttributes;
	};
	FILETIME ftCreateTime;
	FILETIME ftModifyTime;
	ULARGE_INTEGER uliSize;
	UINT uUID, uGID;
	CMyStringW strOwner;
	CMyStringW strGroup;

	CMyStringW strType;
	int iIconIndex;
	int iOpenIconIndex;
};

enum ServerFileAttrDataMask : WORD
{
	Attribute = 0x0001,
	OwnerGroupID = 0x0002,
	OwnerGroupName = 0x0004,
};

struct CServerFileAttrData
{
	CFTPFileItem* pItem;
	CMyStringW strOwner;
	CMyStringW strGroup;
	UINT uUID;
	UINT uGID;
	UINT nUnixMode;
	WORD wMask;
};

class CFTPDirectoryBase;

class CFTPFileItemList : public CDispatchImplT<IEasySFTPFiles>
{
public:
	CFTPFileItemList(CFTPDirectoryBase* pDirectory);
	virtual ~CFTPFileItemList();

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);

	STDMETHOD(get_Item)(VARIANT key, IEasySFTPFile** ppFile);
	STDMETHOD(get_Count)(long* pRet);
	STDMETHOD(get__NewEnum)(IUnknown** ppRet);

private:
	class CEnum : public CUnknownImplT<IEnumVARIANT>
	{
	public:
		CEnum(CFTPFileItemList* pList)
			: m_pList(pList)
			, m_uPos(0)
		{
			pList->AddRef();
		}
		virtual ~CEnum()
		{
			m_pList->Release();
		}

		STDMETHOD(QueryInterface)(REFIID riid, void** ppv);

		STDMETHOD(Next)(ULONG celt, VARIANT* rgelt, ULONG* pceltFetched);
		STDMETHOD(Skip)(ULONG celt);
		STDMETHOD(Reset)();
		STDMETHOD(Clone)(IEnumVARIANT** ppEnum);

	private:
		CFTPFileItemList* m_pList;
		ULONG m_uPos;
	};

	CFTPDirectoryBase* m_pDirectory;
};
