#include "VulkanDescriptorInfo.h"

void VulkanDescriptorSetsLayoutInfo::GenerateHash()
{
	const int32 layoutCount = m_SetLayouts.size();
	m_Hash = Crc::MemCrc32(&m_TypesUsageID, sizeof(uint32), layoutCount);

	for (int32 i = 0; i < layoutCount; ++i)
	{
		m_SetLayouts[i]->GenerateHash();
		m_Hash = Crc::MemCrc32(&(m_SetLayouts[i]->hash), sizeof(uint32), m_Hash);
	}
}

void VulkanDescriptorSetsLayoutInfo::AddDescriptor(uint32 set, uint32 binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, VkSampler* samplers = nullptr)
{
	VulkanDescriptorSetLayoutInfo* setLayout = nullptr;

	for (int32 i = 0; i < m_SetLayouts.size(); ++i)
	{
		if (m_SetLayouts[i]->set == set) {
			setLayout = m_SetLayouts[i];
			break;
		}
	}

	if (setLayout == nullptr) {
		setLayout = new VulkanDescriptorSetLayoutInfo();
		m_SetLayouts.push_back(setLayout);
	}

	for (int32 i = 0; i < setLayout->layoutBindings.size(); ++i)
	{
		VkDescriptorSetLayoutBinding& layoutBinding = setLayout->layoutBindings[i];
		if (layoutBinding.binding == binding && 
			layoutBinding.descriptorType == descriptorType && 
			layoutBinding.stageFlags == stageFlags && 
			layoutBinding.pImmutableSamplers == samplers
		)
		{
			layoutBinding.descriptorCount += 1;
			return;
		}
	}

	VkDescriptorSetLayoutBinding bindingInfo = {};
	bindingInfo.descriptorCount    = 1;
	bindingInfo.descriptorType     = descriptorType;
	bindingInfo.stageFlags         = stageFlags;
	bindingInfo.pImmutableSamplers = samplers;

	setLayout->layoutBindings.push_back(bindingInfo);
}