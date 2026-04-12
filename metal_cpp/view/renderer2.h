//
//  renderer2.h
//  metal_cpp
//
//  Created by Neel on 22/03/26.
//

#pragma once

#include "config.h"
#include "resource_managers/texture_manager.h"
#include "resource_managers/mesh_manager.h"
#include "resource_managers/material_manager.h"
#include "resource_managers/pipeline_manager.h"
#include "scene/RenderObject.h"
#include "scene/SceneNode.h"

class Renderer2{
public:
    Renderer2(MTL::Device* device, CA::MetalLayer* layer, GLFWwindow* glfwWindow);
    // Core
    MTL::Device* device = nullptr;
    CA::MetalLayer* metalLayer = nullptr;
    GLFWwindow* glfwWindow = nullptr;
    
    MTL::CommandQueue* commandQueue = nullptr;
    
    TextureManager* textureManager;
    MeshManager* meshManager;
    MaterialManager* materialManager;
    PipelineManager* pipelineManger;
    
    MTL::SamplerState* defaultSampler = nullptr;
    
    MTL::CommandBuffer* cmd = nullptr;
    
    // Frame Buffer
    MTL::Buffer* frameBuffer[FIF];
    size_t frameStride = 0;
    
    // Object Buffer
    MTL::Buffer* objectBuffer[FIF];
    size_t objectStride = 0;
    size_t maxObjects = 0;
    
    // Material Uniforms Buffer
    MTL::Buffer* materialBuffer = nullptr;
    size_t materialStride = 0;
    size_t maxMaterials = 0;
    int frameIndex = 0;
    
    Scene* scene = nullptr;
    SceneNode* root = nullptr;
    
    int curr_width, curr_height;
    DebugSettings debug;
    
    void initFrameBuffer();
    void uploadFrameUniforms(FrameUniforms& frame);
    
    void initObjectBuffer(size_t maxObjCount);
    void uploadAllObjectUniforms(const std::vector<Entity>& objects);
    void uploadObjectUniforms(const std::vector<ObjectUniforms>& objs);
    
    void initMaterialBuffer(size_t maxMatCount);
    void uploadAllMaterialUniforms(const std::vector<MaterialUniforms>& materials);
};
