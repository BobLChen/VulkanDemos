#pragma once

#include "Engine.h"
#include "DVKCommon.h"

#include "Common/Common.h"
#include "Math/Math.h"

#include "Vulkan/VulkanCommon.h"

#include <vector>

namespace vk_demo
{
  
    class DVKTexture2D
    {
    public:
        DVKTexture2D()
        {
            
        }
        
        ~DVKTexture2D()
        {
            VkDevice device = vulkanDevice->GetInstanceHandle();
            
            if (imageView != VK_NULL_HANDLE) {
                vkDestroyImageView(device, imageView, VULKAN_CPU_ALLOCATOR);
                imageView = VK_NULL_HANDLE;
            }
            
            if (image != VK_NULL_HANDLE) {
                vkDestroyImage(device, image, VULKAN_CPU_ALLOCATOR);
                image = VK_NULL_HANDLE;
            }
            
            if (imageSampler != VK_NULL_HANDLE) {
                vkDestroySampler(device, imageSampler, VULKAN_CPU_ALLOCATOR);
                imageSampler = VK_NULL_HANDLE;
            }
            
            if (imageMemory != VK_NULL_HANDLE) {
                vkFreeMemory(device, imageMemory, VULKAN_CPU_ALLOCATOR);
                imageMemory = VK_NULL_HANDLE;
            }
        }
        
        static DVKTexture2D* Create(std::shared_ptr<VulkanDevice> vulkanDevice, DVKCommandBuffer* cmdBuffer, const std::string& filename);
        
    public:
        std::shared_ptr<VulkanDevice>   vulkanDevice = nullptr;
        
        VkImage                         image = VK_NULL_HANDLE;
        VkImageLayout                   imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkDeviceMemory                  imageMemory = VK_NULL_HANDLE;
        VkImageView                     imageView = VK_NULL_HANDLE;
        VkSampler                       imageSampler = VK_NULL_HANDLE;
        VkDescriptorImageInfo           descriptorInfo;
        
        int32                           width = 0;
        int32                           height = 0;
        int32                           mipLevels = 0;
        VkFormat                        format = VK_FORMAT_R8G8B8A8_UNORM;
    };
    
};
