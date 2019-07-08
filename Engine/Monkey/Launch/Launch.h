#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

#include "Application/AppModuleBase.h"
#include "Application/SlateApplication.h"

#include <string>
#include <vector>

extern std::shared_ptr<Engine> GameEngine;
extern std::shared_ptr<AppModuleBase> AppMode;

extern std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine);

int32 GuardedMain(const std::vector<std::string>& cmdLine);

int32 EnginePreInit(const std::vector<std::string>& cmdLine);

int32 EngineInit();

void EngineLoop();

void EngineExit();