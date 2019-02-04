#include "GLFWWindow.h"
#include "Common/Log.h"
#include "Configuration/Platform.h"
#include "Application/Desktop/GLFWApplication.h"

#include <math.h>
#include <algorithm>

GLFWWindow::GLFWWindow(int32 width, int32 height, const char* title)
	: GenericWindow()
    , m_Window(nullptr)
    , m_WindowMode(WindowMode::Windowed)
	, m_MasterApplication(nullptr)
	, m_Width(width)
	, m_Height(height)
	, m_Title(title)
    , m_Visible(false)
	, m_AspectRatio(1.0f)
	, m_DPIScaleFactor(1.0f)
{
    
}

GLFWWindow::~GLFWWindow()
{
    
}

void GLFWWindow::CreateVKSurface(VkInstance instance, VkSurfaceKHR* outSurface)
{
	VkResult result = glfwCreateWindowSurface(instance, m_Window, VULKAN_CPU_ALLOCATOR, outSurface);
	if (result != VK_SUCCESS)
	{
		MLOG("Failed to create vksurface. %ud", result);
	}
}

const char** GLFWWindow::GetRequiredInstanceExtensions(uint32_t* count)
{
    return glfwGetRequiredInstanceExtensions(count);
}

void* GLFWWindow::GetOSWindowHandle() const
{
	return m_Window;
}

float GLFWWindow::GetAspectRatio() const
{
	return m_AspectRatio;
}

float GLFWWindow::GetDPIScaleFactor() const
{
	return m_DPIScaleFactor;
}

void GLFWWindow::SetDPIScaleFactor(float value)
{
	m_DPIScaleFactor = value;
}

std::shared_ptr<GLFWWindow> GLFWWindow::Make(int32 width, int32 height, const char* title)
{
	return std::shared_ptr<GLFWWindow>(new GLFWWindow(width, height, title));
}

void GLFWWindow::Initialize(GLFWApplication* const application)
{
	m_MasterApplication = application;
	if (glfwInit() != GLFW_TRUE) {
		printf("Failed init glfw.\n");
		return;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
	if (m_Window == nullptr) {
		printf("Failed glfwCreateWindow.\n");
		return;
	}
}

void GLFWWindow::ReshapeWindow(int32 newX, int32 newY, int32 newWidth, int32 newHeight)
{
    m_Width = newWidth;
    m_Height = newHeight;
}

bool GLFWWindow::GetFullScreenInfo(int32& x, int32& y, int32& width, int32& height) const
{

	return true;
}

void GLFWWindow::MoveWindowTo(int32 x, int32 y)
{
    
}

void GLFWWindow::BringToFront(bool force)
{
	
}

void GLFWWindow::Destroy()
{
	glfwDestroyWindow(m_Window);
	glfwTerminate();
	m_Window = nullptr;
}

void GLFWWindow::Minimize()
{
	
}

void GLFWWindow::Maximize()
{
	
}

void GLFWWindow::Restore()
{
	
}

void GLFWWindow::Show()
{
	if (m_Visible) {
		return;
	}
	m_Visible = true;
}

void GLFWWindow::Hide()
{
	if (!m_Visible) {
		return;
	}
	m_Visible = false;
}

void GLFWWindow::SetWindowMode(WindowMode::Type newWindowMode)
{
	
}

bool GLFWWindow::IsMaximized() const
{
	return true;
}

bool GLFWWindow::IsMinimized() const
{
	return true;
}

bool GLFWWindow::IsVisible() const
{
	return m_Visible;
}

bool GLFWWindow::GetRestoredDimensions(int32& x, int32& y, int32& width, int32& height)
{
	
	return true;
}

bool GLFWWindow::IsManualManageDPIChanges() const
{
	return true;
}

void GLFWWindow::SetManualManageDPIChanges(const bool manualDPIChanges)
{
	
}

void GLFWWindow::SetWindowFocus()
{
    
}

void GLFWWindow::SetOpacity(const float opacity)
{

}

void GLFWWindow::Enable(bool enable)
{

}

bool GLFWWindow::IsPointInWindow(int32 x, int32 y) const
{
	return true;
}

int32 GLFWWindow::GetWindowBorderSize() const
{
	return 0;
}

int32 GLFWWindow::GetWindowTitleBarSize() const
{
    return 0;
}

bool GLFWWindow::IsForegroundWindow() const
{
    return true;
}

void GLFWWindow::SetText(const char* const text)
{
	m_Title = text;
}
