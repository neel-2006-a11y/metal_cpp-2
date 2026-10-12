//
//  Engine.h
//  metal_cpp
//
//  Created by Neel on 26/03/26.
//

#pragma once

#include "view/mouseHandler.h"
#include "view/KeyBoardHandler.h"
#include "scene/scene.h"
#include "fluid_stam/fluid_interaction.h"
#include "view/ResourceRegistry.h"
#include "view/FrameDataRegistry.h"
#include "view/renderView.h"
#include "view/portal_texture_pool.h"

class Renderer2;

struct FrameInfo{
    Camera* camera;
    DirectionalLight* sun;
    float dt;
    float time;
};

class Engine
{
public:
    Engine(GLFWwindow* window);
    void init();
    void get_references(RenderDevice rd);
    void update(Scene& scene);
    void publish(ResourceRegistry& resourceRegistry, FrameDataRegistry& frameDataRegistry);
    
    void make_renderViews(Scene& scene, std::vector<RenderView>& views);
    TextureID renderView_portal(Portal* p, Camera* real_camera, int depth, Scene& scene, std::vector<RenderView>& views);
    
private:
    void handleKeyboard(Camera* camera);
    void handleMouse(Camera* camera);
    void makeFrameData(Scene& scene);
    void handleFluidInput();
    
    FrameInfo frameData;
    // time
    double lastTime = 0, currTime = 0;

    GLFWwindow* window;
    MouseHandler mouseHandler;
    KeyBoardHandler keyboard;
    
    // Systems
    FluidInteractor fluid;
    
    
    // portal_cameras
    std::vector<Camera*> portal_cameras;
    
    // portal textures
    RenderTexturePool portal_texture_pool;
};
