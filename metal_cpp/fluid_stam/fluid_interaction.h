//
//  fluid_interaction.h
//  metal_cpp
//
//  Created by Neel on 03/07/26.
//

#pragma once
#include "config.h"
#include <queue>
#include "view/ResourceRegistry.h"
#include "view/FrameDataRegistry.h"

// interaction structs
struct DensityInteraction{
    int x,y,size;
    simd::float4 amount;
};

struct VelocityInteraction{
    int x,y,size;
    float amountX, amountY;
};

struct FluidCommandBuffer{
    std::queue<DensityInteraction> densityIneractQ;
    std::queue<VelocityInteraction> velIneractQ;
    
    void clear(){
        densityIneractQ = std::queue<DensityInteraction>();
        velIneractQ = std::queue<VelocityInteraction>();
    }
};


class FluidInteractor{
public:
    int N = 512; // temporary
    
    void addDensity(int x, int y, int size, simd::float4 amount);
    
    void addVelocity(int x, int y, int size, float amountX, float amountY);
    
    void publish(ResourceRegistry& resourceRegistry, FrameDataRegistry& frameDataRegistry);
    
private:
    FluidCommandBuffer cmd;
};
