#include "Configuration/Platform.h"
#include "Core/PixelFormat.h"

#include "Engine.h"
#include "Launch.h"

#include <string>
#include <vector>

Engine* GameEngine = nullptr;
AppModeBase* AppMode = nullptr;

int32 EnginePreInit(const std::vector<std::string>& cmdLine)
{
	AppMode->PreInit();

	int32 width  = AppMode->GetWidth();
	int32 height = AppMode->GetHeight();
	const char* title = AppMode->GetTitle().c_str();
	
	return GameEngine->PreInit(cmdLine, width, height, title);
}

int32 EngineInit()
{
	AppMode->Setup(GameEngine, GameEngine->GetVulkanRHI(), SlateApplication::Get().GetPlatformApplication(), SlateApplication::Get().GetPlatformApplication()->GetWindow());
	AppMode->Init();

	return GameEngine->Init();
}

void EngineLoop()
{
	GameEngine->PumpMessage();
	if (GameEngine->IsRequestingExit())
	{
		return;
	}
	GameEngine->Tick();
	AppMode->Loop();
}

void EngineExit()
{
	AppMode->Exist();
	GameEngine->Exist();
}

int32 GuardedMain(const std::vector<std::string>& cmdLine)
{
    GameEngine = new Engine();
    
	AppMode = CreateAppMode(cmdLine);
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

	while (!GameEngine->IsRequestingExit())
	{
		EngineLoop();
	}

	EngineExit();
	return errorLevel;
}
