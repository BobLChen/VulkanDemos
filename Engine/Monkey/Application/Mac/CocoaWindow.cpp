#include "CocoaWindow.h"

#include <Cocoa/Cocoa.h>
#include <QuartzCore/CAMetalLayer.h>

NSString* NSDraggingExited          = @"NSDraggingExited";
NSString* NSDraggingUpdated         = @"NSDraggingUpdated";
NSString* NSPrepareForDragOperation = @"NSPrepareForDragOperation";
NSString* NSPerformDragOperation    = @"NSPerformDragOperation";

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



@end
