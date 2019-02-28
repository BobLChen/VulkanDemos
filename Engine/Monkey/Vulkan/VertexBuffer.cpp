#include "Math/Math.h"

#include "VertexBuffer.h"
#include "VulkanRHI.h"

#include <string>
#include <cstring>

VertexBuffer::VertexBuffer()
    : m_VertexCount(0)
    , m_DataSize(0)
    , m_CurrentChannels(0)
	, m_Uploaded(false)
{
    
}

VertexBuffer::~VertexBuffer()
{
	for (int32 i = 0; i < m_Streams.size(); ++i)
	{
		delete[] m_Datas[i];
	}
	m_Datas.clear();
}

void VertexBuffer::AddStream(const VertexStreamInfo& streamInfo, const std::vector<VertexChannelInfo>& channels, uint8* dataPtr)
{
	uint32 newChannelMask = 0;
	for (int32 i = 0; i < channels.size(); ++i)
	{
		newChannelMask = (1 << (int32)channels[i].attribute) | newChannelMask;
	}
	
	int32 stride = 0;
	for (int32 i = 0; i < channels.size(); ++i)
	{
		stride += ElementTypeToSize(channels[i].format);
		m_Channels.push_back(channels[i]);
	}
	
	m_DataSize += streamInfo.size;
	m_VertexCount = streamInfo.size / stride;
	m_Datas.push_back(dataPtr);
	m_Streams.push_back(streamInfo);
	m_CurrentChannels = m_CurrentChannels | newChannelMask;
}

void VertexBuffer::Upload(std::shared_ptr<VulkanRHI> vulkanRHI)
{
	if (m_Uploaded)
	{
		return;
	}

	std::vector<float> vertices = {
		1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
	   -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};
	
	VkDevice device = vulkanRHI->GetDevice()->GetInstanceHandle();
	VkMemoryRequirements memReqInfo;
	VkMemoryAllocateInfo memAllocInfo;
	ZeroVulkanStruct(memAllocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
	VkBufferCreateInfo bufferCreateInfo;
	ZeroVulkanStruct(bufferCreateInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
	
	// ------------------------ host visible ------------------------
	std::vector<VkBuffer> hostBuffers(m_Streams.size());
	std::vector<VkDeviceMemory> hostMemories(m_Streams.size());

	for (int32 i = 0; i < m_Streams.size(); ++i)
	{
		bufferCreateInfo.size  = m_Streams[i].size;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(device, &bufferCreateInfo, VULKAN_CPU_ALLOCATOR, &hostBuffers[i]));

		vkGetBufferMemoryRequirements(device, hostBuffers[i], &memReqInfo);
		uint32 memoryTypeIndex = 0;
		vulkanRHI->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);
		m_Streams[i].alignment       = memReqInfo.alignment;
		m_Streams[i].allocationSize  = memReqInfo.size;
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &hostMemories[i]));
		VERIFYVULKANRESULT(vkBindBufferMemory(device, hostBuffers[i], hostMemories[i], 0));

		void* dataPtr = nullptr;
		VERIFYVULKANRESULT(vkMapMemory(device, hostMemories[i], 0, memAllocInfo.allocationSize, 0, &dataPtr));
		std::memcpy(dataPtr, m_Datas[i], m_Streams[i].size);
		vkUnmapMemory(device, hostMemories[i]);
	}
	
	// ------------------------ device local ------------------------
	m_Buffers.resize(m_Streams.size());
	m_Memories.resize(m_Streams.size());

	for (int32 i = 0; i < m_Streams.size(); ++i)
	{
		bufferCreateInfo.size  = m_Streams[i].size;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(device, &bufferCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Buffers[i]));

		vkGetBufferMemoryRequirements(device, m_Buffers[i], &memReqInfo);
		uint32 memoryTypeIndex = 0;
		vulkanRHI->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &m_Memories[i]));
		VERIFYVULKANRESULT(vkBindBufferMemory(device, m_Buffers[i], m_Memories[i], 0));
	}
	
	// TODO:放到延迟队列中创建
	VkCommandBuffer xferCmdBuffer;
	VkCommandBufferAllocateInfo xferCmdBufferInfo;
	ZeroVulkanStruct(xferCmdBufferInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
	xferCmdBufferInfo.commandPool = vulkanRHI->GetCommandPool();
	xferCmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	xferCmdBufferInfo.commandBufferCount = 1;
	VERIFYVULKANRESULT(vkAllocateCommandBuffers(device, &xferCmdBufferInfo, &xferCmdBuffer));

	VkCommandBufferBeginInfo cmdBufferBeginInfo;
	ZeroVulkanStruct(cmdBufferBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
	VERIFYVULKANRESULT(vkBeginCommandBuffer(xferCmdBuffer, &cmdBufferBeginInfo));

	for (int32 i = 0; i < m_Streams.size(); ++i)
	{
		VkBufferCopy copyRegion = {};
		copyRegion.size = m_Streams[i].allocationSize;
		vkCmdCopyBuffer(xferCmdBuffer, hostBuffers[i], m_Buffers[i], 1, &copyRegion);
	}

	VERIFYVULKANRESULT(vkEndCommandBuffer(xferCmdBuffer));
	
	VkSubmitInfo submitInfo;
	ZeroVulkanStruct(submitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &xferCmdBuffer;

	VkFenceCreateInfo fenceInfo;
	ZeroVulkanStruct(fenceInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
	fenceInfo.flags = 0;

	VkFence fence;
	VERIFYVULKANRESULT(vkCreateFence(device, &fenceInfo, VULKAN_CPU_ALLOCATOR, &fence));
	VERIFYVULKANRESULT(vkQueueSubmit(vulkanRHI->GetDevice()->GetGraphicsQueue()->GetHandle(), 1, &submitInfo, fence));
	VERIFYVULKANRESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, MAX_int64));

	vkDestroyFence(device, fence, VULKAN_CPU_ALLOCATOR);
	vkFreeCommandBuffers(device, vulkanRHI->GetCommandPool(), 1, &xferCmdBuffer);

	for (int32 i = 0; i < m_Streams.size(); ++i)
	{
		vkDestroyBuffer(device, hostBuffers[i], VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(device, hostMemories[i], VULKAN_CPU_ALLOCATOR);
	}

	m_Uploaded = true;
}

void VertexBuffer::Download(std::shared_ptr<VulkanRHI> vulkanRHI)
{
	if (!m_Uploaded) 
	{
		return;
	}

	for (int32 i = 0; i < m_Streams.size(); ++i)
	{
		vkDestroyBuffer(vulkanRHI->GetDevice()->GetInstanceHandle(), m_Buffers[i], VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(vulkanRHI->GetDevice()->GetInstanceHandle(), m_Memories[i], VULKAN_CPU_ALLOCATOR);
	}

	m_Uploaded = false;
}