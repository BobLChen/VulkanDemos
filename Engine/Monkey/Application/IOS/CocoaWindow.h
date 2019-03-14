#pragma once

#include "Common/Common.h"
#include "Common/Log.h"
#include "Application/GenericWindow.h"

#include <Cocoa/Cocoa.h>

@interface VulkanView : NSView

@end


@interface VulkanWindow : NSWindow <NSWindowDelegate, NSDraggingDestination>
{
//    WindowMode::Type m_WindowMode;
//    bool m_DisplayReconfiguring;
//    bool m_RenderInitialized;
//    bool m_IsBeingOrderedFront;
//    float m_Opacity;
//@public
//    bool acceptsInput;
//    bool zoomed;
//    bool isOnActiveSpace;
//    bool isBeingResized;
}

//@property (assign) WindowMode::Type targetWindowMode;
//
//- (NSRect)vulkanFrame;
//
//- (VulkanView*)vulkanView;
//
//- (void)setAcceptsInput:(bool)acceptsInput;
//
//- (void)setWindowMode:(WindowMode::Type)windowMode;
//
//- (WindowMode::Type)windowMode;
//
//- (void)setDisplayReconfiguring:(bool)isDisplayReconfiguring;
//
//- (void)orderFrontAndMakeMain:(bool)isMain andKey:(bool)isKey;
//
//- (void)startRendering;
//
//- (bool)isRenderInitialized;

@end

extern NSString* NSDraggingExited;
extern NSString* NSDraggingUpdated;
extern NSString* NSPrepareForDragOperation;
extern NSString* NSPerformDragOperation;
