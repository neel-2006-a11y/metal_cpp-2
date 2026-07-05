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
//    return (size + 255) & ~255;
}

Renderer2::Renderer2(MTL::Device* device, CA::MetalLayer* layer, GLFWwindow* glfwWindow): device(device), metalLayer(layer), glfwWindow(glfwWindow), pipelineManager(device, device->newDefaultLibrary()), modelLoader(&meshManager, &textureManager, &materialManager), depthStencilStateManager(device), samplerManager(device){
    commandQueue = device->newCommandQueue();
}

void Renderer2::init(){
    //--------------
    // ImGui init
    //--------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOther(glfwWindow, true);
    ImGui_ImplMetal_Init(device);
    
    glfwGetWindowSize(glfwWindow, &curr_window_width, &curr_window_height);
    glfwGetFramebufferSize(glfwWindow, &curr_fb_width, &curr_fb_height);
    
    initFrameBuffer();
    initObjectBuffer(10);
    initMaterialBuffer(10);
    
    auto rd = renderDevice();
    shadowPass.init(rd);
    mainPass.init(rd);
    volumetricPass.init(rd);
    fluidPass.init(rd);
    compositePass.init(rd);
    imGuiPass.init(rd);
}

void Renderer2::resize(int width, int height, int fb_width, int fb_height){
    resizePasses = true;
    
    curr_window_width = width;
    curr_window_height = height;
    curr_fb_width = fb_width;
    curr_fb_height = fb_height;
    
    //--------------
    // Update drawable size
    //--------------
    metalLayer->setDrawableSize(CGSizeMake(fb_width, fb_height));
}

RenderDevice Renderer2::renderDevice(){
    RenderDevice rd;
    
    rd.device = device;
    rd.meshManager = &meshManager;
    rd.materialManager = &materialManager;
    rd.textureManager = &textureManager;
    rd.modelLoader = &modelLoader;
    rd.depthStencilStateManager = &depthStencilStateManager;
    rd.samplerManager = &samplerManager;
    rd.pipelineManager = &pipelineManager;
    
    rd.curr_fb_width = curr_fb_width; rd.curr_fb_height = curr_fb_height;
    rd.curr_window_width = curr_window_width; rd.curr_window_height = curr_window_height;
    
    return rd;
}

void Renderer2::makeFrameUniforms(const FrameInfo& frameData){
    frameUniforms.cameraPos = frameData.camera->position();
    frameUniforms.view = frameData.camera->view();
    frameUniforms.proj = frameData.camera->projection();
    frameUniforms.viewProj = frameData.camera->viewProjection();
    frameUniforms.invViewProj = inverse(frameUniforms.viewProj);
    
    frameUniforms.intensity = frameData.sun->intensity;
    frameData.sun->direction = simd::normalize(debug.sunDirection);
    frameUniforms.direction = frameData.sun->direction;
    frameUniforms.color = frameData.sun->color;
    frameUniforms.cascades = frameData.sun->cascades;
    
    auto sunVps = frameData.sun->viewProj;
    
    memcpy(frameUniforms.sunVPs, sunVps.data(), sizeof(simd::float4x4) * frameData.sun->cascades);
    memcpy(frameUniforms.cascadeSplits, frameData.sun->cascadeSplits.data(), sizeof(float) * frameData.sun->cascades);
    frameUniforms.BayerScale = debug.BayerScale;
    
    frameUniforms.volumeDensity = debug.volumeDensity;
    frameUniforms.g = debug.g;
}

