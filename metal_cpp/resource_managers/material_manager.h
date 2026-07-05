//
//  material_manager.h
//  metal_cpp
//
//  Created by Neel on 22/03/26.
//

#pragma once
#include <unordered_map>
#include "resource_managers/material.h"


class MaterialManager
{
public:
    MaterialID createMaterial(PipelineID pipeline);
    
    void setTexture(MaterialID mat, uint32_t slot, TextureID tex);
    void setDiffuseTexture(MaterialID mat, TextureID tex);
    void setSampler(MaterialID mat, uint32_t slot, SamplerID sampler);
    void setPipeline(MaterialID mat, PipelineID pipeline);
    
    std::unordered_map<MaterialID, int> getAllUniforms(std::vector<MaterialUniforms>& out);
    
    Material* get(MaterialID id);
    
private:
    std::unordered_map<MaterialID, Material> materials;
    MaterialID nextID = 1;
};

