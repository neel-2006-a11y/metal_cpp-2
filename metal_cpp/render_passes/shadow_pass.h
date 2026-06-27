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
    
    void init() override;
    void execute(RenderContext renderContext) override;
    void renderNode(SceneNode* node, RenderContext renderContext, MTL::RenderCommandEncoder* encoder);
    
private:
    MTL::RenderCommandEncoder* encoders[CASCADES] = {nullptr};
    MTL::RenderPassDescriptor* rpDescs[CASCADES] = {nullptr};
};
