//
//  MTL_material_passer.h
//  metal_cpp
//
//  Created by Neel on 22/03/26.
//

#pragma once
#include "config.h"
#include "resource_managers/material_manager.h"
#include "resource_managers/texture_manager.h"
#include "resource_managers/pipeline_manager.h"
#include "view/renderer2.h"

void bindMaterial(MaterialID mat_id, Material& mat, MTL::RenderCommandEncoder* encoder, TextureManager& texMgr, Renderer2& renderer);
