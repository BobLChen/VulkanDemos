#include "Configuration/Platform.h"
#include "Vulkan/Unix/VulkanPlatformDefines.h"
#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanGlobals.h"
#include "Application/SlateApplication.h"

static PFN_vkGetInstanceProcAddr G_GetInstanceProcAddr = nullptr;

bool VulkanUnixPlatform::LoadVulkanLibrary()
{
    return true;
}

bool VulkanUnixPlatform::LoadVulkanInstanceFunctions(VkInstance instance)
{
    G_GetInstanceProcAddr = vkGetInstanceProcAddr;
    return true;
}

void VulkanUnixPlatform::FreeVulkanLibrary()
{
    
}

void VulkanUnixPlatform::GetInstanceExtensions(std::vector<const char*>& outExtensions)
{

}

void VulkanUnixPlatform::GetDeviceExtensions(std::vector<const char*>& outExtensions)
{

}

void VulkanUnixPlatform::CreateSurface(void* windowHandle, VkInstance instance, VkSurfaceKHR* outSurface)
{
    SlateApplication::Get().GetPlatformApplication()->GetWindow()->CreateVKSurface(instance, outSurface);
}

bool VulkanUnixPlatform::SupportsDeviceLocalHostVisibleWithNoPenalty()
{
    return false;
}

void VulkanUnixPlatform::WriteCrashMarker(const OptionalVulkanDeviceExtensions& optionalExtensions, VkCommandBuffer cmdBuffer, VkBuffer destBuffer, const std::vector<uint32>& entries, bool adding)
{
    
}

