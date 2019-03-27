#include "Common/Common.h"
#include "Common/Log.h"
#include "Engine.h"
#include "Texture2D.h"
#include "Math/Math.h"
#include "File/FileManager.h"
#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanMemory.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Loader/stb_image.h"

Texture2D::Texture2D()
{

}

Texture2D::~Texture2D()
{

}

void Texture2D::LoadFromFile(const std::string& filename)
{
    // 加载PNG图片数据
	uint32 dataSize = 0;
	uint8* dataPtr  = nullptr;
	if (!FileManager::ReadFile(filename, dataPtr, dataSize))
	{
		return;
	}
    
    // png解析
    int32 width  = -1;
    int32 height = -1;
    int32 comp   = 0;
    uint8* rgbaData = stbi_load_from_memory(dataPtr, dataSize, &width, &height, &comp, 4);
    
    if (rgbaData == nullptr)
    {
        MLOGE("Failed load image : %s", filename.c_str());
        return;
    }
    
	m_Width       = width;
	m_Height      = height;
    m_MipLevels	  = 1;
	m_ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    
    std::shared_ptr<VulkanRHI> vulkanRHI = Engine::Get()->GetVulkanRHI();
    VkDevice device = vulkanRHI->GetDevice()->GetInstanceHandle();
    uint32 memoryTypeIndex = 0;
    VkMemoryRequirements memReqs = {};
    VkMemoryAllocateInfo memAllocInfo;
    ZeroVulkanStruct(memAllocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
    
    // 准备staging数据
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
    // staging buffer
    VkBufferCreateInfo bufferCreateInfo;
    ZeroVulkanStruct(bufferCreateInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
    bufferCreateInfo.size  = width * height * 4;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VERIFYVULKANRESULT(vkCreateBuffer(device, &bufferCreateInfo, VULKAN_CPU_ALLOCATOR, &stagingBuffer));
    // bind staging memory
    vkGetBufferMemoryRequirements(device, stagingBuffer, &memReqs);
    vulkanRHI->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);
    memAllocInfo.allocationSize  = memReqs.size;
    memAllocInfo.memoryTypeIndex = memoryTypeIndex;
    VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &stagingMemory));
    VERIFYVULKANRESULT(vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0));
    // 将数据拷贝到staging buffer
    void* stagingDataPtr = nullptr;
    VERIFYVULKANRESULT(vkMapMemory(device, stagingMemory, 0, memReqs.size, 0, &stagingDataPtr));
    std::memcpy(stagingDataPtr, rgbaData, bufferCreateInfo.size);
    vkUnmapMemory(device, stagingMemory);
    // 设置mipmap数据范围为将Buffer拷贝到Image做准备
    std::vector<VkBufferImageCopy> bufferCopyRegions;
    uint32 offset = 0;
    for (int32 i = 0; i < m_MipLevels; ++i)
    {
        VkBufferImageCopy bufferCopyRegion = {};
        bufferCopyRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        bufferCopyRegion.imageSubresource.mipLevel       = i;
        bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
        bufferCopyRegion.imageSubresource.layerCount     = 1;
        bufferCopyRegion.imageExtent.width  = width;
        bufferCopyRegion.imageExtent.height = height;
        bufferCopyRegion.imageExtent.depth  = 1;
        bufferCopyRegion.bufferOffset       = offset;
        bufferCopyRegions.push_back(bufferCopyRegion);
        // offset += mipmap size;
    }
    
    // 创建image
    VkImageCreateInfo imageCreateInfo;
    ZeroVulkanStruct(imageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
    imageCreateInfo.imageType       = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format          = VK_FORMAT_R8G8B8A8_UNORM;
    imageCreateInfo.mipLevels       = m_MipLevels;
    imageCreateInfo.arrayLayers     = 1;
    imageCreateInfo.samples         = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling          = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.sharingMode     = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.extent          = { (uint32_t)width, (uint32_t)height, 1 };
    imageCreateInfo.usage           = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VERIFYVULKANRESULT(vkCreateImage(device, &imageCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Image));
    // bind image buffer
    vkGetImageMemoryRequirements(device, m_Image, &memReqs);
    vulkanRHI->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
    memAllocInfo.allocationSize  = memReqs.size;
    memAllocInfo.memoryTypeIndex = memoryTypeIndex;
    VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &m_ImageMemory));
    VERIFYVULKANRESULT(vkBindImageMemory(device, m_Image, m_ImageMemory, 0));
    
    // 创建Command并开始录制命令
    VkCommandBuffer copyCommand = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo copyCommandInfo;
    ZeroVulkanStruct(copyCommandInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
    copyCommandInfo.commandPool = vulkanRHI->GetCommandPool();
    copyCommandInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    copyCommandInfo.commandBufferCount = 1;
    VERIFYVULKANRESULT(vkAllocateCommandBuffers(device, &copyCommandInfo, &copyCommand));
    // 开始录制
    VkCommandBufferBeginInfo commandBeginInfo;
    ZeroVulkanStruct(commandBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
    VERIFYVULKANRESULT(vkBeginCommandBuffer(copyCommand, &commandBeginInfo));
    
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask   = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount   = m_MipLevels;
    subresourceRange.layerCount   = 1;
    
    // barrier0
    VkImageMemoryBarrier imageMemoryBarrier;
    ZeroVulkanStruct(imageMemoryBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.image     = m_Image;
    imageMemoryBarrier.subresourceRange = subresourceRange;
    imageMemoryBarrier.srcAccessMask    = 0;
    imageMemoryBarrier.dstAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(copyCommand, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
    vkCmdCopyBufferToImage(copyCommand, stagingBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (uint32)bufferCopyRegions.size(), bufferCopyRegions.data());
    
    // barrier1
    ZeroVulkanStruct(imageMemoryBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageMemoryBarrier.image     = m_Image;
    imageMemoryBarrier.subresourceRange = subresourceRange;
    imageMemoryBarrier.srcAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask    = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(copyCommand, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
    
    // 结束录制
    VERIFYVULKANRESULT(vkEndCommandBuffer(copyCommand));
    
    // 提交信息
    VkSubmitInfo submitInfo;
    ZeroVulkanStruct(submitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &copyCommand;
    
    // 创建fence同步cpu-gpu
    VkFenceCreateInfo fenceInfo;
    ZeroVulkanStruct(fenceInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
    fenceInfo.flags = 0;
    VkFence fence;
    VERIFYVULKANRESULT(vkCreateFence(device, &fenceInfo, VULKAN_CPU_ALLOCATOR, &fence));
    
    // 提交命令并等待执行完毕
    VERIFYVULKANRESULT(vkQueueSubmit(vulkanRHI->GetDevice()->GetGraphicsQueue()->GetHandle(), 1, &submitInfo, fence));
    VERIFYVULKANRESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, MAX_int64));
    
    vkDestroyFence(device, fence, VULKAN_CPU_ALLOCATOR);
    vkFreeCommandBuffers(device, vulkanRHI->GetCommandPool(), 1, &copyCommand);
    vkFreeMemory(device, stagingMemory, VULKAN_CPU_ALLOCATOR);
    vkDestroyBuffer(device, stagingBuffer, VULKAN_CPU_ALLOCATOR);
    
	// 创建Sampler
	VkSamplerCreateInfo samplerCreateInfo;
	ZeroVulkanStruct(samplerCreateInfo, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
	samplerCreateInfo.magFilter			= VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter			= VK_FILTER_LINEAR;
	samplerCreateInfo.mipmapMode		= VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU		= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV		= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW		= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.mipLodBias		= 0.0f;
	samplerCreateInfo.compareOp			= VK_COMPARE_OP_NEVER;
	samplerCreateInfo.minLod			= 0.0f;
	samplerCreateInfo.maxLod			= m_MipLevels;
	samplerCreateInfo.maxAnisotropy		= 1.0f;
	samplerCreateInfo.anisotropyEnable	= VK_FALSE;
	samplerCreateInfo.borderColor		= VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VERIFYVULKANRESULT(vkCreateSampler(device, &samplerCreateInfo, VULKAN_CPU_ALLOCATOR, &m_ImageSampler));

	// 创建ImageView
	VkImageViewCreateInfo imageViewCreateInfo;
	ZeroVulkanStruct(imageViewCreateInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
	imageViewCreateInfo.image		= m_Image;
	imageViewCreateInfo.viewType	= VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format		= VK_FORMAT_R8G8B8A8_UNORM;
	imageViewCreateInfo.components	= { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	imageViewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	imageViewCreateInfo.subresourceRange.levelCount = m_MipLevels;
	VERIFYVULKANRESULT(vkCreateImageView(device, &imageViewCreateInfo, VULKAN_CPU_ALLOCATOR, &m_ImageView));
	
	m_DescriptorInfo.sampler	 = m_ImageSampler;
	m_DescriptorInfo.imageView	 = m_ImageView;
	m_DescriptorInfo.imageLayout = m_ImageLayout;

    MLOG("Image create success. size=%dx%d", width, height);
}
