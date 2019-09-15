#include "IOSApplication.h"
#include "Common/Log.h"
#include "Engine.h"

#include <memory>
#include <map>
#include <string>

std::shared_ptr<IOSApplication> G_CurrentPlatformApplication = nullptr;

IOSApplication::IOSApplication()
	: m_Window(nullptr)
{
	
}

IOSApplication::~IOSApplication()
{
	if (m_Window != nullptr)
	{
		MLOGE("Window not shutdown.");
	}
}

void IOSApplication::SetMessageHandler(const std::shared_ptr<GenericApplicationMessageHandler>& messageHandler)
{
	GenericApplication::SetMessageHandler(messageHandler);
}

void IOSApplication::PumpMessages(const float deltaTime)
{
    
}

void IOSApplication::Tick(const float deltaTime)
{
    
}

std::shared_ptr<GenericWindow> IOSApplication::MakeWindow(int32 width, int32 height, const char* title)
{
	return IOSWindow::Make(width, height, title);
}

std::shared_ptr<GenericWindow> IOSApplication::GetWindow()
{
	return m_Window;
}

void IOSApplication::InitializeWindow(const std::shared_ptr<GenericWindow>& window, const bool showImmediately)
{
	m_Window = std::dynamic_pointer_cast<IOSWindow>(window);
	m_Window->Initialize(this);
	if (showImmediately)
	{
		m_Window->Show();
	}
}

void IOSApplication::Destroy()
{
	if (m_Window != nullptr) {
		m_Window->Destroy();
		m_Window = nullptr;
	}
}

std::shared_ptr<GenericApplication> GenericApplication::Create()
{
	G_CurrentPlatformApplication = std::make_shared<IOSApplication>();
	return G_CurrentPlatformApplication;
}

GenericApplication& GenericApplication::GetApplication()
{
	return *G_CurrentPlatformApplication;
}
