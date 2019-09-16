#include "Common/Common.h"
#include "Common/Log.h"

#include "Application/Mac/CocoaWindow.h"

#include <stdio.h>
#include <vector>
#include <string>

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

int main(int argc, const char * argv[])
{
    // cmdlines
    std::vector<std::string> cmdlines;
    for (int32 i = 0; i < argc; ++i) {
       cmdlines.push_back(argv[i]);
    }
    
    // new app delegate
    AppDelegate* delegate = [AppDelegate alloc];
    [delegate setCMDLines:cmdlines];
    
    // start application
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy : NSApplicationActivationPolicyRegular];
    [NSApp setDelegate:delegate];
    [NSApp run];
    
    // free delegate
    [delegate dealloc];
    
    return 0;
}
