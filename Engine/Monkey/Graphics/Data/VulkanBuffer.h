#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

#include "Utils/Crc.h"

#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/RHIDefinitions.h"
#include "Vulkan/VulkanDevice.h"

#include <vector>

class VulkanBuffer
{
public:
    
    VulkanBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size);
    
    virtual ~VulkanBuffer();
    
    static VulkanBuffer* CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void* data = nullptr);
    
    VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    
    void Unmap();
    
    VkResult Bind(VkDeviceSize offset = 0);
    
    void SetupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    
    void CopyTo(void* data, VkDeviceSize size);
    
    VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    
    VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    
    void Destroy();
    
    inline const VkBuffer& GetBuffer() const
    {
        return m_Buffer;
    }
    
    inline const VkDeviceMemory& GetMemory() const
    {
        return m_Memory;
    }
    
    inline const VkDescriptorBufferInfo& GetDescriptorBufferInfo() const
    {
        return m_DescriptorBufferInfo;
    }
    
    inline const VkBufferUsageFlags& GetBufferUsage() const
    {
        return m_UsageFlags;
    }
    
    inline const VkMemoryPropertyFlags& GetMemoryProperty() const
    {
        return m_MemoryPropertyFlags;
    }
    
    inline const VkDeviceSize& GetSize() const
    {
        return m_Size;
    }
    
    inline const VkDeviceSize& GetAlignment() const
    {
        return m_Alignment;
    }
    
    inline void* GetMappedPtr() const
    {
        return m_MapDataPtr;
    }
    
private:
    
    VkBuffer				m_Buffer;
    VkDeviceMemory			m_Memory;
    VkDescriptorBufferInfo	m_DescriptorBufferInfo;
    
    VkBufferUsageFlags		m_UsageFlags;
    VkMemoryPropertyFlags	m_MemoryPropertyFlags;
    
    VkDeviceSize			m_Size;
    VkDeviceSize			m_Alignment;
    
    void*					m_MapDataPtr;
};
