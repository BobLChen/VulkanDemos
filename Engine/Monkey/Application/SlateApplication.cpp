#include "SlateApplication.h"
#include "Engine.h"

std::shared_ptr<SlateApplication> SlateApplication::m_CurrentApplication = nullptr;

std::shared_ptr<GenericApplication> GPlatformApplication = nullptr;

SlateApplication::SlateApplication()
	: GenericApplicationMessageHandler()
	, m_DeltaTime(0.16f)
{

}

SlateApplication::~SlateApplication()
{

}

void SlateApplication::SetUpEngine(Engine *engine)
{
	m_Engine = engine;
}

void SlateApplication::Create(Engine* engine)
{
	Create(GenericApplication::Create(), engine);
}

std::shared_ptr<SlateApplication> SlateApplication::Create(const std::shared_ptr<GenericApplication>& platformApplication, Engine* engine)
{
	m_CurrentApplication = std::shared_ptr<SlateApplication>(new SlateApplication());
	m_CurrentApplication->SetUpEngine(engine);

	GPlatformApplication = platformApplication;
	GPlatformApplication->SetMessageHandler(m_CurrentApplication);

	return m_CurrentApplication;
}

std::shared_ptr<GenericApplication> SlateApplication::GetPlatformApplication()
{
	return GPlatformApplication;
}

void SlateApplication::Shutdown(bool shutdownPlatform)
{
	if (SlateApplication::IsInitialized())
	{
		m_CurrentApplication->OnShutdown();
		if (shutdownPlatform) {
			GPlatformApplication->Destroy();
			GPlatformApplication = nullptr;
		}
		m_CurrentApplication = nullptr;
	}
}

void SlateApplication::InitHighDPI(const bool force)
{

}

void SlateApplication::SetCursorPos(const Vector2D& mouseCoordinate)
{
	
}

void SlateApplication::Tick(float detalTime)
{
	m_DeltaTime = detalTime;
	PumpMessages();
	TickPlatform(detalTime);
}

void SlateApplication::PumpMessages()
{
	GPlatformApplication->PumpMessages(m_DeltaTime);
}

void SlateApplication::OnShutdown()
{

}

void SlateApplication::OnRequestingExit()
{
	m_Engine->RequestExit(true);
}

std::shared_ptr<GenericWindow> SlateApplication::MakeWindow(int32 width, int32 height, const char* title)
{
	std::shared_ptr<GenericWindow> newWindow = GPlatformApplication->MakeWindow(width, height, title);
	GPlatformApplication->InitializeWindow(newWindow, nullptr, true);
	return newWindow;
}

void SlateApplication::TickPlatform(float deltaTime)
{
	GPlatformApplication->Tick(deltaTime);
}

void SlateApplication::DrawWindows()
{

}

bool SlateApplication::ShouldProcessUserInputMessages(const std::shared_ptr<GenericWindow>& platformWindow) const
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

bool SlateApplication::OnMouseDown(const std::shared_ptr<GenericWindow>& platformWindow, const MouseButtons::Type button)
{
	return true;
}

bool SlateApplication::OnMouseDown(const std::shared_ptr<GenericWindow>& platformWindow, const MouseButtons::Type button, const Vector2D cursorPos)
{
	return true;
}

bool SlateApplication::OnMouseUp(const MouseButtons::Type button)
{
	return true;
}

bool SlateApplication::OnMouseUp(const MouseButtons::Type button, const Vector2D cursorPos)
{
	return true;
}

bool SlateApplication::OnMouseDoubleClick(const std::shared_ptr<GenericWindow>& platformWindow, const MouseButtons::Type button)
{
	return true;
}

bool SlateApplication::OnMouseDoubleClick(const std::shared_ptr<GenericWindow>& platformWindow, const MouseButtons::Type button, const Vector2D cursorPos)
{
	return true;
}

bool SlateApplication::OnMouseWheel(const float delta)
{
	return true;
}

bool SlateApplication::OnMouseWheel(const float delta, const Vector2D cursorPos)
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

bool SlateApplication::OnTouchStarted(const std::shared_ptr<GenericWindow>& platformWindow, const Vector2D& location, float force, int32 touchIndex, int32 controllerId)
{
	return true;
}

bool SlateApplication::OnTouchMoved(const Vector2D& location, float force, int32 touchIndex, int32 controllerId)
{
	return true;
}

bool SlateApplication::OnTouchEnded(const Vector2D& location, int32 touchIndex, int32 controllerId)
{
	return true;
}

bool SlateApplication::OnTouchForceChanged(const Vector2D& location, float force, int32 touchIndex, int32 controllerId)
{
	return true;
}

bool SlateApplication::OnTouchFirstMove(const Vector2D& location, float force, int32 touchIndex, int32 controllerId)
{
	return true;
}

bool SlateApplication::OnSizeChanged(const std::shared_ptr<GenericWindow>& window, const int32 width, const int32 height, bool wasMinimized)
{
	return true;
}

void SlateApplication::OnOSPaint(const std::shared_ptr<GenericWindow>& window)
{
	return;
}

WindowSizeLimits SlateApplication::GetSizeLimitsForWindow(const std::shared_ptr<GenericWindow>& window) const
{
	return WindowSizeLimits();
}

void SlateApplication::OnResizingWindow(const std::shared_ptr<GenericWindow>& window)
{

}

bool SlateApplication::BeginReshapingWindow(const std::shared_ptr<GenericWindow>& window)
{
	return true;
}

void SlateApplication::FinishedReshapingWindow(const std::shared_ptr<GenericWindow>& window)
{

}

void SlateApplication::SignalSystemDPIChanged(const std::shared_ptr<GenericWindow>& window)
{

}

void SlateApplication::HandleDPIScaleChanged(const std::shared_ptr<GenericWindow>& window)
{

}

void SlateApplication::OnMovedWindow(const std::shared_ptr<GenericWindow>& window, const int32 x, const int32 y)
{

}

void SlateApplication::OnWindowClose(const std::shared_ptr<GenericWindow>& window)
{

}
