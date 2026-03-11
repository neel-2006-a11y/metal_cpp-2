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
#include "config.h"
#include "view/directional_light.h"
#include "view/dithering.h"

Renderer::Renderer(MTL::Device* device, CA::MetalLayer* layer, GLFWwindow* glfwWindow):
device(device->retain()), metalLayer(layer->retain()), glfwWindow(glfwWindow)
{
    commandQueue = device->newCommandQueue();
    buildMeshes();
    buildShaders();
    
    // shadowMap
    MTL::TextureDescriptor* shadowDesc = MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatDepth32Float, 2048, 2048, false);
    shadowDesc->setUsage(MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead);
    
    shadowDesc->setStorageMode(MTL::StorageModePrivate);
    shadowMap = device->newTexture(shadowDesc);
    MTL::SamplerDescriptor* sdesc = MTL::SamplerDescriptor::alloc()->init();
    sdesc->setMinFilter(MTL::SamplerMinMagFilterLinear);
    sdesc->setMagFilter(MTL::SamplerMinMagFilterLinear);
    sdesc->setCompareFunction(MTL::CompareFunctionNever);
    
    shadowSampler = device->newSamplerState(sdesc);
    
    // depth state
    MTL::DepthStencilDescriptor* depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
    depthDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
    depthDesc->setDepthWriteEnabled(true);
    depthState = device->newDepthStencilState(depthDesc);
    depthDesc->release();
    
    // render pass descriptor
    renderPassDesc = MTL::RenderPassDescriptor::alloc()->init();
    shadowPassDesc = MTL::RenderPassDescriptor::alloc()->init();
    
    
    // shadowPassDesc Depth Attachment
    shadowPassDesc->depthAttachment()->setTexture(shadowMap);
    shadowPassDesc->depthAttachment()->setLoadAction(MTL::LoadActionClear);
    shadowPassDesc->depthAttachment()->setStoreAction(MTL::StoreActionStore);
    shadowPassDesc->depthAttachment()->setClearDepth(1.0);
    
    // initial dpeth texture
    CA::MetalDrawable* drawable = nullptr;
    while(!drawable)
        drawable = metalLayer->nextDrawable();
    
    curr_width = (int)drawable->texture()->width();
    curr_height = (int)drawable->texture()->height();
    
    // camera
    camera = new Camera(float(curr_width) / curr_height);
    std::cout << "Original camera ptr:" << camera << std::endl;
    Resize();
    mouseHandler = new MouseHandler(glfwWindow);
    
    // dithering
    Bayer4x4Transitions(BayerTransitions);
    for (int i=0; i<4; i++) {
        for (int j=0; j<8; j++) {
            for (int k=0; k<8; k++) {
                std::cout << BayerTransitions[i][j][k] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    std::cout << "Bayer matrices modified";
    std::cout << std::endl;
}


void Renderer::Resize(){
    prev_width = curr_width;
    prev_height = curr_height;
    
    // adjust camera aspect
    if(camera)
        camera->setAspect(float(curr_width)/curr_height);
    
    
    // new depth texture
    if(depthTexture){
        depthTexture->release();
        depthTexture = nullptr;
    }
    
    MTL::TextureDescriptor* depthDesc = MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatDepth32Float, curr_width, curr_height, false);
    depthDesc->setUsage(MTL::TextureUsageRenderTarget);
    depthDesc->setStorageMode(MTL::StorageModePrivate);
    
    depthTexture = device->newTexture(depthDesc);
    // Depth attachment
    renderPassDesc->depthAttachment()->setTexture(depthTexture);
    renderPassDesc->depthAttachment()->setLoadAction(MTL::LoadActionClear);
    renderPassDesc->depthAttachment()->setStoreAction(MTL::StoreActionDontCare);
    renderPassDesc->depthAttachment()->setClearDepth(1.0);
}

Renderer::~Renderer() {
    cubeMesh.vertexBuffer->release();
        cubeMesh.indexBuffer->release();
        cubeMesh.vertexDescriptor->release();

        quadMesh.vertexBuffer->release();
        quadMesh.indexBuffer->release();
        quadMesh.vertexDescriptor->release();

        triangleMesh.vertexBuffer->release();
        triangleMesh.indexBuffer->release();
        triangleMesh.vertexDescriptor->release();

        depthTexture->release();
        depthState->release();
        renderPassDesc->release();

        generalPipeline->release();
        commandQueue->release();
        metalLayer->release();
        device->release();
}

void Renderer::buildMeshes() {
    triangleMesh = MeshFactory::buildTriangle3D(device);
    quadMesh = MeshFactory::buildQuad(device);
    cubeMesh = MeshFactory::buildCube(device);
    sphereMesh = MeshFactory::buildSphere(device, 16, 16, 1);
}

void Renderer::buildShaders() {
    PipelineBuilder* builder = new PipelineBuilder(device);
    
    builder->set_color_pixel_format(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);
    builder->set_depth_pixel_format(MTL::PixelFormatDepth32Float);
    builder->set_filename("shaders/general3D.metal");
    builder->set_vertex_descriptor(cubeMesh.vertexDescriptor);
    builder->set_vertex_entry_point("vertexMain3D");
    builder->set_fragment_entry_point("fragmentMain3D");
    generalPipeline = builder->build();
    
    
    builder->set_filename("shaders/shadowMap.metal");
    builder->set_color_pixel_format(MTL::PixelFormatInvalid);
    builder->set_vertex_descriptor(cubeMesh.vertexDescriptor);
    builder->set_vertex_entry_point("shadowVertex");
    builder->set_fragment_entry_point(nullptr);
    shadowPipeline = builder->build();
    
    delete builder;
}

void Renderer::DrawFrame() {
NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
    
    glfwGetWindowSize(glfwWindow, &curr_width, &curr_height);
    if(curr_width!=prev_width || curr_height!=prev_height)
        Resize();
    
    // handle camera inputs
    TAB_DOWN = glfwGetKey(glfwWindow, GLFW_KEY_TAB);
    if(TAB_DOWN && !TAB_WAS_DOWN){
        camera->relativeMouse = !camera->relativeMouse;
        if(camera->relativeMouse){
            glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }else{
            glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    TAB_WAS_DOWN = TAB_DOWN;
    
    // WASD + QE
    float dx = 0, dy = 0, dz = 0;
    if (glfwGetKey(glfwWindow, GLFW_KEY_W)) dz += 1;
    if (glfwGetKey(glfwWindow, GLFW_KEY_S)) dz -= 1;
    if (glfwGetKey(glfwWindow, GLFW_KEY_D)) dx += 1;
    if (glfwGetKey(glfwWindow, GLFW_KEY_A)) dx -= 1;
    if (glfwGetKey(glfwWindow, GLFW_KEY_E)) dy += 1;
    if (glfwGetKey(glfwWindow, GLFW_KEY_Q)) dy -= 1;

    float deltaTime = 0.01f;
    camera->processKeyboard(dx, dy, dz, deltaTime);
    
    mouseHandler->updateCamera(camera);

    CA::MetalDrawable* drawable = metalLayer->nextDrawable();
    if (!drawable) {
        pool->release();
        return;
    }
    
    MTL::CommandBuffer* commandBuffer =
        commandQueue->commandBuffer();
    
    // set scene
    
    std::vector<Object*> objects;
    Object cube;
    cubeAngle+=0.2;
    cube.mesh = &cubeMesh;
    cube.objectU.model = mtlm::identity();
    cube.objectU.model *= mtlm::translation({-5, 0, 15});
    cube.objectU.scale = simd::float3{5.0,5.0,5.0};
    cube.objectU.model *= mtlm::scale3D(cube.objectU.scale);
//    cube.objectU.model *= mtlm::y_rotation(cubeAngle);
    cube.objectU.invModel = simd_inverse(cube.objectU.model);
    cube.objectU.scale *= cubeScale;
    
    objects.push_back(&cube);
    
    Object curtain;
    curtain.mesh = &cubeMesh;
    curtain.objectU.model = mtlm::identity();
    curtain.objectU.model *= mtlm::translation({0, 0, 20});
    curtain.objectU.scale = simd::float3{35.0,35.0,5.0};
    curtain.objectU.model *= mtlm::scale3D(curtain.objectU.scale);
    curtain.objectU.invModel = simd_inverse(curtain.objectU.model);
    curtain.objectU.scale *= cubeScale;
    objects.push_back(&curtain);
    
    Object sphere;
    sphere.mesh = &sphereMesh;
    sphere.objectU.model = mtlm::identity();
    sphere.objectU.model *= mtlm::translation({5, 0, 10});
    sphere.objectU.model *= mtlm::y_rotation(cubeAngle);
    sphere.objectU.invModel = simd_inverse(sphere.objectU.model);
    sphere.objectU.scale = simd::float3(1.0);
    sphere.objectU.scale *= sphereScale;
    objects.push_back(&sphere);
    
    
    sun.direction = { 0.3f, 0.3f, 1.0f };
    sun.color = { 1.0f, 0.95f, 0.85f };
    sun.intensity = 1.0f;
    
    // render shadowMap
    MTL::RenderCommandEncoder* shadowEnc = commandBuffer->renderCommandEncoder(shadowPassDesc);
    shadowEnc->setRenderPipelineState(shadowPipeline);
    shadowEnc->setDepthStencilState(depthState);
    
    auto lightVP = sun.ViewProj();
    
    for(auto o : objects){
        shadowEnc->setVertexBuffer(o->mesh->vertexBuffer, 0, 0);
        shadowEnc->setVertexBytes(&lightVP, sizeof(simd::float4x4), 1);
        shadowEnc->setVertexBytes(&o->objectU.model, sizeof(simd::float4x4), 2);
        
        shadowEnc->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, o->mesh->indexCount, MTL::IndexTypeUInt16, o->mesh->indexBuffer, 0);
    }
    
    
    shadowEnc->endEncoding();
    
    // render general
    // ---- Color attachment ----
    auto color = renderPassDesc->colorAttachments()->object(0);
    color->setTexture(drawable->texture());
    color->setLoadAction(MTL::LoadActionClear);
    color->setStoreAction(MTL::StoreActionStore);
    color->setClearColor({0.1, 0.1, 0.15, 1.0});

    MTL::RenderCommandEncoder* encoder =
        commandBuffer->renderCommandEncoder(renderPassDesc);

    encoder->setRenderPipelineState(generalPipeline);
    encoder->setDepthStencilState(depthState);
    encoder->setFrontFacingWinding(MTL::WindingCounterClockwise);

    // ---- cam Uniforms ----
    CameraUniforms camU{};
    camU.view = camera->view();
    camU.proj = camera->projection();
    camU.viewProj = camera->viewProjection();

    encoder->setVertexBytes(&camU, sizeof(camU), 1);
    
    // ---- dithering ----
    encoder->setFragmentBytes(&BayerTransitions, 4*64*(sizeof(float)), 9);
    
    // ---- light ----
    simple sSun;
    sSun.color = sun.color;
    sSun.direction = sun.direction;
    sSun.intensity = sun.intensity;
    encoder->setFragmentBytes(&sSun, sizeof(simple), 3);
    encoder->setFragmentBytes(&lightVP, sizeof(simd::float4x4), 5);
    auto camPosition = camera->position();
    encoder->setFragmentBytes(&camPosition, sizeof(simd::float3), 4);
    
    // ----- shadowMap -----
    encoder->setFragmentTexture(shadowMap, 0);
    encoder->setFragmentSamplerState(shadowSampler, 0);
    
    for(auto o : objects){
        encoder->setVertexBuffer(o->mesh->vertexBuffer,0,0);
        encoder->setVertexBytes(&o->objectU, sizeof(ObjectUniforms), 2);
//        simd::float3 tot_scale = BayerScale * o->objectU.scale;
        simd::float3 tot_scale = BayerScale;
        encoder->setFragmentBytes(&o->objectU.scale, sizeof(simd::float3), 7);
        encoder->setFragmentBytes(&tot_scale, sizeof(simd::float3), 8);
        encoder->drawIndexedPrimitives(
            MTL::PrimitiveTypeTriangle,
            o->mesh->indexCount,
            MTL::IndexTypeUInt16,
            o->mesh->indexBuffer,
            0
        );
    }
    

    encoder->endEncoding();
    commandBuffer->presentDrawable(drawable);
    commandBuffer->commit();

    pool->release();
}

