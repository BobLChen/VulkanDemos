#include "LinuxApplication.h"
#include "Common/Log.h"
#include "Engine.h"

#include <memory>
#include <map>
#include <string>

std::shared_ptr<LinuxApplication> G_CurrentPlatformApplication = nullptr;

LinuxApplication::LinuxApplication()
	: m_Window(nullptr)
{
	
}

LinuxApplication::~LinuxApplication()
{
	if (m_Window != nullptr)
	{
		MLOGE("Window not shutdown.");
	}
}

void LinuxApplication::SetMessageHandler(GenericApplicationMessageHandler* messageHandler)
{
	GenericApplication::SetMessageHandler(messageHandler);
}

void LinuxApplication::PumpMessages()
{
	xcb_connection_t* connection = m_Window->GetConnection();

	xcb_generic_event_t *event;
	while (event)
	{
		event = xcb_poll_for_event(connection);
		while (event) 
		{
            
			xcb_intern_atom_reply_t* atomWmDeleteWindow = m_Window->GetAtomWmDeleteWindow();
			uint8_t eventCode = event->response_type & 0x7f;
			if (eventCode == XCB_CLIENT_MESSAGE)
			{
				if ((*(xcb_client_message_event_t*)event).data.data32[0] == (*atomWmDeleteWindow).atom) {
					Engine::Get()->RequestExit(true);
				}
			}
			
            free(event);
            event = xcb_poll_for_event(connection);
        }
	}
}

void LinuxApplication::Tick(float time, float delta)
{

}

std::shared_ptr<GenericWindow> LinuxApplication::MakeWindow(int32 width, int32 height, const char* title)
{
	return LinuxWindow::Make(width, height, title);
}

std::shared_ptr<GenericWindow> LinuxApplication::GetWindow()
{
	return m_Window;
}

void LinuxApplication::InitializeWindow(const std::shared_ptr<GenericWindow> window, const bool showImmediately)
{
	m_Window = std::dynamic_pointer_cast<LinuxWindow>(window);
	m_Window->Initialize(this);
	if (showImmediately)
	{
		m_Window->Show();
	}
}

void LinuxApplication::Destroy()
{
	if (m_Window != nullptr) {
		m_Window->Destroy();
		m_Window = nullptr;
	}
}

std::shared_ptr<GenericApplication> GenericApplication::Create()
{
	G_CurrentPlatformApplication = std::make_shared<LinuxApplication>();
	return G_CurrentPlatformApplication;
}

GenericApplication& GenericApplication::GetApplication()
{
	return *G_CurrentPlatformApplication;
}
