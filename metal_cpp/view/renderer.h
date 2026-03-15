//
//  renderer.h
//  metal_cpp
//
//  Created by Neel on 13/01/26.
//

#pragma once
#include "config.h"
#include "QuartzCore/CAMetalLayer.hpp"
#include "view/mesh_factory.h"
#include "GLFW/glfw3.h"
#include "boids/boidManager.h"
#include "view/camera.h"
#include "view/mouseHandler.h"
#include "view/directional_light.h"

class Renderer
{
public:
    Renderer(MTL::Device* device, CA::MetalLayer* layer, GLFWwindow* glfwWindow);
    ~Renderer();
    void DrawFrame();
    void Resize();

private:
    void buildMeshes();
    void buildShaders();
    void updateMouse();
    void loadTextures();

    // Core
    MTL::Device* device = nullptr;
    MTL::CommandQueue* commandQueue = nullptr;
    CA::MetalLayer* metalLayer = nullptr;
    GLFWwindow* glfwWindow = nullptr;
    
    // Rendering state
    MTL::Texture* depthTexture = nullptr;
    MTL::DepthStencilState* depthState = nullptr;
    MTL::RenderPassDescriptor* renderPassDesc = nullptr;
    MTL::RenderPassDescriptor* shadowPassDesc = nullptr;
    
    // Pipelines
    MTL::RenderPipelineState *generalPipeline, *shadowPipeline, *ditherPipeline;

    // Meshes
    Mesh triangleMesh, quadMesh, cubeMesh, sphereMesh, curtainMesh;

    // camera
    Camera* camera = nullptr;
    MouseHandler* mouseHandler = nullptr;
    
    // shadow map
    DirectionalLight sun;
    MTL::Texture* shadowMap = nullptr;
    MTL::SamplerState* shadowSampler = nullptr;
    
    
    // window dimensions
    int curr_width, curr_height;
    int prev_width=0, prev_height=0;
    
    // cursor position
    double lastX, lastY;
    bool firstMouse = true;
    bool TAB_DOWN = false;
    bool TAB_WAS_DOWN = false;
    
    // dithering
    float sphereScale = 2.0;
    float cubeScale = 2.0;
    simd::float3 BayerScale = simd::float3(0.15); // 128
    MTL::Texture* halftoneArray = nullptr;
    MTL::SamplerState* ditherSampler = nullptr;
    
    //debug
    bool first_frame = true;
    float cubeAngle = 0;
};
