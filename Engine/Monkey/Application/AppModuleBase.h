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
	{
		
	}

	virtual ~AppModuleBase()
	{

	}

	inline std::shared_ptr<Engine> GetEngine() const
	{
		return m_Engine;
	}

	inline std::shared_ptr<VulkanRHI> GetVulkanRHI() const
	{
		return m_Engine->GetVulkanRHI();
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

	inline void Setup(std::shared_ptr<Engine> engine)
	{
		m_Engine = engine;
	}
	
	virtual void Prepare()
	{		
		
	}

	virtual void Release()
	{
		
	}

	virtual bool PreInit() = 0;
	
	virtual bool Init() = 0;

	virtual void Loop(float time, float delta) = 0;

	virtual void Exist() = 0;

protected:


private:

	int32 								m_Width;
	int32 								m_Height;
	std::string 						m_Title;
	std::shared_ptr<Engine> 			m_Engine;
};
