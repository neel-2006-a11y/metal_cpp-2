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
    
    // Color attachment
    rp->colorAttachments()->object(0)->setTexture(drawableTexture);
    rp->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
    rp->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
    rp->colorAttachments()->object(0)->setClearColor({0.1, 0.1, 0.15, 1.0});
    
    // Depth attachment
    Texture* depth = r.textureManager->get(depthTexture);
    uploadEmptyTextureToGPU(*depth, r.device);
    
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
    MTL::Buffer* objBuffer = r.objectBuffer[r.frameIndex];
    int ind = 0;
    for(auto& obj : r.scene->objects){
        Mesh2* mesh = r.meshManager->getMesh(obj.meshID);
        Material* mat = r.materialManager->get(obj.materialID);
        
        uploadMeshToGPU(*mesh, r.device);
        
        encoder->setRenderPipelineState(r.pipelineManger->get(mat->pipeID));
        
        size_t offset = ind * r.objectStride;
        
        encoder->setVertexBuffer(objBuffer, offset, 2);
        encoder->setFragmentBuffer(objBuffer, offset, 2);
        
        bindMaterial(*mat, encoder, *r.textureManager, r.device);
        
        encoder->setVertexBuffer((MTL::Buffer*)mesh->vertexBuffer, 0, 0);
        
        encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, mesh->indexCount, MTL::IndexTypeUInt16, (MTL::Buffer*)mesh->indexBuffer, 0, 1);
        ind++;
    }
    encoder->endEncoding();
    rp->release();
    depthDesc->release();
    depthState->release();
}

void MainPass::release(){
    if(encoder){
        encoder->release();
        encoder = nullptr;
    }
}
