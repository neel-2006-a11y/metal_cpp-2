//
//  mesh_manager.h
//  metal_cpp
//
//  Created by Neel on 18/03/26.
//

#pragma once
#include "resource_managers/mesh.h"
#include "resource_managers/resource_types.h"
#include <unordered_map>
#include <vector>
#include "config.h"

class MeshManager{
public:
    MeshID createMesh(void* vertices, size_t vertexSize, std::vector<Index>& indices);
    
    Mesh2* getMesh(MeshID id);
    
    void destroyMesh(MeshID id);
    
private:
    std::unordered_map<MeshID, Mesh2> meshes;
    MeshID nextID = 1;
};
