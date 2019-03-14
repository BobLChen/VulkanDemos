#pragma once

#include "Configuration/Platform.h"

#if PLATFORM_WINDOWS
	#include "Vulkan/Windows/VulkanPlatformDefines.h"
#elif PLATFORM_IOS
	#include "Vulkan/IOS/IOSPlatformDefines.h"
#elif PLATFORM_MAC
	#include "Vulkan/Mac/VulkanPlatformDefines.h"
#elif PLATFORM_LINUX
	#include "Vulkan/Linux/VulkanPlatformDefines.h"
#elif PLATFORM_ANDROID
	#include "Vulkan/Android/AndroidPlatformDefines.h"
#endif

#include "Vulkan/VulkanGlobals.h"
