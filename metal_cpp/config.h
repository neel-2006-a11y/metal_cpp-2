//
//  config.h
//  metal_cpp
//
//  Created by Neel on 13/01/26.
//

#pragma once
#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include <simd/simd.h>

#include <iostream>
#include <fstream>
#include <sstream>

struct Vertex {
    simd::float2 pos;
    simd::float3 color;
};

struct Vertex3D{
    simd::float3 position;
    simd::float3 color;
    simd::float2 uv;
    simd::float3 normal;
};

struct CameraUniforms{
    simd::float4x4 view;
    simd::float4x4 proj;
    simd::float4x4 viewProj;
};

struct ObjectUniforms{
    simd::float4x4 model;
    simd::float4x4 invModel;
    simd::float3 scale;
};


static uint BoidVersion = 2;
