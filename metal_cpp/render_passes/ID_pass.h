//
//  ID_pass.h
//  metal_cpp
//
//  Created by Neel on 25/05/26.
//

#pragma once

#include "render_passes/render_pass.h"
#include "config.h"

class IDPass : public RenderPass {
public:
    TextureID ID_texture;
    
    void init() override;
    void execute(RenderContext renderContext) override;
    
private:
    MTL::RenderPassDescriptor* rpDesc = nullptr;
};
