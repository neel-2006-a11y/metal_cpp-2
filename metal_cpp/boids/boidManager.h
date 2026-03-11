//
//  boidManager.h
//  metal_cpp
//
//  Created by Neel on 18/01/26.
//

#pragma once
#include "boids/instance.h"
#include "config.h"
#include "view/mesh_factory.h"
#include <Metal/Metal.hpp>
#include <GLFW/glfw3.h>

class BoidManager {
public:
    uint32_t kCount = 8192*4;
    float cellSize = 0.05f;
    uint32_t gridDim;
    uint32_t parity = 0;
    
    BoidManager(MTL::Device* device, MTL::CommandQueue* commandQueue, GLFWwindow* glfwWindow);
    ~BoidManager();
    
    void update();
    void update_grid();
    void update_grid_2();
    void renderBoids(CA::MetalDrawable* drawable);
    
    // update_grid
    MTL::Buffer* boidIn;
    MTL::Buffer* boidOut;
    MTL::Buffer* gridBuffer;
    
    // update_grid_2
    MTL::Buffer* boidIn_2;
    MTL::Buffer* boidOut_2;
    MTL::Buffer* cellStartBuffer;
    MTL::Buffer* cellFinishBuffer;
    MTL::Buffer* cellCountBuffer;
    MTL::Buffer* cellIndicesBuffer;
    MTL::Buffer* gridParamsBuffer;
//    MTL::Buffer* simParamsBuffer;
    
private:
    void buildComputePipeline();
    void buildComputePipeline2();
    void buildBuffers();
    void buildBuffers2();
    void updateFlow(FlowField& f, float dt);
    
    FlowField flow;
    
    MTL::Device* device;
    MTL::ComputePipelineState* computePipeline;
    MTL::ComputePipelineState* clearGrid_pass, *insertBoids_pass, *update_pass;
    MTL::ComputePipelineState* clearCellCount_pass, *countBoids_pass, *scatter_pass, *update_2_pass;
    
    MTL::RenderPipelineState* boidRenderPipeline;
    Mesh triangleMesh;
    MTL::CommandQueue* commandQueue;
    GLFWwindow* glfwWindow;
};
