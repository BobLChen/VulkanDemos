#pragma once

#define VK_USE_PLATFORM_MAC_KHR                  1

#define VULKAN_SHOULD_ENABLE_DRAW_MARKERS        1
#define VULKAN_HAS_PHYSICAL_DEVICE_PROPERTIES2   1
#define VULKAN_DYNAMICALLYLOADED                 1
#define VULKAN_SIGNAL_UNIMPLEMENTED()
#define VULKAN_SUPPORTS_DEDICATED_ALLOCATION     0
#define VULKAN_SUPPORTS_AMD_BUFFER_MARKER        1
#define VULKAN_SUPPORTS_NV_DIAGNOSTIC_CHECKPOINT 1

#define ENUM_VK_ENTRYPOINTS_PLATFORM_BASE(EnumMacro)

#define ENUM_VK_ENTRYPOINTS_OPTIONAL_PLATFORM_INSTANCE(EnumMacro)

#define ENUM_VK_ENTRYPOINTS_PLATFORM_INSTANCE(EnumMacro)

#include "Vulkan/VulkanLoader.h"
#include "Vulkan/VulkanGenericPlatform.h"

class VulkanWindowsPlatform : public VulkanGenericPlatform
{
public:
    static bool LoadVulkanLibrary();
    static bool LoadVulkanInstanceFunctions(VkInstance inInstance);
    static void FreeVulkanLibrary();
    
    static void GetInstanceExtensions(TArray<const ANSICHAR*>& OutExtensions);
    static void GetDeviceExtensions(TArray<const ANSICHAR*>& OutExtensions);
    
    static void CreateSurface(void* WindowHandle, VkInstance Instance, VkSurfaceKHR* OutSurface);
    
    static bool SupportsDeviceLocalHostVisibleWithNoPenalty();
    
    static void WriteCrashMarker(const FOptionalVulkanDeviceExtensions& OptionalExtensions, VkCommandBuffer CmdBuffer, VkBuffer DestBuffer, const TArrayView<uint32>& Entries, bool bAdding);
    
    // Some platforms only support real or non-real UBs, so this function can optimize it out
    static bool UseRealUBsOptimization(bool bCodeHeaderUseRealUBs)
    {
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
        static auto* CVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.Vulkan.UseRealUBs"));
        return (CVar && CVar->GetValueOnAnyThread() == 0) ? false : bCodeHeaderUseRealUBs;
#else
        return GMaxRHIFeatureLevel >= ERHIFeatureLevel::ES3_1 ? bCodeHeaderUseRealUBs : false;
#endif
    }
};

typedef FVulkanWindowsPlatform FVulkanPlatform;
