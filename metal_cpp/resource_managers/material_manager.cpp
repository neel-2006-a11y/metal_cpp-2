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

void MaterialManager::setDiffuseTexture(MaterialID id, TextureID tex){
    auto& mat = materials[id];
    for(auto& t : mat.textures){
        if(t.slot == DIFFUSE_SLOT){
            t.texture = tex;
            return;
        }
    }
    
    mat.textures.push_back({DIFFUSE_SLOT, tex});
    mat.uniforms.hasDiffuse = 1;
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

void MaterialManager::setPipeline(MaterialID id, PipelineID pipeline){
    auto& mat = materials[id];
    mat.pipeID = pipeline;
}


std::vector<MaterialUniforms> MaterialManager::getAllUniforms(){
    std::vector<MaterialUniforms> uniforms;
    
    uniforms.resize(materials.size());
    
    for(auto& mat : materials){
        uniforms[mat.first - 1] = mat.second.uniforms; // zero_based_indexing
    }
    
    return uniforms;
}
