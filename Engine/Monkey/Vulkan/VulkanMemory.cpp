#include "Common/Log.h"
#include "Math/Math.h"
#include "Utils/Alignment.h"
#include "VulkanDevice.h"
#include "VulkanMemory.h"
#include <algorithm>

enum
{
    GPU_ONLY_HEAP_PAGE_SIZE     = 256 * 1024 * 1024,
    STAGING_HEAP_PAGE_SIZE      = 32 * 1024 * 1024,
    ANDROID_MAX_HEAP_PAGE_SIZE  = 16 * 1024 * 1024,
};

constexpr uint32 VulkanResourceHeapManager::m_PoolSizes[(int32)VulkanResourceHeapManager::PoolSizes::SizesCount];
constexpr uint32 VulkanResourceHeapManager::m_BufferSizes[(int32)VulkanResourceHeapManager::PoolSizes::SizesCount + 1];

// VulkanRange
void VulkanRange::JoinConsecutiveRanges(std::vector<VulkanRange>& ranges)
{
    if (ranges.size() == 0) {
        return;
    }
    
    std::sort(ranges.begin(), ranges.end(), [](const VulkanRange& a, const VulkanRange& b) {
        if (a.offset > b.offset) {
            return 1;
        }
        else if (a.offset == b.offset) {
            return 0;
        }
        else {
            return -1;
        }
    });
    
    for (int32 index = (int32)ranges.size() - 1; index > 0; --index)
    {
        VulkanRange& current = ranges[index + 0];
        VulkanRange& prev    = ranges[index - 1];
        if (prev.offset + prev.size == current.offset)
        {
            prev.size += current.size;
            ranges.erase(ranges.begin() + index);
        }
    }
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
    m_Device             = device;
    m_NumAllocations     = 0;
    m_PeakNumAllocations = 0;
    m_DeviceHandle       = m_Device->GetInstanceHandle();

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
            MLOG("Found %d freed allocations!", (int32)m_HeapInfos[index].allocations.size());
#if MONKEY_DEBUG
            DumpMemory();
#endif
        }
    }
    m_NumAllocations = 0;
}

bool VulkanDeviceMemoryManager::SupportsMemoryType(VkMemoryPropertyFlags properties) const
{
    for (uint32 index = 0; index < m_MemoryProperties.memoryTypeCount; ++index)
    {
        if (m_MemoryProperties.memoryTypes[index].propertyFlags == properties) {
            return true;
        }
    }
    return false;
}

