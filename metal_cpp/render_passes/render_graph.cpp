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

void RenderGraph::init(){
    for(auto* pass : passes){
        pass->init();
    }
}
void RenderGraph::execute(RenderContext renderContext){
    for(auto* pass : passes){
        pass->execute(renderContext);
    }
}
