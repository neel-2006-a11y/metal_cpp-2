//
//  fluid_interaction.cpp
//  metal_cpp
//
//  Created by Neel on 03/07/26.
//

#include "fluid_stam/fluid_interaction.h"



void FluidInteractor::addDensity(int x, int y, int size, simd::float4 amount){
    DensityInteraction di;
    di.size = size;
    di.amount = amount;
    di.x = x;
    di.y = y;
    
    cmd.densityIneractQ.push(di);
}

void FluidInteractor::addVelocity(int x, int y, int size, float amountX, float amountY){
    VelocityInteraction vi;
    vi.size = size;
    vi.amountX = amountX;
    vi.amountY = amountY;
    vi.x = x;
    vi.y = y;
    
    cmd.velIneractQ.push(vi);
}

void FluidInteractor::publish(ResourceRegistry &resourceRegistry, FrameDataRegistry &frameDataRegistry) {
    frameDataRegistry.publish(std::move(cmd));
    cmd.clear();
}
