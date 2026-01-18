//
//  window.h
//  metal_cpp
//
//  Created by Neel on 17/01/26.
//

#pragma once

#include <GLFW/glfw3.h>
#include <Metal/Metal.hpp>
#include <QuartzCore/CAMetalLayer.hpp>

struct NativeWindowBridge
{
    GLFWwindow* glfwWindow;
    CA::MetalLayer* metalLayer;
};

void AttachMetalLayer(const NativeWindowBridge);



