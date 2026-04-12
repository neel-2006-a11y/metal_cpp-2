//
//  renderer2.cpp
//  metal_cpp
//
//  Created by Neel on 22/03/26.
//

#include "view/renderer2.h"
#include "backend/mtlm.h"

inline size_t align256(size_t size) {
    return size;
    return (size + 255) & ~255;
}

Renderer2::Renderer2(MTL::Device* device, CA::MetalLayer* layer, GLFWwindow* glfwWindow): device(device), metalLayer(layer), glfwWindow(glfwWindow){
    commandQueue = device->newCommandQueue();
    
    // Default sampler
    MTL::SamplerDescriptor* default_desc = MTL::SamplerDescriptor::alloc()->init();
    default_desc->setMinFilter(MTL::SamplerMinMagFilterNearest);
    default_desc->setMagFilter(MTL::SamplerMinMagFilterNearest);
    default_desc->setMaxAnisotropy(1);
    default_desc->setMipFilter(MTL::SamplerMipFilterNotMipmapped);
    defaultSampler = device->newSamplerState(default_desc);
}

void Renderer2::initFrameBuffer(){
    frameStride = align256(sizeof(FrameUniforms));
    
    for(int i = 0; i<FIF; i++){
        frameBuffer[i] = device->newBuffer(frameStride, MTL::ResourceStorageModeShared);
    }
}

void Renderer2::uploadFrameUniforms(FrameUniforms& frame){
    
    MTL::Buffer* buffer = frameBuffer[frameIndex];
    memcpy(buffer->contents(), &frame, sizeof(FrameUniforms));
}

void Renderer2::initObjectBuffer(size_t maxObjCount){
    maxObjects = maxObjCount;
    objectStride = align256(sizeof(ObjectUniforms));
    
    size_t totalSize = objectStride * maxObjCount;
    for(int i=0; i<FIF; i++){
        objectBuffer[i] = device->newBuffer(totalSize, MTL::ResourceStorageModeShared);
    }
}

void Renderer2::uploadAllObjectUniforms(const std::vector<Entity> &objects){
    
    assert(objects.size() <= maxObjects);
    
    MTL::Buffer* buffer = objectBuffer[frameIndex];
    uint8_t* base = (uint8_t*) buffer->contents();
    
    for(size_t i = 0; i<objects.size(); i++){
        // build object uniforms
        ObjectUniforms uniforms;
        
        uniforms.model = objects[i].transform.computeModel();
        uniforms.invModel = simd::inverse(uniforms.model);
        uniforms.tileScale = objects[i].renderObject.tileScale;
        
        memcpy(base + i * objectStride, &uniforms, sizeof(ObjectUniforms));
    }
}

void Renderer2::uploadObjectUniforms(const std::vector<ObjectUniforms> &objs){
    assert(objs.size() <= maxObjects);
    
    MTL::Buffer* buffer = objectBuffer[frameIndex];
    uint8_t* base = (uint8_t*)buffer->contents();
    
    for (size_t i=0; i<objs.size(); i++) {
        memcpy(base + i*objectStride, &objs[i], sizeof(ObjectUniforms));
    }
}


void Renderer2::initMaterialBuffer(size_t maxMatCount){
    
    maxMaterials = maxMatCount;
    materialStride = align256(sizeof(MaterialUniforms));
    
    size_t totalSize = materialStride * maxMatCount;
    
    materialBuffer = device->newBuffer(totalSize, MTL::ResourceStorageModeShared);
}

void Renderer2::uploadAllMaterialUniforms(const std::vector<MaterialUniforms>& materials){
    
    assert(materials.size() <= maxMaterials);
    
    uint8_t* base = (uint8_t*)materialBuffer->contents();
    
    for(size_t i=0; i<materials.size(); i++){
        memcpy(base + i * materialStride, &materials[i], sizeof(MaterialUniforms));
    }
}
