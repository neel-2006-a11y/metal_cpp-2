//
//  Engine.cpp
//  metal_cpp
//
//  Created by Neel on 26/03/26.
//

#include "view/Engine.h"
#include "view/mesh_factory.h"
#include "backend/mtlm.h"

Engine::Engine(Renderer2* renderer) : renderer(renderer), pipelineManager(renderer->device, renderer->device->newDefaultLibrary()), camera(float(800)/600), mouseHandler(renderer->glfwWindow), keyboard(renderer->glfwWindow){
    renderer->textureManager = &textureManager;
    renderer->meshManager = &meshManager;
    renderer->materialManager = &materialManager;
    renderer->pipelineManger = &pipelineManager;
    renderer->scene = &scene;
}

void Engine::init(){
    currTime = glfwGetTime();
    lastTime = glfwGetTime();
    
    VertexLayout layout;
    layout.stride = sizeof(Vertex3D);
    layout.attributes = {
        {0, offsetof(Vertex3D, position), VertexAttributeType::Float3},
        {1, offsetof(Vertex3D, color), VertexAttributeType::Float3},
        {2, offsetof(Vertex3D, uv), VertexAttributeType::Float2},
        {3, offsetof(Vertex3D, normal), VertexAttributeType::Float3}
    };
    
    //--------------
    // Create pipelines
    //--------------
    PipelineDesc mainDesc;
    mainDesc.vertexFunc = "vertexMain3D_dither_2";
    mainDesc.fragmentFunc = "fragmentMain3D_dither_2";
    mainDesc.colorFormat = TextureFormat::RGBA8Unorm;
    mainDesc.depthFormat = TextureFormat::Depth32Float;
    mainDesc.vertexLayout = layout;
    
    mainPipelineID = pipelineManager.createPipeline(mainDesc);
    
    PipelineDesc shadowDesc;
    shadowDesc.vertexFunc = "shadowVertex";
    shadowDesc.depthFormat = TextureFormat::Depth32Float;
    shadowDesc.vertexLayout = layout;
    
    shadowPipelineID = pipelineManager.createPipeline(shadowDesc);
    
    //--------------
    // Load Halftone Texture
    //--------------
    std::vector<std::string> filenames;
    int num_tex = HALFTONE_DIM == 8 ? 49 : 13;
    for (int i=0; i<num_tex; i++) {
        std::string filename = "halftone_textures/texture" + std::to_string(i) + ".ppm";
        filenames.push_back(filename);
    }
    
    TextureDesc halftone_desc;
    if(HALFTONE_TEXTURE_16BIT)
        halftone_desc.format = TextureFormat::R16Unorm;
    else
        halftone_desc.format = TextureFormat::R8Unorm;
    
    halftone_desc.usage = TextureUsage::Sampled;
    halftone_desc.storageMode = StorageMode::Shared;
    halftone_textureID = textureManager.loadPPMArray(filenames, halftone_desc);
    
    //--------------
    // build ShadowMap Texture
    //--------------
    TextureDesc shadowMap_desc;
    shadowMap_desc.format = TextureFormat::Depth32Float;
    shadowMap_desc.usage = TextureUsage::Sampled | TextureUsage::RenderTarget;
    shadowMap_desc.storageMode = StorageMode::Shared;
    shadowMap_desc.width = 1024;
    shadowMap_desc.height = 1024;
    shadowMap_desc.layers = CASCADES;
    shadowMapID = textureManager.createEmpty(shadowMap_desc);
    
//    //--------------
//    // build Depth Texture
//    //--------------
//    TextureDesc depth_Desc;
//    depth_Desc.format = TextureFormat::Depth32Float;
//    depth_Desc.usage = TextureUsage::DepthStencil;
//    depth_Desc.storageMode = StorageMode::Private;
//    depth_Desc.width = curr_width;
//    depth_Desc.height = curr_height;
//    depthTextureID = textureManager.createEmpty(depth_Desc);
    
    
    // Samplers
    MTL::SamplerDescriptor* halftone_sampler_desc = MTL::SamplerDescriptor::alloc()->init();
    halftone_sampler_desc->setMinFilter(MTL::SamplerMinMagFilterNearest);
    halftone_sampler_desc->setMagFilter(MTL::SamplerMinMagFilterNearest);
    halftone_sampler_desc->setMaxAnisotropy(1);
    halftone_sampler_desc->setMipFilter(MTL::SamplerMipFilterNotMipmapped);
    halftone_sampler = renderer->device->newSamplerState(halftone_sampler_desc);
    
    MTL::SamplerDescriptor* sdesc = MTL::SamplerDescriptor::alloc()->init();
    sdesc->setMinFilter(MTL::SamplerMinMagFilterLinear);
    sdesc->setMagFilter(MTL::SamplerMinMagFilterLinear);
    sdesc->setCompareFunction(MTL::CompareFunctionNever);
    
    shadowMap_sampler = renderer->device->newSamplerState(sdesc);
    
    //--------------
    // Create material
    //--------------
    halftoneMaterialID = materialManager.createMaterial(mainPipelineID);
    materialManager.setTexture(halftoneMaterialID, 0, shadowMapID);
    materialManager.setTexture(halftoneMaterialID, 1, halftone_textureID);
    materialManager.setSampler(halftoneMaterialID, 0, shadowMap_sampler);
    materialManager.setSampler(halftoneMaterialID, 1, halftone_sampler);
    
    //--------------
    // Setup Passes
    //--------------
    shadowPass.shadowMapArray = shadowMapID;
    shadowPass.shadowPipeline = shadowPipelineID;
    
    mainPass.depthTexture = depthTextureID;
    
    //--------------
    // Build RenderGraph
    //--------------
    renderGraph.addPass(&shadowPass);
    renderGraph.addPass(&mainPass);
    
    //--------------
    // setup Scene (eventually a manager for this too)
    //--------------
    RenderObject curtain;
    vertex_index_pair curtainData = MeshFactory::buildCube2(renderer->device, simd::float3{35,35,5});
    MeshID curtainMesh = meshManager.createMesh(curtainData.vertexData.data(), curtainData.vertexData.size(), curtainData.indexData);
    curtain.meshID = curtainMesh;
    curtain.materialID = halftoneMaterialID;
    
    curtain.uniforms.model = mtlm::identity();
    curtain.uniforms.model *= mtlm::translation({0, 0, 20});
    curtain.uniforms.invModel = simd_inverse(curtain.uniforms.model);
    curtain.uniforms.tileScale *= 0.5;
    scene.objects.push_back(curtain);
    
    
    RenderObject cube;
    vertex_index_pair cubeData = MeshFactory::buildCube2(renderer->device, simd::float3{5,5,5});
    MeshID cubeMesh = meshManager.createMesh(cubeData.vertexData.data(), cubeData.vertexData.size(), cubeData.indexData);
    cube.meshID = cubeMesh;
    cube.materialID = halftoneMaterialID;
    
    cube.uniforms.model = mtlm::identity();
    cube.uniforms.model *= mtlm::translation({-5, 0, 15});
    cube.uniforms.invModel = simd_inverse(cube.uniforms.model);
    cube.uniforms.tileScale *= 0.5;
    scene.objects.push_back(cube);
    
    RenderObject sphere;
    vertex_index_pair sphereData = MeshFactory::buildSphere2(renderer->device, 16, 16, 1);
    MeshID sphereMesh = meshManager.createMesh(sphereData.vertexData.data(), sphereData.vertexData.size(), sphereData.indexData);
    sphere.meshID = sphereMesh;
    sphere.materialID = halftoneMaterialID;
    
    sphere.uniforms.model = mtlm::identity();
    sphere.uniforms.model *= mtlm::translation({5, 0, 10});
    sphere.uniforms.invModel = simd_inverse(sphere.uniforms.model);
    sphere.uniforms.tileScale *= 0.5;
    scene.objects.push_back(sphere);
    
    // sun
    sun.direction = { 1.0f, 0.3f, 1.0f };
    sun.color = { 1.0f, 0.95f, 0.85f };
    sun.intensity = 1.0f;
    sun.cascades = CASCADES;
    
    //--------------
    // renderer Buffer Init
    //--------------
    renderer->initObjectBuffer(scene.objects.size());
    renderer->initFrameBuffer();
}


