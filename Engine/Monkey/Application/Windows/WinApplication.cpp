#include "WinApplication.h"
#include "Common/Log.h"
#include "Engine.h"

#include <memory>
#include <map>
#include <string>

std::shared_ptr<WinApplication> G_CurrentPlatformApplication = nullptr;

WinApplication::WinApplication()
	: m_Window(nullptr)
{
	WNDCLASSEX wc;
	std::memset(&wc, 0, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS; // We want to receive double clicks
	wc.lpfnWndProc = AppWndProc;
	wc.hInstance = g_HInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = WinWindow::AppWindowClass;
	RegisterClassEx(&wc);
}

WinApplication::~WinApplication()
{
	if (m_Window != nullptr)
	{
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
	}
	return 0;

	case WM_KEYDOWN:
		
		return 0;

	case WM_KEYUP:
		
		return 0;

	case WM_PAINT:
		
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void WinApplication::SetMessageHandler(GenericApplicationMessageHandler* messageHandler)
{
	GenericApplication::SetMessageHandler(messageHandler);
}

void WinApplication::PumpMessages(const float deltaTime)
{
	MSG msg = {};
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (msg.message == WM_QUIT)
	{
		Engine::Get()->RequestExit(true);
	}
}

void WinApplication::Tick(const float deltaTime)
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
	if (showImmediately)
	{
		m_Window->Show();
	}
}

void WinApplication::Destroy()
{
	if (m_Window != nullptr) {
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
