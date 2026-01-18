//
//  boidManager.cpp
//  metal_cpp
//
//  Created by Neel on 18/01/26.
//

#include "boids/boidManager.h"
#include <random>

BoidManager::BoidManager(MTL::Device* device, MTL::CommandQueue* commandQueue, GLFWwindow* glfwWindow): device(device), commandQueue(commandQueue), glfwWindow(glfwWindow) {
    buildComputePipeline();
    buildBuffers();
}

void BoidManager::buildComputePipeline(){
    NS::Error* error = nullptr;
    MTL::Library* library = device->newDefaultLibrary();
    MTL::Function* function = library->newFunction(NS::String::string("updateBoids", NS::UTF8StringEncoding));
    computePipeline = device->newComputePipelineState(function, &error);
    assert(computePipeline);
    
    MTL::Function* clrFunction = library->newFunction(NS::String::string("clearGrid", NS::UTF8StringEncoding));
    clearGrid_pass = device->newComputePipelineState(clrFunction, &error);
    assert(clearGrid_pass);
    
    MTL::Function* insertBoidsFunction = library->newFunction(NS::String::string("insertBoids", NS::UTF8StringEncoding));
    insertBoids_pass = device->newComputePipelineState(insertBoidsFunction, &error);
    assert(insertBoids_pass);
    
    MTL::Function* updateFunction = library->newFunction(NS::String::string("updateBoids_grid", NS::UTF8StringEncoding));
    update_pass = device->newComputePipelineState(updateFunction, &error);
    assert(update_pass);
}

void BoidManager::buildBuffers(){
    boidIn = device->newBuffer(sizeof(Boid) * kCount, MTL::ResourceStorageModeShared);
    boidOut = device->newBuffer(sizeof(Boid) * kCount, MTL::ResourceStorageModeShared);
    gridDim = 2.0 / cellSize;
    gridBuffer = device->newBuffer(sizeof(Cell) * gridDim * gridDim, MTL::ResourceStorageModeShared);
    
    Boid* boids = static_cast<Boid*> (boidIn->contents());
    
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> pos(-1.0f, 1.0f);
    std::uniform_real_distribution<float> vel(-0.3f, 0.3f);
    
    for(uint32_t i = 0; i < kCount; i++){
        boids[i].position = {pos(rng), pos(rng)};
        //        boids[i].position = {
        //            float(i%32) * 0.05f,
        //            float(i/32) * 0.05f
        //        };
        simd::float2 v = {vel(rng), vel(rng)};
        if(simd::length(v) < 0.001f){
            v = {0.1f, 0.0f};
        }
        
        boids[i].velocity = simd::normalize(v) * 0.2f;
    }
}

void BoidManager::update(){
    // compute pass
    MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
    
    MTL::ComputeCommandEncoder* cmdEnc = commandBuffer->computeCommandEncoder();
    cmdEnc->setComputePipelineState(computePipeline);
    if(parity == 0){
        cmdEnc->setBuffer(boidIn, 0, 0);
        cmdEnc->setBuffer(boidOut, 0, 1);
    }else{
        cmdEnc->setBuffer(boidIn, 0, 1);
        cmdEnc->setBuffer(boidOut, 0, 0);
    }
    parity = (parity+1) % 2;
    
    // ===== sim params =====
    SimParams simParams;
    simParams.dt = 0.01f;
    simParams.frame++;
    simParams.kCount = kCount;
    
    double mx, my;
    glfwGetCursorPos(glfwWindow, &mx, &my);
    int w,h;
    glfwGetWindowSize(glfwWindow, &w, &h);
    simParams.mousePos.x = (mx/w) * 2.0f - 1.0f;
    simParams.mousePos.y = 1.0f - (my/h) * 2.0f;
    simParams.mouseForce = 0.3f;
    
    updateFlow(flow, simParams.dt);
    simParams.flowDirection = {
        std::cos(flow.angle),
        std::sin(flow.angle)
    };
    simParams.flowStrength = 0.07f;
    
    
    cmdEnc->setBytes(&simParams, sizeof(simParams), 2);
    
    MTL::Size gridSize = MTL::Size(kCount, 1, 1);
    NS::UInteger tgSize = computePipeline->maxTotalThreadsPerThreadgroup();
    MTL::Size threadgroupSize = MTL::Size(tgSize, 1, 1);
    cmdEnc->dispatchThreads(gridSize, threadgroupSize);
    cmdEnc->endEncoding();
    commandBuffer->commit();
}

