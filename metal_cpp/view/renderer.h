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

class Renderer
{
    public:
        Renderer(MTL::Device* device, CA::MetalLayer* layer, GLFWwindow* glfwWindow);
        ~Renderer();
        void DrawFrame();

    private:
        void buildMeshes();
        void buildShaders();
    
        MTL::Device* device;
        MTL::CommandQueue* commandQueue;
        CA::MetalLayer* metalLayer;
        GLFWwindow* glfwWindow;
        
        MTL::RenderPipelineState* trianglePipeline, *generalPipeline, *boidRenderPipeline;
        
        BoidManager* boidManager;
    
    
        MTL::Buffer* triangleMesh;
        Mesh quadMesh;
    
        //debug
        bool first_frame = true;
        int parity = 0;
};
