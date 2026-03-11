//
//  directional_light.h
//  metal_cpp
//
//  Created by Neel on 19/02/26.
//

#pragma once

#include "config.h"
#include "backend/mtlm.h"

struct simple{
    float intensity = 1.0;
    simd::float3 direction = {1.0,0.0,0.0};
    simd::float3 color = {0.5,0.5, 0.0};
};

struct DirectionalLight{
    float intensity = 1.0;
    simd::float3 direction;
    simd::float3 color = {0.5,0.5, 0.0};
    
    bool viewDirty=true, projDirty=true;
    simd::float4x4 view, proj, viewProj;
    
    simd::float4x4 View(){
//        if(!viewDirty)return view;
        
        simd::float3 f = simd::normalize(direction);
        simd::float3 x = {1.0,0.0,0.0};
        if(simd::dot(f,x)>0.99) x = {0.0,1.0,0.0};
        simd::float3 u = simd::cross(f,x);
        view = mtlm::look_at(simd::float3(0), f, u);
        
        viewDirty = false;
        return view;
    }
    simd::float4x4 Proj(){
//        if(!projDirty)return proj;
        projDirty = false;
        float rangeX = 15;
        float rangeZ = 40;
        proj = mtlm::orthographic_projection(-rangeX, rangeX, -rangeX, rangeX, 0, rangeZ);
        return proj;
    }
    simd::float4x4 ViewProj(){
        return Proj() * View();
    }
};

