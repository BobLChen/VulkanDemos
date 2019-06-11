#include "VulkanPipeline.h"
#include "VulkanDescriptorInfo.h"
#include "Engine.h"

VulkanPipelineStateManager::VulkanPipelineStateManager()
{

}

VulkanPipelineStateManager::~VulkanPipelineStateManager()
{
	VkDevice device = Engine::Get()->GetDeviceHandle();
	for (auto it = m_DescriptorSetLayoutCache.begin(); it != m_DescriptorSetLayoutCache.end(); ++it)
	{
		vkDestroyDescriptorSetLayout(device, it->second, VULKAN_CPU_ALLOCATOR);
	}
	m_DescriptorSetLayoutCache.clear();
}

VkDescriptorSetLayout VulkanPipelineStateManager::GetDescriptorSetLayout(const VulkanDescriptorSetLayoutInfo& setLayoutInfo)
{
	auto it = m_DescriptorSetLayoutCache.find(setLayoutInfo.hash);
	if (it != m_DescriptorSetLayoutCache.end()) {
		return it->second;
	}

	VkDescriptorSetLayout handle = VK_NULL_HANDLE;

	VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo;
	ZeroVulkanStruct(descriptorLayoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
	descriptorLayoutInfo.bindingCount = setLayoutInfo.layoutBindings.size();
	descriptorLayoutInfo.pBindings    = setLayoutInfo.layoutBindings.data();
	VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(Engine::Get()->GetDeviceHandle(), &descriptorLayoutInfo, VULKAN_CPU_ALLOCATOR, &handle));

	m_DescriptorSetLayoutCache.insert(std::make_pair(setLayoutInfo.hash, handle));

	return handle;
}