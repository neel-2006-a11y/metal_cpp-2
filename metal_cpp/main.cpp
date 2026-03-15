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


#include "view/texture_generator.h"
#include "control/app_delegate.h"


#include <iostream>

int main(int argc, const char * argv[]) {
    
    NS::AutoreleasePool* autoreleasePool = NS::AutoreleasePool::alloc()->init();
    
    App app;
    app.Run();
    
    bool makeT = false;
    if(makeT){
        bool grad = true;
        int dim = 13;
        Image textures[dim];
        for (int i=0; i<dim; i++) {
            textures[i].resolution = 256;
            textures[i].pixels.resize(textures[i].resolution * textures[i].resolution);
        }
        TextureFactory::generate4x4(textures, textures[0].resolution, grad);
        for (int i=0; i<dim; i++){
            savePPM(textures[i], "texture"+std::to_string(i)+".ppm");
        }
    }
    autoreleasePool->release();
    
    return 0;
}
