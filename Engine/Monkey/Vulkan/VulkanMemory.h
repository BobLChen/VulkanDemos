#pragma once

#include "Common/Common.h"
#include "HAL/ThreadSafeCounter.h"

#include "VulkanPlatform.h"

#include <memory>
#include <vector>

class VulkanDevice;
class VulkanDeviceMemoryManager;
class VulkanResourceHeap;
class VulkanResourceHeapPage;
class VulkanResourceHeapManager;
class VulkanBufferSubAllocation;
class VulkanSubBufferAllocator;
class VulkanSubResourceAllocator;

class RefCount
{
public:
	RefCount()
	{

	}

	virtual ~RefCount()
	{
		if (m_Counter.GetValue() != 0) {
			MLOGE("Ref > 0");
		}
	}
	
	inline int32 AddRef()
	{
		int32 newValue = m_Counter.Increment();
		return newValue;
	}

	inline int32 Release()
	{
		int32 newValue = m_Counter.Decrement();
		if (newValue == 0) {
			delete this;
		}
		return newValue;
	}

	inline int32 GetRefCount() const
	{
		int32 value = m_Counter.GetValue();
		return value;
	}

private:
	ThreadSafeCounter m_Counter;
};

struct VulkanRange
{
    uint32 offset;
    uint32 size;
    
    static void JoinConsecutiveRanges(std::vector<VulkanRange>& ranges);
    
    inline bool operator<(const VulkanRange& vulkanRange) const
    {
        return offset < vulkanRange.offset;
    }
};

class VulkanDeviceMemoryAllocation
{
public:
	VulkanDeviceMemoryAllocation();

	void* Map(VkDeviceSize size, VkDeviceSize offset);

	void Unmap();

	void FlushMappedMemory(VkDeviceSize offset, VkDeviceSize size);

	void InvalidateMappedMemory(VkDeviceSize offset, VkDeviceSize size);

	inline bool CanBeMapped() const
	{
		return m_CanBeMapped;
	}

	inline bool IsMapped() const
	{
		return m_MappedPointer != nullptr;
	}
	
	inline void* GetMappedPointer()
	{
		return m_MappedPointer;
	}

	inline bool IsCoherent() const
	{
		return m_IsCoherent;
	}

	inline VkDeviceMemory GetHandle() const
	{
		return m_Handle;
	}

	inline VkDeviceSize GetSize() const
	{
		return m_Size;
	}

	inline uint32 GetMemoryTypeIndex() const
	{
		return m_MemoryTypeIndex;
	}

protected:
	virtual ~VulkanDeviceMemoryAllocation();

    friend class VulkanDeviceMemoryManager;
protected:
	VkDeviceSize    m_Size;
	VkDevice        m_Device;
	VkDeviceMemory  m_Handle;
	void*           m_MappedPointer;
	uint32          m_MemoryTypeIndex;
	bool            m_CanBeMapped;
	bool            m_IsCoherent;
	bool            m_IsCached;
	bool            m_FreedBySystem;
};

class VulkanDeviceMemoryManager
{
public:
    VulkanDeviceMemoryManager();
    
    virtual ~VulkanDeviceMemoryManager();
    
    void Init(VulkanDevice* device);
    
    void Destory();
    
    bool SupportsMemoryType(VkMemoryPropertyFlags properties) const;
    
    VulkanDeviceMemoryAllocation* Alloc(bool canFail, VkDeviceSize allocationSize, uint32 memoryTypeIndex, void* dedicatedAllocateInfo, const char* file, uint32 line);
    
    void Free(VulkanDeviceMemoryAllocation*& allocation);
    
#if MONKEY_DEBUG
    void DumpMemory();
#endif
    
    uint64 GetTotalMemory(bool gpu) const;
    
    inline bool HasUnifiedMemory() const
    {
        return m_HasUnifiedMemory;
    }
    
