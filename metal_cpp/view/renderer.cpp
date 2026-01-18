//
//  renderer.cpp
//  metal_cpp
//
//  Created by Neel on 13/01/26.
//

#include "view/renderer.h"
#include "view/pipeline_factory.h"
#include "backend/mtlm.h"
#include "boids/instance.h"
#include <random>

Renderer::Renderer(MTL::Device* device, CA::MetalLayer* layer, GLFWwindow* glfwWindow):
device(device->retain()), metalLayer(layer->retain()), glfwWindow(glfwWindow)
{
    commandQueue = device->newCommandQueue();
    buildMeshes();
    buildShaders();
    boidManager = new BoidManager(device, commandQueue, glfwWindow);
}

Renderer::~Renderer() {
    quadMesh.vertexBuffer->release();
    quadMesh.indexBuffer->release();
    quadMesh.vertexDescriptor->release();
    
    triangleMesh->release();
//    trianglePipeline->release();
    
    generalPipeline->release();
    commandQueue->release();
    metalLayer->release();
    device->release();
}

void Renderer::buildMeshes() {
    triangleMesh = MeshFactory::buildTriangle(device);
    quadMesh = MeshFactory::buildQuad(device);
}

void Renderer::buildShaders() {
    PipelineBuilder* builder = new PipelineBuilder(device);
    builder->set_filename("shaders/general.metal");
    builder->set_vertex_descriptor(quadMesh.vertexDescriptor);
    builder->set_vertex_entry_point("vertexMainGeneral");
    builder->set_fragment_entry_point("fragmentMainGeneral");
    generalPipeline = builder->build();
    
    builder->set_filename("shaders/boids_render.metal");
    builder->set_vertex_entry_point("vertexMainBoid");
    builder->set_fragment_entry_point("fragmentMainBoid");
    boidRenderPipeline = builder->build();
    
    delete builder;
}

void Renderer::DrawFrame() {
    
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
    
    CA::MetalDrawable* drawable = metalLayer->nextDrawable();
    if(!drawable)return;
    
    MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
    
    boidManager->update_grid();
    
    // render pass
    MTL::RenderPassDescriptor* rpDesc = MTL::RenderPassDescriptor::alloc()->init();
    rpDesc->colorAttachments()->object(0)->setTexture(drawable->texture());
    rpDesc->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
    rpDesc->colorAttachments()->object(0)->setStoreAction(
            MTL::StoreActionStore);
        rpDesc->colorAttachments()->object(0)->setClearColor(
            MTL::ClearColor(0.1, 0.1, 0.15, 1.0));
    
    MTL::RenderCommandEncoder* encoder = commandBuffer->renderCommandEncoder(rpDesc);
    
    
    encoder->setRenderPipelineState(boidRenderPipeline);
    simd::float4x4 transform = mtlm::scale(0.01);
    encoder->setVertexBytes(&transform, sizeof(simd::float4x4), 1);
    encoder->setVertexBuffer(triangleMesh, 0, 0);
    if(parity == 0)
        encoder->setVertexBuffer(boidManager->boidOut, 0, 2);
    else
        encoder->setVertexBuffer(boidManager->boidIn, 0, 2);
    encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, 6, MTL::IndexTypeUInt16, quadMesh.indexBuffer, 0, boidManager->kCount);
     
    encoder->endEncoding();
    commandBuffer->presentDrawable(drawable);
    commandBuffer->commit();
    pool->release();
    first_frame = false;
}

