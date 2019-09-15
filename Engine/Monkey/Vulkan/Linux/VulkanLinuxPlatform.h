#pragma once

#define VK_USE_PLATFORM_XCB_KHR          1

#include <vector>
#include <vulkan/vulkan.h>

#include "Common/Common.h"
#include "Vulkan/VulkanGenericPlatform.h"

class VulkanLinuxPlatform : public VulkanGenericPlatform
{
public:
    static bool LoadVulkanLibrary();
    
    static bool LoadVulkanInstanceFunctions(VkInstance inInstance);
    
    static void FreeVulkanLibrary();
    
    static void GetInstanceExtensions(std::vector<const char*>& outExtensions);
    
    static void GetDeviceExtensions(std::vector<const char*>& outExtensions);
    
    static void CreateSurface(VkInstance instance, VkSurfaceKHR* outSurface);
    
    static bool SupportsDeviceLocalHostVisibleWithNoPenalty();
    
    static void WriteCrashMarker(const OptionalVulkanDeviceExtensions& optionalExtensions, VkCommandBuffer cmdBuffer, VkBuffer destBuffer, const std::vector<uint32>& entries, bool adding);
    
    static bool UseRealUBsOptimization(bool codeHeaderUseRealUBs)
    {
        return true;
    }
};

typedef VulkanLinuxPlatform VulkanPlatform;
