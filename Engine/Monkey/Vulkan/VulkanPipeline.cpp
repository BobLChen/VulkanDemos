#include "Graphics/Shader/Shader.h"

#include "VulkanPipeline.h"
#include "VulkanDescriptorInfo.h"
#include "VulkanDevice.h"
#include "Engine.h"

// VulkanPipeline
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

void VulkanPipeline::CreateDescriptorWriteInfos()
{
	const VulkanDescriptorSetsLayout& setsLayout = m_Layout->GetDescriptorSetsLayout();

	// 遍历出VkDescriptorType的数量信息，创建出对应的writer。
	int32 numBufferInfos = 0;
	int32 numImageInfos  = 0;
	int32 numWrites      = 0;

	for (uint32 typeIndex = VK_DESCRIPTOR_TYPE_BEGIN_RANGE; typeIndex <= VK_DESCRIPTOR_TYPE_END_RANGE; ++typeIndex)
	{
		VkDescriptorType descriptorType =(VkDescriptorType)typeIndex;
		uint32 numTypesUsed = setsLayout.GetTypesUsed(descriptorType);
		if (numTypesUsed == 0)
		{
			continue;
		}

		numWrites += numTypesUsed;

		switch (descriptorType)
		{
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			numBufferInfos = numTypesUsed;
			break;
		
		default:
			break;
		}
	}
	// buffer
	if (numBufferInfos > 0) {
		m_DSWriteContainer.descriptorBufferInfo.resize(numBufferInfos);
	}
	// image
	// VkWriteDescriptorSet
	m_DSWriteContainer.descriptorWrites.resize(numWrites);

	// 创建出Writer
	const std::vector<VulkanDescriptorSetLayoutInfo*>& setLayoutInfos = setsLayout.GetLayouts();
	m_DSWriter.resize(setLayoutInfos.size());

	VkWriteDescriptorSet* currentWriteSet     = m_DSWriteContainer.descriptorWrites.data();
	VkDescriptorImageInfo* currentImageInfo   = m_DSWriteContainer.descriptorImageInfo.data();
	VkDescriptorBufferInfo* currentBufferInfo = m_DSWriteContainer.descriptorBufferInfo.data();
	
	for (int32 set = 0; set < setLayoutInfos.size(); ++set)
	{
		VulkanDescriptorSetLayoutInfo* setLayoutInfo = setLayoutInfos[set];
		m_DSWriter[set].SetupDescriptorWrites(setLayoutInfo->layoutBindings, currentWriteSet, currentImageInfo, currentBufferInfo);
	}
}

// VulkanGfxPipeline
VulkanGfxPipeline::VulkanGfxPipeline()
{

}

VulkanGfxPipeline::~VulkanGfxPipeline()
{

}

// VulkanPipelineStateManager
VulkanPipelineStateManager::VulkanPipelineStateManager()
	: m_VulkanDevice(nullptr)
    , m_PipelineCache(VK_NULL_HANDLE)
{
    
}

void VulkanPipelineStateManager::Init(VulkanDevice* device)
{
	m_VulkanDevice = device;
    
    VkPipelineCacheCreateInfo createInfo;
    ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO);
    VERIFYVULKANRESULT(vkCreatePipelineCache(device->GetInstanceHandle(), &createInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineCache));
}

void VulkanPipelineStateManager::Destory()
{
    VkDevice device = m_VulkanDevice->GetInstanceHandle();
	for (auto it = m_DescriptorSetLayoutCache.begin(); it != m_DescriptorSetLayoutCache.end(); ++it)
	{
		vkDestroyDescriptorSetLayout(device, it->second, VULKAN_CPU_ALLOCATOR);
	}
	m_DescriptorSetLayoutCache.clear();
    
    if (m_PipelineCache != VK_NULL_HANDLE)
    {
        vkDestroyPipelineCache(device, m_PipelineCache, VULKAN_CPU_ALLOCATOR);
    }
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
    
    m_GfxPipelineCache.insert(std::make_pair(key, pipeline));
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
        inputAttributeDesc.binding  = attrDesc.binding;
		inputAttributeDesc.location = location;
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
    
    VkPipeline vkPipeline = VK_NULL_HANDLE;
    const std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfo = shader->GetStageInfo();
    
	VkGraphicsPipelineCreateInfo pipelineCreateInfo;
	ZeroVulkanStruct(pipelineCreateInfo, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
	pipelineCreateInfo.layout 				= gfxLayout->GetPipelineLayout();
	pipelineCreateInfo.renderPass 			= Engine::Get()->GetVulkanRHI()->GetRenderPass();
    pipelineCreateInfo.stageCount 			= shaderStageInfo.size();
    pipelineCreateInfo.pStages 				= shaderStageInfo.data();
	pipelineCreateInfo.pVertexInputState 	= &vertexInputCreateInfo;
	pipelineCreateInfo.pInputAssemblyState 	= &pipelineStateInfo.inputAssemblyState;
	pipelineCreateInfo.pRasterizationState 	= &pipelineStateInfo.rasterizationState;
	pipelineCreateInfo.pColorBlendState 	= &colorBlendCreateInfo;
	pipelineCreateInfo.pMultisampleState 	= &pipelineStateInfo.multisampleState;
	pipelineCreateInfo.pViewportState 		= &pipelineStateInfo.viewportState;
	pipelineCreateInfo.pDepthStencilState 	= &pipelineStateInfo.depthStencilState;
	pipelineCreateInfo.pDynamicState 		= &dynamicStateCreateInfo;
	VERIFYVULKANRESULT(vkCreateGraphicsPipelines(m_VulkanDevice->GetInstanceHandle(), m_PipelineCache, 1, &pipelineCreateInfo, VULKAN_CPU_ALLOCATOR, &vkPipeline));
    
    return vkPipeline;
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
