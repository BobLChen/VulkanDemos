#include "AndroidPlatformTime.h"

double AndroidPlatformTime::s_SecondsPerCycle = 0.0;

double AndroidPlatformTime::InitTiming()
{
    s_SecondsPerCycle = 1.0f / 1000000.0f;
    return Seconds();
}