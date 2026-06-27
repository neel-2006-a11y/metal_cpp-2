//
//  shadow_pass.cpp
//  metal_cpp
//
//  Created by Neel on 23/03/26.
//

#include "render_passes/shadow_pass.h"
#include "passers/MTL_Texture_passer.h"
#include "passers/MTL_Mesh_passer.h"
#include "config.h"

void ShadowPass::init(){
    for(int i=0; i < CASCADES; i++){
        rpDescs[i] = MTL::RenderPassDescriptor::alloc()->init();
        rpDescs[i]->depthAttachment()->setSlice(i);
        rpDescs[i]->depthAttachment()->setLoadAction(MTL::LoadActionClear);
        rpDescs[i]->depthAttachment()->setStoreAction(MTL::StoreActionStore);
        rpDescs[i]->depthAttachment()->setClearDepth(1.0);
    }
}

void ShadowPass::execute(RenderContext renderContext){
    
    Texture* shadowTex = renderContext.textureManager->get(shadowMapArray);
    uploadTextureToGPU(*shadowTex, renderContext.renderer->device);
    
    MTL::RenderPipelineState* rp = renderContext.pipelineManager->get(shadowPipeline);
    
    auto* mtlTex = (MTL::Texture*)shadowTex->gpuTexture;
    
    for(int i = 0; i<cascades; i++){
        
        rpDescs[i]->depthAttachment()->setTexture(mtlTex);
        
        encoders[i] = renderContext.renderer->cmd->renderCommandEncoder(rpDescs[i]);
        auto* encoder = encoders[i];
        
        // Pipeline
        encoder->setRenderPipelineState(rp);
        
        // depth state
        encoder->setDepthStencilState(renderContext.renderer->depthState);
        
        // Frame Buffer
        encoder->setVertexBuffer(renderContext.renderer->frameBuffer[renderContext.renderer->frameIndex], 0, 1);
        
        // cascade Index
        uint32_t cascadeInd = i;
        encoder->setVertexBytes(&cascadeInd, sizeof(uint32_t), 3);
        
        renderNode(renderContext.root, renderContext, encoder);
        
        encoder->endEncoding();
    }
}

void ShadowPass::renderNode(SceneNode* node, RenderContext renderContext, MTL::RenderCommandEncoder* encoder){
    if(node->renderObject){
        Mesh2* mesh = renderContext.meshManager->getMesh(node->renderObject->meshID);
        uploadMeshToGPU(*mesh, renderContext.renderer->device);
        
        size_t offset = node->renderObject->objectIndex * renderContext.renderer->objectStride;
        
        encoder->setVertexBuffer(renderContext.renderer->objectBuffer[renderContext.renderer->frameIndex], offset, 2);
        
        // Mesh Vertex Data
        encoder->setVertexBuffer((MTL::Buffer*)mesh->vertexBuffer, 0, 0);
        
        encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, mesh->indexCount, INDEX_FORMAT, (MTL::Buffer*)mesh->indexBuffer, 0, 1);
    }
    
    for(auto* child : node->children){
        renderNode(child, renderContext, encoder);
    }
}
