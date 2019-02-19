#pragma once

#include "Configuration/Platform.h"

#if PLATFORM_WINDOWS
	#include "Vulkan/Windows/VulkanPlatformDefines.h"
#endif

#if PLATFORM_MAC
	#include "Vulkan/Mac/VulkanPlatformDefines.h"
#endif

#include "Vulkan/VulkanGlobals.h"
