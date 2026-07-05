//
//  composite_pass.h
//  metal_cpp
//
//  Created by Neel on 04/04/26.
//

#pragma once
#include "render_passes/render_pass.h"

class CompositePass : public RenderPass{
public:
    // input
    TextureID inputColor; // output
    TextureID volumeColor;
    TextureID densityTexture;
    
    // pipeline
    PipelineID pipeline;
    
    void init(RenderDevice renderDevice) override;
    void execute(RenderContext renderContext, RenderGraph* renderGraph) override;
    void resize(RenderContext renderContext) override;
    
private:
    SamplerID sampler;
    MTL::RenderCommandEncoder* encoder = nullptr;
    MTL::RenderPassDescriptor* rpDesc = nullptr;
    
    void get_resources(RenderGraph* renderGraph);
    void publish(RenderGraph* renderGraph);
};
