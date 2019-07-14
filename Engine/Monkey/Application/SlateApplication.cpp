#include "Engine.h"
#include "SlateApplication.h"
#include "GenericPlatform/InputManager.h"

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

void SlateApplication::SetCursorPos(const Vector2& pos)
{
	
}

bool SlateApplication::OnKeyDown(const KeyboardType key)
{
    InputManager::OnKeyDown(key);
	return true;
}

bool SlateApplication::OnKeyUp(const KeyboardType key)
{
    InputManager::OnKeyUp(key);
	return true;
}

bool SlateApplication::OnMouseDown(MouseType type, const Vector2& pos)
{
    InputManager::OnMouseDown(type, pos);
	return true;
}

bool SlateApplication::OnMouseUp(MouseType type, const Vector2& pos)
{
    InputManager::OnMouseUp(type, pos);
	return true;
}

bool SlateApplication::OnMouseDoubleClick(MouseType type, const Vector2& pos)
{
	return true;
}

bool SlateApplication::OnMouseWheel(const float delta, const Vector2& pos)
{
    InputManager::OnMouseWheel(delta, pos);
	return true;
}

bool SlateApplication::OnMouseMove(const Vector2& pos)
{
    InputManager::OnMouseMove(pos);
	return true;
}

bool SlateApplication::OnTouchStarted(const std::vector<Vector2>& locations)
{
	return true;
}

bool SlateApplication::OnTouchMoved(const std::vector<Vector2>& locations)
{
	return true;
}

bool SlateApplication::OnTouchEnded(const std::vector<Vector2>& locations)
{
	return true;
}

bool SlateApplication::OnTouchForceChanged(const std::vector<Vector2>& locations)
{
	return true;
}

bool SlateApplication::OnTouchFirstMove(const std::vector<Vector2>& locations)
{
	return true;
}

bool SlateApplication::OnSizeChanged(const int32 width, const int32 height)
{
	return true;
}

void SlateApplication::OnOSPaint()
{
    
}

WindowSizeLimits SlateApplication::GetSizeLimitsForWindow() const
{
	return WindowSizeLimits();
}

void SlateApplication::OnResizingWindow()
{

}

bool SlateApplication::BeginReshapingWindow()
{
	return true;
}

void SlateApplication::FinishedReshapingWindow()
{

}

void SlateApplication::SignalSystemDPIChanged()
{
    
}

void SlateApplication::HandleDPIScaleChanged()
{
    
}

void SlateApplication::OnMovedWindow(const int32 x, const int32 y)
{

}

void SlateApplication::OnWindowClose()
{
    
}
