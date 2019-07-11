# pragma once

#include "Common/Common.h"

#include <sys/time.h>
#include <cstddef>

struct GenericPlatformTime
{
    static FORCEINLINE double Seconds()
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return ((double) tv.tv_sec) + (((double) tv.tv_usec) / 1000000.0);
	}
};
