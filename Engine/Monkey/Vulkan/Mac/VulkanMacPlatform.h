#pragma once

#define VK_USE_PLATFORM_MAC_KHR                  1

#define VULKAN_SHOULD_ENABLE_DRAW_MARKERS        1
#define VULKAN_HAS_PHYSICAL_DEVICE_PROPERTIES2   1
#define VULKAN_DYNAMICALLYLOADED                 1
#define VULKAN_SIGNAL_UNIMPLEMENTED()
#define VULKAN_SUPPORTS_DEDICATED_ALLOCATION     0
#define VULKAN_SUPPORTS_AMD_BUFFER_MARKER        1
#define VULKAN_SUPPORTS_NV_DIAGNOSTIC_CHECKPOINT 1

#define ENUM_VK_ENTRYPOINTS_PLATFORM_BASE(EnumMacro)

#define ENUM_VK_ENTRYPOINTS_OPTIONAL_PLATFORM_INSTANCE(EnumMacro)

#define ENUM_VK_ENTRYPOINTS_PLATFORM_INSTANCE(EnumMacro)

#include "Vulkan/VulkanLoader.h"
#include "Vulkan/VulkanGenericPlatform.h"

class VulkanWindowsPlatform : public VulkanGenericPlatform
{
public:
    static bool LoadVulkanLibrary();
    static bool LoadVulkanInstanceFunctions(VkInstance inInstance);
    static void FreeVulkanLibrary();
    
    static void GetInstanceExtensions(TArray<const ANSICHAR*>& outExtensions);
    static void GetDeviceExtensions(TArray<const ANSICHAR*>& outExtensions);
    
    static void CreateSurface(void* windowHandle, VkInstance instance, VkSurfaceKHR* outSurface);
    
    static bool SupportsDeviceLocalHostVisibleWithNoPenalty();
    
    static void WriteCrashMarker(const OptionalVulkanDeviceExtensions& optionalExtensions, VkCommandBuffer cmdBuffer, VkBuffer destBuffer, const TArrayView<uint32>& entries, bool adding);
    
    static bool UseRealUBsOptimization(bool codeHeaderUseRealUBs)
    {
        return true;
    }
};

typedef FVulkanWindowsPlatform FVulkanPlatform;
