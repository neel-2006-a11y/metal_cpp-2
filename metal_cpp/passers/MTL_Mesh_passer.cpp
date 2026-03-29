//
//  MTL_Mesh_passer.cpp
//  metal_cpp
//
//  Created by Neel on 21/03/26.
//

#include "MTL_Mesh_passer.h"

void uploadMeshToGPU(Mesh2& mesh, MTL::Device* device){
    if(mesh.uploaded) return;
    
    // vertex buffer
    mesh.vertexBuffer = device->newBuffer(mesh.vertexData.data(), mesh.vertexData.size(), MTL::ResourceStorageModeManaged);
    
    // index buffer
    mesh.indexBuffer = device->newBuffer(mesh.indexData.data(), mesh.indexData.size() * sizeof(uint16_t), MTL::ResourceStorageModeManaged);
    
    mesh.uploaded = true;
}
