//
//  volumetric_pass.cpp
//  metal_cpp
//
//  Created by Neel on 04/04/26.
//

#include "render_passes/volumetric_pass.h"
#include "passers/MTL_Texture_passer.h"

void VolumetricPass::init(){
    rpDesc = MTL::RenderPassDescriptor::alloc()->init();
    rpDesc->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionLoad);
    rpDesc->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
}

void VolumetricPass::execute(RenderContext renderContext){
    // ------------
    // Textures
    // ------------
    Texture* depthTex = renderContext.textureManager->get(depthTexture);
    Texture* volumetricTex = renderContext.textureManager->get(volumetricColorTexture);
    Texture* blueNoiseTex = renderContext.textureManager->get(blueNoiseTexture);
    uploadTextureToGPU(*volumetricTex, renderContext.renderer->device);
    uploadTextureToGPU(*blueNoiseTex, renderContext.renderer->device);
    Texture* shadowMapTex = renderContext.textureManager->get(shadowMap);
    
    rpDesc->colorAttachments()->object(0)->setTexture((MTL::Texture*)volumetricTex->gpuTexture);
    
    encoder = renderContext.renderer->cmd->renderCommandEncoder(rpDesc);
    
    MTL::RenderPipelineState* rp = renderContext.pipelineManager->get(volumetricPipeID);
    encoder->setRenderPipelineState(rp);
    
    encoder->setFragmentTexture((MTL::Texture*)depthTex->gpuTexture, 0);
    encoder->setFragmentTexture((MTL::Texture*)shadowMapTex->gpuTexture, 1);
    encoder->setFragmentTexture((MTL::Texture*)blueNoiseTex->gpuTexture, 2);
    encoder->setFragmentSamplerState(sampler, 0);
    encoder->setFragmentBuffer(renderContext.renderer->frameBuffer[renderContext.renderer->frameIndex], 0, 1);
    
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, (NS::UInteger)0, (NS::UInteger)3);
    
    encoder->endEncoding();
}
