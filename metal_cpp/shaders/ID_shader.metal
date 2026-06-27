//
//  ID_shader.metal
//  metal_cpp
//
//  Created by Neel on 26/05/26.
//

#include <metal_stdlib>
using namespace metal;

struct VertexInputID {
    float3 position [[attribute(0)]];
};

struct VertexOutputID {
    float4 position [[position]];
};

struct GizmoDrawData{
    float4x4 mvp;
    int ID;
};

VertexOutputID vertex vertex_ID(
                                VertexInputID in [[stage_in]],
                                constant GizmoDrawData& gizmoData [[buffer(1)]]
                                )
{
    VertexOutputID out;
    out.position = float4(in.position,1) * gizmoData.mvp;
    
    return out;
}

int fragment frag_ID(VertexOutputID in [[stage_in]],
                     constant GizmoDrawData& gizmoData [[buffer(1)]])
{
    return gizmoData.ID;
}
