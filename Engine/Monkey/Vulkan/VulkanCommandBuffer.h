#pragma once

#include "Common/Common.h"
#include "VulkanPlatform.h"

#include <memory>
#include <vector>

class VulkanCommandListContext;
class VulkanDevice;
class VulkanQueue;
class VulkanDescriptorPoolSetContainer;
class VulkanCommandBufferPool;
class VulkanCommandBufferManager;
class VulkanSemaphore;

class VulkanCmdBuffer
{
protected:
	friend class VulkanCommandBufferManager;
	friend class VulkanCommandBufferPool;
	friend class VulkanQueue;

	VulkanCmdBuffer(VulkanDevice* inDevice, VulkanCommandBufferPool* inCommandBufferPool, bool inIsUploadOnly);

	~VulkanCmdBuffer();

public:

	enum class State : uint8
	{
		ReadyForBegin,
		IsInsideBegin,
		IsInsideRenderPass,
		HasEnded,
		Submitted,
		NotAllocated,
	};

public:

	VulkanCommandBufferPool* GetOwner()
	{
		return m_CommandBufferPool;
	}

	inline bool IsInsideRenderPass() const
	{
		return m_State == State::IsInsideRenderPass;
	}

	inline bool IsOutsideRenderPass() const
	{
		return m_State == State::IsInsideBegin;
	}

	inline bool HasBegun() const
	{
		return m_State == State::IsInsideBegin || m_State == State::IsInsideRenderPass;
	}

	inline bool HasEnded() const
	{
		return m_State == State::HasEnded;
	}

	inline bool IsSubmitted() const
	{
		return m_State == State::Submitted;
	}

	inline bool IsAllocated() const
	{
		return m_State != State::NotAllocated;
	}

	inline const VkCommandBuffer GetHandle()
	{
		return m_Handle;
	}

	inline const VulkanDevice* GetDevice() const
	{
		return m_VulkanDevice;
	}

	inline const VkViewport GetViewport() const
	{
		return m_Viewport;
	}

	inline const VkRect2D GetScissor() const
	{
		return m_Scissor;
	}

	inline const uint32 GetStencilRef() const
	{
		return m_StencilRef;
	}

	inline const State GetState() const
	{
		return m_State;
	}

protected:

	VkViewport					m_Viewport;
	VkRect2D					m_Scissor;
	uint32						m_StencilRef;
	State						m_State;

	VulkanCommandBufferPool*	m_CommandBufferPool;

	VulkanDevice*				m_VulkanDevice;
	VkCommandBuffer				m_Handle;
};

class VulkanCommandBufferPool
{
public:
	VulkanCommandBufferPool(VulkanDevice* inDevice, VulkanCommandBufferManager& inManager);

	~VulkanCommandBufferPool();

	void FreeUnusedCmdBuffers(VulkanQueue* queue);

	inline VkCommandPool GetHandle() const
	{
		return m_Handle;
	}

	inline VulkanCommandBufferManager& GetManager()
	{
		return m_CmdBufferManager;
	}

private:
	friend class VulkanCommandBufferManager;

	VulkanCmdBuffer* Create(bool isUploadOnly);

	void Create(uint32 queueFamilyIndex);

private:
	VkCommandPool					m_Handle;
	VulkanDevice*					m_Device;

	std::vector<VulkanCmdBuffer*>	m_UsedCmdBuffers;
	std::vector<VulkanCmdBuffer*>	m_FreeCmdBuffers;

	VulkanCommandBufferManager&		m_CmdBufferManager;
};

class VulkanCommandBufferManager
{
public:
	VulkanCommandBufferManager(VulkanDevice* inDevice, VulkanCommandListContext* inContext);

	~VulkanCommandBufferManager();

	inline VulkanCmdBuffer* GetActiveCmdBuffer()
	{
		if (m_UploadCmdBuffer) {
			SubmitUploadCmdBuffer();
		}
		return m_ActiveCmdBuffer;
	}

	inline bool HasPendingUploadCmdBuffer() const
	{
		return m_UploadCmdBuffer != nullptr;
	}

	inline bool HasPendingActiveCmdBuffer() const
	{
		return m_ActiveCmdBuffer != nullptr;
	}

	inline VkCommandPool GetHandle() const
	{
		return m_CmdBufferPool->GetHandle();
	}

	VulkanCmdBuffer* GetUploadCmdBuffer();

	void SubmitUploadCmdBuffer(uint32 numSignalSemaphores = 0, VkSemaphore* signalSemaphores = nullptr);

	void SubmitActiveCmdBuffer(VulkanSemaphore* signalSemaphore = nullptr);

	void WaitForCmdBuffer(VulkanCmdBuffer* cmdBuffer, float timeInSecondsToWait = 1.0f);

	void NewActiveCommandBuffer();

	void FreeUnusedCmdBuffers();

private:

	VulkanDevice*					m_Device;
	VulkanCommandBufferPool*		m_CmdBufferPool;
	std::shared_ptr<VulkanQueue>	m_Queue;
	VulkanCmdBuffer*				m_ActiveCmdBuffer;
	VulkanCmdBuffer*				m_UploadCmdBuffer;
};