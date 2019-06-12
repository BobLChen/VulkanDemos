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

VulkanGfxLayout* VulkanPipelineStateManager::GetGfxLayout(std::shared_ptr<Shader> shader)
{
    const uint32 key = shader->GetHash();
    auto it = m_GfxLayoutCache.find(key);
    if (it != m_GfxLayoutCache.end()) {
        return it->second;
    }
    
    VulkanGfxLayout* layout = new VulkanGfxLayout();
    layout->ProcessBindingsForStage(shader->GetVertModule());
    layout->ProcessBindingsForStage(shader->GetCompModule());
    layout->ProcessBindingsForStage(shader->GetGeomModule());
    layout->ProcessBindingsForStage(shader->GetTescModule());
    layout->ProcessBindingsForStage(shader->GetTeseModule());
    layout->ProcessBindingsForStage(shader->GetFragModule());
    layout->Compile();
    
    m_GfxLayoutCache.insert(std::make_pair(key, layout));
    
    return layout;
}

VulkanGfxPipeline* VulkanPipelineStateManager::GetGfxPipeline(const VulkanPipelineStateInfo& pipelineStateInfo, std::shared_ptr<Shader> shader)
{
	uint32 key = Crc::MemCrc32(&(pipelineStateInfo.hash), sizeof(pipelineStateInfo.hash), shader->GetHash());
	
	auto it = m_GfxPipelineCache.find(key);
	if (it != m_GfxPipelineCache.end())
	{
		return it->second;
	}
    
    VulkanGfxLayout* layout = GetGfxLayout(shader);
    VulkanGfxPipeline* pipeline = new VulkanGfxPipeline();
    pipeline->m_Layout   = layout;
    pipeline->m_Pipeline = GetGfxPipeline(pipelineStateInfo, layout, shader);
    
	return pipeline;
}

VkPipeline VulkanPipelineStateManager::GetGfxPipeline(const VulkanPipelineStateInfo& pipelineStateInfo, const VulkanGfxLayout* gfxLayout, std::shared_ptr<Shader> shader)
{
    
    
    
    
    return VK_NULL_HANDLE;
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
