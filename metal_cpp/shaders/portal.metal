//
//  portal.metal
//  metal_cpp
//
//  Created by Neel on 12/10/26.
//

#include <metal_stdlib>
#include "common.metal"
using namespace metal;


VertexOutput3D_2 vertex vertexMain3D_portal(
                                              VertexInput3D_2 in [[stage_in]],
                                              
                                              constant FrameUniforms_2& frame[[buffer(FRAME_BUFFER_BIND)]],
                                              constant ObjectUniforms_2& object[[buffer(OBJ_BUFFER_BIND)]],
                                              constant ViewUniforms& view[[buffer(VIEW_BUFFER_BIND)]]
                                   ){
    VertexOutput3D_2 out;
    float4 worldPos = object.model * float4(in.position,1);
    out.position = view.viewProj * worldPos;
    out.color = in.color;
    out.normal = (transpose(object.invModel) * float4(in.normal,0)).xyz;
    out.worldPos = worldPos.xyz;
    out.uv = in.uv;
    out.viewDepth = (view.view * worldPos).z / view.far;
    out.cameraClipPos = out.position;
    return out;
}

FragmentOutput fragment fragmentMain3D_portal(
                                    VertexOutput3D_2 in [[stage_in]],
                                    sampler diffuseSampler [[sampler(1)]],
                                    texture2d<float> diffuse [[texture(DIFFUSE_BIND)]],
                                    constant FrameUniforms_2& frame [[buffer(FRAME_BUFFER_BIND)]],
                                    constant ObjectUniforms_2& object [[buffer(OBJ_BUFFER_BIND)]],
                                    constant ViewUniforms& view[[buffer(VIEW_BUFFER_BIND)]]
                                    ){
    if(view.has_oblique_plane){
        if(dot(float4(in.worldPos,1.0), view.plane) < 0){
            discard_fragment();
        }
    }
    
    float3 N = normalize(in.normal);
    float3 z_N = normalize( ( transpose(object.invModel) * float4(0,0,1,0) ).xyz);
    
    float2 ndc = in.cameraClipPos.xy / in.cameraClipPos.w;

    float2 cameraUV = float2(
        ndc.x * 0.5 + 0.5,
        0.5 - ndc.y * 0.5
    );

    float3 out_color;
//    out_color = float3(0.0,1.0,0.0);
//    out_color = float3(cameraUV, 0.0);
    out_color = diffuse.sample(diffuseSampler, cameraUV).xyz;
    out_color += 0.1 * float3(0.6627,0.5608,0.9216);
    if(dot(N,z_N)<0.8)out_color = float3(0.0,0.0,0.0);
    
    
    
    FragmentOutput out;
    out.color = float4(out_color,1.0);
    out.normal = float4(N, in.viewDepth);
    return out;
}