    inline uint32 GetNumMemoryTypes() const
    {
        return m_MemoryProperties.memoryTypeCount;
    }
    
    inline VkResult GetMemoryTypeFromProperties(uint32 typeBits, VkMemoryPropertyFlags properties, uint32* outTypeIndex)
    {
        for (uint32 i = 0; i < m_MemoryProperties.memoryTypeCount && typeBits; ++i)
        {
            if ((typeBits & 1) == 1)
            {
                if ((m_MemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
                {
                    *outTypeIndex = i;
                    return VK_SUCCESS;
                }
            }
            typeBits >>= 1;
        }
        
        return VK_ERROR_FEATURE_NOT_PRESENT;
    }
    
    inline VkResult GetMemoryTypeFromPropertiesExcluding(uint32 typeBits, VkMemoryPropertyFlags properties, uint32 excludeTypeIndex, uint32* outTypeIndex)
    {
        for (uint32 i = 0; i < m_MemoryProperties.memoryTypeCount && typeBits; ++i)
        {
            if ((typeBits & 1) == 1)
            {
                if ((m_MemoryProperties.memoryTypes[i].propertyFlags & properties) == properties && excludeTypeIndex != i)
                {
                    *outTypeIndex = i;
                    return VK_SUCCESS;
                }
            }
            typeBits >>= 1;
        }
        
        return VK_ERROR_FEATURE_NOT_PRESENT;
    }
    
    inline const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const
    {
        return m_MemoryProperties;
    }
    
    inline VulkanDeviceMemoryAllocation* Alloc(bool canFail, VkDeviceSize allocationSize, uint32 memoryTypeBits, VkMemoryPropertyFlags memoryPropertyFlags, void* dedicatedAllocateInfo, const char* file, uint32 line)
    {
        uint32 memoryTypeIndex = ~0;
        VERIFYVULKANRESULT(this->GetMemoryTypeFromProperties(memoryTypeBits, memoryPropertyFlags, &memoryTypeIndex));
        return Alloc(canFail, allocationSize, memoryTypeIndex, dedicatedAllocateInfo, file, line);
    }
    
protected:
    struct HeapInfo
    {
        HeapInfo()
            : totalSize(0)
            , usedSize(0)
            , peakSize(0)
        {
            
        }
        
        VkDeviceSize totalSize;
        VkDeviceSize usedSize;
        VkDeviceSize peakSize;
        std::vector<VulkanDeviceMemoryAllocation*> allocations;
    };
    
    void SetupAndPrintMemInfo();
    
protected:

    VkPhysicalDeviceMemoryProperties m_MemoryProperties;
    VulkanDevice*                    m_Device;
    VkDevice                         m_DeviceHandle;
    bool                             m_HasUnifiedMemory;
    uint32                           m_NumAllocations;
    uint32                           m_PeakNumAllocations;
    std::vector<HeapInfo>            m_HeapInfos;
};

class VulkanResourceAllocation : public RefCount
{
public:
    VulkanResourceAllocation(VulkanResourceHeapPage* owner, VulkanDeviceMemoryAllocation* deviceMemoryAllocation, uint32 requestedSize, uint32 alignedOffset, uint32 allocationSize, uint32 allocationOffset, const char* file, uint32 line);
    
    virtual ~VulkanResourceAllocation();
    
    void BindBuffer(VulkanDevice* device, VkBuffer buffer);
    
    void BindImage(VulkanDevice* device, VkImage image);
    
    inline uint32 GetSize() const
    {
        return m_RequestedSize;
    }
    
    inline uint32 GetAllocationSize()
    {
        return m_AllocationSize;
    }
    
    inline uint32 GetOffset() const
    {
        return m_AlignedOffset;
    }
    
    inline VkDeviceMemory GetHandle() const
    {
        return m_DeviceMemoryAllocation->GetHandle();
    }
    
    inline void* GetMappedPointer()
    {
        return (uint8*)m_DeviceMemoryAllocation->GetMappedPointer() + m_AlignedOffset;
    }
    
    inline uint32 GetMemoryTypeIndex() const
    {
        return m_DeviceMemoryAllocation->GetMemoryTypeIndex();
    }
    
    inline void FlushMappedMemory()
    {
        m_DeviceMemoryAllocation->FlushMappedMemory(m_AllocationOffset, m_AllocationSize);
    }
    
    inline void InvalidateMappedMemory()
    {
        m_DeviceMemoryAllocation->InvalidateMappedMemory(m_AllocationOffset, m_AllocationSize);
    }

private:
    friend class VulkanResourceHeapPage;
    
private:
    VulkanResourceHeapPage*         m_Owner;
    uint32                          m_AllocationSize;
    uint32                          m_AllocationOffset;
    uint32                          m_RequestedSize;
    uint32                          m_AlignedOffset;
    VulkanDeviceMemoryAllocation*   m_DeviceMemoryAllocation;
};

class VulkanResourceHeapPage
{
public:
    VulkanResourceHeapPage(VulkanResourceHeap* owner, VulkanDeviceMemoryAllocation* deviceMemoryAllocation, uint32 id);
    
    virtual ~VulkanResourceHeapPage();
    
    void ReleaseAllocation(VulkanResourceAllocation* allocation);
    
    VulkanResourceAllocation* TryAllocate(uint32 size, uint32 alignment, const char* file, uint32 line);
    
    VulkanResourceAllocation* Allocate(uint32 size, uint32 alignment, const char* file, uint32 line)
    {
        VulkanResourceAllocation* resourceAllocation = TryAllocate(size, alignment, file, line);
        return resourceAllocation;
    }
    
    inline VulkanResourceHeap* GetOwner()
    {
        return m_Owner;
    }
    
    inline uint32 GetID() const
    {
        return m_ID;
    }
    
protected:
    bool JoinFreeBlocks();
    
    friend class VulkanResourceHeap;
protected:

    VulkanResourceHeap*                     m_Owner;
    VulkanDeviceMemoryAllocation*           m_DeviceMemoryAllocation;
    std::vector<VulkanResourceAllocation*>  m_ResourceAllocations;
    std::vector<VulkanRange>                m_FreeList;
    
    uint32                                  m_MaxSize;
    uint32                                  m_UsedSize;
    int32                                   m_PeakNumAllocations;
    uint32                                  m_FrameFreed;
    uint32                                  m_ID;
};

class VulkanResourceSubAllocation : public RefCount
{
public:
    VulkanResourceSubAllocation(uint32 requestedSize, uint32 alignedOffset, uint32 allocationSize, uint32 allocationOffset);
    
    virtual ~VulkanResourceSubAllocation();
    
    inline uint32 GetOffset() const
    {
        return m_AlignedOffset;
    }
    
    inline uint32 GetSize() const
    {
        return m_RequestedSize;
    }
    
protected:
    uint32 m_RequestedSize;
    uint32 m_AlignedOffset;
    uint32 m_AllocationSize;
    uint32 m_AllocationOffset;
};

class VulkanBufferSubAllocation : public VulkanResourceSubAllocation
{
public:
    VulkanBufferSubAllocation(VulkanSubBufferAllocator* owner, VkBuffer handle, uint32 requestedSize, uint32 alignedOffset, uint32 allocationSize, uint32 allocationOffset);
    
    virtual ~VulkanBufferSubAllocation();
    
    void* GetMappedPointer();
    
    inline VkBuffer GetHandle() const
    {
        return m_Handle;
    }
    
    inline VulkanSubBufferAllocator* GetBufferAllocator() const
    {
        return m_Owner;
    }
    
protected:
    friend class VulkanSubBufferAllocator;
    
protected:
    VulkanSubBufferAllocator*   m_Owner;
    VkBuffer                    m_Handle;
};

class VulkanSubResourceAllocator
{
public:
    VulkanSubResourceAllocator(VulkanResourceHeapManager* owner, VulkanDeviceMemoryAllocation* deviceMemoryAllocation, uint32 memoryTypeIndex, VkMemoryPropertyFlags memoryPropertyFlags, uint32 alignment);
    
    virtual ~VulkanSubResourceAllocator();
    
    virtual VulkanResourceSubAllocation* CreateSubAllocation(uint32 requestedSize, uint32 alignedOffset, uint32 allocationSize, uint32 allocationOffset) = 0;
    
    virtual void Destroy(VulkanDevice* device) = 0;
    
    VulkanResourceSubAllocation* TryAllocateNoLocking(uint32 size, uint32 alignment, const char* file, uint32 line);
    
    inline VulkanResourceSubAllocation* TryAllocateLocking(uint32 size, uint32 alignment, const char* file, uint32 line)
    {
        return TryAllocateNoLocking(size, alignment, file, line);
    }
    
    inline uint32 GetAlignment() const
    {
        return m_Alignment;
    }
    
    inline void* GetMappedPointer()
    {
        return m_DeviceMemoryAllocation->GetMappedPointer();
    }
    
protected:
    bool JoinFreeBlocks();
    
protected:
    VulkanResourceHeapManager*                  m_Owner;
    uint32                                      m_MemoryTypeIndex;
    VkMemoryPropertyFlags                       m_MemoryPropertyFlags;
    VulkanDeviceMemoryAllocation*               m_DeviceMemoryAllocation;
    uint32                                      m_MaxSize;
    uint32                                      m_Alignment;
    uint32                                      m_FrameFreed;
    int64                                       m_UsedSize;
    std::vector<VulkanRange>                    m_FreeList;
    std::vector<VulkanResourceSubAllocation*>   m_SubAllocations;
};

class VulkanSubBufferAllocator : public VulkanSubResourceAllocator
{
public:
    VulkanSubBufferAllocator(VulkanResourceHeapManager* owner, VulkanDeviceMemoryAllocation* deviceMemoryAllocation, uint32 memoryTypeIndex, VkMemoryPropertyFlags memoryPropertyFlags, uint32 alignment, VkBuffer buffer, VkBufferUsageFlags bufferUsageFlags, int32 poolSizeIndex);
    
    virtual ~VulkanSubBufferAllocator();
    
    virtual void Destroy(VulkanDevice* device) override;
    
    virtual VulkanResourceSubAllocation* CreateSubAllocation(uint32 requestedSize, uint32 alignedOffset, uint32 allocationSize, uint32 allocationOffset) override;
    
    void Release(VulkanBufferSubAllocation* subAllocation);
    
    inline VkBuffer GetHandle() const
    {
        return m_Buffer;
    }
    
protected:
    friend class VulkanResourceHeapManager;

protected:
    VkBufferUsageFlags  m_BufferUsageFlags;
    VkBuffer            m_Buffer;
    int32               m_PoolSizeIndex;
};

class VulkanResourceHeap
{
public:
    enum class Type
    {
        Image,
        Buffer,
    };
    
    VulkanResourceHeap(VulkanResourceHeapManager* owner, uint32 memoryTypeIndex, uint32 pageSize);
    
    virtual ~VulkanResourceHeap();
    
    void FreePage(VulkanResourceHeapPage* page);
    
    void ReleaseFreedPages(bool immediately);
    
    inline VulkanResourceHeapManager* GetOwner()
    {
        return m_Owner;
    }
    
    inline bool IsHostCachedSupported() const
    {
        return m_IsHostCachedSupported;
    }
    
    inline bool IsLazilyAllocatedSupported() const
    {
        return m_IsLazilyAllocatedSupported;
    }
    
    inline uint32 GetMemoryTypeIndex() const
    {
        return m_MemoryTypeIndex;
    }
    
#if MONKEY_DEBUG
    void DumpMemory();
#endif
    
protected:
    VulkanResourceAllocation* AllocateResource(Type type, uint32 size, uint32 alignment, bool mapAllocation, const char* file, uint32 line);
    
    friend class VulkanResourceHeapManager;
    
protected:
    VulkanResourceHeapManager*              m_Owner;
    uint32                                  m_MemoryTypeIndex;
    bool                                    m_IsHostCachedSupported;
    bool                                    m_IsLazilyAllocatedSupported;
    uint32                                  m_DefaultPageSize;
    uint32                                  m_PeakPageSize;
    uint64                                  m_UsedMemory;
    uint32                                  m_PageIDCounter;
    std::vector<VulkanResourceHeapPage*>    m_UsedBufferPages;
    std::vector<VulkanResourceHeapPage*>    m_UsedImagePages;
    std::vector<VulkanResourceHeapPage*>    m_FreePages;
};

class VulkanResourceHeapManager
{
public:
    VulkanResourceHeapManager(VulkanDevice* device);
    
    virtual ~VulkanResourceHeapManager();
    
    void Init();
    
    void Destory();
    
    VulkanBufferSubAllocation* AllocateBuffer(uint32 size, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags memoryPropertyFlags, const char* file, uint32 line);
    
    void ReleaseBuffer(VulkanSubBufferAllocator* bufferAllocator);
    
    void ReleaseFreedPages();
    
#if MONKEY_DEBUG
    void DumpMemory();
#endif
    
    VulkanResourceAllocation* AllocateImageMemory(const VkMemoryRequirements& memoryReqs, VkMemoryPropertyFlags memoryPropertyFlags, const char* file, uint32 line);
    
    VulkanResourceAllocation* AllocateBufferMemory(const VkMemoryRequirements& memoryReqs, VkMemoryPropertyFlags memoryPropertyFlags, const char* file, uint32 line);
    
    VulkanDevice* GetVulkanDevice()
    {
        return m_VulkanDevice;
    }

protected:
    void ReleaseFreedResources(bool immediately);
    
    void DestroyResourceAllocations();
    
protected:
    
    enum
    {
        BufferAllocationSize        = 1 * 1024 * 1024,
        UniformBufferAllocationSize = 2 * 1024 * 1024,
    };
    
    enum class PoolSizes : uint8
    {
        E32,
        E64,
        E128,
        E256,
        E512,
        E1k,
        E2k,
        E8k,
        E16k,
        SizesCount,
    };
    
    constexpr static uint32 m_PoolSizes[(int32)PoolSizes::SizesCount] =
    {
        32,
        64,
        128,
        256,
        512,
        1024,
        2048,
        8192,
        16 * 1024,
    };
    
    constexpr static uint32 m_BufferSizes[(int32)PoolSizes::SizesCount + 1] =
    {
        64 * 1024,
        64 * 1024,
        128 * 1024,
        128 * 1024,
        256 * 1024,
        256 * 1024,
        512 * 1024,
        512 * 1024,
        1024 * 1024,
        2 * 1024 * 1024,
    };
    
    PoolSizes GetPoolTypeForAlloc(uint32 size, uint32 alignment)
    {
        PoolSizes poolSize = PoolSizes::SizesCount;
        for (int32 i = 0; i < (int32)PoolSizes::SizesCount; ++i)
        {
            if (m_PoolSizes[i] >= size)
            {
                poolSize = (PoolSizes)i;
                break;
            }
        }
        return poolSize;
    }
    
protected:
    VulkanDevice*							m_VulkanDevice;
    VulkanDeviceMemoryManager*              m_DeviceMemoryManager;
    std::vector<VulkanResourceHeap*>        m_ResourceTypeHeaps;
    std::vector<VulkanSubBufferAllocator*>  m_UsedBufferAllocations[(int32)PoolSizes::SizesCount + 1];
    std::vector<VulkanSubBufferAllocator*>  m_FreeBufferAllocations[(int32)PoolSizes::SizesCount + 1];
};
