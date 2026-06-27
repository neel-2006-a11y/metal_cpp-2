//
//  Engine.h
//  metal_cpp
//
//  Created by Neel on 26/03/26.
//

#pragma once

#include "view/renderer2.h"
#include "render_passes/render_graph.h"
#include "render_passes/shadow_pass.h"
#include "render_passes/main_pass.h"
#include "render_passes/composite_pass.h"
#include "render_passes/volumetric_pass.h"
#include "render_passes/imgui_pass.h"
#include "view/directional_light.h"
#include "view/camera.h"
#include "view/mouseHandler.h"
#include "view/KeyBoardHandler.h"
#include "resource_managers/ModelLoader.h"
#include "scene/SceneNode.h"
#include "view/gizmoState.h"
#include "view/gizmoFactory.h"
#include "view/renderContext.h"
#include "fluid_stam/fluid_solver.h"
#include "passers/MTL_Texture_passer.h"

class Engine
{
public:
    Engine(Renderer2* renderer);
    void init();
    void update();
    void render();
    void resize(int width, int height, int fbWidth, int fbHeight);
    
private:
    void handleKeyboard();
    
    // time
    double lastTime = 0, currTime = 0;
    // screen dimensions
    int curr_window_width = 0, curr_window_height = 0;
    int curr_fb_width = 0, curr_fb_height = 0;
    int shadow_res = 2048;
    int volumeDownFact = 1;
    
    Renderer2* renderer = nullptr;
    
    RenderGraph renderGraph;
    
    Camera camera;
    MouseHandler mouseHandler;
    KeyBoardHandler keyboard;
    DirectionalLight sun;
    
    // scene
    SceneNode root;
    SceneNode* selected = nullptr;
    
    // gizmos
    std::vector<Gizmo> gizmos;
    
    // managers
    TextureManager textureManager;
    MeshManager meshManager;
    MaterialManager materialManager;
    PipelineManager pipelineManager;
    ModelLoader modelLoader;
    
    std::string assetDirectory;
    
    // pipelines
    PipelineID shadowPipelineID = INVALID_SHADER;
    PipelineID halftone_pipeID = INVALID_SHADER;
    PipelineID volumetricPipelineID = INVALID_SHADER;
    PipelineID compositePipelineID = INVALID_SHADER;
    PipelineID IDPipelineID = INVALID_SHADER;
    PipelineID LinePipelineID = INVALID_SHADER;
    
    // ShadowMap
    TextureID shadowMapID = INVALID_TEXTURE;
    MTL::SamplerState* shadowMap_sampler;
    
    // Textures
    TextureID depthTextureID = INVALID_TEXTURE;
    TextureID sceneColorTextureID = INVALID_TEXTURE;
    TextureID volumetricTextureID = INVALID_TEXTURE;
    TextureID blueNoiseTextureID = INVALID_TEXTURE;
    TextureID IDTextureID = INVALID_TEXTURE;
    
    // materials
    MaterialID halftoneMaterialID = INVALID_MATERIAL;
    TextureID halftone_textureID = INVALID_TEXTURE;
    MTL::SamplerState* halftone_sampler;
    
    MaterialID lineMaterialID = INVALID_MATERIAL;
    
    // passes
    ShadowPass shadowPass;
    MainPass mainPass;
    VolumetricPass volumetricPass;
    CompositePass compositePass;
    ImGuiPass imGuiPass;
    
    // Fluid
    Fluid fluid;
    
    
    FrameUniforms frameU;
};
