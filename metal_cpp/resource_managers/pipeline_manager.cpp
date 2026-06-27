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
    renderPipelines[id] = pipeline;
    return id;
}


ComputePipelineID PipelineManager::createComputePipeline(ComputePipelineDesc &desc){
    NS::Error* error;
    
    MTL::Function* kernel = library->newFunction(NS::String::string(desc.kernelFunc.c_str(), NS::UTF8StringEncoding));
    
    if(!kernel){
        std::cerr << "kernel not found!" << std::endl;
        return INVALID_COMPUTE_PIPE;
    }
    
    MTL::ComputePipelineDescriptor* cpDesc = MTL::ComputePipelineDescriptor::alloc()->init();
    cpDesc->setComputeFunction(kernel);
    
    MTL::ComputePipelineState* pipeline = device->newComputePipelineState(kernel, &error);
    
//    __block MTL::ComputePipelineState* pipeline = nullptr;
//    device->newComputePipelineState(
//                                    cpDesc,
//                                    MTL::PipelineOptionBufferTypeInfo,
//                                    ^(MTL::ComputePipelineState* pPipeline, MTL::ComputePipelineReflection* pReflection, NS::Error* pError){
//                                        if(pError){
//                                            std::cout << "Pipeline compilation failed: " << pError->localizedDescription()->utf8String() << std::endl;
//                                            return;
//                                        }
//                                        
//                                        if(pReflection){
//                                            NS::Array* arguments = pReflection->arguments();
//                                            std::cout << "Kernel arguments count: " << arguments->count() << std::endl;
//                                        }
//                                        
//                                        if(pPipeline){
//                                            pipeline = pPipeline->retain();
//                                            std::cout << "Compute pipeline state successfully created!" << std::endl;
//                                        }
//                                    }
//                                    );
    // Release descriptor and kernel regardless of success
    cpDesc->release();
    kernel->release();

    if (!pipeline) {
        // Failed to create compute pipeline
        std::cout << error->localizedDescription()->utf8String() << std::endl;
        return INVALID_COMPUTE_PIPE;
    }

    ComputePipelineID id = nextComputeID++;
    computePipelines[id] = pipeline;
    return id;
}

MTL::RenderPipelineState* PipelineManager::get(PipelineID id){
    auto it = renderPipelines.find(id);
    if(it == renderPipelines.end()){
        std::cerr << "pipeline with id: " << id << "doesn't exist\n";
        return nullptr;
    }
    
    return it->second;
}

MTL::ComputePipelineState* PipelineManager::getCompute(ComputePipelineID id){
    auto it = computePipelines.find(id);
    if(it == computePipelines.end()){
        std::cerr << "compute pipeline with id: " << id << "doesn't exist\n";
        return nullptr;
    }
    return it->second;
}
