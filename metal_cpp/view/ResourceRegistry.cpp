//
//  ResourceRegistry.cpp
//  metal_cpp
//
//  Created by Neel on 05/07/26.
//

#include "view/ResourceRegistry.h"

void ResourceRegistry::publishTexture(ResourceKey resourcekey, TextureID tex){
    textureData[resourcekey] = tex;
}

TextureID ResourceRegistry::getTexture(ResourceKey resource) const{
    auto it = textureData.find(resource);
    if(it == textureData.end())return INVALID_TEXTURE;
    return it->second;
}
