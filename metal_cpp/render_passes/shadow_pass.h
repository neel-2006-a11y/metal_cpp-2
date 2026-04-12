//
//  shadow_pass.h
//  metal_cpp
//
//  Created by Neel on 23/03/26.
//

#pragma once

#include "render_passes/render_pass.h"
#include "resource_managers/resource_types.h"
#include "view/renderer2.h"

class ShadowPass : public RenderPass
{
public:
    TextureID shadowMapArray;
    PipelineID shadowPipeline;
    
    int cascades = CASCADES;
    
    void execute(Renderer2& renderer) override;
    void renderNode(SceneNode* node, Renderer2& renderer, MTL::RenderCommandEncoder* encoder) override;
    void release() override;
    
private:
    MTL::RenderCommandEncoder* encoders[CASCADES] = {nullptr};
};
