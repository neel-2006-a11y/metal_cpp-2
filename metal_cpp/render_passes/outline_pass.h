//
//  outline_pass.h
//  metal_cpp
//
//  Created by Neel on 02/10/26.
//

#pragma once

#include "render_passes/render_pass.h"

struct OutlineParams{
    int width, height;
    float depth_thres, norm_thres;
    int ink_width;
};

class OutlinePass: public RenderPass
{
public:
    // owned textures
    TextureID outlineTextureID;
    
    // input textures
    TextureID normalTextureID;
    TextureID depthTextureID;
    
    // pipeline
    ComputePipelineID clear_pipelineID;
    ComputePipelineID fill_pipelineID;
    
    void init(RenderDevice renderDevice) override;
    void execute(RenderContext renderContext, RenderGraph* renderGraph) override;
    void resize(RenderContext renderContext) override;
    
private:
//    SamplerID sampler;
//    MTL::ComputeCommandEncoder* encoder = nullptr;
//    MTL::RenderPassDescriptor* rpDesc = nullptr;
    
    OutlineParams params;
    
    void get_resources(RenderGraph* renderGraph);
    void publish(RenderGraph* renderGraph);
};
