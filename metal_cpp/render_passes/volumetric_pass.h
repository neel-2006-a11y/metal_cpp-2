//
//  volumetric_pass.h
//  metal_cpp
//
//  Created by Neel on 04/04/26.
//

#pragma once
#include "render_passes/render_pass.h"

class VolumetricPass : public RenderPass{
public:
    // owned / output
    TextureID blueNoiseTexture;
    TextureID volumetricColorTexture;
    
    // input
    TextureID depthTexture;
    TextureID shadowMap;
    
    PipelineID volumetricPipeID;
    
    void init(RenderDevice renderDevice) override;
    void execute(RenderContext renderContext, RenderGraph* renderGraph) override;
    void resize(RenderContext renderContext) override;
    
private:
    SamplerID sampler = 0;
    MTL::RenderCommandEncoder* encoder = nullptr;
    MTL::RenderPassDescriptor* rpDesc = nullptr;
    int volumeDownFact = 1;
    
    void publish(RenderGraph* renderGraph);
    void get_resources(RenderGraph* renderGraph);
};
