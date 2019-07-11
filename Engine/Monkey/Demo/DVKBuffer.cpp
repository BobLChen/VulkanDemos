#include "Vulkan/VulkanCommon.h"

#include "DVKBuffer.h"
#include "DVKUtils.h"

namespace vk_demo
{
	DVKBuffer* DVKBuffer::CreateBuffer(std::shared_ptr<VulkanDevice> vulkanDevice, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void *data)
	{
		DVKBuffer* dvkBuffer = new DVKBuffer();
		dvkBuffer->device = vulkanDevice->GetInstanceHandle();
		
		VkDevice vkDevice = vulkanDevice->GetInstanceHandle();
		
		uint32 memoryTypeIndex = 0;
		VkMemoryRequirements memReqs = {};
		VkMemoryAllocateInfo memAlloc;
		ZeroVulkanStruct(memAlloc, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
		
		VkBufferCreateInfo bufferCreateInfo;
		ZeroVulkanStruct(bufferCreateInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
		bufferCreateInfo.usage = usageFlags;
		bufferCreateInfo.size  = size;
		vkCreateBuffer(vkDevice, &bufferCreateInfo, nullptr, &(dvkBuffer->buffer));

		vkGetBufferMemoryRequirements(vkDevice, dvkBuffer->buffer, &memReqs);
		vulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, memoryPropertyFlags, &memoryTypeIndex);
		memAlloc.allocationSize  = memReqs.size;
		memAlloc.memoryTypeIndex = memoryTypeIndex;
		
		vkAllocateMemory(vkDevice, &memAlloc, nullptr, &dvkBuffer->memory);

		dvkBuffer->size       = memAlloc.allocationSize;
		dvkBuffer->alignment  = memReqs.alignment;
		dvkBuffer->usageFlags = usageFlags;
		dvkBuffer->memoryPropertyFlags = memoryPropertyFlags;

		if (data != nullptr)
		{
			dvkBuffer->Map();
			memcpy(dvkBuffer->mapped, data, size);
			if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
				dvkBuffer->Flush();
			}
			dvkBuffer->UnMap();
		}

		dvkBuffer->SetupDescriptor();
		dvkBuffer->Bind();

		return dvkBuffer;
	}

	VkResult DVKBuffer::Map(VkDeviceSize size, VkDeviceSize offset)
	{
		if (mapped) {
			return VK_SUCCESS;
		}
		return vkMapMemory(device, memory, offset, size, 0, &mapped);
	}

	void DVKBuffer::UnMap()
	{
		if (!mapped) {
			return;
		}
		vkUnmapMemory(device, memory);
		mapped = nullptr;
	}

	VkResult DVKBuffer::Bind(VkDeviceSize offset)
	{
		return vkBindBufferMemory(device, buffer, memory, offset);
	}

	void DVKBuffer::SetupDescriptor(VkDeviceSize size, VkDeviceSize offset)
	{
		descriptor.offset = offset;
		descriptor.buffer = buffer;
		descriptor.range  = size;
	}

	void DVKBuffer::CopyFrom(void* data, VkDeviceSize size)
	{
		if (!mapped) {
			return;
		}
		memcpy(mapped, data, size);
	}

	VkResult DVKBuffer::Flush(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size   = size;
		return vkFlushMappedMemoryRanges(device, 1, &mappedRange);
	}

	VkResult DVKBuffer::Invalidate(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size   = size;
		return vkInvalidateMappedMemoryRanges(device, 1, &mappedRange);
	}

}
