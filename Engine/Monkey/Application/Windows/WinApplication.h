#pragma once

#include "Application/GenericWindow.h"
#include "Application/GenericApplication.h"

#include "WinWindow.h"

#include <memory>
#include <vector>

class WinApplication : public GenericApplication
{
public:
	WinApplication();

	virtual ~WinApplication();

	virtual void PumpMessages() override;

	virtual void Tick(float time, float delta) override;

	virtual void Destroy() override;

	virtual std::shared_ptr<GenericWindow> MakeWindow(int32 width, int32 height, const char* title) override;

	virtual std::shared_ptr<GenericWindow> GetWindow() override;

	virtual void SetMessageHandler(GenericApplicationMessageHandler* messageHandler) override;

	virtual void InitializeWindow(const std::shared_ptr<GenericWindow> window, const bool showImmediately) override;

protected:

	static LRESULT CALLBACK AppWndProc(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam);

	int32 ProcessMessage(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam);

private:
	std::shared_ptr<WinWindow> m_Window;
};
