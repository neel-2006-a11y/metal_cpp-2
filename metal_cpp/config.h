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

// 0->8bit , 1->16bit
#define HALFTONE_TEXTURE_16BIT 1
#if HALFTONE_TEXTURE_16BIT
    using HalftonePixel = uint16_t;
    constexpr MTL::PixelFormat HALFTONE_PIXEL_FORMAT = MTL::PixelFormatR16Unorm;
    constexpr float HALFTONE_PIXEL_SCALE = 65535.0f;
#else
    using HalftonePixel = uint8_t;
    constexpr MTL::PixelFormat HALFTONE_PIXEL_FORMAT = MTL::PixelFormatR8Unorm;
    constexpr float HALFTONE_PIXEL_SCALE = 255.0f;
#endif

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
    simd::float3 scale = simd::float3{1.0,1.0,1.0};
};


static uint BoidVersion = 2;
