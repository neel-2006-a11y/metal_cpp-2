//
//  main.cpp
//  metal_cpp
//
//  Created by Neel on 13/01/26.
//

#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>


#include "control/app_delegate.h"


#include <iostream>

int main(int argc, const char * argv[]) {
    
    NS::AutoreleasePool* autoreleasePool = NS::AutoreleasePool::alloc()->init();
    
    App app;
    app.Run();
    
    autoreleasePool->release();
    
    return 0;
}
