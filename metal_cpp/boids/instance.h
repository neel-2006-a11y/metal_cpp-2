//
//  instance.h
//  metal_cpp
//
//  Created by Neel on 17/01/26.
//
#pragma once

#include "simd/simd.h"

struct InstanceData
{
    float offset; // xoffset
};

struct Boid
{
    simd::float2 position;
    simd::float2 velocity;
};

struct SimParams {
    simd::float2 mousePos; //NDC space [-1, 1]
    float mouseForce;
    
    simd::float2 flowDirection;
    float flowStrength;
    
    uint32_t frame;
    uint32_t kCount;
    float dt;
};

struct FlowField {
    float angle = 0.0f;
    float angularVelocity = 0.0f;
};

#define MAX_BOIDS_PER_CELL 64
struct Cell {
    uint32_t count;
    uint32_t indices[MAX_BOIDS_PER_CELL];
};
