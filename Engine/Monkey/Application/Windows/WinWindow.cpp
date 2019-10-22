#include "Common/Log.h"
#include "Vulkan/VulkanPlatform.h"

#include "WinWindow.h"
#include "WinApplication.h"

#include <math.h>
#include <algorithm>

HINSTANCE g_HInstance = NULL;

const char WinWindow::AppWindowClass[] = "MonkeyWindow";

static const char* G_ValidationLayersInstance[] =
{
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	nullptr
};

WinWindow::WinWindow(int32 width, int32 height, const char* title)
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

WinWindow::~WinWindow()
{
	if (m_Window != nullptr) {
		m_Window = nullptr;
	}
}

float WinWindow::GetDPIScaleFactorAtPoint(float X, float Y)
{
	float dpiScale = 1.0f;

	HDC Context = GetDC(nullptr);
	int32 DPI   = GetDeviceCaps(Context, LOGPIXELSX);
	dpiScale    = (float)DPI / 96.0f;
	ReleaseDC(nullptr, Context);

	return dpiScale;
}

void WinWindow::CreateVKSurface(VkInstance instance, VkSurfaceKHR* outSurface)
{
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
	ZeroVulkanStruct(surfaceCreateInfo, VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR);
	surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
	surfaceCreateInfo.hwnd      = (HWND)m_Window;
	VERIFYVULKANRESULT(vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, VULKAN_CPU_ALLOCATOR, outSurface));
}

const char** WinWindow::GetRequiredInstanceExtensions(uint32_t* count)
{
	*count = 2;
	return G_ValidationLayersInstance;
}

void* WinWindow::GetOSWindowHandle() const
{
	return m_Window;
}

float WinWindow::GetAspectRatio() const
{
	return m_AspectRatio;
}

float WinWindow::GetDPIScaleFactor() const
{
	return m_DPIScaleFactor;
}

void WinWindow::SetDPIScaleFactor(float value)
{

}

std::shared_ptr<WinWindow> WinWindow::Make(int32 width, int32 height, const char* title)
{
	return std::shared_ptr<WinWindow>(new WinWindow(width, height, title));
}

void WinWindow::Initialize(WinApplication* const application)
{
	RECT windowRect = { 0, 0, m_Width, m_Height };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	m_Window = CreateWindow(
		AppWindowClass,
		m_Title.c_str(),
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right  - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,        
		nullptr,        
		g_HInstance,
		nullptr
	);
}

void WinWindow::ReshapeWindow(int32 newX, int32 newY, int32 newWidth, int32 newHeight)
{
	m_X = newX;
	m_Y = newY;
	m_Width  = newWidth;
	m_Height = newHeight;
}

bool WinWindow::GetFullScreenInfo(int32& x, int32& y, int32& width, int32& height) const
{

	return true;
}

void WinWindow::MoveWindowTo(int32 x, int32 y)
{
	m_X = x;
	m_Y = y;
}

void WinWindow::BringToFront(bool force)
{

}

void WinWindow::Destroy()
{
	m_Window = nullptr;
}

void WinWindow::Minimize()
{
	m_Minimized = false;
}

void WinWindow::Maximize()
{
	m_Maximized = true;
}

void WinWindow::Restore()
{

}

void WinWindow::Show()
{
	if (m_Visible) {
		return;
	}
	m_Visible = true;

	ShowWindow(m_Window, SW_SHOW);
}

void WinWindow::Hide()
{
	if (!m_Visible) {
		return;
	}
	m_Visible = false;

	ShowWindow(m_Window, SW_HIDE);
}

void WinWindow::SetWindowMode(WindowMode::Type newWindowMode)
{

}

bool WinWindow::IsMaximized() const
{
	return m_Maximized;
}

bool WinWindow::IsMinimized() const
{
	return m_Minimized;
}

bool WinWindow::IsVisible() const
{
	return m_Visible;
}

bool WinWindow::GetRestoredDimensions(int32& x, int32& y, int32& width, int32& height)
{

	return true;
}

void WinWindow::SetWindowFocus()
{

}

void WinWindow::SetOpacity(const float opacity)
{

}

void WinWindow::Enable(bool enable)
{

}

bool WinWindow::IsPointInWindow(int32 x, int32 y) const
{
	return (x > m_X && x < m_X + m_Width) && (y > m_Y && y < m_Y + m_Height);
}

int32 WinWindow::GetWindowBorderSize() const
{
	return 0;
}

int32 WinWindow::GetWindowTitleBarSize() const
{
	return 0;
}

bool WinWindow::IsForegroundWindow() const
{
	return false;
}

void WinWindow::SetText(const char* const text)
{
	m_Title = text;
}