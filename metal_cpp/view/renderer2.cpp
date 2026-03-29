//
//  renderer2.cpp
//  metal_cpp
//
//  Created by Neel on 22/03/26.
//

#include "view/renderer2.h"

inline size_t align256(size_t size) {
    return (size + 255) & ~255;
}

Renderer2::Renderer2(MTL::Device* device, CA::MetalLayer* layer, GLFWwindow* glfwWindow): device(device), metalLayer(layer), glfwWindow(glfwWindow){
    commandQueue = device->newCommandQueue();
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

void Renderer2::uploadAllObjectUniforms(const std::vector<RenderObject> &objects){
    MTL::Buffer* buffer = objectBuffer[frameIndex];
    uint8_t* base = (uint8_t*) buffer->contents();
    
    for(size_t i = 0; i<objects.size(); i++){
        memcpy(base + i * objectStride, &objects[i].uniforms, sizeof(ObjectUniforms));
    }
}
