#include "Configuration/Platform.h"
#include "Application/SlateApplication.h"

#include "Vulkan/Windows/VulkanPlatformDefines.h"
#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanGlobals.h"

static PFN_vkGetInstanceProcAddr G_GetInstanceProcAddr = nullptr;

bool VulkanWindowsPlatform::LoadVulkanLibrary()
{
	return true;
}

bool VulkanWindowsPlatform::LoadVulkanInstanceFunctions(VkInstance instance)
{
	G_GetInstanceProcAddr = vkGetInstanceProcAddr;
	return true;
}

void VulkanWindowsPlatform::FreeVulkanLibrary()
{

}

void VulkanWindowsPlatform::GetInstanceExtensions(std::vector<const char*>& outExtensions)
{
	uint32_t count;
    const char** extensions = SlateApplication::Get().GetPlatformApplication()->GetWindow()->GetRequiredInstanceExtensions(&count);
    for (int i = 0; i < count; ++i)
    {
        outExtensions.push_back(extensions[i]);
    }
}

void VulkanWindowsPlatform::GetDeviceExtensions(std::vector<const char*>& outExtensions)
{
	
}

void VulkanWindowsPlatform::CreateSurface(VkInstance instance, VkSurfaceKHR* outSurface)
{
	SlateApplication::Get().GetPlatformApplication()->GetWindow()->CreateVKSurface(instance, outSurface);
}

bool VulkanWindowsPlatform::SupportsDeviceLocalHostVisibleWithNoPenalty()
{
	return false;
}

void VulkanWindowsPlatform::WriteCrashMarker(const OptionalVulkanDeviceExtensions& optionalExtensions, VkCommandBuffer cmdBuffer, VkBuffer destBuffer, const std::vector<uint32>& entries, bool adding)
{
	
}
