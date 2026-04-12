//
//  SceneNode.cpp
//  metal_cpp
//
//  Created by Neel on 11/04/26.
//

#include "scene/SceneNode.h"

void assignParent(SceneNode* child, SceneNode* parent){
    if(!child) return;
    child->parent = parent;
    
    if(parent){
        parent->children.push_back(child);
    }
}

SceneNode* createNode(SceneNode* parent){
    SceneNode* node = new SceneNode();
    node->parent = parent;
    if(parent) parent->children.push_back(node);
    return node;
}

void buildObjectBuffer(SceneNode* node, std::vector<ObjectUniforms>& buffer, uint32_t& index){
    if(!node->visible) return;
    
    if(node->dirty){
        node->worldMatrix = node->localT.computeModel();
        if(node->parent)
            node->worldMatrix = node->parent->worldMatrix * node->worldMatrix;
//            node->dirty = false (nothing is making it dirty right now)
    }
    
    if(node->renderObject){
        node->renderObject->objectIndex = index;
        
        ObjectUniforms obj;
        
        obj.model = node->worldMatrix;
        obj.invModel = simd_inverse(obj.model);
        obj.tileScale = node->renderObject->tileScale;
        
        buffer.push_back(obj);
        index++;
    }
    
    for(auto* child : node->children){
        buildObjectBuffer(child, buffer, index);
    }
}

void assignPipelineID(SceneNode* node, PipelineID pipeID, MaterialManager& matMgr){
    if(node->renderObject){
        matMgr.setPipeline(node->renderObject->materialID, pipeID);
    }
    for(auto* child : node->children){
        assignPipelineID(child, pipeID, matMgr);
    }
}
