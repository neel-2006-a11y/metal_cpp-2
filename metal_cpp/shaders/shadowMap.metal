//
//  shadowMap.metal
//  metal_cpp
//
//  Created by Neel on 20/02/26.
//
#include <metal_stdlib>
#include "common.metal"
using namespace metal;

struct Vertex3D_2{
    float3 position [[attribute(0)]];
    float3 color [[attribute(1)]];
    float2 uv [[attribute(2)]];
    float3 normal [[attribute(3)]];
};


// implicit [[position]]
vertex float4 shadowVertex(Vertex3D_2 in [[stage_in]],
                           constant FrameUniforms_2& frame [[buffer(1)]],
                           constant ObjectUniforms_2& object [[buffer(2)]],
                           constant uint32_t& cascadeInd [[buffer(3)]])
{
    float4 world = object.model * float4(in.position,1.0);
    return frame.sunVPs[cascadeInd] * world;
}
