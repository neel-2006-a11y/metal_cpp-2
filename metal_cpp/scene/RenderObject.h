//
//  RenderObject.h
//  metal_cpp
//
//  Created by Neel on 23/03/26.
//

#pragma once
#include "config.h"
#include "backend/mtlm.h"
#include "resource_managers/resource_types.h"


struct Transform{
    simd::float3 position = {0.0,0.0,0.0};
    simd_quatf rotation = simd_quaternion(0.0f, 0.0f, 0.0f, 1.0f);
    simd::float3 scale = {1.0,1.0,1.0};
    
    simd::float4x4 computeModel() const{
        simd::float4x4 T = mtlm::translation(position);
        simd::float4x4 R = simd_matrix4x4(rotation);
        simd::float4x4 S = mtlm::scale3D(scale);
        
        return T * R * S;
    }
};

struct RenderObject
{
    MeshID meshID;
    MaterialID materialID;
    
    float tileScale;
    
    uint32_t objectIndex = UINT32_MAX;
};

struct Entity{
    Transform transform;
    RenderObject renderObject;
};

struct Scene
{
    std::vector<Entity> objects;
};
