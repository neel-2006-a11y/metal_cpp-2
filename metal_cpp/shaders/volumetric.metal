//
//  volumetric.metal
//  metal_cpp
//
//  Created by Neel on 04/04/26.
//

#include <metal_stdlib>
#include "common.metal"
using namespace metal;

struct VSOut {
    float4 position [[position]];
    float2 uv;
};

vertex VSOut fullScreenVS(uint vid [[vertex_id]]){
    float2 pos[3] = {
        float2(-1.0, -1.0),
        float2( 3.0, -1.0),
        float2(-1.0,  3.0)
    };

    float2 uv[3] = {
        float2(0.0, 0.0),
        float2(2.0, 0.0),
        float2(0.0, 2.0)
    };

    VSOut out;
    out.position = float4(pos[vid], 0.0, 1.0);
    out.uv = uv[vid];
    return out;
}

fragment float4 volumetricFS(VSOut in [[stage_in]],
                             texture2d<float> depthTexture [[texture(0)]],
                             texture2d_array<float> shadowMap [[texture(1)]],
                             sampler samp [[sampler(0)]],
                             texture2d<float> blueNoise [[texture(2)]],
                             constant FrameUniforms_2& frame [[buffer(1)]]){
    
    float2 uv = in.uv;
    float2 uv_flipped = float2(uv.x, 1-uv.y);
    float depth = depthTexture.sample(samp, uv_flipped).r;
    float noise = blueNoise.sample(samp, uv).r;
    
    // reconstruct world pos
    float4 clip = float4(uv * 2.0 - 1.0, depth, 1.0);
    float4 world = frame.invViewProj * clip;
    world /= world.w;
    
    float3 rayOrigin = frame.cameraPos;
    float3 rayDir = normalize(world.xyz - rayOrigin);
    
    float maxDist = length(world.xyz - rayOrigin);
    
    float result = 0.0;
    
    const int STEPS = 32;
    float stepSize = maxDist / STEPS;
    float t = noise * stepSize;
//    float t = 0.0;
    
    // phase function
    float cosTheta = dot(rayDir, -frame.direction);
    
//    float phase = pow(max(cosTheta, 0.0), 8.0) + 0.5;
    float denom = 1.0 + frame.g * frame.g - 2 * frame.g * cosTheta;
    float phase = (1.0 - frame.g *frame.g)/(4.0 * M_PI_F * pow(denom, 1.5));
    
    for(int i = 0; i < STEPS; i++){
        float3 samplePos = rayOrigin + t * rayDir;
        
        int cascade = select_cascade_2((frame.view * float4(samplePos,1)).z, frame.cascadeSplits, frame.cascades);
        
        float lit = 1-shadowCalculationCSM_dither_2(samplePos, shadowMap, samp, frame.sunVPs[cascade], cascade);
        
        result += lit * frame.volumeDensity * stepSize * phase; // density
        t+=stepSize;
    }
    
    return float4(result, result, result, 1.0);
//    return world;
}

