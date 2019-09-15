#include "Application/GenericWindow.h"
#include "Common/Log.h"

GenericWindow::GenericWindow(int32 width, int32 height)
	: m_X(0)
	, m_Y(0)
    , m_Width(width)
    , m_Height(height)
{
	
}

GenericWindow::~GenericWindow()
{
    
}

void GenericWindow::ReshapeWindow(int32 x, int32 y, int32 width, int32 height)
{
	
}

bool GenericWindow::GetFullScreenInfo(int32& x, int32& y, int32& width, int32& height) const
{

	return false;
}

void GenericWindow::MoveWindowTo(int32 x, int32 y)
{
	
}

void GenericWindow::BringToFront(bool force)
{
	
}

void GenericWindow::Destroy()
{
	
}

void GenericWindow::Minimize()
{
	
}

void GenericWindow::Maximize()
{
	
}

void GenericWindow::Restore()
{
	
}

void GenericWindow::Show()
{
	
}

void GenericWindow::Hide()
{
	
}

void GenericWindow::SetWindowMode(WindowMode::Type newWindowMode)
{
	
}

WindowMode::Type GenericWindow::GetWindowMode() const
{

	return WindowMode::Windowed;
}

bool GenericWindow::IsMaximized() const
{

	return true;
}

bool GenericWindow::IsMinimized() const
{

	return false;
}

bool GenericWindow::IsVisible() const
{
	return true;
}

bool GenericWindow::GetRestoredDimensions(int32& x, int32& y, int32& width, int32& height)
{

	return false;
}

void GenericWindow::SetWindowFocus()
{
    
}

void GenericWindow::SetOpacity(const float opacity)
{

}

void GenericWindow::Enable(bool enable)
{

}

bool GenericWindow::IsPointInWindow(int32 x, int32 y) const
{

	return true;
}

int32 GenericWindow::GetWindowBorderSize() const
{

	return 0;
}

int32 GenericWindow::GetWindowTitleBarSize() const
{

	return 0;
}

void* GenericWindow::GetOSWindowHandle() const
{
	return nullptr;
}

const char** GenericWindow::GetRequiredInstanceExtensions(uint32_t* count)
{
    count = 0;
    return nullptr;
}

void GenericWindow::CreateVKSurface(VkInstance instance, VkSurfaceKHR* outSurface)
{

}

bool GenericWindow::IsForegroundWindow() const
{

	return true;
}

void GenericWindow::SetText(const char* const text)
{


}

float GenericWindow::GetDPIScaleFactor() const
{

	return 1.0f;
}

void GenericWindow::SetDPIScaleFactor(const float factor)
{

}

bool GenericWindow::IsManualManageDPIChanges() const
{

	return false;
}

void GenericWindow::SetManualManageDPIChanges(const bool autoHandle)
{

}

const char* GenericWindow::GetTitle() const
{

    return "GenericWindow";
}