VulkanDeviceMemoryAllocation* VulkanDeviceMemoryManager::Alloc(bool canFail, VkDeviceSize allocationSize, uint32 memoryTypeIndex, void* dedicatedAllocateInfo, const char* file, uint32 line)
{
    VkMemoryAllocateInfo allocInfo;
    ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
    allocInfo.allocationSize  = allocationSize;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    allocInfo.pNext           = dedicatedAllocateInfo;
    
    VulkanDeviceMemoryAllocation* newAllocation = new VulkanDeviceMemoryAllocation();
    newAllocation->m_Device          = m_DeviceHandle;
    newAllocation->m_Size            = allocationSize;
    newAllocation->m_MemoryTypeIndex = memoryTypeIndex;
    newAllocation->m_CanBeMapped     = ((m_MemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)  == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    newAllocation->m_IsCoherent      = ((m_MemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    newAllocation->m_IsCached        = ((m_MemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)   == VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
    
    VkResult result = vkAllocateMemory(m_DeviceHandle, &allocInfo, VULKAN_CPU_ALLOCATOR, &newAllocation->m_Handle);
    
    if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
    {
        if (canFail)
        {
            MLOG("Failed to allocate Device Memory, Requested=%fKb MemTypeIndex=%d", (float)allocInfo.allocationSize / 1024.0f, allocInfo.memoryTypeIndex);
            return nullptr;
        }
        MLOG("Out of Device Memory, Requested=%fKb MemTypeIndex=%d", (float)allocInfo.allocationSize / 1024.0f, allocInfo.memoryTypeIndex);
#if MONKEY_DEBUG
        DumpMemory();
#endif
    }
    else if (result == VK_ERROR_OUT_OF_HOST_MEMORY)
    {
        if (canFail)
        {
            MLOG("Failed to allocate Host Memory, Requested=%fKb MemTypeIndex=%d", (float)allocInfo.allocationSize / 1024.0f, allocInfo.memoryTypeIndex);
            return nullptr;
        }
        MLOG("Out of Host Memory, Requested=%fKb MemTypeIndex=%d", (float)allocInfo.allocationSize / 1024.0f, allocInfo.memoryTypeIndex);
#if MONKEY_DEBUG
        DumpMemory();
#endif
    }
    else
    {
        VERIFYVULKANRESULT(result);
    }
    
    m_NumAllocations     += 1;
    m_PeakNumAllocations = MMath::Max(m_NumAllocations, m_PeakNumAllocations);
    if (m_NumAllocations == m_Device->GetLimits().maxMemoryAllocationCount) {
        MLOGE("Hit Maximum # of allocations (%d) reported by device!", m_NumAllocations);
    }
    
    uint32 heapIndex = m_MemoryProperties.memoryTypes[memoryTypeIndex].heapIndex;
    m_HeapInfos[heapIndex].allocations.push_back(newAllocation);
    m_HeapInfos[heapIndex].usedSize += allocationSize;
    m_HeapInfos[heapIndex].peakSize = MMath::Max(m_HeapInfos[heapIndex].peakSize, m_HeapInfos[heapIndex].usedSize);
    
    return newAllocation;
}

void VulkanDeviceMemoryManager::Free(VulkanDeviceMemoryAllocation*& allocation)
{
    m_NumAllocations -= 1;

    vkFreeMemory(m_DeviceHandle, allocation->m_Handle, VULKAN_CPU_ALLOCATOR);
    uint32 heapIndex = m_MemoryProperties.memoryTypes[allocation->m_MemoryTypeIndex].heapIndex;
    m_HeapInfos[heapIndex].usedSize -= allocation->m_Size;
    
    auto it = std::find(m_HeapInfos[heapIndex].allocations.begin(), m_HeapInfos[heapIndex].allocations.end(), allocation);
    if (it != m_HeapInfos[heapIndex].allocations.end()) {
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
    MLOG("Device Memory: %d allocations on %lu heaps", m_NumAllocations, (uint32)m_HeapInfos.size());
    for (int32 index = 0; index < m_HeapInfos.size(); ++index)
    {
        HeapInfo& heapInfo = m_HeapInfos[index];
        MLOG("\tHeap %d, %lu allocations", index, (uint32)heapInfo.allocations.size());
        uint64 totalSize = 0;
        for (int32 subIndex = 0; subIndex < heapInfo.allocations.size(); ++subIndex)
        {
            VulkanDeviceMemoryAllocation* allocation = heapInfo.allocations[subIndex];
            MLOG("\t\t%d Size %llu Handle %p", subIndex, (uint64)allocation->m_Size, (void*)allocation->m_Handle);
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
        if (isGPUHeap == gpu) {
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
        MLOG(
            "%d: Flags 0x%x Size %llu (%.2f MB) %s",
            index,
            m_MemoryProperties.memoryHeaps[index].flags,
            (uint64)(m_MemoryProperties.memoryHeaps[index].size),
            (float)((double)m_MemoryProperties.memoryHeaps[index].size / 1024.0 / 1024.0),
            isGPUHeap ? "GPU" : ""
        );
        m_HeapInfos[index].totalSize = m_MemoryProperties.memoryHeaps[index].size;
    }
    
    m_HasUnifiedMemory = false;
    MLOG("%d Device Memory Types (%sunified)", m_MemoryProperties.memoryTypeCount, m_HasUnifiedMemory ? "" : "Not");
    for (uint32 index = 0; index < m_MemoryProperties.memoryTypeCount; ++index)
    {
        auto GetFlagsString = [](VkMemoryPropertyFlags flags)
        {
            std::string str;
            if ((flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                str += " Local";
            }
            if ((flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                str += " HostVisible";
            }
            if ((flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
                str += " HostCoherent";
            }
            if ((flags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) == VK_MEMORY_PROPERTY_HOST_CACHED_BIT) {
                str += " HostCached";
            }
            if ((flags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) == VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
                str += " Lazy";
            }
            return str;
        };
        
        MLOG(
            "%d: Flags 0x%x Heap %d %s",
            index,
            m_MemoryProperties.memoryTypes[index].propertyFlags,
            m_MemoryProperties.memoryTypes[index].heapIndex,
            GetFlagsString(m_MemoryProperties.memoryTypes[index].propertyFlags).c_str()
        );
    }
    
    for (uint32 index = 0; index < m_MemoryProperties.memoryHeapCount; ++index)
    {
        bool isGPUHeap = ((m_MemoryProperties.memoryHeaps[index].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
        if (isGPUHeap) {
            m_HeapInfos[index].totalSize = (uint64)((float)m_HeapInfos[index].totalSize * 0.95f);
        }
    }
}

// VulkanResourceAllocation
VulkanResourceAllocation::VulkanResourceAllocation(VulkanResourceHeapPage* owner, VulkanDeviceMemoryAllocation* deviceMemoryAllocation, uint32 requestedSize, uint32 alignedOffset, uint32 allocationSize, uint32 allocationOffset, const char* file, uint32 line)
    : m_Owner(owner)
    , m_AllocationSize(allocationSize)
    , m_AllocationOffset(allocationOffset)
    , m_RequestedSize(requestedSize)
    , m_AlignedOffset(alignedOffset)
    , m_DeviceMemoryAllocation(deviceMemoryAllocation)
{

}

VulkanResourceAllocation::~VulkanResourceAllocation()
{
    m_Owner->ReleaseAllocation(this);
}

void VulkanResourceAllocation::BindBuffer(VulkanDevice* device, VkBuffer buffer)
{
    VkResult result = vkBindBufferMemory(device->GetInstanceHandle(), buffer, GetHandle(), GetOffset());
#if MONKEY_DEBUG
    if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY || result == VK_ERROR_OUT_OF_HOST_MEMORY)
    {
        device->GetMemoryManager().DumpMemory();
    }
    VERIFYVULKANRESULT(result);
#endif
}

void VulkanResourceAllocation::BindImage(VulkanDevice* device, VkImage image)
{
    VkResult result = vkBindImageMemory(device->GetInstanceHandle(), image, GetHandle(), GetOffset());
#if MONKEY_DEBUG
    if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY || result == VK_ERROR_OUT_OF_HOST_MEMORY)
    {
        device->GetMemoryManager().DumpMemory();
    }
#endif
    VERIFYVULKANRESULT(result);
}

// VulkanResourceHeapPage
VulkanResourceHeapPage::VulkanResourceHeapPage(VulkanResourceHeap* owner, VulkanDeviceMemoryAllocation* deviceMemoryAllocation, uint32 id)
    : m_Owner(owner)
    , m_DeviceMemoryAllocation(deviceMemoryAllocation)
    , m_MaxSize(0)
    , m_UsedSize(0)
    , m_PeakNumAllocations(0)
    , m_FrameFreed(0)
    , m_ID(id)
{
    m_MaxSize = (uint32)m_DeviceMemoryAllocation->GetSize();

    VulkanRange fullRange;
    fullRange.offset = 0;
    fullRange.size   = m_MaxSize;
    m_FreeList.push_back(fullRange);
}

VulkanResourceHeapPage::~VulkanResourceHeapPage()
{
    if (m_DeviceMemoryAllocation == nullptr) {
        MLOGE("Device memory allocation is null.")
    }
}

void VulkanResourceHeapPage::ReleaseAllocation(VulkanResourceAllocation* allocation)
{
    auto it = std::find(m_ResourceAllocations.begin(), m_ResourceAllocations.end(), allocation);
    if (it != m_ResourceAllocations.end())
    {
        m_ResourceAllocations.erase(it);
        VulkanRange newFree;
        newFree.offset = allocation->m_AllocationOffset;
        newFree.size   = allocation->m_AllocationSize;
        m_FreeList.push_back(newFree);
    }
    
    m_UsedSize -= allocation->m_AllocationSize;
    if (m_UsedSize < 0) {
        MLOGE("Used size less than zero.");
    }
    
    if (JoinFreeBlocks()) {
        m_Owner->FreePage(this);
    }
}

VulkanResourceAllocation* VulkanResourceHeapPage::TryAllocate(uint32 size, uint32 alignment, const char* file, uint32 line)
{
    for (int32 index = 0; index < m_FreeList.size(); ++index)
    {
        VulkanRange& entry         = m_FreeList[index];
        uint32 allocatedOffset     = entry.offset;
        uint32 alignedOffset       = Align(entry.offset, alignment);
        uint32 alignmentAdjustment = alignedOffset - entry.offset;
        uint32 allocatedSize       = alignmentAdjustment + size;

        if (allocatedSize <= entry.size)
        {
            if (allocatedSize < entry.size) {
                entry.size   -= allocatedSize;
                entry.offset += allocatedSize;
            }
            else {
                m_FreeList.erase(m_FreeList.begin() + index);
            }
            m_UsedSize += allocatedSize;
            VulkanResourceAllocation* newResourceAllocation = new VulkanResourceAllocation(this, m_DeviceMemoryAllocation, size, alignedOffset, allocatedSize, allocatedOffset, file, line);
            m_ResourceAllocations.push_back(newResourceAllocation);
            m_PeakNumAllocations = MMath::Max((uint32)m_PeakNumAllocations, (uint32)m_ResourceAllocations.size());
            
            return newResourceAllocation;
        }
    }
    
    return nullptr;
}

bool VulkanResourceHeapPage::JoinFreeBlocks()
{
    VulkanRange::JoinConsecutiveRanges(m_FreeList);
    if (m_FreeList.size() == 1)
    {
        if (m_ResourceAllocations.size() == 0)
        {
            if (m_UsedSize > 0) {
                MLOGE("Memory leak, used size = %d", (int32)m_UsedSize);
            }
            if (m_FreeList[0].offset != 0 || m_FreeList[0].size != m_MaxSize) {
                MLOGE("Memory leak, should have %d free, only have %d; missing %d bytes", m_MaxSize, m_FreeList[0].size, m_MaxSize - m_FreeList[0].size);
            }
            return true;
        }
    }
    
    return false;
}

// VulkanResourceHeap

VulkanResourceHeap::VulkanResourceHeap(VulkanResourceHeapManager* owner, uint32 memoryTypeIndex, uint32 pageSize)
    : m_Owner(owner)
    , m_MemoryTypeIndex(memoryTypeIndex)
    , m_IsHostCachedSupported(false)
    , m_IsLazilyAllocatedSupported(false)
    , m_DefaultPageSize(pageSize)
    , m_PeakPageSize(0)
    , m_UsedMemory(0)
    , m_PageIDCounter(0)
{
    
}

VulkanResourceHeap::~VulkanResourceHeap()
{
    ReleaseFreedPages(true);
    
    auto DeletePages = [&](std::vector<VulkanResourceHeapPage*>& usedPages, const char* name)
    {
        bool leak = false;
        for (int32 index = (int32)usedPages.size() - 1; index >= 0; --index)
        {
            VulkanResourceHeapPage* page = usedPages[index];
            if (!page->JoinFreeBlocks())
            {
                MLOG("Page allocation %p has unfreed %s resources", (void*)page->m_DeviceMemoryAllocation->GetHandle(), name);
                leak = true;
            }
            m_Owner->GetVulkanDevice()->GetMemoryManager().Free(page->m_DeviceMemoryAllocation);
            delete page;
        }
        usedPages.clear();
        return leak;
    };
    
    bool dump = false;
    dump = dump || DeletePages(m_UsedBufferPages,   "Buffer");
    dump = dump || DeletePages(m_UsedImagePages,    "Image");
    dump = dump || DeletePages(m_FreePages,         "Free");
    
    if (dump)
    {
#if MONKEY_DEBUG
        m_Owner->GetVulkanDevice()->GetMemoryManager().DumpMemory();
#endif
    }
}

void VulkanResourceHeap::FreePage(VulkanResourceHeapPage* page)
{
    page->JoinFreeBlocks();
    
    bool removed = false;
    for (int32 i = 0; i < m_UsedBufferPages.size(); ++i)
    {
        if (m_UsedBufferPages[i] == page)
        {
            removed = true;
            m_UsedBufferPages.erase(m_UsedBufferPages.begin() + i);
            break;
        }
    }
    
    for (int32 i = 0; i < m_UsedImagePages.size(); ++i)
    {
        if (m_UsedImagePages[i] == page)
        {
            removed = true;
            m_UsedImagePages.erase(m_UsedImagePages.begin() + i);
            break;
        }
    }
    
    if (removed)
    {
        page->m_FrameFreed = 0;
        m_FreePages.push_back(page);
    }
}

void VulkanResourceHeap::ReleaseFreedPages(bool immediately)
{
    for (int32 index = 0; index < m_FreePages.size(); ++index)
    {
        VulkanResourceHeapPage* page = m_FreePages[index];
        m_UsedMemory -= page->m_MaxSize;
        m_Owner->GetVulkanDevice()->GetMemoryManager().Free(page->m_DeviceMemoryAllocation);
        delete page;
    }
    
    m_FreePages.clear();
}

#if MONKEY_DEBUG
void VulkanResourceHeap::DumpMemory()
{
    MLOG("%d Free Pages", (int32)m_FreePages.size());
    
    auto DumpPages = [&](std::vector<VulkanResourceHeapPage*>& usedPages, const char* typeName)
    {
        MLOG("\t%s Pages: %d Used, Peak Allocation Size on a Page %d", typeName, (int32)usedPages.size(), m_PeakPageSize);
        uint64 subAllocUsedMemory      = 0;
        uint64 subAllocAllocatedMemory = 0;
        uint32 numSubAllocations       = 0;
        for (int32 index = 0; index < usedPages.size(); ++index)
        {
            subAllocUsedMemory      += usedPages[index]->m_UsedSize;
            subAllocAllocatedMemory += usedPages[index]->m_MaxSize;
            numSubAllocations       += (uint32)usedPages[index]->m_ResourceAllocations.size();
            MLOG("\t\t%d: ID %4d %4d suballocs, %4d free chunks (%d used/%d free/%d max) DeviceMemory %p", index, usedPages[index]->GetID(), (int32)usedPages[index]->m_ResourceAllocations.size(), (int32)usedPages[index]->m_FreeList.size(), usedPages[index]->m_UsedSize, usedPages[index]->m_MaxSize - usedPages[index]->m_UsedSize, usedPages[index]->m_MaxSize, (void*)usedPages[index]->m_DeviceMemoryAllocation->GetHandle());
        }
        
        MLOG("%d Suballocations for Used/Total: %d/%d = %.2f%%", numSubAllocations, (int32)subAllocUsedMemory, (int32)subAllocAllocatedMemory, subAllocAllocatedMemory > 0 ? 100.0f * (float)subAllocUsedMemory / (float)subAllocAllocatedMemory : 0.0f);
    };
    
    DumpPages(m_UsedBufferPages, "Buffer");
    DumpPages(m_UsedImagePages,  "Image");
}
#endif

VulkanResourceAllocation* VulkanResourceHeap::AllocateResource(Type type, uint32 size, uint32 alignment, bool mapAllocation, const char* file, uint32 line)
{
    std::vector<VulkanResourceHeapPage*>& usedPages = type == Type::Image ? m_UsedImagePages : m_UsedBufferPages;
    uint32 targetDefaultPageSize = m_DefaultPageSize;
    
    if (size < targetDefaultPageSize)
    {
        for (int32 index = 0; index < usedPages.size(); ++index)
        {
            VulkanResourceHeapPage* page = usedPages[index];
            if (page->m_DeviceMemoryAllocation->IsMapped() == mapAllocation)
            {
                VulkanResourceAllocation* resourceAllocation = page->TryAllocate(size, alignment, file, line);
                if (resourceAllocation) {
                    return resourceAllocation;
                }
            }
        }
    }
    
    for (int32 index = 0; index < m_FreePages.size(); ++index)
    {
        VulkanResourceHeapPage* page = m_FreePages[index];
        if (page->m_DeviceMemoryAllocation->IsMapped() == mapAllocation)
        {
            VulkanResourceAllocation* resourceAllocation = page->TryAllocate(size, alignment, file, line);
            if (resourceAllocation)
            {
                m_FreePages.erase(m_FreePages.begin() + index);
                usedPages.push_back(page);
                return resourceAllocation;
            }
        }
    }
    
    uint32 allocationSize = MMath::Max(size, targetDefaultPageSize);
    VulkanDeviceMemoryAllocation* deviceMemoryAllocation = m_Owner->GetVulkanDevice()->GetMemoryManager().Alloc(true, allocationSize, m_MemoryTypeIndex, nullptr, file, line);
    if (!deviceMemoryAllocation && size < allocationSize) {
        deviceMemoryAllocation = m_Owner->GetVulkanDevice()->GetMemoryManager().Alloc(false, size, m_MemoryTypeIndex, nullptr, file, line);
    }
    
    VulkanResourceHeapPage* newPage = new VulkanResourceHeapPage(this, deviceMemoryAllocation, m_PageIDCounter);
    usedPages.push_back(newPage);

    m_PageIDCounter += 1;
    m_UsedMemory    += allocationSize;
    m_PeakPageSize   = MMath::Max(m_PeakPageSize, allocationSize);
    
    if (mapAllocation) {
        deviceMemoryAllocation->Map(allocationSize, 0);
    }
    
    return newPage->Allocate(size, alignment, file, line);
}

// VulkanResourceSubAllocation
VulkanResourceSubAllocation::VulkanResourceSubAllocation(uint32 requestedSize, uint32 alignedOffset, uint32 allocationSize, uint32 allocationOffset)
    : m_RequestedSize(requestedSize)
    , m_AlignedOffset(alignedOffset)
    , m_AllocationSize(allocationSize)
    , m_AllocationOffset(allocationOffset)
{
    
}

VulkanResourceSubAllocation::~VulkanResourceSubAllocation()
{
    
}

// VulkanBufferSubAllocation
VulkanBufferSubAllocation::VulkanBufferSubAllocation(VulkanSubBufferAllocator* owner, VkBuffer handle, uint32 requestedSize, uint32 alignedOffset, uint32 allocationSize, uint32 allocationOffset)
    : VulkanResourceSubAllocation(requestedSize, alignedOffset, allocationSize, allocationOffset)
    , m_Owner(owner)
    , m_Handle(handle)
{
    
}

VulkanBufferSubAllocation::~VulkanBufferSubAllocation()
{
    m_Owner->Release(this);
}

void* VulkanBufferSubAllocation::GetMappedPointer()
{
    return (uint8*)m_Owner->GetMappedPointer() + m_AlignedOffset;
}

// VulkanSubResourceAllocator
VulkanSubResourceAllocator::VulkanSubResourceAllocator(VulkanResourceHeapManager* owner, VulkanDeviceMemoryAllocation* deviceMemoryAllocation, uint32 memoryTypeIndex, VkMemoryPropertyFlags memoryPropertyFlags, uint32 alignment)
    : m_Owner(owner)
    , m_MemoryTypeIndex(memoryTypeIndex)
    , m_MemoryPropertyFlags(memoryPropertyFlags)
    , m_DeviceMemoryAllocation(deviceMemoryAllocation)
    , m_Alignment(alignment)
    , m_FrameFreed(0)
    , m_UsedSize(0)
{
    m_MaxSize = (uint32)deviceMemoryAllocation->GetSize();

    VulkanRange fullRange;
    fullRange.offset = 0;
    fullRange.size   = m_MaxSize;
    m_FreeList.push_back(fullRange);
}

VulkanSubResourceAllocator::~VulkanSubResourceAllocator()
{
    
}

VulkanResourceSubAllocation* VulkanSubResourceAllocator::TryAllocateNoLocking(uint32 size, uint32 alignment, const char* file, uint32 line)
{
    m_Alignment = MMath::Max(m_Alignment, alignment);
    for (int32 index = 0; index < m_FreeList.size(); ++index)
    {
        VulkanRange& entry         = m_FreeList[index];
        uint32 allocatedOffset     = entry.offset;
        uint32 alignedOffset       = Align(entry.offset, m_Alignment);
        uint32 alignmentAdjustment = alignedOffset - entry.offset;
        uint32 allocatedSize       = alignmentAdjustment + size;

        if (allocatedSize <= entry.size)
        {
            if (allocatedSize < entry.size)
            {
                entry.size   -= allocatedSize;
                entry.offset += allocatedSize;
            }
            else {
                m_FreeList.erase(m_FreeList.begin() + index);
            }
            m_UsedSize += allocatedSize;
            VulkanResourceSubAllocation* newSubAllocation = CreateSubAllocation(size, alignedOffset, allocatedSize, allocatedOffset);
            m_SubAllocations.push_back(newSubAllocation);
            return newSubAllocation;
        }
    }
    
    return nullptr;
}

bool VulkanSubResourceAllocator::JoinFreeBlocks()
{
    VulkanRange::JoinConsecutiveRanges(m_FreeList);
    if (m_FreeList.size() == 1)
    {
        if (m_SubAllocations.size() == 0)
        {
            if (m_UsedSize != 0 || m_FreeList[0].offset != 0 || m_FreeList[0].size != m_MaxSize) {
                MLOG("Resource Suballocation leak, should have %d free, only have %d; missing %d bytes", m_MaxSize, m_FreeList[0].size, m_MaxSize - m_FreeList[0].size);
            }
            return true;
        }
    }
    
    return false;
}

// VulkanSubBufferAllocator
VulkanSubBufferAllocator::VulkanSubBufferAllocator(VulkanResourceHeapManager* owner, VulkanDeviceMemoryAllocation* deviceMemoryAllocation, uint32 memoryTypeIndex, VkMemoryPropertyFlags memoryPropertyFlags, uint32 alignment, VkBuffer buffer, VkBufferUsageFlags bufferUsageFlags, int32 poolSizeIndex)
    : VulkanSubResourceAllocator(owner, deviceMemoryAllocation, memoryTypeIndex, memoryPropertyFlags, alignment)
    , m_BufferUsageFlags(bufferUsageFlags)
    , m_Buffer(buffer)
    , m_PoolSizeIndex(poolSizeIndex)
{
    
}

VulkanSubBufferAllocator::~VulkanSubBufferAllocator()
{
    if (m_Buffer != VK_NULL_HANDLE) {
        MLOGE("Failed destory VulkanSubBufferAllocator, buffer not null.")
    }
}

void VulkanSubBufferAllocator::Destroy(VulkanDevice* device)
{
    vkDestroyBuffer(device->GetInstanceHandle(), m_Buffer, VULKAN_CPU_ALLOCATOR);
	m_Buffer = VK_NULL_HANDLE;
}

VulkanResourceSubAllocation* VulkanSubBufferAllocator::CreateSubAllocation(uint32 size, uint32 alignedOffset, uint32 allocatedSize, uint32 allocatedOffset)
{
    return new VulkanBufferSubAllocation(this, m_Buffer, size, alignedOffset, allocatedSize, allocatedOffset);
}

void VulkanSubBufferAllocator::Release(VulkanBufferSubAllocation* subAllocation)
{
    bool released = false;
    for (int32 index = 0; index < m_SubAllocations.size(); ++index)
    {
        if (m_SubAllocations[index] == subAllocation)
        {
            released = true;
            m_SubAllocations.erase(m_SubAllocations.begin() + index);
            break;
        }
    }
    
    if (released)
    {
        VulkanRange newFree;
        newFree.offset = subAllocation->m_AllocationOffset;
        newFree.size   = subAllocation->m_AllocationSize;
        m_FreeList.push_back(newFree);
        m_UsedSize -= subAllocation->m_AllocationSize;
    }
    
    if (JoinFreeBlocks()) {
        m_Owner->ReleaseBuffer(this);
    }
}

// VulkanResourceHeapManager

VulkanResourceHeapManager::VulkanResourceHeapManager(VulkanDevice* device)
    : m_VulkanDevice(device)
    , m_DeviceMemoryManager(&device->GetMemoryManager())
{
    
}

VulkanResourceHeapManager::~VulkanResourceHeapManager()
{
    Destory();
}

void VulkanResourceHeapManager::Init()
{
    VulkanDeviceMemoryManager& memoryManager = m_VulkanDevice->GetMemoryManager();

    const uint32 typeBits = (1 << memoryManager.GetNumMemoryTypes()) - 1;
    const VkPhysicalDeviceMemoryProperties& memoryProperties = memoryManager.GetMemoryProperties();

    m_ResourceTypeHeaps.resize(memoryProperties.memoryTypeCount);
    
    auto GetMemoryTypesFromProperties = [memoryProperties](uint32 typeBits, VkMemoryPropertyFlags properties, std::vector<uint32>& outTypeIndices)
    {
        for (uint32 i = 0; i < memoryProperties.memoryTypeCount && typeBits; ++i)
        {
            if ((typeBits & 1) == 1)
            {
                if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    outTypeIndices.push_back(i);
                }
            }
            typeBits >>= 1;
        }
        
        for (int32 index = (int32)outTypeIndices.size() - 1; index >= 1; --index)
        {
            if (memoryProperties.memoryTypes[index].propertyFlags != memoryProperties.memoryTypes[0].propertyFlags) {
                outTypeIndices.erase(outTypeIndices.begin() + index);
            }
        }
        
        return outTypeIndices.size() > 0;
    };
    
    {
        std::vector<uint32> typeIndices;
        GetMemoryTypesFromProperties(typeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, typeIndices);
        for (int32 index = 0; index < typeIndices.size(); ++index)
        {
            int32 heapIndex = memoryProperties.memoryTypes[typeIndices[index]].heapIndex;
            VkDeviceSize heapSize = memoryProperties.memoryHeaps[heapIndex].size;
            VkDeviceSize pageSize = MMath::Min<VkDeviceSize>(heapSize / 8, GPU_ONLY_HEAP_PAGE_SIZE);
            m_ResourceTypeHeaps[typeIndices[index]] = new VulkanResourceHeap(this, typeIndices[index], uint32(pageSize));
            m_ResourceTypeHeaps[typeIndices[index]]->m_IsHostCachedSupported      = ((memoryProperties.memoryTypes[index].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)      == VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
            m_ResourceTypeHeaps[typeIndices[index]]->m_IsLazilyAllocatedSupported = ((memoryProperties.memoryTypes[index].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) == VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
        }
    }
    
    {
        uint32 typeIndex = 0;
        VERIFYVULKANRESULT(memoryManager.GetMemoryTypeFromProperties(typeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &typeIndex));
        m_ResourceTypeHeaps[typeIndex] = new VulkanResourceHeap(this, typeIndex, STAGING_HEAP_PAGE_SIZE);
    }
    
    {
        uint32 typeIndex          = 0;
        uint32 hostVisCachedIndex = 0;
        VkResult hostCachedResult = memoryManager.GetMemoryTypeFromProperties(typeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, &hostVisCachedIndex);
        uint32 hostVisIndex       = 0;
        VkResult hostResult       = memoryManager.GetMemoryTypeFromProperties(typeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &hostVisIndex);
        if (hostCachedResult == VK_SUCCESS) {
            typeIndex = hostVisCachedIndex;
        }
        else if (hostResult == VK_SUCCESS) {
            typeIndex = hostVisIndex;
        }
        else {
            MLOG("No Memory Type found supporting VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT!");
        }
        m_ResourceTypeHeaps[typeIndex] = new VulkanResourceHeap(this, typeIndex, STAGING_HEAP_PAGE_SIZE);
    }
}

void VulkanResourceHeapManager::Destory()
{
    DestroyResourceAllocations();
    for (int32 index = 0; index < m_ResourceTypeHeaps.size(); ++index)
    {
        delete m_ResourceTypeHeaps[index];
        m_ResourceTypeHeaps[index] = nullptr;
    }
    m_ResourceTypeHeaps.clear();
}

VulkanResourceAllocation* VulkanResourceHeapManager::AllocateBufferMemory(const VkMemoryRequirements& memoryReqs, VkMemoryPropertyFlags memoryPropertyFlags, const char* file, uint32 line)
{
    uint32 typeIndex = 0;
    VERIFYVULKANRESULT(m_DeviceMemoryManager->GetMemoryTypeFromProperties(memoryReqs.memoryTypeBits, memoryPropertyFlags, &typeIndex));
    
    bool canMapped = (memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    
    if (!m_ResourceTypeHeaps[typeIndex])
    {
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) == VK_MEMORY_PROPERTY_HOST_CACHED_BIT) {
            memoryPropertyFlags = memoryPropertyFlags & ~VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        }
        
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) == VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
            memoryPropertyFlags = memoryPropertyFlags & ~VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
        }
        
        uint32 originalTypeIndex = typeIndex;
        if (m_DeviceMemoryManager->GetMemoryTypeFromPropertiesExcluding(memoryReqs.memoryTypeBits, memoryPropertyFlags, typeIndex, &typeIndex) != VK_SUCCESS)
        {
            MLOG("Unable to find alternate type for index %d, MemSize %d, MemPropTypeBits %u, MemPropertyFlags %u, %s(%d)", originalTypeIndex, (uint32)memoryReqs.size, (uint32)memoryReqs.memoryTypeBits, (uint32)memoryPropertyFlags, file, line);
        }
        
#if MONKEY_DEBUG
        DumpMemory();
#endif
        MLOG("Missing memory type index %d (originally requested %d), MemSize %d, MemPropTypeBits %u, MemPropertyFlags %u, %s(%d)", typeIndex, originalTypeIndex, (uint32)memoryReqs.size, (uint32)memoryReqs.memoryTypeBits, (uint32)memoryPropertyFlags, file, line);
    }
    
    VulkanResourceAllocation* allocation = m_ResourceTypeHeaps[typeIndex]->AllocateResource(VulkanResourceHeap::Type::Buffer, uint32(memoryReqs.size), uint32(memoryReqs.alignment), canMapped, file, line);
    
    if (!allocation)
    {
        VERIFYVULKANRESULT(m_DeviceMemoryManager->GetMemoryTypeFromPropertiesExcluding(memoryReqs.memoryTypeBits, memoryPropertyFlags, typeIndex, &typeIndex));
        canMapped = (memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        if (!m_ResourceTypeHeaps[typeIndex]) {
            MLOG("Missing memory type index %d, MemSize %d, MemPropTypeBits %u, MemPropertyFlags %u, %s(%d)", typeIndex, (uint32)memoryReqs.size, (uint32)memoryReqs.memoryTypeBits, (uint32)memoryPropertyFlags, file, line);
        }
        allocation = m_ResourceTypeHeaps[typeIndex]->AllocateResource(VulkanResourceHeap::Type::Buffer, uint32(memoryReqs.size), uint32(memoryReqs.alignment), canMapped, file, line);
    }

    return allocation;
}

VulkanResourceAllocation* VulkanResourceHeapManager::AllocateImageMemory(const VkMemoryRequirements& memoryReqs, VkMemoryPropertyFlags memoryPropertyFlags, const char* file, uint32 line)
{
    uint32 typeIndex = 0;
    VERIFYVULKANRESULT(m_DeviceMemoryManager->GetMemoryTypeFromProperties(memoryReqs.memoryTypeBits, memoryPropertyFlags, &typeIndex));
    
    bool canMapped = (memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    
    if (!m_ResourceTypeHeaps[typeIndex]) {
        MLOG("Missing memory type index %d, MemSize %d, MemPropTypeBits %u, MemPropertyFlags %u, %s(%d)", typeIndex, (uint32)memoryReqs.size, (uint32)memoryReqs.memoryTypeBits, (uint32)memoryPropertyFlags, file, line);
    }
    
    VulkanResourceAllocation* allocation = m_ResourceTypeHeaps[typeIndex]->AllocateResource(VulkanResourceHeap::Type::Image, uint32(memoryReqs.size), uint32(memoryReqs.alignment), canMapped, file, line);
    
    if (!allocation)
    {
        VERIFYVULKANRESULT(m_DeviceMemoryManager->GetMemoryTypeFromPropertiesExcluding(memoryReqs.memoryTypeBits, memoryPropertyFlags, typeIndex, &typeIndex));
        canMapped  = (memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        allocation = m_ResourceTypeHeaps[typeIndex]->AllocateResource(VulkanResourceHeap::Type::Image, uint32(memoryReqs.size), uint32(memoryReqs.alignment), canMapped, file, line);
    }
    
    return allocation;
}

VulkanBufferSubAllocation* VulkanResourceHeapManager::AllocateBuffer(uint32 size, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags memoryPropertyFlags, const char* file, uint32 line)
{
    const VkPhysicalDeviceLimits& limits = m_VulkanDevice->GetLimits();
    uint32 alignment = 1;
    
    bool isStorageOrTexel = (bufferUsageFlags & (VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)) != 0;
    if (isStorageOrTexel)
    {
        alignment = MMath::Max(alignment, ((bufferUsageFlags & (VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)) != 0) ? (uint32)limits.minTexelBufferOffsetAlignment : 1u);
        alignment = MMath::Max(alignment, ((bufferUsageFlags & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) != 0) ? (uint32)limits.minStorageBufferOffsetAlignment : 1u);
    }
    else
    {
        alignment = (uint32)limits.minUniformBufferOffsetAlignment;
        bufferUsageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }
    
    int32 poolSize = (int32)GetPoolTypeForAlloc(size, alignment);
    if (poolSize != (int32)PoolSizes::SizesCount) {
        size = m_PoolSizes[poolSize];
    }
    
    for (int32 index = 0; index < m_UsedBufferAllocations[poolSize].size(); ++index)
    {
        VulkanSubBufferAllocator* bufferAllocation = m_UsedBufferAllocations[poolSize][index];
        if ((bufferAllocation->m_BufferUsageFlags & bufferUsageFlags) == bufferUsageFlags &&
            (bufferAllocation->m_MemoryPropertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
        {
            VulkanBufferSubAllocation* subAllocation = (VulkanBufferSubAllocation*)bufferAllocation->TryAllocateNoLocking(size, alignment, file, line);
            if (subAllocation) {
                return subAllocation;
            }
        }
    }
    
    for (int32 index = 0; index < m_FreeBufferAllocations[poolSize].size(); ++index)
    {
        VulkanSubBufferAllocator* bufferAllocation = m_FreeBufferAllocations[poolSize][index];
        if ((bufferAllocation->m_BufferUsageFlags & bufferUsageFlags) == bufferUsageFlags &&
            (bufferAllocation->m_MemoryPropertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
        {
            VulkanBufferSubAllocation* subAllocation = (VulkanBufferSubAllocation*)bufferAllocation->TryAllocateNoLocking(size, alignment, file, line);
            if (subAllocation)
            {
                m_FreeBufferAllocations[poolSize].erase(m_FreeBufferAllocations[poolSize].begin() + index);
                m_UsedBufferAllocations[poolSize].push_back(bufferAllocation);
                return subAllocation;
            }
        }
    }
    
    uint32 bufferSize = MMath::Max(size, m_BufferSizes[poolSize]);
    
    VkBuffer buffer = VK_NULL_HANDLE;
    VkBufferCreateInfo bufferCreateInfo;
    ZeroVulkanStruct(bufferCreateInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
    bufferCreateInfo.size  = bufferSize;
    bufferCreateInfo.usage = bufferUsageFlags;
    VERIFYVULKANRESULT(vkCreateBuffer(m_VulkanDevice->GetInstanceHandle(), &bufferCreateInfo, VULKAN_CPU_ALLOCATOR, &buffer));
    
    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(m_VulkanDevice->GetInstanceHandle(), buffer, &memReqs);
    uint32 memoryTypeIndex = 0;
    VERIFYVULKANRESULT(m_VulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, memoryPropertyFlags, &memoryTypeIndex));
    alignment = MMath::Max((uint32)memReqs.alignment, alignment);
    
    VulkanDeviceMemoryAllocation* deviceMemoryAllocation = m_VulkanDevice->GetMemoryManager().Alloc(false, memReqs.size, memoryTypeIndex, nullptr, file, line);
    VERIFYVULKANRESULT(vkBindBufferMemory(m_VulkanDevice->GetInstanceHandle(), buffer, deviceMemoryAllocation->GetHandle(), 0));

    if (deviceMemoryAllocation->CanBeMapped()) {
        deviceMemoryAllocation->Map(bufferSize, 0);
    }
    
    VulkanSubBufferAllocator* bufferAllocation = new VulkanSubBufferAllocator(this, deviceMemoryAllocation, memoryTypeIndex, memoryPropertyFlags, uint32(memReqs.alignment), buffer, bufferUsageFlags, poolSize);
    m_UsedBufferAllocations[poolSize].push_back(bufferAllocation);
    
    return (VulkanBufferSubAllocation*)bufferAllocation->TryAllocateNoLocking(size, alignment, file, line);
}

void VulkanResourceHeapManager::ReleaseBuffer(VulkanSubBufferAllocator* bufferAllocator)
{
    bufferAllocator->JoinFreeBlocks();
    for (int32 index = 0; index < m_UsedBufferAllocations[bufferAllocator->m_PoolSizeIndex].size(); ++index)
    {
        if (m_UsedBufferAllocations[bufferAllocator->m_PoolSizeIndex][index] == bufferAllocator) {
            m_UsedBufferAllocations[bufferAllocator->m_PoolSizeIndex].erase(m_UsedBufferAllocations[bufferAllocator->m_PoolSizeIndex].begin() + index);
        }
    }
    bufferAllocator->m_FrameFreed = 0;
    m_FreeBufferAllocations[bufferAllocator->m_PoolSizeIndex].push_back(bufferAllocator);
}

void VulkanResourceHeapManager::ReleaseFreedPages()
{
    for (int32 index = 0; index < m_ResourceTypeHeaps.size(); ++index)
    {
        VulkanResourceHeap* heap = m_ResourceTypeHeaps[index];
        if (heap) {
            heap->ReleaseFreedPages(true);
        }
    }
    ReleaseFreedResources(false);
}

#if MONKEY_DEBUG
void VulkanResourceHeapManager::DumpMemory()
{
    for (int32 index = 0; index < m_ResourceTypeHeaps.size(); ++index)
    {
        if (m_ResourceTypeHeaps[index]) {
            MLOG("Heap %d, Memory Type Index %d", index, m_ResourceTypeHeaps[index]->m_MemoryTypeIndex);
            m_ResourceTypeHeaps[index]->DumpMemory();
        }
        else {
            MLOG("Heap %d, NOT USED", index);
        }
    }
    
    uint64 usedBinnedTotal  = 0;
    uint64 allocBinnedTotal = 0;
    uint64 usedLargeTotal   = 0;
    uint64 allocLargeTotal  = 0;
    
    for (int32 poolSizeIndex = 0; poolSizeIndex < (int32)PoolSizes::SizesCount + 1; poolSizeIndex++)
    {
        std::vector<VulkanSubBufferAllocator*>& usedAllocations = m_UsedBufferAllocations[poolSizeIndex];
        std::vector<VulkanSubBufferAllocator*>& freeAllocations = m_FreeBufferAllocations[poolSizeIndex];
        if (poolSizeIndex == (int32)PoolSizes::SizesCount) {
            MLOG("Buffer of large size Allocations: %d Used / %d Free", (int32)usedAllocations.size(), (int32)freeAllocations.size());
        }
        else {
            MLOG("Buffer of %d size Allocations: %d Used / %d Free", m_PoolSizes[poolSizeIndex], (int32)usedAllocations.size(), (int32)freeAllocations.size());
        }
        
        if (usedAllocations.size() > 0)
        {
            uint64 _UsedBinnedTotal  = 0;
            uint64 _AllocBinnedTotal = 0;
            uint64 _UsedLargeTotal   = 0;
            uint64 _AllocLargeTotal  = 0;
            
            MLOG("Index  BufferHandle   DeviceMemoryHandle MemFlags BufferFlags #Suballocs #FreeChunks UsedSize/MaxSize");
            for (int32 index = 0; index < usedAllocations.size(); ++index)
            {
                VulkanSubBufferAllocator* bufferAllocation = usedAllocations[index];
                MLOG("%6d %p %p 0x%06x 0x%08x %6d   %6d    %d/%d", index, (void*)bufferAllocation->m_Buffer, (void*)bufferAllocation->m_DeviceMemoryAllocation->GetHandle(), bufferAllocation->m_MemoryPropertyFlags, bufferAllocation->m_BufferUsageFlags, (int32)bufferAllocation->m_SubAllocations.size(), (int32)bufferAllocation->m_FreeList.size(), (int32)bufferAllocation->m_UsedSize, bufferAllocation->m_MaxSize);
                
                if (poolSizeIndex == (int32)PoolSizes::SizesCount)
                {
                    _UsedLargeTotal  += bufferAllocation->m_UsedSize;
                    _AllocLargeTotal += bufferAllocation->m_MaxSize;
                    usedLargeTotal   += bufferAllocation->m_UsedSize;
                    allocLargeTotal  += bufferAllocation->m_MaxSize;
                }
                else
                {
                    _UsedBinnedTotal  += bufferAllocation->m_UsedSize;
                    _AllocBinnedTotal += bufferAllocation->m_MaxSize;
                    usedBinnedTotal   += bufferAllocation->m_UsedSize;
                    allocBinnedTotal  += bufferAllocation->m_MaxSize;
                }
            }
            
            if (poolSizeIndex == (int32)PoolSizes::SizesCount)  {
                MLOG(" Large Alloc Used/Max %d/%d %.2f%%", (int32)_UsedLargeTotal, (int32)_AllocLargeTotal, 100.0f * (float)_UsedLargeTotal / (float)_AllocLargeTotal);
            }
            else {
                MLOG(" Binned [%d] Alloc Used/Max %d/%d %.2f%%", m_PoolSizes[poolSizeIndex], (int32)_UsedBinnedTotal, (int32)_AllocBinnedTotal, 100.0f * (float)_UsedBinnedTotal / (float)_AllocBinnedTotal);
            }
        }
    }
    
    MLOG("::Totals::");
    MLOG("Large Alloc Used/Max %d/%d %.2f%%", (int32)usedLargeTotal, (int32)allocLargeTotal, 100.0f * allocLargeTotal > 0 ? (float)usedLargeTotal / (float)allocLargeTotal : 0.0f);
    MLOG("Binned Alloc Used/Max %d/%d %.2f%%", (int32)usedBinnedTotal, (int32)allocBinnedTotal, allocBinnedTotal > 0 ? 100.0f * (float)usedBinnedTotal / (float)allocBinnedTotal : 0.0f);
}
#endif

void VulkanResourceHeapManager::ReleaseFreedResources(bool immediately)
{
    for (auto& freeAllocations : m_FreeBufferAllocations)
    {
        for (int32 index = 0; index < freeAllocations.size(); ++index)
        {
            VulkanSubBufferAllocator* bufferAllocation = freeAllocations[index];
            bufferAllocation->Destroy(m_VulkanDevice);
            m_VulkanDevice->GetMemoryManager().Free(bufferAllocation->m_DeviceMemoryAllocation);
            delete bufferAllocation;
        }
        freeAllocations.clear();
    }
}

void VulkanResourceHeapManager::DestroyResourceAllocations()
{
    ReleaseFreedResources(true);
    for (auto& usedAllocations : m_UsedBufferAllocations)
    {
        for (int32 index = (int32)usedAllocations.size() - 1; index >= 0; --index)
        {
            VulkanSubBufferAllocator* bufferAllocation = usedAllocations[index];
            if (!bufferAllocation->JoinFreeBlocks()) {
                MLOG("Suballocation(s) for Buffer %p were not released.", (void*)bufferAllocation->m_Buffer);
            }
            bufferAllocation->Destroy(m_VulkanDevice);
            m_VulkanDevice->GetMemoryManager().Free(bufferAllocation->m_DeviceMemoryAllocation);
            delete bufferAllocation;
        }
        usedAllocations.clear();
    }
    
    for (auto& freeAllocations : m_FreeBufferAllocations)
    {
        for (int32 index = 0; index < freeAllocations.size(); ++index)
        {
            VulkanSubBufferAllocator* bufferAllocation = freeAllocations[index];
            bufferAllocation->Destroy(m_VulkanDevice);
            m_VulkanDevice->GetMemoryManager().Free(bufferAllocation->m_DeviceMemoryAllocation);
            delete bufferAllocation;
        }
        freeAllocations.clear();
    }
}
