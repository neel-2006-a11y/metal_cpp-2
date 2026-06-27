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
    TextureID inputColor;
    TextureID volumeColor;
    TextureID densityTexture;
    MTL::Texture* drawableTexture;
    PipelineID pipeline;
    
    void init() override;
    void execute(RenderContext renderContext) override;
    
private:
    MTL::RenderCommandEncoder* encoder = nullptr;
    MTL::RenderPassDescriptor* rpDesc = nullptr;
};
