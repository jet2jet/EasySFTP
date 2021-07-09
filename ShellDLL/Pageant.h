#pragma once

#include "SSHAgent.h"

class CPageantAgent : public CSSHAgent
{
public:
	static bool IsAvailable();
	virtual bool Query(const void* dataSend, size_t dataSendSize, void** dataReceived, size_t* dataReceivedSize);
};
