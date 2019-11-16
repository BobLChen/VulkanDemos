#include "Common/Common.h"
#include "Common/Log.h"
#include "Launch.h"
#include "Engine.h"
#include "Application/Android/AndroidWindow.h"

#include <stdio.h>
#include <string>
#include <vector>

#include <jni.h>
#include <android/native_activity.h>
#include <android/asset_manager.h>
#include <android_native_app_glue.h>
#include <sys/system_properties.h>

std::vector<std::string> g_CmdLine;

static bool initialized = false;
static bool active      = false;

static void InitAppEngine()
{
    g_GameEngine = std::make_shared<Engine>();
    g_AppModule  = CreateAppMode(g_CmdLine);

    if (g_AppModule == nullptr) {
        MLOGE("Failed create app.")
    }
    
    int32 errorLevel = EnginePreInit(g_CmdLine);
    if (errorLevel != 0) {
        MLOGE("Failed init engine.");
    }
    
    errorLevel = EngineInit();
    if (errorLevel != 0) {
        MLOGE("Failed init engine.");
    }
}

static void ProcessCommand(struct android_app *app, int32_t cmd) 
{
    switch (cmd) {
        case APP_CMD_INIT_WINDOW: 
        {
            if (app->window) 
            {
                InitAppEngine();
                initialized = true;
            }
            MLOG("APP_CMD_INIT_WINDOW");
            break;
        }
        case APP_CMD_GAINED_FOCUS: 
        {
            active = true;
            MLOG("APP_CMD_GAINED_FOCUS");
            break;
        }
        case APP_CMD_LOST_FOCUS: 
        {
            active = false;
            MLOG("APP_CMD_LOST_FOCUS");
            break;
        }
    }
}

void android_main(android_app* app)
{
    MLOG("................................ 进入android_main ................................\n");

    g_AndroidApp = app;
    g_AndroidApp->onAppCmd = ProcessCommand;

    while (true) 
    {
        int events = -1;
        struct android_poll_source *source = nullptr;
        while (ALooper_pollAll(active ? 0 : -1, NULL, &events, (void **)&source) >= 0) 
        {
            if (source) 
            {
                source->process(app, source);
            }

            if (app->destroyRequested != 0) 
            {
                return;
            }
        }

        if (initialized && active && !g_GameEngine->IsRequestingExit())
        {
            EngineLoop();
        }
    }
    
    MLOG("................................ Android Window inited ................................");
}