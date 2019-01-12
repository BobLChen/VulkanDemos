#include "Configuration/Platform.h"
#include "Common/Common.h"
#include "Common/Log.h"
#include "Engine.h"
#include "Core/PixelFormat.h"

Engine GameEngine;

int32 EnginePreInit(const char* cmdline, int32 width, int height, const char* title)
{
	return GameEngine.PreInit(cmdline, width, height, title);
}

int32 EngineInit()
{
	return GameEngine.Init();
}

void EngineLoop()
{
	GameEngine.Tick();
}

void EngineExit()
{
	GameEngine.Exist();
}

int32 GuardedMain(const char* cmdLine, int32 cmdShow)
{
	int32 errorLevel = EnginePreInit(cmdLine, 1280, 760, "Game");
	if (errorLevel != 0) {
		return errorLevel;
	}

	errorLevel = EngineInit();

	while (!GameEngine.IsRequestingExit()) {
		EngineLoop();
	}

	EngineExit();
	return errorLevel;
}
