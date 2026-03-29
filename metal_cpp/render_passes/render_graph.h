//
//  render_pipeline.h
//  metal_cpp
//
//  Created by Neel on 23/03/26.
//

#pragma once

#include "view/renderer2.h"
#include "render_passes/render_pass.h"

class RenderGraph{
public:
    void addPass(RenderPass* pass);
    void execute(Renderer2& renderer);
    void release();
    
private:
    std::vector<RenderPass*> passes;
};
