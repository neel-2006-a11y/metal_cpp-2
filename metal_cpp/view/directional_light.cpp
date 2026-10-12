//
//  directional_light.cpp
//  metal_cpp
//
//  Created by Neel on 17/03/26.
//

#include "view/directional_light.h"

DirectionalLight::DirectionalLight(){
    proj.resize(cascades);
    viewProj.resize(cascades);
}

void DirectionalLight::update(Camera *camera){
    worldFrustrums.clear();
    computeCascadeSplits(camera, 0.5);
    computeCascadeFrustrums(camera);
    fitCameraFrustrums(worldFrustrums);
}

void DirectionalLight::computeCascadeSplits(Camera *camera, float lambda){
    cascadeSplits.resize(cascades+1);
    cascadeSplits[0] = camera->nearZ;
    for(int i = 0; i < cascades; i++){
        float p = (i+1)/(float)cascades;
        
        float logSplit = camera->nearZ * pow(camera->farZ / camera->nearZ, p);
        float linSplit = camera->nearZ + (camera->farZ-camera->nearZ)*p;
        
        cascadeSplits[i+1] = lambda * logSplit + (1-lambda) * linSplit;
    }
}

void DirectionalLight::computeCascadeFrustrums(Camera* camera){
    float ndcs[8][3] = {
        {-1,-1,0}, {1,-1,0}, {1,1,0}, {-1,1,0},
        {-1,-1,1}, {1,-1,1}, {1,1,1}, {-1,1,1},
    };
    int cascades = (int)cascadeSplits.size()-1;
    simd::float4x4 view = camera->view();
    
    for(int i=0;i<cascades;i++){
        simd::float4x4 proj = mtlm::perspective_projection(camera->fov, camera->aspect, cascadeSplits[i], cascadeSplits[i+1]);
        simd::float4x4 VP = proj * view;
        
        worldFrustrum frust;
        for (int i=0; i<8; i++) {
            simd::float4 worldPos = simd::inverse(VP) * simd::float4{ndcs[i][0], ndcs[i][1], ndcs[i][2], 1.0};
            worldPos /= worldPos.w;
            frust.points.push_back(worldPos.xyz);
        }
        worldFrustrums.push_back(frust);
    }
}

simd::float3 DirectionalLight::forward(){
    return simd::normalize(direction);
}

simd::float3 DirectionalLight::up(){
    simd::float3 f = forward();
    simd::float3 x = {1.0,0.0,0.0};
    if(fabs(simd::dot(f,x))>0.99) x = {0.0,1.0,0.0};
    simd::float3 u = simd::cross(f,x);
    return simd::normalize(u);
}

simd::float4x4 DirectionalLight::View(){
//        if(!viewDirty) return view;
    simd::float3 f = forward();
    simd::float3 u = up();
    view = mtlm::look_at(simd::float3(0), f, u);
    
    viewDirty = false;
    return view;
}

void DirectionalLight::fitCameraFrustrums(const std::vector<worldFrustrum>& frustrums){
    simd::float4x4 light_view = View();
    for (int i=0; i<cascades; i++) {
        simd::float3 max_ = simd::float3(-FLT_MAX);
        simd::float3 min_ = simd::float3(FLT_MAX);
        
        for(auto p : frustrums[i].points){
            simd::float4 p_light_space = light_view * simd::float4{p.x, p.y, p.z, 1.0};
            max_ = simd_max(max_, p_light_space.xyz);
            min_ = simd_min(min_, p_light_space.xyz);
        }

        
        float margin = 0.1f * fmax((max_.x - min_.x), (max_.y-min_.y));
        float marginZ = 200.0f;
        proj[i] = mtlm::orthographic_projection(min_.x-margin, max_.x+margin, min_.y-margin, max_.y+margin, min_.z - marginZ, max_.z + marginZ);
        viewProj[i] = proj[i] * light_view;
    }
}

void DirectionalLight::fitCameraFrustrums2Sphere(std::vector<worldFrustrum> frustrums){
    
    simd::float4x4 light_view = View();
    for (int i=0; i<cascades; i++) {
        simd::float3 center = 0;
        for(auto p : frustrums[i].points){
            center += p;
        }
        center /= 8;
        
        float radius = 0.0f;
        for(auto p : frustrums[i].points){
            float dist = simd::length(p-center);
            radius = fmax(radius, dist);
        }
        
        radius = ceil(radius * 16.0f) / 16.0f;
        
//        float margin = 0.1 * radius;
        proj[i] = mtlm::orthographic_projection(-radius, radius, -radius, radius, -300, 300);
        viewProj[i] = proj[i] * light_view;
    }
}

std::vector<simd::float4x4> DirectionalLight::ViewProj(){
    return viewProj;
}
