#include "Engine.h"
#include "CocoaWindow.h"

#include "Launch/Launch.h"
#include "GenericPlatform/GenericPlatformTime.h"

#include <Cocoa/Cocoa.h>
#include <QuartzCore/CAMetalLayer.h>

enum LaunchErrorType
{
    OK = 0,
    FailedCreateAppModule    = -1,
    FailedPreInitAppModule   = -2,
    FailedInitAppModule      = -3,
};

std::shared_ptr<Engine> g_GameEngine;
std::shared_ptr<AppModuleBase> g_AppModule;

double g_LastTime = 0.0;
double g_CurrTime = 0.0;

int32 g_ErrorLevel = 0;
std::vector<std::string> g_CmdLine;

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
    
    if (!g_AppModule->Init()) {
        return FailedInitAppModule;
    }
    
    return errorLevel;
}

void EngineLoop()
{
    double nowT  = GenericPlatformTime::Seconds();
    double delta = nowT - g_LastTime;
    
    g_AppModule->Loop(g_CurrTime, delta);
    g_GameEngine->Tick(g_CurrTime, delta);
    
    g_LastTime = nowT;
    g_CurrTime = g_CurrTime + delta;
}

void EngineExit()
{
    g_AppModule->Exist();
    g_AppModule = nullptr;
    
    g_GameEngine->Exist();
    g_GameEngine = nullptr;
}

// ------------------------------ VulkanView ------------------------------
@implementation VulkanView

-(BOOL) wantsUpdateLayer {
    return YES;
}

+(Class) layerClass {
    return [CAMetalLayer class];
}

-(CALayer*) makeBackingLayer {
    CALayer* layer = [self.class.layerClass layer];
    CGSize viewScale = [self convertSizeToBacking: CGSizeMake(1.0, 1.0)];
    layer.contentsScale = MIN(viewScale.width, viewScale.height);
    return layer;
}

@end

// ------------------------------ VulkanWindow ------------------------------
@implementation VulkanWindow

@end

// ------------------------------ AppDelegate ------------------------------
@implementation AppDelegate
{
    CVDisplayLinkRef    m_DisplayLink;
}

- (void)setCMDLines:(const std::vector<std::string>&)cmdLines
{
    g_CmdLine = cmdLines;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)Sender;
{
    printf("applicationShouldTerminate\n");
    return NSTerminateNow;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    printf("applicationWillTerminate\n");
    CVDisplayLinkRelease(m_DisplayLink);
    EngineExit();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    printf("applicationShouldTerminateAfterLastWindowClosed\n");
    return YES;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    printf("applicationDidFinishLaunching\n");
    
    g_GameEngine = std::make_shared<Engine>();
    g_AppModule  = CreateAppMode(g_CmdLine);
    if (!g_AppModule) {
        g_ErrorLevel = FailedCreateAppModule;
        return;
    }
    
    g_ErrorLevel = EnginePreInit(g_CmdLine);
    if (g_ErrorLevel) {
        return;
    }
    
    g_LastTime = GenericPlatformTime::Seconds();
    
    g_ErrorLevel = EngineInit();
    if (g_ErrorLevel) {
        return;
    }
    
    CVDisplayLinkCreateWithActiveCGDisplays(&m_DisplayLink);
    CVDisplayLinkSetOutputCallback(m_DisplayLink, &DisplayLinkCallback, nil);
    CVDisplayLinkStart(m_DisplayLink);
}

#pragma mark Display loop callback function

static CVReturn DisplayLinkCallback(
                                    CVDisplayLinkRef displayLink,
                                    const CVTimeStamp* now,
                                    const CVTimeStamp* outputTime,
                                    CVOptionFlags flagsIn,
                                    CVOptionFlags* flagsOut,
                                    void* target
                                    ) {
    EngineLoop();
    
    if (g_GameEngine->IsRequestingExit()) {
        [NSApp terminate: nil];
    }
    
    return kCVReturnSuccess;
}

@end
