//
//  mesh_manager.cpp
//  metal_cpp
//
//  Created by Neel on 18/03/26.
//

#include "resource_managers/mesh_manager.h"

MeshID MeshManager::createMesh(void* vertices, size_t vertexSize, std::vector<uint16_t>& indices){
    Mesh2 mesh;
    
    // copy vertex data
    mesh.vertexData.resize(vertexSize);
    std::memcpy(mesh.vertexData.data(), vertices, vertexSize);
    
    // copy index data
    mesh.indexData = indices;
    mesh.indexCount = static_cast<uint32_t>(indices.size());
    
    MeshID id = nextID++;
    
    meshes[id] = std::move(mesh);
    
    return id;
}

Mesh2* MeshManager::getMesh(MeshID id){
    auto it = meshes.find(id);
    if(it == meshes.end()) return nullptr;
    return &it->second;
}

void MeshManager::destroyMesh(MeshID id){
    meshes.erase(id);
}
