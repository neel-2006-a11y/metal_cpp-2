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
    MTL::Texture* drawableTexture;
    PipelineID pipeline;
    
    void execute(Renderer2& renderer) override;
    void release() override;
    
private:
    MTL::RenderCommandEncoder* encoder;
};
