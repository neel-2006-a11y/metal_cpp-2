//
//  renderer2.cpp
//  metal_cpp
//
//  Created by Neel on 22/03/26.
//

#include "view/renderer2.h"
#include "backend/mtlm.h"
#include <dispatch/dispatch.h>
#include "passers/MTL_Texture_passer.h"

inline size_t align256(size_t size) {
    return size;
//    return (size + 255) & ~255;
}

Renderer2::Renderer2(MTL::Device* device, CA::MetalLayer* layer, GLFWwindow* glfwWindow, dispatch_semaphore_t* frameSemaphore): device(device), metalLayer(layer), glfwWindow(glfwWindow), pipelineManager(device, device->newDefaultLibrary()), modelLoader(&meshManager, &textureManager, &materialManager), depthStencilStateManager(device), samplerManager(device){
    
    commandQueue = device->newCommandQueue();
    this->frameSemaphore = frameSemaphore;
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
//    initMaterialBuffer(10);
    
    auto rd = renderDevice();
    shadowPass.init(rd);
    mainPass.init(rd);
    outlinePass.init(rd);
    volumetricPass.init(rd);
//    fluidPass.init(rd);
    compositePass.init(rd);
    imGuiPass.init(rd);
    
    
    // debug data init
    debug.sunDirection = { 1.0f, 0.3f, 1.0f };
    debug.BayerScale = 0.5;
    debug.volumeDensity = 0.03;
    debug.g = 0.6;

}

void Renderer2::resize(){
    
    resizePasses = true;
    std::cout << "renderer resized!!!!\n";
    
    int window_w,window_h, fb_w,fb_h;
    glfwGetWindowSize(glfwWindow, &window_w, &window_h);
    glfwGetFramebufferSize(glfwWindow, &fb_w, &fb_h);
    
    curr_window_width = window_w;
    curr_window_height = window_h;
    curr_fb_width = fb_w;
    curr_fb_height = fb_h;
    
    //--------------
    // Update drawable size
    //--------------
    metalLayer->setDrawableSize(CGSizeMake(fb_w, fb_h));
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
    frameUniforms.far = frameData.camera->farZ;
    frameUniforms.view = frameData.camera->view();
    frameUniforms.proj = frameData.camera->projection();
    frameUniforms.viewProj = frameData.camera->viewProjection();
    frameUniforms.invViewProj = inverse(frameUniforms.viewProj);
    
    frameUniforms.intensity = frameData.sun->intensity;
    frameData.sun->direction = simd::normalize(debug.sunDirection); // get from debug
    frameUniforms.direction = frameData.sun->direction;
    frameUniforms.color = frameData.sun->color;
    frameUniforms.cascades = frameData.sun->cascades;
    
    auto sunVps = frameData.sun->viewProj;
    
    memcpy(frameUniforms.sunVPs, sunVps.data(), sizeof(simd::float4x4) * frameData.sun->cascades);
    memcpy(frameUniforms.cascadeSplits, frameData.sun->cascadeSplits.data(), sizeof(float) * frameData.sun->cascades);
    
    // debug
    frameUniforms.BayerScale = debug.BayerScale; // get from debug
    frameUniforms.volumeDensity = debug.volumeDensity;
    frameUniforms.g = debug.g;
    frameUniforms.exp_factor = debug.exp_factor;
    
    frameUniforms.has_oblique_plane = (int)frameData.camera->hasPlane();
    frameUniforms.plane = frameData.camera->getPlane();
}

ViewUniforms Renderer2::makeViewUniforms(Camera* camera, DirectionalLight* sun){
    ViewUniforms vf;
    vf.cameraPos = camera->position();
    vf.far = camera->farZ;
    vf.view = camera->view();
    vf.proj = camera->projection();
    vf.viewProj = camera->viewProjection();
    vf.invViewProj = inverse(vf.viewProj);
    
    // fit the shadowmap in camera's frustrum
    sun->update(camera);
    
    auto sunVps = sun->viewProj;
    
    memcpy(vf.sunVPs, sunVps.data(), sizeof(simd::float4x4) * sun->cascades);
    memcpy(vf.cascadeSplits, sun->cascadeSplits.data(), sizeof(float) * sun->cascades);
    
    vf.has_oblique_plane = (int)camera->hasPlane();
    vf.plane = camera->getPlane();
    
    return vf;
}

