
#include "MacPlatformTime.h"

double MacPlatformTime::s_SecondsPerCycle = 0.0;

double MacPlatformTime::InitTiming()
{
    s_SecondsPerCycle = 1.0f / 1000000.0f;
    return Seconds();
}
