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
#include "view/directional_light.h"
#include "view/camera.h"
#include "view/mouseHandler.h"
#include "view/KeyBoardHandler.h"
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
    
    // pipelines
    PipelineID shadowPipelineID;
    PipelineID mainPipelineID;
    
    // ShadowMap
    TextureID shadowMapID;
    MTL::SamplerState* shadowMap_sampler;
    
    // depth Texture
    TextureID depthTextureID;
    
    // materials
    MaterialID halftoneMaterialID;
    TextureID halftone_textureID;
    MTL::SamplerState* halftone_sampler;
    
    // passes
    ShadowPass shadowPass;
    MainPass mainPass;
    
    FrameUniforms frameU;
};
