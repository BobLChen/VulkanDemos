#include "VulkanQueue.h"
#include "VulkanDevice.h"
#include "VulkanFence.h"

VulkanQueue::VulkanQueue(VulkanDevice* device, uint32 familyIndex)
    : m_Queue(VK_NULL_HANDLE)
    , m_FamilyIndex(familyIndex)
    , m_Device(device)
{
    vkGetDeviceQueue(m_Device->GetInstanceHandle(), m_FamilyIndex, 0, &m_Queue);
}

VulkanQueue::~VulkanQueue()
{
    
}