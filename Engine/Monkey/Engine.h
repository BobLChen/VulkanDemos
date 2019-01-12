#pragma once

#include "Common/Common.h"
#include "Vulkan/VulkanRHI.h"

class Engine
{
public:
	Engine();

	virtual ~Engine();

	int32 PreInit(const char* cmdline, int32 width, int32 height, const char* title);

	int32 Init();

	void Exist();

	void Tick();

	bool IsRequestingExit();

	void RequestExit(bool request);

	std::shared_ptr<VulkanRHI> GetVulkanRHI();

protected:
	std::shared_ptr<VulkanRHI> m_VulkanRHI;
	bool m_IsRequestingExit;
};
