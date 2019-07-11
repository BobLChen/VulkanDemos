#pragma once

#include "Common/Common.h"
#include "Common/Log.h"
#include "Application/GenericWindow.h"

#include <Cocoa/Cocoa.h>

@interface VulkanView : NSView

@end

@interface VulkanWindow : NSWindow <NSWindowDelegate, NSDraggingDestination>
{
    
}

@end

extern NSString* NSDraggingExited;
extern NSString* NSDraggingUpdated;
extern NSString* NSPrepareForDragOperation;
extern NSString* NSPerformDragOperation;