void Renderer2::render(Scene &scene, ResourceRegistry &resourceRegistry, FrameDataRegistry &frameDataRegistry, std::vector<RenderView> &renderViews){
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
    beginFrame();
    
    // frame buffer
    makeFrameUniforms(frameDataRegistry.get<FrameInfo>());
    uploadFrameUniforms();
    
    // material buffer
    std::vector<MaterialUniforms> matUniforms;
    auto mat_mp = materialManager.getAllUniforms(matUniforms);
    uploadAllMaterialUniforms(mat_mp, matUniforms);
    
    // object buffer
    std::vector<Renderable> renderables;
    scene.extractRenderables(renderables);
    uploadObjectUniforms(renderables);
    
    
    // build rendercontext
    RenderContext renderContext;
    
    renderContext.device = device;
    renderContext.cmd = cmd;
    
    
    // buffers
    renderContext.frameBuffer = frameBuffer[frameIndex];
    
    renderContext.materialBufferData.materialBuffer = materialBuffer[frameIndex];
    renderContext.materialBufferData.materialStride = materialStride;
    
    renderContext.objectBufferData.objectBuffer = objectBuffer[frameIndex];
    renderContext.objectBufferData.objectStride = objectStride;
    
    // render Queue (render objects)
    renderContext.renderQ = makeRenderQueue(renderables);
    
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
    renderContext.debugData = &debug;
    
    
    //--------------
    // Build RenderGraph
    //--------------
    RenderGraph renderGraph(&resourceRegistry, &frameDataRegistry);
    renderGraph.addPass(&shadowPass);
    renderGraph.addPass(&mainPass);
    renderGraph.addPass(&outlinePass);
//    renderGraph.addPass(&volumetricPass);
//    renderGraph.addPass(&fluidPass);
    renderGraph.addPass(&compositePass);
    renderGraph.addPass(&imGuiPass);
    
    
    RenderGraph renderGraph2(&resourceRegistry, &frameDataRegistry);
    renderGraph2.addPass(&shadowPass);
    renderGraph2.addPass(&mainPass);
    renderGraph2.addPass(&outlinePass);
    renderGraph2.addPass(&compositePass);
    
    for(auto rv : renderViews){
        // material overrides
        for(auto mo : rv.mat_overrides){
            for(auto tb : mo.bindings)
                materialManager.setTexture(mo.mat, tb.slot, tb.texture);
        }
        
        ViewUniforms vf = makeViewUniforms(rv.camera, &scene.getSun());
        
        renderContext.viewUniforms = vf;
        if(rv.useDrawable){
            renderContext.backBuffer = backbuffer;
            renderGraph.execute(renderContext, resizePasses);
        }else{
            Texture* RT = textureManager.get(rv.renderTarget);
            uploadEmptyTextureToGPU(*RT, device);
            renderContext.backBuffer = RT->gpuTexture;
            renderGraph2.execute(renderContext, resizePasses);
        }
        
        resizePasses = false;
    }
    
    
    endFrame();
    
    pool->release();
}


void Renderer2::beginFrame(TextureID RT){
    cmd = commandQueue->commandBuffer();
    if(RT==INVALID_TEXTURE){
        currentDrawable = metalLayer->nextDrawable();
        backbuffer = currentDrawable->texture();
    }else{
        Texture* RTTex = textureManager.get(RT);
        backbuffer = (MTL::Texture*)RTTex->gpuTexture;
    }
//    printf("fb_width: %d, fb_height: %d\n", curr_fb_width, curr_fb_height);
//    printf("tex_width: %d, tex_height: %d\n", backbuffer->width(), backbuffer->height());
//    printf("dr_width: %d, dr_height: %d\n", metalLayer->drawableSize().width, metalLayer->drawableSize().height);
//    printf("\n");
}

void Renderer2::endFrame(TextureID RT){
    if (RT == INVALID_TEXTURE && currentDrawable) {
        cmd->presentDrawable(currentDrawable);
    }

    dispatch_semaphore_t semaphore = *(this->frameSemaphore);

    cmd->addCompletedHandler([semaphore](MTL::CommandBuffer* /*cb*/) {
        if (semaphore) {
            dispatch_semaphore_signal(semaphore);
        }
    });

    cmd->commit();
    frameIndex = (frameIndex + 1) % FIF;
}

RenderQueue Renderer2::makeRenderQueue(const std::vector<Renderable> &renderables){
    RenderQueue rq;
    
    for(int i=0; i<renderables.size(); i++){
        RenderObject ro;
        ro.mesh = renderables[i].renderComponent->meshID;
        ro.material = renderables[i].renderComponent->materialID;
        
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


//void Renderer2::initMaterialBuffer(size_t maxMatCount){
//    
//    maxMaterials = maxMatCount;
//    materialStride = align256(sizeof(MaterialUniforms));
//    
//    size_t totalSize = materialStride * maxMatCount;
//    
//    for(int i=0; i<FIF; i++){
//        materialBuffer[i] = device->newBuffer(totalSize, MTL::ResourceStorageModeShared);
//    }
//}

void Renderer2::uploadAllMaterialUniforms(std::unordered_map<int,MaterialID> &mat_mp, const std::vector<MaterialUniforms>& materials){
    
//    assert(materials.size() <= maxMaterials);
    size_t totalSize = 0;
    for(auto matU : materials){
        totalSize += matU.data.size();
    }
    if(materialBuffer[frameIndex] == nullptr)
        materialBuffer[frameIndex] = device->newBuffer(totalSize, MTL::ResourceStorageModeShared);
    
    uint8_t* base = (uint8_t*)materialBuffer[frameIndex]->contents();
    
    uint32_t offset = 0;
    
    for(int i=0; i<materials.size(); i++){
        MaterialID mat_ID = mat_mp[i];
        auto mat = materialManager.get(mat_ID);
        mat->uniform_offset = offset;
        memcpy(base + offset, materials[i].data.data(), materials[i].data.size());
        offset += materials[i].data.size();
    }
}

