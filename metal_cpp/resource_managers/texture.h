//
//  texture.h
//  metal_cpp
//
//  Created by Neel on 18/03/26.
//

#pragma once
#include <vector>
#include <cstdint>

#include "config.h"
#include "resource_managers/texture_types.h"


struct Texture{
    TextureDesc desc;
    
    // CPU-side data
    std::vector<uint8_t> raw_data;
    
    // GPU-handle
    void* gpuTexture = nullptr;
    
    bool uploaded = false;
};
