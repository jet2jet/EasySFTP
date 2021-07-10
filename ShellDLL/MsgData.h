
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
		WRD_PASSIVE = 8,
		WRD_PASSIVEMSG = 9,
		WRD_GETFEATURE = 10,
		WRD_GETFILEINFO = 11
	};
};
