//
//  general.metal
//  metal_cpp
//
//  Created by Neel on 17/01/26.
//

#include <metal_stdlib>
using namespace metal;

struct VertexInput {
    float2 position [[attribute(0)]];
    float3 color [[attribute(1)]];
};

struct Boid {
    float2 position;
    float2 velocity;
};

struct VertexOutput {
    float4 position [[position]];
    float3 color;
};

VertexOutput vertex vertexMainBoid(VertexInput in [[stage_in]],
                                      constant float4x4& transform [[buffer(1)]],
                                      constant Boid* boids [[buffer(2)]],
                                      uint instanceID [[instance_id]]){
    VertexOutput out;
    
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
    out.color = in.color;
    return out;
}

float4 fragment fragmentMainBoid(VertexOutput frag [[stage_in]]){
//    return float4(0.0,0.0,0.0,0.0);
    return float4(frag.color,1.0);
}

