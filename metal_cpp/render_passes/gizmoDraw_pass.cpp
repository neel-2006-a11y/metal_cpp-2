//
//  gimzoDraw_pass.cpp
//  metal_cpp
//
//  Created by Neel on 06/06/26.
//

#include "render_passes/gizmoDraw_pass.h"


void GizmoDrawPass::init(){
    rpDesc = MTL::RenderPassDescriptor::alloc()->init();
    
    rpDesc->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionLoad);
    rpDesc->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
    
    rpDesc->depthAttachment()->setLoadAction(MTL::LoadActionClear);
    rpDesc->depthAttachment()->setStoreAction(MTL::StoreActionStore);
}

void GizmoDrawPass::execute(RenderContext renderContext){
    
}

