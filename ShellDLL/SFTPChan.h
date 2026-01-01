/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 SFTPChan.h - declarations of CSFTPChannel and SFTP definitions
 */

#pragma once

#include "MySocket.h"
#include "SSHChan.h"
#include "Array.h"

// use S_IF***
#include "FileList.h"

/* version */
#define	SFTP_VERSION			6

/* client to server */
#define SSH_FXP_INIT			1
#define SSH_FXP_OPEN			3
#define SSH_FXP_CLOSE			4
#define SSH_FXP_READ			5
#define SSH_FXP_WRITE			6
#define SSH_FXP_LSTAT			7
#define SSH_FXP_STAT_VERSION_0		7
#define SSH_FXP_FSTAT			8
#define SSH_FXP_SETSTAT		9
#define SSH_FXP_FSETSTAT		10
#define SSH_FXP_OPENDIR		11
#define SSH_FXP_READDIR		12
#define SSH_FXP_REMOVE			13
#define SSH_FXP_MKDIR			14
#define SSH_FXP_RMDIR			15
#define SSH_FXP_REALPATH		16
#define SSH_FXP_STAT			17
#define SSH_FXP_RENAME			18
#define SSH_FXP_READLINK		19
#define SSH_FXP_SYMLINK		20
// for version 6
#define SSH_FXP_LINK               21
#define SSH_FXP_BLOCK              22
#define SSH_FXP_UNBLOCK            23

/* server to client */
#define SSH_FXP_VERSION		2
#define SSH_FXP_STATUS			101
#define SSH_FXP_HANDLE			102
#define SSH_FXP_DATA			103
#define SSH_FXP_NAME			104
#define SSH_FXP_ATTRS			105

#define SSH_FXP_EXTENDED		200
#define SSH_FXP_EXTENDED_REPLY		201

/* attributes */
#define SSH_FILEXFER_ATTR_SIZE              0x00000001
#define SSH_FILEXFER_ATTR_UIDGID            0x00000002
#define SSH_FILEXFER_ATTR_PERMISSIONS       0x00000004
#define SSH_FILEXFER_ATTR_ACMODTIME         0x00000008
#define SSH_FILEXFER_ATTR_EXTENDED          0x80000000
// for version 4
#define SSH_FILEXFER_ATTR_ACCESSTIME        0x00000008
#define SSH_FILEXFER_ATTR_CREATETIME        0x00000010
#define SSH_FILEXFER_ATTR_MODIFYTIME        0x00000020
#define SSH_FILEXFER_ATTR_ACL               0x00000040
#define SSH_FILEXFER_ATTR_OWNERGROUP        0x00000080
#define SSH_FILEXFER_ATTR_SUBSECOND_TIMES   0x00000100
// for version 5
#define SSH_FILEXFER_ATTR_BITS              0x00000200
// for version 6
#define SSH_FILEXFER_ATTR_ALLOCATION_SIZE   0x00000400
#define SSH_FILEXFER_ATTR_TEXT_HINT         0x00000800
#define SSH_FILEXFER_ATTR_MIME_TYPE         0x00001000
#define SSH_FILEXFER_ATTR_LINK_COUNT        0x00002000
#define SSH_FILEXFER_ATTR_UNTRANSLATED_NAME 0x00004000
#define SSH_FILEXFER_ATTR_CTIME             0x00008000

#define _SSH_FILEXFER_ATTR_SUPPORTED        0x8000FFFF

/* portable open modes */
#define SSH_FXF_READ			0x00000001
#define SSH_FXF_WRITE			0x00000002
#define SSH_FXF_APPEND			0x00000004
#define SSH_FXF_CREAT			0x00000008
#define SSH_FXF_TRUNC			0x00000010
#define SSH_FXF_EXCL			0x00000020
// for version 4
#define SSH_FXF_TEXT            0x00000040

/* portable open modes for version 5 (as dwMode in OpenFileEx) */
#define SSH_FXF_ACCESS_DISPOSITION        0x00000007
	#define SSH_FXF_CREATE_NEW            0x00000000
	#define SSH_FXF_CREATE_TRUNCATE       0x00000001
	#define SSH_FXF_OPEN_EXISTING         0x00000002
	#define SSH_FXF_OPEN_OR_CREATE        0x00000003
	#define SSH_FXF_TRUNCATE_EXISTING     0x00000004
