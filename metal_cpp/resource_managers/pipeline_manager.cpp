//
//  pipeline_manager.cpp
//  metal_cpp
//
//  Created by Neel on 23/03/26.
//

#include "resource_managers/pipeline_manager.h"
#include "converters/MTL_converters.h"

PipelineManager::PipelineManager(MTL::Device* device, MTL::Library* library): device(device), library(library){}


PipelineID PipelineManager::createPipeline(PipelineDesc& desc){
    NS::Error* error = nullptr;
    
    MTL::Function* vFunc = library->newFunction(NS::String::string(desc.vertexFunc.c_str(), NS::UTF8StringEncoding));
    
    if (!vFunc) {
        printf("Vertex function not found!\n");
    }
    
    MTL::Function* fFunc = nullptr;
    if(!desc.fragmentFunc.empty()){
        fFunc = library->newFunction(NS::String::string(desc.fragmentFunc.c_str(), NS::UTF8StringEncoding));
    }
    
    auto* rpDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    rpDesc->setVertexFunction(vFunc);
    if(fFunc!=nullptr)
        rpDesc->setFragmentFunction(fFunc);
    
    auto* vertexDescriptor = buildMTLVertexDescriptor(desc.vertexLayout);
    rpDesc->setVertexDescriptor(vertexDescriptor);
    
    rpDesc->colorAttachments()->object(0)->setPixelFormat(toMTLPixelFormat(desc.colorFormat));
    rpDesc->setDepthAttachmentPixelFormat(toMTLPixelFormat(desc.depthFormat));
    
    MTL::RenderPipelineState* pipeline = device->newRenderPipelineState(rpDesc, &error);
    if(!pipeline){
        std::cerr << error->localizedDescription()->utf8String() << std::endl;
    }
    
    vFunc->release();
    if(fFunc)
        fFunc->release();
    vertexDescriptor->release();
    rpDesc->release();
    
    PipelineID id = nextID++;
    pipelines[id] = pipeline;
    return id;
}


MTL::RenderPipelineState* PipelineManager::get(PipelineID id){
    auto it = pipelines.find(id);
    if(it == pipelines.end()){
        std::cerr << "pipeline with id: " << id << "doesn't exist\n";
    }
    
    return it->second;
}
