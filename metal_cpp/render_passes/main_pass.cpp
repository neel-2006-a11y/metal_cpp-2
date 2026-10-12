//
//  main_pass.cpp
//  metal_cpp
//
//  Created by Neel on 23/03/26.
//

#include "render_passes/main_pass.h"
#include "passers/MTL_Texture_passer.h"
#include "passers/MTL_Mesh_passer.h"
#include "passers/MTL_material_passer.h"

void MainPass::init(RenderDevice renderDevice){
    // make depth and color textures
    TextureDesc depth_Desc;
    depth_Desc.format = TextureFormat::Depth32Float;
    depth_Desc.usage = TextureUsage::DepthStencil | TextureUsage::Sampled;
    depth_Desc.storageMode = StorageMode::Private;
    depth_Desc.width = renderDevice.curr_fb_width;
    depth_Desc.height = renderDevice.curr_fb_height;
    depthTexture = renderDevice.textureManager->createEmptyNoCPU(depth_Desc);
    
    TextureDesc col_Desc;
    col_Desc.format = TextureFormat::RGBA8Unorm;
    col_Desc.usage = TextureUsage::RenderTarget | TextureUsage::Sampled;
    col_Desc.storageMode = StorageMode::Shared;
    col_Desc.width = renderDevice.curr_fb_width;
    col_Desc.height = renderDevice.curr_fb_height;
    sceneColorTexture = renderDevice.textureManager->createEmptyNoCPU(col_Desc);
    
    TextureDesc norm_Desc;
    norm_Desc.format = TextureFormat::RGBA16Float;
    norm_Desc.usage = TextureUsage::RenderTarget | TextureUsage::Sampled;
    norm_Desc.storageMode = StorageMode::Shared;
    norm_Desc.width = renderDevice.curr_fb_width;
    norm_Desc.height = renderDevice.curr_fb_height;
    normalTexture = renderDevice.textureManager->createEmptyNoCPU(norm_Desc);
    
    
    // render pass descriptor
    rpDesc = MTL::RenderPassDescriptor::alloc()->init();
    
    rpDesc->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
    rpDesc->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
//    rpDesc->colorAttachments()->object(0)->setClearColor({0.0, 0.0, 1.0, 1.0});
    rpDesc->colorAttachments()->object(0)->setClearColor({1.0, 0.0, 0.0, 1.0});
    
    rpDesc->colorAttachments()->object(1)->setLoadAction(MTL::LoadActionClear);
    rpDesc->colorAttachments()->object(1)->setStoreAction(MTL::StoreActionStore);
    rpDesc->colorAttachments()->object(1)->setClearColor({0,0,0,1.0});
    
    rpDesc->depthAttachment()->setLoadAction(MTL::LoadActionClear);
    rpDesc->depthAttachment()->setStoreAction(MTL::StoreActionStore);
    rpDesc->depthAttachment()->setClearDepth(1.0);
    
    // depth state
    DepthStencilDesc desc;
    desc.compare = MTL::CompareFunctionLess;
    desc.depthWrite = true;
    
    depthState = renderDevice.depthStencilStateManager->createDepthState(desc);
    
    // sampler
    SamplerDesc samp_desc;
    sampler = renderDevice.samplerManager->createSampler(samp_desc);
}

