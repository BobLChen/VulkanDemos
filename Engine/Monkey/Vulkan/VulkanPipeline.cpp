#include "Graphics/Shader/Shader.h"

#include "VulkanPipeline.h"
#include "VulkanDescriptorInfo.h"
#include "VulkanDevice.h"
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
	: m_VulkanDevice(nullptr)
{

}

void VulkanPipelineStateManager::Init(VulkanDevice* device)
{
	m_VulkanDevice = device;
}

void VulkanPipelineStateManager::Destory()
{
	VkDevice device = Engine::Get()->GetDeviceHandle();
	for (auto it = m_DescriptorSetLayoutCache.begin(); it != m_DescriptorSetLayoutCache.end(); ++it)
	{
		vkDestroyDescriptorSetLayout(device, it->second, VULKAN_CPU_ALLOCATOR);
	}
	m_DescriptorSetLayoutCache.clear();
}

VulkanPipelineStateManager::~VulkanPipelineStateManager()
{
	
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

VulkanGfxPipeline* VulkanPipelineStateManager::GetGfxPipeline(const VulkanPipelineStateInfo& pipelineStateInfo, std::shared_ptr<Shader> shader, const VertexInputDeclareInfo& inputInfo)
{
	uint32 key = Crc::MemCrc32(&(pipelineStateInfo.hash), sizeof(pipelineStateInfo.hash), shader->GetHash());
	key = Crc::MemCrc32(&key, sizeof(uint32), inputInfo.GetHash());
	
	auto it = m_GfxPipelineCache.find(key);
	if (it != m_GfxPipelineCache.end()) {
		return it->second;
	}
    
    VulkanGfxLayout* layout = GetGfxLayout(shader);
    VulkanGfxPipeline* pipeline = new VulkanGfxPipeline();
    pipeline->m_Layout   = layout;
    pipeline->m_Pipeline = GetVulkanGfxPipeline(pipelineStateInfo, layout, shader, inputInfo);
    
	return pipeline;
}

VkPipeline VulkanPipelineStateManager::GetVulkanGfxPipeline(const VulkanPipelineStateInfo& pipelineStateInfo, const VulkanGfxLayout* gfxLayout, std::shared_ptr<Shader> shader, const VertexInputDeclareInfo& inputInfo)
{
	const VertexInputBindingInfo& inputBindingInfo = gfxLayout->GetVertexInputBindingInfo();
	const std::vector<VertexAttribute>& attributes = inputBindingInfo.GetAttributes();
	 
	std::vector<VkVertexInputAttributeDescription> vertexInputAttributs;
	std::vector<VkVertexInputBindingDescription>   vertexInputBindings;

	for (int32 i = 0; i < attributes.size(); ++i)
	{
		VertexInputDeclareInfo::AttributeDescription attrDesc;
		VertexInputDeclareInfo::BindingDescription   bindDesc;

		VertexAttribute attribute = attributes[i];
		int32 location            = inputBindingInfo.GetLocation(attribute);
		
		if (!inputInfo.GetAttributeDescription(attribute, attrDesc)) 
		{
			MLOGE("Attribute not found in vertex streaming. %d", attribute);
			return VK_NULL_HANDLE;
		}

		if (!inputInfo.GetBindingDescription(attribute, bindDesc))
		{
			MLOGE("Attribute not found in vertex streaming. %d", attribute);
			return VK_NULL_HANDLE;
		}

		VkVertexInputAttributeDescription inputAttributeDesc = {};
		inputAttributeDesc.location = attrDesc.binding;
		inputAttributeDesc.binding  = location;
		inputAttributeDesc.format   = attrDesc.format;
		inputAttributeDesc.offset   = attrDesc.offset;
		vertexInputAttributs.push_back(inputAttributeDesc);

		bool found = false;
		for (int32 j = 0; j < vertexInputBindings.size(); ++j)
		{
			if (vertexInputBindings[j].binding   == bindDesc.binding && 
				vertexInputBindings[j].stride    == bindDesc.stride  &&
				vertexInputBindings[j].inputRate == bindDesc.inputRate
			)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			VkVertexInputBindingDescription inputBindingDesc = {};
			inputBindingDesc.binding   = bindDesc.binding;
			inputBindingDesc.stride    = bindDesc.stride;
			inputBindingDesc.inputRate = bindDesc.inputRate;
			vertexInputBindings.push_back(inputBindingDesc);
		}
	}
	
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
	ZeroVulkanStruct(vertexInputCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
	vertexInputCreateInfo.vertexBindingDescriptionCount   = vertexInputBindings.size();
	vertexInputCreateInfo.pVertexBindingDescriptions      = vertexInputBindings.data();
	vertexInputCreateInfo.vertexAttributeDescriptionCount = vertexInputAttributs.size();
	vertexInputCreateInfo.pVertexAttributeDescriptions    = vertexInputAttributs.data();

	VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo;
	ZeroVulkanStruct(colorBlendCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
	colorBlendCreateInfo.attachmentCount   = 1;
    colorBlendCreateInfo.pAttachments      = &(pipelineStateInfo.colorBlendAttachmentState);
    colorBlendCreateInfo.blendConstants[0] = 1.0f;
	colorBlendCreateInfo.blendConstants[1] = 1.0f;
	colorBlendCreateInfo.blendConstants[2] = 1.0f;
	colorBlendCreateInfo.blendConstants[3] = 1.0f;

	std::vector<VkDynamicState> dynamicStates;
	dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo;
	ZeroVulkanStruct(dynamicStateCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO);
	dynamicStateCreateInfo.dynamicStateCount = (uint32_t)dynamicStates.size();
	dynamicStateCreateInfo.pDynamicStates    = dynamicStates.data();

	std::vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos;
	std::vector<std::shared_ptr<ShaderModule>> shaderModules;
	shaderModules.push_back(shader->GetVertModule());
	shaderModules.push_back(shader->GetFragModule());
	shaderModules.push_back(shader->GetGeomModule());
	shaderModules.push_back(shader->GetTescModule());
	shaderModules.push_back(shader->GetTeseModule());
	shaderModules.push_back(shader->GetCompModule());
	for (int32 i = 0; i < shaderModules.size(); ++i)
	{
		std::shared_ptr<ShaderModule> shaderModule = shaderModules[i];
		if (shaderModule == nullptr) {
			continue;
		}
		VkPipelineShaderStageCreateInfo shaderCreateInfo;
		ZeroVulkanStruct(shaderCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
		shaderCreateInfo.stage  = shaderModule->GetStageFlags();
		shaderCreateInfo.module = shaderModule->GetHandle();
		shaderCreateInfo.pName  = "main";
		shaderCreateInfos.push_back(shaderCreateInfo);
	}

	VkGraphicsPipelineCreateInfo pipelineCreateInfo;
	ZeroVulkanStruct(pipelineCreateInfo, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
	pipelineCreateInfo.layout 				= gfxLayout->GetPipelineLayout();
	pipelineCreateInfo.renderPass 			= Engine::Get()->GetVulkanRHI()->GetRenderPass();
	pipelineCreateInfo.stageCount 			= (uint32_t)shaderCreateInfos.size();
	pipelineCreateInfo.pStages 				= shaderCreateInfos.data();
	pipelineCreateInfo.pVertexInputState 	= &sdasd;
	pipelineCreateInfo.pInputAssemblyState 	= &pipelineStateInfo.inputAssemblyState;
	pipelineCreateInfo.pRasterizationState 	= &pipelineStateInfo.rasterizationState;
	pipelineCreateInfo.pColorBlendState 	= &colorBlendCreateInfo;
	pipelineCreateInfo.pMultisampleState 	= &pipelineStateInfo.multisampleState;
	pipelineCreateInfo.pViewportState 		= &pipelineStateInfo.viewportState;
	pipelineCreateInfo.pDepthStencilState 	= &pipelineStateInfo.depthStencilState;
	pipelineCreateInfo.pDynamicState 		= &dynamicStateCreateInfo;
	VERIFYVULKANRESULT(vkCreateGraphicsPipelines(m_Device, m_VulkanRHI->GetPipelineCache(), 1, &pipelineCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Pipeline));
		

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
