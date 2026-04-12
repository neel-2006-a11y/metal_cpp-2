//
//  dither_runevision_2.metal
//  metal_cpp
//
//  Created by Neel on 28/03/26.
//

#include <metal_stdlib>
#include "common.metal"
using namespace metal;


VertexOutput3D_2 vertex vertexMain3D_dither_2(VertexInput3D_2 in [[stage_in]],
                                   constant FrameUniforms_2& frame[[buffer(1)]],
                                   constant ObjectUniforms_2& object[[buffer(2)]]
                                   ){
    VertexOutput3D_2 out;
    float4 worldPos = object.model * float4(in.position,1);
    out.position = frame.viewProj * worldPos;
    out.color = in.color;
    out.normal = (transpose(object.invModel) * float4(in.normal,0)).xyz;
    out.worldPos = worldPos.xyz;
    out.uv = in.uv;
    out.viewDepth = (frame.view * worldPos).z;
    return out;
}

float4 fragment fragmentMain3D_dither_2(
                                    VertexOutput3D_2 in [[stage_in]],
                                    texture2d_array<float> shadowMap [[texture(0)]],
                                    sampler shadowSampler [[sampler(0)]],
                                    texture2d_array<half> halftoneTex [[texture(1)]],
                                    sampler halftoneSampler [[sampler(1)]],
                                    texture2d<float> diffuse [[texture(2)]],
                                    constant FrameUniforms_2& frame [[buffer(1)]],
                                    constant ObjectUniforms_2& object [[buffer(2)]],
                                    constant MaterialUniforms& material [[buffer(3)]]
                                    ){
    float3 N = normalize(in.normal);
    float3 V = normalize(frame.cameraPos - in.worldPos);

    float3 L = normalize(-frame.direction);
    float3 H = normalize(L + V);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), 64.0);
    spec = 0;
    
    int selected_cascade = select_cascade_2(in.viewDepth, frame.cascadeSplits, frame.cascades);
    
    float shadow = shadowCalculationCSM_dither_2(in.worldPos, shadowMap, shadowSampler, frame.sunVPs[selected_cascade], selected_cascade);
    
    float3 diffuse_color = float3(1.0);
    if(material.hasDiffuse)
        diffuse_color = diffuse.sample(halftoneSampler, in.uv).xyz;
    
    float3 light =
        frame.color * frame.intensity * diff +
        frame.color * spec * 0.2;
    
    light *= (1.0-shadow);
    
    light += 0.05*float3(1.0); // ambient light
    
    float luminance = dot(light, float3(0.299, 0.587, 0.114));
    float luminance_max = dot(float3(1.0,1.0,1.0), float3(0.299, 0.587, 0.114));
    luminance /= luminance_max;
    
    // ----------------------------------------
    // UV change
    float2 duvdx = dfdx(in.uv);
    float2 duvdy = dfdy(in.uv);
    
    float MaxAdapt = exp2(10.0f);
    float MinAdapt = exp2(-3.0f);
    
    float pixelCoverage = max(length(duvdx), length(duvdy));
    pixelCoverage = max(pixelCoverage, 1e-6);
    float scale = (frame.BayerScale * object.tileScale / pixelCoverage) * sqrt(luminance);
    
    scale = clamp(scale, MinAdapt * sqrt(luminance), MaxAdapt * sqrt(luminance));
    float log_scale = log2(scale);
    float lod = floor(log_scale);
    float frac = fract(log_scale);
    int transition = 49*frac;
    
    float adaptiveScale = exp2(lod);
    
    float2 sampleUV = fract(in.uv * adaptiveScale);
    
    float value = halftoneTex.sample(halftoneSampler, sampleUV, transition).r;
    
    float threshold = 1-(luminance) +0.01;
    float dithered = value > threshold ? 1.0 : 0.0;
    
    float3 out;
    
    out = dithered * diffuse_color;
//    out = diffuse_color.b;
//    out = sampleUV.x;
//    out = scale == MinAdapt * sqrt(luminance) ? 1.0 : 0.0;
    
//    return float4(1.0,0.0,0.0,1.0);
//    return float4(out, out, out, 1.0);
    return float4(out, 1.0);
//    return float4(in.worldPos,1.0);
}
