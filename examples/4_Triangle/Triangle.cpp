#include "Common/Common.h"
#include "Common/Log.h"
#include "Application/AppModeBase.h"
#include "Vulkan/VulkanPlatform.h"
#include <vector>

class TriangleMode : public AppModeBase
{
public:
	TriangleMode(int width, int height, const char* title)
		: AppModeBase(width, height, title)
	{

	}

	virtual ~TriangleMode()
	{

	}

	virtual void PreInit() override
	{
		
	}

	virtual void Init() override
	{
		std::shared_ptr<VulkanRHI> vulkanRHI = GetVulkanRHI();

	}

	virtual void Loop() override
	{

	}

	virtual void Exist() override
	{

	}

public:
	std::vector<VkFence> m_Fences;
};

AppModeBase* CreateAppMode(const char* cmdLine, int32 cmdShow)
{
	return new TriangleMode(800, 600, "Triangle");
}