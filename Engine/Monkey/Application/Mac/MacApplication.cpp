#include "MacApplication.h"
#include "Common/Log.h"
#include "Engine.h"

#include <memory>
#include <map>
#include <string>

#include <Cocoa/Cocoa.h>

std::shared_ptr<MacApplication> G_CurrentPlatformApplication = nullptr;

MacApplication::MacApplication()
	: m_Window(nullptr)
{
	
}

MacApplication::~MacApplication()
{
	if (m_Window != nullptr)
	{
		MLOGE("Window not shutdown.");
	}
}

void MacApplication::SetMessageHandler(const std::shared_ptr<GenericApplicationMessageHandler>& messageHandler)
{
	GenericApplication::SetMessageHandler(messageHandler);
}

void MacApplication::PumpMessages(const float deltaTime)
{
    while (true)
    {
        NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                            untilDate:[NSDate distantPast]
                                               inMode:NSDefaultRunLoopMode
                                              dequeue:YES];
        if (event == nil)
        {
            break;
        }
        
        [NSApp sendEvent:event];
    }
}

void MacApplication::Tick(const float deltaTime)
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

void MacApplication::InitializeWindow(const std::shared_ptr<GenericWindow>& window, const bool showImmediately)
{
	m_Window = std::dynamic_pointer_cast<MacWindow>(window);
	m_Window->Initialize(this);
	if (showImmediately)
	{
		m_Window->Show();
	}
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
