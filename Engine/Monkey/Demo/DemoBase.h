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
    
    virtual void UpdateFPS(float time, float delta)
    {
        m_FrameCounter  += 1;
        m_LastFrameTime += delta;
        if (m_LastFrameTime >= 1.0f) {
            m_LastFPS = m_FrameCounter;
            m_FrameCounter  = 0;
            m_LastFrameTime = 0.0f;
        }
    }
    
	void Setup();

	void Prepare() override
	{
        AppModuleBase::Prepare();
		CreateFences();
		CreateCommandBuffers();
		CreatePipelineCache();
		CreateDefaultRes();
	}

	void Release() override
	{
        AppModuleBase::Release();
		DestroyDefaultRes();
		DestroyFences();
		DestroyCommandBuffers();
		DestroyPipelineCache();
	}

	void Present(int backBufferIndex);

	int32 AcquireBackbufferIndex();

	uint32 GetMemoryTypeFromProperties(uint32 typeBits, VkMemoryPropertyFlags properties);

private:

	void CreateDefaultRes();

	void DestroyDefaultRes();

	void CreateCommandBuffers();

	void DestroyCommandBuffers();

	void CreateFences();

	void DestroyFences();

	void DestroyPipelineCache();

	void CreatePipelineCache();
    
protected:

	typedef std::shared_ptr<VulkanSwapChain> VulkanSwapChainRef;
    
	VkDevice						m_Device;
	std::shared_ptr<VulkanDevice>	m_VulkanDevice;
	VkQueue							m_GfxQueue;
	VkQueue							m_PresentQueue;

	int32							m_FrameWidth;
	int32							m_FrameHeight;
    
	VkPipelineCache                 m_PipelineCache;
    
	std::vector<VkFence> 			m_Fences;
	VkSemaphore 					m_PresentComplete;
	VkSemaphore 					m_RenderComplete;

	VkCommandPool					m_CommandPool;
	VkCommandPool					m_ComputeCommandPool;
	std::vector<VkCommandBuffer>	m_CommandBuffers;
    
	VkPipelineStageFlags			m_WaitStageMask;
    
	VulkanSwapChainRef				m_SwapChain;
    
    int32                           m_FrameCounter = 0;
    float                           m_LastFrameTime = 0.0f;
    float                           m_LastFPS = 0.0f;
};
