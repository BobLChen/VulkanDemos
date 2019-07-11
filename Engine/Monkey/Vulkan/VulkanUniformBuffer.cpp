#include "Graphics/Shader/Shader.h"

#include "VulkanPipeline.h"
#include "VulkanDescriptorInfo.h"
#include "VulkanResources.h"
#include "VulkanDevice.h"
#include "Engine.h"

enum
{
	PackedUniformsSIngleDrawRingBufferSize = 32 * 1024 * 1024,
};

// VulkanUniformBuffer
VulkanUniformBuffer::VulkanUniformBuffer(uint32 contentSize)
{
    if (contentSize > 0) {
        constantData.resize(contentSize);
    }
}

void VulkanUniformBuffer::UpdateConstantData(const void* contents, int32 contentsSize)
{
    memcpy(constantData.data(), contents, contentsSize);
}

// VulkanRingBuffer
VulkanRingBuffer::VulkanRingBuffer(VulkanDevice* device, uint64 totalSize, VkFlags usage, VkMemoryPropertyFlags memPropertyFlags)
	: m_VulkanDevice(device)
	, m_BufferSize(totalSize)
	, m_BufferOffset(0)
	, m_MinAlignment(0)
{
	m_BufferSubAllocation = device->GetResourceHeapManager().AllocateBuffer(totalSize, usage, memPropertyFlags, __FILE__, __LINE__);
	m_MinAlignment        = m_BufferSubAllocation->GetBufferAllocator()->GetAlignment();
	m_BufferOffset        = totalSize;
}

VulkanRingBuffer::~VulkanRingBuffer()
{
	delete m_BufferSubAllocation;
}

uint64 VulkanRingBuffer::WrapAroundAllocateMemory(uint64 size, uint32 alignment)
{
	m_BufferOffset = size;
	return 0;
}

VulkanSubBufferAllocator* VulkanRingBuffer::GetBufferAllocator() const
{
	return m_BufferSubAllocation->GetBufferAllocator();
}

uint32 VulkanRingBuffer::GetBufferOffset() const
{
	return m_BufferSubAllocation->GetOffset();
}

VkBuffer VulkanRingBuffer::GetHandle() const
{
	return m_BufferSubAllocation->GetHandle();
}

void* VulkanRingBuffer::GetMappedPointer()
{
	return m_BufferSubAllocation->GetMappedPointer();
}

// uniformbuffer uploader
VulkanUniformBufferUploader::VulkanUniformBufferUploader(VulkanDevice* device)
	: m_VulkanDevice(device)
{
	m_RingBuffer = new VulkanRingBuffer(device, PackedUniformsSIngleDrawRingBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

VulkanUniformBufferUploader::~VulkanUniformBufferUploader()
{
	delete m_RingBuffer;
}