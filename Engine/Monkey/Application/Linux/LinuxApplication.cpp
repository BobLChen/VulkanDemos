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
	if (m_Window != nullptr) {
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
	static Vector2 mousePos(0, 0);

	xcb_generic_event_t* event = nullptr;
	while ((event = xcb_poll_for_event(connection)))
	{
		int32 eventType = event->response_type & 0x7f;
		xcb_intern_atom_reply_t* atomWmDeleteWindow = m_Window->GetAtomWmDeleteWindow();

		switch (eventType)
		{
			case XCB_CLIENT_MESSAGE:
			{
				if ((*(xcb_client_message_event_t*)event).data.data32[0] == (*atomWmDeleteWindow).atom) {
					m_MessageHandler->OnRequestingExit();
				}
				break;
			}
			case XCB_MOTION_NOTIFY:
			{
				xcb_motion_notify_event_t* motion = (xcb_motion_notify_event_t*)event;
				mousePos.x = (int32_t)motion->event_x;
				mousePos.y = (int32_t)motion->event_y;
				m_MessageHandler->OnMouseMove(mousePos);
				break;
			}
			case XCB_BUTTON_PRESS:
			{
				xcb_button_press_event_t* press = (xcb_button_press_event_t*)event;
				if (press->detail == XCB_BUTTON_INDEX_1) {
					m_MessageHandler->OnMouseDown(MouseType::MOUSE_BUTTON_LEFT, mousePos);
				}
				if (press->detail == XCB_BUTTON_INDEX_2) {
					m_MessageHandler->OnMouseDown(MouseType::MOUSE_BUTTON_MIDDLE, mousePos);
				}
				if (press->detail == XCB_BUTTON_INDEX_3) {
					m_MessageHandler->OnMouseDown(MouseType::MOUSE_BUTTON_RIGHT, mousePos);
				}
				break;
			}
			case XCB_BUTTON_RELEASE:
			{
				xcb_button_press_event_t* press = (xcb_button_press_event_t*)event;
				if (press->detail == XCB_BUTTON_INDEX_1) {
					m_MessageHandler->OnMouseUp(MouseType::MOUSE_BUTTON_LEFT, mousePos);
				}
				if (press->detail == XCB_BUTTON_INDEX_2) {
					m_MessageHandler->OnMouseUp(MouseType::MOUSE_BUTTON_MIDDLE, mousePos);
				}
				if (press->detail == XCB_BUTTON_INDEX_3) {
					m_MessageHandler->OnMouseUp(MouseType::MOUSE_BUTTON_RIGHT, mousePos);
				}
				break;
			}
			case XCB_KEY_PRESS:
			{
				const xcb_key_release_event_t* keyEvent = (const xcb_key_release_event_t*)event;
				KeyboardType key = InputManager::GetKeyFromKeyCode(keyEvent->detail);
				m_MessageHandler->OnKeyDown(key);
			}
			break;	
			case XCB_KEY_RELEASE:
			{
				const xcb_key_release_event_t* keyEvent = (const xcb_key_release_event_t*)event;
				KeyboardType key = InputManager::GetKeyFromKeyCode(keyEvent->detail);
				m_MessageHandler->OnKeyUp(key);
				break;
			}
			case XCB_DESTROY_NOTIFY:
			{
				m_MessageHandler->OnRequestingExit();
				break;
			}
			case XCB_CONFIGURE_NOTIFY:
			{
				const xcb_configure_notify_event_t* cfgEvent = (const xcb_configure_notify_event_t*)event;
				if (cfgEvent->width != m_Window->GetWidth() || cfgEvent->height != m_Window->GetHeight()) {
					m_MessageHandler->OnSizeChanged(cfgEvent->width, cfgEvent->height);
				}
				break;
			}
		}
		
		free(event);
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
	if (showImmediately) {
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
