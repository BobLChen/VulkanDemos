#pragma once

#include "Common/Common.h"
#include "Common/Log.h"
#include "Vulkan/VulkanRHI.h"
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

	const Engine& GetEngine()
	{
		return *m_Engine;
	}

	std::shared_ptr<VulkanRHI> GetVulkanRHI() const
	{
		return m_VulkanRHI;
	}

	std::shared_ptr<GenericApplication> GetApplication() const
	{
		return m_Application;
	}

	std::shared_ptr<GenericWindow> GetWindow() const
	{
		return m_Window;
	}

	int32 GetWidth() const
	{
		return m_Width;
	}

	int32 GetHeight() const
	{
		return m_Height;
	}

	const std::string& GetTitle()
	{
		return m_Title;
	}

	void Setup(Engine* engine, std::shared_ptr<VulkanRHI> vulkanRHI, std::shared_ptr<GenericApplication> application, std::shared_ptr<GenericWindow> window)
	{
		m_Engine      = engine;
		m_Window      = window;
		m_VulkanRHI   = vulkanRHI;
		m_Application = application;
	}
	
	virtual void PreInit() = 0;
	
	virtual void Init() = 0;

	virtual void Loop() = 0;

	virtual void Exist() = 0;
	
private:

	int32 								m_Width;
	int32 								m_Height;
	std::string 						m_Title;
	Engine* 							m_Engine;
	std::shared_ptr<VulkanRHI> 			m_VulkanRHI;
	std::shared_ptr<GenericApplication> m_Application;
	std::shared_ptr<GenericWindow> 		m_Window;
};
