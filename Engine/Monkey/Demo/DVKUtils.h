#pragma once

#include <string>
#include <cstring>
#include <memory>

#include "File/FileManager.h"
#include "Vulkan/VulkanCommon.h"

namespace vk_demo
{
    inline VkShaderModule LoadSPIPVShader(VkDevice device, const std::string& filepath)
    {
        uint8* dataPtr  = nullptr;
        uint32 dataSize = 0;
        FileManager::ReadFile(filepath, dataPtr, dataSize);
        
        VkShaderModuleCreateInfo moduleCreateInfo;
        ZeroVulkanStruct(moduleCreateInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
        moduleCreateInfo.codeSize = dataSize;
        moduleCreateInfo.pCode    = (uint32_t*)dataPtr;
        
        VkShaderModule shaderModule;
        VERIFYVULKANRESULT(vkCreateShaderModule(device, &moduleCreateInfo, VULKAN_CPU_ALLOCATOR, &shaderModule));
        delete[] dataPtr;
        
        return shaderModule;
    }
    
}
