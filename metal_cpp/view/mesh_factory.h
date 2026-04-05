//
//  mesh_factory.h
//  metal_cpp
//
//  Created by Neel on 14/01/26.
//

#pragma once
#include "config.h"
#include "resource_managers/mesh.h"
#include "resource_managers/resource_types.h"

struct vertex_index_pair{
    std::vector<uint8_t> vertexData;
    std::vector<Index> indexData;
    int indexCount;
};

namespace MeshFactory {
    Mesh buildTriangle(MTL::Device* device);
    Mesh buildTriangle3D(MTL::Device* device);
    Mesh buildQuad(MTL::Device* device);
    Mesh buildCube(MTL::Device* device, simd::float3 scale);
    Mesh buildSphere(MTL::Device* device, uint16_t stacks, uint16_t slices, float radius);

    vertex_index_pair buildCube2(MTL::Device* device, simd::float3 scale);
    vertex_index_pair buildSphere2(MTL::Device* device, uint16_t stacks, uint16_t slices, float radius);
}

struct Object{
    MeshID meshID;
    ObjectUniforms objectU;
};
