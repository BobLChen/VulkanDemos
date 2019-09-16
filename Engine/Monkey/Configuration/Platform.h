# pragma once

#if defined( _WIN64 )
#define PLATFORM_64BITS					1
#else
#define PLATFORM_64BITS					0
#endif

#if !defined(PLATFORM_WINDOWS)
	#define PLATFORM_WINDOWS 0
#endif

#if !defined(PLATFORM_MAC)
	#define PLATFORM_MAC 0
#endif

#if !defined(PLATFORM_IOS)
	#define PLATFORM_IOS 0
#endif

#if !defined(PLATFORM_ANDROID)
	#define PLATFORM_ANDROID 0
#endif

#if !defined(PLATFORM_LINUX)
	#define PLATFORM_LINUX 0
#endif

#include "PerPlatformCppDefines.h"
