#pragma once

#include "Configuration/Platform.h"
#include "GenericPlatformMath.h"

#if PLATFORM_WINDOWS
    #include "Math/Windows/WindowsPlatformMath.h"
#elif PLATFORM_MAC
    #include "Math/Mac/MacPlatformMath.h"
#elif PLATFORM_LINUX
    #include "Math/Linux/LinuxPlatformMath.h"
#endif
