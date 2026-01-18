//
//  app_delegate.cpp
//  metal_cpp
//
//  Created by Neel on 13/01/26.
//

#include "control/app_delegate.h"

#include <GLFW/glfw3.h>
#include <Metal/Metal.hpp>
#include <QuartzCore/CAMetalLayer.hpp>

App::App(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "Metal-cpp", nullptr, nullptr);
    
    device = MTL::CreateSystemDefaultDevice();
    CA::MetalLayer* metalLayer = CA::MetalLayer::layer();
    metalLayer->setDevice(device);
    metalLayer->setPixelFormat(MTL::PixelFormatBGRA8Unorm_sRGB);
    
    windowBridge.glfwWindow = window;
    windowBridge.metalLayer = metalLayer;
    
    AttachMetalLayer(windowBridge);
    
    renderer = new Renderer(device, windowBridge.metalLayer, windowBridge.glfwWindow);
}

App::~App(){
    glfwTerminate();
    windowBridge.metalLayer->release();
    
    delete renderer;
}

void App::Run(){
    while(!glfwWindowShouldClose(windowBridge.glfwWindow)){
        glfwPollEvents();
        renderer->DrawFrame();
    }
}
