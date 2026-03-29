//
//  material_manager.cpp
//  metal_cpp
//
//  Created by Neel on 22/03/26.
//

#include "resource_managers/material_manager.h"


MaterialID MaterialManager::createMaterial(PipelineID pipeline){
    Material mat;
    mat.pipeID = pipeline;
    
    MaterialID id = nextID++;
    materials[id] = std::move(mat);
    
    return id;
}

void MaterialManager::setTexture(MaterialID id, uint32_t slot, TextureID tex){
    auto& mat = materials[id];
    
    // if slot exists update texture
    for(auto& t : mat.textures){
        if(t.slot == slot){
            t.texture = tex;
            return;
        }
    }
    
    // if slot doesn't exist create binding
    mat.textures.push_back({slot, tex});
}

void MaterialManager::setSampler(MaterialID id, uint32_t slot, void* sampler){
    auto& mat = materials[id];
    
    for(auto& s : mat.samplers){
        if(s.slot == slot){
            s.sampler = sampler;
            return;
        }
    }
    
    mat.samplers.push_back({slot, sampler});
}

Material* MaterialManager::get(MaterialID id)
{
    auto it = materials.find(id);
    if(it == materials.end()) return nullptr;
    return &it->second;
}
