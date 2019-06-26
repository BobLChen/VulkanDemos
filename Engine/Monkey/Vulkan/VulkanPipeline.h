#pragma once

#include "Common/Common.h"
#include "HAL/ThreadSafeCounter.h"
#include "Utils/Crc.h"
#include "VulkanPlatform.h"
#include "VulkanResources.h"
#include "VulkanDescriptorInfo.h"

#include <memory>
#include <vector>

class VulkanDevice;
class Shader;

struct VulkanPipelineStateInfo
{
	VulkanPipelineStateInfo()
	{
		// input assembly
		ZeroVulkanStruct(inputAssemblyState, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		// rasterization
		ZeroVulkanStruct(rasterizationState, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
		rasterizationState.polygonMode			   = VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode				   = VK_CULL_MODE_NONE;
		rasterizationState.frontFace			   = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationState.depthClampEnable		   = VK_FALSE;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.depthBiasEnable		   = VK_FALSE;
		rasterizationState.lineWidth			   = 1.0f;

		// color blend
		std::memset(&colorBlendAttachmentState, 0, sizeof(VkPipelineColorBlendAttachmentState));
		colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachmentState.blendEnable    = VK_FALSE;

		// viewport and scissor
		ZeroVulkanStruct(viewportState, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);
		viewportState.viewportCount = 1;
		viewportState.scissorCount  = 1;

		// depth stencil
		ZeroVulkanStruct(depthStencilState, VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO);
		depthStencilState.sType					= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable		= VK_TRUE;
		depthStencilState.depthWriteEnable		= VK_TRUE;
		depthStencilState.depthCompareOp		= VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.back.failOp			= VK_STENCIL_OP_KEEP;
		depthStencilState.back.passOp			= VK_STENCIL_OP_KEEP;
		depthStencilState.back.compareOp		= VK_COMPARE_OP_ALWAYS;
		depthStencilState.stencilTestEnable		= VK_FALSE;
		depthStencilState.front					= depthStencilState.back;

		// multi sample
		ZeroVulkanStruct(multisampleState, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleState.pSampleMask          = nullptr;

		GenerateHash();
	}

	void GenerateHash()
	{
		hash = 0;
		hash = Crc::MemCrc32(&inputAssemblyState,		 sizeof(VkPipelineInputAssemblyStateCreateInfo), hash);
		hash = Crc::MemCrc32(&rasterizationState,		 sizeof(VkPipelineRasterizationStateCreateInfo), hash);
		hash = Crc::MemCrc32(&colorBlendAttachmentState, sizeof(VkPipelineColorBlendAttachmentState),	 hash);
		hash = Crc::MemCrc32(&viewportState,			 sizeof(VkPipelineViewportStateCreateInfo),		 hash);
		hash = Crc::MemCrc32(&depthStencilState,		 sizeof(VkPipelineDepthStencilStateCreateInfo),	 hash);
		hash = Crc::MemCrc32(&multisampleState,			 sizeof(VkPipelineMultisampleStateCreateInfo),	 hash);
	}

	uint32 hash;

	VkPipelineInputAssemblyStateCreateInfo	inputAssemblyState;
	VkPipelineRasterizationStateCreateInfo	rasterizationState;
	VkPipelineColorBlendAttachmentState		colorBlendAttachmentState;
	VkPipelineViewportStateCreateInfo		viewportState;
	VkPipelineDepthStencilStateCreateInfo	depthStencilState;
	VkPipelineMultisampleStateCreateInfo	multisampleState;
};

class VulkanPipeline
{
public:
	VulkanPipeline();

	virtual ~VulkanPipeline();

	inline VkPipeline GetHandle() const
	{
		return m_Pipeline;
	}

	inline const VulkanLayout& GetLayout() const
	{
		return *m_Layout;
	}

protected:
    friend class VulkanPipelineStateManager;
    
protected:
	VkPipeline     m_Pipeline;
	VulkanLayout*  m_Layout;
};

class VulkanGfxPipeline : public VulkanPipeline
{
public:
	VulkanGfxPipeline();

	~VulkanGfxPipeline();

	inline const VertexInputBindingInfo& GetVertexInputInfo() const
	{
		return m_Layout->GetVertexInputBindingInfo();
	}
    
protected:
    
};

class VulkanPipelineStateManager
{
public:
	
	VkDescriptorSetLayout GetDescriptorSetLayout(const VulkanDescriptorSetLayoutInfo* setLayoutInfo);

	VulkanPipelineStateManager();

	~VulkanPipelineStateManager();

	VulkanGfxPipeline* GetGfxPipeline(const VulkanPipelineStateInfo& pipelineStateInfo, std::shared_ptr<Shader> shader, const VertexInputDeclareInfo& inputInfo);
    
    VulkanGfxLayout* GetGfxLayout(std::shared_ptr<Shader> shader);
    
	void Init(VulkanDevice* device);

	void Destory();

private:

    VkPipeline GetVulkanGfxPipeline(const VulkanPipelineStateInfo& pipelineStateInfo, const VulkanGfxLayout* gfxLayout, std::shared_ptr<Shader> shader, const VertexInputDeclareInfo& inputInfo);

private:
    VulkanDevice*	m_VulkanDevice;
    VkPipelineCache m_PipelineCache;
    
    std::unordered_map<uint32, VulkanGfxLayout*>        m_GfxLayoutCache;
	std::unordered_map<uint32, VulkanGfxPipeline*>      m_GfxPipelineCache;
	std::unordered_map<uint32, VkDescriptorSetLayout>   m_DescriptorSetLayoutCache;
};
