//
//  texture_types.h
//  metal_cpp
//
//  Created by Neel on 18/03/26.
//

#pragma once
#include <cstdint>

// ---------------
// Texture Format
// ---------------
enum class TextureFormat {
    Unknown = 0,

    R8Unorm,
    R16Unorm,
    RG8Unorm,
    RGBA8Unorm,
    RGBA16Float,

    Depth32Float
};

// byte helpers
inline size_t bytesPerPixel(TextureFormat format)
{
    switch (format)
    {
        case TextureFormat::R8Unorm:        return 1;
        case TextureFormat::R16Unorm:       return 2;
        case TextureFormat::RG8Unorm:       return 2;
        case TextureFormat::RGBA8Unorm:     return 4;
        case TextureFormat::RGBA16Float:    return 8;
        case TextureFormat::Depth32Float:   return 4;
        default: return 0;
    }
}


// ---------------
// Usage Flags
// ---------------
enum class TextureUsage : uint32_t {
    None          = 0,
    Sampled       = 1 << 0,
    RenderTarget  = 1 << 1,
    DepthStencil  = 1 << 2
};

// Bitmask helpers
inline TextureUsage operator|(TextureUsage a, TextureUsage b) {
    return (TextureUsage)((uint32_t)a | (uint32_t)b);
}
inline bool hasFlag(TextureUsage value, TextureUsage flag) {
    return ((uint32_t)value & (uint32_t)flag) != 0;
}

// ---------------
// Storage Mode
// ---------------
enum class StorageMode {
    Private,
    Shared,
    Managed,
    MemoryLess
};

// ---------------
// Texture Descriptor
// ---------------
struct TextureDesc{
    int width = 1;
    int height = 1;
    int layers = 1;
    TextureFormat format = TextureFormat::RGBA8Unorm;
    TextureUsage usage = TextureUsage::Sampled;
    StorageMode storageMode = StorageMode::Private;
    
    bool generateMipmaps = false;
};

