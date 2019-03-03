#include "GLFWApplication.h"
#include "Common/Log.h"

#include <memory>
#include <map>
#include <string>

std::shared_ptr<GLFWApplication> G_CurrentPlatformApplication = nullptr;

void GLFWApplication::OnGLFWkeyCallback(GLFWwindow* window, int32 key, int32 scancode, int32 action, int32 mods)
{
	G_CurrentPlatformApplication->ProcessKey(key, scancode, action, mods);
}

GLFWApplication::GLFWApplication()
	: m_Window(nullptr)
{
    
}

GLFWApplication::~GLFWApplication()
{
	if (m_Window != nullptr)
	{
		MLOGE("Window not shutdown.");
	}
}

void GLFWApplication::ProcessKey(int32 key, int32 scancode, int32 action, int32 mods)
{
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_ESCAPE) {
			m_MessageHandler->OnRequestingExit();
		}
	}
}

void GLFWApplication::SetMessageHandler(const std::shared_ptr<GenericApplicationMessageHandler>& messageHandler)
{
	GenericApplication::SetMessageHandler(messageHandler);
}

void GLFWApplication::PumpMessages(const float deltaTime)
{
	glfwPollEvents();
}

void GLFWApplication::Tick(const float deltaTime)
{
	PumpMessages(deltaTime);
}

std::shared_ptr<GenericWindow> GLFWApplication::MakeWindow(int32 width, int32 height, const char* title)
{
	return GLFWWindow::Make(width, height, title);
}

std::shared_ptr<GenericWindow> GLFWApplication::GetWindow()
{
	return m_Window;
}

void GLFWApplication::InitializeWindow(const std::shared_ptr<GenericWindow>& window, const bool showImmediately)
{
	m_Window = std::dynamic_pointer_cast<GLFWWindow>(window);
	m_Window->Initialize(this);

	if (showImmediately) {
		m_Window->Show();
	}

	glfwSetKeyCallback(reinterpret_cast<GLFWwindow*>(m_Window->GetOSWindowHandle()), &GLFWApplication::OnGLFWkeyCallback);
}

void GLFWApplication::Destroy()
{
	if (m_Window != nullptr) {
		m_Window->Destroy();
		m_Window = nullptr;
	}
}

std::shared_ptr<GenericApplication> GenericApplication::Create()
{
	G_CurrentPlatformApplication = std::make_shared<GLFWApplication>();
	return G_CurrentPlatformApplication;
}

GenericApplication& GenericApplication::GetApplication()
{
	return *G_CurrentPlatformApplication;
}
