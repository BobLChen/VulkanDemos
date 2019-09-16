#pragma once

#include <string>
#include <cstring>
#include <memory>

#include "FileManager.h"
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

	inline VkImageLayout GetImageLayout(ImageLayoutBarrier target)
	{
		VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;

		switch (target)
		{
			case ImageLayoutBarrier::Undefined:
			{
				layout = VK_IMAGE_LAYOUT_UNDEFINED;
			}
			break;

			case ImageLayoutBarrier::TransferDest:
			{
				layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			}
			break;

			case ImageLayoutBarrier::ColorAttachment:
			{
				layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
			break;

			case ImageLayoutBarrier::DepthStencilAttachment:
			{
				layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}
			break;

			case ImageLayoutBarrier::TransferSource:
			{
				layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			}
			break;

			case ImageLayoutBarrier::Present:
			{
				layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			}
			break;

			case ImageLayoutBarrier::PixelShaderRead:
			{
				layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}
			break;

			case ImageLayoutBarrier::PixelDepthStencilRead:
			{
				layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			}
			break;

			case ImageLayoutBarrier::ComputeGeneralRW:
			{
				layout = VK_IMAGE_LAYOUT_GENERAL;
			}
			break;

			case ImageLayoutBarrier::PixelGeneralRW:
			{
				layout = VK_IMAGE_LAYOUT_GENERAL;
			}
			break;

			default:
			{
				MLOGE("Unknown ImageLayoutBarrier %d", (int32)target);
			}
			break;
		}

		return layout;
	}
    
    inline VkPipelineStageFlags GetImageBarrierFlags(ImageLayoutBarrier target, VkAccessFlags& accessFlags, VkImageLayout& layout)
    {
        VkPipelineStageFlags stageFlags = (VkPipelineStageFlags)0;
        switch (target)
        {
            case ImageLayoutBarrier::Undefined:
            {
                accessFlags = 0;
                stageFlags  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                layout      = VK_IMAGE_LAYOUT_UNDEFINED;
            }
            break;
                
            case ImageLayoutBarrier::TransferDest:
            {
                accessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
                stageFlags  = VK_PIPELINE_STAGE_TRANSFER_BIT;
                layout      = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            }
            break;
                
            case ImageLayoutBarrier::ColorAttachment:
            {
                accessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                stageFlags  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
            break;
            
            case ImageLayoutBarrier::DepthStencilAttachment:
            {
                accessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                stageFlags  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                layout      = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }
            break;
            
            case ImageLayoutBarrier::TransferSource:
            {
                accessFlags = VK_ACCESS_TRANSFER_READ_BIT;
                stageFlags  = VK_PIPELINE_STAGE_TRANSFER_BIT;
                layout      = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            }
            break;
                
            case ImageLayoutBarrier::Present:
            {
                accessFlags = 0;
                stageFlags  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                layout      = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            }
            break;
                
            case ImageLayoutBarrier::PixelShaderRead:
            {
                accessFlags = VK_ACCESS_SHADER_READ_BIT;
                stageFlags  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                layout      = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
            break;
                
            case ImageLayoutBarrier::PixelDepthStencilRead:
            {
                accessFlags = VK_ACCESS_SHADER_READ_BIT;
                stageFlags  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                layout      = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            }
            break;
                
            case ImageLayoutBarrier::ComputeGeneralRW:
            {
                accessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                stageFlags  = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
                layout      = VK_IMAGE_LAYOUT_GENERAL;
            }
            break;
                
            case ImageLayoutBarrier::PixelGeneralRW:
            {
                accessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                stageFlags  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                layout      = VK_IMAGE_LAYOUT_GENERAL;
            }
            break;
                
            default:
            {
                MLOGE("Unknown ImageLayoutBarrier %d", (int32)target);
            }
            break;
        }
        
        return stageFlags;
    }
    
    inline void SetImageBarrierInfo(ImageLayoutBarrier source, ImageLayoutBarrier dest, VkImageMemoryBarrier& inOutBarrier, VkPipelineStageFlags& inOutSourceStage, VkPipelineStageFlags& inOutDestStage)
    {
        inOutSourceStage |= GetImageBarrierFlags(source, inOutBarrier.srcAccessMask, inOutBarrier.oldLayout);
        inOutDestStage   |= GetImageBarrierFlags(dest,   inOutBarrier.dstAccessMask, inOutBarrier.newLayout);
    }
    
    inline void ImagePipelineBarrier(VkCommandBuffer cmdBuffer, VkImage image, ImageLayoutBarrier source, ImageLayoutBarrier dest, const VkImageSubresourceRange& subresourceRange)
    {
        VkImageMemoryBarrier imageBarrier;
        ZeroVulkanStruct(imageBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
        imageBarrier.image               = image;
        imageBarrier.subresourceRange    = subresourceRange;
        imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        
        VkPipelineStageFlags sourceStages = (VkPipelineStageFlags)0;
        VkPipelineStageFlags destStages   = (VkPipelineStageFlags)0;
        SetImageBarrierInfo(source, dest, imageBarrier, sourceStages, destStages);
        
        if (source == ImageLayoutBarrier::Present) {
            sourceStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        }
        else if (dest == ImageLayoutBarrier::Present) {
            destStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        }
        
        vkCmdPipelineBarrier(cmdBuffer, sourceStages, destStages, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
    }
    
}
