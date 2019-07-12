# pragma once

#include "Common/Common.h"

#if PLATFORM_WINDOWS
	#include "GenericPlatform/Windows/WindowsPlatformTime.h"
#elif PLATFORM_MAC
	#include "GenericPlatform/Mac/MacPlatformTime.h"
#elif PLATFORM_IOS
	#include "GenericPlatform/IOS/IOSPlatformTime.h"
#elif PLATFORM_LINUX
	#include "GenericPlatform/Linux/LinuxPlatformTime.h"
#elif PLATFORM_ANDROID
	#include "GenericPlatform/Android/AndroidPlatformTime.h"
#endif
