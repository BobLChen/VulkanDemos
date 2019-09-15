#pragma once

#include "Application/GenericApplication.h"
#include "Application/GenericWindow.h"

#include "AndroidWindow.h"

#include <memory>
#include <vector>

class AndroidApplication : public GenericApplication
{
public:
	AndroidApplication();

	virtual ~AndroidApplication();

	virtual void PumpMessages(const float timeDelta) override;

	virtual void Tick(const float timeDelta) override;

	virtual void Destroy() override;

	virtual std::shared_ptr<GenericWindow> MakeWindow(int32 width, int32 height, const char* title) override;

	virtual std::shared_ptr<GenericWindow> GetWindow() override;

	virtual void SetMessageHandler(const std::shared_ptr<GenericApplicationMessageHandler>& messageHandler) override;

	virtual void InitializeWindow(const std::shared_ptr<GenericWindow>& window, const bool showImmediately) override;

private:
	std::shared_ptr<AndroidWindow> m_Window;
};
