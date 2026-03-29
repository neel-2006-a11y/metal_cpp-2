//
//  pipeline_manager.h
//  metal_cpp
//
//  Created by Neel on 23/03/26.
//

#pragma once


#include <unordered_map>
#include <string>
#include "resource_managers/mesh_types.h"
#include "resource_managers/texture_types.h"
#include "resource_managers/resource_types.h"
#include "config.h"

struct PipelineDesc
{
    std::string vertexFunc;
    std::string fragmentFunc;
    
    VertexLayout vertexLayout;
    
    TextureFormat colorFormat;
    TextureFormat depthFormat;
};

class PipelineManager
{
public:
    PipelineManager(MTL::Device* device, MTL::Library* library);
    
    PipelineID createPipeline(PipelineDesc& desc);
    
    MTL::RenderPipelineState* get(PipelineID id);
    
private:
    MTL::Device* device;
    MTL::Library* library;
    
    std::unordered_map<PipelineID, MTL::RenderPipelineState*> pipelines;
    PipelineID nextID = 1;
};
