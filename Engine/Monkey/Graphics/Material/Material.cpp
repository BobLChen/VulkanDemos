#include "Material.h"
#include "Engine.h"
#include "Vulkan/VulkanDevice.h"
#include "Utils/Alignment.h"
#include <vector>
#include <cstring>
#include <unordered_map>

std::unordered_map<uint32, VkPipeline> G_PipelineCache;

Material::Material(std::shared_ptr<Shader> shader)
	: m_Hash(0)
	, m_InvalidPipeline(true)
	, m_Shader(shader)
	
{
	InitState();
}

Material::~Material()
{
	m_Shader = nullptr;
}

void Material::DestroyCache()
{
    VkDevice device = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    for (auto it = G_PipelineCache.begin(); it != G_PipelineCache.end(); ++it)
    {
        vkDestroyPipeline(device, it->second, VULKAN_CPU_ALLOCATOR);
    }
    G_PipelineCache.clear();
}

void Material::InitState()
{
	// input assembly
	ZeroVulkanStruct(m_InputAssemblyState, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
	m_InputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// rasterization
	ZeroVulkanStruct(m_RasterizationState, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
	m_RasterizationState.polygonMode			 = VK_POLYGON_MODE_FILL;
	m_RasterizationState.cullMode				 = VK_CULL_MODE_NONE;
	m_RasterizationState.frontFace				 = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	m_RasterizationState.depthClampEnable		 = VK_FALSE;
	m_RasterizationState.rasterizerDiscardEnable = VK_FALSE;
	m_RasterizationState.depthBiasEnable		 = VK_FALSE;
	m_RasterizationState.lineWidth				 = 1.0f;

	// color blend
	std::memset(&m_ColorBlendAttachmentState, 0, sizeof(VkPipelineColorBlendAttachmentState));
	m_ColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	m_ColorBlendAttachmentState.blendEnable    = VK_FALSE;

	// viewport and scissor
	ZeroVulkanStruct(m_ViewportState, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);
	m_ViewportState.viewportCount = 1;
	m_ViewportState.scissorCount  = 1;

	// depth stencil
	ZeroVulkanStruct(m_DepthStencilState, VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO);
	m_DepthStencilState.sType					= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	m_DepthStencilState.depthTestEnable			= VK_TRUE;
	m_DepthStencilState.depthWriteEnable		= VK_TRUE;
	m_DepthStencilState.depthCompareOp			= VK_COMPARE_OP_LESS_OR_EQUAL;
	m_DepthStencilState.depthBoundsTestEnable   = VK_FALSE;
	m_DepthStencilState.back.failOp				= VK_STENCIL_OP_KEEP;
	m_DepthStencilState.back.passOp				= VK_STENCIL_OP_KEEP;
	m_DepthStencilState.back.compareOp			= VK_COMPARE_OP_ALWAYS;
	m_DepthStencilState.stencilTestEnable		= VK_FALSE;
	m_DepthStencilState.front					= m_DepthStencilState.back;

	// multi sample
	ZeroVulkanStruct(m_MultisampleState, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
	m_MultisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	m_MultisampleState.pSampleMask          = nullptr;
}

VkPipeline Material::GetPipeline(const VertexInputDeclareInfo& inputDeclareInfo, const VertexInputBindingInfo& inputBindingInfo)
{
	VkPipeline pipelineResult = VK_NULL_HANDLE;
	std::shared_ptr<VulkanRHI> vulkanRHI = Engine::Get()->GetVulkanRHI();
	
    if (inputDeclareInfo.GetAttributes().size() != inputBindingInfo.GetAttributes().size())
    {
        MLOGE(
              "VertexBuffer(%d) not match VertexShader(%d).",
              (int32)inputDeclareInfo.GetAttributes().size(),
              (int32)inputBindingInfo.GetAttributes().size()
        );
        return pipelineResult;
    }
    // TODO:快速校验VertexData格式是否匹配VertexShader
    
	uint32 hash0 = inputDeclareInfo.GetHash();
	uint32 hash1 = inputBindingInfo.GetHash();

	// Pipeline发生修改，重新计算Pipeline的Key
	if (m_InvalidPipeline)
	{
		m_InvalidPipeline = false;
		m_MultisampleState.rasterizationSamples = vulkanRHI->GetSampleCount();
		m_Hash = Crc::StrCrc32((const char*)&m_InputAssemblyState, sizeof(VkPipelineInputAssemblyStateCreateInfo), m_Shader->GetHash());
		m_Hash = Crc::StrCrc32((const char*)&m_RasterizationState, sizeof(VkPipelineRasterizationStateCreateInfo), m_Hash);
		m_Hash = Crc::StrCrc32((const char*)&m_ColorBlendAttachmentState, sizeof(VkPipelineColorBlendAttachmentState), m_Hash);
		m_Hash = Crc::StrCrc32((const char*)&m_ViewportState, sizeof(VkPipelineViewportStateCreateInfo), m_Hash);
		m_Hash = Crc::StrCrc32((const char*)&m_DepthStencilState, sizeof(VkPipelineDepthStencilStateCreateInfo), m_Hash);
		m_Hash = Crc::StrCrc32((const char*)&m_MultisampleState, sizeof(VkPipelineMultisampleStateCreateInfo), m_Hash);
	}
    
	uint32 hash = Crc::MakeHashCode(hash0, hash1, m_Hash);
	auto it = G_PipelineCache.find(hash);

	// 缓存中不存在Pipeline，创建一个新的
	if (it == G_PipelineCache.end())
	{
		VkPipelineColorBlendStateCreateInfo colorBlendState;
		ZeroVulkanStruct(colorBlendState, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments    = &m_ColorBlendAttachmentState;

		std::vector<VkDynamicState> dynamicStateEnables;
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		ZeroVulkanStruct(dynamicState, VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO);
		dynamicState.dynamicStateCount = dynamicStateEnables.size();
		dynamicState.pDynamicStates    = dynamicStateEnables.data();
		
		const std::vector<VertexInputDeclareInfo::BindingDescription>& inVertexBindings = inputDeclareInfo.GetBindings();
		std::vector<VkVertexInputBindingDescription> vertexInputBindings(inVertexBindings.size());
		for (int32 i = 0; i < vertexInputBindings.size(); ++i)
		{
			vertexInputBindings[i].binding   = inVertexBindings[i].binding;
			vertexInputBindings[i].stride    = inVertexBindings[i].stride;
			vertexInputBindings[i].inputRate = inVertexBindings[i].inputRate;
		}

		const std::vector<VertexInputDeclareInfo::AttributeDescription>& inVertexDescInfos = inputDeclareInfo.GetAttributes();
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs(inVertexDescInfos.size());
		for (int32 i = 0; i < vertexInputAttributs.size(); ++i)
		{
			vertexInputAttributs[i].binding  = inVertexDescInfos[i].binding;
			vertexInputAttributs[i].location = inputBindingInfo.GetLocation(inVertexDescInfos[i].attribute);
			vertexInputAttributs[i].format   = inVertexDescInfos[i].format;
			vertexInputAttributs[i].offset   = inVertexDescInfos[i].offset;
		}
        
		VkPipelineVertexInputStateCreateInfo vertexInputState;
		ZeroVulkanStruct(vertexInputState, VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
		vertexInputState.vertexBindingDescriptionCount   = vertexInputBindings.size();
		vertexInputState.pVertexBindingDescriptions      = vertexInputBindings.data();
		vertexInputState.vertexAttributeDescriptionCount = vertexInputAttributs.size();
		vertexInputState.pVertexAttributeDescriptions    = vertexInputAttributs.data();

		VkGraphicsPipelineCreateInfo pipelineCreateInfo;
		ZeroVulkanStruct(pipelineCreateInfo, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
		pipelineCreateInfo.layout              = m_Shader->GetPipelineLayout();
		pipelineCreateInfo.renderPass          = vulkanRHI->GetRenderPass();
		pipelineCreateInfo.stageCount          = m_Shader->GetShaderStages().size();
		pipelineCreateInfo.pStages             = m_Shader->GetShaderStages().data();
		pipelineCreateInfo.pVertexInputState   = &vertexInputState;
		pipelineCreateInfo.pInputAssemblyState = &m_InputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &m_RasterizationState;
		pipelineCreateInfo.pColorBlendState    = &colorBlendState;
		pipelineCreateInfo.pMultisampleState   = &m_MultisampleState;
		pipelineCreateInfo.pViewportState      = &m_ViewportState;
		pipelineCreateInfo.pDepthStencilState  = &m_DepthStencilState;
		pipelineCreateInfo.pDynamicState       = &dynamicState;

		VkPipeline pipeline = VK_NULL_HANDLE;
		VERIFYVULKANRESULT(vkCreateGraphicsPipelines(vulkanRHI->GetDevice()->GetInstanceHandle(), vulkanRHI->GetPipelineCache(), 1, &pipelineCreateInfo, VULKAN_CPU_ALLOCATOR, &pipeline));
		pipelineResult = pipeline;
		G_PipelineCache.insert(std::make_pair(hash, pipeline));
        
        MLOG("Add GraphicsPipelines[%ud] to cache.", hash);
	}
	else 
	{
		pipelineResult = it->second;
	}
    
	return pipelineResult;
}
