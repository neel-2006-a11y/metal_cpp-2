//
//  main_pass.h
//  metal_cpp
//
//  Created by Neel on 23/03/26.
//

#pragma once

#include "view/renderer2.h"
#include "render_passes/render_pass.h"

class MainPass : public RenderPass
{
public:
    TextureID depthTexture;
    TextureID sceneColorTexture;
    
    void init() override;
    void execute(RenderContext renderContext) override;
    void renderNode(SceneNode* node, RenderContext renderContext, MTL::RenderCommandEncoder* encoder);
private:
    MTL::RenderCommandEncoder* encoder = nullptr;
    MTL::RenderPassDescriptor* rpDesc = nullptr;
};
