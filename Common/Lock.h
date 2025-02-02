#pragma once

class CMyScopedCSLock
{
public:
	CMyScopedCSLock(CRITICAL_SECTION& cs) : _(cs)
	{
		::EnterCriticalSection(&cs);
	}
	~CMyScopedCSLock()
	{
		::LeaveCriticalSection(&_);
	}
private:
	CRITICAL_SECTION& _;
};
