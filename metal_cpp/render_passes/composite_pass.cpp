//
//  composite_pass.cpp
//  metal_cpp
//
//  Created by Neel on 04/04/26.
//

#include "render_passes/composite_pass.h"
#include "passers/MTL_Texture_passer.h"


void CompositePass::init(){
    rpDesc = MTL::RenderPassDescriptor::alloc()->init();
    
    rpDesc->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionLoad);
    rpDesc->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
}

void CompositePass::execute(RenderContext renderContext){
    
    rpDesc->colorAttachments()->object(0)->setTexture(drawableTexture);
    
    encoder = renderContext.renderer->cmd->renderCommandEncoder(rpDesc);
    
    MTL::RenderPipelineState* rp = renderContext.pipelineManager->get(pipeline);
    encoder->setRenderPipelineState(rp);
    
    Texture* tex = renderContext.textureManager->get(inputColor);
    Texture* volTex = renderContext.textureManager->get(volumeColor);
    Texture* densTex = renderContext.textureManager->get(densityTexture);
    if(tex)
        encoder->setFragmentTexture((MTL::Texture*)tex->gpuTexture, 0);
    if(volTex)
        encoder->setFragmentTexture((MTL::Texture*)volTex->gpuTexture, 1);
    if(densTex)
        encoder->setFragmentTexture((MTL::Texture*)densTex->gpuTexture, 2);
    
    encoder->setFragmentSamplerState(renderContext.renderer->defaultSampler, 0);
    
    auto primitiveType = MTL::PrimitiveTypeTriangle;
    encoder->drawPrimitives(primitiveType, (NS::UInteger)0, (NS::UInteger)3);
    
    encoder->endEncoding();
}

