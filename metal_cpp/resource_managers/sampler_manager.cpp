//
//  sampler_manager.cpp
//  metal_cpp
//
//  Created by Neel on 03/07/26.
//

#include "resource_managers/sampler_manager.h"

SamplerManager::SamplerManager(MTL::Device* device){
    this->device = device;
}

SamplerID SamplerManager::createSampler(SamplerDesc desc){
    SamplerState sampler;
    sampler.desc = desc;
    
    MTL::SamplerDescriptor* gpu_desc = MTL::SamplerDescriptor::alloc()->init();
    gpu_desc->setMinFilter(desc.minFilter);
    gpu_desc->setMagFilter(desc.magFilter);
    gpu_desc->setMipFilter(desc.mipFilter);
    gpu_desc->setRAddressMode(desc.addressU);
    gpu_desc->setSAddressMode(desc.addressV);
    gpu_desc->setTAddressMode(desc.addressW);
    
    sampler.state = device->newSamplerState(gpu_desc);
    
    SamplerID id = nextID++;
    samplers[id] = sampler;
    return id;
}

SamplerState* SamplerManager::getState(SamplerID id){
    auto it = samplers.find(id);
    if(it == samplers.end()) return nullptr;
    return &it->second;
}
