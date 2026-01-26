//
//  boidManager.cpp
//  metal_cpp
//
//  Created by Neel on 18/01/26.
//

#include "boids/boidManager.h"
#include "config.h"
#include <random>
#include <iostream>

BoidManager::BoidManager(MTL::Device* device, MTL::CommandQueue* commandQueue, GLFWwindow* glfwWindow): device(device), commandQueue(commandQueue), glfwWindow(glfwWindow) {
    
    gridDim = 2.0 / cellSize;
    if(BoidVersion == 1){
        buildComputePipeline();
        buildBuffers();
    }else if (BoidVersion == 2){
        buildComputePipeline2();
        buildBuffers2();
    }
    
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

void BoidManager::buildComputePipeline2(){
    NS::Error* error = nullptr;
    MTL::Library* library = device->newDefaultLibrary();
    
    MTL::Function* clearCellCounts = library->newFunction(NS::String::string("clearCellCounts", NS::UTF8StringEncoding));
    clearCellCount_pass = device->newComputePipelineState(clearCellCounts, &error);
    assert(clearCellCount_pass);
    
    MTL::Function* countBoids = library->newFunction(NS::String::string("countBoids", NS::UTF8StringEncoding));
    countBoids_pass = device->newComputePipelineState(countBoids, &error);
    assert(countBoids_pass);
    
    MTL::Function* scatter = library->newFunction(NS::String::string("scatterBoids", NS::UTF8StringEncoding));
    scatter_pass = device->newComputePipelineState(scatter, &error);
    assert(scatter_pass);
    
    MTL::Function* update = library->newFunction(NS::String::string("updateBoids_grid_2", NS::UTF8StringEncoding));
    update_2_pass = device->newComputePipelineState(update, &error);
    assert(update_2_pass);
}

void BoidManager::buildBuffers(){
    boidIn = device->newBuffer(sizeof(Boid) * kCount, MTL::ResourceStorageModeShared);
    boidOut = device->newBuffer(sizeof(Boid) * kCount, MTL::ResourceStorageModeShared);
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
//        boids[i].color = simd_make_float3(1.0f,0.0f,0.0f);
    }
}

void BoidManager::buildBuffers2(){
    boidIn_2 = device->newBuffer(sizeof(Boid) * kCount, MTL::ResourceStorageModeShared);
    boidOut_2 = device->newBuffer(sizeof(Boid) * kCount, MTL::ResourceStorageModeShared);
    cellStartBuffer = device->newBuffer(sizeof(uint) * gridDim * gridDim, MTL::ResourceStorageModeShared);
    cellFinishBuffer = device->newBuffer(sizeof(uint) * gridDim * gridDim, MTL::ResourceStorageModeShared);
    cellCountBuffer = device->newBuffer(sizeof(uint) * gridDim * gridDim, MTL::ResourceStorageModeShared);
    cellIndicesBuffer = device->newBuffer(sizeof(uint) * kCount, MTL::ResourceStorageModeShared);
    gridParamsBuffer = device->newBuffer(sizeof(gridParams), MTL::ResourceStorageModeShared);
//    simParamsBuffer = device->newBuffer(sizeof(SimParams), MTL::ResourceStorageModeShared);
    
    Boid* boids = static_cast<Boid*> (boidIn_2->contents());
    
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> pos(-1.0f, 1.0f);
    std::uniform_real_distribution<float> vel(-0.3f, 0.3f);
    
    for(uint32_t i = 0; i < kCount; i++){
        boids[i].position = {pos(rng), pos(rng)};
        simd::float2 v = {vel(rng), vel(rng)};
        if(simd::length(v) < 0.001f){
            v = {0.1f, 0.0f};
        }
        
        boids[i].velocity = simd::normalize(v) * 0.2f;
    }
    
    gridParams gridParam;
    gridParam.cellSize = cellSize;
    gridParam.gridDim = gridDim;
    gridParam.numBoids = kCount;
    gridParam.numCells = gridDim * gridDim;
    memcpy(gridParamsBuffer->contents(), &gridParam, sizeof(gridParams));
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



//============================================
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
        NS::UInteger tgSize = clearGrid_pass->maxTotalThreadsPerThreadgroup();
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
        NS::UInteger tgSize = insertBoids_pass->maxTotalThreadsPerThreadgroup();
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
        NS::UInteger tgSize = update_pass->maxTotalThreadsPerThreadgroup();
        MTL::Size threadgroupSize = MTL::Size(tgSize, 1, 1);
        enc->dispatchThreads(gridSize, threadgroupSize);
        enc->endEncoding();
//        commandBuffer->commit();
    }
    commandBuffer->commit();
    parity ^= 1;
}

