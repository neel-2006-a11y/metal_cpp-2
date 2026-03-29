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
    TextureID createTexture(TextureDesc& desc, void* data);
    
    TextureID createEmpty(TextureDesc& desc);
    
    TextureID loadPPM(std::string& filename, TextureDesc& desc);
    
    TextureID loadPPMArray(std::vector<std::string>& filenames, TextureDesc& desc);
    Texture* get(TextureID id);
    
    void destroy(TextureID id);
    
private:
    std::unordered_map<TextureID, Texture> textures;
    TextureID nextID = 1;
};
