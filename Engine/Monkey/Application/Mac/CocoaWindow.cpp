#include "CocoaWindow.h"

#include <Cocoa/Cocoa.h>
#include <QuartzCore/CAMetalLayer.h>

NSString* NSDraggingExited = @"NSDraggingExited";
NSString* NSDraggingUpdated = @"NSDraggingUpdated";
NSString* NSPrepareForDragOperation = @"NSPrepareForDragOperation";
NSString* NSPerformDragOperation = @"NSPerformDragOperation";

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

@implementation VulkanWindow
//
//@synthesize targetWindowMode;
//
//- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)style backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag
//{
//    m_WindowMode = WindowMode::Windowed;
//    m_DisplayReconfiguring = false;
//    m_RenderInitialized = false;
//    m_IsBeingOrderedFront = false;
//    m_Opacity = 0.0f;
//    acceptsInput = false;
//    zoomed = false;
//    isOnActiveSpace = false;
//    isBeingResized = false;
//
//    id newSelf = [super initWithContentRect:contentRect styleMask:style backing:bufferingType defer:flag];
//
//    if (newSelf)
//    {
//        zoomed = [super isZoomed];
//        isOnActiveSpace = [super isOnActiveSpace];
//        self.targetWindowMode = WindowMode::Windowed;
//
//        [super setAlphaValue:m_Opacity];
//        [super setRestorable:NO];
//        [super disableSnapshotRestoration];
//    }
//
//    return newSelf;
//}
//
//- (NSRect)vulkanFrame
//{
//    if ([self styleMask] & NSWindowStyleMaskTexturedBackground)
//    {
//        return [self frame];
//    }
//    else
//    {
//        return [[self contentView] frame];
//    }
//}
//
//- (NSView*)vulkanView
//{
//    return [self contentView];
//}
//
//- (void)setAcceptsInput:(bool)inAcceptsInput
//{
//    acceptsInput = inAcceptsInput;
//}
//
//- (void)setWindowMode:(WindowMode::Type)windowMode
//{
//    m_WindowMode = windowMode;
//    NSView* tempView = [self vulkanView];
//    [[NSNotificationCenter defaultCenter] postNotificationName:NSViewGlobalFrameDidChangeNotification object:tempView];
//}
//
//- (WindowMode::Type)windowMode
//{
//    return m_WindowMode;
//}
//
//- (void)setDisplayReconfiguring:(bool)isDisplayReconfiguring
//{
//    m_DisplayReconfiguring = isDisplayReconfiguring;
//}
//
//- (void)orderFrontAndMakeMain:(bool)isMain andKey:(bool)isKey
//{
//    if ([NSApp isHidden] == NO)
//    {
//        m_IsBeingOrderedFront = true;
//
//        [self orderFront:nil];
//
//        if (isMain && [self canBecomeMainWindow] && self != [NSApp mainWindow])
//        {
//            [self makeMainWindow];
//        }
//
//        if (isKey && [self canBecomeKeyWindow] && self != [NSApp keyWindow])
//        {
//            [self makeKeyWindow];
//        }
//
//        m_IsBeingOrderedFront = false;
//    }
//}
//
//- (BOOL)canBecomeMainWindow
//{
//    return acceptsInput && ![self ignoresMouseEvents];
//}
//
//- (BOOL)canBecomeKeyWindow
//{
//    return acceptsInput && ([self styleMask] != NSWindowStyleMaskBorderless);
//}
//
//- (BOOL)validateMenuItem:(NSMenuItem*)MenuItem
//{
//    return ([MenuItem action] == @selector(performClose:) || [MenuItem action] == @selector(miniaturize:) || [MenuItem action] == @selector(zoom:)) ? YES : [super validateMenuItem:MenuItem];
//}
//
//- (void)setAlphaValue:(CGFloat)WindowAlpha
//{
//    m_Opacity = WindowAlpha;
//    if (m_RenderInitialized)
//    {
//        [super setAlphaValue:WindowAlpha];
//    }
//}
//
//- (void)startRendering
//{
//    if (!m_RenderInitialized)
//    {
//        m_RenderInitialized = true;
//        [super setAlphaValue:m_Opacity];
//    }
//}
//
//- (bool)isRenderInitialized
//{
//    return m_RenderInitialized;
//}
//
//- (void)performClose:(id)Sender
//{
//
//}
//
//- (void)performZoom:(id)Sender
//{
//
//}
//
//- (void)zoom:(id)Sender
//{
//    zoomed = !zoomed;
//    [super zoom:Sender];
//}
//
//- (void)keyDown:(NSEvent*)Event
//{
//
//}
//
//- (void)keyUp:(NSEvent*)Event
//{
//
//}
//
//- (NSSize)window:(NSWindow *)window willUseFullScreenContentSize:(NSSize)proposedSize
//{
//    return proposedSize;
//}
//
//- (void)windowWillEnterFullScreen:(NSNotification*)Notification
//{
//    if (self.targetWindowMode == WindowMode::Windowed)
//    {
//        self.targetWindowMode = WindowMode::WindowedFullscreen;
//    }
//}
//
//- (void)windowDidEnterFullScreen:(NSNotification*)Notification
//{
//    m_WindowMode = self.targetWindowMode;
//}
//
//- (void)windowWillExitFullScreen:(NSNotification *)Notification
//{
//    if (self.targetWindowMode != WindowMode::Windowed)
//    {
//        self.targetWindowMode = WindowMode::Windowed;
//    }
//}
//
//- (void)windowDidExitFullScreen:(NSNotification*)Notification
//{
//    m_WindowMode = WindowMode::Windowed;
//    self.targetWindowMode = WindowMode::Windowed;
//}
//
//- (void)windowDidBecomeMain:(NSNotification*)Notification
//{
//    if ([NSApp isHidden] == NO)
//    {
//        [self orderFrontAndMakeMain:false andKey:false];
//    }
//}
//
//- (void)windowDidResignMain:(NSNotification*)Notification
//{
//    [self setMovable: YES];
//    [self setMovableByWindowBackground: NO];
//}
//
//- (void)windowWillMove:(NSNotification*)Notification
//{
//
//}
//
//- (void)windowDidMove:(NSNotification*)Notification
//{
//    zoomed = [self isZoomed];
//    NSView* tempView = [self vulkanView];
//    [[NSNotificationCenter defaultCenter] postNotificationName:NSViewGlobalFrameDidChangeNotification object:tempView];
//}
//
//- (NSRect)constrainFrameRect:(NSRect)frameRect toScreen:(NSScreen*)screen
//{
//    NSRect constrainedRect = [super constrainFrameRect:frameRect toScreen:screen];
//
//    if (self.targetWindowMode == WindowMode::Windowed)
//    {
//        constrainedRect.origin.y -= frameRect.size.height - constrainedRect.size.height;
//        constrainedRect.size = frameRect.size;
//    }
//
//    return constrainedRect;
//}
//
//- (void)windowDidChangeScreen:(NSNotification*)notification
//{
//    if (m_DisplayReconfiguring)
//    {
//        NSScreen* screen = [self screen];
//        NSRect frame = [self frame];
//        NSRect visibleFrame = [screen visibleFrame];
//        if (NSContainsRect(visibleFrame, frame) == NO)
//        {
//            if (frame.size.width > visibleFrame.size.width || frame.size.height > visibleFrame.size.height)
//            {
//                NSRect newFrame;
//                newFrame.size.width = frame.size.width > visibleFrame.size.width ? visibleFrame.size.width : frame.size.width;
//                newFrame.size.height = frame.size.height > visibleFrame.size.height ? visibleFrame.size.height : frame.size.height;
//                newFrame.origin = visibleFrame.origin;
//
//                [self setFrame:newFrame display:NO];
//            }
//            else
//            {
//                NSRect intersection = NSIntersectionRect(visibleFrame, frame);
//                NSPoint origin = frame.origin;
//                if (intersection.size.width > 0 && intersection.size.height > 0)
//                {
//                    CGFloat x = frame.size.width - intersection.size.width;
//                    CGFloat y = frame.size.height - intersection.size.height;
//
//                    if (intersection.size.width + intersection.origin.x >= visibleFrame.size.width + visibleFrame.origin.x)
//                    {
//                        origin.x -= x;
//                    }
//                    else if (origin.x < visibleFrame.origin.x)
//                    {
//                        origin.x += x;
//                    }
//
//                    if (intersection.size.height + intersection.origin.y >= visibleFrame.size.height + visibleFrame.origin.y)
//                    {
//                        origin.y -= y;
//                    }
//                    else if (origin.y < visibleFrame.origin.y)
//                    {
//                        origin.y += y;
//                    }
//                }
//                else
//                {
//                    origin = visibleFrame.origin;
//                }
//
//                [self setFrameOrigin:origin];
//            }
//        }
//    }
//    else
//    {
//
//    }
//}
//
//- (void)windowWillStartLiveResize:(NSNotification*)notification
//{
//
//}
//
//- (void)windowDidEndLiveResize:(NSNotification*)notification
//{
//
//}
//
//- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize
//{
//    return frameSize;
//}
//
//- (void)windowDidResize:(NSNotification*)notification
//{
//    zoomed = [self isZoomed];
//}
//
//- (void)windowWillClose:(NSNotification*)notification
//{
//    [self setDelegate:nil];
//}
//
//- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)Sender
//{
//    return NSDragOperationGeneric;
//}
//
//- (void)draggingExited:(id <NSDraggingInfo>)Sender
//{
//
//}
//
//- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)Sender
//{
//    return NSDragOperationGeneric;
//}
//
//- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)Sender
//{
//    return YES;
//}
//
//- (BOOL)performDragOperation:(id <NSDraggingInfo>)Sender
//{
//    return YES;
//}

@end
