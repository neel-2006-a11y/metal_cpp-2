//
//  shadow_pass.h
//  metal_cpp
//
//  Created by Neel on 23/03/26.
//

#pragma once

#include "render_passes/render_pass.h"
#include "resource_managers/resource_types.h"

class ShadowPass : public RenderPass
{
public:
    // owned / output
    TextureID shadowMapArray;
    
    // pipeline
    PipelineID shadowPipeline;
    
    int cascades = CASCADES;
    
    void init(RenderDevice renderDevice) override;
    void execute(RenderContext renderContext, RenderGraph* renderGraph) override;
    void resize(RenderContext renderContext) override;
    
private:
    
    int shadow_res = 512;
    MTL::RenderCommandEncoder* encoders[CASCADES] = {nullptr};
    MTL::RenderPassDescriptor* rpDescs[CASCADES] = {nullptr};
    
    DepthStencilStateID depthState = 0;
    
    void publish(RenderGraph* renderGraph);
};
