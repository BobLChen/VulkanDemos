#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanSwapChain.h"
#include "Vulkan/VulkanDevice.h"

#include "GenericWindow.h"
#include "GenericApplication.h"
#include "Engine.h"

#include <string>

class AppModuleBase
{
public:
	AppModuleBase(int32 width, int32 height, const char* title)
		: m_Width(width)
		, m_Height(height)
		, m_Title(title)
		, m_DepthStencilImage(VK_NULL_HANDLE)
		, m_DepthStencilView(VK_NULL_HANDLE)
		, m_DepthStencilMemory(VK_NULL_HANDLE)
		, m_RenderPass(VK_NULL_HANDLE)
		, m_SampleCount(VK_SAMPLE_COUNT_1_BIT)
		, m_DepthFormat(PF_DepthStencil)
	{

	}

	virtual ~AppModuleBase()
	{

	}
    
	inline void SetSize(int32 width, int32 height)
	{
		m_Width  = width;
		m_Height = height;
	}

	inline std::shared_ptr<VulkanRHI> GetVulkanRHI() const
	{
		return Engine::Get()->GetVulkanRHI();
	}

	inline int32 GetWidth() const
	{
		return m_Width;
	}
    
	inline int32 GetHeight() const
	{
		return m_Height;
	}
    
	inline const std::string& GetTitle()
	{
		return m_Title;
	}
    
	virtual void Prepare()
	{		
		CreateDepthStencil();
		CreateRenderPass();
		CreateFrameBuffers();
	}

	virtual void Release()
	{
		DestroyFrameBuffers();
		DestoryRenderPass();
		DestoryDepthStencil();
	}

	virtual bool PreInit() = 0;
	
	virtual bool Init() = 0;

	virtual void Loop(float time, float delta) = 0;

	virtual void Exist() = 0;

protected:

