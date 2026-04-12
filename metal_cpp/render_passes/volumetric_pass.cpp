//
//  volumetric_pass.cpp
//  metal_cpp
//
//  Created by Neel on 04/04/26.
//

#include "render_passes/volumetric_pass.h"
#include "passers/MTL_Texture_passer.h"

void VolumetricPass::execute(Renderer2 &r){
    // ------------
    // Textures
    // ------------
    Texture* depthTex = r.textureManager->get(depthTexture);
    Texture* volumetricTex = r.textureManager->get(volumetricColorTexture);
    Texture* blueNoiseTex = r.textureManager->get(blueNoiseTexture);
    uploadTextureToGPU(*volumetricTex, r.device);
    uploadTextureToGPU(*blueNoiseTex, r.device);
    Texture* shadowMapTex = r.textureManager->get(shadowMap);
    
    
    MTL::RenderPassDescriptor* rpDesc = MTL::RenderPassDescriptor::alloc()->init();
    rpDesc->colorAttachments()->object(0)->setTexture((MTL::Texture*)volumetricTex->gpuTexture);
    rpDesc->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionLoad);
    rpDesc->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
    
    encoder = r.cmd->renderCommandEncoder(rpDesc);
    
    MTL::RenderPipelineState* rp = r.pipelineManger->get(volumetricPipeID);
    encoder->setRenderPipelineState(rp);
    
    encoder->setFragmentTexture((MTL::Texture*)depthTex->gpuTexture, 0);
    encoder->setFragmentTexture((MTL::Texture*)shadowMapTex->gpuTexture, 1);
    encoder->setFragmentTexture((MTL::Texture*)blueNoiseTex->gpuTexture, 2);
    encoder->setFragmentSamplerState(sampler, 0);
    encoder->setFragmentBuffer(r.frameBuffer[r.frameIndex], 0, 1);
    
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, (NS::UInteger)0, (NS::UInteger)3);
    
    encoder->endEncoding();
    rpDesc->release();
}

void VolumetricPass::release(){
    
}
