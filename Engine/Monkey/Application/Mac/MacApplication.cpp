#include "Common/Log.h"

#include "Engine.h"
#include "MacApplication.h"

#include <memory>
#include <string>

#include <Cocoa/Cocoa.h>

std::shared_ptr<MacApplication> G_CurrentPlatformApplication = nullptr;

MacApplication::MacApplication()
	: m_Window(nullptr)
{
	
}

MacApplication::~MacApplication()
{
	if (m_Window != nullptr) {
		MLOGE("Window not shutdown.");
	}
}

void MacApplication::SetMessageHandler(GenericApplicationMessageHandler* messageHandler)
{
	GenericApplication::SetMessageHandler(messageHandler);
}

void MacApplication::PumpMessages()
{
    
}

void MacApplication::Tick(float time, float delta)
{
    
}

std::shared_ptr<GenericWindow> MacApplication::MakeWindow(int32 width, int32 height, const char* title)
{
	return MacWindow::Make(width, height, title);
}

std::shared_ptr<GenericWindow> MacApplication::GetWindow()
{
	return m_Window;
}

void MacApplication::InitializeWindow(const std::shared_ptr<GenericWindow> window, const bool showImmediately)
{
	m_Window = std::dynamic_pointer_cast<MacWindow>(window);
	m_Window->Initialize(this);
	if (showImmediately) {
		m_Window->Show();
	}
    m_Window->SetMessageHandler(m_MessageHandler);
}

void MacApplication::Destroy()
{
	if (m_Window != nullptr) {
		m_Window->Destroy();
		m_Window = nullptr;
	}
}

std::shared_ptr<GenericApplication> GenericApplication::Create()
{
	G_CurrentPlatformApplication = std::make_shared<MacApplication>();
	return G_CurrentPlatformApplication;
}

GenericApplication& GenericApplication::GetApplication()
{
	return *G_CurrentPlatformApplication;
}