#define SSH_FXF_ACCESS_APPEND_DATA        0x00000008
#define SSH_FXF_APPEND_DATA               SSH_FXF_ACCESS_APPEND_DATA
#define SSH_FXF_ACCESS_APPEND_DATA_ATOMIC 0x00000010
#define SSH_FXF_APPEND_DATA_ATOMIC        SSH_FXF_ACCESS_APPEND_DATA_ATOMIC
#define SSH_FXF_ACCESS_TEXT_MODE          0x00000020
#define SSH_FXF_TEXT_MODE                 SSH_FXF_ACCESS_TEXT_MODE
#define SSH_FXF_ACCESS_READ_LOCK          0x00000040
#define SSH_FXF_BLOCK_READ                SSH_FXF_ACCESS_READ_LOCK
#define SSH_FXF_ACCESS_WRITE_LOCK         0x00000080
#define SSH_FXF_BLOCK_WRITE               SSH_FXF_ACCESS_WRITE_LOCK
#define SSH_FXF_ACCESS_DELETE_LOCK        0x00000100
#define SSH_FXF_BLOCK_DELETE              SSH_FXF_ACCESS_DELETE_LOCK
// for version 6
#define SSH_FXF_BLOCK_ADVISORY            0x00000200
#define SSH_FXF_NOFOLLOW                  0x00000400
#define SSH_FXF_DELETE_ON_CLOSE           0x00000800
#define SSH_FXF_ACCESS_AUDIT_ALARM_INFO   0x00001000
#define SSH_FXF_ACCESS_BACKUP             0x00002000
#define SSH_FXF_BACKUP_STREAM             0x00004000
#define SSH_FXF_OVERRIDE_OWNER            0x00008000

/* statvfs@openssh.com f_flag flags */
#define SSH_FXE_STATVFS_ST_RDONLY	0x00000001
#define SSH_FXE_STATVFS_ST_NOSUID	0x00000002

/* status messages */
#define SSH_FX_OK			0
#define SSH_FX_EOF			1
#define SSH_FX_NO_SUCH_FILE		2
#define SSH_FX_PERMISSION_DENIED	3
#define SSH_FX_FAILURE			4
#define SSH_FX_BAD_MESSAGE		5
#define SSH_FX_NO_CONNECTION		6
#define SSH_FX_CONNECTION_LOST		7
#define SSH_FX_OP_UNSUPPORTED		8
#define SSH_FX_MAX			8
// for version 4
#define SSH_FX_INVALID_HANDLE                9
#define SSH_FX_NO_SUCH_PATH                  10
#define SSH_FX_FILE_ALREADY_EXISTS           11
#define SSH_FX_WRITE_PROTECT                 12
#define SSH_FX_NO_MEDIA                      13
// for version 5
#define SSH_FX_NO_SPACE_ON_FILESYSTEM        14
#define SSH_FX_QUOTA_EXCEEDED                15
#define SSH_FX_UNKNOWN_PRINCIPAL             16
#define SSH_FX_LOCK_CONFLICT                 17
// for version 6
#define SSH_FX_DIR_NOT_EMPTY                 18
#define SSH_FX_NOT_A_DIRECTORY               19
#define SSH_FX_INVALID_FILENAME              20
#define SSH_FX_LINK_LOOP                     21
#define SSH_FX_CANNOT_DELETE                 22
#define SSH_FX_INVALID_PARAMETER             23
#define SSH_FX_FILE_IS_A_DIRECTORY           24
#define SSH_FX_BYTE_RANGE_LOCK_CONFLICT      25
#define SSH_FX_BYTE_RANGE_LOCK_REFUSED       26
#define SSH_FX_DELETE_PENDING                27
#define SSH_FX_FILE_CORRUPT                  28
#define SSH_FX_OWNER_INVALID                 29
#define SSH_FX_GROUP_INVALID                 30
#define SSH_FX_NO_MATCHING_BYTE_RANGE_LOCK   31

