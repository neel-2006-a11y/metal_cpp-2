//
//  app_delegate.h
//  metal_cpp
//
//  Created by Neel on 13/01/26.
//

#pragma once
#include "config.h"
#include "backend/windowBridge.h"
#include "view/Engine.h"
#include "view/renderer2.h"

class App
{
public:
    
    App();
    ~App();

    void Run();
    void resize(int width, int height, int fbWidth, int fbHeight);

private:
    MTL::Device* device;
    NativeWindowBridge windowBridge;
    Renderer2* renderer2;
    Engine* engine;
};
