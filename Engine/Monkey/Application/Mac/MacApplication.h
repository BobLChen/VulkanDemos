#pragma once

#include "Application/GenericApplication.h"
#include "Application/GenericWindow.h"

#include "MacWindow.h"

#include <memory>
#include <vector>

class MacApplication : public GenericApplication
{
public:
	MacApplication();

	virtual ~MacApplication();

	virtual void PumpMessages() override;

	virtual void Tick(float time, float delta) override;

	virtual void Destroy() override;

	virtual std::shared_ptr<GenericWindow> MakeWindow(int32 width, int32 height, const char* title) override;

	virtual std::shared_ptr<GenericWindow> GetWindow() override;

	virtual void SetMessageHandler(GenericApplicationMessageHandler* messageHandler) override;

	virtual void InitializeWindow(const std::shared_ptr<GenericWindow> window, const bool showImmediately) override;
    
private:
	
    std::shared_ptr<MacWindow> m_Window;
};
