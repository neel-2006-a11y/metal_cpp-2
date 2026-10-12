//
//  renderView.h
//  metal_cpp
//
//  Created by Neel on 09/10/26.
//

#pragma once

#include "view/camera.h"
#include "resource_managers/resource_types.h"

struct MaterialOverride{
    MaterialID mat;
    std::vector<TextureBinding> bindings;
};

struct RenderView {
    Camera* camera;
    
    bool useDrawable = false;
    TextureID renderTarget;
    
    std::vector<MaterialOverride> mat_overrides;
};
