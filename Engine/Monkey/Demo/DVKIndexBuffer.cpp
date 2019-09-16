#include "DVKIndexBuffer.h"
#include "DVKBuffer.h"
#include "DVKCommand.h"

namespace vk_demo
{
	DVKIndexBuffer* DVKIndexBuffer::Create(std::shared_ptr<VulkanDevice> vulkanDevice, DVKCommandBuffer* cmdBuffer, std::vector<uint16> indices, VkIndexType type)
	{
		VkDevice device = vulkanDevice->GetInstanceHandle();

		DVKIndexBuffer* indexBuffer = new DVKIndexBuffer();
		indexBuffer->device = device;
		indexBuffer->indexCount = indices.size();
		
		vk_demo::DVKBuffer* indexStaging = vk_demo::DVKBuffer::CreateBuffer(
			vulkanDevice, 
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			indices.size() * sizeof(uint16), 
			indices.data()
		);

		indexBuffer->dvkBuffer = vk_demo::DVKBuffer::CreateBuffer(
			vulkanDevice, 
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			indices.size() * sizeof(uint16)
		);

		cmdBuffer->Begin();

		VkBufferCopy copyRegion = {};
		copyRegion.size = indices.size() * sizeof(uint16);

		vkCmdCopyBuffer(cmdBuffer->cmdBuffer, indexStaging->buffer, indexBuffer->dvkBuffer->buffer, 1, &copyRegion);
        
		cmdBuffer->End();
		cmdBuffer->Submit();
        
		delete indexStaging;

		return indexBuffer;
	}
};
