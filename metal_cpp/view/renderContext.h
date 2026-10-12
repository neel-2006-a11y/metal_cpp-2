//
//  renderContext.h
//  metal_cpp
//
//  Created by Neel on 10/06/26.
//

#pragma once

#include "resource_managers/material_manager.h"
#include "resource_managers/mesh_manager.h"
#include "resource_managers/pipeline_manager.h"
#include "resource_managers/texture_manager.h"
#include "resource_managers/ModelLoader.h"
#include "resource_managers/depthStencilManager.h"
#include "resource_managers/sampler_manager.h"
#include "config.h"

struct RenderObject {
    MeshID mesh;
    MaterialID material;
    
    size_t object_index; // object's uniform's index in the object buffer
};

struct RenderQueue{
    std::vector<RenderObject> renderObjects;
};

struct ObjectBufferData{
    size_t objectStride;
    MTL::Buffer* objectBuffer;
};

struct MaterialBufferData{
    size_t materialStride;
    MTL::Buffer* materialBuffer;
};

struct RenderDevice {
    MTL::Device* device;
    MaterialManager* materialManager;
    MeshManager* meshManager;
    PipelineManager* pipelineManager;
    TextureManager* textureManager;
    ModelLoader* modelLoader;
    DepthStencilManager* depthStencilStateManager;
    SamplerManager* samplerManager;
    
    int curr_window_width, curr_window_height;
    int curr_fb_width, curr_fb_height;
};

struct RenderContext{
    MTL::Device* device;
    MTL::CommandBuffer* cmd;
    MTL::Texture* backBuffer;
    
    ViewUniforms viewUniforms;
    MTL::Buffer* frameBuffer;
    ObjectBufferData objectBufferData;
    MaterialBufferData materialBufferData;
    
    RenderQueue renderQ;
    
    MaterialManager* materialManager;
    MeshManager* meshManager;
    PipelineManager* pipelineManager;
    TextureManager* textureManager;
    ModelLoader* modelLoader;
    DepthStencilManager* depthStencilStateManager;
    SamplerManager* samplerManager;
    
    
    int curr_window_width, curr_window_height;
    int curr_fb_width, curr_fb_height;
    
    DebugSettings* debugData;
};
