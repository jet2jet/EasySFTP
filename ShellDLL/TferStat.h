/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 TferStat.h - declaration of CTransferStatus
 */

#pragma once

////////////////////////////////////////////////////////////////////////////////

class __declspec(novtable) CTransferStatus
{
public:
	virtual void TransferInProgress(void* pvObject, ULONGLONG uliPosition) = 0;
	virtual bool TransferIsCanceled(void* pvObject) = 0;
};
