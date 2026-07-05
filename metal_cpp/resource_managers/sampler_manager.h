//
//  sampler_manager.h
//  metal_cpp
//
//  Created by Neel on 03/07/26.
//

#pragma once
#include "config.h"
#include "resource_managers/resource_types.h"


struct SamplerDesc
{
    MTL::SamplerMinMagFilter minFilter = MTL::SamplerMinMagFilterNearest;
    MTL::SamplerMinMagFilter magFilter = MTL::SamplerMinMagFilterNearest;
    MTL::SamplerMipFilter mipFilter = MTL::SamplerMipFilterNotMipmapped;

    MTL::SamplerAddressMode addressU = MTL::SamplerAddressModeRepeat;
    MTL::SamplerAddressMode addressV = MTL::SamplerAddressModeRepeat;
    MTL::SamplerAddressMode addressW = MTL::SamplerAddressModeRepeat;
};

struct SamplerState{
    MTL::SamplerState* state;
    SamplerDesc desc;
};


class SamplerManager{
public:
    SamplerManager(MTL::Device* device);
    
    SamplerID createSampler(SamplerDesc desc);
    
    SamplerState* getState(SamplerID id);
    
private:
    std::unordered_map<SamplerID, SamplerState> samplers;
    SamplerID nextID = 1;
    
    MTL::Device* device = nullptr;
};

