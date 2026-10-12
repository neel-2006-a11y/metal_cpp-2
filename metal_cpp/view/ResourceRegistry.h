//
//  ResourceRegistry.h
//  metal_cpp
//
//  Created by Neel on 05/07/26.
//

#pragma once

#include <typeindex>
#include <any>
#include <map>
#include "resource_managers/resource_types.h"

enum class ResourceKey : uint32_t{
    SceneColorTexture,
    SceneDepthTexture,
    SceneNormalTexture,
    
    ShadowMapTexture,
    VolumetricTexture,
    
    OutlineTexture,
    
    DensityTexture
};

struct EnumClassHash
{
    template<typename T>
    std::size_t operator()(T t) const
    {
        return static_cast<std::size_t>(t);
    }
};

class ResourceRegistry{
public:
    void publishTexture(ResourceKey resource, TextureID tex);
    
    TextureID getTexture(ResourceKey resource) const;
    
private:
    std::unordered_map<ResourceKey, TextureID, EnumClassHash> textureData;
};
