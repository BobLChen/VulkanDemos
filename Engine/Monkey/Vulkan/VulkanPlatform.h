#pragma once

#include "Configuration/Platform.h"

#ifdef PLATFORM_WINDOWS
	#include "Vulkan/Windows/VulkanPlatformDefines.h"
#endif

#ifdef PLATFORM_MAC
	#include "Vulkan/Mac/VulkanPlatformDefines.h"
#endif


