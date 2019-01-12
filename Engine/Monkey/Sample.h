#pragma once

#include "Common/Common.h"
#include <vulkan/vulkan.h>

NS_MONKEY_BEGIN

class Application;

class Sample
{
public:
    explicit Sample();
    virtual ~Sample();
	
	virtual bool OnInit() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnRender() = 0;
	virtual void OnDestory() = 0;
	virtual void OnDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg) = 0;
    
protected:
    friend class Application;
    
    void SetupApplication(Application* application);
    Application* GetApplication();
    
    Application* m_Application;
};

NS_MONKEY_END