//============================================
void BoidManager::update_grid_2(){
    MTL::CommandBuffer* commandBuffer1 = commandQueue->commandBuffer();;
    {// clear cell Counts
//        device atomic_uint* cellCounts [[ buffer(0) ]],
//        constant gridParams& gridParam [[ buffer(1)]],
//        uint id [[thread_position_in_grid]]
        MTL::ComputeCommandEncoder* enc = commandBuffer1->computeCommandEncoder();
        enc->setComputePipelineState(clearCellCount_pass);
        enc->setBuffer(cellCountBuffer, 0, 0);
        enc->setBuffer(gridParamsBuffer, 0, 1);
        
        uint32_t cellCnt = gridDim * gridDim;
        MTL::Size gridSize = MTL::Size(cellCnt, 1, 1);
        NS::UInteger tgSize = clearCellCount_pass->maxTotalThreadsPerThreadgroup();
        MTL::Size threadgroupSize = MTL::Size(tgSize, 1, 1);
        enc->dispatchThreads(gridSize, threadgroupSize);
        enc->endEncoding();
    }
    {// count Boids
//        device Boid* boids [[ buffer(0) ]],
//        device atomic_uint* cellCounts [[ buffer(1) ]],
//        constant gridParams& gridParam [[ buffer(2)]],
//        uint id [[thread_position_in_grid]]
        MTL::ComputeCommandEncoder* enc = commandBuffer1->computeCommandEncoder();
        enc->setComputePipelineState(countBoids_pass);
        if(parity == 0){
            enc->setBuffer(boidIn_2, 0, 0);
        }else{
            enc->setBuffer(boidOut_2, 0, 0);
        }
        enc->setBuffer(cellCountBuffer, 0, 1);
        enc->setBuffer(gridParamsBuffer, 0, 2);
        
        MTL::Size gridSize = MTL::Size(kCount, 1, 1);
        NS::UInteger tgSize = countBoids_pass->maxTotalThreadsPerThreadgroup();
        MTL::Size threadgroupSize = MTL::Size(tgSize, 1, 1);
        enc->dispatchThreads(gridSize, threadgroupSize);
        enc->endEncoding();
    }
    commandBuffer1->commit();
    commandBuffer1->waitUntilCompleted();

    
    
    // prefix sum
    uint* cellCounts = static_cast<uint*>(cellCountBuffer->contents());
    uint* cellStart = static_cast<uint*>(cellStartBuffer->contents());
    uint* cellFinish = static_cast<uint*>(cellFinishBuffer->contents());
    uint cellCnt = gridDim * gridDim;
    
    cellStart[0] = 0;
    cellFinish[0] = cellCounts[0];

    for(uint i = 1; i < cellCnt; i++){
        cellStart[i] = cellStart[i-1] + cellCounts[i-1];
        cellFinish[i] = cellStart[i] + cellCounts[i];
    }
    
    
    MTL::CommandBuffer* commandBuffer2 = commandQueue->commandBuffer();
    {// scatter boids in cells
//        device const Boid* boids [[ buffer(0) ]],
//        device atomic_uint* cellCounts [[ buffer(1) ]],
//        device const uint* cellOffsets [[ buffer(2) ]],
//        device uint* cellIndices [[ buffer(3) ]],
//        constant gridParams& gridParam [[ buffer(4) ]],
//        uint id [[thread_position_in_grid]]
        MTL::ComputeCommandEncoder* enc = commandBuffer2->computeCommandEncoder();
        enc->setComputePipelineState(scatter_pass);
        if(parity == 0){
            enc->setBuffer(boidIn_2, 0, 0);
        }else{
            enc->setBuffer(boidOut_2, 0, 0);
        }
        enc->setBuffer(cellCountBuffer, 0, 1);
        enc->setBuffer(cellStartBuffer, 0, 2);
        enc->setBuffer(cellIndicesBuffer, 0, 3);
        enc->setBuffer(gridParamsBuffer, 0 ,4);
        
        MTL::Size gridSize = MTL::Size(kCount, 1, 1);
        NS::UInteger tgSize = scatter_pass->maxTotalThreadsPerThreadgroup();
        MTL::Size threadgroupSize = MTL::Size(tgSize, 1, 1);
        enc->dispatchThreads(gridSize, threadgroupSize);
        enc->endEncoding();
    }
    {// update boids
//        device const Boid* boidsIn [[buffer(0)]],
//        device Boid* boidsOut [[buffer(1)]],
//        constant SimParams& params [[buffer(2)]],
//        device uint* cellIndices [[ buffer(3) ]],
//        device uint* cellStart [[ buffer(4) ]],
//        device uint* cellFinal [[ buffer(5) ]],
//        constant gridParams& gridParam [[ buffer(6) ]],
//        uint id [[thread_position_in_grid]]
        MTL::ComputeCommandEncoder* enc = commandBuffer2->computeCommandEncoder();
        enc->setComputePipelineState(update_2_pass);
        if(parity == 0){
            enc->setBuffer(boidIn_2, 0, 0);
            enc->setBuffer(boidOut_2, 0, 1);
        }else{
            enc->setBuffer(boidOut_2, 0, 0);
            enc->setBuffer(boidIn_2, 0, 1);
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
//        simParams.flowStrength = 0.0007f;
        simParams.flowStrength = 0.00f;
        enc->setBytes(&simParams, sizeof(SimParams), 2);
        enc->setBuffer(cellIndicesBuffer, 0, 3);
        enc->setBuffer(cellStartBuffer, 0, 4);
        enc->setBuffer(cellFinishBuffer, 0, 5);
        enc->setBuffer(gridParamsBuffer, 0, 6);
        
        MTL::Size gridSize = MTL::Size(kCount, 1, 1);
        NS::UInteger tgSize = update_2_pass->maxTotalThreadsPerThreadgroup();
        MTL::Size threadgroupSize = MTL::Size(tgSize, 1, 1);
        enc->dispatchThreads(gridSize, threadgroupSize);
        enc->endEncoding();
    }
    commandBuffer2->commit();
    parity^=1;
}


void BoidManager::updateFlow(FlowField& f, float dt){
    static std::mt19937 rng(std::random_device{}());
    static std::normal_distribution<float> noise(0.0f, 10.0f);

    // Slowly varying angular acceleration
    f.angularVelocity += noise(rng) * dt;
//    f.angularVelocity = std::clamp(f.angularVelocity, -1.0f, 1.0f);

    f.angle += f.angularVelocity * dt;
}


