#include "DVKTexture2D.h"
#include "DVKBuffer.h"

#include "Math/Math.h"
#include "File/FileManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Loader/stb_image.h"

namespace vk_demo
{
    
    DVKTexture2D* DVKTexture2D::Create(std::shared_ptr<VulkanDevice> vulkanDevice, DVKCommandBuffer* cmdBuffer, const std::string& filename)
    {
        DVKTexture2D* texture = new DVKTexture2D();
        texture->vulkanDevice = vulkanDevice;
        
        uint32 dataSize = 0;
        uint8* dataPtr  = nullptr;
        if (!FileManager::ReadFile(filename, dataPtr, dataSize)) {
            MLOGE("Failed load image : %s", filename.c_str());
            return texture;
        }
        
        // png解析
        int32 comp   = 0;
        int32 width  = 0;
        int32 height = 0;
        uint8* rgbaData = stbi_load_from_memory(dataPtr, dataSize, &width, &height, &comp, 4);
        
        if (rgbaData == nullptr) {
            MLOGE("Failed load image : %s", filename.c_str());
            return texture;
        }
        
        VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
        int32 mipLevels = MMath::FloorToInt(MMath::Log2(MMath::Max(width, height))) + 1;
        VkDevice device = vulkanDevice->GetInstanceHandle();
        
        DVKBuffer* stageingBuffer = DVKBuffer::CreateBuffer(vulkanDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, width * height * 4);
        stageingBuffer->Map();
        stageingBuffer->CopyFrom(rgbaData, width * height * 4);
        
        uint32 memoryTypeIndex = 0;
        VkMemoryRequirements memReqs = {};
        VkMemoryAllocateInfo memAllocInfo;
        ZeroVulkanStruct(memAllocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
        
        // image info
        VkImage                         image = VK_NULL_HANDLE;
        VkImageLayout                   imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkDeviceMemory                  imageMemory = VK_NULL_HANDLE;
        VkImageView                     imageView = VK_NULL_HANDLE;
        VkSampler                       imageSampler = VK_NULL_HANDLE;
        VkDescriptorImageInfo           descriptorInfo;
        
        // 创建image
        VkImageCreateInfo imageCreateInfo;
        ZeroVulkanStruct(imageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
        imageCreateInfo.imageType       = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format          = format;
        imageCreateInfo.mipLevels       = mipLevels;
        imageCreateInfo.arrayLayers     = 1;
        imageCreateInfo.samples         = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling          = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.sharingMode     = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.extent          = { (uint32_t)width, (uint32_t)height, 1 };
        imageCreateInfo.usage           = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        VERIFYVULKANRESULT(vkCreateImage(device, &imageCreateInfo, VULKAN_CPU_ALLOCATOR, &image));
        
        // bind image buffer
        vkGetImageMemoryRequirements(device, image, &memReqs);
        vulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
        memAllocInfo.allocationSize  = memReqs.size;
        memAllocInfo.memoryTypeIndex = memoryTypeIndex;
        VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &imageMemory));
        VERIFYVULKANRESULT(vkBindImageMemory(device, image, imageMemory, 0));
        
        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.levelCount = 1;
        subresourceRange.layerCount = 1;
        
        {
            VkImageMemoryBarrier imageMemoryBarrier;
            ZeroVulkanStruct(imageMemoryBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.image = image;
            imageMemoryBarrier.subresourceRange = subresourceRange;
            vkCmdPipelineBarrier(cmdBuffer->cmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        }
        
        VkBufferImageCopy bufferCopyRegion = {};
        bufferCopyRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        bufferCopyRegion.imageSubresource.mipLevel       = 0;
        bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
        bufferCopyRegion.imageSubresource.layerCount     = 1;
        bufferCopyRegion.imageExtent.width  = width;
        bufferCopyRegion.imageExtent.height = height;
        bufferCopyRegion.imageExtent.depth  = 0;
        
        vkCmdCopyBufferToImage(cmdBuffer->cmdBuffer, stageingBuffer->buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);
        
        {
            VkImageMemoryBarrier imageMemoryBarrier;
            ZeroVulkanStruct(imageMemoryBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            imageMemoryBarrier.image = image;
            imageMemoryBarrier.subresourceRange = subresourceRange;
            vkCmdPipelineBarrier(cmdBuffer->cmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        }
        
        // Generate the mip chain
        for (uint32_t i = 1; i < mipLevels; i++) {
            VkImageBlit imageBlit = {};
            
            imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlit.srcSubresource.layerCount = 1;
            imageBlit.srcSubresource.mipLevel   = i - 1;
            imageBlit.srcOffsets[1].x = int32_t(width >> (i - 1));
            imageBlit.srcOffsets[1].y = int32_t(height >> (i - 1));
            imageBlit.srcOffsets[1].z = 1;
            
            imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlit.dstSubresource.layerCount = 1;
            imageBlit.dstSubresource.mipLevel   = i;
            imageBlit.dstOffsets[1].x = int32_t(width >> i);
            imageBlit.dstOffsets[1].y = int32_t(height >> i);
            imageBlit.dstOffsets[1].z = 1;
            
            VkImageSubresourceRange mipSubRange = {};
            mipSubRange.aspectMask   = VK_IMAGE_ASPECT_COLOR_BIT;
            mipSubRange.baseMipLevel = i;
            mipSubRange.levelCount   = 1;
            mipSubRange.layerCount   = 1;
            
            {
                VkImageMemoryBarrier imageMemoryBarrier;
                ZeroVulkanStruct(imageMemoryBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                imageMemoryBarrier.srcAccessMask = 0;
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                imageMemoryBarrier.image = image;
                imageMemoryBarrier.subresourceRange = mipSubRange;
                vkCmdPipelineBarrier(cmdBuffer->cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
            }
            
            vkCmdBlitImage(cmdBuffer->cmdBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);
            
            {
                VkImageMemoryBarrier imageMemoryBarrier;
                ZeroVulkanStruct(imageMemoryBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                imageMemoryBarrier.image = image;
                imageMemoryBarrier.subresourceRange = mipSubRange;
                vkCmdPipelineBarrier(cmdBuffer->cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
            }
        }
        
        return texture;
    }
    
};
