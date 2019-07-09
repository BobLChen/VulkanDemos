#include "Engine.h"
#include "SlateApplication.h"

SlateApplication::SlateApplication()
	: GenericApplicationMessageHandler()
	, m_Engine(nullptr)
    , m_Window(nullptr)
	, m_Application(nullptr)
{
    
}

SlateApplication::~SlateApplication()
{
    m_Engine = nullptr;
    m_Window = nullptr;
    m_Application = nullptr;
}

void SlateApplication::Init(Engine* engine)
{
	m_Engine = engine;
	m_Application = GenericApplication::Create();
	m_Application->SetMessageHandler(this);
}

std::shared_ptr<GenericApplication> SlateApplication::GetPlatformApplication()
{
	return m_Application;
}

std::shared_ptr<GenericWindow> SlateApplication::GetPlatformWindow()
{
    return m_Window;
}

void SlateApplication::Shutdown(bool shutdownPlatform)
{
	if (m_Application)
	{
		m_Application->Destroy();
		m_Application = nullptr;
	}
}

void SlateApplication::Tick(float time, float delta)
{
	PumpMessages();
	TickPlatform(time, delta);
}

void SlateApplication::PumpMessages()
{
	m_Application->PumpMessages();
}

void SlateApplication::OnRequestingExit()
{
	m_Engine->RequestExit(true);
}

std::shared_ptr<GenericWindow> SlateApplication::MakeWindow(int32 width, int32 height, const char* title)
{
	m_Window = m_Application->MakeWindow(width, height, title);
	m_Application->InitializeWindow(m_Window, true);
	return m_Window;
}

void SlateApplication::TickPlatform(float time, float delta)
{
	m_Application->Tick(time, delta);
}

void SlateApplication::SetCursorPos(const Vector2& mouseCoordinate)
{
	
}

bool SlateApplication::ShouldProcessUserInputMessages(const std::shared_ptr<GenericWindow> platformWindow) const
{
	return false;
}

bool SlateApplication::OnKeyChar(const char character, const bool isRepeat)
{
	return true;
}

bool SlateApplication::OnKeyDown(const int32 keyCode, const uint32 characterCode, const bool isRepeat)
{
	return true;
}

bool SlateApplication::OnKeyUp(const int32 keyCode, const uint32 characterCode, const bool isRepeat)
{
	return true;
}

bool SlateApplication::OnMouseDown(const std::shared_ptr<GenericWindow> platformWindow, const MouseButtons::Type button)
{
	return true;
}

bool SlateApplication::OnMouseDown(const std::shared_ptr<GenericWindow> platformWindow, const MouseButtons::Type button, const Vector2 cursorPos)
{
	return true;
}

bool SlateApplication::OnMouseUp(const MouseButtons::Type button)
{
	return true;
}

bool SlateApplication::OnMouseUp(const MouseButtons::Type button, const Vector2 cursorPos)
{
	return true;
}

bool SlateApplication::OnMouseDoubleClick(const std::shared_ptr<GenericWindow> platformWindow, const MouseButtons::Type button)
{
	return true;
}

bool SlateApplication::OnMouseDoubleClick(const std::shared_ptr<GenericWindow> platformWindow, const MouseButtons::Type button, const Vector2 cursorPos)
{
	return true;
}

bool SlateApplication::OnMouseWheel(const float delta)
{
	return true;
}

bool SlateApplication::OnMouseWheel(const float delta, const Vector2 cursorPos)
{
	return true;
}

bool SlateApplication::OnMouseMove()
{
	return true;
}

bool SlateApplication::OnRawMouseMove(const int32 x, const int32 y)
{
	return true;
}

bool SlateApplication::OnCursorSet()
{
	return true;
}

bool SlateApplication::OnTouchStarted(const std::shared_ptr<GenericWindow> platformWindow, const Vector2& location, float force, int32 touchIndex, int32 controllerId)
{
	return true;
}

bool SlateApplication::OnTouchMoved(const Vector2& location, float force, int32 touchIndex, int32 controllerId)
{
	return true;
}

bool SlateApplication::OnTouchEnded(const Vector2& location, int32 touchIndex, int32 controllerId)
{
	return true;
}

bool SlateApplication::OnTouchForceChanged(const Vector2& location, float force, int32 touchIndex, int32 controllerId)
{
	return true;
}

bool SlateApplication::OnTouchFirstMove(const Vector2& location, float force, int32 touchIndex, int32 controllerId)
{
	return true;
}

bool SlateApplication::OnSizeChanged(const std::shared_ptr<GenericWindow> window, const int32 width, const int32 height, bool wasMinimized)
{
	return true;
}

void SlateApplication::OnOSPaint(const std::shared_ptr<GenericWindow> window)
{

}

WindowSizeLimits SlateApplication::GetSizeLimitsForWindow(const std::shared_ptr<GenericWindow> window) const
{
	return WindowSizeLimits();
}

void SlateApplication::OnResizingWindow(const std::shared_ptr<GenericWindow> window)
{

}

bool SlateApplication::BeginReshapingWindow(const std::shared_ptr<GenericWindow> window)
{
	return true;
}

void SlateApplication::FinishedReshapingWindow(const std::shared_ptr<GenericWindow> window)
{

}

void SlateApplication::SignalSystemDPIChanged(const std::shared_ptr<GenericWindow> window)
{

}

void SlateApplication::HandleDPIScaleChanged(const std::shared_ptr<GenericWindow> window)
{

}

void SlateApplication::OnMovedWindow(const std::shared_ptr<GenericWindow> window, const int32 x, const int32 y)
{

}

void SlateApplication::OnWindowClose(const std::shared_ptr<GenericWindow> window)
{

}
