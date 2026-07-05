//
//  render_pipeline.h
//  metal_cpp
//
//  Created by Neel on 23/03/26.
//

#pragma once

#include "view/renderContext.h"
#include "view/ResourceRegistry.h"
#include "view/FrameDataRegistry.h"

class RenderPass;
class RenderGraph{
public:
    RenderGraph(ResourceRegistry* resourceRegistry, FrameDataRegistry* frameDataRegistry);
    void addPass(RenderPass* pass);
    void init(RenderDevice renderDevice);
    void execute(RenderContext renderContext, bool resize);
    void resize(RenderContext renderContext);
    
    ResourceRegistry* resourceRegistry;
    FrameDataRegistry* frameDataRegistry;    
private:
    std::vector<RenderPass*> passes;
};