/* file types (introduced in version 4) */
#define SSH_FILEXFER_TYPE_REGULAR          1
#define SSH_FILEXFER_TYPE_DIRECTORY        2
#define SSH_FILEXFER_TYPE_SYMLINK          3
#define SSH_FILEXFER_TYPE_SPECIAL          4
#define SSH_FILEXFER_TYPE_UNKNOWN          5
// introduced in version 5 (also used in CSFTPFileAttributeExtendedData for ver.3 or prior)
#define SSH_FILEXFER_TYPE_SOCKET           6
#define SSH_FILEXFER_TYPE_CHAR_DEVICE      7
#define SSH_FILEXFER_TYPE_BLOCK_DEVICE     8
#define SSH_FILEXFER_TYPE_FIFO             9

/* extended attribute bits (introduced in version 5) */
#define SSH_FILEXFER_ATTR_FLAGS_READONLY         0x00000001
#define SSH_FILEXFER_ATTR_FLAGS_SYSTEM           0x00000002
#define SSH_FILEXFER_ATTR_FLAGS_HIDDEN           0x00000004
#define SSH_FILEXFER_ATTR_FLAGS_CASE_INSENSITIVE 0x00000008
#define SSH_FILEXFER_ATTR_FLAGS_ARCHIVE          0x00000010
#define SSH_FILEXFER_ATTR_FLAGS_ENCRYPTED        0x00000020
#define SSH_FILEXFER_ATTR_FLAGS_COMPRESSED       0x00000040
#define SSH_FILEXFER_ATTR_FLAGS_SPARSE           0x00000080
#define SSH_FILEXFER_ATTR_FLAGS_APPEND_ONLY      0x00000100
#define SSH_FILEXFER_ATTR_FLAGS_IMMUTABLE        0x00000200
#define SSH_FILEXFER_ATTR_FLAGS_SYNC             0x00000400
// for version 6
#define SSH_FILEXFER_ATTR_FLAGS_TRANSLATION_ERR  0x00000800

/* text hints (introduced in version 6) */
#define SSH_FILEXFER_ATTR_KNOWN_TEXT        0x00
#define SSH_FILEXFER_ATTR_GUESSED_TEXT      0x01
#define SSH_FILEXFER_ATTR_KNOWN_BINARY      0x02
#define SSH_FILEXFER_ATTR_GUESSED_BINARY    0x03

/* ace access mask (introduced in version 4; also used in OpenFileEx from version 5 and later) */
#define ACE4_READ_DATA         0x00000001
#define ACE4_LIST_DIRECTORY    0x00000001
#define ACE4_WRITE_DATA        0x00000002
#define ACE4_ADD_FILE          0x00000002
#define ACE4_APPEND_DATA       0x00000004
#define ACE4_ADD_SUBDIRECTORY  0x00000004
#define ACE4_READ_NAMED_ATTRS  0x00000008
#define ACE4_WRITE_NAMED_ATTRS 0x00000010
#define ACE4_EXECUTE           0x00000020
#define ACE4_DELETE_CHILD      0x00000040
#define ACE4_READ_ATTRIBUTES   0x00000080
#define ACE4_WRITE_ATTRIBUTES  0x00000100
#define ACE4_DELETE            0x00010000
#define ACE4_READ_ACL          0x00020000
#define ACE4_WRITE_ACL         0x00040000
#define ACE4_WRITE_OWNER       0x00080000
#define ACE4_SYNCHRONIZE       0x00100000

/* rename flags (introduced from version 5) */
#define SSH_FXF_RENAME_OVERWRITE  0x00000001
#define SSH_FXF_RENAME_ATOMIC     0x00000002
#define SSH_FXF_RENAME_NATIVE     0x00000004

/* realpath control bytes (introduced from version 6) */
#define SSH_FXP_REALPATH_NO_CHECK    0x00000001
#define SSH_FXP_REALPATH_STAT_IF     0x00000002
#define SSH_FXP_REALPATH_STAT_ALWAYS 0x00000003

////////////////////////////////////////////////////////////////////////////////

class CSFTPChannel;

DECLARE_HANDLE(HSFTPHANDLE);

