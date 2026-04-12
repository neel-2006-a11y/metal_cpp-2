//
//  render_pass.h
//  metal_cpp
//
//  Created by Neel on 23/03/26.
//

#pragma once

#include "view/renderer2.h"

class RenderPass
{
public:
    virtual ~RenderPass() = default;
    virtual void execute(Renderer2& renderer) = 0;
    virtual void renderNode(SceneNode* node, Renderer2& renderer, MTL::RenderCommandEncoder* encoder) {};
    virtual void release() = 0;
};
