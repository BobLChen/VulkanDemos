#include "VulkanBuffer.h"
#include "Math/Math.h"
#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanDevice.h"
#include "Engine.h"

VulkanBuffer::VulkanBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size)
    : m_Buffer(VK_NULL_HANDLE)
    , m_Memory(VK_NULL_HANDLE)
    , m_UsageFlags(usageFlags)
    , m_MemoryPropertyFlags(memoryPropertyFlags)
    , m_Size(size)
    , m_Alignment(256)
    , m_MapDataPtr(nullptr)
{
    
}

VulkanBuffer::~VulkanBuffer()
{
    if (m_Buffer != VK_NULL_HANDLE)
    {
        MLOGE("uniform buffer not destory.");
    }
}

VulkanBuffer* VulkanBuffer::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void* data)
{
    VkDevice device = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    VulkanBuffer* buffer = new VulkanBuffer(usageFlags, memoryPropertyFlags, size);
    
    VkBufferCreateInfo bufferCreateInfo;
    ZeroVulkanStruct(bufferCreateInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
    bufferCreateInfo.usage = usageFlags;
    bufferCreateInfo.size  = size;
    VERIFYVULKANRESULT(vkCreateBuffer(device, &bufferCreateInfo, VULKAN_CPU_ALLOCATOR, &(buffer->m_Buffer)));
    
    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAlloc;
    ZeroVulkanStruct(memAlloc, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
    vkGetBufferMemoryRequirements(device, buffer->m_Buffer, &memReqs);
    memAlloc.allocationSize  = memReqs.size;
    Engine::Get()->GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, memoryPropertyFlags, &memAlloc.memoryTypeIndex);
    VERIFYVULKANRESULT(vkAllocateMemory(device, &memAlloc, VULKAN_CPU_ALLOCATOR, &(buffer->m_Memory)));
    
    buffer->Bind();
    buffer->m_Alignment = memReqs.alignment;
    buffer->m_Size      = memReqs.size;
    buffer->SetupDescriptor();
    
    if (data != nullptr)
    {
        VERIFYVULKANRESULT(buffer->Map());
        memcpy(buffer->m_MapDataPtr, data, size);
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
        {
            buffer->Flush();
        }
        buffer->Unmap();
    }
    
    return buffer;
}

VkResult VulkanBuffer::Map(VkDeviceSize size, VkDeviceSize offset)
{
    VkDevice handle = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    return vkMapMemory(handle, m_Memory, offset, size, 0, &m_MapDataPtr);
}

void VulkanBuffer::Unmap()
{
    VkDevice handle = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    if (m_MapDataPtr)
    {
        vkUnmapMemory(handle, m_Memory);
        m_MapDataPtr = nullptr;
    }
}

VkResult VulkanBuffer::Bind(VkDeviceSize offset)
{
    VkDevice handle = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    return vkBindBufferMemory(handle, m_Buffer, m_Memory, offset);
}

void VulkanBuffer::SetupDescriptor(VkDeviceSize size, VkDeviceSize offset)
{
    m_DescriptorBufferInfo.offset = offset;
    m_DescriptorBufferInfo.buffer = m_Buffer;
    m_DescriptorBufferInfo.range  = size;
}

void VulkanBuffer::CopyTo(void* data, VkDeviceSize size)
{
    memcpy(m_MapDataPtr, data, size);
}

VkResult VulkanBuffer::Flush(VkDeviceSize size, VkDeviceSize offset)
{
    VkDevice handle = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    
    VkMappedMemoryRange mappedRange;
    ZeroVulkanStruct(mappedRange, VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE);
    mappedRange.memory = m_Memory;
    mappedRange.offset = offset;
    mappedRange.size   = size;
    return vkFlushMappedMemoryRanges(handle, 1, &mappedRange);
}

VkResult VulkanBuffer::Invalidate(VkDeviceSize size, VkDeviceSize offset)
{
    VkDevice handle = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    
    VkMappedMemoryRange mappedRange;
    ZeroVulkanStruct(mappedRange, VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE);
    mappedRange.memory = m_Memory;
    mappedRange.offset = offset;
    mappedRange.size   = size;
    return vkInvalidateMappedMemoryRanges(handle, 1, &mappedRange);
}

void VulkanBuffer::Destroy()
{
    VkDevice handle = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    
    if (m_Buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(handle, m_Buffer, VULKAN_CPU_ALLOCATOR);
        m_Buffer = VK_NULL_HANDLE;
    }
    
    if (m_Memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(handle, m_Memory, VULKAN_CPU_ALLOCATOR);
        m_Memory = VK_NULL_HANDLE;
    }
    
}