void Engine::update(){
    lastTime = currTime;
    currTime = glfwGetTime();
    
    keyboard.update();
    handleKeyboard();
    mouseHandler.updateCamera(&camera);
    
    frameU.view = camera.view();
    frameU.proj = camera.projection();
    frameU.viewProj = camera.viewProjection();
    
    frameU.intensity = sun.intensity;
    frameU.direction = sun.direction;
    frameU.color = sun.color;
    frameU.cascades = sun.cascades;
    
    auto cascadeSplits = camera.computeCascadeSplits(CASCADES, 0.5);
    auto worldFrustrums = camera.computeCascadeFrustrums(cascadeSplits);
    
    sun.fitCameraFrustrums(worldFrustrums);
    auto sunVps = sun.viewProj;
    
    std::memcpy(frameU.sunVPs, sunVps.data(), sizeof(simd::float4x4) * CASCADES);
    std::memcpy(frameU.cascadeSplits, cascadeSplits.data(), sizeof(float) * CASCADES);
    frameU.BayerScale = 0.06;
}

void Engine::render(){
    CA::MetalDrawable* drawable = renderer->metalLayer->nextDrawable();
    if (!drawable) {
        return;
    }
    
    std::cout << renderer->metalLayer->drawableSize().width << " " << renderer->metalLayer->drawableSize().height << std::endl;
    renderer->frameIndex = (renderer->frameIndex + 1) % FIF;
    
    renderer->cmd = renderer->commandQueue->commandBuffer();
    
    renderer->uploadFrameUniforms(frameU);
    renderer->uploadAllObjectUniforms(scene.objects);
    
    mainPass.drawableTexture = drawable->texture();
    
    renderGraph.execute(*renderer);
    renderer->cmd->presentDrawable(drawable);
    renderer->cmd->commit();
    renderer->cmd->waitUntilCompleted();
    renderGraph.release();
}

