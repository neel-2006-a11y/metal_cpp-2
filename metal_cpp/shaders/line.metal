//
//  line.metal
//  metal_cpp
//
//  Created by Neel on 19/01/26.
//

#include <metal_stdlib>
using namespace metal;

struct VertexInputLine {
    float3 position [[attribute(0)]];
};

struct VertexOutputLine {
    float4 position [[position]];
};

struct GizmoDrawData{
    float4x4 mvp;
    float4 color;
};

VertexOutputLine vertex lineVertex(
                                   VertexInputLine in [[stage_in]],
                                   constant GizmoDrawData& gizmoData [[buffer(1)]]
                                   )
{
    VertexOutputLine out;
    
    out.position = float4(in.position, 1) * gizmoData.mvp;

    return out;
}


float4 fragment lineFragment(
                             VertexOutputLine in [[stage_in]],
                             constant GizmoDrawData& gizmoData [[buffer(1)]]
                             )
{
    return gizmoData.color;
}
