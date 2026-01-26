//
//  boids_render.metal
//  metal_cpp
//
//  Created by Neel on 17/01/26.
//

#include <metal_stdlib>
using namespace metal;

struct VertexInputBoid {
    float2 position [[attribute(0)]];
    float3 color [[attribute(1)]];
};

struct Boid {
    float2 position;
    float2 velocity;
    float3 color;
};

struct VertexOutputBoid {
    float4 position [[position]];
    float3 color;
};

VertexOutputBoid vertex vertexMainBoid(VertexInputBoid in [[stage_in]],
                                      constant float4x4& transform [[buffer(1)]],
                                      device const Boid* boids [[buffer(2)]],
                                      uint instanceID [[instance_id]]){
    VertexOutputBoid out;
    
    Boid b = boids[instanceID];
    
    float2 dir = b.velocity;
    float len = length(dir);
    
    if(len > 0.0001)
        dir /= len;
    else
        dir = float2(1.0,0.0);
    
    float2 local = in.position;
    
    float2 rotated;
    rotated.x = local.x * dir.x - local.y * dir.y;
    rotated.y = local.x * dir.y + local.y * dir.x;
    
    float4 pos = transform * float4(rotated, 0.0, 1.0);
    pos.x += b.position.x;
    pos.y += b.position.y;
    
    
    out.position = pos;
//    out.color = in.color;
//    out.color = float3(
//        float(instanceID % 7) / 6.0,
//        float((instanceID / 7) % 7) / 6.0,
//        1.0
//    );

    out.color = b.color;
    return out;
}

float4 fragment fragmentMainBoid(VertexOutputBoid frag [[stage_in]]){
//    return float4(1.0,0.0,0.0,1.0);
    return float4(frag.color,1.0);
}

