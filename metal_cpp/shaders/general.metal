//
//  general.metal
//  metal_cpp
//
//  Created by Neel on 14/01/26.
//

#include <metal_stdlib>
using namespace metal;

struct VertexInput {
    float2 position [[attribute(0)]];
    float3 color [[attribute(1)]];
};

struct VertexOutput {
    float4 position [[position]];
    float3 color;
};

VertexOutput vertex vertexMainGeneral(VertexInput in [[stage_in]],
                                      constant float4x4& transform [[buffer(1)]]){
    VertexOutput out;
    
    out.position = transform * float4(in.position,0.0,1.0);
    out.color = in.color;
    
    return out;
}

float4 fragment fragmentMainGeneral(VertexOutput frag [[stage_in]]){
//    return float4(0.0,0.0,0.0,0.0);
    return float4(frag.color,1.0);
}

