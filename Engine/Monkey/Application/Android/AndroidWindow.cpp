#include "Common/Log.h"
#include "Vulkan/VulkanPlatform.h"

#include "AndroidWindow.h"
#include "AndroidApplication.h"

#include <math.h>
#include <algorithm>

android_app* g_AndroidApp = nullptr;

static const char* G_ValidationLayersInstance[] =
{
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
	nullptr
};

AndroidWindow::AndroidWindow(int32 width, int32 height, const char* title)
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

AndroidWindow::~AndroidWindow()
{

}

float AndroidWindow::GetDPIScaleFactorAtPoint(float X, float Y)
{
	return 1.0f;
}

void AndroidWindow::CreateVKSurface(VkInstance instance, VkSurfaceKHR* outSurface)
{
	// // don't use cached window handle coming from VulkanViewport, as it could be gone by now
	// WindowHandle = FAndroidWindow::GetHardwareWindow();
	// if (WindowHandle == NULL)
	// {
	// 	// Sleep if the hardware window isn't currently available.
	// 	// The Window may not exist if the activity is pausing/resuming, in which case we make this thread wait
	// 	FPlatformMisc::LowLevelOutputDebugString(TEXT("Waiting for Native window in FVulkanAndroidPlatform::CreateSurface"));
	// 	while (WindowHandle == NULL)
	// 	{
	// 		FPlatformProcess::Sleep(0.001f);
	// 		WindowHandle = FAndroidWindow::GetHardwareWindow();
	// 	}
	// }

	// VkAndroidSurfaceCreateInfoKHR SurfaceCreateInfo;
	// ZeroVulkanStruct(SurfaceCreateInfo, VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR);
	// SurfaceCreateInfo.window = (ANativeWindow*)WindowHandle;

	// VERIFYVULKANRESULT(VulkanDynamicAPI::vkCreateAndroidSurfaceKHR(Instance, &SurfaceCreateInfo, VULKAN_CPU_ALLOCATOR, OutSurface));
}

const char** AndroidWindow::GetRequiredInstanceExtensions(uint32_t* count)
{
	*count = 2;
	return G_ValidationLayersInstance;
}

void* AndroidWindow::GetOSWindowHandle() const
{
	return nullptr;
}

float AndroidWindow::GetAspectRatio() const
{
	return m_AspectRatio;
}

float AndroidWindow::GetDPIScaleFactor() const
{
	return m_DPIScaleFactor;
}

void AndroidWindow::SetDPIScaleFactor(float value)
{

}

std::shared_ptr<AndroidWindow> AndroidWindow::Make(int32 width, int32 height, const char* title)
{
	return std::shared_ptr<AndroidWindow>(new AndroidWindow(width, height, title));
}


void AndroidWindow::Initialize(AndroidApplication* const application)
{
	
}

void AndroidWindow::ReshapeWindow(int32 newX, int32 newY, int32 newWidth, int32 newHeight)
{
	m_X = newX;
	m_Y = newY;
	m_Width = newWidth;
	m_Height = newHeight;
}

bool AndroidWindow::GetFullScreenInfo(int32& x, int32& y, int32& width, int32& height) const
{

	return true;
}

void AndroidWindow::MoveWindowTo(int32 x, int32 y)
{
	m_X = x;
	m_Y = y;
}

void AndroidWindow::BringToFront(bool force)
{

}

void AndroidWindow::Destroy()
{
	
}

void AndroidWindow::Minimize()
{
	m_Minimized = false;
}

void AndroidWindow::Maximize()
{
	m_Maximized = true;
}

void AndroidWindow::Restore()
{

}

void AndroidWindow::Show()
{
	if (m_Visible) {
		return;
	}
	m_Visible = true;
}

void AndroidWindow::Hide()
{
	if (!m_Visible) {
		return;
	}
	m_Visible = false;
}

void AndroidWindow::SetWindowMode(WindowMode::Type newWindowMode)
{

}

bool AndroidWindow::IsMaximized() const
{
	return m_Maximized;
}

bool AndroidWindow::IsMinimized() const
{
	return m_Minimized;
}

bool AndroidWindow::IsVisible() const
{
	return m_Visible;
}

bool AndroidWindow::GetRestoredDimensions(int32& x, int32& y, int32& width, int32& height)
{

	return true;
}

void AndroidWindow::SetWindowFocus()
{

}

void AndroidWindow::SetOpacity(const float opacity)
{

}

void AndroidWindow::Enable(bool enable)
{

}

bool AndroidWindow::IsPointInWindow(int32 x, int32 y) const
{
	if ((x > m_X && x < m_X + m_Width) && (y > m_Y && y < m_Y + m_Height)) {
		return true;
	}
	return false;
}

int32 AndroidWindow::GetWindowBorderSize() const
{
	return 0;
}

int32 AndroidWindow::GetWindowTitleBarSize() const
{
	return 0;
}

bool AndroidWindow::IsForegroundWindow() const
{
	return false;
}

void AndroidWindow::SetText(const char* const text)
{
	m_Title = text;
}
