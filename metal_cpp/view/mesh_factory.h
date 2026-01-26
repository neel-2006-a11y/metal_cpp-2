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
};

namespace MeshFactory {
    Mesh buildTriangle(MTL::Device* device);
    Mesh buildQuad(MTL::Device* device);
}
