#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "VulkanQueue.h"
#include "VulkanMemory.h"
#include "VulkanDescriptorInfo.h"
#include "VulkanRHI.h"
#include "VulkanResources.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"

// VulkanCommandListContext
VulkanCommandListContext::VulkanCommandListContext(VulkanDevice* inDevice, std::shared_ptr<VulkanQueue> inQueue, VulkanCommandListContext* inImmediate)
	: m_RenderPass(VK_NULL_HANDLE)
	, m_Immediate(inImmediate)
	, m_Device(inDevice)
	, m_Queue(inQueue)
	, m_UniformUploader(nullptr)
	, m_CommandBufferManager(nullptr)
{
	m_UniformUploader = new VulkanUniformBufferUploader(inDevice);
	m_CommandBufferManager = new VulkanCommandBufferManager(inDevice, this);
	if (IsImmediate())
	{
		m_CommandBufferManager->GetActiveCmdBuffer();
		m_CommandBufferManager->SubmitActiveCmdBuffer();
		m_CommandBufferManager->NewActiveCommandBuffer();
	}
}

VulkanCommandListContext::~VulkanCommandListContext()
{
	delete m_UniformUploader;
	delete m_CommandBufferManager;
	
	VkDevice device = m_Device->GetInstanceHandle();
	if (m_RenderPass != VK_NULL_HANDLE) {
		vkDestroyRenderPass(device, m_RenderPass, VULKAN_CPU_ALLOCATOR);
		m_RenderPass = VK_NULL_HANDLE;
	}
}

VkRenderPass VulkanCommandListContext::GetRenderPass(const struct VulkanPipelineStateInfo& pipelineStateInfo, const VulkanGfxLayout* layout)
{
	if (m_RenderPass != VK_NULL_HANDLE) {
		return m_RenderPass;
	}

	VkDevice device = m_Device->GetInstanceHandle();
	
	std::vector<VkAttachmentDescription> attachments(2);
	// color attachment
	attachments[0].format		  = PixelFormatToVkFormat(PF_B8G8R8A8, false);
	attachments[0].samples		  = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp		  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	// depth stencil attachment
	attachments[1].format         = PixelFormatToVkFormat(PF_D24, false);
	attachments[1].samples        = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout	  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
	VkAttachmentReference colorReference = { };
	colorReference.attachment = 0;
	colorReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
	VkAttachmentReference depthReference = { };
	depthReference.attachment = 1;
	depthReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
	VkSubpassDescription subpassDescription = { };
	subpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount    = 1;
	subpassDescription.pColorAttachments       = &colorReference;
	subpassDescription.pDepthStencilAttachment = &depthReference;
	subpassDescription.pResolveAttachments     = nullptr;
	subpassDescription.inputAttachmentCount    = 0;
	subpassDescription.pInputAttachments       = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments    = nullptr;
		
	std::vector<VkSubpassDependency> dependencies(2);
	dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass      = 0;
	dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		
	dependencies[1].srcSubpass      = 0;
	dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		    
	VkRenderPassCreateInfo renderPassInfo;
	ZeroVulkanStruct(renderPassInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments    = attachments.data();
	renderPassInfo.subpassCount    = 1;
	renderPassInfo.pSubpasses      = &subpassDescription;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies   = dependencies.data();
	VERIFYVULKANRESULT(vkCreateRenderPass(device, &renderPassInfo, VULKAN_CPU_ALLOCATOR, &m_RenderPass));

	return m_RenderPass;
}

// VulkanCommandListContextImmediate
VulkanCommandListContextImmediate::VulkanCommandListContextImmediate(VulkanDevice* inDevice, std::shared_ptr<VulkanQueue> inQueue)
	: VulkanCommandListContext(inDevice, inQueue, nullptr)
{

}
