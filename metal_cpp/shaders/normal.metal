//
//  normal.metal
//  metal_cpp
//
//  Created by Neel on 01/10/26.
//

#include <metal_stdlib>
#include "common.metal"
using namespace metal;

struct VertexOutput3D_normal{
    float4 position [[position]];
    float3 normal;
};

VertexOutput3D_normal vertex vertexMain3D_normal(VertexInput3D_2 in [[stage_in]],
                                   constant FrameUniforms_2& frame[[buffer(FRAME_BUFFER_BIND)]],
                                   constant ObjectUniforms_2& object[[buffer(OBJ_BUFFER_BIND)]]
                                   ){
    VertexOutput3D_normal out;
    float4 worldPos = object.model * float4(in.position,1);
    out.position = frame.viewProj * worldPos;
    out.normal = (transpose(object.invModel) * float4(in.normal,0)).xyz;
    return out;
}

float4 fragment fragmentMain3D_normal(
                                    VertexOutput3D_2 in [[stage_in]],
                                    constant FrameUniforms_2& frame [[buffer(FRAME_BUFFER_BIND)]],
                                    constant ObjectUniforms_2& object [[buffer(OBJ_BUFFER_BIND)]]
                                        ){
    return float4(in.normal,1.0);
}
