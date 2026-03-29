//
//  MTL_Texture_passer.h
//  metal_cpp
//
//  Created by Neel on 21/03/26.
//

#pragma once
#include "resource_managers/texture.h"
#include "config.h"

void uploadTextureToGPU(Texture& tex, MTL::Device* device);
void uploadEmptyTextureToGPU(Texture& tex, MTL::Device* device);
