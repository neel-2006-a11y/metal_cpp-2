//
//  common.metal
//  metal_cpp
//
//  Created by Neel on 04/04/26.
//
#include <metal_stdlib>
using namespace metal;
#ifndef COMMON_METAL
#define COMMON_METAL

#define CASCADES 4

#define FRAME_BUFFER_BIND 1
#define OBJ_BUFFER_BIND 2
#define MAT_BUFFER_BIND 3
#define VIEW_BUFFER_BIND 4

#define SHADOW_MAP_BIND 0
#define DIFFUSE_BIND 2


struct VertexInput3D_2{
    float3 position [[attribute(0)]];
    float3 color [[attribute(1)]];
    float2 uv [[attribute(2)]];
    float3 normal [[attribute(3)]];
};

struct VertexOutput3D_2{
    float4 position [[position]];
    float3 normal;
    float3 worldPos;
    float3 color;
    float2 uv;
    float viewDepth;
    float4 cameraClipPos;
};

struct FragmentOutput{
    float4 color [[color(0)]];
    float4 normal [[color(1)]];
};

struct ObjectUniforms_2{
    float4x4 model;
    float4x4 invModel;
};

struct ViewUniforms{
    //--------------
    // Camera Uniforms
    //--------------
    float4x4 view;
    float4x4 proj;
    float4x4 viewProj;
    float4x4 invViewProj;
    
    float3 cameraPos;
    
    float far;
    
    int has_oblique_plane;
    float4 plane;
    
    float4x4 sunVPs[CASCADES];
    float cascadeSplits[CASCADES];
};

struct FrameUniforms_2{
    float4x4 view;
    float4x4 proj;
    float4x4 viewProj;
    float4x4 invViewProj;
    
    float3 cameraPos;
    float far;
    
    float intensity;
    float3 direction;
    float3 color;
    int cascades;
    
    float4x4 sunVPs[CASCADES];
    float cascadeSplits[CASCADES];
    
    float BayerScale;
    float exp_factor;
    
    float volumeDensity;
    float g;
    
    int has_oblique_plane;
    float4 plane;
};

static inline int select_cascade(float viewDepth, constant float* cascadeSplits, int num_cascades){
    int cascadeIndex = 0;
    for(int i = 1; i <= num_cascades; i++){
        if(viewDepth <= cascadeSplits[i]){
            cascadeIndex = i;
            break;
        }
    }
    
    if(cascadeIndex==0)return num_cascades-1;
    
    return cascadeIndex-1;
}

static inline float shadowCalculationCSM(float3 worldPos, texture2d_array<float> shadowMap, sampler shadowSampler, float4x4 lightVP, int cascade){
    float4 lightClip = lightVP * float4(worldPos, 1.0);
    float3 proj = lightClip.xyz / lightClip.w;
    
    float2 uv = proj.xy * 0.5 + 0.5;
    uv.y = 1-uv.y;
    if (uv.x < 0 || uv.x > 1 || uv.y < 0 || uv.y > 1)
        return 0.0;
    float currentDepth = proj.z;
    float bias = 0.0005;
    
    float shadow = 0.0;
    
    float texDepth = shadowMap.sample(shadowSampler, uv, cascade).r;
    if(currentDepth-bias > texDepth)shadow+=1.0;
    return shadow;
}

#endif
