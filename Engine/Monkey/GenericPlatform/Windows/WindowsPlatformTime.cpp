#include "WindowsPlatformTime.h"

double WindowsPlatformTime::s_SecondsPerCycle = 0.0f;

double WindowsPlatformTime::InitTiming(void)
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	s_SecondsPerCycle = 1.0 / frequency.QuadPart;
	return GenericPlatformTime::Seconds();
}