#include "Math/Math.h"
#include "Vulkan/VulkanRHI.h"
#include "Engine.h"
#include "IndexBuffer.h"

IndexBuffer::IndexBuffer(uint8* dataPtr, uint32 dataSize, PrimitiveType primitiveType, VkIndexType indexType)
	: m_IndexType(indexType)
	, m_PrimitiveType(primitiveType)
	, m_Data(dataPtr)
	, m_DataSize(dataSize)
	, m_IndexCount(0)
	, m_TriangleCount(0)
	, m_Buffer(VK_NULL_HANDLE)
	, m_Memory(VK_NULL_HANDLE)
	, m_Valid(false)
	, m_AllocationSize(0)
	, m_Alignment(0)
{
	m_IndexCount = dataSize / IndexTypeToSize(indexType);
	m_TriangleCount = m_IndexCount / PrimitiveTypeToSize(primitiveType);
}

IndexBuffer::~IndexBuffer()
{
	DestroyBuffer();
	
	if (m_Data) {
		delete[] m_Data;
		m_Data = nullptr;
	}
}

void IndexBuffer::CreateBuffer()
{
	if (m_Valid)
	{
		return;
	}

	void* dataPtr = nullptr;
	uint32 memoryTypeIndex = 0;
	VkMemoryRequirements memReqInfo;

	VkMemoryAllocateInfo memAllocInfo;
	ZeroVulkanStruct(memAllocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

	VkBuffer hostBuffer;
	VkDeviceMemory hostMemory;
	std::shared_ptr<VulkanRHI> vulkanRHI = Engine::Get()->GetVulkanRHI();
	std::shared_ptr<VulkanDevice> vulkanDevice = Engine::Get()->GetVulkanRHI()->GetDevice();
	VkDevice device = vulkanDevice->GetInstanceHandle();

	// index buffer
	VkBufferCreateInfo bufferCreateInfo;
	ZeroVulkanStruct(bufferCreateInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
	bufferCreateInfo.size = m_DataSize;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VERIFYVULKANRESULT(vkCreateBuffer(device, &bufferCreateInfo, VULKAN_CPU_ALLOCATOR, &hostBuffer));

	vkGetBufferMemoryRequirements(device, hostBuffer, &memReqInfo);
	vulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);
	memAllocInfo.allocationSize = memReqInfo.size;
	memAllocInfo.memoryTypeIndex = memoryTypeIndex;
	m_Alignment = memReqInfo.alignment;
	m_AllocationSize = memReqInfo.size;

	VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &hostMemory));
	VERIFYVULKANRESULT(vkBindBufferMemory(device, hostBuffer, hostMemory, 0));

	VERIFYVULKANRESULT(vkMapMemory(device, hostMemory, 0, memAllocInfo.allocationSize, 0, &dataPtr));
	std::memcpy(dataPtr, m_Data, bufferCreateInfo.size);
	vkUnmapMemory(device, hostMemory);

	bufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	VERIFYVULKANRESULT(vkCreateBuffer(device, &bufferCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Buffer));

	vkGetBufferMemoryRequirements(device, m_Buffer, &memReqInfo);
	vulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
	memAllocInfo.allocationSize = memReqInfo.size;
	memAllocInfo.memoryTypeIndex = memoryTypeIndex;
	VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &m_Memory));
	VERIFYVULKANRESULT(vkBindBufferMemory(device, m_Buffer, m_Memory, 0));

	// TODO:延迟的专用队列拷贝
	VkCommandBuffer xferCmdBuffer;
	VkCommandBufferAllocateInfo xferCmdBufferInfo;
	ZeroVulkanStruct(xferCmdBufferInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
	xferCmdBufferInfo.commandPool = vulkanRHI->GetCommandPool();
	xferCmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	xferCmdBufferInfo.commandBufferCount = 1;
	VERIFYVULKANRESULT(vkAllocateCommandBuffers(device, &xferCmdBufferInfo, &xferCmdBuffer));

	// 开始录制命令
	VkCommandBufferBeginInfo cmdBufferBeginInfo;
	ZeroVulkanStruct(cmdBufferBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
	VERIFYVULKANRESULT(vkBeginCommandBuffer(xferCmdBuffer, &cmdBufferBeginInfo));

	VkBufferCopy copyRegion = {};
	copyRegion.size = m_DataSize;
	vkCmdCopyBuffer(xferCmdBuffer, hostBuffer, m_Buffer, 1, &copyRegion);

	// 结束录制
	VERIFYVULKANRESULT(vkEndCommandBuffer(xferCmdBuffer));

	// 提交命令，并且等待命令执行完毕。
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
	vkDestroyBuffer(device, hostBuffer, VULKAN_CPU_ALLOCATOR);
	vkFreeMemory(device, hostMemory, VULKAN_CPU_ALLOCATOR);

	m_Valid = true;
}

void IndexBuffer::DestroyBuffer()
{
	if (!m_Valid)
	{
		return;
	}

	VkDevice device = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();

	vkDestroyBuffer(device, m_Buffer, VULKAN_CPU_ALLOCATOR);
	vkFreeMemory(device, m_Memory, VULKAN_CPU_ALLOCATOR);

	m_Memory = VK_NULL_HANDLE;
	m_Memory = VK_NULL_HANDLE;
	
	m_Valid = false;
}