//
//  pipeline_factory.cpp
//  metal_cpp
//
//  Created by Neel on 15/01/26.
//

#include "view/pipeline_factory.h"

PipelineBuilder::PipelineBuilder(MTL::Device* device){
    this->device = device;
}

PipelineBuilder::~PipelineBuilder(){
}

void PipelineBuilder::set_filename(const char *filename){
    this->filename = filename;
}

void PipelineBuilder::set_vertex_entry_point(const char *entryPoint){
    this->vertEntryPoint = entryPoint;
}

void PipelineBuilder::set_fragment_entry_point(const char *entryPoint){
    this->fragEntryPoint = entryPoint;
}

void PipelineBuilder::set_vertex_descriptor(MTL::VertexDescriptor *descriptor){
    if(this->vertexDescriptor){
        this->vertexDescriptor->release();
    }
    this->vertexDescriptor = descriptor->retain();
}

MTL::RenderPipelineState* PipelineBuilder::build(){
    //Read the source code from the file.
    std::ifstream file;
    file.open(filename);
    std::stringstream reader;
    reader << file.rdbuf();
    std::string raw_string = reader.str();
    std::cout << raw_string << std::endl;
    NS::String* source_code = NS::String::string(
        raw_string.c_str(), NS::StringEncoding::UTF8StringEncoding);
    
    //A Metal Library constructs functions from source code
    NS::Error* error = nullptr;
    MTL::CompileOptions* options = nullptr;
    MTL::Library* library = device->newLibrary(source_code, options, &error);
    if (!library) {
        std::cout << error->localizedDescription()->utf8String() << std::endl;
    }
    
    NS::String* vertexName = NS::String::string(
        vertEntryPoint, NS::StringEncoding::UTF8StringEncoding);
    MTL::Function* vertexMain = library->newFunction(vertexName);
    
    NS::String* fragmentName = NS::String::string(
        fragEntryPoint, NS::StringEncoding::UTF8StringEncoding);
    MTL::Function* fragmentMain = library->newFunction(fragmentName);
    
    MTL::RenderPipelineDescriptor* pipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    pipelineDescriptor->setVertexFunction(vertexMain);
    pipelineDescriptor->setFragmentFunction(fragmentMain);
    pipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);
    pipelineDescriptor->setVertexDescriptor(vertexDescriptor);
    
    MTL::RenderPipelineState* pipeline = device->newRenderPipelineState(pipelineDescriptor, &error);
    if (!pipeline) {
        std::cout << error->localizedDescription()->utf8String() << std::endl;
    }
    
    if (error) {
        std::cout << "Pipeline warning: "
                  << error->localizedDescription()->utf8String()
                  << std::endl;
    }

    
    vertexMain->release();
    fragmentMain->release();
    pipelineDescriptor->release();
    library->release();
    file.close();
    
    return pipeline;
}


