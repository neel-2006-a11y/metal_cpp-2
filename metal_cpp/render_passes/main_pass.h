//
//  main_pass.h
//  metal_cpp
//
//  Created by Neel on 23/03/26.
//

#pragma once

#include "view/renderer2.h"
#include "render_passes/render_pass.h"

class MainPass : public RenderPass
{
public:
    TextureID depthTexture;
    MTL::Texture* drawableTexture; // swapchain
    
    void execute(Renderer2& r) override;
    void release() override;
private:
    MTL::RenderCommandEncoder* encoder = nullptr;
};
