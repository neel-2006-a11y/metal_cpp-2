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
#include "GLFW/glfw3.h"
#include "QuartzCore/CAMetalLayer.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

// 0->8bit , 1->16bit
#define HALFTONE_DIM 8

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

#define INDEX_SIZE_32BIT 1
#if INDEX_SIZE_32BIT
    using Index = uint32_t;
    constexpr MTL::IndexType INDEX_FORMAT = MTL::IndexTypeUInt32;
#else
    using Index = uint16_t;
    constexpr MTL::IndexType INDEX_FORMAT = MTL::IndexTypeUInt16;
#endif

#define CASCADES 4

#define FIF 3

#define MAX_OBJECT 10
#define MAX_MATERIALS 10

struct Vertex {
    simd::float2 pos;
    simd::float3 color;
};

struct CameraUniforms{
    simd::float4x4 view;
    simd::float4x4 proj;
    simd::float4x4 viewProj;
};

struct ObjectUniforms{
    simd::float4x4 model;
    simd::float4x4 invModel;
    float tileScale = 1.0;
};

#define DIFFUSE_SLOT 2

struct MaterialUniforms{
    int hasDiffuse = 0;
    
//    float roughness;
//    float metallic;
//    
//    simd::float3 baseColor;
};

struct FrameUniforms{
    //--------------
    // Camera Uniforms
    //--------------
    simd::float4x4 view;
    simd::float4x4 proj;
    simd::float4x4 viewProj;
    simd::float4x4 invViewProj;
    
    simd::float3 cameraPos;
    
    //--------------
    // Directional Light
    //--------------
    float intensity;
    simd::float3 direction;
    simd::float3 color;
    int cascades = CASCADES;
    
    simd::float4x4 sunVPs[CASCADES];
    float cascadeSplits[CASCADES];
    
    //--------------
    // Dithering Uniforms
    //--------------
    float BayerScale; // should be in material uniforms, but here for now
    
    //--------------
    // Volumetric Uniforms
    //--------------
    float volumeDensity;
    float g; // HG Phase param
};

struct DebugSettings{
    simd::float3 sunDirection = { 1.0f, 0.3f, 1.0f };
    float BayerScale = 0.04;
    float volumeDensity = 0.03;
    float g = 0.6;
};

static uint BoidVersion = 2;
