#include "Material.h"
#include "Vulkan/VulkanDevice.h"
#include <vector>

Material::Material(std::shared_ptr<Shader> shader)
{
	InitState();
}

Material::~Material()
{

}

void Material::Bind(std::shared_ptr<VulkanRHI> vulkanRHI, const VkCommandBuffer& cmdBuffer)
{
	if (m_InvalidPipeline)
	{
		DestroyPipeline(vulkanRHI);
		CreatePipeline(vulkanRHI);
	}
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
}

void Material::CreatePipeline(std::shared_ptr<VulkanRHI> vulkanRHI)
{
	m_MultisampleState.rasterizationSamples = vulkanRHI->GetSampleCount();

	// 由ColorBlendAttach提供
	VkPipelineColorBlendStateCreateInfo colorBlendState;
	ZeroVulkanStruct(colorBlendState, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &m_ColorBlendAttachmentState;

	// 暂时硬编码
	std::vector<VkDynamicState> dynamicStateEnables;
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStateEnables.data();
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

	// Mesh提供
	std::vector<VkVertexInputBindingDescription> vertexInputBindings(1);
	vertexInputBindings[0].binding = 0;
	vertexInputBindings[0].stride = 24;
	vertexInputBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	// Mesh提供一部分，Shader提供一部分
	std::vector<VkVertexInputAttributeDescription> vertexInputAttributs(2);
	vertexInputAttributs[0].binding = 0;
	vertexInputAttributs[0].location = 0;
	vertexInputAttributs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributs[0].offset = 0;

	vertexInputAttributs[1].binding = 0;
	vertexInputAttributs[1].location = 1;
	vertexInputAttributs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributs[1].offset = 12;

	// input state
	VkPipelineVertexInputStateCreateInfo vertexInputState;
	ZeroVulkanStruct(vertexInputState, VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
	vertexInputState.vertexBindingDescriptionCount = vertexInputBindings.size();
	vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
	vertexInputState.vertexAttributeDescriptionCount = vertexInputAttributs.size();
	vertexInputState.pVertexAttributeDescriptions = vertexInputAttributs.data();

	// create info
	VkGraphicsPipelineCreateInfo pipelineCreateInfo;
	ZeroVulkanStruct(pipelineCreateInfo, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
	pipelineCreateInfo.layout = m_Shader->GetPipelineLayout();
	pipelineCreateInfo.renderPass = vulkanRHI->GetRenderPass();
	pipelineCreateInfo.stageCount = m_Shader->GetStages().size();
	pipelineCreateInfo.pStages = m_Shader->GetStages().data();
	pipelineCreateInfo.pVertexInputState = &vertexInputState;
	pipelineCreateInfo.pInputAssemblyState = &m_InputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &m_RasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &m_MultisampleState;
	pipelineCreateInfo.pViewportState = &m_ViewportState;
	pipelineCreateInfo.pDepthStencilState = &m_DepthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;

	VERIFYVULKANRESULT(vkCreateGraphicsPipelines(vulkanRHI->GetDevice()->GetInstanceHandle(), vulkanRHI->GetPipelineCache(), 1, &pipelineCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Pipeline));
}

void Material::DestroyPipeline(std::shared_ptr<VulkanRHI> vulkanRHI)
{
	if (m_Pipeline == VK_NULL_HANDLE)
	{
		return;
	}
	vkDestroyPipeline(vulkanRHI->GetDevice()->GetInstanceHandle(), m_Pipeline, VULKAN_CPU_ALLOCATOR);
	m_Pipeline = VK_NULL_HANDLE;
}

void Material::InitState()
{
	m_Pipeline = VK_NULL_HANDLE;

	// input assembly
	ZeroVulkanStruct(m_InputAssemblyState, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
	m_InputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// rasterization
	ZeroVulkanStruct(m_RasterizationState, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
	m_RasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	m_RasterizationState.cullMode = VK_CULL_MODE_NONE;
	m_RasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	m_RasterizationState.depthClampEnable = VK_FALSE;
	m_RasterizationState.rasterizerDiscardEnable = VK_FALSE;
	m_RasterizationState.depthBiasEnable = VK_FALSE;
	m_RasterizationState.lineWidth = 1.0f;

	// color blend
	std::memset(&m_ColorBlendAttachmentState, 0, sizeof(VkPipelineColorBlendAttachmentState));
	m_ColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	m_ColorBlendAttachmentState.blendEnable = VK_FALSE;

	// viewport and scissor
	ZeroVulkanStruct(m_ViewportState, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);
	m_ViewportState.viewportCount = 1;
	m_ViewportState.scissorCount = 1;

	// depth stencil
	ZeroVulkanStruct(m_DepthStencilState, VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO);
	m_DepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	m_DepthStencilState.depthTestEnable = VK_TRUE;
	m_DepthStencilState.depthWriteEnable = VK_TRUE;
	m_DepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	m_DepthStencilState.depthBoundsTestEnable = VK_FALSE;
	m_DepthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
	m_DepthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
	m_DepthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
	m_DepthStencilState.stencilTestEnable = VK_FALSE;
	m_DepthStencilState.front = m_DepthStencilState.back;

	// multi sample
	ZeroVulkanStruct(m_MultisampleState, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
	m_MultisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	m_MultisampleState.pSampleMask = nullptr;
}
