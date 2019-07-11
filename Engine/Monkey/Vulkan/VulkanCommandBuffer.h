#pragma once

#include "Common/Common.h"
#include "VulkanPlatform.h"

#include <memory>
#include <vector>
#include <map>

class VulkanDevice;
class VulkanQueue;
class VulkanFence;
class VulkanSemaphore;
class VulkanCommandListContext;
class VulkanCommandBufferPool;
class VulkanCommandBufferManager;
class VulkanDescriptorSetsLayout;
class VulkanTypedDescriptorPoolSet;
class VulkanDescriptorPoolSetContainer;

class VulkanCmdBuffer
{
protected:
	friend class VulkanQueue;
	friend class VulkanCommandBufferPool;
	friend class VulkanCommandBufferManager;
	
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

    inline void SetState(State state)
    {
        m_State = state;
    }
    
	inline VulkanCommandBufferPool* GetOwner() const
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

	inline volatile uint64 GetFenceSignaledCounter() const
	{
		return m_FenceSignaledCounter;
	}

	inline const VulkanDevice* GetDevice() const
	{
		return m_VulkanDevice;
	}

	inline const State GetState() const
	{
		return m_State;
	}

	inline VulkanFence* GetFence() const
	{
		return m_Fence;
	}
    
    inline const std::vector<VkSemaphore>& GetWaitSemaphores() const
    {
        return m_WaitSemaphores;
    }
    
    inline const std::vector<VkPipelineStageFlags>& GetWaitFlags() const
    {
        return m_WaitFlags;
    }
    
	void AddWaitSemaphore(VkPipelineStageFlags inWaitFlags, VkSemaphore inWaitSemaphore);

	void Begin();

	void End();

	bool AcquirePoolSetAndDescriptorsIfNeeded(const class VulkanDescriptorSetsLayout& layout, VkDescriptorSet* outDescriptors);

protected:

	void RefreshFenceStatus();

	void AcquirePoolSetContainer();

	void AllocMemory();

	void FreeMemory();

	void MarkSemaphoresAsSubmitted();
    
    void RefreshSubmittedFenceCounter();
    
protected:

	typedef std::map<uint32, VulkanTypedDescriptorPoolSet*> VulkanTypedDescriptorPoolSetMap;

	State						m_State;
	bool						m_IsUploadOnly;
	
	uint64						m_FenceSignaledCounter;
	uint64						m_SubmittedFenceCounter;

	VulkanCommandBufferPool*	m_CommandBufferPool;

	VkCommandBuffer				m_Handle;
	VulkanFence*				m_Fence;
	VulkanDevice*				m_VulkanDevice;
	
	VulkanDescriptorPoolSetContainer*   m_DescriptorPoolSetContainer;

	std::vector<VkPipelineStageFlags>	m_WaitFlags;
	std::vector<VkSemaphore>			m_WaitSemaphores;
	std::vector<VkSemaphore>			m_SubmittedWaitSemaphores;

	VulkanTypedDescriptorPoolSetMap		m_TypedDescriptorPoolSets;
};

class VulkanCommandBufferPool
{
public:
	VulkanCommandBufferPool(VulkanDevice* inDevice, VulkanCommandBufferManager& inManager);

	~VulkanCommandBufferPool();

	void RefreshFenceStatus(VulkanCmdBuffer* skipCmdBuffer = nullptr);

	void FreeUnusedCmdBuffers(std::shared_ptr<VulkanQueue> queue);

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
