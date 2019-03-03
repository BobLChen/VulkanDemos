#pragma once

#include "Application/GenericApplication.h"
#include "Application/GenericWindow.h"

#include "GLFWWindow.h"

#include <memory>
#include <vector>

class GLFWApplication : public GenericApplication
{
public:
	GLFWApplication();

	virtual ~GLFWApplication();
    
	virtual void PumpMessages(const float timeDelta) override;

	virtual void Tick(const float timeDelta) override;

	virtual void Destroy() override;

	virtual std::shared_ptr<GenericWindow> MakeWindow(int32 width, int32 height, const char* title) override;

	virtual std::shared_ptr<GenericWindow> GetWindow() override;
    
	virtual void SetMessageHandler(const std::shared_ptr<GenericApplicationMessageHandler>& messageHandler) override;

	virtual void InitializeWindow(const std::shared_ptr<GenericWindow>& window, const bool showImmediately) override;

protected:
	void ProcessKey(int32 key, int32 scancode, int32 action, int32 mods);

	void static OnGLFWkeyCallback(GLFWwindow* window, int32 key, int32 scancode, int32 action, int32 mods);

private:
	std::shared_ptr<GLFWWindow> m_Window;
	
};
