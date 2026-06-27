//
//  ID_pass.cpp
//  metal_cpp
//
//  Created by Neel on 25/05/26.
//

#include "render_passes/ID_pass.h"

void IDPass::init(){
    rpDesc = MTL::RenderPassDescriptor::alloc()->init();
    
    rpDesc->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
    rpDesc->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
    rpDesc->colorAttachments()->object(0)->setClearColor({0,0,0,0});
    
    rpDesc->depthAttachment()->setLoadAction(MTL::LoadActionClear);
    rpDesc->depthAttachment()->setStoreAction(MTL::StoreActionStore);
    rpDesc->depthAttachment()->setClearDepth(1.0);
}

void IDPass::execute(RenderContext renderContext){
    
}
