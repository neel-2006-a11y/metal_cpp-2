//
//  camera.cpp
//  metal_cpp
//
//  Created by Neel on 27/01/26.
//

#include "view/camera.h"
#include "backend/mtlm.h"
#include <cmath>
#include <stdio.h>

Camera::Camera(float aspect):
pos{0,0,-1},
yaw(90.0f),
pitch(0.0f),
fov(60.0f),
aspect(aspect),
nearZ(0.1f),
farZ(100.0f),
speed(10.0f)
{}

void Camera::setAspect(float aspect){
//    printf("camera ptr: %p\n", this);
    this->aspect = aspect;
}

void Camera::processKeyboard(float dx, float dy, float dz, float dt){
    float disp = speed * dt;
    
    pos += forward() * dz * disp;
    pos += right() * dx * disp;
    pos += up() * dy * disp;
}

void Camera::processMouse(float dx, float dy){
    float sensitivity = 0.1f;
    
    yaw += dx * sensitivity;
    pitch -= dy * sensitivity;
    if(pitch > 89.0f) pitch = 89.0f;
    if(pitch < -89.0f) pitch = -89.0f;
}

simd::float3 Camera::forward() const{
    float cy = cosf(yaw * M_PI / 180.0f);
    float sy = sinf(yaw * M_PI / 180.0f);
    float cp = cosf(pitch * M_PI / 180.0f);
    float sp = sinf(pitch * M_PI / 180.0f);
    simd::float3 f = { cy * cp, sp, sy * cp };
    return simd::normalize(f);
}

simd::float3 Camera::right() const {
    return simd::normalize(simd::cross(forward(), simd::float3{0, 1, 0}));
}

simd::float3 Camera::up() const {
    return simd::normalize(simd::cross(right(), forward()));
}

simd::float4x4 Camera::view() const {
    simd::float3 f = forward();
    simd::float3 u = up();
    
    return mtlm::look_at(pos, pos+f*1.0, u);
}

simd::float4x4 Camera::projection() const {
    return mtlm::perspective_projection(fov, aspect, nearZ, farZ);
//    float rangeY = 2.0f;
//    return mtlm::orthographic_projection(-rangeY*aspect, rangeY*aspect, -rangeY, rangeY, -farZ, farZ);
}

simd::float4x4 Camera::viewProjection() const {
    return projection() * view();
}



std::vector<float> Camera::computeCascadeSplits(int num_cascades, float lambda){
    std::vector<float> splits(num_cascades+1);
    splits[0] = this->nearZ;
    for(int i = 0; i < num_cascades; i++){
        float p = (i+1)/(float)num_cascades;
        
        float logSplit = this->nearZ * pow(this->farZ / this->nearZ, p);
        float linSplit = this->nearZ + (this->farZ-this->nearZ)*p;
        
        splits[i+1] = lambda * logSplit + (1-lambda) * linSplit;
    }
    return splits;
}

std::vector<worldFrustrum> Camera::computeCascadeFrustrums(std::vector<float> cascadeSplits){
    float ndcs[8][3] = {
        {-1,-1,0}, {1,-1,0}, {1,1,0}, {-1,1,0},
        {-1,-1,1}, {1,-1,1}, {1,1,1}, {-1,1,1},
    };
    int cascades = (int)cascadeSplits.size()-1;
    simd::float4x4 view = this->view();
    
    std::vector<worldFrustrum> frusts;
    for(int i=0;i<cascades;i++){
        simd::float4x4 proj = mtlm::perspective_projection(this->fov, this->aspect, cascadeSplits[i], cascadeSplits[i+1]);
        simd::float4x4 VP = proj * view;
        
        worldFrustrum frust;
        for (int i=0; i<8; i++) {
            simd::float4 worldPos = simd::inverse(VP) * simd::float4{ndcs[i][0], ndcs[i][1], ndcs[i][2], 1.0};
            worldPos /= worldPos.w;
            frust.points.push_back(worldPos.xyz);
        }
        frusts.push_back(frust);
    }
    
    return frusts;
}
