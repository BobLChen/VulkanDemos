#pragma once

#include "Common/Common.h"
#include "Vulkan/VulkanRHI.h"

#include <string>
#include <vector>

class SlateApplication;

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

	std::shared_ptr<VulkanDevice> GetVulkanDevice();

	std::shared_ptr<SlateApplication> GetApplication();

	VkDevice GetDeviceHandle();

	const std::string& GetAssetsPath() const;

	static Engine* Get();
    
    const char* GetTitle() const
    {
        return m_AppTitle.c_str();
    }
    
protected:
    
    void ParseAssetsPath(const std::vector<std::string>& cmdLine);
    
protected:

	static Engine*						g_Instance;

	std::shared_ptr<VulkanRHI>			m_VulkanRHI;
	std::shared_ptr<SlateApplication>	m_SlateApplication;
    
    std::string                         m_AppTitle;
	std::string							m_AssetsPath;
	bool								m_IsRequestingExit;
};
