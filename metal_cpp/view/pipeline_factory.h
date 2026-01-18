//
//  pipeline_factory.h
//  metal_cpp
//
//  Created by Neel on 15/01/26.
//

#pragma once
#include "config.h"

class PipelineBuilder{
public:
    PipelineBuilder(MTL::Device* device);
    
    ~PipelineBuilder();
    
    void set_filename(const char* filename);
    
    void set_vertex_entry_point(const char* entryPoint);
    
    void set_fragment_entry_point(const char* entryPoint);
    
    void set_vertex_descriptor(MTL::VertexDescriptor* descriptor);
    
    MTL::RenderPipelineState* build();
    
private:
    const char* filename, *vertEntryPoint, *fragEntryPoint;
    MTL::VertexDescriptor* vertexDescriptor = nullptr;
    MTL::Device* device;
};
