#pragma once

#include "Configuration/Platform.h"
#include "Configuration/PerPlatformCppDefines.h"

#if PLATFORM_WINDOWS
	#include "HAL/Windows/WindowsPlatformAtomics.h"
#elif PLATFORM_MAC
	#include "HAL/Mac/MacPlatformAtomics.h"
#endif

