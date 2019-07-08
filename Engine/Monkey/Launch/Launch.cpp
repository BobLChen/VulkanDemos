#include "Configuration/Platform.h"

#include "Engine.h"
#include "Launch.h"

#include <string>
#include <vector>
#include <chrono>

enum LaunchErrorType
{
	OK = 0,
	FailedCreateAppModule	= -1,
	FailedPreInitAppModule	= -2,
	FailedInitAppModule		= -3,
};

std::shared_ptr<Engine> GameEngine;
std::shared_ptr<AppModuleBase> AppMode;

int32 EnginePreInit(const std::vector<std::string>& cmdLine)
{
    int32 width  = AppMode->GetWidth();
    int32 height = AppMode->GetHeight();
    const char* title = AppMode->GetTitle().c_str();
    
    int32 errorLevel = GameEngine->PreInit(cmdLine, width, height, title);
    
	if (!AppMode->PreInit()) {
		return FailedPreInitAppModule;
	}

    return errorLevel;
}

int32 EngineInit()
{
	
	AppMode->Setup(GameEngine, GameEngine->GetVulkanRHI(), GameEngine->GetApplication()->GetPlatformApplication(), GameEngine->GetApplication()->GetPlatformApplication()->GetWindow());
	if (!AppMode->Init()) {
		return FailedInitAppModule;
	}

	return GameEngine->Init();
}

void EngineLoop()
{
	auto tStart = std::chrono::high_resolution_clock::now();

	GameEngine->PumpMessage();
	if (GameEngine->IsRequestingExit())
	{
		return;
	}
	GameEngine->Tick();
	AppMode->Loop();
    
    auto tEnd  = std::chrono::high_resolution_clock::now();
    auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
    GameEngine->SetDeltaTime((float)tDiff / 1000.0f);
}

void EngineExit()
{
	AppMode->Exist();
	GameEngine->Exist();
}

int32 GuardedMain(const std::vector<std::string>& cmdLine)
{
    GameEngine = std::make_shared<Engine>();

	AppMode = CreateAppMode(cmdLine);
	if (!AppMode) {
		return FailedCreateAppModule;
	}
	
	int32 errorLevel = EnginePreInit(cmdLine);
	if (errorLevel) {
		return errorLevel;
	}

	errorLevel = EngineInit();
	if (errorLevel) {
		return errorLevel;
	}

	while (!GameEngine->IsRequestingExit()) {
		EngineLoop();
	}

	EngineExit();
	return errorLevel;
}
