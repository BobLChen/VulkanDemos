#include "Common/Log.h"
#include "Math/Math.h"
#include "VulkanDevice.h"
#include "VulkanMemory.h"
#include <algorithm>

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
	// VkDevice deviceHandle = m_Device->GetInstanceHandle();
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
        return false;
	default:
		MLOG("Unkow error %d", (int)result);
        return false;
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
    : m_VkSemaphore(VK_NULL_HANDLE)
	, m_Device(device)
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

// VulkanDeviceMemoryManager
VulkanDeviceMemoryManager::VulkanDeviceMemoryManager()
    : m_Device(nullptr)
    , m_DeviceHandle(VK_NULL_HANDLE)
    , m_HasUnifiedMemory(false)
    , m_NumAllocations(0)
    , m_PeakNumAllocations(0)
{
    memset(&m_MemoryProperties, 0, sizeof(VkPhysicalDeviceMemoryProperties));
}

VulkanDeviceMemoryManager::~VulkanDeviceMemoryManager()
{
    Destory();
}

void VulkanDeviceMemoryManager::Init(VulkanDevice* device)
{
    m_Device = device;
    m_NumAllocations = 0;
    m_PeakNumAllocations = 0;
    m_DeviceHandle = m_Device->GetInstanceHandle();
    vkGetPhysicalDeviceMemoryProperties(m_Device->GetPhysicalHandle(), &m_MemoryProperties);
    m_HeapInfos.resize(m_MemoryProperties.memoryHeapCount);
    SetupAndPrintMemInfo();
}

void VulkanDeviceMemoryManager::Destory()
{
    for (int32 index = 0; index < m_HeapInfos.size(); ++index)
    {
        if (m_HeapInfos[index].allocations.size() > 0)
        {
            MLOG("Found %lu unfreed allocations!", m_HeapInfos[index].allocations.size());
#if MONKEY_DEBUG
            DumpMemory();
#endif
        }
    }
    m_NumAllocations = 0;
}

bool VulkanDeviceMemoryManager::SupportsMemoryType(VkMemoryPropertyFlags properties) const
{
    for (int32 index = 0; index < m_MemoryProperties.memoryTypeCount; ++index)
    {
        if (m_MemoryProperties.memoryTypes[index].propertyFlags == properties)
        {
            return true;
        }
    }
    return false;
}

