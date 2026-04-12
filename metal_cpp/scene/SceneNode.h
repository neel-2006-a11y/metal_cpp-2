//
//  SceneNode.h
//  metal_cpp
//
//  Created by Neel on 11/04/26.
//

#pragma once

#include "scene/RenderObject.h"
#include "resource_managers/material_manager.h"

struct SceneNode {
    // ---------------
    // Name
    // ---------------
    std::string name;
    
    // ---------------
    // Transform
    // ---------------
    Transform localT;
    
    simd::float4x4 worldMatrix = matrix_identity_float4x4;
    bool dirty = true;
    
    // ---------------
    // Hierarchy
    // ---------------
    SceneNode* parent = nullptr;
    std::vector<SceneNode*> children;
    
    // ---------------
    // Rendering // (should be component system here)
    // ---------------
    RenderObject* renderObject = nullptr;
    
    // ---------------
    // Flags
    // ---------------
    bool visible = true;
};

void assignParent(SceneNode* child, SceneNode* parent);

SceneNode* createNode(SceneNode* parent);

void buildObjectBuffer(SceneNode* node, std::vector<ObjectUniforms>& buffer, uint32_t& index);

void assignPipelineID(SceneNode* node, PipelineID pipeID, MaterialManager& matMgr);
