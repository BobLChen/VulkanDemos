#pragma once

#include "GenericWindow.h"
#include "GenericApplicationMessageHandler.h"

#include <memory>

class GenericApplication
{
public:

	GenericApplication();

	virtual ~GenericApplication();

	virtual void PumpMessages(const float timeDelta);

	virtual void Tick(const float timeDelta);

	virtual void Destroy();

	virtual std::shared_ptr<GenericWindow> MakeWindow(int32 width, int32 height, const char* title);

	virtual std::shared_ptr<GenericWindow> GetWindow();

	virtual void SetMessageHandler(const std::shared_ptr<GenericApplicationMessageHandler>& messageHandler);

	virtual void InitializeWindow(const std::shared_ptr<GenericWindow>& window, const bool showImmediately);

public:

	static std::shared_ptr<GenericApplication> Create();

	static GenericApplication& GetApplication();

protected:

	std::shared_ptr<GenericApplicationMessageHandler> m_MessageHandler;
};

