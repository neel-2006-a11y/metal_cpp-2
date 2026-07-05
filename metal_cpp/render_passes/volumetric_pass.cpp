//
//  volumetric_pass.cpp
//  metal_cpp
//
//  Created by Neel on 04/04/26.
//

#include "render_passes/volumetric_pass.h"
#include "passers/MTL_Texture_passer.h"

void VolumetricPass::init(RenderDevice renderDevice){
    // define pipeline
    PipelineDesc volDesc;
    volDesc.vertexFunc = "fullScreenVS";
    volDesc.fragmentFunc = "volumetricFS";
    volDesc.colorFormat = TextureFormat::RGBA8Unorm;
    
    volumetricPipeID = renderDevice.pipelineManager->createPipeline(volDesc);
    
    //--------------
    // Load Blue Noise
    //--------------
    std::string blueNoiseFile = "/Users/admin/Desktop/metal_cpp-2/metal_cpp/assets/BlueNoise470.png";
    blueNoiseTexture = renderDevice.textureManager->loadFromFile(blueNoiseFile);
    
    //--------------
    // create volumetric texture
    //--------------
    TextureDesc vol_Desc;
    vol_Desc.format = TextureFormat::RGBA8Unorm;
    vol_Desc.usage = TextureUsage::RenderTarget | TextureUsage::Sampled;
    vol_Desc.storageMode = StorageMode::Shared;
    vol_Desc.width = renderDevice.curr_fb_width / volumeDownFact;
    vol_Desc.height = renderDevice.curr_fb_height / volumeDownFact;
    volumetricColorTexture = renderDevice.textureManager->createEmptyNoCPU(vol_Desc);
    
    
    // render pass
    rpDesc = MTL::RenderPassDescriptor::alloc()->init();
    rpDesc->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionLoad);
    rpDesc->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
    
    // sampler
    SamplerDesc samp_desc;
    sampler = renderDevice.samplerManager->createSampler(samp_desc);
}

void VolumetricPass::execute(RenderContext renderContext, RenderGraph* renderGraph){
    get_resources(renderGraph);
    // ------------
    // Textures
    // ------------
    Texture* depthTex = renderContext.textureManager->get(depthTexture);
    Texture* volumetricTex = renderContext.textureManager->get(volumetricColorTexture);
    Texture* blueNoiseTex = renderContext.textureManager->get(blueNoiseTexture);
    uploadTextureToGPU(*volumetricTex, renderContext.device);
    uploadTextureToGPU(*blueNoiseTex, renderContext.device);
    Texture* shadowMapTex = renderContext.textureManager->get(shadowMap);
    
    rpDesc->colorAttachments()->object(0)->setTexture((MTL::Texture*)volumetricTex->gpuTexture);
    
    encoder = renderContext.cmd->renderCommandEncoder(rpDesc);
    
    MTL::RenderPipelineState* rp = renderContext.pipelineManager->get(volumetricPipeID);
    encoder->setRenderPipelineState(rp);
    
    encoder->setFragmentTexture((MTL::Texture*)depthTex->gpuTexture, 0);
    encoder->setFragmentTexture((MTL::Texture*)shadowMapTex->gpuTexture, 1);
    encoder->setFragmentTexture((MTL::Texture*)blueNoiseTex->gpuTexture, 2);
    encoder->setFragmentSamplerState(renderContext.samplerManager->getState(sampler)->state, 0);
    encoder->setFragmentBuffer(renderContext.frameBuffer, 0, 1);
    
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, (NS::UInteger)0, (NS::UInteger)3);
    
    encoder->endEncoding();
    publish(renderGraph);
}

void VolumetricPass::get_resources(RenderGraph *renderGraph){
    depthTexture = renderGraph->resourceRegistry->getTexture(ResourceKey::SceneDepthTexture);
    shadowMap = renderGraph->resourceRegistry->getTexture(ResourceKey::ShadowMapTexture);
}

void VolumetricPass::publish(RenderGraph* renderGraph){
    renderGraph->resourceRegistry->publishTexture(ResourceKey::VolumetricTexture, volumetricColorTexture);
}

void VolumetricPass::resize(RenderContext renderContext){
    //--------------
    // Recreate volume Texture
    //--------------
    // delete
    Texture* volTex = renderContext.textureManager->get(volumetricColorTexture);
    if(volTex && volTex->uploaded){
        ((MTL::Texture*)volTex->gpuTexture)->release();
        volTex->uploaded = false;
    }
    // create
    TextureDesc vol_Desc;
    vol_Desc.format = TextureFormat::RGBA8Unorm;
    vol_Desc.usage = TextureUsage::RenderTarget | TextureUsage::Sampled;
    vol_Desc.storageMode = StorageMode::Shared;
    vol_Desc.width = renderContext.curr_fb_width / volumeDownFact;
    vol_Desc.height = renderContext.curr_fb_height / volumeDownFact;
    volumetricColorTexture = renderContext.textureManager->createEmptyNoCPU(vol_Desc);
}
