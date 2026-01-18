//
//  boidManager.h
//  metal_cpp
//
//  Created by Neel on 18/01/26.
//

#pragma once
#include "boids/instance.h"
#include <Metal/Metal.hpp>
#include <GLFW/glfw3.h>

class BoidManager {
public:
    uint32_t kCount = 8192;
    float cellSize = 0.1f;
    uint32_t gridDim;
    
    BoidManager(MTL::Device* device, MTL::CommandQueue* commandQueue, GLFWwindow* glfwWindow);
    ~BoidManager();
    
    void update();
    void update_grid();
    
    MTL::Buffer* boidIn;
    MTL::Buffer* boidOut;
    MTL::Buffer* gridBuffer;
    
private:
    void buildComputePipeline();
    void buildBuffers();
    void updateFlow(FlowField& f, float dt);
    
    uint32_t parity = 0;
    FlowField flow;
    
    MTL::Device* device;
    MTL::ComputePipelineState* computePipeline;
    MTL::ComputePipelineState* clearGrid_pass, *insertBoids_pass, *update_pass;
    MTL::CommandQueue* commandQueue;
    GLFWwindow* glfwWindow;
};
