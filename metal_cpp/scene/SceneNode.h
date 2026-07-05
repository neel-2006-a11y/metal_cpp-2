//
//  SceneNode.h
//  metal_cpp
//
//  Created by Neel on 11/04/26.
//

#pragma once

#include "scene/RenderComponent.h"
#include "resource_managers/material_manager.h"
#include "view/renderContext.h"

struct SceneNode {
    // ---------------
    // Name
    // ---------------
    std::string name;
    
    // ---------------
    // Transform
    // ---------------
    Transform localT;
    
    simd::float4x4 worldRotation = matrix_identity_float4x4;
    simd::float4x4 worldMatrix = matrix_identity_float4x4;
    
    // ---------------
    // Hierarchy
    // ---------------
    SceneNode* parent = nullptr;
    std::vector<SceneNode*> children;
    
    // ---------------
    // Rendering // (should be component system here)
    // ---------------
    RenderComponent* renderComponent = nullptr;
    
    // ---------------
    // Flags
    // ---------------
    bool visible = true;
};

void assignParent(SceneNode* child, SceneNode* parent);

SceneNode* createNode(SceneNode* parent);


void updateNode(SceneNode* node);

struct Renderable{
    RenderComponent* renderComponent;
    
    ObjectUniforms objectUniforms;
};

void assignPipelineID(SceneNode* node, PipelineID pipeID, MaterialManager& matMgr);

SceneNode* buildSampleSceneNode(RenderDevice renderDevice);

void extractRenderables(SceneNode* node, std::vector<Renderable>& renderables);
