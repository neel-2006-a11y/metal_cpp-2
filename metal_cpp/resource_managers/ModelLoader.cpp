//
//  ModelLoader.cpp
//  metal_cpp
//
//  Created by Neel on 01/04/26.
//

#include "resource_managers/ModelLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "backend/mtlm.h"

ModelLoader::ModelLoader(MeshManager* meshMgr, TextureManager* texMgr, MaterialManager* matMgr):
meshManager(meshMgr), textureManager(texMgr), materialManager(matMgr){}

SceneNode* ModelLoader::loadModelAsNode(const std::string &path){
    Assimp::Importer importer;
    
    const aiScene* aScene = importer.ReadFile(path,
                                              aiProcess_Triangulate |
                                              aiProcess_GenNormals |
                                              aiProcess_CalcTangentSpace |
                                              aiProcess_FlipUVs);
    if(!aScene || !aScene->mRootNode){
        printf("Assimp error: %s\n", importer.GetErrorString());
        return nullptr;
    }
    
    printf("Meshes in file: %d\n", aScene->mNumMeshes);
    directory = path.substr(0, path.find_last_of("/"));
    SceneNode* node = processNode2(aScene->mRootNode, aScene);
    
    printf("Model Loaded: %s\n", path.c_str());
    return node;
}

SceneNode* ModelLoader::processNode2(aiNode* ainode, const aiScene* scene){
    SceneNode* node = new SceneNode();
    node->name = ainode->mName.C_Str();
    
    // transform
    aiVector3D pos, scl;
    aiQuaternion rot;
    ainode->mTransformation.Decompose(scl, rot, pos);
    
    node->localT.position = {pos.x, pos.y, pos.z};
    node->localT.rotation = simd_quaternion(rot.x, rot.y, rot.z, rot.w); // debug
    node->localT.scale = {scl.x, scl.y, scl.z};
    
    for(uint32_t i = 0; i < ainode->mNumMeshes; i++){
        aiMesh* mesh = scene->mMeshes[ainode->mMeshes[i]];
        
        SceneNode* meshNode = new SceneNode();
        meshNode->parent = node;
        node->children.push_back(meshNode);
        
        meshNode->name = mesh->mName.C_Str();
        meshNode->renderObject = processMesh2(mesh, scene);
        meshNode->renderObject->tileScale = 1.0;
    }
    
    // children
    for(uint32_t i=0; i<ainode->mNumChildren; i++){
        SceneNode* child = processNode2(ainode->mChildren[i], scene);
        assignParent(child, node);
    }
    
    return node;
}

RenderObject* ModelLoader::processMesh2(aiMesh *mesh, const aiScene *scene){
    std::vector<Vertex3D> vertices;
    std::vector<Index> indices;
    
    // ----------
    // Vertices
    // ----------
    for(uint32_t i = 0; i < mesh->mNumVertices; i++){
        Vertex3D v;
        v.position = {
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        };
        
        v.normal = mesh->HasNormals()
        ? simd::float3{
            mesh->mNormals[i].x,
            mesh->mNormals[i].y,
            mesh->mNormals[i].z
        }
        : simd::float3{0,1,0};
        
        if(mesh->mTextureCoords[0]){
            v.uv = {
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            };
        }else{
            v.uv = {0,0};
        }
        
        v.color = {1,1,1};
        vertices.push_back(v);
    }
    
    // ----------
    // Indices
    // ----------
    for(uint32_t i=0; i<mesh->mNumFaces; i++){
        aiFace face = mesh->mFaces[i];
        for(uint32_t j=0; j<face.mNumIndices; j++){
            indices.push_back(face.mIndices[j]);
        }
    }
    
    // ----------
    // Create Mesh
    // ----------
    MeshID meshID = meshManager->createMesh(vertices.data(), vertices.size() * sizeof(Vertex3D), indices);
    
    // ----------
    // Material
    // ----------
    aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
    MaterialID matID = loadMaterial(mat);
    
    // ----------
    // Build Object
    // ----------
    RenderObject* obj = new RenderObject();
    obj->meshID = meshID;
    obj->materialID = matID;
    
    return obj;
}

