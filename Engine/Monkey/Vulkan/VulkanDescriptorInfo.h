#pragma once

#include "Common/Common.h"
#include "HAL/ThreadSafeCounter.h"
#include "Utils/Crc.h"
#include "VulkanPlatform.h"
#include "VulkanResources.h"

#include <memory>
#include <vector>

class ShaderModule;
class VulkanDevice;
class VulkanCommandListContext;

struct FVulkanDescriptorSetWriteContainer
{
    std::vector<VkDescriptorImageInfo>  descriptorImageInfo;
    std::vector<VkDescriptorBufferInfo> descriptorBufferInfo;
    std::vector<VkWriteDescriptorSet>   descriptorWrites;
    std::vector<uint8>                  bindingToDynamicOffsetMap;
};

struct VulkanDescriptorSetLayoutInfo
{
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
	uint32 set;
	uint32 hash;

	inline void GenerateHash()
	{
		hash = Crc::MemCrc32(layoutBindings.data(), sizeof(VkDescriptorSetLayoutBinding) * layoutBindings.size());
	}

	inline bool operator == (const VulkanDescriptorSetLayoutInfo& info)
	{
		if (info.hash != hash)
		{
			return false;
		}

		const int size = layoutBindings.size();
		if (info.layoutBindings.size() != size)
		{
			return false;
		}

		if (size == 0)
		{
			return true;
		}

		if (memcmp(info.layoutBindings.data(), layoutBindings.data(), size * sizeof(VkDescriptorSetLayoutBinding)) != 0)
		{
			return false;
		}

		return true;
	}

	inline bool operator != (const VulkanDescriptorSetLayoutInfo& info)
	{
		return !(*this == info);
	}
};

class VulkanDescriptorSetsLayout
{
public:
	VulkanDescriptorSetsLayout()
	{
		memset(m_LayoutTypes, 0, sizeof(m_LayoutTypes));
	}

	~VulkanDescriptorSetsLayout()
	{
		for (int32 i = 0; i < m_SetLayouts.size(); ++i) {
			VulkanDescriptorSetLayoutInfo* layout = m_SetLayouts[i];
			delete layout;
		}
		m_SetLayouts.clear();
	}

	inline uint32 GetTypesUsed(VkDescriptorType type) const
	{
		return m_LayoutTypes[type] > 0;
	}

	const std::vector<VulkanDescriptorSetLayoutInfo*>& GetLayouts() const
	{
		return m_SetLayouts;
	}

	uint32 GetHash() const
	{
		return m_Hash;
	}

	inline const uint32* GetLayoutTypes() const
	{
		return m_LayoutTypes;
	}

	inline const std::vector<VkDescriptorSetLayout>& GetHandles() const
	{
		return m_LayoutHandles;
	}

	inline const VkDescriptorSetAllocateInfo& GetAllocateInfo() const
	{
		return m_DescriptorSetAllocateInfo;
	}

	void CopyFrom(const VulkanDescriptorSetsLayout& info)
	{
		memcpy(m_LayoutTypes, info.m_LayoutTypes, sizeof(m_LayoutTypes));
		m_Hash = info.m_Hash;
		m_SetLayouts = info.m_SetLayouts;
	}

	void AddDescriptor(uint32 set, uint32 binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, VkSampler* samplers = nullptr);
	
	void Compile();

protected:
	uint32										m_Hash = 0;
	uint32										m_LayoutTypes[VK_DESCRIPTOR_TYPE_RANGE_SIZE];
	std::vector<VulkanDescriptorSetLayoutInfo*>	m_SetLayouts;
	std::vector<VkDescriptorSetLayout>			m_LayoutHandles;
	VkDescriptorSetAllocateInfo					m_DescriptorSetAllocateInfo;
};

class VulkanLayout
{
public:
	VulkanLayout();

	virtual ~VulkanLayout();

	virtual bool IsGfxLayout() const = 0;

	inline const VulkanDescriptorSetsLayout& GetDescriptorSetsLayout() const
	{
		return m_SetsLayoutInfo;
	}

	inline VkPipelineLayout GetPipelineLayout() const
	{
		return m_PipelineLayout;
	}

	inline bool HasDescriptors() const
	{
		return m_SetsLayoutInfo.GetLayouts().size() > 0;
	}

	inline uint32 GetLayoutHash() const
	{
		return m_SetsLayoutInfo.GetHash();
	}

	inline const VertexInputBindingInfo& GetVertexInputBindingInfo() const
	{
		return m_VertexInputBindingInfo;
	}

	void ProcessBindingsForStage(std::shared_ptr<ShaderModule> shaderModule);

	void Compile();

protected:

	VkPipelineLayout               m_PipelineLayout;
	VertexInputBindingInfo         m_VertexInputBindingInfo;
	VulkanDescriptorSetsLayout m_SetsLayoutInfo;
};

class VulkanGfxLayout : public VulkanLayout
{
public:
	VulkanGfxLayout();

	~VulkanGfxLayout();

	virtual bool IsGfxLayout() const final override
	{
		return true;
	}
};

class VulkanComputeLayout : public VulkanLayout
{
public:
	VulkanComputeLayout();

	~VulkanComputeLayout();

	virtual bool IsGfxLayout() const final override
	{
		return false;
	}
};

class VulkanDescriptorPool
{
public:
    VulkanDescriptorPool(VulkanDevice* inDevice, const VulkanDescriptorSetsLayout& layout, uint32 maxSetsAllocations);
    
    ~VulkanDescriptorPool();
    
    inline VkDescriptorPool GetHandle() const
    {
        return m_DescriptorPool;
    }
    
    inline bool CanAllocate(const VulkanDescriptorSetsLayout& inLayout) const
    {
        return m_MaxDescriptorSets > m_NumAllocatedDescriptorSets + inLayout.GetLayouts().size();
    }
    
