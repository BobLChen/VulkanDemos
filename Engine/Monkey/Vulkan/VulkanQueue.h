#pragma once

#include "VulkanPlatform.h"

#include <memory>

class VulkanDevice;
class VulkanCmdBuffer;

class VulkanQueue
{
public:
    
    VulkanQueue(VulkanDevice* device, uint32 familyIndex);
    
    virtual ~VulkanQueue();
    
    void Submit(VulkanCmdBuffer* cmdBuffer, uint32 numSignalSemaphores = 0, VkSemaphore* signalSemaphores = nullptr);
    
	inline void Submit(VulkanCmdBuffer* cmdBuffer, VkSemaphore signalSemaphore)
	{
		Submit(cmdBuffer, 1, &signalSemaphore);
	}

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

