#include "Common/Log.h"

#include "Engine.h"
#include "WinApplication.h"

#include <memory>
#include <map>
#include <string>

#include <Windows.h>
#include <windowsx.h>

static int GetKeyMods()
{
    int mods = 0;

    if (GetKeyState(VK_SHIFT) & (1 << 31)) {
		mods |= (int32)KeyboardType::KEY_MOD_SHIFT;
	}

    if (GetKeyState(VK_CONTROL) & (1 << 31)) {
		mods |= (int32)KeyboardType::KEY_MOD_CONTROL;
	}

    if (GetKeyState(VK_MENU) & (1 << 31)) {
		mods |= (int32)KeyboardType::KEY_MOD_ALT;
	}

    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & (1 << 31)) {
		mods |= (int32)KeyboardType::KEY_MOD_SUPER;
	}

    return mods;
}

std::shared_ptr<WinApplication> G_CurrentPlatformApplication = nullptr;

WinApplication::WinApplication()
	: m_Window(nullptr)
{
	WNDCLASSEX wc;
	std::memset(&wc, 0, sizeof(WNDCLASSEX));
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc   = AppWndProc;
	wc.hInstance     = g_HInstance;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = WinWindow::AppWindowClass;
	RegisterClassEx(&wc);
}

WinApplication::~WinApplication()
{
	if (m_Window != nullptr) {
		MLOGE("Window not shutdown.");
	}
}

LRESULT CALLBACK WinApplication::AppWndProc(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam)
{
	return G_CurrentPlatformApplication->ProcessMessage(hwnd, msg, wParam, lParam);
}

int32 WinApplication::ProcessMessage(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CREATE:
		{
			LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
			return 0;
		}
		case WM_KEYDOWN:
		{
			const int32 keycode = HIWORD(lParam) & 0x1FF;
			KeyboardType key    = InputManager::GetKeyFromKeyCode(keycode);
			m_MessageHandler->OnKeyDown(key);
			return 0;
		}
		case WM_KEYUP:
		{
			const int32 keycode = HIWORD(lParam) & 0x1FF;
			KeyboardType key    = InputManager::GetKeyFromKeyCode(keycode);
			m_MessageHandler->OnKeyUp(key);
			return 0;
		}
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
		{
			MouseType button = MouseType::MOUSE_BUTTON_LEFT;
			int32 action     = 0;

			const int x = GET_X_LPARAM(lParam);
			const int y = GET_Y_LPARAM(lParam);

			Vector2 pos(x, y);

			if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP) {
				button = MouseType::MOUSE_BUTTON_LEFT;
			}
			else if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP) {
				button = MouseType::MOUSE_BUTTON_RIGHT;
			}
			else if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP) {
				button = MouseType::MOUSE_BUTTON_MIDDLE;
			}
			else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) {
				button = MouseType::MOUSE_BUTTON_4;
			}
			else {
				button = MouseType::MOUSE_BUTTON_5;
			}

			if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_XBUTTONDOWN)
			{
				action = 1;
				SetCapture(hwnd);
			}
			else
			{
				action = 0;
				ReleaseCapture();
			}

			if (action == 1) {
				m_MessageHandler->OnMouseDown(button, pos);
			}
			else {
				m_MessageHandler->OnMouseUp(button, pos);
			}

			if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONUP) {
				return TRUE;
			}

			return 0;
		}
		case WM_MOUSEMOVE:
		{
			const int x = GET_X_LPARAM(lParam);
			const int y = GET_Y_LPARAM(lParam);
			Vector2 pos(x, y);
			m_MessageHandler->OnMouseMove(pos);
			return 0;
		}
		case WM_MOUSEWHEEL:
		{
			const int x = GET_X_LPARAM(lParam);
			const int y = GET_Y_LPARAM(lParam);
			Vector2 pos(x, y);
			m_MessageHandler->OnMouseWheel((float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA, pos);
			return 0;
		}
		case WM_MOUSEHWHEEL:
		{
			const int x = GET_X_LPARAM(lParam);
			const int y = GET_Y_LPARAM(lParam);
			Vector2 pos(x, y);
			m_MessageHandler->OnMouseWheel((float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA, pos);
			return 0;
		}
		case WM_SIZE:
		{
			m_MessageHandler->OnSizeChanged(LOWORD(lParam), HIWORD(lParam));
			return 0;
		}
		case WM_PAINT:
		{
			m_MessageHandler->OnOSPaint();
			return 0;
		}
		case WM_CLOSE:
		{
			m_MessageHandler->OnRequestingExit();
			return 0;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void WinApplication::SetMessageHandler(GenericApplicationMessageHandler* messageHandler)
{
	GenericApplication::SetMessageHandler(messageHandler);
}

void WinApplication::PumpMessages()
{
	MSG msg = {};
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void WinApplication::Tick(float time, float delta)
{

}

std::shared_ptr<GenericWindow> WinApplication::MakeWindow(int32 width, int32 height, const char* title)
{
	return WinWindow::Make(width, height, title);
}

std::shared_ptr<GenericWindow> WinApplication::GetWindow()
{
	return m_Window;
}

void WinApplication::InitializeWindow(const std::shared_ptr<GenericWindow> window, const bool showImmediately)
{
	m_Window = std::dynamic_pointer_cast<WinWindow>(window);
	m_Window->Initialize(this);

	if (showImmediately) {
		m_Window->Show();
	}
}

void WinApplication::Destroy()
{
	if (m_Window != nullptr) 
	{
		m_Window->Destroy();
		m_Window = nullptr;
	}
}

std::shared_ptr<GenericApplication> GenericApplication::Create()
{
	G_CurrentPlatformApplication = std::make_shared<WinApplication>();
	return G_CurrentPlatformApplication;
}

GenericApplication& GenericApplication::GetApplication()
{
	return *G_CurrentPlatformApplication;
}