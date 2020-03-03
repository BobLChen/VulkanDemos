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

	const std::string& GetAppPath() const;

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

	inline void SetPhysicalDeviceFeatures(VkPhysicalDeviceFeatures2* deviceFeatures)
	{
		m_PhysicalDeviceFeatures2 = deviceFeatures;
	}

protected:
    
    void ParseAppPath(const std::vector<std::string>& cmdLine);
    
protected:

	static Engine*						g_Instance;

	std::shared_ptr<VulkanRHI>			m_VulkanRHI;
	std::shared_ptr<Application>		m_Application;
    
    std::string                         m_AppTitle;
	std::string							m_AppPath;
	bool								m_IsRequestingExit;

	std::vector<const char*>			m_AppDeviceExtensions;
	std::vector<const char*>			m_AppInstanceExtensions;
	VkPhysicalDeviceFeatures2*			m_PhysicalDeviceFeatures2;
	
};
