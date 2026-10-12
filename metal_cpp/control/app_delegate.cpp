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

#include "view/ResourceRegistry.h"
#include "view/FrameDataRegistry.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    App* app = (App*)glfwGetWindowUserPointer(window);
    app->to_resize = true;
}

App::App(): scene(1,1){
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
    
    int window_w,window_h, fb_w,fb_h;
    glfwGetWindowSize(window, &window_w, &window_h);
    glfwGetFramebufferSize(window, &fb_w, &fb_h);
    
    //--------------
    // set initial drawable size drawable size
    //--------------
    metalLayer->setDrawableSize(CGSizeMake(fb_w, fb_h));
    
    std::cout << metalLayer->drawableSize().width << " " << metalLayer->drawableSize().height << std::endl;
    
    renderer2 = new Renderer2(device, windowBridge.metalLayer, windowBridge.glfwWindow, &frameSemaphore);
    engine = new Engine(windowBridge.glfwWindow);
}

App::~App(){
    glfwTerminate();
    windowBridge.metalLayer->release();
    
    delete renderer2;
    delete engine;
}

void App::Run(){
    engine->init();
    renderer2->init();
    RenderDevice rd = renderer2->renderDevice();
    engine->get_references(rd);
    scene.buildSampleScene(rd);
    
    while(!glfwWindowShouldClose(windowBridge.glfwWindow)){

        NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
        
        glfwPollEvents();
        
        ResourceRegistry rReg;
        FrameDataRegistry fdReg;
        
        dispatch_semaphore_wait(frameSemaphore, DISPATCH_TIME_FOREVER);
        engine->update(scene); // outputs a RenderView list
        engine->publish(rReg, fdReg);
        
        if(to_resize){
            resize();
            to_resize = false;
        }
        
        std::vector<RenderView> renderList;
        engine->make_renderViews(scene, renderList);
//        RenderView rv;
//        rv.camera = &scene.activeCamera();
//        rv.useDrawable = true;
//        renderList.push_back(rv);
        renderer2->render(scene, rReg, fdReg, renderList);
        
        // renderer2->render(scene, rReg, fdReg, renderList)
        // inside it will render each renderView in order and also perform some operation to change the materials textures for some of the portal materials
//        renderer2->render(scene, rReg, fdReg);
        
        pool->release();
    }
}

void App::resize(){
    renderer2->resize();
}
