#include "Configuration/Platform.h"
#include "Vulkan/Mac/VulkanPlatformDefines.h"
#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanGlobals.h"

static PFN_vkGetInstanceProcAddr G_GetInstanceProcAddr = nullptr;

bool VulkanMacPlatform::LoadVulkanLibrary()
{
    return true;
}

bool VulkanMacPlatform::LoadVulkanInstanceFunctions(VkInstance instance)
{
    G_GetInstanceProcAddr = vkGetInstanceProcAddr;
    return true;
}

void VulkanMacPlatform::FreeVulkanLibrary()
{
    
}

void VulkanMacPlatform::GetInstanceExtensions(std::vector<const char*>& outExtensions)
{
    outExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    outExtensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
}

void VulkanMacPlatform::GetDeviceExtensions(std::vector<const char*>& outExtensions)
{
#if VULKAN_SUPPORTS_DEDICATED_ALLOCATION
    outExtensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    outExtensions.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
#endif
    //outExtensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
    //outExtensions.push_back("VK_MVK_moltenvk");
}

void VulkanMacPlatform::CreateSurface(void* windowHandle, VkInstance instance, VkSurfaceKHR* outSurface)
{
    VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo;
    ZeroVulkanStruct(surfaceCreateInfo, VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK);
    VERIFYVULKANRESULT(vkCreateMacOSSurfaceMVK(instance, &surfaceCreateInfo, nullptr, outSurface));
}

bool VulkanMacPlatform::SupportsDeviceLocalHostVisibleWithNoPenalty()
{
    return false;
}

void VulkanMacPlatform::WriteCrashMarker(const OptionalVulkanDeviceExtensions& optionalExtensions, VkCommandBuffer cmdBuffer, VkBuffer destBuffer, const std::vector<uint32>& entries, bool adding)
{
    
}

