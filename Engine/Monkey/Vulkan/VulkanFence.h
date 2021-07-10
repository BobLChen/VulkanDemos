#pragma once

#include "Common/Common.h"
#include "HAL/ThreadSafeCounter.h"

#include "VulkanPlatform.h"

#include <memory>
#include <vector>

class VulkanDevice;
class VulkanFenceManager;

class VulkanFence
{
public:
	enum class State
	{
		NotReady,
		Signaled,
	};

	VulkanFence(VulkanDevice* device, VulkanFenceManager* owner, bool createSignaled);

	FORCE_INLINE VkFence GetHandle() const
	{
		return m_VkFence;
	}

	FORCE_INLINE bool IsSignaled() const
	{
		return m_State == State::Signaled;
	}

	VulkanFenceManager* GetOwner()
	{
		return m_Owner;
	}

protected:
	virtual ~VulkanFence();
	friend class VulkanFenceManager;

	VkFence             m_VkFence;
	State               m_State;
	VulkanFenceManager* m_Owner;
};

class VulkanFenceManager
{
public:
	VulkanFenceManager();
	
	virtual ~VulkanFenceManager();

	void Init(VulkanDevice* device);

	void Destory();

	VulkanFence* CreateFence(bool createSignaled = false);

	bool WaitForFence(VulkanFence* fence, uint64 timeInNanoseconds);

	void ResetFence(VulkanFence* fence);

	void ReleaseFence(VulkanFence*& fence);

	void WaitAndReleaseFence(VulkanFence*& fence, uint64 timeInNanoseconds);

	FORCE_INLINE bool IsFenceSignaled(VulkanFence* fence)
	{
		if (fence->IsSignaled()) {
			return true;
		}
		return CheckFenceState(fence);
	}

protected:
	bool CheckFenceState(VulkanFence* fence);

	void DestoryFence(VulkanFence* fence);

protected:
	VulkanDevice*             m_Device;
	std::vector<VulkanFence*> m_FreeFences;
	std::vector<VulkanFence*> m_UsedFences;
};

class VulkanSemaphore
{
public:
	VulkanSemaphore(VulkanDevice* device);

	virtual ~VulkanSemaphore();

	FORCE_INLINE VkSemaphore GetHandle() const
	{
		return m_VkSemaphore;
	}

protected:
	VkSemaphore     m_VkSemaphore;
	VulkanDevice*   m_Device;
};