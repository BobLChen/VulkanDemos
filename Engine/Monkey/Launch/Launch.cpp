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

std::shared_ptr<Engine> g_GameEngine;
std::shared_ptr<AppModuleBase> g_AppModule;
std::chrono::time_point<std::chrono::steady_clock> g_LastTime;
float g_CurrTime = 0.0f;

int32 EnginePreInit(const std::vector<std::string>& cmdLine)
{
    int32 width  = g_AppModule->GetWidth();
    int32 height = g_AppModule->GetHeight();
    const char* title = g_AppModule->GetTitle().c_str();
    
    int32 errorLevel = g_GameEngine->PreInit(cmdLine, width, height, title);
	if (errorLevel) {
		return errorLevel;
	}
    
	if (!g_AppModule->PreInit()) {
		return FailedPreInitAppModule;
	}

    return errorLevel;
}

int32 EngineInit()
{
	int32 errorLevel = g_GameEngine->Init();
	if (errorLevel) {
		return errorLevel;
	}

	g_AppModule->Setup(g_GameEngine);
	
	if (!g_AppModule->Init()) {
		return FailedInitAppModule;
	}

	return errorLevel;
}

void EngineLoop()
{
	auto tNow   = std::chrono::high_resolution_clock::now();
	auto tDiff  = std::chrono::duration<double, std::milli>(g_LastTime - tNow).count();
	float delta = (float)tDiff / 1000.0f;
	
	g_AppModule->Loop(g_CurrTime, delta);
	g_GameEngine->Tick(g_CurrTime, delta);
	
	g_LastTime = tNow;
	g_CurrTime = g_CurrTime + delta;
}

void EngineExit()
{
	g_AppModule->Exist();
	g_GameEngine->Exist();
}

int32 GuardedMain(const std::vector<std::string>& cmdLine)
{
	g_LastTime   = std::chrono::high_resolution_clock::now();
    g_GameEngine = std::make_shared<Engine>();

	g_AppModule = CreateAppMode(cmdLine);
	if (!g_AppModule) {
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

	while (!g_GameEngine->IsRequestingExit()) {
		EngineLoop();
	}

	EngineExit();
	return errorLevel;
}
