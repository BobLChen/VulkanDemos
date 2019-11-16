#include "AndroidApplication.h"
#include "AndroidWindow.h"
#include "Common/Log.h"
#include "Engine.h"

#include <memory>
#include <map>
#include <string>

#include <android/native_activity.h>
#include <android/asset_manager.h>
#include <android_native_app_glue.h>
#include <sys/system_properties.h>

std::shared_ptr<AndroidApplication> G_CurrentPlatformApplication = nullptr;

AndroidApplication::AndroidApplication()
	: m_Window(nullptr)
{
	
}

AndroidApplication::~AndroidApplication()
{
	if (m_Window != nullptr)
	{
		MLOGE("Window not shutdown.");
	}
}

void AndroidApplication::SetMessageHandler(GenericApplicationMessageHandler* messageHandler)
{
	GenericApplication::SetMessageHandler(messageHandler);
}

void AndroidApplication::PumpMessages()
{
	
}

void AndroidApplication::Tick(float time, float delta)
{

}

std::shared_ptr<GenericWindow> AndroidApplication::MakeWindow(int32 width, int32 height, const char* title)
{
	return AndroidWindow::Make(width, height, title);
}

std::shared_ptr<GenericWindow> AndroidApplication::GetWindow()
{
	return m_Window;
}

void AndroidApplication::InitializeWindow(const std::shared_ptr<GenericWindow> window, const bool showImmediately)
{
	m_Window = std::dynamic_pointer_cast<AndroidWindow>(window);
	m_Window->Initialize(this);
	if (showImmediately)
	{
		m_Window->Show();
	}
}

void AndroidApplication::Destroy()
{
	if (m_Window != nullptr) {
		m_Window->Destroy();
		m_Window = nullptr;
	}
}

std::shared_ptr<GenericApplication> GenericApplication::Create()
{
	G_CurrentPlatformApplication = std::make_shared<AndroidApplication>();
	return G_CurrentPlatformApplication;
}

GenericApplication& GenericApplication::GetApplication()
{
	return *G_CurrentPlatformApplication;
}
