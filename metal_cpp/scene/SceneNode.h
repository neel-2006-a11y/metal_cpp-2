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

struct Renderable{
    RenderComponent* renderComponent;
    
    ObjectUniforms objectUniforms;
};

namespace Node{
    void assignParent(SceneNode* child, SceneNode* parent);

    SceneNode* createNode(SceneNode* parent);

    SceneNode* findNode(SceneNode* parent, std::string name);

    void updateNode(SceneNode* node);

    void assignPipelineID(SceneNode* node, PipelineID pipeID, MaterialManager* matMgr);
    void copyMaterialBindings(SceneNode* node, MaterialID matID, MaterialManager* matMgr);
    void assignMaterialBuffer(SceneNode* node, MaterialUniforms MU, MaterialManager* matMgr);

    SceneNode* buildSampleSceneNode(RenderDevice renderDevice);

    void extractRenderables(SceneNode* node, std::vector<Renderable>& renderables);
}
