#include "Common/Log.h"
#include "GLFWWindow.h"
#include "GLFWApplication.h"

#include <math.h>
#include <algorithm>

GLFWWindow::GLFWWindow(int32 width, int32 height, const char* title)
	: GenericWindow(width, height)
	, m_Title(title)
    , m_Window(nullptr)
    , m_WindowMode(WindowMode::Windowed)
	, m_Application(nullptr)
    , m_Visible(false)
	, m_AspectRatio(width * 1.0f / height)
	, m_DPIScaleFactor(1.0f)
	, m_Minimized(false)
    , m_Maximized(false)
{

}

GLFWWindow::~GLFWWindow()
{
	if (m_Window != nullptr)
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
		m_Window = nullptr;
	}
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
	
}

std::shared_ptr<GLFWWindow> GLFWWindow::Make(int32 width, int32 height, const char* title)
{
	return std::shared_ptr<GLFWWindow>(new GLFWWindow(width, height, title));
}

void GLFWWindow::Initialize(GLFWApplication* const application)
{
	m_Application = application;
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

	int32 frameWidth  = -1;
	int32 frameHeight = -1;
	glfwGetFramebufferSize(m_Window, &frameWidth, &frameHeight);
	glfwGetWindowPos(m_Window, &m_X, &m_Y);
	m_DPIScaleFactor = frameWidth * 1.0f / m_Width;
}

void GLFWWindow::ReshapeWindow(int32 newX, int32 newY, int32 newWidth, int32 newHeight)
{
	m_X = newX;
	m_Y = newY;
    m_Width  = newWidth;
    m_Height = newHeight;

	glfwSetWindowPos(m_Window, newX, newY);
	glfwSetWindowSize(m_Window, newWidth, newHeight);
}

bool GLFWWindow::GetFullScreenInfo(int32& x, int32& y, int32& width, int32& height) const
{

	return true;
}

void GLFWWindow::MoveWindowTo(int32 x, int32 y)
{
	m_X = x;
	m_Y = y;
	glfwSetWindowPos(m_Window, x, y);
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
	m_Minimized = false;
}

void GLFWWindow::Maximize()
{
	m_Maximized = true;
	glfwMaximizeWindow(m_Window);
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
	glfwShowWindow(m_Window);
}

void GLFWWindow::Hide()
{
	if (!m_Visible) {
		return;
	}
	m_Visible = false;
	glfwHideWindow(m_Window);
}

void GLFWWindow::SetWindowMode(WindowMode::Type newWindowMode)
{
	
}

bool GLFWWindow::IsMaximized() const
{
	return m_Maximized;
}

bool GLFWWindow::IsMinimized() const
{
	return m_Minimized;
}

bool GLFWWindow::IsVisible() const
{
	return m_Visible;
}

bool GLFWWindow::GetRestoredDimensions(int32& x, int32& y, int32& width, int32& height)
{
	
	return true;
}

void GLFWWindow::SetWindowFocus()
{
	glfwFocusWindow(m_Window);
}

void GLFWWindow::SetOpacity(const float opacity)
{
	glfwSetWindowOpacity(m_Window, opacity);
}

void GLFWWindow::Enable(bool enable)
{

}

bool GLFWWindow::IsPointInWindow(int32 x, int32 y) const
{
	if ((x > m_X && x < m_X + m_Width) && (y > m_Y && y < m_Y + m_Height)) {
		return true;
	}
	return false;
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
    return false;
}

void GLFWWindow::SetText(const char* const text)
{
	m_Title = text;
}
