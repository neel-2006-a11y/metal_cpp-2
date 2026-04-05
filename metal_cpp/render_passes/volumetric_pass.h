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
    TextureID depthTexture;
    TextureID shadowMap;
    TextureID volumetricColorTexture;
    
    MTL::SamplerState* sampler;
    
    PipelineID volumetricPipeID;
    
    void execute(Renderer2& r) override;
    void release() override;
private:
    MTL::RenderCommandEncoder* encoder;
};
