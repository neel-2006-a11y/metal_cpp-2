//
//  resource_types.h
//  metal_cpp
//
//  Created by Neel on 18/03/26.
//

#pragma once
#include <cstdint>

using MeshID = uint32_t;
using TextureID = uint32_t;
using PipelineID = uint32_t;
using MaterialID = uint32_t;


constexpr MeshID INVALID_MESH = 0;
constexpr TextureID INVALID_TEXTURE = 0;
constexpr PipelineID INVALID_SHADER = 0;
constexpr MaterialID INVALID_MATERIAL = 0;

