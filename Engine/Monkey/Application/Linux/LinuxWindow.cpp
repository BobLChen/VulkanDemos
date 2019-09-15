#include "Common/Log.h"
#include "Vulkan/VulkanPlatform.h"

#include "LinuxWindow.h"
#include "LinuxApplication.h"

#include <math.h>
#include <algorithm>

static const char* G_ValidationLayersInstance[] =
{
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_XCB_SURFACE_EXTENSION_NAME,
	nullptr
};

LinuxWindow::LinuxWindow(int32 width, int32 height, const char* title)
	: GenericWindow(width, height)
	, m_Title(title)
	, m_Window(0)
	, m_Screen(nullptr)
	, m_Connection(nullptr)
	, m_AtomWmDeleteWindow(nullptr)
	, m_WindowMode(WindowMode::Windowed)
	, m_Application(nullptr)
	, m_Visible(false)
	, m_AspectRatio(width * 1.0f / height)
	, m_DPIScaleFactor(1.0f)
	, m_Minimized(false)
	, m_Maximized(false)
{

}

LinuxWindow::~LinuxWindow()
{

}

float LinuxWindow::GetDPIScaleFactorAtPoint(float X, float Y)
{
	return 1.0f;
}

void LinuxWindow::CreateVKSurface(VkInstance instance, VkSurfaceKHR* outSurface)
{
	VkXcbSurfaceCreateInfoKHR createInfo;
	ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR);
	createInfo.connection = m_Connection;
	createInfo.window     = m_Window;
	vkCreateXcbSurfaceKHR(instance, &createInfo, VULKAN_CPU_ALLOCATOR, outSurface);
}

const char** LinuxWindow::GetRequiredInstanceExtensions(uint32_t* count)
{
	*count = 2;
	return G_ValidationLayersInstance;
}

void* LinuxWindow::GetOSWindowHandle() const
{
	return m_Connection;
}

float LinuxWindow::GetAspectRatio() const
{
	return m_AspectRatio;
}

float LinuxWindow::GetDPIScaleFactor() const
{
	return m_DPIScaleFactor;
}

void LinuxWindow::SetDPIScaleFactor(float value)
{

}

std::shared_ptr<LinuxWindow> LinuxWindow::Make(int32 width, int32 height, const char* title)
{
	return std::shared_ptr<LinuxWindow>(new LinuxWindow(width, height, title));
}

void LinuxWindow::Initialize(LinuxApplication* const application)
{
	int screenp  = 0;
	m_Connection = xcb_connect(NULL, &screenp);

	if (m_Connection == NULL) {
		printf("Could not find a compatible Vulkan ICD!\n");
		return;
	}
	
	const xcb_setup_t* setup = xcb_get_setup(m_Connection);
	xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);

	while (screenp-- > 0) {
		xcb_screen_next(&iter);
	}

	m_Screen = iter.data;

	uint32_t value_mask = 0;
	uint32_t value_list[32];

	m_Window = xcb_generate_id(m_Connection);

	value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	value_list[0] = m_Screen->black_pixel;
	value_list[1] =
		XCB_EVENT_MASK_KEY_RELEASE |
		XCB_EVENT_MASK_KEY_PRESS |
		XCB_EVENT_MASK_EXPOSURE |
		XCB_EVENT_MASK_STRUCTURE_NOTIFY |
		XCB_EVENT_MASK_POINTER_MOTION |
		XCB_EVENT_MASK_BUTTON_PRESS |
		XCB_EVENT_MASK_BUTTON_RELEASE;
	
	xcb_create_window(m_Connection, XCB_COPY_FROM_PARENT, m_Window, m_Screen->root, 0, 0, m_Width, m_Height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, m_Screen->root_visual, value_mask, value_list);

	xcb_intern_atom_cookie_t cookie = xcb_intern_atom(m_Connection, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(m_Connection, cookie, 0);
	xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(m_Connection, 0, 16, "WM_DELETE_WINDOW");
    m_AtomWmDeleteWindow = xcb_intern_atom_reply(m_Connection, cookie2, 0);

	xcb_change_property(m_Connection, XCB_PROP_MODE_REPLACE, m_Window, (*reply).atom, 4, 32, 1, &(*m_AtomWmDeleteWindow).atom);
    free(reply);
    xcb_map_window(m_Connection, m_Window);
	xcb_flush(m_Connection);
}

void LinuxWindow::ReshapeWindow(int32 newX, int32 newY, int32 newWidth, int32 newHeight)
{
	m_X = newX;
	m_Y = newY;
	m_Width = newWidth;
	m_Height = newHeight;
}

bool LinuxWindow::GetFullScreenInfo(int32& x, int32& y, int32& width, int32& height) const
{

	return true;
}

void LinuxWindow::MoveWindowTo(int32 x, int32 y)
{
	m_X = x;
	m_Y = y;
}

void LinuxWindow::BringToFront(bool force)
{

}

void LinuxWindow::Destroy()
{
	xcb_destroy_window(m_Connection, m_Window);
	xcb_disconnect(m_Connection);
	free(m_AtomWmDeleteWindow);
}

void LinuxWindow::Minimize()
{
	m_Minimized = false;
}

void LinuxWindow::Maximize()
{
	m_Maximized = true;
}

void LinuxWindow::Restore()
{

}

void LinuxWindow::Show()
{
	if (m_Visible) {
		return;
	}
	m_Visible = true;
}

void LinuxWindow::Hide()
{
	if (!m_Visible) {
		return;
	}
	m_Visible = false;
}

void LinuxWindow::SetWindowMode(WindowMode::Type newWindowMode)
{

}

bool LinuxWindow::IsMaximized() const
{
	return m_Maximized;
}

bool LinuxWindow::IsMinimized() const
{
	return m_Minimized;
}

bool LinuxWindow::IsVisible() const
{
	return m_Visible;
}

bool LinuxWindow::GetRestoredDimensions(int32& x, int32& y, int32& width, int32& height)
{

	return true;
}

void LinuxWindow::SetWindowFocus()
{

}

void LinuxWindow::SetOpacity(const float opacity)
{

}

void LinuxWindow::Enable(bool enable)
{

}

bool LinuxWindow::IsPointInWindow(int32 x, int32 y) const
{
	if ((x > m_X && x < m_X + m_Width) && (y > m_Y && y < m_Y + m_Height)) {
		return true;
	}
	return false;
}

int32 LinuxWindow::GetWindowBorderSize() const
{
	return 0;
}

int32 LinuxWindow::GetWindowTitleBarSize() const
{
	return 0;
}

bool LinuxWindow::IsForegroundWindow() const
{
	return false;
}

void LinuxWindow::SetText(const char* const text)
{
	m_Title = text;
}
