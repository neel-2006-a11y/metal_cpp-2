//
//  main_pass.h
//  metal_cpp
//
//  Created by Neel on 23/03/26.
//

#pragma once

#include "render_passes/render_pass.h"


class MainPass : public RenderPass
{
public:
    // owned / output
    TextureID depthTexture;
    TextureID sceneColorTexture;
    
    // input
    TextureID shadowMapTexture;
    
    void init(RenderDevice renderDevice) override;
    void execute(RenderContext renderContext, RenderGraph* renderGraph) override;
    void resize(RenderContext renderContext) override;
    
private:
    MTL::RenderCommandEncoder* encoder = nullptr;
    MTL::RenderPassDescriptor* rpDesc = nullptr;
    DepthStencilStateID depthState = 0;
    SamplerID sampler = 0;
    
    void get_resources(RenderGraph* renderGraph);
    void publish(RenderGraph* renderGraph);
};