void BoidManager::update_grid(){
    MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
    { // clear grid pass
//        MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
        MTL::ComputeCommandEncoder* enc = commandBuffer->computeCommandEncoder();
        enc->setComputePipelineState(clearGrid_pass);
        enc->setBuffer(gridBuffer, 0, 0);
        uint32_t gridCellCount = gridDim * gridDim;
        
        enc->setBytes(&gridCellCount, sizeof(uint32_t), 1);
        
        MTL::Size gridSize = MTL::Size(gridCellCount, 1, 1);
        NS::UInteger tgSize = computePipeline->maxTotalThreadsPerThreadgroup();
        MTL::Size threadgroupSize = MTL::Size(tgSize, 1, 1);
        enc->dispatchThreads(gridSize, threadgroupSize);
        enc->endEncoding();
//        commandBuffer->commit();
    }
    
    { // insert boids pass
//        MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
        MTL::ComputeCommandEncoder* enc = commandBuffer->computeCommandEncoder();
        enc->setComputePipelineState(insertBoids_pass);
        if(parity == 0){
            enc->setBuffer(boidIn, 0, 0);
        }else{
            enc->setBuffer(boidOut, 0, 0);
        }
        enc->setBuffer(gridBuffer, 0 , 1);
        enc->setBytes(&cellSize, sizeof(float), 2);
        enc->setBytes(&gridDim, sizeof(uint32_t), 3);
        
        MTL::Size gridSize = MTL::Size(kCount, 1, 1);
        NS::UInteger tgSize = computePipeline->maxTotalThreadsPerThreadgroup();
        MTL::Size threadgroupSize = MTL::Size(tgSize, 1, 1);
        enc->dispatchThreads(gridSize, threadgroupSize);
        enc->endEncoding();
//        commandBuffer->commit();
    }
    
    { // update pass
//        MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
        MTL::ComputeCommandEncoder* enc = commandBuffer->computeCommandEncoder();
        enc->setComputePipelineState(update_pass);
        if(parity == 0){
            enc->setBuffer(boidIn, 0, 0);
            enc->setBuffer(boidOut, 0 , 1);
        }else{
            enc->setBuffer(boidOut, 0, 0);
            enc->setBuffer(boidIn, 0 , 1);
        }
        // ===== sim params =====
        SimParams simParams;
        simParams.dt = 0.01f;
        simParams.frame++;
        simParams.kCount = kCount;
        
        double mx, my;
        glfwGetCursorPos(glfwWindow, &mx, &my);
        int w,h;
        glfwGetWindowSize(glfwWindow, &w, &h);
        simParams.mousePos.x = (mx/w) * 2.0f - 1.0f;
        simParams.mousePos.y = 1.0f - (my/h) * 2.0f;
        simParams.mouseForce = 0.3f;
        
        updateFlow(flow, simParams.dt);
        simParams.flowDirection = {
            std::cos(flow.angle),
            std::sin(flow.angle)
        };
        simParams.flowStrength = 0.07f;
        
        
        enc->setBytes(&simParams, sizeof(simParams), 2);
        enc->setBuffer(gridBuffer, 0, 3);
        enc->setBytes(&cellSize, sizeof(float), 4);
        enc->setBytes(&gridDim, sizeof(uint32_t), 5);
        
        MTL::Size gridSize = MTL::Size(kCount, 1, 1);
        NS::UInteger tgSize = computePipeline->maxTotalThreadsPerThreadgroup();
        MTL::Size threadgroupSize = MTL::Size(tgSize, 1, 1);
        enc->dispatchThreads(gridSize, threadgroupSize);
        enc->endEncoding();
//        commandBuffer->commit();
    }
    commandBuffer->commit();
    parity ^= 1;
}

void BoidManager::updateFlow(FlowField& f, float dt){
    static std::mt19937 rng(std::random_device{}());
    static std::normal_distribution<float> noise(0.0f, 3.0f);

    // Slowly varying angular acceleration
    f.angularVelocity += noise(rng) * dt;
//    f.angularVelocity = std::clamp(f.angularVelocity, -1.0f, 1.0f);

    f.angle += f.angularVelocity * dt;
}
