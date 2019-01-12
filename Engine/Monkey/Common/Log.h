#pragma once

#include "Configuration/Platform.h"
#include <stdio.h>

#if PLATFORM_WINDOWS || PLATFORM_MAC
	#define MLOG(...) { fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n"); }
#elif PLATFORM_ANDROID
	#include <android/log.h>
	#define MLOG(FORMAT,...) { __android_log_print(ANDROID_LOG_DEBUG, "t", FORMAT, ##__VA_ARGS__); }
#endif
