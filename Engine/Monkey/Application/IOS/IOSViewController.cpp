#include "Application/IOS/IOSWindow.h"
#include "Launch/Launch.h"
#include "Engine.h"
#include "IOSViewController.h"

#include <QuartzCore/CAMetalLayer.h>

@implementation VulkanView

+(Class) layerClass { return [CAMetalLayer class]; }

@end


@implementation VulkanViewController
{
    CADisplayLink* displayLink;
}

-(void) renderLoop
{
    while (!GameEngine->IsRequestingExit())
    {
        EngineLoop();
    }
}

-(void) viewDidLoad
{
    [super viewDidLoad];
    
    VulkanView* vulkanView = [[VulkanView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    vulkanView.backgroundColor = [UIColor darkGrayColor];
    vulkanView.clearsContextBeforeDrawing = NO;
    vulkanView.multipleTouchEnabled = YES;
    
    g_IOSView = (__bridge void*)vulkanView;
    
    CGRect rect = [UIScreen mainScreen].bounds;
    [self.view addSubview:vulkanView];
    self.view.contentScaleFactor = [[UIScreen mainScreen] scale];
    
    std::vector<std::string> cmdLines;
    cmdLines.push_back([NSBundle.mainBundle.resourcePath stringByAppendingString: @"/"].UTF8String);
    
    GameEngine = new Engine();
    AppMode    = CreateAppMode(cmdLines);
    AppMode->SetWidth((int32)rect.size.width);
    AppMode->SetHeight((int32)rect.size.height);
    
    if (AppMode == nullptr)
    {
        MLOGE("Failed create app.")
        // TODO:Quit app
    }
    
    int32 errorLevel = EnginePreInit(cmdLines);
    if (errorLevel != 0)
    {
        MLOGE("Failed init engine.");
    }
    
    errorLevel = EngineInit();
    
    uint32_t fps = 60;
    displayLink = [CADisplayLink displayLinkWithTarget: self selector: @selector(renderLoop)];
    [displayLink setFrameInterval: 60 / fps];
    [displayLink addToRunLoop: NSRunLoop.currentRunLoop forMode: NSDefaultRunLoopMode];
}

@end



