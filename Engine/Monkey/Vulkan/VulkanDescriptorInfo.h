#pragma once

#include "Common/Common.h"
#include "HAL/ThreadSafeCounter.h"
#include "Utils/Crc.h"
#include "VulkanPlatform.h"

#include <memory>
#include <vector>

struct VulkanDescriptorSetLayoutInfo
{
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
	uint32 set;
	uint32 hash;

	inline void GenerateHash()
	{
		hash = Crc::MemCrc32(layoutBindings.data(), sizeof(VkDescriptorSetLayoutBinding) * layoutBindings.size());
		// set不管
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
		return m_LayoutTypes[type];
	}

	const std::vector<VulkanDescriptorSetLayoutInfo*>& GetLayouts() const
	{
		return m_SetLayouts;
	}

	uint32 GetHash() const
	{
		return m_Hash;
	}

	uint32 GetTypesUsageI() const
	{
		return m_TypesUsageID;
	}

	inline const uint32* GetLayoutTypes() const
	{
		return m_LayoutTypes;
	}

	void CopyFrom(const VulkanDescriptorSetsLayoutInfo& info)
	{
		memcpy(m_LayoutTypes, info.m_LayoutTypes, sizeof(m_LayoutTypes));
		m_Hash = info.m_Hash;
		m_TypesUsageID = info.m_TypesUsageID;
		m_SetLayouts = info.m_SetLayouts;
	}

	void AddDescriptor(uint32 set, uint32 binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, VkSampler* samplers = nullptr);
	
	void GenerateHash();

protected:
	uint32										m_LayoutTypes[VK_DESCRIPTOR_TYPE_RANGE_SIZE];
	std::vector<VulkanDescriptorSetLayoutInfo*>	m_SetLayouts;
	uint32										m_Hash = 0;
	uint32										m_TypesUsageID = ~0;
};
