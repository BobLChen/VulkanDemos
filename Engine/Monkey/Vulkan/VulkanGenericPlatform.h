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
    
    static bool LoadVulkanLibrary()
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
    
};

