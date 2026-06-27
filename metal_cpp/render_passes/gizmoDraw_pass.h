//
//  gimzoDraw_pass.h
//  metal_cpp
//
//  Created by Neel on 06/06/26.
//

#pragma once
#include "config.h"
#include "render_passes/render_pass.h"

class GizmoDrawPass : RenderPass{
public:
    void init() override;
    void execute(RenderContext renderContext) override;
    
private:
    MTL::RenderPassDescriptor* rpDesc = nullptr;
};
