# pragma once

#include "Common/Common.h"

#if !PLATFORM_WINDOWS
	#include <sys/time.h>
#endif

#include <cstddef>

struct GenericPlatformTime
{

#if !PLATFORM_WINDOWS
    static FORCEINLINE double Seconds()
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return ((double) tv.tv_sec) + (((double) tv.tv_usec) / 1000000.0);
	}
#else
	static FORCEINLINE double Seconds()
	{
		return 0.0;
	}
#endif

};
