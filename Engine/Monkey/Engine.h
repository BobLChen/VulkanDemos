#pragma once

#include "Common/Common.h"
#include "Vulkan/VulkanRHI.h"

#include <string>
#include <vector>

class GenericWindow;
class GenericApplication;
class Application;

class Engine
{
public:
	Engine();

	virtual ~Engine();

	int32 PreInit(const std::vector<std::string>& cmdLine, int32 width, int32 height, const char* title);

	int32 Init();

	void Exist();

	void Tick(float time, float delta);

	void PumpMessage();

	bool IsRequestingExit();

	void RequestExit(bool request);

	std::shared_ptr<VulkanRHI> GetVulkanRHI();

	std::shared_ptr<Application> GetApplication();

	std::shared_ptr<GenericApplication> GetPlatformApplication();

	std::shared_ptr<GenericWindow> GetPlatformWindow();

	std::shared_ptr<VulkanDevice> GetVulkanDevice();

	VkDevice GetDeviceHandle();

	const std::string& GetAssetsPath() const;

	static Engine* Get();
    
    const char* GetTitle() const
    {
        return m_AppTitle.c_str();
    }

	const std::vector<const char*>& GetAppDeviceExtensions() const
	{
		return m_AppDeviceExtensions;
	}

	const std::vector<const char*>& GetAppInstanceExtensions() const
	{
		return m_AppInstanceExtensions;
	}

	void AddAppDeviceExtensions(const char* name)
	{
		m_AppDeviceExtensions.push_back(name);
	}

	void AddAppInstanceExtensions(const char* name)
	{
		m_AppInstanceExtensions.push_back(name);
	}

protected:
    
    void ParseAssetsPath(const std::vector<std::string>& cmdLine);
    
protected:

	static Engine*						g_Instance;

	std::shared_ptr<VulkanRHI>			m_VulkanRHI;
	std::shared_ptr<Application>	m_SlateApplication;
    
    std::string                         m_AppTitle;
	std::string							m_AssetsPath;
	bool								m_IsRequestingExit;

	std::vector<const char*>			m_AppDeviceExtensions;
	std::vector<const char*>			m_AppInstanceExtensions;
};
