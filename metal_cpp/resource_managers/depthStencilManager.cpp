//
//  depthStencilManager.cpp
//  metal_cpp
//
//  Created by Neel on 03/07/26.
//

#include "resource_managers/depthStencilManager.h"

DepthStencilManager::DepthStencilManager(MTL::Device* device){
    this->device = device;
}

DepthStencilStateID DepthStencilManager::createDepthState(DepthStencilDesc desc){
    DepthStencilState state;
    state.desc = desc;
    
    MTL::DepthStencilDescriptor* gpu_desc = MTL::DepthStencilDescriptor::alloc()->init();
    gpu_desc->setDepthCompareFunction(desc.compare);
    gpu_desc->setDepthWriteEnabled(desc.depthWrite);
    
    state.gpu_depthState = device->newDepthStencilState(gpu_desc);
    
    DepthStencilStateID id = nextID++;
    depthStates[id] = state;
    return id;
}

DepthStencilState* DepthStencilManager::getState(DepthStencilStateID id){
    auto it = depthStates.find(id);
    if(it == depthStates.end()) return nullptr;
    return &it->second;
}
