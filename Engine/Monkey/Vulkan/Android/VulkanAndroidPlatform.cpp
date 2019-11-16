#include "Configuration/Platform.h"
#include "Vulkan/Android/VulkanPlatformDefines.h"
#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanGlobals.h"
#include "Application/Application.h"
#include "Engine.h"

#include <dlfcn.h>

#define DEFINE_VK_ENTRYPOINTS(Type,Func) Type Func = NULL;
ENUM_VK_ENTRYPOINTS_ALL(DEFINE_VK_ENTRYPOINTS)
#undef DEFINE_VK_ENTRYPOINTS

void* VulkanAndroidPlatform::g_VulkanLib = nullptr;
bool  VulkanAndroidPlatform::g_AttemptedLoad = false;

#define CHECK_VK_ENTRYPOINTS(Type,Func) if (Func == NULL) { foundAllEntryPoints = false; MLOG("Failed to find entry point for %s", #Func); }

bool VulkanAndroidPlatform::LoadVulkanLibrary()
{
    if (g_AttemptedLoad)
    {
        return g_VulkanLib != nullptr;
    }
    g_AttemptedLoad = true;

    MLOG("Loading libvulkan.so...");

    // Load vulkan library
    g_VulkanLib = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (g_VulkanLib == nullptr)
    {
        MLOG("Could not load vulkan library : %s!\n", dlerror());
        return false;
    }
    
    bool foundAllEntryPoints = true;

#define GET_VK_ENTRYPOINTS(Type,Func) Func = (Type)dlsym(g_VulkanLib, #Func);
    
    ENUM_VK_ENTRYPOINTS_BASE(GET_VK_ENTRYPOINTS);
	ENUM_VK_ENTRYPOINTS_BASE(CHECK_VK_ENTRYPOINTS);

    if (!foundAllEntryPoints)
    {
        dlclose(g_VulkanLib);
		g_VulkanLib = nullptr;
		return false;
    }

#if MONKEY_DEBUG
    ENUM_VK_ENTRYPOINTS_OPTIONAL_BASE(CHECK_VK_ENTRYPOINTS);
#endif

#undef GET_VK_ENTRYPOINTS

    return true;
}

bool VulkanAndroidPlatform::LoadVulkanInstanceFunctions(VkInstance instance)
{
    MLOG("Load Vulkan Functions");

    bool foundAllEntryPoints = true;

#define GETINSTANCE_VK_ENTRYPOINTS(Type, Func) Func = (Type)vkGetInstanceProcAddr(instance, #Func);

	ENUM_VK_ENTRYPOINTS_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS);
	ENUM_VK_ENTRYPOINTS_INSTANCE(CHECK_VK_ENTRYPOINTS);

	ENUM_VK_ENTRYPOINTS_PLATFORM_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS);
	ENUM_VK_ENTRYPOINTS_PLATFORM_INSTANCE(CHECK_VK_ENTRYPOINTS);

	if (!foundAllEntryPoints) {
		return false;
	}
    
	ENUM_VK_ENTRYPOINTS_OPTIONAL_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS);
	ENUM_VK_ENTRYPOINTS_OPTIONAL_PLATFORM_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS);

#if MONKEY_DEBUG
	ENUM_VK_ENTRYPOINTS_OPTIONAL_INSTANCE(CHECK_VK_ENTRYPOINTS);
	ENUM_VK_ENTRYPOINTS_OPTIONAL_PLATFORM_INSTANCE(CHECK_VK_ENTRYPOINTS);
#endif

#undef GETINSTANCE_VK_ENTRYPOINTS

    return true;
}

void VulkanAndroidPlatform::FreeVulkanLibrary()
{
    if (g_VulkanLib != nullptr)
	{
#define CLEAR_VK_ENTRYPOINTS(Type,Func) Func = nullptr;
		ENUM_VK_ENTRYPOINTS_ALL(CLEAR_VK_ENTRYPOINTS);

		dlclose(g_VulkanLib);
		g_VulkanLib = nullptr;
	}
	g_AttemptedLoad = false;
}

#undef CHECK_VK_ENTRYPOINTS

void VulkanAndroidPlatform::GetInstanceExtensions(std::vector<const char*>& outExtensions)
{
    uint32_t count;
    const char** extensions = Engine::Get()->GetApplication()->GetPlatformApplication()->GetWindow()->GetRequiredInstanceExtensions(&count);
    for (int32 i = 0; i < count; ++i) {
        outExtensions.push_back(extensions[i]);
    }
}

void VulkanAndroidPlatform::GetDeviceExtensions(std::vector<const char*>& outExtensions)
{

}

void VulkanAndroidPlatform::CreateSurface(VkInstance instance, VkSurfaceKHR* outSurface)
{
    Engine::Get()->GetApplication()->GetPlatformApplication()->GetWindow()->CreateVKSurface(instance, outSurface);
}

bool VulkanAndroidPlatform::SupportsDeviceLocalHostVisibleWithNoPenalty()
{
    return false;
}

void VulkanAndroidPlatform::WriteCrashMarker(const OptionalVulkanDeviceExtensions& optionalExtensions, VkCommandBuffer cmdBuffer, VkBuffer destBuffer, const std::vector<uint32>& entries, bool adding)
{
    
}