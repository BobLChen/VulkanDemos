#pragma once

#include "Configuration/Platform.h"

#if PLATFORM_WINDOWS
	#include "Vulkan/Windows/VulkanPlatformDefines.h"
#endif

#if PLATFORM_MAC
	#include "Vulkan/Mac/VulkanPlatformDefines.h"
#endif

#if PLATFORM_LINUX
	#include "Vulkan/Linux/VulkanPlatformDefines.h"
#endif

#if PLATFORM_ANDROID
	#include "Vulkan/Android/AndroidPlatformDefines.h"
#endif

#include "Vulkan/VulkanGlobals.h"
