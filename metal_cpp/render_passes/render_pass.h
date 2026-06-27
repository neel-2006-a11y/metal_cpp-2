//
//  render_pass.h
//  metal_cpp
//
//  Created by Neel on 23/03/26.
//

#pragma once

#include "view/renderer2.h"
#include "view/renderContext.h"

class RenderPass
{
public:
    virtual ~RenderPass() = default;
    virtual void init() = 0;
    virtual void execute(RenderContext renderContext) = 0;
    virtual void renderNode(SceneNode* node, Renderer2& renderer, MTL::RenderCommandEncoder* encoder) {};
};
