//
//  material.h
//  metal_cpp
//
//  Created by Neel on 22/03/26.
//

#pragma once

#include <vector>
#include <cstdint>
#include "resource_managers/resource_types.h"
#include "resource_managers/pipeline_manager.h"


struct TextureBinding{
    uint32_t slot;
    TextureID texture;
};

struct SamplerBinding{
    uint32_t slot;
    void* sampler;
};

struct Material
{
    PipelineDesc pipeDesc;
    PipelineID pipeID;
    
    std::vector<TextureBinding> textures;
    std::vector<SamplerBinding> samplers;
};
