#pragma once

#include "Common/Common.h"
#include "HAL/ThreadSafeCounter.h"
#include "Utils/Crc.h"
#include "VulkanPlatform.h"

#include <memory>
#include <vector>

class ShaderModule;

class VertexInputBindingInfo
{
public:
    VertexInputBindingInfo()
    : m_Invalid(true)
    , m_Hash(0)
    {
        
    }
    
    FORCEINLINE int32 GetLocation(VertexAttribute attribute) const
    {
        for (int32 i = 0; i < m_Attributes.size(); ++i)
        {
            if (m_Attributes[i] == attribute)
            {
                return m_Locations[i];
            }
        }
        
        MLOGE("Can't found location, Attribute : %d", attribute);
        return -1;
    }
    
    FORCEINLINE uint32 GetHash() const
    {
        return m_Hash;
    }
    
    FORCEINLINE void AddBinding(VertexAttribute attribute, int32 location)
    {
        m_Invalid = true;
        m_Attributes.push_back(attribute);
        m_Locations.push_back(location);
    }
    
    FORCEINLINE int32 GetInputCount() const
    {
        return int32(m_Attributes.size());
    }
    
    FORCEINLINE void Clear()
    {
        m_Invalid = false;
        m_Hash    = 0;
        m_Attributes.clear();
        m_Locations.clear();
    }
    
    FORCEINLINE void GenerateHash()
    {
        if (m_Invalid)
        {
            m_Hash = Crc::MemCrc32(m_Attributes.data(), int32(m_Attributes.size() * sizeof(int32)), 0);
            m_Hash = Crc::MemCrc32(m_Locations.data(), int32(m_Locations.size() * sizeof(int32)), m_Hash);
            m_Invalid = false;
        }
    }
    
    FORCEINLINE const std::vector<VertexAttribute>& GetAttributes() const
    {
        return m_Attributes;
    }
    
protected:
    bool                         m_Invalid;
    uint32                       m_Hash;
    std::vector<VertexAttribute> m_Attributes;
    std::vector<int32>           m_Locations;
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

class VulkanDescriptorSetsLayoutInfo
{
public:
	VulkanDescriptorSetsLayoutInfo()
	{
		memset(m_LayoutTypes, 0, sizeof(m_LayoutTypes));
	}

	~VulkanDescriptorSetsLayoutInfo()
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

	inline const std::vector<VkDescriptorSetLayout>& GetHandles()
	{
		return m_LayoutHandles;
	}

	void CopyFrom(const VulkanDescriptorSetsLayoutInfo& info)
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
};

class VulkanLayout
{
public:
	VulkanLayout();

	virtual ~VulkanLayout();

	virtual bool IsGfxLayout() const = 0;

	inline const VulkanDescriptorSetsLayoutInfo& GetDescriptorSetsLayout() const
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

	FORCEINLINE const VertexInputBindingInfo& GetVertexInputBindingInfo() const
	{
		return m_VertexInputBindingInfo;
	}

	void ProcessBindingsForStage(std::shared_ptr<ShaderModule> shaderModule);

	void Compile();

protected:

	VkPipelineLayout               m_PipelineLayout;
	VertexInputBindingInfo         m_VertexInputBindingInfo;
	VulkanDescriptorSetsLayoutInfo m_SetsLayoutInfo;
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
