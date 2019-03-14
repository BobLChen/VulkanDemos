#include "Configuration/Platform.h"
#include "Vulkan/Android/VulkanPlatformDefines.h"
#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanGlobals.h"
#include "Application/SlateApplication.h"

static PFN_vkGetInstanceProcAddr G_GetInstanceProcAddr = nullptr;

bool VulkanAndroidPlatform::LoadVulkanLibrary()
{
    return true;
}

bool VulkanAndroidPlatform::LoadVulkanInstanceFunctions(VkInstance instance)
{
    G_GetInstanceProcAddr = vkGetInstanceProcAddr;
    return true;
}

void VulkanAndroidPlatform::FreeVulkanLibrary()
{
    
}

void VulkanAndroidPlatform::GetInstanceExtensions(std::vector<const char*>& outExtensions)
{
    uint32_t count;
    const char** extensions = SlateApplication::Get().GetPlatformApplication()->GetWindow()->GetRequiredInstanceExtensions(&count);
    for (int32 i = 0; i < count; ++i)
    {
        outExtensions.push_back(extensions[i]);
    }
}

void VulkanAndroidPlatform::GetDeviceExtensions(std::vector<const char*>& outExtensions)
{

}

void VulkanAndroidPlatform::CreateSurface(VkInstance instance, VkSurfaceKHR* outSurface)
{
    SlateApplication::Get().GetPlatformApplication()->GetWindow()->CreateVKSurface(instance, outSurface);
}

bool VulkanAndroidPlatform::SupportsDeviceLocalHostVisibleWithNoPenalty()
{
    return false;
}

void VulkanAndroidPlatform::WriteCrashMarker(const OptionalVulkanDeviceExtensions& optionalExtensions, VkCommandBuffer cmdBuffer, VkBuffer destBuffer, const std::vector<uint32>& entries, bool adding)
{
    
}

