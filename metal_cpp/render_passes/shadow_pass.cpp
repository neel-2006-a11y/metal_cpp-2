//
//  shadow_pass.cpp
//  metal_cpp
//
//  Created by Neel on 23/03/26.
//

#include "render_passes/shadow_pass.h"
#include "passers/MTL_Texture_passer.h"
#include "passers/MTL_Mesh_passer.h"
#include "scene/SceneNode.h"
#include "config.h"

void ShadowPass::init(RenderDevice renderDevice){
    // define vertex layout
    VertexLayout layout;
    layout.stride = sizeof(Vertex3D);
    layout.attributes = {
        {0,0, offsetof(Vertex3D, position), VertexAttributeType::Float3},
        {0,1, offsetof(Vertex3D, color), VertexAttributeType::Float3},
        {0,2, offsetof(Vertex3D, uv), VertexAttributeType::Float2},
        {0,3, offsetof(Vertex3D, normal), VertexAttributeType::Float3}
    };
    
    // define pipeline
    PipelineDesc shadowDesc;
    shadowDesc.vertexFunc = "shadowVertex";
    shadowDesc.depthFormat = TextureFormat::Depth32Float;
    shadowDesc.vertexLayout = layout;
    
    shadowPipeline = renderDevice.pipelineManager->createPipeline(shadowDesc);
    
    // depth state
    DepthStencilDesc desc;
    desc.compare = MTL::CompareFunctionLess;
    desc.depthWrite = true;
    
    depthState = renderDevice.depthStencilStateManager->createDepthState(desc);
    
    //--------------
    // build ShadowMap Texture
    //--------------
    TextureDesc shadowMap_desc;
    shadowMap_desc.textureType = TextureType::TextureType2DArray;
    shadowMap_desc.format = TextureFormat::Depth32Float;
    shadowMap_desc.usage = TextureUsage::Sampled | TextureUsage::RenderTarget;
    shadowMap_desc.storageMode = StorageMode::Shared;
    shadowMap_desc.width = shadow_res;
    shadowMap_desc.height = shadow_res;
    shadowMap_desc.layers = cascades;
    shadowMapArray = renderDevice.textureManager->createEmpty(shadowMap_desc);
    
    
    for(int i=0; i < cascades; i++){
        rpDescs[i] = MTL::RenderPassDescriptor::alloc()->init();
        rpDescs[i]->depthAttachment()->setSlice(i);
        rpDescs[i]->depthAttachment()->setLoadAction(MTL::LoadActionClear);
        rpDescs[i]->depthAttachment()->setStoreAction(MTL::StoreActionStore);
        rpDescs[i]->depthAttachment()->setClearDepth(1.0);
    }
}

void ShadowPass::execute(RenderContext renderContext, RenderGraph* renderGraph){
    
    Texture* shadowTex = renderContext.textureManager->get(shadowMapArray);
    uploadTextureToGPU(*shadowTex, renderContext.device);
    
    MTL::RenderPipelineState* rp = renderContext.pipelineManager->get(shadowPipeline);
    
    auto* mtlTex = (MTL::Texture*)shadowTex->gpuTexture;
    
    for(int i = 0; i<cascades; i++){
        
        rpDescs[i]->depthAttachment()->setTexture(mtlTex);
        
        encoders[i] = renderContext.cmd->renderCommandEncoder(rpDescs[i]);
        auto* encoder = encoders[i];
        
        // Pipeline
        encoder->setRenderPipelineState(rp);
        
        // depth state
        encoder->setDepthStencilState(renderContext.depthStencilStateManager->getState(depthState)->gpu_depthState);
        
        // Frame Buffer
        encoder->setVertexBuffer(renderContext.frameBuffer, 0, 1);
        
        // View Buffer
        encoder->setVertexBytes(&renderContext.viewUniforms, sizeof(ViewUniforms), 4);
        
        // cascade Index
        uint32_t cascadeInd = i;
        encoder->setVertexBytes(&cascadeInd, sizeof(uint32_t), 3);
        
        for(auto ro : renderContext.renderQ.renderObjects){
            Mesh2* mesh = renderContext.meshManager->getMesh(ro.mesh);
            uploadMeshToGPU(*mesh, renderContext.device);
            
            size_t offset = ro.object_index * renderContext.objectBufferData.objectStride;
            
            encoder->setVertexBuffer(renderContext.objectBufferData.objectBuffer, offset, 2);
            
            // Mesh Vertex Data
            encoder->setVertexBuffer((MTL::Buffer*)mesh->vertexBuffer, 0, 0);
            
            encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, mesh->indexCount, INDEX_FORMAT, (MTL::Buffer*)mesh->indexBuffer, 0, 1);
        }
        
        encoder->endEncoding();
    }
    publish(renderGraph);
}

void ShadowPass::publish(RenderGraph* renderGraph){
    renderGraph->resourceRegistry->publishTexture(ResourceKey::ShadowMapTexture, shadowMapArray);
}

void ShadowPass::resize(RenderContext renderContext){
    Texture* tex = renderContext.textureManager->get(shadowMapArray);
    
    // delete previous
    if(tex && tex->uploaded){
        tex->gpuTexture->release();
        tex->uploaded = false;
    }
    // build new
    TextureDesc shadowMap_desc;
    shadowMap_desc.textureType = TextureType::TextureType2DArray;
    shadowMap_desc.format = TextureFormat::Depth32Float;
    shadowMap_desc.usage = TextureUsage::Sampled | TextureUsage::RenderTarget;
    shadowMap_desc.storageMode = StorageMode::Shared;
    shadowMap_desc.width = shadow_res;
    shadowMap_desc.height = shadow_res;
    shadowMap_desc.layers = cascades;
    shadowMapArray = renderContext.textureManager->createEmpty(shadowMap_desc);
    
}
