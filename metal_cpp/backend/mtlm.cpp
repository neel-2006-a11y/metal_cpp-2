//
//  mtlm.cpp
//  metal_cpp
//
//  Created by Neel on 16/01/26.
//

#include "backend/mtlm.h"

simd::float4x4 mtlm::identity(){
    simd_float4 col0 = {1.0f,0.0f,0.0f,0.0f};
    simd_float4 col1 = {0.0f,1.0f,0.0f,0.0f};
    simd_float4 col2 = {0.0f,0.0f,1.0f,0.0f};
    simd_float4 col3 = {0.0f,0.0f,0.0f,1.0f};
    return simd_matrix(col0, col1, col2, col3);
}

simd::float4x4 mtlm::translation(simd::float3 dPos){
    simd_float4 col0 = {1.0f,0.0f,0.0f,0.0f};
    simd_float4 col1 = {0.0f,1.0f,0.0f,0.0f};
    simd_float4 col2 = {0.0f,0.0f,1.0f,0.0f};
    simd_float4 col3 = {dPos[0],dPos[1],dPos[2],1.0f};
    return simd_matrix(col0, col1, col2, col3);
}

simd::float4x4 mtlm::z_rotation(float theta){
    theta = theta * M_PI / 180.0f;
    float c = cosf(theta);
    float s = sinf(theta);
    simd_float4 col0 = { c, s, 0.0f, 0.0f};
    simd_float4 col1 = { -s, c, 0.0f, 0.0f};
    simd_float4 col2 = { 0.0f, 0.0f, 1.0f, 0.0f};
    simd_float4 col3 = { 0.0f, 0.0f, 0.0f, 1.0f};
    return simd_matrix(col0, col1, col2, col3);
}

simd::float4x4 mtlm::y_rotation(float theta){
    theta = theta * M_PI / 180.0f;
    float c = cosf(theta);
    float s = sinf(theta);
    simd_float4 col0 = { -s, 0.0, c, 0.0f};
    simd_float4 col1 = { 0.0, 1.0, 0.0f, 0.0f};
    simd_float4 col2 = { c, 0.0f, s, 0.0f};
    simd_float4 col3 = { 0.0f, 0.0f, 0.0f, 1.0f};
    return simd_matrix(col0, col1, col2, col3);
}

simd::float4x4 mtlm::x_rotation(float theta){
    theta = theta * M_PI / 180.0f;
    float c = cosf(theta);
    float s = sinf(theta);
    simd_float4 col0 = { 1.0, 0.0, 0.0, 0.0f};
    simd_float4 col1 = { 0.0, c, s, 0.0f};
    simd_float4 col2 = { 0.0, -s, c, 0.0f};
    simd_float4 col3 = { 0.0f, 0.0f, 0.0f, 1.0f};
    return simd_matrix(col0, col1, col2, col3);
}

simd::float4x4 mtlm::scale(float factor){
    simd_float4 col0 = {factor,0.0f,0.0f,0.0f};
    simd_float4 col1 = {0.0f,factor,0.0f,0.0f};
    simd_float4 col2 = {0.0f,0.0f,factor,0.0f};
    simd_float4 col3 = {0.0f,0.0f,0.0f,1.0f};
    return simd_matrix(col0, col1, col2, col3);
}

simd::float4x4 mtlm::scale3D(simd::float3 factor){
    simd_float4 col0 = {factor.x,0.0f,0.0f,0.0f};
    simd_float4 col1 = {0.0f,factor.y,0.0f,0.0f};
    simd_float4 col2 = {0.0f,0.0f,factor.z,0.0f};
    simd_float4 col3 = {0.0f,0.0f,0.0f,1.0f};
    return simd_matrix(col0, col1, col2, col3);
}

simd::float4x4 mtlm::perspective_projection(float fovy, float aspect, float near, float far){
    fovy = fovy * M_PI / 360.0f; // half_fovy
    float t = tanf(fovy);
    float A = 1.0f / (t * aspect);
    float B = 1.0f / t;
    float C = far / (far - near);
    float D = -near * far/(far - near);
    simd_float4 col0 = {    A, 0.0f, 0.0f, 0.0f};
    simd_float4 col1 = { 0.0f,    B, 0.0f, 0.0f};
    simd_float4 col2 = { 0.0f, 0.0f,    C, 1.0f};
    simd_float4 col3 = { 0.0f, 0.0f,    D, 0.0f};

    return simd_matrix(col0, col1, col2, col3);
}

simd::float4x4 mtlm::look_at(simd::float3 from, simd::float3 target, simd::float3 up){
    simd::float3 f = simd::normalize(target-from);
    simd::float3 r = simd::normalize(simd::cross(f, up));
    simd::float3 u = simd::normalize(simd::cross(r,f));
    
    
    simd::float4 col0 = { r.x, u.x, f.x, 0 };
    simd::float4 col1 = { r.y, u.y, f.y, 0 };
    simd::float4 col2 = { r.z, u.z, f.z, 0 };
    simd::float4 col3 = {
        -simd::dot(r, from),
        -simd::dot(u, from),
        -simd::dot(f, from),
        1
    };
    
    return simd_matrix(col0,col1,col2,col3);
}

simd::float4x4 mtlm::orthographic_projection(float x1, float x2, float y1, float y2, float z1, float z2){
    simd::float4 col0 = { 2.0f/(x2-x1), 0, 0, 0 };
    simd::float4 col1 = { 0, 2.0f/(y2-y1), 0, 0 };
    simd::float4 col2 = { 0, 0, 1.0f/(z2-z1), 0};
    simd::float4 col3 = { -(x2+x1)/(x2-x1), -(y2+y1)/(y2-y1), -z1/(z2-z1), 1};
    return simd_matrix(col0, col1, col2, col3);
}
