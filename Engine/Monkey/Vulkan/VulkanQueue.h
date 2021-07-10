#pragma once

#include "VulkanPlatform.h"

#include <memory>

class VulkanDevice;

class VulkanQueue
{
public:
    
    VulkanQueue(VulkanDevice* device, uint32 familyIndex);
    
    virtual ~VulkanQueue();
    
    FORCE_INLINE uint32 GetFamilyIndex() const
    {
        return m_FamilyIndex;
    }
    
	FORCE_INLINE VkQueue GetHandle() const
    {
        return m_Queue;
    }
    
private:
    VkQueue         m_Queue;
    uint32          m_FamilyIndex;
	VulkanDevice*   m_Device;
};