    inline bool IsEmpty() const
    {
        return m_NumAllocatedDescriptorSets == 0;
    }
    
    inline uint32 GetNumAllocatedDescriptorSets() const
    {
        return m_NumAllocatedDescriptorSets;
    }
    
    void TrackAddUsage(const VulkanDescriptorSetsLayout& inLayout);
    
    void TrackRemoveUsage(const VulkanDescriptorSetsLayout& inLayout);
    
    void Reset();
    
    bool AllocateDescriptorSets(const VkDescriptorSetAllocateInfo& inDescriptorSetAllocateInfo, VkDescriptorSet* outSets);
    
private:
    VulkanDevice*       m_Device;
    
    uint32              m_MaxDescriptorSets;
    uint32              m_NumAllocatedDescriptorSets;
    uint32              m_PeakAllocatedDescriptorSets;
    
    VkDescriptorPool    m_DescriptorPool;
    
    const VulkanDescriptorSetsLayout& m_Layout;
    
private:
    friend class VulkanCommandListContext;
};

class VulkanTypedDescriptorPoolSet
{
	typedef std::vector<VulkanDescriptorPool*> PoolList;

public:

	bool AllocateDescriptorSets(const VulkanDescriptorSetsLayout& inLayout, VkDescriptorSet* outSets);

protected:
	friend class VulkanDescriptorPoolSetContainer;
	
	VulkanTypedDescriptorPoolSet(VulkanDevice* inDevice, const VulkanDescriptorSetsLayout& inLayout);

	~VulkanTypedDescriptorPoolSet();

	void Reset();

	VulkanDescriptorPool* GetFreePool(bool forceNewPool = false);

	VulkanDescriptorPool* PushNewPool();

	inline uint32 GetPoolsCount() const 
	{
		return m_PoolsCount;
	}

private:
	VulkanDevice*	m_Device;
	uint32			m_PoolsCount;
	PoolList		m_PoolsList;
	int32			m_CurrentPool;

	const VulkanDescriptorSetsLayout& m_Layout;
};

class VulkanDescriptorPoolSetContainer
{
public:
    VulkanDescriptorPoolSetContainer(VulkanDevice* inDevice)
        : m_VulkanDevice(inDevice)
        , m_Used(true)
    {
        
    }
    
    ~VulkanDescriptorPoolSetContainer();
    
    VulkanTypedDescriptorPoolSet* AcquireTypedPoolSet(const VulkanDescriptorSetsLayout& layout);
    
    void Reset();
    
    inline void SetUsed(bool inUsed)
    {
        m_Used = inUsed;
    }
    
    inline bool IsUnused() const
    {
        return !m_Used;
    }
    
private:
    VulkanDevice*   m_VulkanDevice;
    bool            m_Used;
    std::unordered_map<uint32, VulkanTypedDescriptorPoolSet*> m_TypedDescriptorPools;
};

class VulkanDescriptorPoolsManager
{
    typedef std::vector<VulkanDescriptorPoolSetContainer*> PoolSets;
    
public:
    
    VulkanDescriptorPoolsManager()
		: m_VulkanDevice(nullptr)
	{

	}
    
    virtual ~VulkanDescriptorPoolsManager()
	{

	}

	void Init(VulkanDevice* inDevice)
    {
        m_VulkanDevice = inDevice;
    }
    
    VulkanDescriptorPoolSetContainer& AcquirePoolSetContainer();
    
    void ReleasePoolSet(VulkanDescriptorPoolSetContainer& poolSet);
    
    void GC();
    
    void Destroy();
    
private:
    VulkanDevice*   m_VulkanDevice;
    PoolSets        m_PoolSets;
};

class VulkanDescriptorSetWriter
{
public:
	VulkanDescriptorSetWriter()
		: m_WriteDescriptorSet(nullptr)
		, m_NumWrites(0)
	{

	}

	const VkWriteDescriptorSet* GetWriteDescriptors() const
	{
		return m_WriteDescriptorSet;
	} 

	const uint32 GetNumWrites() const
	{
		return m_NumWrites;
	}

	void SetDescriptorSet(VkDescriptorSet descriptorSet)
	{
		for (uint32 i = 0; i < m_NumWrites; ++i)
		{
			m_WriteDescriptorSet[i].dstSet = descriptorSet;
		}
	}

	bool WriteDynamicUniformBuffer(uint32 descriptorIndex, const VulkanBufferSubAllocation& bufferAllocation, VkDeviceSize offset, VkDeviceSize range, uint32 dynamicOffset)
	{
		return WriteBuffer<VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC>(descriptorIndex, bufferAllocation, offset, range, dynamicOffset);
	} 

protected:
	
	uint32 SetupDescriptorWrites(const std::vector<VkDescriptorType>& types, VkWriteDescriptorSet* inWriteDescriptors, VkDescriptorImageInfo* inImageInfo, VkDescriptorBufferInfo* inBufferInfo);

	template <VkDescriptorType DescriptorType>
	bool WriteBuffer(uint32 descriptorIndex, const VulkanBufferSubAllocation& bufferAllocation, VkDeviceSize offset, VkDeviceSize range, uint32 dynamicOffset = 0)
	{
		VkDescriptorBufferInfo* bufferInfo = const_cast<VkDescriptorBufferInfo*>(m_WriteDescriptorSet[descriptorIndex].pBufferInfo);
		bufferInfo->buffer = bufferAllocation.GetHandle();
		bufferInfo->offset = offset;
		bufferInfo->range  = range;
		return true;
	}

protected:
	VkWriteDescriptorSet*		m_WriteDescriptorSet;
	std::vector<uint32>			m_DynamicOffsets;
	uint32						m_NumWrites;

};