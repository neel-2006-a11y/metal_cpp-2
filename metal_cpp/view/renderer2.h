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
#include "scene/RenderComponent.h"
#include "scene/SceneNode.h"
#include "view/Engine.h"
#include "resource_managers/ModelLoader.h"

#include "render_passes/composite_pass.h"
#include "render_passes/fluid_pass.h"
#include "render_passes/imgui_pass.h"
#include "render_passes/main_pass.h"
#include "render_passes/shadow_pass.h"
#include "render_passes/volumetric_pass.h"
#include "render_passes/render_graph.h"

#include "view/ResourceRegistry.h"
#include "view/FrameDataRegistry.h"

class Renderer2{
public:
    Renderer2(MTL::Device* device, CA::MetalLayer* layer, GLFWwindow* glfwWindow);
    // Core
    MTL::Device* device = nullptr;
    CA::MetalLayer* metalLayer = nullptr;
    GLFWwindow* glfwWindow = nullptr;
    MTL::CommandQueue* commandQueue = nullptr;
    
    // frame states
    MTL::CommandBuffer* cmd = nullptr;
    CA::MetalDrawable* currentDrawable;
    MTL::Texture* backbuffer;
    
    // screen dimensions
    int curr_window_width = 0, curr_window_height = 0;
    int curr_fb_width = 0, curr_fb_height = 0;
    
    // Frame Buffer
    FrameUniforms frameUniforms;
    MTL::Buffer* frameBuffer[FIF];
    size_t frameStride = 0;
    
    // Object Buffer
    MTL::Buffer* objectBuffer[FIF];
    size_t objectStride = 0;
    size_t maxObjects = 0;
    
    // Material Uniforms Buffer
    MTL::Buffer* materialBuffer[FIF];
    size_t materialStride = 0;
    size_t maxMaterials = 0;
    
    int frameIndex = 0;
    
    // managers
    TextureManager textureManager;
    MeshManager meshManager;
    MaterialManager materialManager;
    PipelineManager pipelineManager;
    SamplerManager samplerManager;
    DepthStencilManager depthStencilStateManager;
    
    ModelLoader modelLoader;
    
    
    // passes
    ShadowPass shadowPass;
    MainPass mainPass;
    VolumetricPass volumetricPass;
    FluidPass fluidPass;
    CompositePass compositePass;
    ImGuiPass imGuiPass;
    
    bool resizePasses = false;
    
    DebugSettings debug;
    
    //-----------------------
    void init();
    void render(Scene& root, ResourceRegistry& resourceRegistry, FrameDataRegistry& frameDataRegistry);
    void resize(int width, int height, int fb_width, int fb_height);
    
    RenderDevice renderDevice();
    
    
private:
    void beginFrame();
    void endFrame();
    
    RenderQueue makeRenderQueue(const std::unordered_map<MaterialID, int>& mat_mp, const std::vector<Renderable>& renderables);
    
    // buffer helpers
    void initFrameBuffer();
    void makeFrameUniforms(const FrameInfo& frameData);
    void uploadFrameUniforms();
    
    void initObjectBuffer(size_t maxObjCount);
    void uploadObjectUniforms(const std::vector<Renderable>& renderables);
    
    void initMaterialBuffer(size_t maxMatCount);
    void uploadAllMaterialUniforms(const std::vector<MaterialUniforms>& materials);
};
