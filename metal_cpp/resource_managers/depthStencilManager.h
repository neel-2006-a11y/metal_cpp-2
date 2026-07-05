//
//  depthStencilManager.h
//  metal_cpp
//
//  Created by Neel on 03/07/26.
//
#pragma once

#include "resource_managers/resource_types.h"
#include "config.h"

struct DepthStencilDesc {
    bool depthWrite = true;
    MTL::CompareFunction compare = MTL::CompareFunctionLess;
};

struct DepthStencilState {
    MTL::DepthStencilState* gpu_depthState;
    DepthStencilDesc desc;
};

class DepthStencilManager{
public:
    DepthStencilManager(MTL::Device* device);
    
    DepthStencilStateID createDepthState(DepthStencilDesc desc);
    
    DepthStencilState* getState(DepthStencilStateID id);
    
private:
    std::unordered_map<DepthStencilStateID, DepthStencilState> depthStates;
    DepthStencilStateID nextID = 1;
    
    MTL::Device* device = nullptr;
};
