#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/RHIDefinitions.h"
#include "Vulkan/VulkanDevice.h"
#include "Utils/Crc.h"
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
    
    FORCEINLINE const VkBuffer& GetBuffer() const
    {
        return m_Buffer;
    }
    
    FORCEINLINE const VkDeviceMemory& GetMemory() const
    {
        return m_Memory;
    }
    
    FORCEINLINE const VkDescriptorBufferInfo& GetDescriptorBufferInfo() const
    {
        return m_DescriptorBufferInfo;
    }
    
    FORCEINLINE const VkBufferUsageFlags& GetBufferUsage() const
    {
        return m_UsageFlags;
    }
    
    FORCEINLINE const VkMemoryPropertyFlags& GetMemoryProperty() const
    {
        return m_MemoryPropertyFlags;
    }
    
    FORCEINLINE const VkDeviceSize& GetSize() const
    {
        return m_Size;
    }
    
    FORCEINLINE const VkDeviceSize& GetAlignment() const
    {
        return m_Alignment;
    }
    
    FORCEINLINE void* GetMappedPtr() const
    {
        return m_MapDataPtr;
    }
    
private:
    
    VkBuffer m_Buffer;
    VkDeviceMemory m_Memory;
    VkDescriptorBufferInfo m_DescriptorBufferInfo;
    
    VkBufferUsageFlags m_UsageFlags;
    VkMemoryPropertyFlags m_MemoryPropertyFlags;
    
    VkDeviceSize m_Size;
    VkDeviceSize m_Alignment;
    
    void* m_MapDataPtr;
};
