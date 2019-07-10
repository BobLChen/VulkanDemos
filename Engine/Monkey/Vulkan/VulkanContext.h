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
class VulkanGfxLayout;
class VulkanPipelineStateInfo;
class VulkanCommandBufferManager;

class VulkanCommandListContext
{
public:

	VulkanCommandListContext(VulkanDevice* inDevice, std::shared_ptr<VulkanQueue> inQueue, VulkanCommandListContext* inImmediate);

	virtual ~VulkanCommandListContext();

	inline bool IsImmediate() const
	{
		return m_Immediate == nullptr;
	}

	inline VulkanCommandBufferManager* GetCommandBufferManager() const
	{
		return m_CommandBufferManager;
	}

	inline VulkanUniformBufferUploader* GetUniformBufferUploader() const
	{
		return m_UniformUploader;
	}

	inline std::shared_ptr<VulkanQueue> GetQueue() const
	{
		return m_Queue;
	}

	inline VulkanDevice* GetDevice() const
	{
		return m_Device;
	}

public:

	VkRenderPass GetRenderPass(const class VulkanPipelineStateInfo& pipelineStateInfo, const VulkanGfxLayout* layout);

protected:

	VkRenderPass					m_RenderPass;

	VulkanCommandListContext*		m_Immediate;
	VulkanDevice*					m_Device;
	std::shared_ptr<VulkanQueue>    m_Queue;
	VulkanUniformBufferUploader*	m_UniformUploader;
	VulkanCommandBufferManager*		m_CommandBufferManager;
};

class VulkanCommandListContextImmediate : public VulkanCommandListContext
{
public:
	VulkanCommandListContextImmediate(VulkanDevice* inDevice, std::shared_ptr<VulkanQueue> inQueue);
};
