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
    MTL::RenderPipelineState *generalPipeline;
    MTL::RenderPipelineState *shadowPipeline;

    // Meshes
    Mesh triangleMesh, quadMesh, cubeMesh, sphereMesh;

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
    float BayerTransitions[4][8][8];
    
    float sphereScale = 20.0;
    float cubeScale = 4.0;
    
    simd::float3 BayerScale = simd::float3(128.0); // 128
    
    //debug
    bool first_frame = true;
    float cubeAngle = 0;
};
