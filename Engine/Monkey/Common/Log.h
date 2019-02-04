#pragma once

#include "Configuration/Platform.h"
#include <stdio.h>

#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_UNIX
#define MLOG(...) { fprintf(stdout, "%-6s", "LOG:"); fprintf(stdout, "%-40s:%-5d", __func__, __LINE__); fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n"); }
#define MLOGE(...) { fprintf(stdout, "%-6s", "ERROR:");  fprintf(stdout, "%-40s:%-5d", __func__, __LINE__); fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n"); }
#elif PLATFORM_ANDROID
	#include <android/log.h>
	#define MLOG(FORMAT,...) { __android_log_print(ANDROID_LOG_DEBUG, "t", FORMAT, ##__VA_ARGS__); }
    #define MLOGE(FORMAT,...) { __android_log_print(ANDROID_LOG_DEBUG, "t", FORMAT, ##__VA_ARGS__); }
#endif