void Renderer2::render(Scene& scene, ResourceRegistry& resourceRegistry, FrameDataRegistry& frameDataRegisry){
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
    
    beginFrame();
    
    // frame buffer
    makeFrameUniforms(frameDataRegisry.get<FrameInfo>());
    uploadFrameUniforms();
    
    // material buffer
    std::vector<MaterialUniforms> matUniforms;
    auto mat_mp = materialManager.getAllUniforms(matUniforms);
    uploadAllMaterialUniforms(matUniforms);
    
    // object buffer
    std::vector<Renderable> renderables;
    extractRenderables(scene.getRoot(), renderables);
    uploadObjectUniforms(renderables);
    
    
    // rendercontext
    RenderContext renderContext;
    
    renderContext.device = device;
    renderContext.cmd = cmd;
    renderContext.backBuffer = backbuffer;
    
    // buffers
    renderContext.frameBuffer = frameBuffer[frameIndex];
    
    renderContext.materialBufferData.materialBuffer = materialBuffer[frameIndex];
    renderContext.materialBufferData.materialStride = materialStride;
    
    renderContext.objectBufferData.objectBuffer = objectBuffer[frameIndex];
    renderContext.objectBufferData.objectStride = objectStride;
    
    // render Queue (render objects)
    renderContext.renderQ = makeRenderQueue(mat_mp, renderables);
    
    // window dim
    renderContext.curr_fb_width = curr_fb_width;
    renderContext.curr_fb_height = curr_fb_height;
    renderContext.curr_window_width = curr_window_width;
    renderContext.curr_window_height = curr_window_height;
    
    // managers
    renderContext.textureManager = &textureManager;
    renderContext.materialManager = &materialManager;
    renderContext.meshManager = &meshManager;
    renderContext.pipelineManager = &pipelineManager;
    renderContext.modelLoader = &modelLoader;
    renderContext.depthStencilStateManager = &depthStencilStateManager;
    renderContext.samplerManager = &samplerManager;
    
    // debug
    renderContext.debugData = debug;
    
    
    //--------------
    // Build RenderGraph
    //--------------
    RenderGraph renderGraph(&resourceRegistry, &frameDataRegisry);
    renderGraph.addPass(&shadowPass);
    renderGraph.addPass(&mainPass);
    renderGraph.addPass(&volumetricPass);
    renderGraph.addPass(&fluidPass);
    renderGraph.addPass(&compositePass);
//    renderGraph.addPass(&imGuiPass);
    
//    // temporary
//    imGuiPass.root = scene.getRoot();
    
    renderGraph.execute(renderContext, resizePasses);
    resizePasses = false;
    
    endFrame();
    
    pool->release();
}


void Renderer2::beginFrame(){
    frameIndex = (frameIndex + 1) % FIF;
    cmd = commandQueue->commandBuffer();
    currentDrawable = metalLayer->nextDrawable();
    backbuffer = currentDrawable->texture();
}

void Renderer2::endFrame(){
    cmd->presentDrawable(currentDrawable);
    cmd->commit();
}

RenderQueue Renderer2::makeRenderQueue(const std::unordered_map<MaterialID, int> &mat_mp, const std::vector<Renderable> &renderables){
    RenderQueue rq;
    
    for(int i=0; i<renderables.size(); i++){
        RenderObject ro;
        ro.mesh = renderables[i].renderComponent->meshID;
        ro.material = renderables[i].renderComponent->materialID;
        
        auto it = mat_mp.find(ro.material);
        ro.material_index = it == mat_mp.end() ? -1 : it->second;
        
        ro.object_index = i;
        
        rq.renderObjects.push_back(std::move(ro));
    }
    
    return rq;
}


// buffer helpers
void Renderer2::initFrameBuffer(){
    frameStride = align256(sizeof(FrameUniforms));
    
    for(int i = 0; i<FIF; i++){
        frameBuffer[i] = device->newBuffer(frameStride, MTL::ResourceStorageModeShared);
    }
}

void Renderer2::uploadFrameUniforms(){
    
    MTL::Buffer* buffer = frameBuffer[frameIndex];
    memcpy(buffer->contents(), &frameUniforms, sizeof(FrameUniforms));
}

void Renderer2::initObjectBuffer(size_t maxObjCount){
    maxObjects = maxObjCount;
    objectStride = align256(sizeof(ObjectUniforms));
    
    size_t totalSize = objectStride * maxObjCount;
    for(int i=0; i<FIF; i++){
        objectBuffer[i] = device->newBuffer(totalSize, MTL::ResourceStorageModeShared);
    }
}

void Renderer2::uploadObjectUniforms(const std::vector<Renderable> &renderables){
    assert(renderables.size() <= maxObjects);
    
    MTL::Buffer* buffer = objectBuffer[frameIndex];
    uint8_t* base = (uint8_t*)buffer->contents();
    
    for (size_t i=0; i<renderables.size(); i++) {
        memcpy(base + i*objectStride, &renderables[i].objectUniforms, sizeof(ObjectUniforms));
    }
}


void Renderer2::initMaterialBuffer(size_t maxMatCount){
    
    maxMaterials = maxMatCount;
    materialStride = align256(sizeof(MaterialUniforms));
    
    size_t totalSize = materialStride * maxMatCount;
    
    for(int i=0; i<FIF; i++){
        materialBuffer[i] = device->newBuffer(totalSize, MTL::ResourceStorageModeShared);
    }
}

void Renderer2::uploadAllMaterialUniforms(const std::vector<MaterialUniforms>& materials){
    
    assert(materials.size() <= maxMaterials);
    
    uint8_t* base = (uint8_t*)materialBuffer[frameIndex]->contents();
    
    for(size_t i=0; i<materials.size(); i++){
        memcpy(base + i * materialStride, &materials[i], sizeof(MaterialUniforms));
    }
}
