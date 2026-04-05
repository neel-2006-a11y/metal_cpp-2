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

void ShadowPass::execute(Renderer2 &renderer){
    auto* device = renderer.device;
    auto* cmd = renderer.cmd;
    
    Texture* shadowTex = renderer.textureManager->get(shadowMapArray);
    uploadTextureToGPU(*shadowTex, device);
    
    MTL::RenderPipelineState* rp = renderer.pipelineManger->get(shadowPipeline);
    
    auto* mtlTex = (MTL::Texture*)shadowTex->gpuTexture;
    
    for(int i = 0; i<cascades; i++){
        MTL::RenderPassDescriptor* rpDesc = MTL::RenderPassDescriptor::alloc()->init();
        
        rpDesc->depthAttachment()->setTexture(mtlTex);
        rpDesc->depthAttachment()->setSlice(i);
        rpDesc->depthAttachment()->setLoadAction(MTL::LoadActionClear);
        rpDesc->depthAttachment()->setStoreAction(MTL::StoreActionStore);
        rpDesc->depthAttachment()->setClearDepth(1.0);
        
        encoders[i] = cmd->renderCommandEncoder(rpDesc)->retain();
        auto* encoder = encoders[i];
        
        // Pipeline
        encoder->setRenderPipelineState(rp);
        
        // depth state
        MTL::DepthStencilDescriptor* depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
        depthDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
        depthDesc->setDepthWriteEnabled(true);
        MTL::DepthStencilState* depthState = device->newDepthStencilState(depthDesc);
        depthDesc->release();
        encoder->setDepthStencilState(depthState);
        
        // Frame Buffer
        encoder->setVertexBuffer(renderer.frameBuffer[renderer.frameIndex], 0, 1);
        
        // cascade Index
        uint32_t cascadeInd = i;
        encoder->setVertexBytes(&cascadeInd, sizeof(uint32_t), 3);
        
        // Draw all objects
        MTL::Buffer* objBuffer = renderer.objectBuffer[renderer.frameIndex];
        int ind = 0;
        for(auto& obj : renderer.scene->objects)
        {
            Mesh2* mesh = renderer.meshManager->getMesh(obj.meshID);
            uploadMeshToGPU(*mesh, renderer.device);
            
            size_t offset = ind * renderer.objectStride;
            
            encoder->setVertexBuffer(objBuffer, offset, 2);
            
            // Mesh Vertex Data
            encoder->setVertexBuffer((MTL::Buffer*)mesh->vertexBuffer, 0, 0);
            
            encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, mesh->indexCount, INDEX_FORMAT, (MTL::Buffer*)mesh->indexBuffer, 0, 1);
            ind++;
        }
        
        encoder->endEncoding();
        depthState->release();
        rpDesc->release();
    }
}

void ShadowPass::release(){
    for(int i=0; i<cascades; i++){
        encoders[i]->release();
    }
}
