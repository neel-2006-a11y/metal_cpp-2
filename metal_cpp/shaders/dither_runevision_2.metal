//
//  dither_runevision_2.metal
//  metal_cpp
//
//  Created by Neel on 28/03/26.
//

#include <metal_stdlib>
#include "common.metal"
using namespace metal;

struct MU_dither{
    int hasDiffuse;
    int halftone_dim;
};


VertexOutput3D_2 vertex vertexMain3D_dither_2(
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
    return out;
}

FragmentOutput fragment fragmentMain3D_dither_2(
                                    VertexOutput3D_2 in [[stage_in]],
                                    texture2d_array<float> shadowMap [[texture(SHADOW_MAP_BIND)]],
                                    sampler shadowSampler [[sampler(0)]],
                                    texture2d_array<half> halftoneTex [[texture(1)]],
                                    sampler halftoneSampler [[sampler(1)]],
                                    texture2d<float> diffuse [[texture(DIFFUSE_BIND)]],
                                    constant FrameUniforms_2& frame [[buffer(FRAME_BUFFER_BIND)]],
                                    constant ObjectUniforms_2& object [[buffer(OBJ_BUFFER_BIND)]],
                                    constant ViewUniforms& view[[buffer(VIEW_BUFFER_BIND)]],
                                    constant MU_dither& material [[buffer(MAT_BUFFER_BIND)]]
                                    ){
    if(view.has_oblique_plane){
        if(dot(float4(in.worldPos,1.0), view.plane) < 0){
            discard_fragment();
        }
    }
    
    float3 N = normalize(in.normal);
    float3 V = normalize(view.cameraPos - in.worldPos);

    float3 L = normalize(-frame.direction);
    float3 H = normalize(L + V);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), 64.0);
    spec = 0;
    
    int selected_cascade = select_cascade(in.viewDepth * view.far, view.cascadeSplits, frame.cascades);
    
    float shadow = shadowCalculationCSM(in.worldPos, shadowMap, shadowSampler, view.sunVPs[selected_cascade], selected_cascade);
    
    float3 diffuse_color = float3(1.0);
    if(material.hasDiffuse == 1)
//        return float4(1.0,0,0,0);
        diffuse_color = diffuse.sample(halftoneSampler, in.uv).xyz;
//    return float4(diffuse_color,1.0);
    
    float3 light =
        frame.color * frame.intensity * diff +
        frame.color * spec * 0.2;
    
    light *= (1.0-shadow);
    
    light += 0.05*float3(1.0); // ambient light
    
    float luminance = dot(light, float3(0.299, 0.587, 0.114));
    float luminance_max = dot(float3(1.0,1.0,1.0), float3(0.299, 0.587, 0.114));
    luminance /= luminance_max;
    float sqrt_lum = sqrt(luminance);
    
    // ----------------------------------------
    // UV change
    float2 duvdx = dfdx(in.uv);
    float2 duvdy = dfdy(in.uv);
    
    float MaxAdapt = exp2(10.0f);
    float MinAdapt = exp2(-3.0f);
    
    float pixelCoverage = max(length(duvdx), length(duvdy));
    pixelCoverage = max(pixelCoverage, 1e-6);
    float scale = (frame.BayerScale / pixelCoverage) * sqrt_lum;
    
    scale = clamp(scale, MinAdapt * sqrt_lum, MaxAdapt * sqrt_lum);
    float log_scale = log2(scale);
    float lod = floor(log_scale);
    float frac = fract(log_scale);
    int transition = material.halftone_dim*frac;
//    int transition = 0;
    
    float adaptiveScale = exp2(lod);
    
    float2 sampleUV = fract(in.uv * adaptiveScale);
    
    float value = halftoneTex.sample(halftoneSampler, sampleUV, transition).r;
//    return float4(value,value,value,1.0);
    
    float threshold = 1-(luminance) + 0.01; // radius of white dot
    float dithered = value > threshold ? 1.0 : 0.0;
    
    float3 out_color;
    
    out_color = dithered * diffuse_color;
    
    
    FragmentOutput out;
    out.color = float4(out_color,1.0);
    out.normal = float4(N, in.viewDepth);
    return out;
}
