#include "Math/Math.h"
#include "Utils/Crc.h"

#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanDevice.h"

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
	, m_Invalid(true)
	, m_AllocationSize(0)
	, m_Alignment(0)
	, m_Hash(0)
{
	
}

IndexBuffer::~IndexBuffer()
{
	if (m_Data) {
		delete[] m_Data;
		m_Data = nullptr;
	}
    DestroyBuffer();
}

void IndexBuffer::CreateBuffer()
{
	if (!m_Invalid) {
		return;
	}

    m_Invalid       = false;
	m_Hash          = Crc::MemCrc32(m_Data, m_DataSize, 0);
	m_IndexCount    = m_DataSize / IndexTypeToSize(m_IndexType);
	m_TriangleCount = m_IndexCount / PrimitiveTypeToSize(m_PrimitiveType);

	VkBuffer hostBuffer                         = VK_NULL_HANDLE;
	VkDeviceMemory hostMemory                   = VK_NULL_HANDLE;
	std::shared_ptr<VulkanRHI> vulkanRHI        = Engine::Get()->GetVulkanRHI();
	std::shared_ptr<VulkanDevice> vulkanDevice  = Engine::Get()->GetVulkanRHI()->GetDevice();
	VkDevice device                             = vulkanDevice->GetInstanceHandle();
    
	// Host端创建Buffer
	VkBufferCreateInfo bufferCreateInfo;
	ZeroVulkanStruct(bufferCreateInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
	bufferCreateInfo.size  = m_DataSize;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VERIFYVULKANRESULT(vkCreateBuffer(device, &bufferCreateInfo, VULKAN_CPU_ALLOCATOR, &hostBuffer));

	// 获取需要分配的内心信息
	VkMemoryRequirements memReqInfo;
	vkGetBufferMemoryRequirements(device, hostBuffer, &memReqInfo);
	uint32 memoryTypeIndex = 0;
	vulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);
	
	// 填充AllocateInfo
	VkMemoryAllocateInfo memAllocInfo;
	ZeroVulkanStruct(memAllocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
	memAllocInfo.allocationSize  = memReqInfo.size;
	memAllocInfo.memoryTypeIndex = memoryTypeIndex;

	// 分配内存并且与Buffer绑定
	VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &hostMemory));
	VERIFYVULKANRESULT(vkBindBufferMemory(device, hostBuffer, hostMemory, 0));

	// 映射内存，拷贝数据到内存
	void* dataPtr = nullptr;
	VERIFYVULKANRESULT(vkMapMemory(device, hostMemory, 0, memAllocInfo.allocationSize, 0, &dataPtr));
	std::memcpy(dataPtr, m_Data, bufferCreateInfo.size);
	vkUnmapMemory(device, hostMemory);

	// 再次在LocalDevice端创建一个Buffer
	bufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	VERIFYVULKANRESULT(vkCreateBuffer(device, &bufferCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Buffer));

	// 获取需要分配的内存信息
	vkGetBufferMemoryRequirements(device, m_Buffer, &memReqInfo);
	vulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
	memAllocInfo.allocationSize  = memReqInfo.size;
	memAllocInfo.memoryTypeIndex = memoryTypeIndex;

	// 记录本次的Alignment以及AllocationSize
	m_Alignment      = (uint32)memReqInfo.alignment;
	m_AllocationSize = (uint32)memReqInfo.size;
    
	// 分配内存并且与Buffer绑定
	VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &m_Memory));
	VERIFYVULKANRESULT(vkBindBufferMemory(device, m_Buffer, m_Memory, 0));

	// TODO:延迟的专用队列拷贝
	// 创建Command，将Host数据拷贝到Local端
	VkCommandBuffer xferCmdBuffer;
	VkCommandBufferAllocateInfo xferCmdBufferInfo;
	ZeroVulkanStruct(xferCmdBufferInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
	xferCmdBufferInfo.commandPool        = vulkanRHI->GetCommandPool();
	xferCmdBufferInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	xferCmdBufferInfo.commandBufferCount = 1;
	VERIFYVULKANRESULT(vkAllocateCommandBuffers(device, &xferCmdBufferInfo, &xferCmdBuffer));
    
	// 开始录制命令
	VkCommandBufferBeginInfo cmdBufferBeginInfo;
	ZeroVulkanStruct(cmdBufferBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
	VERIFYVULKANRESULT(vkBeginCommandBuffer(xferCmdBuffer, &cmdBufferBeginInfo));

	// 将Host Buffer拷贝至LocalDevice Buffer
	VkBufferCopy copyRegion = {};
	copyRegion.size = m_AllocationSize;
	vkCmdCopyBuffer(xferCmdBuffer, hostBuffer, m_Buffer, 1, &copyRegion);

	// 结束录制
	VERIFYVULKANRESULT(vkEndCommandBuffer(xferCmdBuffer));

	// 准备CommitInfo
	VkSubmitInfo submitInfo;
	ZeroVulkanStruct(submitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers    = &xferCmdBuffer;
    
	// 准备同步对象
	VkFenceCreateInfo fenceInfo;
	ZeroVulkanStruct(fenceInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
	fenceInfo.flags = 0;
    
	// 创建Fence同步对象
	VkFence fence;
	VERIFYVULKANRESULT(vkCreateFence(device, &fenceInfo, VULKAN_CPU_ALLOCATOR, &fence));

	// 提交Command命令
	VERIFYVULKANRESULT(vkQueueSubmit(vulkanRHI->GetDevice()->GetGraphicsQueue()->GetHandle(), 1, &submitInfo, fence));
	// 等待执行完毕
	VERIFYVULKANRESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, MAX_int64));

	// 销毁Host端数据、Command、同步对象
	vkDestroyFence(device, fence, VULKAN_CPU_ALLOCATOR);
	vkFreeCommandBuffers(device, vulkanRHI->GetCommandPool(), 1, &xferCmdBuffer);
	vkDestroyBuffer(device, hostBuffer, VULKAN_CPU_ALLOCATOR);
	vkFreeMemory(device, hostMemory, VULKAN_CPU_ALLOCATOR);
}

void IndexBuffer::DestroyBuffer()
{
    if (m_Invalid) {
        return;
    }
    
	VkDevice device = Engine::Get()->GetDeviceHandle();

	vkDestroyBuffer(device, m_Buffer, VULKAN_CPU_ALLOCATOR);
	vkFreeMemory(device, m_Memory, VULKAN_CPU_ALLOCATOR);

	m_Memory  = VK_NULL_HANDLE;
	m_Memory  = VK_NULL_HANDLE;
	m_Invalid = true;
}
