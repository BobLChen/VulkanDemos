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
	outExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	outExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
}

void VulkanWindowsPlatform::GetDeviceExtensions(std::vector<const char*>& outExtensions)
{
#if VULKAN_SUPPORTS_DEDICATED_ALLOCATION
	outExtensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
	outExtensions.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
#endif
}

void VulkanWindowsPlatform::CreateSurface(void* windowHandle, VkInstance instance, VkSurfaceKHR* outSurface)
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
