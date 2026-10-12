//
//  portal.cpp
//  metal_cpp
//
//  Created by Neel on 05/10/26.
//

#include "scene/portal.h"

Portal::Portal(float width, float height, Transform t){
    this->width = width;
    this->height = height;
    depth = 0.01f;
    this->transform = t;
}


void Portal::assign_render_materials(MeshManager* meshMan, MaterialManager* matMan, PipelineID pipeID, SamplerID samplerID){
    
    vertex_index_pair meshData = MeshFactory::buildCube2(simd::float3{width, height, depth});
    meshID = meshMan->createMesh(meshData);
    
    materialID = matMan->createMaterial(pipeID); // assign the textures on each pass
    matMan->setSampler(materialID, 1, samplerID);
}

void Portal::assign_other_portal(Portal *other){
    otherPortal = other;
}

Camera* Portal::create_virtual(Camera *real_camera){
    // the camera behind the other portal
    auto virtualCamera = new Camera(real_camera->aspect);
    
    if(!real_camera || !otherPortal)return nullptr;
    
    simd::quatf q180 = simd_quaternion(M_PI, simd::float3{0.0,1.0,0.0});
//    simd::quatf q180 = simd::quatf(0.0,1.0,0.0,0.0);
    
    //----------
    // position
    //----------
    simd::float3 relative_world = real_camera->position() - transform.position;
    simd::float3 relative_local = simd_act(simd::inverse(transform.rotation), relative_world);
    
    // rotate by 180 around Y axis
    relative_local = simd_act(q180, relative_local);
//    relative_local =
//    (R180 * simd::float4{relative_local.x,relative_local.y,relative_local.z,1.0f}).xyz;
    
    simd::float3 virtualPosition =
    otherPortal->transform.position +
    simd_act(otherPortal->transform.rotation, relative_local);
    
    virtualCamera->set_position(virtualPosition);
    
    //----------
    // orientation
    //----------
    simd::float3 forward = real_camera->forward();
    forward = simd_act(simd::inverse(transform.rotation), forward); // in portal local space
    forward = simd_act(q180, forward);
//    simd::float4 f = R180 * simd::float4{forward.x, forward.y, forward.z, 1.0}; // other portal's local space
    forward = simd_act(otherPortal->transform.rotation, forward); // world space
    forward = simd::normalize(forward);
    
    virtualCamera->setForward(forward);
    
    //----------
    // Exit portal's clipping plane
    //----------
    simd::float3 localNormal = simd::float3{0.0f,0.0f,1.0f};
//    simd::float3 worldNormal = (simd::float4x4(otherPortal->transform.rotation) * simd::float4{localNormal.x, localNormal.y, localNormal.z, 0.0}).xyz;
    simd::float3 worldNormal = simd_act(otherPortal->transform.rotation, localNormal);
    simd::float3 worldPoint = otherPortal->transform.position;
    float d = -simd::dot(worldNormal, worldPoint) - depth / 2 - 0.01; // padding
    
    virtualCamera->setPlane(simd::float4{worldNormal.x, worldNormal.y, worldNormal.z, d});
    
    return virtualCamera;
}

portal_AABB Portal::get_AABB(){
    portal_AABB aabb;
    simd::float4 corners[4] = {
        simd::float4{width/2, height/2, 0, 1.0},
        simd::float4{width/2, -height/2, 0, 1.0},
        simd::float4{-width/2, height/2, 0, 1.0},
        simd::float4{-width/2, -height/2, 0, 1.0}
    };
    
    for(int i=0; i<4; i++){
        corners[i] = transform.computeModel() * corners[i];
        aabb.p_mn = simd_min(corners[i].xyz, aabb.p_mn);
        aabb.p_mx = simd_max(corners[i].xyz, aabb.p_mx);
    }
    
    return aabb;
}
