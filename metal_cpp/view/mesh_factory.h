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

    vertex_index_pair buildGizmoLine(simd::float3 p1, simd::float3 p2);
    
    vertex_index_pair buildCube2(simd::float3 scale);
    vertex_index_pair buildSphere2(uint16_t stacks, uint16_t slices, float radius);
}

struct Object{
    MeshID meshID;
    ObjectUniforms objectU;
};
