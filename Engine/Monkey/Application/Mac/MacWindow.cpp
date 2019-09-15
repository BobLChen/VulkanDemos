#include "Common/Log.h"
#include "Vulkan/VulkanPlatform.h"
#include "Application/GenericApplicationMessageHandler.h"

#include "CocoaWindow.h"
#include "MacWindow.h"
#include "MacApplication.h"

#include <math.h>
#include <algorithm>

static const char* G_ValidationLayersInstance[] =
{
	VK_KHR_SURFACE_EXTENSION_NAME,
    VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
	nullptr
};

MacWindow::MacWindow(int32 width, int32 height, const char* title)
	: GenericWindow(width, height)
	, m_Title(title)
	, m_WindowMode(WindowMode::Windowed)
    , m_Window(nullptr)
    , m_View(nullptr)
	, m_Application(nullptr)
    , m_MessageHandler(nullptr)
	, m_Visible(false)
	, m_AspectRatio(width * 1.0f / height)
	, m_DPIScaleFactor(1.0f)
	, m_Minimized(false)
	, m_Maximized(false)
{
    
}

MacWindow::~MacWindow()
{

}

float MacWindow::GetDPIScaleFactorAtPoint(float X, float Y)
{
	return 1.0f;
}

void MacWindow::CreateVKSurface(VkInstance instance, VkSurfaceKHR* outSurface)
{
    VkMacOSSurfaceCreateInfoMVK createInfo;
    ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK);
    createInfo.pView = m_View;
    VERIFYVULKANRESULT(vkCreateMacOSSurfaceMVK(instance, &createInfo, VULKAN_CPU_ALLOCATOR, outSurface));
}

const char** MacWindow::GetRequiredInstanceExtensions(uint32_t* count)
{
	*count = 2;
	return G_ValidationLayersInstance;
}

void* MacWindow::GetOSWindowHandle() const
{
	return nullptr;
}

float MacWindow::GetAspectRatio() const
{
	return m_AspectRatio;
}

float MacWindow::GetDPIScaleFactor() const
{
	return m_DPIScaleFactor;
}

void MacWindow::SetDPIScaleFactor(float value)
{

}

std::shared_ptr<MacWindow> MacWindow::Make(int32 width, int32 height, const char* title)
{
	return std::shared_ptr<MacWindow>(new MacWindow(width, height, title));
}

void MacWindow::Initialize(MacApplication* const application)
{
    m_Application = application;
    
    NSUInteger windowStyle =
        NSWindowStyleMaskTitled |
        NSWindowStyleMaskClosable |
        NSWindowStyleMaskResizable |
        NSWindowStyleMaskMiniaturizable;
    
    NSRect windowRect = NSMakeRect(0, 0, m_Width, m_Height);
    
    VulkanView* view = [[VulkanView alloc] initWithFrame:windowRect];
    [view setWantsLayer:YES];
    
    VulkanWindow* window = [[VulkanWindow alloc] initWithContentRect:windowRect styleMask:windowStyle backing:NSBackingStoreBuffered defer:NO];
    
    [window setContentView:view];
    [window makeFirstResponder:view];
    [window setTitle:[NSString stringWithUTF8String:m_Title.c_str()]];
    [window setAcceptsMouseMovedEvents:YES];
    [window setRestorable:NO];
    [window makeKeyAndOrderFront:nil];
    [window center];
    [window orderFrontRegardless];
    
    m_View   = view;
    m_Window = window;
}

void MacWindow::SetMessageHandler(GenericApplicationMessageHandler* messageHandler)
{
    m_MessageHandler = messageHandler;
    VulkanWindow* window = static_cast<VulkanWindow*>(m_Window);
    [window SetMessageHandler: messageHandler];
}

void MacWindow::ReshapeWindow(int32 newX, int32 newY, int32 newWidth, int32 newHeight)
{
	m_X = newX;
	m_Y = newY;
	m_Width  = newWidth;
	m_Height = newHeight;
}

bool MacWindow::GetFullScreenInfo(int32& x, int32& y, int32& width, int32& height) const
{

	return true;
}

void MacWindow::MoveWindowTo(int32 x, int32 y)
{
	m_X = x;
	m_Y = y;
}

void MacWindow::BringToFront(bool force)
{

}

void MacWindow::Destroy()
{
    
}

void MacWindow::Minimize()
{
	m_Minimized = false;
}

void MacWindow::Maximize()
{
	m_Maximized = true;
}

void MacWindow::Restore()
{

}

void MacWindow::Show()
{
	if (m_Visible) {
		return;
	}
	m_Visible = true;
    
    VulkanWindow* window = static_cast<VulkanWindow*>(m_Window);
    [window orderFrontRegardless];
}

void MacWindow::Hide()
{
	if (!m_Visible) {
		return;
	}
	m_Visible = false;
    
    VulkanWindow* window = static_cast<VulkanWindow*>(m_Window);
    [window orderOut:nil];
}

void MacWindow::SetWindowMode(WindowMode::Type newWindowMode)
{

}

bool MacWindow::IsMaximized() const
{
	return m_Maximized;
}

bool MacWindow::IsMinimized() const
{
	return m_Minimized;
}

bool MacWindow::IsVisible() const
{
	return m_Visible;
}

bool MacWindow::GetRestoredDimensions(int32& x, int32& y, int32& width, int32& height)
{

	return true;
}

void MacWindow::SetWindowFocus()
{

}

void MacWindow::SetOpacity(const float opacity)
{

}

void MacWindow::Enable(bool enable)
{

}

bool MacWindow::IsPointInWindow(int32 x, int32 y) const
{
	if ((x > m_X && x < m_X + m_Width) && (y > m_Y && y < m_Y + m_Height)) {
		return true;
	}
	return false;
}

int32 MacWindow::GetWindowBorderSize() const
{
	return 0;
}

int32 MacWindow::GetWindowTitleBarSize() const
{
	return 0;
}

bool MacWindow::IsForegroundWindow() const
{
	return false;
}

void MacWindow::SetText(const char* const text)
{
	m_Title = text;
}
