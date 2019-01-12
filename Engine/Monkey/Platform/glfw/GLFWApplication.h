#pragma once

#include "Common/Common.h"
#include "Application.h"

struct GLFWwindow;

NS_MONKEY_BEGIN

class GLFWApplication : public Application
{
public:
	GLFWApplication(int width, int height, const std::string& title) :
		Application(width, height, title)
	{

	}
    
	virtual ~GLFWApplication();
    
protected:
    virtual bool InitVulkanSurface();
    virtual bool InitWindow();
    virtual void OnDestory();
	virtual void OnLoop();
	virtual void OnUpdate();
	virtual void OnKeyDown(int key);
	virtual void OnKeyUP(int key);
	virtual void OnMouseMove(int x, int y);
	virtual void OnMouseDown(int x, int y, int type);
	virtual void OnMouseUP(int x, int y, int type);

protected:
	GLFWwindow* m_Window;
};

NS_MONKEY_END
