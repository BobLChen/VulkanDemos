#include "Graphics/Shader/Shader.h"

#include "VulkanPipeline.h"
#include "VulkanDescriptorInfo.h"
#include "VulkanResources.h"
#include "VulkanDevice.h"
#include "Engine.h"

enum
{
	PackedUniformsSIngleDrawRingBufferSize = 8 * 1024 * 1024,
	PackedUniformsMultiDrawRingBufferSize  = 8 * 1024 * 1024
};

// VulkanUniformBuffer
VulkanUniformBuffer::VulkanUniformBuffer(const UniformBufferLayout& inLayout, const void* contents, UniformBufferUsage inUsage)
    : UniformBuffer(inLayout)
{
    if (inLayout.constantBufferSize > 0)
    {
        constantData.resize(inLayout.constantBufferSize);
        if (contents)
        {
            memcpy(constantData.data(), contents, inLayout.constantBufferSize);
        }
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

// uniformbuffer uploader
VulkanUniformBufferUploader::VulkanUniformBufferUploader(VulkanDevice* device)
	: m_VulkanDevice(device)
	, m_RingBuffer(nullptr)
{
	
}

VulkanUniformBufferUploader::~VulkanUniformBufferUploader()
{

}