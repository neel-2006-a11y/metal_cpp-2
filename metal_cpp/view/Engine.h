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
#include "view/directional_light.h"
#include "view/camera.h"
#include "view/mouseHandler.h"
#include "view/KeyBoardHandler.h"
#include "resource_managers/ModelLoader.h"

class Engine
{
public:
    Engine(Renderer2* renderer);
    void init();
    void update();
    void render();
    void resize(int width, int height);
    
private:
    void handleKeyboard();
    
    // time
    double lastTime = 0, currTime = 0;
    // screen dimensions
    int curr_width = 0, curr_height = 0;
    int shadow_res = 2048;
    int volumeDownFact = 2;
    
    Renderer2* renderer;
    
    RenderGraph renderGraph;
    
    Camera camera;
    MouseHandler mouseHandler;
    KeyBoardHandler keyboard;
    Scene scene;
    DirectionalLight sun;
    
    // managers
    TextureManager textureManager;
    MeshManager meshManager;
    MaterialManager materialManager;
    PipelineManager pipelineManager;
    ModelLoader modelLoader;
    
    std::string assetDirectory;
    
    // pipelines
    PipelineID shadowPipelineID;
    PipelineID halftone_pipeID;
    PipelineID volumetricPipelineID;
    PipelineID compositePipelineID;
    
    // ShadowMap
    TextureID shadowMapID;
    MTL::SamplerState* shadowMap_sampler;
    
    // Textures
    TextureID depthTextureID;
    TextureID sceneColorTextureID;
    TextureID volumetricTextureID;
    
    // materials
    MaterialID halftoneMaterialID;
    TextureID halftone_textureID;
    MTL::SamplerState* halftone_sampler;
    
    // passes
    ShadowPass shadowPass;
    MainPass mainPass;
    VolumetricPass volumetricPass;
    CompositePass compositePass;
    
    
    FrameUniforms frameU;
};
