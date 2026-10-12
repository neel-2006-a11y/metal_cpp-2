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
#include "render_passes/outline_pass.h"
#include "render_passes/render_graph.h"

#include "view/ResourceRegistry.h"
#include "view/FrameDataRegistry.h"

#include "view/renderView.h"

class Renderer2{
public:
    Renderer2(MTL::Device* device, CA::MetalLayer* layer, GLFWwindow* glfwWindow, dispatch_semaphore_t* frameSemaphore);
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
    OutlinePass outlinePass;
    VolumetricPass volumetricPass;
    FluidPass fluidPass;
    CompositePass compositePass;
    ImGuiPass imGuiPass;
    
    //-----------------------
    void init();
    void render(Scene& root, ResourceRegistry& resourceRegistry, FrameDataRegistry& frameDataRegistry, std::vector<RenderView>& renderViews);
    void resize();
    
    RenderDevice renderDevice();
    
    
private:
    void beginFrame(TextureID renderTarget = INVALID_TEXTURE);
    void endFrame(TextureID renderTarget = INVALID_TEXTURE);
    
    RenderQueue makeRenderQueue(const std::vector<Renderable>& renderables);
    
    // buffer helpers
    void initFrameBuffer();
    void makeFrameUniforms(const FrameInfo& frameData);
    void uploadFrameUniforms();
    
    void initObjectBuffer(size_t maxObjCount);
    void uploadObjectUniforms(const std::vector<Renderable>& renderables);
    
    void initMaterialBuffer(size_t maxMatCount);
    void uploadAllMaterialUniforms(std::unordered_map<int, MaterialID>& mat_mp, const std::vector<MaterialUniforms>& materials);
    
    ViewUniforms makeViewUniforms(Camera* camera, DirectionalLight* sun);
    
    
    // Core
    MTL::Device* device = nullptr;
    CA::MetalLayer* metalLayer = nullptr;
    GLFWwindow* glfwWindow = nullptr;
    MTL::CommandQueue* commandQueue = nullptr;
    
    // frame states
    MTL::CommandBuffer* cmd = nullptr;
    CA::MetalDrawable* currentDrawable = nullptr;
    MTL::Texture* backbuffer = nullptr;
    
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
    MTL::Buffer* materialBuffer[FIF] = {nullptr};
    size_t materialStride = 0;
    size_t maxMaterials = 0;
    
    int frameIndex = 0;
    
    bool resizePasses = false;
    
    DebugSettings debug;
    
    dispatch_semaphore_t* frameSemaphore = nullptr;
};
