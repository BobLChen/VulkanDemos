#pragma once

#include "Engine.h"

#include "Common/Common.h"
#include "Math/Math.h"

#include "Vulkan/VulkanCommon.h"

#include "DVKShader.h"

#include <string>
#include <cstring>
#include <vector>
#include <memory>

namespace vk_demo
{

	struct DVKGfxPipelineInfo
	{
		VkPipelineInputAssemblyStateCreateInfo		inputAssemblyState;
		VkPipelineRasterizationStateCreateInfo		rasterizationState;
		VkPipelineColorBlendAttachmentState			blendAttachmentStates[8];
		VkPipelineDepthStencilStateCreateInfo		depthStencilState;
		VkPipelineMultisampleStateCreateInfo		multisampleState;
		VkPipelineTessellationStateCreateInfo		tessellationState;
        
		VkShaderModule	vertShaderModule = VK_NULL_HANDLE;
		VkShaderModule	fragShaderModule = VK_NULL_HANDLE;
		VkShaderModule	compShaderModule = VK_NULL_HANDLE;
		VkShaderModule	tescShaderModule = VK_NULL_HANDLE;
		VkShaderModule	teseShaderModule = VK_NULL_HANDLE;
		VkShaderModule	geomShaderModule = VK_NULL_HANDLE;

		DVKShader*		shader  = nullptr;
		int32			subpass = 0;
        int32           colorAttachmentCount = 1;

		DVKGfxPipelineInfo()
		{
			ZeroVulkanStruct(inputAssemblyState, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
			inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			
			ZeroVulkanStruct(rasterizationState, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
			rasterizationState.polygonMode 			   = VK_POLYGON_MODE_FILL;
			rasterizationState.cullMode                = VK_CULL_MODE_BACK_BIT;
			rasterizationState.frontFace               = VK_FRONT_FACE_CLOCKWISE;
			rasterizationState.depthClampEnable        = VK_FALSE;
			rasterizationState.rasterizerDiscardEnable = VK_FALSE;
			rasterizationState.depthBiasEnable         = VK_FALSE;
			rasterizationState.lineWidth 			   = 1.0f;
			
            for (int32 i = 0; i < 8; ++i)
            {
                blendAttachmentStates[i] = {};
                blendAttachmentStates[i].colorWriteMask = (
                    VK_COLOR_COMPONENT_R_BIT |
                    VK_COLOR_COMPONENT_G_BIT |
                    VK_COLOR_COMPONENT_B_BIT |
                    VK_COLOR_COMPONENT_A_BIT
                );
                blendAttachmentStates[i].blendEnable = VK_FALSE;
                blendAttachmentStates[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                blendAttachmentStates[i].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
                blendAttachmentStates[i].colorBlendOp        = VK_BLEND_OP_ADD;
                blendAttachmentStates[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                blendAttachmentStates[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
                blendAttachmentStates[i].alphaBlendOp        = VK_BLEND_OP_ADD;
            }
            
			ZeroVulkanStruct(depthStencilState, VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO);
			depthStencilState.depthTestEnable 		= VK_TRUE;
			depthStencilState.depthWriteEnable 		= VK_TRUE;
			depthStencilState.depthCompareOp		= VK_COMPARE_OP_LESS_OR_EQUAL;
			depthStencilState.depthBoundsTestEnable = VK_FALSE;
			depthStencilState.back.failOp 			= VK_STENCIL_OP_KEEP;
			depthStencilState.back.passOp 			= VK_STENCIL_OP_KEEP;
			depthStencilState.back.compareOp 		= VK_COMPARE_OP_ALWAYS;
			depthStencilState.stencilTestEnable 	= VK_TRUE;
			depthStencilState.front 				= depthStencilState.back;

			ZeroVulkanStruct(multisampleState, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
			multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampleState.pSampleMask 		  = nullptr;

			ZeroVulkanStruct(tessellationState, VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO);
			tessellationState.patchControlPoints = 0;
		}

		void FillShaderStages(std::vector<VkPipelineShaderStageCreateInfo>& shaderStages)
		{
			if (vertShaderModule != VK_NULL_HANDLE) 
			{
				VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
				ZeroVulkanStruct(shaderStageCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
				shaderStageCreateInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
				shaderStageCreateInfo.module = vertShaderModule;
				shaderStageCreateInfo.pName  = "main";
				shaderStages.push_back(shaderStageCreateInfo);
			}

			if (fragShaderModule != VK_NULL_HANDLE) 
			{
				VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
				ZeroVulkanStruct(shaderStageCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
				shaderStageCreateInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
				shaderStageCreateInfo.module = fragShaderModule;
				shaderStageCreateInfo.pName  = "main";
				shaderStages.push_back(shaderStageCreateInfo);
			}

			if (compShaderModule != VK_NULL_HANDLE) 
			{
				VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
				ZeroVulkanStruct(shaderStageCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
				shaderStageCreateInfo.stage  = VK_SHADER_STAGE_COMPUTE_BIT;
				shaderStageCreateInfo.module = compShaderModule;
				shaderStageCreateInfo.pName  = "main";
				shaderStages.push_back(shaderStageCreateInfo);
			}

			if (geomShaderModule != VK_NULL_HANDLE) 
			{
				VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
				ZeroVulkanStruct(shaderStageCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
				shaderStageCreateInfo.stage  = VK_SHADER_STAGE_GEOMETRY_BIT;
				shaderStageCreateInfo.module = geomShaderModule;
				shaderStageCreateInfo.pName  = "main";
				shaderStages.push_back(shaderStageCreateInfo);
			}

			if (tescShaderModule != VK_NULL_HANDLE) 
			{
				VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
				ZeroVulkanStruct(shaderStageCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
				shaderStageCreateInfo.stage  = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
				shaderStageCreateInfo.module = tescShaderModule;
				shaderStageCreateInfo.pName  = "main";
				shaderStages.push_back(shaderStageCreateInfo);
			}

			if (teseShaderModule != VK_NULL_HANDLE) 
			{
				VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
				ZeroVulkanStruct(shaderStageCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
				shaderStageCreateInfo.stage  = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
				shaderStageCreateInfo.module = teseShaderModule;
				shaderStageCreateInfo.pName  = "main";
				shaderStages.push_back(shaderStageCreateInfo);
			}
		}

	};

	class DVKGfxPipeline
	{
	public:
		
		DVKGfxPipeline()
			: vulkanDevice(nullptr)
			, pipeline(VK_NULL_HANDLE)
		{

		}

		~DVKGfxPipeline()
		{
			VkDevice device = vulkanDevice->GetInstanceHandle();
			if (pipeline != VK_NULL_HANDLE) {
				vkDestroyPipeline(device, pipeline, VULKAN_CPU_ALLOCATOR);
			}
		}

		static DVKGfxPipeline* Create(
			std::shared_ptr<VulkanDevice> vulkanDevice,
			VkPipelineCache pipelineCache,
			DVKGfxPipelineInfo& pipelineInfo, 
			const std::vector<VkVertexInputBindingDescription>& inputBindings, 
			const std::vector<VkVertexInputAttributeDescription>& vertexInputAttributs,
			VkPipelineLayout pipelineLayout,
			VkRenderPass renderPass
		);

	public:
		
		typedef std::shared_ptr<VulkanDevice> VulkanDeviceRef;

		VulkanDeviceRef		vulkanDevice;
		VkPipeline			pipeline;
		VkPipelineLayout	pipelineLayout;
	};

};
