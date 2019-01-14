#pragma once

#include "VulkanPlatform.h"
#include <memory>

class VulkanDevice;

class VulkanQueue
{
public:
    
    VulkanQueue(std::shared_ptr<VulkanDevice> device, uint32 familyIndex);
    
    virtual ~VulkanQueue();
    
    inline uint32 GetFamilyIndex() const
    {
        return m_FamilyIndex;
    }
    
    inline VkQueue GetHandle() const
    {
        return m_Queue;
    }
    
private:
    VkQueue m_Queue;
    uint32 m_FamilyIndex;
    uint32 m_QueueIndex;
    std::shared_ptr<VulkanDevice> m_Device;
};

