#include "Configuration/Platform.h"
#include "Vulkan/IOS/VulkanPlatformDefines.h"
#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanGlobals.h"
#include "Application/Application.h"

static PFN_vkGetInstanceProcAddr G_GetInstanceProcAddr = nullptr;

bool VulkanIOSPlatform::LoadVulkanLibrary()
{
    return true;
}

bool VulkanIOSPlatform::LoadVulkanInstanceFunctions(VkInstance instance)
{
    G_GetInstanceProcAddr = vkGetInstanceProcAddr;
    return true;
}

void VulkanIOSPlatform::FreeVulkanLibrary()
{
    
}

void VulkanIOSPlatform::GetInstanceExtensions(std::vector<const char*>& outExtensions)
{
    uint32_t count;
    const char** extensions = Application::Get().GetPlatformApplication()->GetWindow()->GetRequiredInstanceExtensions(&count);
    for (int32 i = 0; i < count; ++i)
    {
        outExtensions.push_back(extensions[i]);
    }
}

void VulkanIOSPlatform::GetDeviceExtensions(std::vector<const char*>& outExtensions)
{
    
}

void VulkanIOSPlatform::CreateSurface(VkInstance instance, VkSurfaceKHR* outSurface)
{
    Application::Get().GetPlatformApplication()->GetWindow()->CreateVKSurface(instance, outSurface);
}

bool VulkanIOSPlatform::SupportsDeviceLocalHostVisibleWithNoPenalty()
{
    return false;
}

void VulkanIOSPlatform::WriteCrashMarker(const OptionalVulkanDeviceExtensions& optionalExtensions, VkCommandBuffer cmdBuffer, VkBuffer destBuffer, const std::vector<uint32>& entries, bool adding)
{
    
}

