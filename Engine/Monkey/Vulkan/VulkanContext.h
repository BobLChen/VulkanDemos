#pragma once

#include "Common/Common.h"
#include "HAL/ThreadSafeCounter.h"
#include "VulkanPlatform.h"
#include "VulkanResources.h"

#include <memory>
#include <vector>

class VulkanRHI;
class VulkanDevice;
class VulkanQueue;

class VulkanCommandListContext
{
public:

	VulkanCommandListContext(VulkanRHI* vulkanRHI, VulkanDevice* inDevice, VulkanQueue* inQueue, VulkanCommandListContext* inImmediate);

	virtual ~VulkanCommandListContext();

	inline bool IsImmediate() const
	{
		return m_Immediate == nullptr;
	}

	inline VulkanUniformBufferUploader* GetUniformBufferUploader() const
	{
		return m_UniformUploader;
	}

	inline VulkanQueue* GetQueue() const
	{
		return m_Queue;
	}

	inline VulkanDevice* GetDevice() const
	{
		return m_Device;
	}

protected:

	VulkanRHI*						m_VulkanRHI;
	VulkanCommandListContext*		m_Immediate;
	VulkanDevice*					m_Device;
	VulkanQueue*					m_Queue;
	VulkanUniformBufferUploader*	m_UniformUploader;
};

class VulkanCommandListContextImmediate : public VulkanCommandListContext
{
public:
	VulkanCommandListContextImmediate(VulkanRHI* inVulkanRHI, VulkanDevice* inDevice, VulkanQueue* inQueue);
};