//
//  texture_manager.h
//  metal_cpp
//
//  Created by Neel on 18/03/26.
//

#pragma once
#include "config.h"

#include "resource_managers/texture.h"
#include "resource_managers/resource_types.h"

class TextureManager{
public:
    TextureID createTexture(TextureDesc& desc, void* data, bool no_cpu);
    TextureID createEmpty(TextureDesc& desc);
    TextureID createEmptyNoCPU(TextureDesc& desc);
    
    TextureID loadPPM(std::string& filename, TextureDesc& desc);
    TextureID loadPPMArray(std::vector<std::string>& filenames, TextureDesc& desc);
    
    TextureID loadFromFile(const std::string& path);
    
    Texture* get(TextureID id);
    
    void destroy(TextureID id);
    
private:
    std::unordered_map<TextureID, Texture> textures;
    std::unordered_map<std::string, TextureID> cache;
    TextureID nextID = 1;
};
