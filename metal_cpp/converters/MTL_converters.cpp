//
//  MTL_converters.cpp
//  metal_cpp
//
//  Created by Neel on 20/03/26.
//

#include "converters/MTL_converters.h"
#include "passers/MTL_Texture_passer.h"

inline MTL::PixelFormat toMTLPixelFormat(TextureFormat format)
{
    switch (format)
    {
        case TextureFormat::R8Unorm:        return MTL::PixelFormatR8Unorm;
        case TextureFormat::R16Unorm:       return MTL::PixelFormatR16Unorm;
        case TextureFormat::RG8Unorm:       return MTL::PixelFormatRG8Unorm;
        case TextureFormat::RGBA8Unorm:     return MTL::PixelFormatBGRA8Unorm_sRGB;
        case TextureFormat::RGBA16Float:    return MTL::PixelFormatRGBA16Float;
        case TextureFormat::Depth32Float:   return MTL::PixelFormatDepth32Float;
        default:                            return MTL::PixelFormatInvalid;
    }
}

inline MTL::TextureUsage toMTLUsage(TextureUsage usage)
{
    MTL::TextureUsage result = MTL::TextureUsageUnknown;

    if (hasFlag(usage, TextureUsage::Sampled))
        result |= MTL::TextureUsageShaderRead;

    if (hasFlag(usage, TextureUsage::RenderTarget))
        result |= MTL::TextureUsageRenderTarget;

    if (hasFlag(usage, TextureUsage::DepthStencil))
        result |= MTL::TextureUsageRenderTarget;

    return result;
}

inline MTL::StorageMode toMTLStorageMode(StorageMode storageMode){
    switch(storageMode)
    {
        case StorageMode::Private:    return MTL::StorageModePrivate;
        case StorageMode::Shared:     return MTL::StorageModeShared;
        case StorageMode::Managed:    return MTL::StorageModeManaged;
        case StorageMode::MemoryLess: return MTL::StorageModeMemoryless;
    }
}

MTL::TextureDescriptor* buildMTLTextureDesc(TextureDesc& desc)
{
    auto mtlDesc = MTL::TextureDescriptor::alloc()->init();

    // --------------------
    // Texture Type
    // --------------------
    if (desc.layers > 1)
    {
        mtlDesc->setTextureType(MTL::TextureType2DArray);
        mtlDesc->setArrayLength(desc.layers);
    }
    else
    {
        mtlDesc->setTextureType(MTL::TextureType2D);
    }

    // --------------------
    // Dimensions
    // --------------------
    mtlDesc->setWidth(desc.width);
    mtlDesc->setHeight(desc.height);
    mtlDesc->setDepth(1);

    // --------------------
    // Format
    // --------------------
    auto pf = toMTLPixelFormat(desc.format);
    if (pf == MTL::PixelFormatInvalid) {
        std::cerr << "Invalid pixel format!\n";
        return nullptr;
    }
    mtlDesc->setPixelFormat(pf);

    // --------------------
    // Usage
    // --------------------
    mtlDesc->setUsage(toMTLUsage(desc.usage));

    // --------------------
    // Storage Mode
    // --------------------
    mtlDesc->setStorageMode(toMTLStorageMode(desc.storageMode));

    // --------------------
    // Mipmaps
    // --------------------
    if (desc.generateMipmaps)
    {
        int levels = 1 + (int)floor(log2(std::max(desc.width, desc.height)));
        mtlDesc->setMipmapLevelCount(levels);
    }
    else
    {
        mtlDesc->setMipmapLevelCount(1);
    }

    return mtlDesc;
}

MTL::VertexDescriptor* buildMTLVertexDescriptor(VertexLayout& layout){
    auto desc = MTL::VertexDescriptor::alloc()->init();
    
    for(auto& attrib : layout.attributes){
        auto mtlAttr = desc->attributes()->object(attrib.location);
        
        mtlAttr->setOffset(attrib.offset);
        mtlAttr->setBufferIndex(0);
        
        switch (attrib.type) {
                
            case VertexAttributeType::Float:
                mtlAttr->setFormat(MTL::VertexFormatFloat);
                break;
                
            case VertexAttributeType::Float2:
                mtlAttr->setFormat(MTL::VertexFormatFloat2);
                break;
                
            case VertexAttributeType::Float3:
                mtlAttr->setFormat(MTL::VertexFormatFloat3);
                break;
                
            case VertexAttributeType::Float4:
                mtlAttr->setFormat(MTL::VertexFormatFloat4);
                break;
                
            default:
                break;
        }
    }
    
    desc->layouts()->object(0)->setStride(layout.stride);
    
    return desc;
}

