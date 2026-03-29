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


void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    App* app = (App*)glfwGetWindowUserPointer(window);
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    app->resize(width, height, fbWidth, fbHeight);
}

App::App(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "Metal-cpp", nullptr, nullptr);
    // set user pointer to this
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    device = MTL::CreateSystemDefaultDevice();
    CA::MetalLayer* metalLayer = CA::MetalLayer::layer();
    metalLayer->setDevice(device);
    metalLayer->setPixelFormat(MTL::PixelFormatBGRA8Unorm_sRGB);
    
    windowBridge.glfwWindow = window;
    windowBridge.metalLayer = metalLayer;
    
    AttachMetalLayer(windowBridge);
    
    std::cout << metalLayer->drawableSize().width << " " << metalLayer->drawableSize().height << std::endl;
    
    renderer2 = new Renderer2(device, windowBridge.metalLayer, windowBridge.glfwWindow);
    engine = new Engine(renderer2);
    
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    engine->resize(fbWidth, fbHeight);
}

App::~App(){
    glfwTerminate();
    windowBridge.metalLayer->release();
    
    delete renderer2;
    delete engine;
}

void App::Run(){
    engine->init();
    while(!glfwWindowShouldClose(windowBridge.glfwWindow)){
        glfwPollEvents();
        engine->update();
        engine->render();
    }
}

void App::resize(int width, int height, int fbWidth, int fbHeight){
    engine->resize(fbWidth, fbHeight);
}
