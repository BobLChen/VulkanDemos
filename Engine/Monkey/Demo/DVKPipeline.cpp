#include "DVKPipeline.h"

namespace vk_demo
{

	DVKGfxPipeline* DVKGfxPipeline::Create(
		std::shared_ptr<VulkanDevice> vulkanDevice,
		VkPipelineCache pipelineCache,
		DVKGfxPipelineInfo& pipelineInfo, 
		const std::vector<VkVertexInputBindingDescription>& inputBindings, 
		const std::vector<VkVertexInputAttributeDescription>& vertexInputAttributs,
		VkPipelineLayout pipelineLayout,
		VkRenderPass renderPass
	)
	{
		DVKGfxPipeline* pipeline    = new DVKGfxPipeline();
		pipeline->vulkanDevice   = vulkanDevice;
		pipeline->pipelineLayout = pipelineLayout;

		VkDevice device = vulkanDevice->GetInstanceHandle();

		VkPipelineVertexInputStateCreateInfo vertexInputState;
		ZeroVulkanStruct(vertexInputState, VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
		vertexInputState.vertexBindingDescriptionCount   = inputBindings.size();
		vertexInputState.pVertexBindingDescriptions      = inputBindings.data();
		vertexInputState.vertexAttributeDescriptionCount = vertexInputAttributs.size();
		vertexInputState.pVertexAttributeDescriptions    = vertexInputAttributs.data();

		VkPipelineColorBlendStateCreateInfo colorBlendState;
		ZeroVulkanStruct(colorBlendState, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
		colorBlendState.attachmentCount = pipelineInfo.colorAttachmentCount;
		colorBlendState.pAttachments    = pipelineInfo.blendAttachmentStates;
		
		VkPipelineViewportStateCreateInfo viewportState;
		ZeroVulkanStruct(viewportState, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);
		viewportState.viewportCount = 1;
		viewportState.scissorCount  = 1;
			
		std::vector<VkDynamicState> dynamicStateEnables;
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);

		VkPipelineDynamicStateCreateInfo dynamicState;
		ZeroVulkanStruct(dynamicState, VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO);
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates    = dynamicStateEnables.data();

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		if (pipelineInfo.shader) {
			shaderStages = pipelineInfo.shader->shaderStageCreateInfos;
		}
		else {
			pipelineInfo.FillShaderStages(shaderStages);
		}
		
		VkGraphicsPipelineCreateInfo pipelineCreateInfo;
		ZeroVulkanStruct(pipelineCreateInfo, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
		pipelineCreateInfo.layout 				= pipelineLayout;
		pipelineCreateInfo.renderPass 			= renderPass;
		pipelineCreateInfo.subpass              = pipelineInfo.subpass;
		pipelineCreateInfo.stageCount 			= shaderStages.size();
		pipelineCreateInfo.pStages 				= shaderStages.data();
		pipelineCreateInfo.pVertexInputState 	= &vertexInputState;
		pipelineCreateInfo.pInputAssemblyState 	= &(pipelineInfo.inputAssemblyState);
		pipelineCreateInfo.pRasterizationState 	= &(pipelineInfo.rasterizationState);
		pipelineCreateInfo.pColorBlendState 	= &colorBlendState;
		pipelineCreateInfo.pMultisampleState 	= &(pipelineInfo.multisampleState);
		pipelineCreateInfo.pViewportState 		= &viewportState;
		pipelineCreateInfo.pDepthStencilState 	= &(pipelineInfo.depthStencilState);
		pipelineCreateInfo.pDynamicState 		= &dynamicState;

		if (pipelineInfo.tessellationState.patchControlPoints != 0) {
			pipelineCreateInfo.pTessellationState = &(pipelineInfo.tessellationState);
		}

		VERIFYVULKANRESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, VULKAN_CPU_ALLOCATOR, &(pipeline->pipeline)));
		
		return pipeline;
	}

}
