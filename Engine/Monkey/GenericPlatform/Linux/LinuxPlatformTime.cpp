#include "LinuxPlatformTime.h"

double LinuxPlatformTime::s_SecondsPerCycle = 0.0;

double LinuxPlatformTime::InitTiming()
{
    s_SecondsPerCycle = 1.0f / 1000000.0f;
    return Seconds();
}