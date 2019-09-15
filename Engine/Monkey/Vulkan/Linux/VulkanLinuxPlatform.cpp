#include "Engine.h"

#include "Configuration/Platform.h"
#include "Application/Application.h"

#include "Vulkan/Linux/VulkanPlatformDefines.h"
#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanGlobals.h"

static PFN_vkGetInstanceProcAddr G_GetInstanceProcAddr = nullptr;

bool VulkanLinuxPlatform::LoadVulkanLibrary()
{
    return true;
}

bool VulkanLinuxPlatform::LoadVulkanInstanceFunctions(VkInstance instance)
{
    G_GetInstanceProcAddr = vkGetInstanceProcAddr;
    return true;
}

void VulkanLinuxPlatform::FreeVulkanLibrary()
{
    
}

void VulkanLinuxPlatform::GetInstanceExtensions(std::vector<const char*>& outExtensions)
{
    uint32_t count;
    const char** extensions = Engine::Get()->GetApplication()->GetPlatformApplication()->GetWindow()->GetRequiredInstanceExtensions(&count);
    for (int32 i = 0; i < count; ++i)
    {
        outExtensions.push_back(extensions[i]);
    }
}

void VulkanLinuxPlatform::GetDeviceExtensions(std::vector<const char*>& outExtensions)
{

}

void VulkanLinuxPlatform::CreateSurface(VkInstance instance, VkSurfaceKHR* outSurface)
{
    Engine::Get()->GetApplication()->GetPlatformApplication()->GetWindow()->CreateVKSurface(instance, outSurface);
}

bool VulkanLinuxPlatform::SupportsDeviceLocalHostVisibleWithNoPenalty()
{
    return false;
}

void VulkanLinuxPlatform::WriteCrashMarker(const OptionalVulkanDeviceExtensions& optionalExtensions, VkCommandBuffer cmdBuffer, VkBuffer destBuffer, const std::vector<uint32>& entries, bool adding)
{
    
}

