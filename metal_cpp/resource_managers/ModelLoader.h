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
#include "scene/RenderObject.h"
#include "config.h"

class ModelLoader{
public:
    ModelLoader(MeshManager* meshMgr, TextureManager* texMgr, MaterialManager* matMgr);
    
    void loadModel(const std::string& path, Scene* scene);
    
private:
    MeshManager* meshManager;
    TextureManager* textureManager;
    MaterialManager* materialManager;
    
    std::string directory;
    
    void processNode(aiNode* node, const aiScene* scene, Scene* outScene);
    RenderObject processMesh(aiMesh* mesh, const aiScene* scene);
    
    MaterialID loadMaterial(aiMaterial* mat);
};
