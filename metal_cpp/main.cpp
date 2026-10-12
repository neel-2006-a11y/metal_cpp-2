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
    
    bool run_app = true;
    bool makeT = false;
    bool makeT_hatch = false;
    
    
    if(run_app){
        App app;
        app.Run();
    }
    
    
    if(makeT){
        bool grad = true;
        int n = HALFTONE_MAT_DIM;
        int dim = HALFTONE_DIM;
        Image textures[dim];
        for (int i=0; i<dim; i++) {
            textures[i].resolution = 256;
            textures[i].pixels.resize(textures[i].resolution * textures[i].resolution);
        }
        switch(n){
            case 8:
                TextureFactory::generate8x8(textures, textures[0].resolution, grad);
                break;
            case 4:
                TextureFactory::generate4x4(textures, textures[0].resolution, grad);
                break;
            default:
                TextureFactory::generate2x2(textures, textures[0].resolution, grad);
                break;
        }
        for (int i=0; i<dim; i++){
            savePPM(textures[i], "halftone_textures/texture"+std::to_string(i)+".ppm");
        }
    }
    
    
    
    if(makeT_hatch){
        bool grad = true;
        int n = HATCH_MAT_DIM;
        int dim = HATCH_DIM;
        Image textures[dim];
        for (int i=0; i<dim; i++) {
            textures[i].resolution = 256;
            textures[i].pixels.resize(textures[i].resolution * textures[i].resolution);
        }
        TextureFactory::generatenxnHatch(textures, textures[0].resolution, n, grad);
        for (int i=0; i<dim; i++){
            savePPM(textures[i], "hatch_textures/texture"+std::to_string(i)+".ppm");
        }
    }
    
    return 0;
}