struct CSFTPFileAttributeExtendedData
{
	char* pszType;
	char* pszData;
};

struct CSFTPFileAttribute
{
	BYTE bFileType;
	DWORD dwMask;
	// SSH_FILEXFER_ATTR_SIZE
	ULARGE_INTEGER uliSize;
	// SSH_FILEXFER_ATTR_ALLOCATION_SIZE
	ULARGE_INTEGER uliAllocationSize;
	// SSH_FILEXFER_ATTR_UIDGID (deprecated; removed from version 4 and later)
	UINT uUserID, uGroupID;
	// SSH_FILEXFER_ATTR_OWNERGROUP
	CMyStringW strOwner, strGroup;
	// SSH_FILEXFER_ATTR_PERMISSIONS
	DWORD dwPermissions;
	// SSH_FILEXFER_ATTR_ACMODTIME (<=ver.3) or SSH_FILEXFER_ATTR_ACCESSTIME (>=ver.4)
	ULONGLONG dwAccessTime;
	// (SSH_FILEXFER_ATTR_ACCESSTIME | SSH_FILEXFER_ATTR_SUBSECOND_TIMES)
	ULONG dwAccessTimeNano;
	// SSH_FILEXFER_ATTR_CREATETIME
	ULONGLONG dwCreateTime;
	// (SSH_FILEXFER_ATTR_CREATETIME | SSH_FILEXFER_ATTR_SUBSECOND_TIMES)
	ULONG dwCreateTimeNano;
	// SSH_FILEXFER_ATTR_ACMODTIME (<=ver.3) or SSH_FILEXFER_ATTR_MODIFYTIME (>=ver.4)
	ULONGLONG dwModifiedTime;
	// (SSH_FILEXFER_ATTR_MODIFYTIME | SSH_FILEXFER_ATTR_SUBSECOND_TIMES)
	ULONG dwModifiedTimeNano;
	// SSH_FILEXFER_ATTR_CTIME
	ULONGLONG dwAttrChangedTime;
	// (SSH_FILEXFER_ATTR_CTIME | SSH_FILEXFER_ATTR_SUBSECOND_TIMES)
	ULONG dwAttrChangedTimeNano;
	// SSH_FILEXFER_ATTR_ACL
	CExBuffer bufferACL;
	// SSH_FILEXFER_ATTR_BITS
	DWORD dwAttributesEx;
	// SSH_FILEXFER_ATTR_BITS (>=ver.6)
	DWORD dwValidAttributesEx;
	// SSH_FILEXFER_ATTR_TEXT_HINT
	BYTE bTextHint;
	// SSH_FILEXFER_ATTR_MIME_TYPE
	CMyStringW strMIMEType;
	// SSH_FILEXFER_ATTR_LINK_COUNT
	DWORD dwLinkCount;
	// SSH_FILEXFER_ATTR_UNTRANSLATED_NAME
	CMyStringW strUntranslatedName;
	// SSH_FILEXFER_ATTR_EXTENDED
	int nExAttrCount;
	// SSH_FILEXFER_ATTR_EXTENDED
	CSFTPFileAttributeExtendedData* aExAttrs;

	CSFTPFileAttribute()
		: bFileType(SSH_FILEXFER_TYPE_REGULAR)
		, dwMask(0)
		, uUserID(0), uGroupID(0)
		, dwPermissions(0)
		, dwAccessTime(0)
		, dwAccessTimeNano(0)
		, dwCreateTime(0)
		, dwCreateTimeNano(0)
		, dwModifiedTime(0)
		, dwModifiedTimeNano(0)
		, dwAttributesEx(0)
		, nExAttrCount(0)
		, aExAttrs(NULL)
	{
		uliSize.QuadPart = 0;
		uliAllocationSize.QuadPart = 0;
	}
};

struct CSFTPFileData
{
	CMyStringW strFileName;
	CMyStringW strLongName;
	CSFTPFileAttribute attr;
};

#ifndef ST_RDONLY
#define ST_RDONLY   1
#endif
#ifndef ST_NOSUID
#define ST_NOSUID   2
#endif

