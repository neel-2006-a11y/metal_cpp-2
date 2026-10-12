//
//  portal.h
//  metal_cpp
//
//  Created by Neel on 05/10/26.
//

#pragma once

#include "camera.h"
#include "resource_managers/resource_types.h"
#include "scene/RenderComponent.h"
#include "resource_managers/texture_manager.h"
#include "resource_managers/material_manager.h"
#include "resource_managers/mesh_manager.h"
#include "view/mesh_factory.h"

struct portal_AABB{
    simd::float3 p_mn, p_mx;
};

class Portal{
    Portal* otherPortal = nullptr; // the one u get out of
    
    float width, height, depth;
    
public:
    Transform transform;
    MeshID meshID;
    MaterialID materialID; // all portals have the same pipeline but different materials
    
public:
    Portal(float width, float height, Transform t);
    void assign_render_materials(MeshManager* meshMan, MaterialManager* matMan, PipelineID pipeID, SamplerID samplerID);
    void assign_other_portal(Portal* other);
    Camera* create_virtual(Camera* real_camera);
    portal_AABB get_AABB();
};
