#pragma once

#include "Configuration/Platform.h"
#include <stdio.h>

#if PLATFORM_WINDOWS
	#include <Windows.h>
	#define MLOG(...)   { char __str__buf__[2048]; sprintf_s(__str__buf__, "%-6s", "LOG:");   OutputDebugString(__str__buf__); sprintf_s(__str__buf__, "%-40s:%-5d", __func__, __LINE__); OutputDebugString(__str__buf__); sprintf_s(__str__buf__, __VA_ARGS__); OutputDebugString(__str__buf__); sprintf_s(__str__buf__, "\n"); OutputDebugString(__str__buf__); }
	#define MLOGE(...)  { char __str__buf__[2048]; sprintf_s(__str__buf__, "%-6s", "ERROR:"); OutputDebugString(__str__buf__); sprintf_s(__str__buf__, "%-40s:%-5d", __func__, __LINE__); OutputDebugString(__str__buf__); sprintf_s(__str__buf__, __VA_ARGS__); OutputDebugString(__str__buf__); sprintf_s(__str__buf__, "\n"); OutputDebugString(__str__buf__); }
#elif PLATFORM_MAC || PLATFORM_LINUX || PLATFORM_IOS
	#define MLOG(...)   { fprintf(stdout, "%-6s", "LOG:");    fprintf(stdout, "%-40s:%-5d", __func__, __LINE__); fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n"); }
	#define MLOGE(...)  { fprintf(stdout, "%-6s", "ERROR:");  fprintf(stdout, "%-40s:%-5d", __func__, __LINE__); fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n"); }
#elif PLATFORM_ANDROID
	#include <android/log.h>
	#define MLOG(FORMAT,...)    { __android_log_print(ANDROID_LOG_DEBUG, "t", FORMAT, ##__VA_ARGS__); }
    #define MLOGE(FORMAT,...)   { __android_log_print(ANDROID_LOG_DEBUG, "t", FORMAT, ##__VA_ARGS__); }
#endif