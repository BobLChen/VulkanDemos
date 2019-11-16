#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

#include "Application/AppModuleBase.h"
#include "Application/Application.h"

#include <string>
#include <vector>

extern std::shared_ptr<Engine> g_GameEngine;
extern std::shared_ptr<AppModuleBase> g_AppModule;

extern std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine);

int32 GuardedMain(const std::vector<std::string>& cmdLine);

int32 EnginePreInit(const std::vector<std::string>& cmdLine);

int32 EngineInit();

void EngineLoop();

void EngineExit();