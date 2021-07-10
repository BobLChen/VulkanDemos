# pragma once

#include "Common/Common.h"

#include <Windows.h>

class WindowsPlatformTime
{
public:

	static double InitTiming();

	static FORCE_INLINE double Seconds()
	{
		LARGE_INTEGER cycles;
		QueryPerformanceCounter(&cycles);
		return cycles.QuadPart * GetSecondsPerCycle() + 16777216.0;
	}

	static FORCE_INLINE double GetSecondsPerCycle()
	{
		return s_SecondsPerCycle;
	}

protected:
	static double s_SecondsPerCycle;
};

typedef WindowsPlatformTime GenericPlatformTime;