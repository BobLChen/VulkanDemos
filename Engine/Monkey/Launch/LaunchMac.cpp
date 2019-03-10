#include "Common/Common.h"
#include "Common/Log.h"

#include <stdio.h>
#include <vector>
#include <string>

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

int32 g_ErrorLevel = 0;
std::vector<std::string> g_CmdLine;

// external guard main
extern int32 GuardedMain(const std::vector<std::string>& cmdLine);

// callback
static CVReturn DisplayLinkCallback(CVDisplayLinkRef displayLink,
                                    const CVTimeStamp* now,
                                    const CVTimeStamp* outputTime,
                                    CVOptionFlags flagsIn,
                                    CVOptionFlags* flagsOut,
                                    void* target) {
    
    // printf("display link call back\n");
    
    return kCVReturnSuccess;
}

// interface
@interface MonkeyView : NSView

@end

@interface MonkeyDelegate : NSObject <NSApplicationDelegate>

@end

// implementation MonkeyView
@implementation MonkeyView

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
    printf("scale=%fx%f\n", viewScale.width, viewScale.height);
    return layer;
}

@end

// implemention MonkeyDelegate
@implementation MonkeyDelegate
{
    CVDisplayLinkRef    m_DisplayLink;
    NSWindow*           m_Window;
    MonkeyView*         m_View;
}

- (void)awakeFromNib
{
    printf("awakeFromNib\n");
}

- (IBAction)requestQuit:(id)Sender
{
    printf("requestQuit\n");
}

- (IBAction)showAboutWindow:(id)Sender
{
    printf("showAboutWindow\n");
    [NSApp orderFrontStandardAboutPanel:Sender];
}

- (void)handleQuitEvent:(NSAppleEventDescriptor*)Event withReplyEvent:(NSAppleEventDescriptor*)ReplyEvent
{
    printf("handleQuitEvent\n");
    [self requestQuit:self];
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)Sender;
{
    printf("applicationShouldTerminate\n");
    return NSTerminateNow;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    printf("applicationWillTerminate\n");
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    printf("applicationShouldTerminateAfterLastWindowClosed\n");
    return YES;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    
    printf("applicationDidFinishLaunching\n");
    
    NSUInteger windowStyle = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;
    NSRect windowRect      = NSMakeRect(100, 100, 1600, 900);
    
    m_View   = [[MonkeyView alloc] initWithFrame:windowRect];
    m_View.wantsLayer = YES;
    
    m_Window = [[NSWindow alloc] initWithContentRect:windowRect styleMask:windowStyle backing:NSBackingStoreBuffered defer:NO];
    m_Window.contentView = m_View;
    (void)m_Window.orderFrontRegardless;
    (void)m_Window.makeMainWindow;
    (void)m_Window.makeKeyWindow;
    
    CVDisplayLinkCreateWithActiveCGDisplays(&m_DisplayLink);
    CVDisplayLinkSetOutputCallback(m_DisplayLink, &DisplayLinkCallback, nil);
    CVDisplayLinkStart(m_DisplayLink);
}

@end

int main(int argc, const char * argv[]) {
    [NSApplication sharedApplication];
    [NSApp setDelegate:[MonkeyDelegate new]];
    [NSApp run];
    return g_ErrorLevel;
}
