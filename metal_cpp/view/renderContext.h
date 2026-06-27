//
//  renderContext.h
//  metal_cpp
//
//  Created by Neel on 10/06/26.
//

#pragma once

#include "view/renderer2.h"
#include "resource_managers/material_manager.h"
#include "resource_managers/mesh_manager.h"
#include "resource_managers/pipeline_manager.h"
#include "resource_managers/texture_manager.h"
#include "view/camera.h"
#include "scene/SceneNode.h"

struct RenderContext{
    Renderer2* renderer;
    
    SceneNode* root;
    Camera* camera;
    
    MaterialManager* materialManager;
    MeshManager* meshManager;
    PipelineManager* pipelineManager;
    TextureManager* textureManager;
    
    int curr_window_width, curr_window_height;
    int curr_fb_width, curr_fb_height;
};
