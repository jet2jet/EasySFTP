#pragma once

#include "Unknown.h"

class CEasySFTPLogger : public CUnknownImplT<IEasySFTPLogger>
{
public:
	CEasySFTPLogger() {}

public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override;
	STDMETHOD(Log)(EasySFTPLogLevel Level, BSTR Message, long HResult) override;
};
