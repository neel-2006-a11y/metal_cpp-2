//
//  shadowMap.metal
//  metal_cpp
//
//  Created by Neel on 20/02/26.
//
#include <metal_stdlib>
using namespace metal;

struct Vertex3D{
    float3 position [[attribute(0)]];
    float3 color [[attribute(1)]];
    float2 uv [[attribute(2)]];
    float3 normal [[attribute(3)]];
};

// implicit [[position]]
vertex float4 shadowVertex(Vertex3D in [[stage_in]],
                           constant float4x4& lightVP [[buffer(1)]],
                           constant float4x4& model [[buffer(2)]],
                           uint vid [[vertex_id]])
{
    float4 world = model * float4(in.position,1.0);
//    world /= world.w;
    return lightVP * world;
}
