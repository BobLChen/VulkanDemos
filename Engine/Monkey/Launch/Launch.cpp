#include "Configuration/Platform.h"
#include "Common/Common.h"
#include "Common/Log.h"
#include "Core/PixelFormat.h"
#include "Application/AppModeBase.h"
#include "Application/SlateApplication.h"
#include "Engine.h"

Engine GameEngine;
AppModeBase* AppMode;

extern AppModeBase* CreateAppMode(const char* cmdLine, int32 cmdShow);

int32 EnginePreInit(const char* cmdline)
{
	AppMode->PreInit();

	int width = AppMode->GetWidth();
	int height = AppMode->GetHeight();
	const char* title = AppMode->GetTitle().c_str();
	
	return GameEngine.PreInit(cmdline, width, height, title);
}

int32 EngineInit()
{
	AppMode->Setup(&GameEngine, GameEngine.GetVulkanRHI(), SlateApplication::Get().GetPlatformApplication(), SlateApplication::Get().GetPlatformApplication()->GetWindow());
	AppMode->Init();

	return GameEngine.Init();
}

void EngineLoop()
{
	GameEngine.Tick();
	AppMode->Loop();
}

void EngineExit()
{
	AppMode->Exist();
	GameEngine.Exist();
}

int32 GuardedMain(const char* cmdLine, int32 cmdShow)
{
	AppMode = CreateAppMode(cmdLine, cmdShow);
	if (AppMode == nullptr)
	{
		return -1;
	}

	int32 errorLevel = EnginePreInit(cmdLine);
	if (errorLevel != 0) 
	{
		return errorLevel;
	}

	errorLevel = EngineInit();

	while (!GameEngine.IsRequestingExit()) 
	{
		EngineLoop();
	}

	EngineExit();
	return errorLevel;
}
