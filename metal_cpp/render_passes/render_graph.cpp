//
//  render_pipeline.cpp
//  metal_cpp
//
//  Created by Neel on 23/03/26.
//

#include "render_passes/render_graph.h"
#include "render_passes/render_pass.h"

RenderGraph::RenderGraph(ResourceRegistry* rR, FrameDataRegistry* fR){
    resourceRegistry = rR;
    frameDataRegistry = fR;
}

void RenderGraph::addPass(RenderPass *pass){
    passes.push_back(pass);
}

void RenderGraph::init(RenderDevice renderDevice){
    for(auto* pass : passes){
        pass->init(renderDevice);
    }
}

void RenderGraph::execute(RenderContext renderContext, bool resize = false){
    for(auto* pass : passes){
        if(resize){
            pass->resize(renderContext);
        }
        pass->execute(renderContext, this);
    }
}