struct sftp_statvfs
{
	ULONGLONG f_bsize;
	ULONGLONG f_frsize;
	ULONGLONG f_blocks;
	ULONGLONG f_bfree;
	ULONGLONG f_bavail;
	ULONGLONG f_files;
	ULONGLONG f_ffree;
	ULONGLONG f_favail;
	ULONGLONG f_fsid;
	ULONGLONG f_flag;
	ULONGLONG f_namemax;
};

struct CSFTPMessage
{
	ULONG uMsgID;
	LPCSTR lpszAddInfo;
	BYTE bSentMsg;
	BYTE bRecvMsg;
};

class __declspec(novtable) CSFTPChannelListener : public CSSHChannelListener
{
public:
	virtual void SFTPOpened(CSFTPChannel* pChannel) = 0;
	virtual void SFTPConfirm(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
		int nStatus, const CMyStringW& strMessage) = 0;
	virtual void SFTPFileHandle(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
		HSFTPHANDLE hSFTP) = 0;
	virtual void SFTPReceiveFileName(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
		const CSFTPFileData* aFiles, int nCount) = 0;
	virtual void SFTPReceiveData(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
		const void* pvData, size_t nLen, const bool* pbEOF) = 0;
	virtual void SFTPReceiveAttributes(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
		const CSFTPFileAttribute& attrs) = 0;
	virtual void SFTPReceiveStatVFS(CSFTPChannel* pChannel, CSFTPMessage* pMsg,
		const struct sftp_statvfs& statvfs) = 0;
};

class CSFTPChannel : public CSSHChannel
{
public:
	CSFTPChannel(CSFTPChannelListener* pListener);
	virtual ~CSFTPChannel();

public:
	bool Startup();
	bool InitSFTP();
	void SetCharset(ServerCharset nCharset);
	ULONG OpenDirectory(LPCWSTR lpszPath);
	ULONG ReadDirectory(HSFTPHANDLE hSFTP);
	ULONG CloseHandle(HSFTPHANDLE hSFTP);
	// for version 4 and prior
	ULONG OpenFile(LPCWSTR lpszPath, int nMode);
	// for version 4 and prior
	ULONG OpenFile(LPCWSTR lpszPath, int nMode, const CSFTPFileAttribute& attr);
	// for version 5 and later
	ULONG OpenFileEx(LPCWSTR lpszPath, DWORD dwDesiredAccess, DWORD dwMode);
	// for version 5 and later
	ULONG OpenFileEx(LPCWSTR lpszPath, DWORD dwDesiredAccess, DWORD dwMode, const CSFTPFileAttribute& attr);
	ULONG ReadFile(HSFTPHANDLE hSFTP, ULONGLONG uliOffset, size_t nLen);
	ULONG WriteFile(HSFTPHANDLE hSFTP, ULONGLONG uliOffset, const void* pvData, size_t nLen);
	ULONG Remove(LPCWSTR lpszPath);
	ULONG CreateRemoteDirectory(LPCWSTR lpszPath);
	ULONG CreateRemoteDirectory(LPCWSTR lpszPath, const CSFTPFileAttribute& attr);
	ULONG RemoveRemoteDirectory(LPCWSTR lpszPath);
	// may be called SFTPReceiveFileName
	ULONG RealPath(LPCWSTR lpszPath);
	// for version 6 or later only
	// may be called SFTPReceiveFileName
	ULONG WINAPIV RealPath(LPCWSTR lpszPath, BYTE bControl, int nComposePathCount, ...);
	ULONG Stat(LPCWSTR lpszPath)
		{ return Stat(lpszPath, _SSH_FILEXFER_ATTR_SUPPORTED); }
	ULONG Stat(LPCWSTR lpszPath, DWORD dwMask);
	ULONG LStat(LPCWSTR lpszPath)
		{ return LStat(lpszPath, _SSH_FILEXFER_ATTR_SUPPORTED); }
	ULONG LStat(LPCWSTR lpszPath, DWORD dwMask);
	ULONG FStat(HSFTPHANDLE hSFTP)
		{ return FStat(hSFTP, _SSH_FILEXFER_ATTR_SUPPORTED); }
	ULONG FStat(HSFTPHANDLE hSFTP, DWORD dwMask);
	ULONG SetStat(LPCWSTR lpszPath, const CSFTPFileAttribute& attr);
	ULONG FSetStat(HSFTPHANDLE hSFTP, const CSFTPFileAttribute& attr);
	ULONG Rename(LPCWSTR lpszOldPath, LPCWSTR lpszNewPath, DWORD dwFlags = 0);
	ULONG SymLink(LPCWSTR lpszLinkPath, LPCWSTR lpszTargetPath);
	// for version 6 or later only
	ULONG Link(LPCWSTR lpszNewLinkPath, LPCWSTR lpszExistingPath, bool bSymLink);
	// may be called SFTPReceiveFileName
	ULONG ReadLink(LPCWSTR lpszPath);
	// for version 6 or later only
	// dwBlockMask := SSH_FXF_BLOCK_*
	ULONG Block(HSFTPHANDLE hSFTP, ULONGLONG uliOffset, ULONGLONG uliLength, DWORD dwBlockMask);
	// for version 6 or later only
	ULONG Unblock(HSFTPHANDLE hSFTP, ULONGLONG uliOffset, ULONGLONG uliLength);

