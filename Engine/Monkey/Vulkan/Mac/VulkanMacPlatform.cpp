#include "Configuration/Platform.h"
#include "Vulkan/Mac/VulkanPlatformDefines.h"
#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanGlobals.h"
#include "Application/Application.h"
#include "Engine.h"

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
    uint32_t count;
    const char** extensions = Engine::Get()->GetApplication()->GetPlatformApplication()->GetWindow()->GetRequiredInstanceExtensions(&count);
    for (int32 i = 0; i < count; ++i)
    {
        outExtensions.push_back(extensions[i]);
    }
}

void VulkanMacPlatform::GetDeviceExtensions(std::vector<const char*>& outExtensions)
{
    
}

void VulkanMacPlatform::CreateSurface(VkInstance instance, VkSurfaceKHR* outSurface)
{
    Engine::Get()->GetApplication()->GetPlatformApplication()->GetWindow()->CreateVKSurface(instance, outSurface);
}

bool VulkanMacPlatform::SupportsDeviceLocalHostVisibleWithNoPenalty()
{
    return false;
}

void VulkanMacPlatform::WriteCrashMarker(const OptionalVulkanDeviceExtensions& optionalExtensions, VkCommandBuffer cmdBuffer, VkBuffer destBuffer, const std::vector<uint32>& entries, bool adding)
{
    
}

