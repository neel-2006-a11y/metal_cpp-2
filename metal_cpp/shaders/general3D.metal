//
//  general3D.metal
//  metal_cpp
//
//  Created by Neel on 26/01/26.
//

#include <metal_stdlib>
using namespace metal;

struct VertexInput3D{
    float3 position [[attribute(0)]];
    float3 color [[attribute(1)]];
    float2 uv [[attribute(2)]];
    float3 normal [[attribute(3)]];
};

struct VertexOutput3D{
    float4 position [[position]];
    float3 normal;
    float3 worldPos;
    float3 color;
    float3 objectPos;
    float2 uv;
};

struct CameraUniforms{
    float4x4 view;
    float4x4 proj;
    float4x4 viewProj;
};

struct ObjectUniforms{
    float4x4 model;
    float4x4 invModel;
    float3 scale;
};

struct DirectionalLight{
    float intensity;
    float3 direction;
    float3 color;
};

float shadowCalculation(float3 worldPos, texture2d<float> shadowMap, sampler shadowSampler, float4x4 lightVP){
    float4 lightClip = lightVP * float4(worldPos, 1.0);
    float3 proj = lightClip.xyz / lightClip.w;
    
    float2 uv = proj.xy * 0.5 + 0.5;
    uv.y = 1-uv.y;
    if (uv.x < 0 || uv.x > 1 || uv.y < 0 || uv.y > 1)
        return 0.0;
    float currentDepth = proj.z;
    float bias = 0.0005;
    
    float shadow = 0.0;
    
    float texDepth = shadowMap.sample(shadowSampler, uv).r;
    if(currentDepth-bias > texDepth)shadow+=1.0;
    return shadow;
}

float bayer4x4Sample(uint2 pixel, constant float4x4& bayer){
    uint x = pixel.x & 3;
    uint y = pixel.y & 3;
    
    return bayer[x][y] / 16.0;
}

float bayer8x8TransitionSample(uint2 pixel, constant float* bayer){
    uint x = pixel.x & 7;
    uint y = pixel.y & 7;
    
    return bayer[y*8+x] / 16.0;
}

float bayer4x4x4Sample(uint3 voxel, constant float* bayer){
    uint x = voxel.x & 3;
    uint y = voxel.y & 3;
    uint z = voxel.z & 3;
    
    uint index = x + y*4 + z*16;
    return bayer[index] / 64.0;
}


VertexOutput3D vertex vertexMain3D(VertexInput3D in [[stage_in]],
                                   constant CameraUniforms& cameraUniforms[[buffer(1)]],
                                   constant ObjectUniforms& objectuniforms[[buffer(2)]]
                                   ){
    VertexOutput3D out;
    float4 worldPos = objectuniforms.model * float4(in.position,1);
    out.position = cameraUniforms.viewProj * worldPos;
    out.color = in.color;
    out.normal = (transpose(objectuniforms.invModel) * float4(in.normal,1)).xyz;
    out.worldPos = worldPos.xyz;
    out.objectPos = in.position * objectuniforms.scale;
    out.uv = in.uv;
    return out;
}

float4 fragment fragmentMain3D(VertexOutput3D in [[stage_in]],
                               texture2d<float> shadowMap [[texture(0)]],
                               sampler shadowSampler [[sampler(0)]],
                               constant DirectionalLight& sun [[buffer(3)]],
                               constant float3& cameraPos [[buffer(4)]],
                               constant float4x4& sunVP [[buffer(5)]],
                               constant float3& TileScale [[buffer(7)]],
                               constant float3& BayerScale [[buffer(8)]],
                               constant float* BayerTransitions [[buffer(9)]]){
    float3 N = normalize(in.normal);
    float3 V = normalize(cameraPos - in.worldPos);

    float3 L = normalize(-sun.direction);
    float3 H = normalize(L + V);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), 64.0);
    spec = 0;

    float shadow = shadowCalculation(in.worldPos, shadowMap, shadowSampler, sunVP);
    float3 color =
        sun.color * sun.intensity * diff +
        sun.color * spec * 0.2;
    
    color *= (1.0-shadow);
    
    color += 0.05*float3(1.0); // ambient light
    
    // ----------------------------------------
    // UV
    float2 duvdx = dfdx(in.uv);
    float2 duvdy = dfdy(in.uv);
    
    float MaxAdapt = 4096;
    float MinAdapt = 256;
    // rand (stable noise)
    uint2 tile = uint2(floor(in.uv * MaxAdapt * BayerScale.x * 1.0 * TileScale.x/256));
    float rand = fract(dot(float2(tile), float2(0.75487766, 0.56984029)));
    
    float pixelCoverage = max(length(duvdx), length(duvdy));
    float logScale = -log2(pixelCoverage);
    float frac = fract(logScale);
    float snapped = floor(logScale + (rand-0.5) * 0.02);// 0.02
    if(rand>0.9)snapped++;
    float adaptiveScale = exp2(snapped) * BayerScale.x / (TileScale.x);
    // we are dividing by TileScale so that we can multiply by it after and all objects at similar distance and different scales approach the MaxAdapt and MinAdapt at same time
    adaptiveScale = clamp(adaptiveScale, MinAdapt, MaxAdapt);
    
    
    // sample using tiled UV in BayerMatrix
    uint2 tiledUV = uint2(in.uv * TileScale.x/256 * adaptiveScale);
    
    
    float randStrength = 0.05; // 0.05 // to prevent banding between transitions
    float tileFrac = fract((1-randStrength)*frac + randStrength * rand);
    
    uint fracLvl = 0;
    if(adaptiveScale == MaxAdapt){
        fracLvl = 3;
    }else{
        int lvl1 = tileFrac / log2(1.25);
        int lvl2 = tileFrac / log2(1.5);
        int lvl3 = tileFrac / log2(1.75);
//        if(tileFrac>log2(1.25))fracLvl++;
//        if(tileFrac>log2(1.5))fracLvl++;
//        if(tileFrac>log2(1.75))fracLvl++;
        fracLvl = lvl1 = lvl2 + lvl3;
    }
    
    
    
    float threshold = bayer8x8TransitionSample(tiledUV, BayerTransitions+64*fracLvl);
    
    
    float luminance = dot(color, float3(0.299, 0.587, 0.114));
    float dithered = luminance > threshold ? 1.0 : 0.0;
    
    // ----------------------------------------
//    
//    float2 duvdx = dfdx(in.uv);
//    float2 duvdy = dfdy(in.uv);
//
//    float footprint = max(length(duvdx), length(duvdy));
//    float lod = log2(footprint);
//    float MaxAdapt = 1024;
////    float MinAdapt = 256;
//
//    uint2 tile = uint2(in.uv * MaxAdapt * TileScale.x / 256);
//
//    float bayer = bayer8x8TransitionSample(tile, BayerTransitions);
//
//    float phi = 0.61803398875;
//    float animated = fract(bayer + lod * phi);
//
//    float luminance = dot(color, float3(0.299,0.587,0.114));
//
//    float dithered = luminance > animated ? 1.0 : 0.0;
    //----------------------------------------
    
//    float debug = adaptiveScale/1024.0;
//    return float4(debug, debug, debug, 1.0);
//    return float4(dithered, tileFrac,0,1);
//    return float4(in.objectPos.xyz, 1.0);
    return float4(dithered, dithered, dithered, 1.0);
//    return float4(color, 1.0);
}
