#include "GLFWApplication.h"
#include "Common/Log.h"

#include <memory>
#include <map>
#include <string>

std::shared_ptr<GLFWApplication> G_CurrentPlatformApplication = nullptr;

void GLFWApplication::OnGLFWkeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	G_CurrentPlatformApplication->ProcessKey(key, scancode, action, mods);
}

GLFWApplication::GLFWApplication()
	: m_MainWindow(nullptr)
{
    
}

GLFWApplication::~GLFWApplication()
{

}

void GLFWApplication::ProcessKey(int key, int scancode, int action, int mods)
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
	return m_MainWindow;
}

void GLFWApplication::InitializeWindow(const std::shared_ptr<GenericWindow>& window, const std::shared_ptr<GenericWindow>& parent, const bool showImmediately)
{
	m_MainWindow = std::dynamic_pointer_cast<GLFWWindow>(window);
	m_MainWindow->Initialize(this);

	if (showImmediately) {
		m_MainWindow->Show();
	}

	glfwSetKeyCallback(reinterpret_cast<GLFWwindow*>(m_MainWindow->GetOSWindowHandle()), &GLFWApplication::OnGLFWkeyCallback);
}

void GLFWApplication::Destroy()
{
	if (m_MainWindow != nullptr) {
		m_MainWindow->Destroy();
		m_MainWindow = nullptr;
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
