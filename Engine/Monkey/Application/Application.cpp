#include "Engine.h"
#include "Application.h"
#include "GenericPlatform/InputManager.h"

Application::Application()
	: GenericApplicationMessageHandler()
	, m_Engine(nullptr)
    , m_Window(nullptr)
	, m_Application(nullptr)
{
    
}

Application::~Application()
{
    m_Engine = nullptr;
    m_Window = nullptr;
    m_Application = nullptr;
}

void Application::Init(Engine* engine)
{
	m_Engine = engine;
	m_Application = GenericApplication::Create();
	m_Application->SetMessageHandler(this);
}

std::shared_ptr<GenericApplication> Application::GetPlatformApplication()
{
	return m_Application;
}

std::shared_ptr<GenericWindow> Application::GetPlatformWindow()
{
    return m_Window;
}

void Application::Shutdown(bool shutdownPlatform)
{
	if (m_Application)
	{
		m_Application->Destroy();
		m_Application = nullptr;
	}
}

void Application::Tick(float time, float delta)
{
	PumpMessages();
	TickPlatform(time, delta);
}

void Application::PumpMessages()
{
	m_Application->PumpMessages();
}

void Application::OnRequestingExit()
{
	m_Engine->RequestExit(true);
}

std::shared_ptr<GenericWindow> Application::MakeWindow(int32 width, int32 height, const char* title)
{
	m_Window = m_Application->MakeWindow(width, height, title);
	m_Application->InitializeWindow(m_Window, true);
	return m_Window;
}

void Application::TickPlatform(float time, float delta)
{
	m_Application->Tick(time, delta);
}

void Application::SetCursorPos(const Vector2& pos)
{
	
}

bool Application::OnKeyDown(const KeyboardType key)
{
    InputManager::OnKeyDown(key);
	return true;
}

bool Application::OnKeyUp(const KeyboardType key)
{
    InputManager::OnKeyUp(key);
	return true;
}

bool Application::OnMouseDown(MouseType type, const Vector2& pos)
{
    InputManager::OnMouseDown(type, pos);
	return true;
}

bool Application::OnMouseUp(MouseType type, const Vector2& pos)
{
    InputManager::OnMouseUp(type, pos);
	return true;
}

bool Application::OnMouseDoubleClick(MouseType type, const Vector2& pos)
{
	return true;
}

bool Application::OnMouseWheel(const float delta, const Vector2& pos)
{
    InputManager::OnMouseWheel(delta, pos);
	return true;
}

bool Application::OnMouseMove(const Vector2& pos)
{
    InputManager::OnMouseMove(pos);
	return true;
}

bool Application::OnTouchStarted(const std::vector<Vector2>& locations)
{
	return true;
}

bool Application::OnTouchMoved(const std::vector<Vector2>& locations)
{
	return true;
}

bool Application::OnTouchEnded(const std::vector<Vector2>& locations)
{
	return true;
}

bool Application::OnTouchForceChanged(const std::vector<Vector2>& locations)
{
	return true;
}

bool Application::OnTouchFirstMove(const std::vector<Vector2>& locations)
{
	return true;
}

bool Application::OnSizeChanged(const int32 width, const int32 height)
{
	return true;
}

void Application::OnOSPaint()
{
    
}

WindowSizeLimits Application::GetSizeLimitsForWindow() const
{
	return WindowSizeLimits();
}

void Application::OnResizingWindow()
{

}

bool Application::BeginReshapingWindow()
{
	return true;
}

void Application::FinishedReshapingWindow()
{

}

void Application::SignalSystemDPIChanged()
{
    
}

void Application::HandleDPIScaleChanged()
{
    
}

void Application::OnMovedWindow(const int32 x, const int32 y)
{

}

void Application::OnWindowClose()
{
    
}
