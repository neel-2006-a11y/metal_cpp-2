//
//  composite_pass.cpp
//  metal_cpp
//
//  Created by Neel on 04/04/26.
//

#include "render_passes/composite_pass.h"
#include "passers/MTL_Texture_passer.h"


void CompositePass::init(RenderDevice renderDevice){
    // define pipeline
    PipelineDesc compDesc;
    compDesc.vertexFunc = "compositeVS";
    compDesc.fragmentFunc = "compositeFS";
    compDesc.colorFormat = TextureFormat::RGBA8Unorm;
    
    pipeline = renderDevice.pipelineManager->createPipeline(compDesc);
    
    // render pass descriptor
    rpDesc = MTL::RenderPassDescriptor::alloc()->init();
    
    rpDesc->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionLoad);
    rpDesc->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
    
    // sampler
    SamplerDesc samp_desc;
    sampler = renderDevice.samplerManager->createSampler(samp_desc);
}

void CompositePass::execute(RenderContext renderContext, RenderGraph* renderGraph){
    get_resources(renderGraph);
    
    rpDesc->colorAttachments()->object(0)->setTexture(renderContext.backBuffer);
    
    encoder = renderContext.cmd->renderCommandEncoder(rpDesc);
    
    MTL::RenderPipelineState* rp = renderContext.pipelineManager->get(pipeline);
    encoder->setRenderPipelineState(rp);
    
    Texture* tex = renderContext.textureManager->get(inputColor);
    Texture* volTex = renderContext.textureManager->get(volumeColor);
    Texture* densTex = renderContext.textureManager->get(densityTexture);
    Texture* outlineTex = renderContext.textureManager->get(outlineTexture);
    if(tex)
        encoder->setFragmentTexture((MTL::Texture*)tex->gpuTexture, 0);
    if(volTex)
        encoder->setFragmentTexture((MTL::Texture*)volTex->gpuTexture, 1);
    if(densTex)
        encoder->setFragmentTexture((MTL::Texture*)densTex->gpuTexture, 2);
    if(outlineTex)
        encoder->setFragmentTexture((MTL::Texture*)outlineTex->gpuTexture, 3);
    
    encoder->setFragmentSamplerState(renderContext.samplerManager->getState(sampler)->state, 0);
    
    auto primitiveType = MTL::PrimitiveTypeTriangle;
    encoder->drawPrimitives(primitiveType, (NS::UInteger)0, (NS::UInteger)3);
    
    encoder->endEncoding();
}

void CompositePass::get_resources(RenderGraph* renderGraph) {
    inputColor = renderGraph->resourceRegistry->getTexture(ResourceKey::SceneColorTexture);
    volumeColor = renderGraph->resourceRegistry->getTexture(ResourceKey::VolumetricTexture);
    densityTexture = renderGraph->resourceRegistry->getTexture(ResourceKey::DensityTexture);
    outlineTexture = renderGraph->resourceRegistry->getTexture(ResourceKey::OutlineTexture);
}

void CompositePass::resize(RenderContext renderContext){}