void Engine::handleKeyboard(){
    if(keyboard.wasPressed(GLFW_KEY_TAB)){
        camera.relativeMouse = !camera.relativeMouse;
        
        if(camera.relativeMouse){
            glfwSetInputMode(renderer->glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }else{
            glfwSetInputMode(renderer->glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    
    float dx = 0, dy = 0, dz = 0;
    if(keyboard.isDown(GLFW_KEY_W)) dz += 1;
    if (keyboard.isDown(GLFW_KEY_S)) dz -= 1;
    if (keyboard.isDown(GLFW_KEY_D)) dx += 1;
    if (keyboard.isDown(GLFW_KEY_A)) dx -= 1;
    if (keyboard.isDown(GLFW_KEY_E)) dy += 1;
    if (keyboard.isDown(GLFW_KEY_Q)) dy -= 1;
    
    float deltaTime = currTime - lastTime;
    camera.processKeyboard(dx, dy, dz, deltaTime);
}

void Engine::resize(int fbWidth, int fbHeight){
    std::cout << "RESIZED!!\n";
    curr_width = fbWidth;
    curr_height = fbHeight;
    renderer->curr_width = fbWidth;
    renderer->curr_height = fbHeight;
    
    //--------------
    // Update drawable size
    //--------------
    renderer->metalLayer->setDrawableSize(CGSizeMake(fbWidth, fbHeight));
    
    //--------------
    // Recreate depth Texture
    //--------------
    // delete Depth texture
    Texture* tex = textureManager.get(depthTextureID);
    if(tex && tex->uploaded){
        ((MTL::Texture*)tex->gpuTexture)->release();
        tex->uploaded = false;
    }
    // create Depth Texture
    TextureDesc depth_Desc;
    depth_Desc.format = TextureFormat::Depth32Float;
    depth_Desc.usage = TextureUsage::DepthStencil;
    depth_Desc.storageMode = StorageMode::Private;
    depth_Desc.width = fbWidth;
    depth_Desc.height = fbHeight;
    depthTextureID = textureManager.createEmpty(depth_Desc);
    
    //--------------
    // update Main Pass
    //--------------
    mainPass.depthTexture = depthTextureID;
    
    //--------------
    // update Camera aspect
    //--------------
    camera.setAspect(float(fbWidth)/ (float)fbHeight);
    
}
