#include "Common/Log.h"
#include "Vulkan/VulkanPlatform.h"

#include "IOSWindow.h"
#include "IOSApplication.h"

#include <math.h>
#include <algorithm>

void* g_IOSView = nullptr;
void* g_IOSViewController = nullptr;

static const char* G_ValidationLayersInstance[] =
{
	VK_KHR_SURFACE_EXTENSION_NAME,
    VK_MVK_IOS_SURFACE_EXTENSION_NAME,
	nullptr
};

IOSWindow::IOSWindow(int32 width, int32 height, const char* title)
	: GenericWindow(width, height)
	, m_Title(title)
	, m_WindowMode(WindowMode::Windowed)
	, m_Application(nullptr)
	, m_Visible(false)
	, m_AspectRatio(width * 1.0f / height)
	, m_DPIScaleFactor(1.0f)
	, m_Minimized(false)
	, m_Maximized(false)
{
    
}

IOSWindow::~IOSWindow()
{

}

float IOSWindow::GetDPIScaleFactorAtPoint(float X, float Y)
{
	return 1.0f;
}

void IOSWindow::CreateVKSurface(VkInstance instance, VkSurfaceKHR* outSurface)
{
    VkIOSSurfaceCreateInfoMVK createInfo;
    ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK);
    createInfo.pView = g_IOSView;
    VERIFYVULKANRESULT(vkCreateIOSSurfaceMVK(instance, &createInfo, VULKAN_CPU_ALLOCATOR, outSurface));
}

const char** IOSWindow::GetRequiredInstanceExtensions(uint32_t* count)
{
	*count = 2;
	return G_ValidationLayersInstance;
}

void* IOSWindow::GetOSWindowHandle() const
{
	return nullptr;
}

float IOSWindow::GetAspectRatio() const
{
	return m_AspectRatio;
}

float IOSWindow::GetDPIScaleFactor() const
{
	return m_DPIScaleFactor;
}

void IOSWindow::SetDPIScaleFactor(float value)
{

}

std::shared_ptr<IOSWindow> IOSWindow::Make(int32 width, int32 height, const char* title)
{
	return std::shared_ptr<IOSWindow>(new IOSWindow(width, height, title));
}

void IOSWindow::Initialize(IOSApplication* const application)
{
    m_Application = application;
}

void IOSWindow::ReshapeWindow(int32 newX, int32 newY, int32 newWidth, int32 newHeight)
{
	m_X = newX;
	m_Y = newY;
	m_Width = newWidth;
	m_Height = newHeight;
}

bool IOSWindow::GetFullScreenInfo(int32& x, int32& y, int32& width, int32& height) const
{

	return true;
}

void IOSWindow::MoveWindowTo(int32 x, int32 y)
{
	m_X = x;
	m_Y = y;
}

void IOSWindow::BringToFront(bool force)
{

}

void IOSWindow::Destroy()
{
	
}

void IOSWindow::Minimize()
{
	m_Minimized = false;
}

void IOSWindow::Maximize()
{
	m_Maximized = true;
}

void IOSWindow::Restore()
{

}

void IOSWindow::Show()
{
	if (m_Visible) {
		return;
	}
	m_Visible = true;
}

void IOSWindow::Hide()
{
	if (!m_Visible) {
		return;
	}
	m_Visible = false;
}

void IOSWindow::SetWindowMode(WindowMode::Type newWindowMode)
{

}

bool IOSWindow::IsMaximized() const
{
	return m_Maximized;
}

bool IOSWindow::IsMinimized() const
{
	return m_Minimized;
}

bool IOSWindow::IsVisible() const
{
	return m_Visible;
}

bool IOSWindow::GetRestoredDimensions(int32& x, int32& y, int32& width, int32& height)
{

	return true;
}

void IOSWindow::SetWindowFocus()
{

}

void IOSWindow::SetOpacity(const float opacity)
{

}

void IOSWindow::Enable(bool enable)
{

}

bool IOSWindow::IsPointInWindow(int32 x, int32 y) const
{
	if ((x > m_X && x < m_X + m_Width) && (y > m_Y && y < m_Y + m_Height)) {
		return true;
	}
	return false;
}

int32 IOSWindow::GetWindowBorderSize() const
{
	return 0;
}

int32 IOSWindow::GetWindowTitleBarSize() const
{
	return 0;
}

bool IOSWindow::IsForegroundWindow() const
{
	return false;
}

void IOSWindow::SetText(const char* const text)
{
	m_Title = text;
}
