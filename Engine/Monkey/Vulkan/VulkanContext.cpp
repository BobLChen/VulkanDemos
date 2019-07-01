#include "VulkanContext.h"

#include "VulkanDevice.h"
#include "VulkanQueue.h"
#include "VulkanMemory.h"
#include "VulkanDescriptorInfo.h"
#include "VulkanRHI.h"
#include "VulkanResources.h"

// VulkanCommandListContext
VulkanCommandListContext::VulkanCommandListContext(VulkanDevice* inDevice, std::shared_ptr<VulkanQueue> inQueue, VulkanCommandListContext* inImmediate)
    : m_Immediate(inImmediate)
	, m_Device(inDevice)
	, m_Queue(inQueue)
	, m_UniformUploader(nullptr)
{
	m_UniformUploader = new VulkanUniformBufferUploader(inDevice);
}

VulkanCommandListContext::~VulkanCommandListContext()
{
	delete m_UniformUploader;
}

// VulkanCommandListContextImmediate
VulkanCommandListContextImmediate::VulkanCommandListContextImmediate(VulkanDevice* inDevice, std::shared_ptr<VulkanQueue> inQueue)
	: VulkanCommandListContext(inDevice, inQueue, nullptr)
{

}