	ULONG StatVFS(LPCWSTR lpszPath);
	ULONG FStatVFS(HSFTPHANDLE hSFTP);

	ULONG GetServerVersion() const { return m_uServerVersion; }
	bool IsSupportStatVFS() const { return m_exts.m_bStatVFS != 0; }
	bool IsSupportFStatVFS() const { return m_exts.m_bFStatVFS != 0; }

	//bool FlushMessage(ULONG uMsgID, CSFTPChannelListener* pListener);
	bool FlushAllMessages(CSFTPChannelListener* pListener = NULL);
	bool HasQueue() const;
	bool RegisterMessageListener(ULONG uMsgID, CSFTPChannelListener* pListener);
	void UnregisterMessageListener(ULONG uMsgID, CSFTPChannelListener* pListener);

private:
	bool m_bStartingSubsystem;
	bool m_bInitializing;
	bool m_bSymLinkBug;
	ServerCharset m_nCharset;
	ULONG m_uMsgID, m_uCurMsgID, m_uServerVersion;
	void* m_pvMsgCache;
	CRITICAL_SECTION m_csMessages;
	CRITICAL_SECTION m_csListeners;
	CSFTPMessage* m_pMsgTemp;
	struct
	{
		unsigned m_bPosixRename : 1;
		unsigned m_bStatVFS : 1;
		unsigned m_bFStatVFS : 1;
		unsigned m_bNewLines : 1;
	} m_exts;
	CMyStringW m_strNewLines;
	CRITICAL_SECTION m_csQueue;
	void* m_pvMsgQueue;
	CRITICAL_SECTION m_csBuffer;
	CExBuffer m_bufferLastData;
	struct CListenerData
	{
		ULONG uMsgID;
		CSFTPChannelListener* pListener;
	};
	CMySimpleArrayT<CListenerData, const CListenerData&> m_aListeners;

	ULONG GetNextMsgID(BYTE bSentMsg, LPCSTR lpszAddInfo = NULL);
	CSFTPMessage* PopMessage(ULONG uMsgID);
	bool ProcessQueuedBuffer(CSFTPChannelListener* pListener, CExBuffer& buffer);

protected:
	bool SendSFTPChannelData(const void* pvBuffer, size_t nLen);
	virtual void ProcessChannelData(CExBuffer& buffer);

	bool ProcessSFTPVersion(CSFTPChannelListener* pListener, CExBuffer& buffer);
	bool ProcessSFTPStatus(CSFTPChannelListener* pListener, CExBuffer& buffer);
	bool ProcessSFTPHandle(CSFTPChannelListener* pListener, CExBuffer& buffer);
	bool ProcessSFTPData(CSFTPChannelListener* pListener, CExBuffer& buffer);
	bool ProcessSFTPName(CSFTPChannelListener* pListener, CExBuffer& buffer);
	bool ProcessSFTPAttrs(CSFTPChannelListener* pListener, CExBuffer& buffer);
	bool ProcessSFTPExtendedReply(CSFTPChannelListener* pListener, CExBuffer& buffer);
};
