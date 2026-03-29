//
//  render_pipeline.cpp
//  metal_cpp
//
//  Created by Neel on 23/03/26.
//

#include "render_passes/render_graph.h"

void RenderGraph::addPass(RenderPass *pass){
    passes.push_back(pass);
}

void RenderGraph::execute(Renderer2 &renderer){
    for(auto* pass : passes){
        pass->execute(renderer);
    }
}

void RenderGraph::release(){
    for(auto* pass : passes){
        pass->release();
    }
}
