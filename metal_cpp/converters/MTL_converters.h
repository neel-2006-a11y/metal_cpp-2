//
//  MTL_converters.h
//  metal_cpp
//
//  Created by Neel on 20/03/26.
//

#pragma once
#include "config.h"
#include "resource_managers/texture_types.h"
#include "resource_managers/texture_manager.h"
#include "resource_managers/mesh.h"
#include "resource_managers/material.h"

MTL::PixelFormat toMTLPixelFormat(TextureFormat format);

MTL::TextureUsage toMTLTextureUsage(TextureUsage usage);

MTL::StorageMode toMTLStorageMode(StorageMode storageMode);

MTL::TextureDescriptor* buildMTLTextureDesc(TextureDesc& desc);


MTL::VertexDescriptor* buildMTLVertexDescriptor(VertexLayout& layout);


