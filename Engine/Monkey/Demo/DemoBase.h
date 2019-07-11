#pragma once

#include "Engine.h"

#include "Common/Common.h"
#include "Common/Log.h"

#include "Vulkan/VulkanCommon.h"

#include "Application/AppModuleBase.h"
#include "Application/GenericWindow.h"
#include "Application/GenericApplication.h"

#include <string>

class DemoBase : public AppModuleBase
{
public:
	DemoBase(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: AppModuleBase(width, height, title)
		, m_Device(VK_NULL_HANDLE)
		, m_VulkanDevice(nullptr)
		, m_GfxQueue(VK_NULL_HANDLE)
		, m_PresentQueue(VK_NULL_HANDLE)
		, m_FrameWidth(0)
		, m_FrameHeight(0)
		, m_SampleCount(VK_SAMPLE_COUNT_1_BIT)
		, m_PixelFormat(PF_R8G8B8A8)
		, m_DepthFormat(PF_D24)
		, m_DepthStencilImage(VK_NULL_HANDLE)
		, m_DepthStencilView(VK_NULL_HANDLE)
		, m_DepthStencilMemory(VK_NULL_HANDLE)
		, m_RenderPass(VK_NULL_HANDLE)
		, m_PipelineCache(VK_NULL_HANDLE)
		, m_PresentComplete(VK_NULL_HANDLE)
		, m_RenderComplete(VK_NULL_HANDLE)
		, m_CommandPool(VK_NULL_HANDLE)
		, m_WaitStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
		, m_SwapChain(VK_NULL_HANDLE)
	{

	}

	virtual ~DemoBase()
	{

	}

	VkDevice GetDeviceHandle()
	{
		return GetVulkanRHI()->GetDevice()->GetInstanceHandle();
	}

	virtual bool PreInit() override
	{

		return true;
	}
	
	virtual bool Init() override
	{

		return true;
	}

	virtual void Loop(float time, float delta) override
	{

	}

	virtual void Exist() override
	{

	}

	void Setup();

	void Prepare()
	{
		CreateFences();
		CreateCommandBuffers();
		CreatePipelineCache();
		CreateDepthStencil();
		CreateRenderPass();
		CreateFrameBuffers();
	}

	void Release()
	{
		DestroyFences();
		DestroyCommandBuffers();
		DestroyPipelineCache();
		DestroyFrameBuffers();
		DestoryRenderPass();
		DestoryDepthStencil();
	}

	void Present();

	uint32 GetMemoryTypeFromProperties(uint32 typeBits, VkMemoryPropertyFlags properties);

private:

	void CreateCommandBuffers();

	void DestroyCommandBuffers();

	void CreateFences();

	void DestroyFences();

	void DestroyPipelineCache();

	void CreatePipelineCache();

	void CreateDepthStencil();

	void CreateRenderPass();

	void CreateFrameBuffers();

	void DestroyFrameBuffers();

	void DestoryRenderPass();

	void DestoryDepthStencil();

protected:

	typedef std::shared_ptr<VulkanSwapChain> VulkanSwapChainRef;

	VkDevice						m_Device;
	std::shared_ptr<VulkanDevice>	m_VulkanDevice;
	VkQueue							m_GfxQueue;
	VkQueue							m_PresentQueue;

	int32							m_FrameWidth;
	int32							m_FrameHeight;

	VkSampleCountFlagBits			m_SampleCount;

	PixelFormat						m_PixelFormat;
    PixelFormat						m_DepthFormat;

	std::vector<VkFramebuffer>		m_FrameBuffers;
    
    VkImage							m_DepthStencilImage;
    VkImageView						m_DepthStencilView;
    VkDeviceMemory					m_DepthStencilMemory;

	VkRenderPass					m_RenderPass;

	VkPipelineCache                 m_PipelineCache;

	std::vector<VkFence> 			m_Fences;
	VkSemaphore 					m_PresentComplete;
	VkSemaphore 					m_RenderComplete;

	VkCommandPool					m_CommandPool;
	std::vector<VkCommandBuffer>	m_CommandBuffers;

	VkPipelineStageFlags			m_WaitStageMask;

	VulkanSwapChainRef				m_SwapChain;
};