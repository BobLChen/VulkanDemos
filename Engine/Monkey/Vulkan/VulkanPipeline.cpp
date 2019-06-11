#include "Graphics/Shader/Shader.h"

#include "VulkanPipeline.h"
#include "VulkanDescriptorInfo.h"
#include "Engine.h"

VulkanPipeline::VulkanPipeline()
	: m_Pipeline(VK_NULL_HANDLE)
	, m_Layout(nullptr)
{

}

VulkanPipeline::~VulkanPipeline()
{
	if (m_Pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(Engine::Get()->GetDeviceHandle(), m_Pipeline, VULKAN_CPU_ALLOCATOR);
		m_Pipeline = VK_NULL_HANDLE;
	}
}

VulkanGfxPipeline::VulkanGfxPipeline()
{

}

VulkanGfxPipeline::~VulkanGfxPipeline()
{

}

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

VulkanPipeline* VulkanPipelineStateManager::GetVulkanPipeline(const VulkanPipelineStateInfo& pipelineStateInfo, std::shared_ptr<Shader> shader)
{
	uint32 key = Crc::MemCrc32(&(pipelineStateInfo.hash), sizeof(pipelineStateInfo.hash), shader->GetHash());
	
	auto it = m_PipelineCache.find(key);
	if (it != m_PipelineCache.end()) 
	{
		return it->second;
	}



	return nullptr;
}

VkDescriptorSetLayout VulkanPipelineStateManager::GetDescriptorSetLayout(const VulkanDescriptorSetLayoutInfo* setLayoutInfo)
{
	auto it = m_DescriptorSetLayoutCache.find(setLayoutInfo->hash);
	if (it != m_DescriptorSetLayoutCache.end()) {
		return it->second;
	}

	VkDescriptorSetLayout handle = VK_NULL_HANDLE;

	VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo;
	ZeroVulkanStruct(descriptorLayoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
	descriptorLayoutInfo.bindingCount = setLayoutInfo->layoutBindings.size();
	descriptorLayoutInfo.pBindings    = setLayoutInfo->layoutBindings.data();
	VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(Engine::Get()->GetDeviceHandle(), &descriptorLayoutInfo, VULKAN_CPU_ALLOCATOR, &handle));

	m_DescriptorSetLayoutCache.insert(std::make_pair(setLayoutInfo->hash, handle));

	return handle;
}