void MainPass::execute(RenderContext renderContext, RenderGraph* renderGraph){
    
    get_resources(renderGraph);
    
    // Color attachment
    Texture* sceneColor = renderContext.textureManager->get(sceneColorTexture);
    uploadTextureToGPU(*sceneColor, renderContext.device);
    rpDesc->colorAttachments()->object(0)->setTexture((MTL::Texture*)sceneColor->gpuTexture);

    // Normal attachment
    Texture* normTex = renderContext.textureManager->get(normalTexture);
    uploadTextureToGPU(*normTex, renderContext.device);
    rpDesc->colorAttachments()->object(1)->setTexture((MTL::Texture*)normTex->gpuTexture);
    
    // Depth attachment
    Texture* depth = renderContext.textureManager->get(depthTexture);
    uploadTextureToGPU(*depth, renderContext.device);
    rpDesc->depthAttachment()->setTexture((MTL::Texture*)depth->gpuTexture);
    
    
    encoder = renderContext.cmd->renderCommandEncoder(rpDesc);
    
    // depth state
    encoder->setDepthStencilState(renderContext.depthStencilStateManager->getState(depthState)->gpu_depthState);
    
    // FrameBuffer
    encoder->setVertexBuffer(renderContext.frameBuffer, 0, 1);
    encoder->setFragmentBuffer(renderContext.frameBuffer, 0, 1);
    
    // viewBuffer
    encoder->setVertexBytes(&renderContext.viewUniforms, sizeof(ViewUniforms), 4);
    encoder->setFragmentBytes(&renderContext.viewUniforms, sizeof(ViewUniforms), 4);
    
//    MTL::Viewport viewPort;
//    viewPort.originX = 0;
//    viewPort.originY = 0;
//    viewPort.width = renderContext.curr_fb_width;
//    viewPort.height = renderContext.curr_fb_height;
//    viewPort.znear = 0.0;
//    viewPort.zfar = 1.0;
//    encoder->setViewport(viewPort);
    
    // Draw everything
    for(auto ro : renderContext.renderQ.renderObjects){
        auto* objBuffer = renderContext.objectBufferData.objectBuffer;
        Mesh2* mesh = renderContext.meshManager->getMesh(ro.mesh);
        Material* mat = renderContext.materialManager->get(ro.material);
        
        uploadMeshToGPU(*mesh, renderContext.device);
        
        encoder->setRenderPipelineState(renderContext.pipelineManager->get(mat->pipeID));
        
        size_t offset = ro.object_index * renderContext.objectBufferData.objectStride;
        
        encoder->setVertexBuffer(objBuffer, offset, 2);
        encoder->setFragmentBuffer(objBuffer, offset, 2);
        
        bindMaterial(*mat, encoder, renderContext);
        
        // temporary fix
        Texture* shadowMap = renderContext.textureManager->get(shadowMapTexture);
        if(shadowMap)
            encoder->setFragmentTexture((MTL::Texture*)shadowMap->gpuTexture, 0);
        encoder->setFragmentSamplerState(renderContext.samplerManager->getState(sampler)->state, 0);
        
        encoder->setVertexBuffer((MTL::Buffer*)mesh->vertexBuffer, 0, 0);
        
        
        encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, mesh->indexCount, INDEX_FORMAT, (MTL::Buffer*)mesh->indexBuffer, 0, 1);
    }
    
    encoder->endEncoding();
    
    publish(renderGraph);
}

void MainPass::get_resources(RenderGraph *renderGraph){
    shadowMapTexture = renderGraph->resourceRegistry->getTexture(ResourceKey::ShadowMapTexture);
}

void MainPass::publish(RenderGraph* renderGraph){
    renderGraph->resourceRegistry->publishTexture(ResourceKey::SceneColorTexture, sceneColorTexture);
    renderGraph->resourceRegistry->publishTexture(ResourceKey::SceneDepthTexture, depthTexture);
    renderGraph->resourceRegistry->publishTexture(ResourceKey::SceneNormalTexture, normalTexture);
}

void MainPass::resize(RenderContext renderContext){
    //--------------
    // Recreate depth Texture
    //--------------
    // delete Depth texture
    Texture* tex = renderContext.textureManager->get(depthTexture);
    if(tex && tex->uploaded){
        ((MTL::Texture*)tex->gpuTexture)->release();
        tex->uploaded = false;
    }
    // create Depth Texture
    TextureDesc depth_Desc;
    depth_Desc.format = TextureFormat::Depth32Float;
    depth_Desc.usage = TextureUsage::DepthStencil | TextureUsage::Sampled;
    depth_Desc.storageMode = StorageMode::Private;
    depth_Desc.width = renderContext.curr_fb_width;
    depth_Desc.height = renderContext.curr_fb_height;
    depthTexture = renderContext.textureManager->createEmptyNoCPU(depth_Desc);
    
    //--------------
    // Recreate color Texture
    //--------------
    // delete
    Texture* colTex = renderContext.textureManager->get(sceneColorTexture);
    if(colTex && colTex->uploaded){
        ((MTL::Texture*)colTex->gpuTexture)->release();
        colTex->uploaded = false;
    }
    // create
    TextureDesc col_Desc;
    col_Desc.format = TextureFormat::RGBA8Unorm;
    col_Desc.usage = TextureUsage::RenderTarget | TextureUsage::Sampled;
    col_Desc.storageMode = StorageMode::Shared;
    col_Desc.width = renderContext.curr_fb_width;
    col_Desc.height = renderContext.curr_fb_height;
    sceneColorTexture = renderContext.textureManager->createEmptyNoCPU(col_Desc);
    
    //--------------
    // Recreate normal Texture
    //--------------
    // delete
    Texture* normTex = renderContext.textureManager->get(normalTexture);
    if(normTex && normTex->uploaded){
        ((MTL::Texture*)normTex->gpuTexture)->release();
        normTex->uploaded = false;
    }
    // create
    TextureDesc norm_Desc;
    norm_Desc.format = TextureFormat::RGBA16Float;
    norm_Desc.usage = TextureUsage::RenderTarget | TextureUsage::Sampled;
    norm_Desc.storageMode = StorageMode::Shared;
    norm_Desc.width = renderContext.curr_fb_width;
    norm_Desc.height = renderContext.curr_fb_height;
    normalTexture = renderContext.textureManager->createEmptyNoCPU(norm_Desc);
}

