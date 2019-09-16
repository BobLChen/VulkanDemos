#pragma once

#define VK_USE_PLATFORM_WIN32_KHR					1

#include <vector>
#include <vulkan/vulkan.h>

#include "Common/Common.h"
#include "Vulkan/VulkanGenericPlatform.h"

class VulkanWindowsPlatform : public VulkanGenericPlatform
{
public:
	static bool LoadVulkanLibrary();

	static bool LoadVulkanInstanceFunctions(VkInstance instance);

	static void FreeVulkanLibrary();

	static void GetInstanceExtensions(std::vector<const char*>& outExtensions);

	static void GetDeviceExtensions(std::vector<const char*>& outExtensions);

	static void CreateSurface(VkInstance instance, VkSurfaceKHR* outSurface);
};

typedef VulkanWindowsPlatform VulkanPlatform;