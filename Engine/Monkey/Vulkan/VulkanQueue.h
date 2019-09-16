#pragma once

#include "VulkanPlatform.h"

#include <memory>

class VulkanDevice;

class VulkanQueue
{
public:
    
    VulkanQueue(VulkanDevice* device, uint32 familyIndex);
    
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
    VkQueue         m_Queue;
    uint32          m_FamilyIndex;
	VulkanDevice*   m_Device;
};

