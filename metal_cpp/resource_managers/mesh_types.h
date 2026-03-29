//
//  mesh_types.h
//  metal_cpp
//
//  Created by Neel on 21/03/26.
//

#pragma once
#include <vector>
#include <cstdint>

// Vertex Attribute Type
enum class VertexAttributeType{
    Float,
    Float2,
    Float3,
    Float4
};

// Vertex Attribute
struct VertexAttribute {
    uint32_t location; // shader location
    uint32_t offset;
    VertexAttributeType type;
};

// Vertex layout
struct VertexLayout{
    uint32_t stride = 0;
    std::vector<VertexAttribute> attributes;
};
