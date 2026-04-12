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

void MainPass::execute(Renderer2 &r){
    auto* rp = MTL::RenderPassDescriptor::alloc()->init();
    
    
    Texture* sceneColor = r.textureManager->get(sceneColorTexture);
    uploadTextureToGPU(*sceneColor, r.device);
    // Color attachment
    rp->colorAttachments()->object(0)->setTexture((MTL::Texture*)sceneColor->gpuTexture);
    rp->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
    rp->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
    rp->colorAttachments()->object(0)->setClearColor({0.1, 0.1, 0.15, 1.0});
    
    // Depth attachment
    Texture* depth = r.textureManager->get(depthTexture);
    uploadTextureToGPU(*depth, r.device);
    
    rp->depthAttachment()->setTexture((MTL::Texture*)depth->gpuTexture);
    rp->depthAttachment()->setLoadAction(MTL::LoadActionClear);
    rp->depthAttachment()->setStoreAction(MTL::StoreActionStore);
    rp->depthAttachment()->setClearDepth(1.0);
    
    encoder = r.cmd->renderCommandEncoder(rp)->retain();
    
    // depth state
    MTL::DepthStencilDescriptor* depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
    depthDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
    depthDesc->setDepthWriteEnabled(true);
    MTL::DepthStencilState* depthState = r.device->newDepthStencilState(depthDesc);
    encoder->setDepthStencilState(depthState);
    
    // FrameBuffer
    encoder->setVertexBuffer(r.frameBuffer[r.frameIndex], 0, 1);
    encoder->setFragmentBuffer(r.frameBuffer[r.frameIndex], 0, 1);
    
    MTL::Viewport viewPort;
    viewPort.originX = 0;
    viewPort.originY = 0;
    viewPort.width = r.curr_width;
    viewPort.height = r.curr_height;
    viewPort.znear = 0.0;
    viewPort.zfar = 1.0;
    encoder->setViewport(viewPort);
    
    // Draw everything
    renderNode(r.root, r, encoder);
    
    encoder->endEncoding();
    rp->release();
    depthDesc->release();
    depthState->release();
}

void MainPass::renderNode(SceneNode *node, Renderer2 &renderer, MTL::RenderCommandEncoder *encoder){
    if(node->renderObject){
        auto* objBuffer = renderer.objectBuffer[renderer.frameIndex];
        Mesh2* mesh = renderer.meshManager->getMesh(node->renderObject->meshID);
        Material* mat = renderer.materialManager->get(node->renderObject->materialID);
        
        uploadMeshToGPU(*mesh, renderer.device);
        
        encoder->setRenderPipelineState(renderer.pipelineManger->get(mat->pipeID));
        
        size_t offset = node->renderObject->objectIndex * renderer.objectStride;
        
        encoder->setVertexBuffer(objBuffer, offset, 2);
        encoder->setFragmentBuffer(objBuffer, offset, 2);
        
        bindMaterial(node->renderObject->materialID, *mat, encoder, *renderer.textureManager, renderer);
        
        encoder->setVertexBuffer((MTL::Buffer*)mesh->vertexBuffer, 0, 0);
        
        encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, mesh->indexCount, INDEX_FORMAT, (MTL::Buffer*)mesh->indexBuffer, 0, 1);
    }
    
    for(auto* child : node->children){
        renderNode(child, renderer, encoder);
    }
}

void MainPass::release(){
    if(encoder){
        encoder->release();
        encoder = nullptr;
    }
}
