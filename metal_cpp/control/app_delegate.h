//
//  app_delegate.h
//  metal_cpp
//
//  Created by Neel on 13/01/26.
//

#pragma once
#include "config.h"
#include "backend/windowBridge.h"
#include "view/renderer.h"

class App
{
public:
    App();
    ~App();

    void Run();

private:
    MTL::Device* device;
    NativeWindowBridge windowBridge;
    Renderer* renderer;
};
