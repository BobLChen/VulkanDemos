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

class AppModeBase
{
public:
	AppModeBase(int32 width, int32 height, const char* title)
		: m_Width(width)
		, m_Height(height)
		, m_Title(title)
		, m_Engine(nullptr)
		, m_VulkanRHI(nullptr)
		, m_Application(nullptr)
		, m_Window(nullptr)
	{
		
	}

	virtual ~AppModeBase()
	{

	}

	FORCEINLINE const Engine& GetEngine() const
	{
		return *m_Engine;
	}

	FORCEINLINE std::shared_ptr<VulkanRHI> GetVulkanRHI() const
	{
		return m_VulkanRHI;
	}

	FORCEINLINE std::shared_ptr<GenericApplication> GetApplication() const
	{
		return m_Application;
	}

	FORCEINLINE std::shared_ptr<GenericWindow> GetWindow() const
	{
		return m_Window;
	}
    
    FORCEINLINE VkDevice GetDevice()
    {
        return m_VulkanRHI->GetDevice()->GetInstanceHandle();
    }
    
    FORCEINLINE int32 GetRealWidth() const
    {
        return m_VulkanRHI->GetSwapChain()->GetWidth();
    }
    
    FORCEINLINE int32 GetRealHeight() const
    {
        return m_VulkanRHI->GetSwapChain()->GetHeight();
    }
    
	FORCEINLINE int32 GetWidth() const
	{
		return m_Width;
	}
    
    FORCEINLINE void SetWidth(int32 width)
    {
        m_Width = width;
    }
    
	FORCEINLINE int32 GetHeight() const
	{
		return m_Height;
	}
    
    FORCEINLINE void SetHeight(int32 height)
    {
        m_Height = height;
    }
    
    FORCEINLINE int32 GetBufferCount()
    {
        return GetVulkanRHI()->GetSwapChain()->GetBackBufferCount();
    }
    
	FORCEINLINE const std::string& GetTitle()
	{
		return m_Title;
	}

	FORCEINLINE void Setup(Engine* engine, std::shared_ptr<VulkanRHI> vulkanRHI, std::shared_ptr<GenericApplication> application, std::shared_ptr<GenericWindow> window)
	{
		m_Engine      = engine;
		m_Window      = window;
		m_VulkanRHI   = vulkanRHI;
		m_Application = application;
	}
	
	FORCEINLINE void WaitFences(int index)
	{
		VERIFYVULKANRESULT(vkWaitForFences(GetDevice(), 1, &(m_Fences[index]), VK_TRUE, MAX_uint64));
		VERIFYVULKANRESULT(vkResetFences(GetDevice(), 1, &(m_Fences[index])));
	}

	virtual void Prepare()
	{
		m_Fences.resize(GetBufferCount());

		// 创建fence
		VkFenceCreateInfo fenceCreateInfo;
		ZeroVulkanStruct(fenceCreateInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (int32 i = 0; i < m_Fences.size(); ++i)
		{
			VERIFYVULKANRESULT(vkCreateFence(GetDevice(), &fenceCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Fences[i]));
		}

	}

	virtual void Release()
	{
		// 销毁fence
		for (int32 i = 0; i < m_Fences.size(); ++i)
		{
			vkDestroyFence(GetDevice(), m_Fences[i], VULKAN_CPU_ALLOCATOR);
		}
	}

	virtual void PreInit() = 0;
	
	virtual void Init() = 0;

	virtual void Loop() = 0;

	virtual void Exist() = 0;

protected:
	std::vector<VkFence>				m_Fences;
	
private:

	int32 								m_Width;
	int32 								m_Height;
	std::string 						m_Title;
	Engine* 							m_Engine;
	std::shared_ptr<VulkanRHI> 			m_VulkanRHI;
	std::shared_ptr<GenericApplication> m_Application;
	std::shared_ptr<GenericWindow> 		m_Window;

};
