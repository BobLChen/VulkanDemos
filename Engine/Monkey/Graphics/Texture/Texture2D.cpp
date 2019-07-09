#include "Common/Common.h"
#include "Common/Log.h"

#include "Engine.h"
#include "Texture2D.h"

#include "Math/Math.h"
#include "File/FileManager.h"

#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanMemory.h"
#include "Vulkan/VulkanCommandBuffer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Loader/stb_image.h"

Texture2D::Texture2D()
{

}

Texture2D::~Texture2D()
{

}

void Texture2D::LoadFromFiles(const std::vector<std::string>& filenames)
{
	struct ImageInfo
	{
		int32 width;
		int32 height;
		int32 comp;
		uint8* data;
		uint32 size;
	};

	std::vector<ImageInfo> images(filenames.size());

	for (int32 i = 0; i < filenames.size(); ++i) {
		uint32 dataSize = 0;
		uint8* dataPtr  = nullptr;
		if (!FileManager::ReadFile(filenames[i], dataPtr, dataSize))
		{
			return;
		}

		ImageInfo imageInfo;
		imageInfo.data = stbi_load_from_memory(dataPtr, dataSize, &imageInfo.width, &imageInfo.height, &imageInfo.comp, 0);

		delete[] dataPtr;
		dataSize = -1;
		dataPtr  = nullptr;

		if (!imageInfo.data)
		{
			return;
		}

		if (imageInfo.comp != 4)
		{
			uint32 size = imageInfo.width * imageInfo.height;
			uint8* temp = new uint8[size * 4];
			for (uint32 i = 0; i < size; ++i)
			{
				uint32 idx1 = i * 4;
				uint32 idx0 = i * imageInfo.comp;
				for (int j = 0; j < imageInfo.comp; ++j) 
				{
					temp[idx1 + j] = imageInfo.data[idx0 + j];
				}
				for (int j = imageInfo.comp; j < 4; ++j) 
				{
					temp[idx1 + j] = j == 3 ? 255 : 0;
				}
			}
			free(imageInfo.data);
			imageInfo.data = temp;
			imageInfo.comp = 4;
		}

		images[i] = imageInfo;
	}

	m_Width       = images[0].width;
	m_Height      = images[0].height;
	m_Depth       = 1;
	m_MipLevels   = 1;
	m_LayerCount  = images.size();
	m_ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	m_Format      = VK_FORMAT_R8G8B8A8_UNORM;
	
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
    bufferCreateInfo.size  = m_Width * m_Height * 4 * images.size();
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
	for (int32 i = 0; i < filenames.size(); ++i)
	{
		uint32 size = m_Width * m_Height * 4;
		void* stagingDataPtr = nullptr;
		VERIFYVULKANRESULT(vkMapMemory(device, stagingMemory, i * size, size, 0, &stagingDataPtr));
		std::memcpy(stagingDataPtr, images[i].data, size);
		vkUnmapMemory(device, stagingMemory);
	}

	// 创建image
    VkImageCreateInfo imageCreateInfo;
    ZeroVulkanStruct(imageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
    imageCreateInfo.imageType       = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format          = m_Format;
    imageCreateInfo.mipLevels       = m_MipLevels;
    imageCreateInfo.arrayLayers     = m_LayerCount;
    imageCreateInfo.samples         = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling          = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.sharingMode     = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.extent          = { (uint32_t)m_Width, (uint32_t)m_Height, 1 };
    imageCreateInfo.usage           = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VERIFYVULKANRESULT(vkCreateImage(device, &imageCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Image));

    // bind image buffer
    vkGetImageMemoryRequirements(device, m_Image, &memReqs);
    vulkanRHI->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
    memAllocInfo.allocationSize  = memReqs.size;
    memAllocInfo.memoryTypeIndex = memoryTypeIndex;
    VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &m_ImageMemory));
    VERIFYVULKANRESULT(vkBindImageMemory(device, m_Image, m_ImageMemory, 0));

	VulkanCommandListContextImmediate& context = Engine::Get()->GetVulkanDevice()->GetImmediateContext();
	VulkanCmdBuffer* cmdBuffer = context.GetCommandBufferManager()->GetUploadCmdBuffer();

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = m_LayerCount;
	subresourceRange.baseMipLevel = 0;

	{
		VkImageMemoryBarrier imageMemoryBarrier;
		ZeroVulkanStruct(imageMemoryBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.image = m_Image;
		imageMemoryBarrier.subresourceRange = subresourceRange;
		vkCmdPipelineBarrier(cmdBuffer->GetHandle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	}

	std::vector<VkBufferImageCopy> bufferCopyRegions;
	uint32 offset = 0;
	for (int32 i = 0; i < images.size(); ++i)
	{
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel       = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = i;
		bufferCopyRegion.imageSubresource.layerCount     = 1;
		bufferCopyRegion.imageExtent.width  = m_Width;
		bufferCopyRegion.imageExtent.height = m_Height;
		bufferCopyRegion.imageExtent.depth  = 1;
		bufferCopyRegion.bufferOffset       = offset;
		bufferCopyRegions.push_back(bufferCopyRegion);
		offset += m_Width * m_Height * 4;
	}
	
	vkCmdCopyBufferToImage(cmdBuffer->GetHandle(), stagingBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, bufferCopyRegions.size(), bufferCopyRegions.data());

	{
		VkImageMemoryBarrier imageMemoryBarrier;
		ZeroVulkanStruct(imageMemoryBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imageMemoryBarrier.image = m_Image;
		imageMemoryBarrier.subresourceRange = subresourceRange;
		vkCmdPipelineBarrier(cmdBuffer->GetHandle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	}

	context.GetCommandBufferManager()->SubmitUploadCmdBuffer();

	vkFreeMemory(device, stagingMemory, VULKAN_CPU_ALLOCATOR);
	vkDestroyBuffer(device, stagingBuffer, VULKAN_CPU_ALLOCATOR);

	VkSamplerCreateInfo samplerInfo;
	ZeroVulkanStruct(samplerInfo, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerInfo.maxLod = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.anisotropyEnable = VK_FALSE;
	VERIFYVULKANRESULT(vkCreateSampler(device, &samplerInfo, VULKAN_CPU_ALLOCATOR, &m_ImageSampler));
	
	VkImageViewCreateInfo viewInfo;
	ZeroVulkanStruct(viewInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	viewInfo.image = m_Image;
	viewInfo.format = m_Format;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.layerCount = m_LayerCount;
	viewInfo.subresourceRange.levelCount = 1;
	VERIFYVULKANRESULT(vkCreateImageView(device, &viewInfo, VULKAN_CPU_ALLOCATOR, &m_ImageView));

	m_DescriptorInfo.sampler	 = m_ImageSampler;
	m_DescriptorInfo.imageView	 = m_ImageView;
	m_DescriptorInfo.imageLayout = m_ImageLayout;

	m_Invalid = false;

    MLOG("Texture2D Array create success. size=%dx%d,Layer=%d", m_Width, m_Height, m_LayerCount);
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
	m_Depth       = 1;
	m_LayerCount  = 1;
	m_Format      = VK_FORMAT_R8G8B8A8_UNORM;
    m_MipLevels	  = MMath::FloorToInt(MMath::Log2(MMath::Max(width, height))) + 1;
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

	// 创建image
    VkImageCreateInfo imageCreateInfo;
    ZeroVulkanStruct(imageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
    imageCreateInfo.imageType       = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format          = m_Format;
    imageCreateInfo.mipLevels       = m_MipLevels;
    imageCreateInfo.arrayLayers     = 1;
    imageCreateInfo.samples         = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling          = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.sharingMode     = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.extent          = { (uint32_t)width, (uint32_t)height, 1 };
    imageCreateInfo.usage           = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VERIFYVULKANRESULT(vkCreateImage(device, &imageCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Image));

    // bind image buffer
    vkGetImageMemoryRequirements(device, m_Image, &memReqs);
    vulkanRHI->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
    memAllocInfo.allocationSize  = memReqs.size;
    memAllocInfo.memoryTypeIndex = memoryTypeIndex;
    VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &m_ImageMemory));
    VERIFYVULKANRESULT(vkBindImageMemory(device, m_Image, m_ImageMemory, 0));

	VulkanCommandListContextImmediate& context = Engine::Get()->GetVulkanDevice()->GetImmediateContext();
	VulkanCmdBuffer* cmdBuffer = context.GetCommandBufferManager()->GetUploadCmdBuffer();

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = m_LayerCount;

	{
		VkImageMemoryBarrier imageMemoryBarrier;
		ZeroVulkanStruct(imageMemoryBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.image = m_Image;
		imageMemoryBarrier.subresourceRange = subresourceRange;
		vkCmdPipelineBarrier(cmdBuffer->GetHandle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	}

	VkBufferImageCopy bufferCopyRegion = {};
	bufferCopyRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferCopyRegion.imageSubresource.mipLevel       = 0;
	bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
	bufferCopyRegion.imageSubresource.layerCount     = 1;
	bufferCopyRegion.imageExtent.width  = m_Width;
	bufferCopyRegion.imageExtent.height = m_Height;
	bufferCopyRegion.imageExtent.depth  = m_Depth;

	vkCmdCopyBufferToImage(cmdBuffer->GetHandle(), stagingBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);

	{
		VkImageMemoryBarrier imageMemoryBarrier;
		ZeroVulkanStruct(imageMemoryBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.image = m_Image;
		imageMemoryBarrier.subresourceRange = subresourceRange;
		vkCmdPipelineBarrier(cmdBuffer->GetHandle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	}

	vkFreeMemory(device, stagingMemory, VULKAN_CPU_ALLOCATOR);
	vkDestroyBuffer(device, stagingBuffer, VULKAN_CPU_ALLOCATOR);

	// Generate the mip chain
	for (uint32_t i = 1; i < m_MipLevels; i++) {
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
			imageMemoryBarrier.image = m_Image;
			imageMemoryBarrier.subresourceRange = mipSubRange;
			vkCmdPipelineBarrier(cmdBuffer->GetHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		}

		vkCmdBlitImage(cmdBuffer->GetHandle(), m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

		{
			VkImageMemoryBarrier imageMemoryBarrier;
			ZeroVulkanStruct(imageMemoryBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			imageMemoryBarrier.image = m_Image;
			imageMemoryBarrier.subresourceRange = mipSubRange;
			vkCmdPipelineBarrier(cmdBuffer->GetHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		}
	}

	subresourceRange.levelCount = m_MipLevels;
	
	{
		VkImageMemoryBarrier imageMemoryBarrier;
		ZeroVulkanStruct(imageMemoryBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.image = m_Image;
		imageMemoryBarrier.subresourceRange = subresourceRange;
		vkCmdPipelineBarrier(cmdBuffer->GetHandle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	}

	context.GetCommandBufferManager()->SubmitUploadCmdBuffer();

	VkSamplerCreateInfo samplerInfo;
	ZeroVulkanStruct(samplerInfo, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerInfo.maxAnisotropy = 1.0;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxLod = (float)m_MipLevels;
	VERIFYVULKANRESULT(vkCreateSampler(device, &samplerInfo, VULKAN_CPU_ALLOCATOR, &m_ImageSampler));
	
	VkImageViewCreateInfo viewInfo;
	ZeroVulkanStruct(viewInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
	viewInfo.image = m_Image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = m_Format;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.layerCount = 1;
	viewInfo.subresourceRange.levelCount = m_MipLevels;
	VERIFYVULKANRESULT(vkCreateImageView(device, &viewInfo, VULKAN_CPU_ALLOCATOR, &m_ImageView));

	m_DescriptorInfo.sampler	 = m_ImageSampler;
	m_DescriptorInfo.imageView	 = m_ImageView;
	m_DescriptorInfo.imageLayout = m_ImageLayout;

	m_Invalid = false;

    MLOG("Texture2D create success. size=%dx%d", width, height);
}
