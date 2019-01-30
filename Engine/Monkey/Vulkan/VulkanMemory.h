#pragma once

#include "Common/Common.h"
#include "HAL/ThreadSafeCounter.h"
#include "VulkanPlatform.h"

#include <memory>
#include <vector>

class VulkanDevice;
class VulkanFenceManager;
class VulkanDeviceMemoryManager;

class RefCount
{
public:
	RefCount()
	{

	}

	virtual ~RefCount()
	{
		if (m_Counter.GetValue() != 0) 
		{
			MLOG("ERROR:Ref > 0");
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
		if (newValue == 0)
		{
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

class VulkanDeviceChild
{
public:
	VulkanDeviceChild(VulkanDevice* device = nullptr)
		: m_Device(device)
	{

	}

	virtual ~VulkanDeviceChild()
	{

	}

	inline VulkanDevice* GetParent() const
	{
		return m_Device;
	}

	inline void SetParent(VulkanDevice* device)
	{
		m_Device = device;
	}

protected:
	VulkanDevice* m_Device;
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
		return !!m_MappedPointer;
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

	VkDeviceSize m_Size;
	VkDevice m_Device;
	VkDeviceMemory m_Handle;
	void* m_MappedPointer;
	uint32 m_MemoryTypeIndex;
	bool m_CanBeMapped;
	bool m_IsCoherent;
	bool m_IsCached;
	bool m_FreedBySystem;
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
        for (uint32 i = 0; i < m_MemoryProperties.memoryTypeCount && typeBits; i++)
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
    VulkanDevice* m_Device;
    VkDevice m_DeviceHandle;
    bool m_HasUnifiedMemory;
    uint32 m_NumAllocations;
    uint32 m_PeakNumAllocations;
    std::vector<HeapInfo> m_HeapInfos;
};

class VulkanFence
{
public:
	enum class State
	{
		NotReady,
		Signaled,
	};

	VulkanFence(VulkanDevice* device, VulkanFenceManager* owner, bool createSignaled);

	inline VkFence GetHandle() const
	{
		return m_VkFence;
	}

	inline bool IsSignaled() const
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

	VkFence m_VkFence;
	State m_State;
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

	inline bool IsFenceSignaled(VulkanFence* fence)
	{
		if (fence->IsSignaled())
		{
			return true;
		}
		return CheckFenceState(fence);
	}

protected:
	bool CheckFenceState(VulkanFence* fence);

	void DestoryFence(VulkanFence* fence);

	VulkanDevice* m_Device;
	std::vector<VulkanFence*> m_FreeFences;
	std::vector<VulkanFence*> m_UsedFences;
};

class VulkanSemaphore : public RefCount
{
public:
	VulkanSemaphore(VulkanDevice* device);

	virtual ~VulkanSemaphore();

	inline VkSemaphore GetHandle() const
	{
		return m_VkSemaphore;
	}

protected:
	VkSemaphore m_VkSemaphore;
	VulkanDevice* m_Device;
};
