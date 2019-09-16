#pragma once

#include "Common/Common.h"
#include "Common/Log.h"
#include "Application/GenericWindow.h"
#include "Application/GenericApplicationMessageHandler.h"

#include <Cocoa/Cocoa.h>

// ------------------------------ VulkanView ------------------------------
@interface VulkanView : NSView

@end

// ------------------------------ VulkanWindow ------------------------------
@interface VulkanWindow : NSWindow <NSWindowDelegate, NSDraggingDestination>

-(void)SetMessageHandler:(GenericApplicationMessageHandler*)messageHandler;

@end

// ------------------------------ AppDelegate ------------------------------
@interface AppDelegate : NSObject <NSApplicationDelegate>

- (void)setCMDLines:(const std::vector<std::string>&)cmdLines;

@end
