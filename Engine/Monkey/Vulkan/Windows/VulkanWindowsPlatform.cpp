#include "Configuration/Platform.h"
#include "Application/Application.h"

#include "Engine.h"

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
    const char** extensions = Engine::Get()->GetApplication()->GetPlatformApplication()->GetWindow()->GetRequiredInstanceExtensions(&count);
    for (int32 i = 0; i < (int32)count; ++i)
    {
        outExtensions.push_back(extensions[i]);
    }
	
}

void VulkanWindowsPlatform::GetDeviceExtensions(std::vector<const char*>& outExtensions)
{
	
}

void VulkanWindowsPlatform::CreateSurface(VkInstance instance, VkSurfaceKHR* outSurface)
{
	Engine::Get()->GetApplication()->GetPlatformApplication()->GetWindow()->CreateVKSurface(instance, outSurface);
}