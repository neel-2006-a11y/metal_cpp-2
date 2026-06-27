//
//  mesh.h
//  metal_cpp
//
//  Created by Neel on 18/03/26.
//

#pragma once
#include <vector>
#include "config.h"
#include "resource_managers/mesh_types.h"

struct Vertex3D{
    simd::float3 position;
    simd::float3 color;
    simd::float2 uv;
    simd::float3 normal;
};

struct LineVertex {
    simd::float3 position;
};


struct Mesh2{
    std::vector<uint8_t> vertexData;
    std::vector<Index> indexData;
    
    // gpu handles
    void* vertexBuffer = nullptr;
    void* indexBuffer = nullptr;
    
    uint32_t indexCount = 0;
    
    bool uploaded = false;
};
