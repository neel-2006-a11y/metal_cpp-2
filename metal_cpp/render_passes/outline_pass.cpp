//
//  outline_pass.cpp
//  metal_cpp
//
//  Created by Neel on 02/10/26.
//

#include "passers/MTL_Texture_passer.h"
#include "render_passes/outline_pass.h"

void OutlinePass::init(RenderDevice renderDevice){
    params.width = renderDevice.curr_fb_width;
    params.height = renderDevice.curr_fb_height;
    
    // make outline texture
    TextureDesc outline_Desc;
    outline_Desc.format = TextureFormat::R8Unorm;
    outline_Desc.usage = TextureUsage::Sampled | TextureUsage::ShaderWrite;
    outline_Desc.storageMode = StorageMode::Shared;
    outline_Desc.width = renderDevice.curr_fb_width;
    outline_Desc.height = renderDevice.curr_fb_height;
    outlineTextureID = renderDevice.textureManager->createEmptyNoCPU(outline_Desc);
    
    // make pipeline
    ComputePipelineDesc fill_desc;
    fill_desc.kernelFunc = "fill_outline";
    fill_pipelineID = renderDevice.pipelineManager->createComputePipeline(fill_desc);
    
    ComputePipelineDesc clear_desc;
    clear_desc.kernelFunc = "clear_outline";
    clear_pipelineID = renderDevice.pipelineManager->createComputePipeline(clear_desc);
}

void OutlinePass::resize(RenderContext renderContext){
    params.width = renderContext.curr_fb_width;
    params.height = renderContext.curr_fb_height;
    // delete outline texture
    Texture* tex = renderContext.textureManager->get(outlineTextureID);
    if(tex && tex->uploaded){
        ((MTL::Texture*)tex->gpuTexture)->release();
        tex->uploaded = false;
    }
    // make outline texture
    TextureDesc outline_Desc;
    outline_Desc.format = TextureFormat::R8Unorm;
    outline_Desc.usage = TextureUsage::Sampled | TextureUsage::ShaderWrite;
    outline_Desc.storageMode = StorageMode::Shared;
    outline_Desc.width = renderContext.curr_fb_width;
    outline_Desc.height = renderContext.curr_fb_height;
    outlineTextureID = renderContext.textureManager->createEmptyNoCPU(outline_Desc);
}

void OutlinePass::execute(RenderContext renderContext, RenderGraph *renderGraph){
    get_resources(renderGraph);
    
    params.depth_thres = renderContext.debugData->depth_thres;
    params.norm_thres = renderContext.debugData->normal_thres;
    params.ink_width = renderContext.debugData->ink_width;
    
    // Normal texture
    Texture* normTex = renderContext.textureManager->get(normalTextureID);
    
    // Depth texture
    Texture* depthTex = renderContext.textureManager->get(depthTextureID);
    
    // Outline attachment
    Texture* outlineTex = renderContext.textureManager->get(outlineTextureID);
    uploadTextureToGPU(*outlineTex, renderContext.device);
    
    auto clear_enc = renderContext.cmd->computeCommandEncoder();
    
    auto clear_pipe = renderContext.pipelineManager->getCompute(clear_pipelineID);
    clear_enc->setComputePipelineState(clear_pipe);
    
    clear_enc->setTexture((MTL::Texture*)outlineTex->gpuTexture, 0);
    
    MTL::Size gridSize = MTL::Size(params.width, params.height, 1);
    NS::UInteger w = clear_pipe->threadExecutionWidth();
    NS::UInteger h = clear_pipe->maxTotalThreadsPerThreadgroup() / w;
    MTL::Size tgSize = MTL::Size(w,h,1);
    clear_enc->dispatchThreads(gridSize, tgSize);
    clear_enc->endEncoding();
    
    auto fill_enc = renderContext.cmd->computeCommandEncoder();
    
    auto fill_pipe = renderContext.pipelineManager->getCompute(fill_pipelineID);
    fill_enc->setComputePipelineState(fill_pipe);
    
    if(normTex)
        fill_enc->setTexture((MTL::Texture*)normTex->gpuTexture, 0);
    if(depthTex)
        fill_enc->setTexture((MTL::Texture*)depthTex->gpuTexture, 1);
    fill_enc->setTexture((MTL::Texture*)outlineTex->gpuTexture, 2);
    
    fill_enc->setBytes(&params, sizeof(OutlineParams), 0);
    NS::UInteger w_2 = fill_pipe->threadExecutionWidth();
    NS::UInteger h_2 = fill_pipe->maxTotalThreadsPerThreadgroup() / w;
    MTL::Size tgSize_2 = MTL::Size(w_2,h_2,1);
    fill_enc->dispatchThreads(gridSize, tgSize_2);
    fill_enc->endEncoding();
    
    publish(renderGraph);
}


void OutlinePass::get_resources(RenderGraph *renderGraph){
    depthTextureID = renderGraph->resourceRegistry->getTexture(ResourceKey::SceneDepthTexture);
    
    normalTextureID = renderGraph->resourceRegistry->getTexture(ResourceKey::SceneNormalTexture);
}

void OutlinePass::publish(RenderGraph *renderGraph){
    renderGraph->resourceRegistry->publishTexture(ResourceKey::OutlineTexture, outlineTextureID);
}

