#include "Common/Log.h"
#include "VulkanDevice.h"
#include "VulkanMemory.h"

// VulkanFence

VulkanFence::VulkanFence(VulkanDevice* device, VulkanFenceManager* owner, bool createSignaled)
	: m_State(createSignaled ? VulkanFence::State::Signaled : VulkanFence::State::NotReady)
	, m_Owner(owner)
{
	VkFenceCreateInfo info;
	ZeroVulkanStruct(info, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
	info.flags = createSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
	vkCreateFence(device->GetInstanceHandle(), &info, VULKAN_CPU_ALLOCATOR, &m_VkFence);
}

VulkanFence::~VulkanFence()
{
	if (m_VkFence == VK_NULL_HANDLE)
	{
		MLOG("Didn't get properly destroyed by FFenceManager!");
	}
}

// VulkanFenceManager

VulkanFenceManager::VulkanFenceManager()
	: m_Device(nullptr)
{

}

VulkanFenceManager::~VulkanFenceManager()
{
	if (m_UsedFences.size() > 0)
	{
		MLOG("No all fences are done!");
	}
}

void VulkanFenceManager::Init(VulkanDevice* device)
{
	m_Device = device;
}

void VulkanFenceManager::Destory()
{
	if (m_UsedFences.size() > 0)
	{
		MLOG("No all fences are done!");
	}

	VkDevice deviceHandle = m_Device->GetInstanceHandle();
	for (int i = 0; i < m_FreeFences.size(); ++i)
	{
		DestoryFence(m_FreeFences[i]);
	}
}

VulkanFence* VulkanFenceManager::CreateFence(bool createSignaled)
{
	if (m_FreeFences.size() > 0)
	{
		VulkanFence* fence = m_FreeFences.back();
		m_FreeFences.pop_back();
		m_UsedFences.push_back(fence);
		if (createSignaled)
		{
			fence->m_State = VulkanFence::State::Signaled;
		}
		return fence;
	}

	VulkanFence* newFence = new VulkanFence(m_Device, this, createSignaled);
	m_UsedFences.push_back(newFence);
	return newFence;
}

bool VulkanFenceManager::WaitForFence(VulkanFence* fence, uint64 timeInNanoseconds)
{
	VkResult result = vkWaitForFences(m_Device->GetInstanceHandle(), 1, &fence->m_VkFence, true, timeInNanoseconds);
	switch (result)
	{
	case VK_SUCCESS:
		fence->m_State = VulkanFence::State::Signaled;
		return true;
	case VK_TIMEOUT:
		break;
	default:
		MLOG("Unkow error %d", (int)result);
		break;
	}
}

void VulkanFenceManager::ResetFence(VulkanFence* fence)
{
	if (fence->m_State != VulkanFence::State::NotReady)
	{
		vkResetFences(m_Device->GetInstanceHandle(), 1, &fence->m_VkFence);
		fence->m_State = VulkanFence::State::NotReady;
	}
}

void VulkanFenceManager::ReleaseFence(VulkanFence*& fence)
{
	ResetFence(fence);
	for (int i = 0; i < m_UsedFences.size(); ++i) {
		if (m_UsedFences[i] == fence)
		{
			m_UsedFences.erase(m_UsedFences.begin() + i);
			break;
		}
	}
	m_FreeFences.push_back(fence);
	fence = nullptr;
}

void VulkanFenceManager::WaitAndReleaseFence(VulkanFence*& fence, uint64 timeInNanoseconds)
{
	if (!fence->IsSignaled()) {
		WaitForFence(fence, timeInNanoseconds);
	}
	ReleaseFence(fence);
}

bool VulkanFenceManager::CheckFenceState(VulkanFence* fence)
{
	VkResult result = vkGetFenceStatus(m_Device->GetInstanceHandle(), fence->m_VkFence);
	switch (result)
	{
	case VK_SUCCESS:
		fence->m_State = VulkanFence::State::Signaled;
		break;
	case VK_NOT_READY:
		break;
	default:
		break;
	}
	return false;
}

void VulkanFenceManager::DestoryFence(VulkanFence* fence)
{
	vkDestroyFence(m_Device->GetInstanceHandle(), fence->m_VkFence, VULKAN_CPU_ALLOCATOR);
	fence->m_VkFence = VK_NULL_HANDLE;
	delete fence;
}

// VulkanSemaphore
VulkanSemaphore::VulkanSemaphore(VulkanDevice* device)
	: m_Device(device)
	, m_VkSemaphore(VK_NULL_HANDLE)
{
	VkSemaphoreCreateInfo info;
	ZeroVulkanStruct(info, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
	vkCreateSemaphore(device->GetInstanceHandle(), &info, VULKAN_CPU_ALLOCATOR, &m_VkSemaphore);
}

VulkanSemaphore::~VulkanSemaphore()
{
	if (m_VkSemaphore == VK_NULL_HANDLE)
	{
		MLOG("Failed destory VkSemaphore.");
	}
	vkDestroySemaphore(m_Device->GetInstanceHandle(), m_VkSemaphore, VULKAN_CPU_ALLOCATOR);
}

// VulkanDeviceMemoryAllocation
VulkanDeviceMemoryAllocation::VulkanDeviceMemoryAllocation()
	: m_Size(0)
	, m_Device(VK_NULL_HANDLE)
	, m_Handle(VK_NULL_HANDLE)
	, m_MappedPointer(nullptr)
	, m_MemoryTypeIndex(0)
	, m_CanBeMapped(false)
	, m_IsCoherent(false)
	, m_IsCached(false)
	, m_FreedBySystem(false)
{

}

VulkanDeviceMemoryAllocation::~VulkanDeviceMemoryAllocation()
{

}

void* VulkanDeviceMemoryAllocation::Map(VkDeviceSize size, VkDeviceSize offset)
{
	vkMapMemory(m_Device, m_Handle, offset, size, 0, &m_MappedPointer);
	return m_MappedPointer;
}

void VulkanDeviceMemoryAllocation::Unmap()
{
	vkUnmapMemory(m_Device, m_Handle);
}

void VulkanDeviceMemoryAllocation::FlushMappedMemory(VkDeviceSize offset, VkDeviceSize size)
{
	if (!IsCoherent())
	{
		VkMappedMemoryRange range;
		ZeroVulkanStruct(range, VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE);
		range.memory = m_Handle;
		range.offset = offset;
		range.size   = size;
		vkFlushMappedMemoryRanges(m_Device, 1, &range);
	}
}

void VulkanDeviceMemoryAllocation::InvalidateMappedMemory(VkDeviceSize offset, VkDeviceSize size)
{
	if (!IsCoherent())
	{
		VkMappedMemoryRange range;
		ZeroVulkanStruct(range, VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE);
		range.memory = m_Handle;
		range.offset = offset;
		range.size   = size;
		vkInvalidateMappedMemoryRanges(m_Device, 1, &range);
	}
}