//
//  render_pass.h
//  metal_cpp
//
//  Created by Neel on 23/03/26.
//

#pragma once

#include "view/renderContext.h"
#include "render_passes/render_graph.h"

class RenderPass
{
public:
    virtual ~RenderPass() = default;
    virtual void init(RenderDevice renderDevice) = 0;
    virtual void execute(RenderContext renderContext, RenderGraph* renderGraph) = 0;
    virtual void resize(RenderContext renderContext) = 0;
};
