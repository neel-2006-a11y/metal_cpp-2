//
//  window.mm
//  metal_cpp
//
//  Created by Neel on 17/01/26.
//

#include "backend/windowBridge.h"

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

#include <Cocoa/Cocoa.h>
#include <QuartzCore/CAMetalLayer.h>
#include <Metal/Metal.h>

void AttachMetalLayer(const NativeWindowBridge bridge){
    CAMetalLayer* layer = (__bridge CAMetalLayer*)bridge.metalLayer;
    
    NSWindow* nsWindow = glfwGetCocoaWindow(bridge.glfwWindow);
    NSView* contentView = [nsWindow contentView];
    [contentView setWantsLayer:YES];
    [contentView setLayer:layer];
}
