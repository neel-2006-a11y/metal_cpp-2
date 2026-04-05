//
//  composite_pass.cpp
//  metal_cpp
//
//  Created by Neel on 04/04/26.
//

#include "render_passes/composite_pass.h"
#include "passers/MTL_Texture_passer.h"


void CompositePass::execute(Renderer2 &renderer){
    MTL::RenderPassDescriptor* rpDesc = MTL::RenderPassDescriptor::alloc()->init();
    
    rpDesc->colorAttachments()->object(0)->setTexture(drawableTexture);
    rpDesc->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionLoad);
    rpDesc->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
    
    encoder = renderer.cmd->renderCommandEncoder(rpDesc);
    
    MTL::RenderPipelineState* rp = renderer.pipelineManger->get(pipeline);
    encoder->setRenderPipelineState(rp);
    
    Texture* tex = renderer.textureManager->get(inputColor);
    Texture* volTex = renderer.textureManager->get(volumeColor);
    encoder->setFragmentTexture((MTL::Texture*)tex->gpuTexture, 0);
    encoder->setFragmentTexture((MTL::Texture*)volTex->gpuTexture, 1);
    
    encoder->setFragmentSamplerState(renderer.defaultSampler, 0);
    
    auto primitiveType = MTL::PrimitiveTypeTriangle;
    encoder->drawPrimitives(primitiveType, (NS::UInteger)0, (NS::UInteger)3);
    
    encoder->endEncoding();
    rpDesc->release();
}


void CompositePass::release(){
    if(encoder){
        encoder->release();
        encoder = nullptr;
    }
}