	virtual void CreateFrameBuffers()
	{
		DestroyFrameBuffers();

		int32 fwidth    = GetVulkanRHI()->GetSwapChain()->GetWidth();
		int32 fheight   = GetVulkanRHI()->GetSwapChain()->GetHeight();
		VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();

		VkImageView attachments[2];
		attachments[1] = m_DepthStencilView;

		VkFramebufferCreateInfo frameBufferCreateInfo;
		ZeroVulkanStruct(frameBufferCreateInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
		frameBufferCreateInfo.renderPass      = m_RenderPass;
		frameBufferCreateInfo.attachmentCount = 2;
		frameBufferCreateInfo.pAttachments    = attachments;
		frameBufferCreateInfo.width			  = fwidth;
		frameBufferCreateInfo.height		  = fheight;
		frameBufferCreateInfo.layers		  = 1;

		const std::vector<VkImageView>& backbufferViews = GetVulkanRHI()->GetBackbufferViews();

		m_FrameBuffers.resize(backbufferViews.size());
		for (uint32 i = 0; i < m_FrameBuffers.size(); ++i) 
		{
			attachments[0] = backbufferViews[i];
			VERIFYVULKANRESULT(vkCreateFramebuffer(device, &frameBufferCreateInfo, VULKAN_CPU_ALLOCATOR, &m_FrameBuffers[i]));
		}
	}

	virtual void CreateDepthStencil()
	{
		DestoryDepthStencil();

		int32 fwidth    = GetVulkanRHI()->GetSwapChain()->GetWidth();
		int32 fheight   = GetVulkanRHI()->GetSwapChain()->GetHeight();
		VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();

		VkImageCreateInfo imageCreateInfo;
		ZeroVulkanStruct(imageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
		imageCreateInfo.imageType   = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format      = PixelFormatToVkFormat(m_DepthFormat, false);
		imageCreateInfo.extent      = { (uint32)fwidth, (uint32)fheight, 1 };
		imageCreateInfo.mipLevels   = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples     = m_SampleCount;
		imageCreateInfo.tiling      = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage       = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		imageCreateInfo.flags       = 0;
		VERIFYVULKANRESULT(vkCreateImage(device, &imageCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DepthStencilImage));
		
		VkMemoryRequirements memRequire;
		vkGetImageMemoryRequirements(device, m_DepthStencilImage, &memRequire);
		uint32 memoryTypeIndex = 0;
		VERIFYVULKANRESULT(GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memRequire.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex));
		
		VkMemoryAllocateInfo memAllocateInfo;
		ZeroVulkanStruct(memAllocateInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
		memAllocateInfo.allocationSize  = memRequire.size;
		memAllocateInfo.memoryTypeIndex = memoryTypeIndex;
		vkAllocateMemory(device, &memAllocateInfo, VULKAN_CPU_ALLOCATOR, &m_DepthStencilMemory);
		vkBindImageMemory(device, m_DepthStencilImage, m_DepthStencilMemory, 0);

		VkImageViewCreateInfo imageViewCreateInfo;
		ZeroVulkanStruct(imageViewCreateInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format   = PixelFormatToVkFormat(m_DepthFormat, false);
		imageViewCreateInfo.flags    = 0;
		imageViewCreateInfo.image    = m_DepthStencilImage;
		imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
		imageViewCreateInfo.subresourceRange.levelCount     = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount     = 1;
		VERIFYVULKANRESULT(vkCreateImageView(device, &imageViewCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DepthStencilView));
	}

	virtual void CreateRenderPass()
	{
		DestoryRenderPass();

		VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
		PixelFormat pixelFormat = GetVulkanRHI()->GetPixelFormat();

		std::vector<VkAttachmentDescription> attachments(2);
		// color attachment
		attachments[0].format		  = PixelFormatToVkFormat(pixelFormat, false);
		attachments[0].samples		  = m_SampleCount;
		attachments[0].loadOp		  = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		// depth stencil attachment
		attachments[1].format         = PixelFormatToVkFormat(m_DepthFormat, false);
		attachments[1].samples        = m_SampleCount;
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
	}

	virtual void DestroyFrameBuffers()
	{
		VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
		for (int32 i = 0; i < m_FrameBuffers.size(); ++i) {
			vkDestroyFramebuffer(device, m_FrameBuffers[i], VULKAN_CPU_ALLOCATOR);
		}
		m_FrameBuffers.clear();
	}

	virtual void DestoryRenderPass()
	{
		VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
		if (m_RenderPass != VK_NULL_HANDLE) 
		{
			vkDestroyRenderPass(device, m_RenderPass, VULKAN_CPU_ALLOCATOR);
			m_RenderPass = VK_NULL_HANDLE;
		}
	}

	virtual void DestoryDepthStencil()
	{
		VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();

		if (m_DepthStencilMemory != VK_NULL_HANDLE) 
		{
			vkFreeMemory(device, m_DepthStencilMemory, VULKAN_CPU_ALLOCATOR);
			m_DepthStencilMemory = VK_NULL_HANDLE;
		}

		if (m_DepthStencilView != VK_NULL_HANDLE) 
		{
			vkDestroyImageView(device, m_DepthStencilView, VULKAN_CPU_ALLOCATOR);
			m_DepthStencilView = VK_NULL_HANDLE;
		}

		if (m_DepthStencilImage != VK_NULL_HANDLE) 
		{
			vkDestroyImage(device, m_DepthStencilImage, VULKAN_CPU_ALLOCATOR);
			m_DepthStencilImage = VK_NULL_HANDLE;
		}
	}

public:
	std::vector<const char*>	deviceExtensions;
	std::vector<const char*>	instanceExtensions;
	VkPhysicalDeviceFeatures2*	physicalDeviceFeatures = nullptr;

protected:

	int32 						m_Width;
	int32 						m_Height;
	std::string 				m_Title;
    
	std::vector<VkFramebuffer>	m_FrameBuffers;
    
    VkImage						m_DepthStencilImage;
    VkImageView					m_DepthStencilView;
    VkDeviceMemory				m_DepthStencilMemory;

	VkRenderPass				m_RenderPass;
	VkSampleCountFlagBits		m_SampleCount;

    PixelFormat					m_DepthFormat;
};
