//
//  MTL_Texture_passer.cpp
//  metal_cpp
//
//  Created by Neel on 21/03/26.
//

#include "passers/MTL_Texture_passer.h"
#include "resource_managers/texture_types.h"
#include "converters/MTL_converters.h"

void uploadTextureToGPU(Texture& tex, MTL::Device* device){
    if(tex.uploaded) return;
    
    TextureDesc& d = tex.desc;
    
    MTL::TextureDescriptor* desc = buildMTLTextureDesc(d);
    
    MTL::Texture* gpuTex = device->newTexture(desc);
    desc->release();
    
    if(!gpuTex){
        std::cerr << "Failed to create MTL Texture\n";
        return;
    }
    // ---------
    // empty texture
    // ---------
    if(tex.raw_data.empty()){
        tex.gpuTexture = gpuTex;
        tex.uploaded = true;
        return;
    }
    
    // ---------
    // upload data
    // ---------
    size_t bpp = bytesPerPixel(d.format);
    
    size_t bytesPerRow = d.width * bpp;
    size_t bytesPerImage = d.width * d.height * bpp;
    
    MTL::Region region = MTL::Region(0, 0, d.width, d.height);
    
    uint8_t* base_ptr = tex.raw_data.data();
    
    if(d.layers>1){
        // teture2DArray
        for(int layer = 0; layer < d.layers; layer++){
            uint8_t* layer_ptr = base_ptr + layer * bytesPerImage;
            
            gpuTex->replaceRegion(region, 0, layer, layer_ptr, bytesPerRow, 0);
        }
    }else{
        // regular texture2D
        gpuTex->replaceRegion(region, 0, base_ptr, bytesPerRow);
    }
    
    tex.gpuTexture = (MTL::Texture*)gpuTex;
    tex.uploaded = true;
    
}

void uploadEmptyTextureToGPU(Texture& tex, MTL::Device* device){
    if(tex.uploaded) return;
    
    TextureDesc& d = tex.desc;
    
    MTL::TextureDescriptor* desc = buildMTLTextureDesc(d);
    
    MTL::Texture* gpuTex = device->newTexture(desc);
    desc->release();
    
    if(!gpuTex){
        std::cerr << "Failed to create MTL Texture\n";
        return;
    }
    
    tex.gpuTexture = gpuTex;
    tex.uploaded = true;
}
