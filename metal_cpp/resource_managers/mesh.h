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

//struct Mesh {
//    MTL::Buffer* vertexBuffer, *indexBuffer;
//    MTL::VertexDescriptor* vertexDescriptor;
//    uint indexCount;
//};

struct Vertex3D{
    simd::float3 position;
    simd::float3 color;
    simd::float2 uv;
    simd::float3 normal;
};

struct Mesh{
    std::vector<Vertex3D> vertices;
    std::vector<Index> indices;
    int indexCount;
    
    MTL::Buffer *vertexBuffer = nullptr, *indexBuffer = nullptr;
    MTL::VertexDescriptor *vertexDescriptor = nullptr;
    
    bool uploaded = false;
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
