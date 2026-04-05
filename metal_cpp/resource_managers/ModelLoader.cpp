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
        RenderObject obj = processMesh(mesh, scene);
        
        // Apply node Transform
        aiMatrix4x4 t = node->mTransformation;
        simd::float4x4 transform = {
            simd::float4{t.a1, t.b1, t.c1, t.d1},
            simd::float4{t.a2, t.b2, t.c2, t.d2},
            simd::float4{t.a3, t.b3, t.c3, t.d3},
            simd::float4{t.a4, t.b4, t.c4, t.d4}
        };
        
        obj.uniforms.model = transform;
        obj.uniforms.invModel = simd_inverse(transform);
        
        outScene->objects.push_back(obj);
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
    
    obj.uniforms.model = mtlm::identity();
    obj.uniforms.invModel = mtlm::identity();
    obj.uniforms.tileScale = {1,1,1};
    
    return obj;
}

MaterialID ModelLoader::loadMaterial(aiMaterial *mat){
    MaterialID materialID = materialManager->createMaterial(INVALID_SHADER); // assign pipeline later
    
    aiString path;
    
    // Diffuse Texture
    if(mat->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS){
        std::string texPath = directory + "/" + path.C_Str();
        
        TextureID tex = textureManager->loadFromFile(texPath);
        materialManager->setTexture(materialID, 2, tex);
    }
    
    // extend to normal, roughness etc. later
    
    return materialID;
}
