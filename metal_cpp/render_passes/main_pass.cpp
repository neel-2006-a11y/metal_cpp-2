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

void MainPass::init(){
    rpDesc = MTL::RenderPassDescriptor::alloc()->init();
    
    rpDesc->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
    rpDesc->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
    rpDesc->colorAttachments()->object(0)->setClearColor({0.1, 0.1, 0.15, 1.0});
    
    rpDesc->depthAttachment()->setLoadAction(MTL::LoadActionClear);
    rpDesc->depthAttachment()->setStoreAction(MTL::StoreActionStore);
    rpDesc->depthAttachment()->setClearDepth(1.0);
}

void MainPass::execute(RenderContext renderContext){
    
    // Color attachment
    Texture* sceneColor = renderContext.textureManager->get(sceneColorTexture);
    uploadTextureToGPU(*sceneColor, renderContext.renderer->device);
    rpDesc->colorAttachments()->object(0)->setTexture((MTL::Texture*)sceneColor->gpuTexture);
    

    // Depth attachment
    Texture* depth = renderContext.textureManager->get(depthTexture);
    uploadTextureToGPU(*depth, renderContext.renderer->device);
    rpDesc->depthAttachment()->setTexture((MTL::Texture*)depth->gpuTexture);
    
    
    encoder = renderContext.renderer->cmd->renderCommandEncoder(rpDesc);
    
    // depth state
    encoder->setDepthStencilState(renderContext.renderer->depthState);
    
    // FrameBuffer
    encoder->setVertexBuffer(renderContext.renderer->frameBuffer[renderContext.renderer->frameIndex], 0, 1);
    encoder->setFragmentBuffer(renderContext.renderer->frameBuffer[renderContext.renderer->frameIndex], 0, 1);
    
    MTL::Viewport viewPort;
    viewPort.originX = 0;
    viewPort.originY = 0;
    viewPort.width = renderContext.curr_fb_width;
    viewPort.height = renderContext.curr_fb_height;
    viewPort.znear = 0.0;
    viewPort.zfar = 1.0;
    encoder->setViewport(viewPort);
    
    // Draw everything
    renderNode(renderContext.root, renderContext, encoder);
    
    encoder->endEncoding();
}

void MainPass::renderNode(SceneNode *node, RenderContext renderContext, MTL::RenderCommandEncoder *encoder){
    if(node->renderObject){
        auto* objBuffer = renderContext.renderer->objectBuffer[renderContext.renderer->frameIndex];
        Mesh2* mesh = renderContext.meshManager->getMesh(node->renderObject->meshID);
        Material* mat = renderContext.materialManager->get(node->renderObject->materialID);
        
        uploadMeshToGPU(*mesh, renderContext.renderer->device);
        
        encoder->setRenderPipelineState(renderContext.pipelineManager->get(mat->pipeID));
        
        size_t offset = node->renderObject->objectIndex * renderContext.renderer->objectStride;
        
        encoder->setVertexBuffer(objBuffer, offset, 2);
        encoder->setFragmentBuffer(objBuffer, offset, 2);
        
        bindMaterial(node->renderObject->materialID, *mat, encoder, renderContext);
        
        encoder->setVertexBuffer((MTL::Buffer*)mesh->vertexBuffer, 0, 0);
        
        encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, mesh->indexCount, INDEX_FORMAT, (MTL::Buffer*)mesh->indexBuffer, 0, 1);
    }
    
    for(auto* child : node->children){
        renderNode(child, renderContext, encoder);
    }
}
