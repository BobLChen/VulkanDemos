#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

#include "Application/AppModeBase.h"
#include "Application/SlateApplication.h"

#include <string>
#include <vector>

extern Engine* GameEngine;
extern AppModeBase* AppMode;

extern AppModeBase* CreateAppMode(const std::vector<std::string>& cmdLine);

int32 GuardedMain(const std::vector<std::string>& cmdLine);

int32 EnginePreInit(const std::vector<std::string>& cmdLine);

int32 EngineInit();

void EngineLoop();

void EngineExit();
