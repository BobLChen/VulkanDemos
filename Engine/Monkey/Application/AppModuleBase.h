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
		, m_Engine(nullptr)
		, m_VulkanRHI(nullptr)
		, m_Application(nullptr)
		, m_Window(nullptr)
	{
		
	}

	virtual ~AppModuleBase()
	{

	}

	inline const Engine& GetEngine() const
	{
		return *m_Engine;
	}

	inline std::shared_ptr<VulkanRHI> GetVulkanRHI() const
	{
		return m_VulkanRHI;
	}

	inline std::shared_ptr<GenericApplication> GetApplication() const
	{
		return m_Application;
	}

	inline std::shared_ptr<GenericWindow> GetWindow() const
	{
		return m_Window;
	}
    
    inline VkDevice GetDevice()
    {
        return m_VulkanRHI->GetDevice()->GetInstanceHandle();
    }
    
    inline int32 GetFrameWidth() const
    {
        return m_VulkanRHI->GetSwapChain()->GetWidth();
    }
    
    inline int32 GetFrameHeight() const
    {
        return m_VulkanRHI->GetSwapChain()->GetHeight();
    }
    
	inline int32 GetWidth() const
	{
		return m_Width;
	}
    
    inline void SetWidth(int32 width)
    {
        m_Width = width;
    }
    
	inline int32 GetHeight() const
	{
		return m_Height;
	}
    
    inline void SetHeight(int32 height)
    {
        m_Height = height;
    }
    
    inline int32 GetFrameCount()
    {
        return GetVulkanRHI()->GetSwapChain()->GetBackBufferCount();
    }
    
	inline const std::string& GetTitle()
	{
		return m_Title;
	}

	inline void Setup(std::shared_ptr<Engine> engine, std::shared_ptr<VulkanRHI> vulkanRHI, std::shared_ptr<GenericApplication> application, std::shared_ptr<GenericWindow> window)
	{
		m_Engine      = engine;
		m_Window      = window;
		m_VulkanRHI   = vulkanRHI;
		m_Application = application;
	}
	
	inline int AcquireImageIndex()
	{
		return GetVulkanRHI()->GetSwapChain()->AcquireImageIndex(&m_PresentComplete);
	}

	inline void WaitFences(int index)
	{
		VERIFYVULKANRESULT(vkWaitForFences(GetDevice(), 1, &(m_Fences[index]), VK_TRUE, MAX_uint64));
		VERIFYVULKANRESULT(vkResetFences(GetDevice(), 1, &(m_Fences[index])));
	}

	void Present(int index)
	{
		WaitFences(index);

		std::shared_ptr<VulkanSwapChain> swapChain   = GetVulkanRHI()->GetSwapChain();
		VkPipelineStageFlags waitStageMask 			 = GetVulkanRHI()->GetStageMask();
		std::shared_ptr<VulkanQueue> gfxQueue 		 = GetVulkanRHI()->GetDevice()->GetGraphicsQueue();
		std::shared_ptr<VulkanQueue> presentQueue    = GetVulkanRHI()->GetDevice()->GetPresentQueue();
		std::vector<VkCommandBuffer>& drawCmdBuffers = GetVulkanRHI()->GetCommandBuffers();
		
		VkSubmitInfo submitInfo = {};
		submitInfo.sType 				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitDstStageMask 	= &waitStageMask;
		submitInfo.pWaitSemaphores 		= &m_PresentComplete;
		submitInfo.waitSemaphoreCount 	= 1;
		submitInfo.pSignalSemaphores 	= &m_RenderComplete;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pCommandBuffers 		= &drawCmdBuffers[index];
		submitInfo.commandBufferCount 	= 1;
        
		VERIFYVULKANRESULT(vkQueueSubmit(gfxQueue->GetHandle(), 1, &submitInfo, m_Fences[index]));
		swapChain->Present(gfxQueue, presentQueue, &m_RenderComplete);
	}

	virtual void Prepare()
	{		
		// 创建fence
		VkFenceCreateInfo fenceCreateInfo;
		ZeroVulkanStruct(fenceCreateInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		m_Fences.resize(GetFrameCount());
		for (int32 i = 0; i < m_Fences.size(); ++i)
		{
			VERIFYVULKANRESULT(vkCreateFence(GetDevice(), &fenceCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Fences[i]));
		}

		// 创建Semaphore
		VkSemaphoreCreateInfo createInfo;
		ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
		vkCreateSemaphore(GetDevice(), &createInfo, VULKAN_CPU_ALLOCATOR, &m_RenderComplete);

		// frame and cmd
		m_DrawCmdBuffers = GetVulkanRHI()->GetCommandBuffers();
        m_FrameBuffers   = GetVulkanRHI()->GetFrameBuffers();

		// render pass
		m_RenderPass     = GetVulkanRHI()->GetRenderPass();

		// frame size
		m_FrameWidth     = GetFrameWidth();
		m_FrameHeight    = GetFrameHeight();
	}

	virtual void Release()
	{
		// 销毁fence
		for (int32 i = 0; i < m_Fences.size(); ++i)
		{
			vkDestroyFence(GetDevice(), m_Fences[i], VULKAN_CPU_ALLOCATOR);
		}
		// 销毁Semaphore
		vkDestroySemaphore(GetDevice(), m_RenderComplete, VULKAN_CPU_ALLOCATOR);
	}

	virtual bool PreInit() = 0;
	
	virtual bool Init() = 0;

	virtual void Loop(float time, float delta) = 0;

	virtual void Exist() = 0;

protected:

	std::vector<VkFence>				m_Fences;
	VkSemaphore 						m_RenderComplete;
	VkSemaphore							m_PresentComplete;
	VkRenderPass						m_RenderPass;
	std::vector<VkCommandBuffer>		m_DrawCmdBuffers;
    std::vector<VkFramebuffer>			m_FrameBuffers;

	int32								m_FrameWidth;
	int32								m_FrameHeight;

private:
	int32 								m_Width;
	int32 								m_Height;
	std::string 						m_Title;
	std::shared_ptr<Engine> 			m_Engine;
	std::shared_ptr<VulkanRHI> 			m_VulkanRHI;
	std::shared_ptr<GenericApplication> m_Application;
	std::shared_ptr<GenericWindow> 		m_Window;
};