void ModelLoader::loadModel(const std::string &path, Scene *scene){
    Assimp::Importer importer;
    
    const aiScene* aScene = importer.ReadFile(path,
                                              aiProcess_Triangulate |
                                              aiProcess_GenNormals |
                                              aiProcess_CalcTangentSpace |
                                              aiProcess_FlipUVs);
    if(!aScene || !aScene->mRootNode){
        printf("Assimp error: %s\n", importer.GetErrorString());
        return;
    }
    
    printf("Meshes in file: %d\n", aScene->mNumMeshes);
    directory = path.substr(0, path.find_last_of("/"));
    processNode(aScene->mRootNode, aScene, scene);
    
    printf("Model Loaded: %s\n", path.c_str());
}

void ModelLoader::processNode(aiNode *node, const aiScene *scene, Scene *outScene){
    for(unsigned int i = 0; i<node->mNumMeshes; i++){
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        printf("Vertices: %d\n", mesh->mNumVertices);
        
        Entity objE;
        objE.renderObject = processMesh(mesh, scene);
        
        // Apply node Transform
        aiVector3D pos;
        aiQuaternion rot;
        aiVector3D scl;
        
        node->mTransformation.Decompose(scl, rot, pos);
        
        objE.transform.position = {pos.x, pos.y, pos.z};
        objE.transform.rotation = simd_quaternion(rot.x, rot.y, rot.z, rot.w); // debug
        objE.transform.scale = {scl.x, scl.y, scl.z};
        
        outScene->objects.push_back(objE);
    }
    
    // Recurse
    for(unsigned int i=0; i<node->mNumChildren; i++){
        processNode(node->mChildren[i], scene, outScene);
    }
}

RenderObject ModelLoader::processMesh(aiMesh *mesh, const aiScene *scene){
    std::vector<Vertex3D> vertices;
    std::vector<Index> indices;
    
    // ----------
    // Vertices
    // ----------
    for(uint32_t i = 0; i < mesh->mNumVertices; i++){
        Vertex3D v;
        v.position = {
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        };
        
        v.normal = mesh->HasNormals()
        ? simd::float3{
            mesh->mNormals[i].x,
            mesh->mNormals[i].y,
            mesh->mNormals[i].z
        }
        : simd::float3{0,1,0};
        
        if(mesh->mTextureCoords[0]){
            v.uv = {
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            };
        }else{
            v.uv = {0,0};
        }
        
        v.color = {1,1,1};
        vertices.push_back(v);
    }
    
    // ----------
    // Indices
    // ----------
    for(uint32_t i=0; i<mesh->mNumFaces; i++){
        aiFace face = mesh->mFaces[i];
        for(uint32_t j=0; j<face.mNumIndices; j++){
            indices.push_back(face.mIndices[j]);
        }
    }
    
    // ----------
    // Create Mesh
    // ----------
    MeshID meshID = meshManager->createMesh(vertices.data(), vertices.size() * sizeof(Vertex3D), indices);
    
    // ----------
    // Material
    // ----------
    aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
    MaterialID matID = loadMaterial(mat);
    
    // ----------
    // Build Object
    // ----------
    RenderObject obj;
    obj.meshID = meshID;
    obj.materialID = matID;
    
    return obj;
}

MaterialID ModelLoader::loadMaterial(aiMaterial *mat){
    MaterialID materialID = materialManager->createMaterial(INVALID_SHADER); // assign pipeline later
    
    aiString path;
    
    // Diffuse Texture
    if(mat->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS){
        std::string texPath = directory + "/" + path.C_Str();
        
        TextureID tex = textureManager->loadFromFile(texPath);
        materialManager->setDiffuseTexture(materialID, tex);
    }
    
    // extend to normal, roughness etc. later
    
    return materialID;
}
