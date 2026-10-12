//
//  portal_texture_pool.h
//  metal_cpp
//
//  Created by Neel on 12/10/26.
//

#pragma once

#include "resource_managers/resource_types.h"
struct PooledTexture {
    TextureID id;
    bool inUse = false;
};

class RenderTexturePool{
public:
    TextureManager* textureManager = nullptr;
    std::vector<PooledTexture> textures;
    
    RenderTexturePool(){}
    
    void assign_texManager(TextureManager* texMan){
        textureManager = texMan;
    }
    
    TextureID aquire(){
        // first try to reuse
        for(auto& texture : textures){
            if(!texture.inUse){
                texture.inUse = true;
                return texture.id;
            }
        }
        
        // Nothing is available, so allocate another texture
        TextureID tID = createRenderTexture();
        
        textures.push_back({
            .id = tID,
            .inUse = true
        });
        
        return tID;
    }
    
    void release(TextureID id){
        for(auto& texture : textures){
            if(texture.id == id){
                assert(texture.inUse);
                texture.inUse = false;
                return;
            }
        }
    }
    
private:
    TextureID createRenderTexture(){
        TextureDesc tex_desc;
        tex_desc.width = PORTAL_RESOLUTION;
        tex_desc.height = tex_desc.width;
        tex_desc.format = TextureFormat::RGBA8Unorm;
        tex_desc.storageMode = StorageMode::Private;
        tex_desc.usage = TextureUsage::RenderTarget | TextureUsage::Sampled;
        
        auto tID = textureManager->createEmptyNoCPU(tex_desc);
        return tID;
    }
};
