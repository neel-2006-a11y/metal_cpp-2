//
//  mesh_factory.h
//  metal_cpp
//
//  Created by Neel on 14/01/26.
//

#pragma once
#include "config.h"

struct Mesh {
    MTL::Buffer* vertexBuffer, *indexBuffer;
    MTL::VertexDescriptor* vertexDescriptor;
    uint indexCount;
};

namespace MeshFactory {
    Mesh buildTriangle(MTL::Device* device);
    Mesh buildTriangle3D(MTL::Device* device);
    Mesh buildQuad(MTL::Device* device);
    Mesh buildCube(MTL::Device* device, simd::float3 scale);
    Mesh buildSphere(MTL::Device* device, uint16_t stacks, uint16_t slices, float radius);
}

struct Object{
    Mesh* mesh;
    ObjectUniforms objectU;
};
