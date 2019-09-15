#pragma once

#include "GenericWindow.h"
#include "GenericApplicationMessageHandler.h"

#include <memory>

class GenericApplication
{
public:

	GenericApplication();

	virtual ~GenericApplication();

	virtual void PumpMessages();

	virtual void Tick(float time, float delta);

	virtual void Destroy();

	virtual std::shared_ptr<GenericWindow> MakeWindow(int32 width, int32 height, const char* title);

	virtual std::shared_ptr<GenericWindow> GetWindow();

	virtual void SetMessageHandler(GenericApplicationMessageHandler* messageHandler);

	virtual void InitializeWindow(const std::shared_ptr<GenericWindow> window, const bool showImmediately);

public:

	static std::shared_ptr<GenericApplication> Create();

	static GenericApplication& GetApplication();

protected:

	GenericApplicationMessageHandler* m_MessageHandler;
};

