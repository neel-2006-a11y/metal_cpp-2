//
//  ModelLoader.h
//  metal_cpp
//
//  Created by Neel on 01/04/26.
//

#pragma once

#include <assimp/include/assimp/scene.h>
#include "resource_managers/mesh_manager.h"
#include "resource_managers/texture_manager.h"
#include "resource_managers/material_manager.h"
#include "scene/RenderComponent.h"
#include "config.h"

struct SceneNode;

class ModelLoader{
public:
    ModelLoader(MeshManager* meshMgr, TextureManager* texMgr, MaterialManager* matMgr);
    
    SceneNode* loadModelAsNode(const std::string& path);
    
private:
    MeshManager* meshManager;
    TextureManager* textureManager;
    MaterialManager* materialManager;
    
    std::string directory;
    
    SceneNode* processNode2(aiNode* node, const aiScene* scene);
    RenderComponent* processMesh2(aiMesh *mesh, const aiScene *scene);
    
    MaterialID loadMaterial(aiMaterial* mat);
};
