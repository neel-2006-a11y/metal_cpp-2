//
//  directional_light.h
//  metal_cpp
//
//  Created by Neel on 19/02/26.
//

#pragma once

#include "config.h"
#include "backend/mtlm.h"
#include "view/camera.h"

struct simple{
    float intensity = 1.0;
    simd::float3 direction = {1.0,0.0,0.0};
    simd::float3 color = {0.5,0.5, 0.0};
    int cascades = 4;
};

class DirectionalLight{
public:
    float intensity = 1.0;
    simd::float3 direction;
    simd::float3 color = {0.5,0.5, 0.0};
    
    int cascades = 4;
    
    bool viewDirty=true, projDirty=true;
    simd::float4x4 view;
    std::vector<simd::float4x4> proj;
    std::vector<simd::float4x4> viewProj;
    
    
    DirectionalLight();
    
    simd::float4x4 View();
    void fitCameraFrustrums(std::vector<worldFrustrum> frustrums); // updates proj vector
    void fitCameraFrustrums2Sphere(std::vector<worldFrustrum> frustrums);
    std::vector<simd::float4x4> ViewProj();
    
    simd::float3 forward();
    simd::float3 up();
};

