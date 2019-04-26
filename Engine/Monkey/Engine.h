#pragma once

#include "Common/Common.h"
#include "Vulkan/VulkanRHI.h"
#include <string>
#include <vector>

class Engine
{
public:
	Engine();

	virtual ~Engine();

	int32 PreInit(const std::vector<std::string>& cmdLine, int32 width, int32 height, const char* title);

	int32 Init();

	void Exist();

	void Tick();

	void PumpMessage();

	bool IsRequestingExit();

	void RequestExit(bool request);

	std::shared_ptr<VulkanRHI> GetVulkanRHI();

	const std::string& GetAssetsPath() const;

	static Engine* Get();
    
    void SetDeltaTime(float deltaTime)
    {
        m_DeltaTime = deltaTime;
    }
    
    float GetDeltaTime()
    {
        return m_DeltaTime;
    }
    
protected:

	static Engine* g_Instance;
	
	std::shared_ptr<VulkanRHI> m_VulkanRHI;
	std::string m_AssetsPath;
	bool m_IsRequestingExit;
    
    float m_DeltaTime;
};
