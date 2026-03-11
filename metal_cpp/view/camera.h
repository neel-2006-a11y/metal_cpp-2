//
//  camera.h
//  metal_cpp
//
//  Created by Neel on 27/01/26.
//

#pragma once
#include <simd/simd.h>

class Camera {
public:
    Camera(float aspect);
    
    // Input
    void processKeyboard(float dx, float dy, float dz, float dt);
    void processMouse(float dx, float dy);
    
    // Resize
    void setAspect(float aspect);
    
    simd::float4x4 view() const;
    simd::float4x4 projection() const;
    simd::float4x4 viewProjection() const;
    
    simd::float3 position() const {return pos;}
    bool relativeMouse = false;
    
private:
    // state
    simd::float3 pos;
    float yaw;
    float pitch;
    float speed;

    
    // projection
    float fov;
    float aspect;
    float nearZ;
    float farZ;
    
    // Internal helpers
    simd::float3 forward() const;
    simd::float3 right() const;
    simd::float3 up() const;
};
