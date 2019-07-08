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

@interface AppDelegate : NSObject <NSApplicationDelegate>

@end

// implemention MonkeyDelegate
@implementation AppDelegate
{
    
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
    g_ErrorLevel = GuardedMain(g_CmdLine);
}

@end

int main(int argc, const char * argv[])
{
    for (int32 i = 0; i < argc; ++i) {
        g_CmdLine.push_back(argv[i]);
    }
    
    [NSApplication sharedApplication];
    [NSApp setDelegate:[AppDelegate new]];
    [NSApp run];
    
    return g_ErrorLevel;
}
