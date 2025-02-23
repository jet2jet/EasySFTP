
#pragma once

struct CWaitResponseData
{
	inline CWaitResponseData(int nWaitType) : nWaitType(nWaitType) { }
	int nWaitType;
	union
	{
		// for FTP messages
		bool bWaiting;
		// for SFTP messages
		ULONG uMsgID;
	};
	enum
	{
		WRD_FTPWAITATTR = 1,
		WRD_DIRECTORY = 2,
		WRD_CONFIRM = 3,
		WRD_RENAME = 4,
		WRD_MAKEDIR = 5,
		WRD_DELETE = 6,
		WRD_SETSTAT = 7,
		WRD_FILEHANDLE = 8,
		WRD_ESTABLISHPASSIVE = 8,
		WRD_STARTPASSIVE = 9,
		WRD_ENDPASSIVE = 10,
		WRD_GETFEATURE = 11,
		WRD_GETFILEINFO = 12,
		WRD_READDATA = 13
	};
};
