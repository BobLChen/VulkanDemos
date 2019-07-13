# pragma once

#include "Common/Common.h"

#include <sys/time.h>

class LinuxPlatformTime
{
public:

    static double InitTiming();

	static FORCEINLINE double Seconds()
	{
		struct timeval tv;
		gettimeofday(&tv, 0);
		return ((double)tv.tv_sec) + (((double)tv.tv_usec) / 1000000.0);
	}

    static double GetSecondsPerCycle()
	{
		return s_SecondsPerCycle;
	}
    
protected:

    static double s_SecondsPerCycle;

};

typedef LinuxPlatformTime GenericPlatformTime;