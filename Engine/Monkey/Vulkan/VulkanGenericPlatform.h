#pragma once

#include <vector>
#include <string>

#include "Common/Common.h"
#include "Core/PixelFormat.h"

struct OptionalVulkanDeviceExtensions;

class VulkanGenericPlatform
{
public:
    static void GetInstanceExtensions(std::vector<const char*>& outExtensions);
    
    static void GetDeviceExtensions(std::vector<const char*>& outExtensions);
    
    static void CreateSurface(VkSurfaceKHR* outSurface);
    
    static bool IsSupported()
    {
        return true;
    }
    
    static bool LoadVulkanLibrary()
    {
        return true;
    }
    
    static bool LoadVulkanInstanceFunctions(VkInstance instance)
    {
        return true;
    }
    
    static void FreeVulkanLibrary()
    {
        
    }
    
    static void NotifyFoundInstanceLayersAndExtensions(const std::vector<std::string>& layers, const std::vector<std::string>& extensions)
    {
        
    }
    
    static void NotifyFoundDeviceLayersAndExtensions(VkPhysicalDevice physicalDevice, const std::vector<std::string>& layers, const std::vector<std::string>& extensions)
    {
        
    }
    
    static bool SupportsBCTextureFormats()
    {
        return true;
    }
    
    static bool SupportsASTCTextureFormats()
    {
        return false;
    }
    
    static bool SupportsQuerySurfaceProperties()
    {
        return true;
    }
    
    static void SetupFeatureLevels()
    {
        
    }
    
    static bool SupportsStandardSwapchain()
    {
        return true;
    }
    
    static PixelFormat GetPixelFormatForNonDefaultSwapchain()
    {
        return PF_Unknown;
    }
    
    static bool SupportsDepthFetchDuringDepthTest()
    {
        return true;
    }
    
    static bool SupportsTimestampRenderQueries()
    {
        return true;
    }
    
    static bool RequiresPresentLayoutFix()
    {
        return false;
    }
    
    static bool ForceEnableDebugMarkers()
    {
        return false;
    }
    
    static bool HasUnifiedMemory()
    {
        return false;
    }
    
    static bool RegisterGPUWork()
    {
        return true;
    }
    
    static void WriteCrashMarker(const OptionalVulkanDeviceExtensions& optionalExtensions, VkCommandBuffer cmdBuffer, VkBuffer destBuffer, const std::vector<uint32>& entries, bool adding)
    {
        
    }
    
    static void RestrictEnabledPhysicalDeviceFeatures(VkPhysicalDeviceFeatures& inOutFeaturesToEnable)
    {
        inOutFeaturesToEnable.shaderResourceResidency   = VK_FALSE;
        inOutFeaturesToEnable.shaderResourceMinLod      = VK_FALSE;
        inOutFeaturesToEnable.sparseBinding             = VK_FALSE;
        inOutFeaturesToEnable.sparseResidencyBuffer     = VK_FALSE;
        inOutFeaturesToEnable.sparseResidencyImage2D    = VK_FALSE;
        inOutFeaturesToEnable.sparseResidencyImage3D    = VK_FALSE;
        inOutFeaturesToEnable.sparseResidency2Samples   = VK_FALSE;
        inOutFeaturesToEnable.sparseResidency4Samples   = VK_FALSE;
        inOutFeaturesToEnable.sparseResidency8Samples   = VK_FALSE;
        inOutFeaturesToEnable.sparseResidencyAliased    = VK_FALSE;
    }
    
    static bool SupportParallelRenderingTasks()
    {
        return true;
    }
    
    static void EnablePhysicalDeviceFeatureExtensions(VkDeviceCreateInfo& deviceInfo)
    {
        
    }
    
    static bool RequiresSwapchainGeneralInitialLayout()
    {
        return false;
    }
    
    static void EnablePresentInfoExtensions(VkPresentInfoKHR& presentInfo)
    {
        
    }
    
    static bool RequiresWaitingForFrameCompletionEvent()
    {
        return true;
    }
};

