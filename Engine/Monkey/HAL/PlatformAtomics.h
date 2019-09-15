#pragma once

#include "Configuration/Platform.h"

#if PLATFORM_WINDOWS
	#include "HAL/Windows/WindowsPlatformAtomics.h"
#elif PLATFORM_MAC
	#include "HAL/Mac/MacPlatformAtomics.h"
#elif PLATFORM_IOS
	#include "HAL/IOS/IOSPlatformAtomics.h"
#elif PLATFORM_LINUX
	#include "HAL/Linux/LinuxPlatformAtomics.h"
#elif PLATFORM_ANDROID
	#include "HAL/Android/AndroidPlatformAtomics.h"
#endif