VulkanDeviceMemoryAllocation* VulkanDeviceMemoryManager::Alloc(bool canFail, VkDeviceSize allocationSize, uint32 memoryTypeIndex, void* dedicatedAllocateInfo, const char* file, uint32 line)
{
    VkMemoryAllocateInfo info;
    ZeroVulkanStruct(info, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
    info.allocationSize = allocationSize;
    info.memoryTypeIndex = memoryTypeIndex;
    info.pNext = dedicatedAllocateInfo;
    
    VulkanDeviceMemoryAllocation* newAllocation = new VulkanDeviceMemoryAllocation();
    newAllocation->m_Device = m_DeviceHandle;
    newAllocation->m_Size = allocationSize;
    newAllocation->m_MemoryTypeIndex = memoryTypeIndex;
    newAllocation->m_CanBeMapped = ((m_MemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    newAllocation->m_IsCoherent = ((m_MemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    newAllocation->m_IsCached = ((m_MemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) == VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
    
    VkResult result = vkAllocateMemory(m_DeviceHandle, &info, VULKAN_CPU_ALLOCATOR, &newAllocation->m_Handle);
    
    if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
    {
        if (canFail)
        {
            MLOG("Failed to allocate Device Memory, Requested=%fKb MemTypeIndex=%d", (float)info.allocationSize / 1024.0f, info.memoryTypeIndex);
            return nullptr;
        }
        MLOG("Out of Device Memory, Requested=%fKb MemTypeIndex=%d", (float)info.allocationSize / 1024.0f, info.memoryTypeIndex);
#if MONKEY_DEBUG
        DumpMemory();
#endif
    }
    else if (result == VK_ERROR_OUT_OF_HOST_MEMORY)
    {
        if (canFail)
        {
            MLOG("Failed to allocate Host Memory, Requested=%fKb MemTypeIndex=%d", (float)info.allocationSize / 1024.0f, info.memoryTypeIndex);
            return nullptr;
        }
        MLOG("Out of Host Memory, Requested=%fKb MemTypeIndex=%d", (float)info.allocationSize / 1024.0f, info.memoryTypeIndex);
#if MONKEY_DEBUG
        DumpMemory();
#endif
    }
    else
    {
        VERIFYVULKANRESULT(result);
    }
    
    ++m_NumAllocations;
    m_PeakNumAllocations = MMath::Max(m_NumAllocations, m_PeakNumAllocations);
    
    if (m_NumAllocations == m_Device->GetLimits().maxMemoryAllocationCount)
    {
        MLOG("Hit Maximum # of allocations (%d) reported by device!", m_NumAllocations);
    }
    
    uint32 heapIndex = m_MemoryProperties.memoryTypes[memoryTypeIndex].heapIndex;
    m_HeapInfos[heapIndex].allocations.push_back(newAllocation);
    m_HeapInfos[heapIndex].usedSize += allocationSize;
    m_HeapInfos[heapIndex].peakSize = MMath::Max(m_HeapInfos[heapIndex].peakSize, m_HeapInfos[heapIndex].usedSize);
    
    return newAllocation;
}

void VulkanDeviceMemoryManager::Free(VulkanDeviceMemoryAllocation*& allocation)
{
    
    vkFreeMemory(m_DeviceHandle, allocation->m_Handle, VULKAN_CPU_ALLOCATOR);
    --m_NumAllocations;
    uint32 heapIndex = m_MemoryProperties.memoryTypes[allocation->m_MemoryTypeIndex].heapIndex;
    
    m_HeapInfos[heapIndex].usedSize -= allocation->m_Size;
    auto it = std::find(m_HeapInfos[heapIndex].allocations.begin(), m_HeapInfos[heapIndex].allocations.end(), allocation);
    if (it != m_HeapInfos[heapIndex].allocations.end())
    {
        m_HeapInfos[heapIndex].allocations.erase(it);
    }
    allocation->m_FreedBySystem = true;
    delete allocation;
    allocation = nullptr;
}

#if MONKEY_DEBUG
void VulkanDeviceMemoryManager::DumpMemory()
{
    SetupAndPrintMemInfo();
    MLOG("Device Memory: %d allocations on %lu heaps", m_NumAllocations, m_HeapInfos.size());
    for (int32 index = 0; index < m_HeapInfos.size(); ++index)
    {
        HeapInfo& heapInfo = m_HeapInfos[index];
        MLOG("\tHeap %d, %lu allocations", index, heapInfo.allocations.size());
        uint64 totalSize = 0;
        for (int32 subIndex = 0; subIndex < heapInfo.allocations.size(); ++subIndex)
        {
            VulkanDeviceMemoryAllocation* allocation = heapInfo.allocations[subIndex];
            MLOG("\t\t%d Size %llu Handle %p", subIndex, allocation->m_Size, (void*)allocation->m_Handle);
            totalSize += allocation->m_Size;
        }
        MLOG("\t\tTotal Allocated %.2f MB, Peak %.2f MB", totalSize / 1024.0f / 1024.0f, heapInfo.peakSize / 1024.0f / 1024.0f);
    }
}
#endif

uint64 VulkanDeviceMemoryManager::GetTotalMemory(bool gpu) const
{
    uint64 totalMemory = 0;
    for (uint32 index = 0; index < m_MemoryProperties.memoryHeapCount; ++index)
    {
        const bool isGPUHeap = ((m_MemoryProperties.memoryHeaps[index].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
        if (isGPUHeap == gpu)
        {
            totalMemory += m_HeapInfos[index].totalSize;
        }
    }
    return totalMemory;
}

void VulkanDeviceMemoryManager::SetupAndPrintMemInfo()
{
    const uint32 maxAllocations = m_Device->GetLimits().maxMemoryAllocationCount;
    MLOG("%d Device Memory Heaps; Max memory allocations %d", m_MemoryProperties.memoryHeapCount, maxAllocations);
    for (uint32 index = 0; index < m_MemoryProperties.memoryHeapCount; ++index)
    {
        bool isGPUHeap = ((m_MemoryProperties.memoryHeaps[index].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
        MLOG("%d: Flags 0x%x Size %llu (%.2f MB) %s",
               index,
               m_MemoryProperties.memoryHeaps[index].flags,
               m_MemoryProperties.memoryHeaps[index].size,
               (float)((double)m_MemoryProperties.memoryHeaps[index].size / 1024.0 / 1024.0),
               isGPUHeap ? "GPU" : "");
        m_HeapInfos[index].totalSize = m_MemoryProperties.memoryHeaps[index].size;
    }
    
    m_HasUnifiedMemory = VulkanPlatform::HasUnifiedMemory();
    MLOG("%d Device Memory Types (%sunified)", m_MemoryProperties.memoryTypeCount, m_HasUnifiedMemory ? "" : "Not");
    for (uint32 index = 0; index < m_MemoryProperties.memoryTypeCount; ++index)
    {
        auto GetFlagsString = [](VkMemoryPropertyFlags flags)
        {
            std::string str;
            if ((flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            {
                str += " Local";
            }
            if ((flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                str += " HostVisible";
            }
            if ((flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
            {
                str += " HostCoherent";
            }
            if ((flags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) == VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
            {
                str += " HostCached";
            }
            if ((flags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) == VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
            {
                str += " Lazy";
            }
            return str;
        };
        
        MLOG("%d: Flags 0x%x Heap %d %s",
            index,
            m_MemoryProperties.memoryTypes[index].propertyFlags,
            m_MemoryProperties.memoryTypes[index].heapIndex,
            GetFlagsString(m_MemoryProperties.memoryTypes[index].propertyFlags).c_str());
    }
    
    for (uint32 index = 0; index < m_MemoryProperties.memoryHeapCount; ++index)
    {
        const bool isGPUHeap = ((m_MemoryProperties.memoryHeaps[index].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
        if (isGPUHeap)
        {
            m_HeapInfos[index].totalSize = (uint64)((float)m_HeapInfos[index].totalSize * 0.95f);
        }
    }
}
