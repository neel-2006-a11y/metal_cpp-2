//
//  Engine.cpp
//  metal_cpp
//
//  Created by Neel on 26/03/26.
//

#include "view/Engine.h"
#include "view/mesh_factory.h"
#include "backend/mtlm.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_metal.h"

Engine::Engine(Renderer2* renderer) : renderer(renderer), pipelineManager(renderer->device, renderer->device->newDefaultLibrary()), camera(float(800)/600), mouseHandler(renderer->glfwWindow), keyboard(renderer->glfwWindow), modelLoader(&meshManager, &textureManager, &materialManager), fluid(256, 200, &textureManager, &pipelineManager){
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
    
    VertexLayout LineVertexLayout;
    LineVertexLayout.stride = sizeof(LineVertex);
    LineVertexLayout.attributes = {
        {0, offsetof(Vertex3D, position), VertexAttributeType::Float3},
        {1, offsetof(Vertex3D, color), VertexAttributeType::Float4}
    };
    
    // asset directory
    assetDirectory = "/Users/admin/Desktop/metal_cpp-2/metal_cpp/assets";
    
    //--------------
    // Create pipelines
    //--------------
    PipelineDesc mainDesc;
    mainDesc.vertexFunc = "vertexMain3D_dither_2";
    mainDesc.fragmentFunc = "fragmentMain3D_dither_2";
    mainDesc.colorFormat = TextureFormat::RGBA8Unorm;
    mainDesc.depthFormat = TextureFormat::Depth32Float;
    mainDesc.vertexLayout = layout;
    
    halftone_pipeID = pipelineManager.createPipeline(mainDesc);
    
    //
    PipelineDesc shadowDesc;
    shadowDesc.vertexFunc = "shadowVertex";
    shadowDesc.depthFormat = TextureFormat::Depth32Float;
    shadowDesc.vertexLayout = layout;
    
    shadowPipelineID = pipelineManager.createPipeline(shadowDesc);
    
    //
    PipelineDesc compDesc;
    compDesc.vertexFunc = "compositeVS";
    compDesc.fragmentFunc = "compositeFS";
    compDesc.colorFormat = TextureFormat::RGBA8Unorm;
    
    compositePipelineID = pipelineManager.createPipeline(compDesc);
    
    //
    PipelineDesc volDesc;
    volDesc.vertexFunc = "fullScreenVS";
    volDesc.fragmentFunc = "volumetricFS";
    volDesc.colorFormat = TextureFormat::RGBA8Unorm;
    
    volumetricPipelineID = pipelineManager.createPipeline(volDesc);
    
    //
    PipelineDesc IDDesc;
    IDDesc.vertexFunc = "vertex_ID";
    IDDesc.fragmentFunc = "frag_ID";
    IDDesc.colorFormat = TextureFormat::R32Uint;
    IDDesc.depthFormat = TextureFormat::Depth32Float;
    IDDesc.vertexLayout = LineVertexLayout;
    
    IDPipelineID = pipelineManager.createPipeline(IDDesc);
    
    //
    PipelineDesc LineDesc;
    LineDesc.vertexFunc = "lineVertex";
    LineDesc.fragmentFunc = "lineFragment";
    LineDesc.colorFormat = TextureFormat::RGBA8Unorm;
    LineDesc.depthFormat = TextureFormat::Depth32Float;
    LineDesc.vertexLayout = LineVertexLayout;
    
    LinePipelineID = pipelineManager.createPipeline(LineDesc);
    
    //-------------- Compute Pipelines
    ComputePipelineDesc add_val_desc;
    add_val_desc.kernelFunc = "add_val";
    fluid.add_val_pipe = pipelineManager.createComputePipeline(add_val_desc);
    
    ComputePipelineDesc set_bnd_1_desc;
    set_bnd_1_desc.kernelFunc = "set_bnd_1";
    fluid.set_bnd_1_pipe = pipelineManager.createComputePipeline(set_bnd_1_desc);
    
    ComputePipelineDesc set_bnd_2_desc;
    set_bnd_2_desc.kernelFunc = "set_bnd_2";
    fluid.set_bnd_2_pipe = pipelineManager.createComputePipeline(set_bnd_2_desc);
    
    ComputePipelineDesc add_source_desc;
    add_source_desc.kernelFunc = "add_source";
    fluid.add_source_pipe = pipelineManager.createComputePipeline(add_source_desc);
    std::cout << "fluid _add_source_pipe: " << fluid.add_source_pipe << std::endl;
    auto add_source_PS = pipelineManager.getCompute(fluid.add_source_pipe);
    std::cout << "add_source_max_threads_per_TG: " << add_source_PS->maxTotalThreadsPerThreadgroup() << std::endl;
    std::cout << "add_source_thread_exec_width: " << add_source_PS->threadExecutionWidth() << std::endl;
    
    ComputePipelineDesc diffuse_step_desc;
    diffuse_step_desc.kernelFunc = "diffuse_step";
    fluid.diffuse_step_pipe = pipelineManager.createComputePipeline(diffuse_step_desc);
    
    ComputePipelineDesc advect_desc;
    advect_desc.kernelFunc = "advect";
    fluid.advect_pipe = pipelineManager.createComputePipeline(advect_desc);
    
    ComputePipelineDesc divergence_desc;
    divergence_desc.kernelFunc = "divergence";
    fluid.divergence_pipe = pipelineManager.createComputePipeline(divergence_desc);
    
    ComputePipelineDesc p_step_desc;
    p_step_desc.kernelFunc = "p_step";
    fluid.p_step_pipe = pipelineManager.createComputePipeline(p_step_desc);
    
    ComputePipelineDesc sub_p_desc;
    sub_p_desc.kernelFunc = "sub_p";
    fluid.sub_p_pipe = pipelineManager.createComputePipeline(sub_p_desc);
    
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
    // Load Blue Noise
    //--------------
    std::string blueNoiseFile = "/Users/admin/Desktop/metal_cpp-2/metal_cpp/assets/BlueNoise470.png";
    blueNoiseTextureID = textureManager.loadFromFile(blueNoiseFile);
    
    //--------------
    // build ShadowMap Texture
    //--------------
    TextureDesc shadowMap_desc;
    shadowMap_desc.format = TextureFormat::Depth32Float;
    shadowMap_desc.usage = TextureUsage::Sampled | TextureUsage::RenderTarget;
    shadowMap_desc.storageMode = StorageMode::Shared;
    shadowMap_desc.width = shadow_res;
    shadowMap_desc.height = shadow_res;
    shadowMap_desc.layers = CASCADES;
    shadowMapID = textureManager.createEmpty(shadowMap_desc);
    
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
    halftoneMaterialID = materialManager.createMaterial(halftone_pipeID);
    materialManager.setTexture(halftoneMaterialID, 0, shadowMapID);
    materialManager.setTexture(halftoneMaterialID, 1, halftone_textureID);
    materialManager.setSampler(halftoneMaterialID, 0, shadowMap_sampler);
    materialManager.setSampler(halftoneMaterialID, 1, halftone_sampler);
    
    lineMaterialID = materialManager.createMaterial(LinePipelineID);
    
    //--------------
    // Setup Passes
    //--------------
    shadowPass.shadowMapArray = shadowMapID;
    shadowPass.shadowPipeline = shadowPipelineID;
    
    volumetricPass.shadowMap = shadowMapID;
    volumetricPass.volumetricPipeID = volumetricPipelineID;
    volumetricPass.sampler = shadowMap_sampler;
    volumetricPass.blueNoiseTexture = blueNoiseTextureID;
    
    compositePass.pipeline = compositePipelineID;
    
    //--------------
    // Build RenderGraph
    //--------------
    renderGraph.addPass(&shadowPass);
    renderGraph.addPass(&mainPass);
    renderGraph.addPass(&volumetricPass);
    renderGraph.addPass(&compositePass);
    renderGraph.addPass(&imGuiPass);
    renderGraph.init();
    
    
    
    //--------------
    // load Meshes from Mesh Factory
    //--------------
    vertex_index_pair curtainData = MeshFactory::buildCube2(simd::float3{35,35,5});
    MeshID curtainMesh = meshManager.createMesh(curtainData);
    
    vertex_index_pair cubeData = MeshFactory::buildCube2(simd::float3{5,5,5});
    MeshID cubeMesh = meshManager.createMesh(cubeData);
    
    vertex_index_pair sphereData = MeshFactory::buildSphere2(16, 16, 1);
    MeshID sphereMesh = meshManager.createMesh(sphereData);
    
    //--------------
    // load gizmos
    //--------------
    gizmos.push_back(GizmoFactory::generateTranslateGizmo(lineMaterialID, INVALID_MATERIAL, meshManager));
    
    //
    
    //--------------
    // setup Scene (eventually a manager for this too)
    //--------------
    // root
    //  |-->cube
    //  |-->curtain
    //  |-->sphere
    //  |-->Cactus
    //       |-->flower
    //       |-->pot
    //       |-->cactus
    //--------------
    root.name = "Root";
    
    //--------------
    SceneNode* curtainNode = createNode(&root);
    curtainNode->name = "Curtain";

    curtainNode->renderObject = new RenderObject();
    curtainNode->renderObject->meshID = curtainMesh;
    curtainNode->renderObject->materialID = halftoneMaterialID;
    curtainNode->renderObject->tileScale = 0.75;

    curtainNode->localT.position = {0, 0, 20};
    
    //--------------
    SceneNode* cubeNode = createNode(&root);
    cubeNode->name = "Cube";

    cubeNode->renderObject = new RenderObject();
    cubeNode->renderObject->meshID = cubeMesh;
    cubeNode->renderObject->materialID = halftoneMaterialID;
    cubeNode->renderObject->tileScale = 0.75;

    cubeNode->localT.position = {-5, 0, 15};
    cubeNode->localT.scale = {2.0,1.0,1.0};
    
    //--------------
    SceneNode* sphereNode = createNode(&root);
    sphereNode->name = "Sphere";
    
    sphereNode->renderObject = new RenderObject();
    sphereNode->renderObject->meshID = sphereMesh;
    sphereNode->renderObject->materialID = halftoneMaterialID;
    sphereNode->renderObject->tileScale = 0.75;
    
    sphereNode->localT.position = {5, 0, 15};
    
    //--------------
    SceneNode* cactusNode = modelLoader.loadModelAsNode(assetDirectory + "/test_cactus/test_cactus.obj");
    assignParent(cactusNode, &root);
    assignPipelineID(cactusNode, halftone_pipeID, materialManager);
    
    // sun
    sun.direction = { 1.0f, 0.3f, 1.0f };
    sun.color = { 1.0f, 0.95f, 0.85f };
    sun.intensity = 1.0f;
    sun.cascades = CASCADES;
    
    //--------------
    // renderer Buffer Init
    //--------------
    renderer->initObjectBuffer(10);
    renderer->initFrameBuffer();
    renderer->initMaterialBuffer(MAX_MATERIALS);
    
    //--------------
    // ImGui init
    //--------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    
    ImGui_ImplGlfw_InitForOther(renderer->glfwWindow, true);
    ImGui_ImplMetal_Init(renderer->device);
    
    //--------------
    // upload textures to gpu
    //--------------
    uploadTextureToGPU(*textureManager.get(fluid.density_source_texture), renderer->device);
    uploadEmptyTextureToGPU(*textureManager.get(fluid.density_prev_texture), renderer->device);
    uploadEmptyTextureToGPU(*textureManager.get(fluid.density_texture), renderer->device);
    
    uploadEmptyTextureToGPU(*textureManager.get(fluid.u_texture), renderer->device);
    uploadEmptyTextureToGPU(*textureManager.get(fluid.v_texture), renderer->device);
    
    uploadEmptyTextureToGPU(*textureManager.get(fluid.uPrev_texture), renderer->device);
    uploadEmptyTextureToGPU(*textureManager.get(fluid.vPrev_texture), renderer->device);
    
    uploadTextureToGPU(*textureManager.get(fluid.su_texture), renderer->device);
    uploadEmptyTextureToGPU(*textureManager.get(fluid.sv_texture), renderer->device);
    
    uploadEmptyTextureToGPU(*textureManager.get(fluid.pingPong_texture), renderer->device);
    uploadEmptyTextureToGPU(*textureManager.get(fluid.zero_texture), renderer->device);
}


void Engine::update(){
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
    
    lastTime = currTime;
    currTime = glfwGetTime();
    
    keyboard.update();
    mouseHandler.update();
    handleKeyboard();
    mouseHandler.updateCamera(&camera);
    
    frameU.cameraPos = camera.position();
    frameU.view = camera.view();
    frameU.proj = camera.projection();
    frameU.viewProj = camera.viewProjection();
    frameU.invViewProj = inverse(frameU.viewProj);
    
    frameU.intensity = sun.intensity;
    sun.direction = simd::normalize(renderer->debug.sunDirection);
    frameU.direction = sun.direction;
    frameU.color = sun.color;
    frameU.cascades = sun.cascades;
    
    auto cascadeSplits = camera.computeCascadeSplits(CASCADES, 0.5);
    auto worldFrustrums = camera.computeCascadeFrustrums(cascadeSplits);
    
    sun.fitCameraFrustrums(worldFrustrums);
    auto sunVps = sun.viewProj;
    
    memcpy(frameU.sunVPs, sunVps.data(), sizeof(simd::float4x4) * CASCADES);
    memcpy(frameU.cascadeSplits, cascadeSplits.data(), sizeof(float) * CASCADES);
    frameU.BayerScale = renderer->debug.BayerScale;
    
    frameU.volumeDensity = renderer->debug.volumeDensity;
    frameU.g = renderer->debug.g;
    
    
    // fluid interaction
    auto fluid_CMD = renderer->commandQueue->commandBuffer();
    if(keyboard.isDown(GLFW_KEY_K)){
        int gridX = (mouseHandler.currX / curr_window_width) * (fluid.N+2);
        int gridY= (mouseHandler.currY / curr_window_height) * (fluid.N+2);
        
        gridX = std::max(0,std::min(fluid.N+1, gridX));
        gridY = std::max(0,std::min(fluid.N+1, gridY));
        fluid.addDensity(gridX, gridY, 10, 10.0, fluid_CMD);
    }
    fluid.addVelocity(fluid.N/2, fluid.N/2, 10, 40.0, 0, fluid_CMD);
    
    fluid.step_2(0.016, fluid_CMD);
    fluid_CMD->commit();
//    Texture* dens_tex = textureManager.get(fluid.density_texture);
    
    // 1
//    memcpy(dens_tex->raw_data.data(), fluid.getDensity().data(), (fluid.N+2) * (fluid.N+2) * sizeof(float));
//    updateTexture(*dens_tex, renderer->device);
    
    pool->release();
}

void Engine::render(){
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
    
    CA::MetalDrawable* drawable = renderer->metalLayer->nextDrawable();
    if (!drawable) {
        return;
    }
    
    //--------------
    // update Passes
    //--------------
    mainPass.depthTexture = depthTextureID;
    mainPass.sceneColorTexture = sceneColorTextureID;
    
    volumetricPass.depthTexture = depthTextureID;
    volumetricPass.volumetricColorTexture = volumetricTextureID;
    
    compositePass.inputColor = sceneColorTextureID;
    compositePass.volumeColor = volumetricTextureID;
    compositePass.drawableTexture = drawable->texture();
    compositePass.densityTexture = fluid.density_texture;
    
    imGuiPass.root = &root;
    imGuiPass.selected = selected;
    imGuiPass.drawableTexture = drawable->texture();
    
    renderer->cmd = renderer->commandQueue->commandBuffer();
    
    //----------
    // upload Buffers
    //----------
    renderer->frameIndex = (renderer->frameIndex + 1) % FIF;
    
    // frame
    renderer->uploadFrameUniforms(frameU);
    
    // object
    std::vector<ObjectUniforms> objs;
    uint32_t index = 0;
    buildObjectBuffer(&root, objs, index);
    renderer->uploadObjectUniforms(objs);
    objs.clear();
    
    // material
    auto matUniforms = materialManager.getAllUniforms();
    renderer->uploadAllMaterialUniforms(matUniforms);
    
    //----------
    // Execute
    //----------
//    struct RenderContext{
//        Renderer2* renderer;
//        
//        SceneNode* root;
//        Camera* camera;
//        
//        MaterialManager* materialManager;
//        MeshManager* meshManager;
//        PipelineManager* pipelineManager;
//        TextureManager* textureManager;
//        
//        int curr_window_width, curr_window_height;
//        int curr_fb_width, curr_fb_height;
//    };
    
    RenderContext rc;
    rc.renderer = renderer;
    rc.root = &root;
    rc.camera = &camera;
    rc.materialManager = &materialManager;
    rc.meshManager = &meshManager;
    rc.pipelineManager = &pipelineManager;
    rc.textureManager = &textureManager;
    rc.curr_window_width = curr_window_width;
    rc.curr_window_height = curr_window_height;
    rc.curr_fb_width = curr_fb_width;
    rc.curr_fb_height = curr_fb_height;
    
    renderGraph.execute(rc);
    
    renderer->cmd->presentDrawable(drawable);
    
    renderer->cmd->commit();
//    renderer->cmd->waitUntilCompleted();
    
    //----------
    // fetch selected node for future
    //----------
    selected = imGuiPass.selected;
    
    pool->release();
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

void Engine::resize(int width, int height, int fbWidth, int fbHeight){
    std::cout << "RESIZED!!\n";
    curr_window_width = width;
    curr_window_height = height;
    curr_fb_width = fbWidth;
    curr_fb_height = fbHeight;
    
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
    depth_Desc.usage = TextureUsage::DepthStencil | TextureUsage::Sampled;
    depth_Desc.storageMode = StorageMode::Private;
    depth_Desc.width = fbWidth;
    depth_Desc.height = fbHeight;
    depthTextureID = textureManager.createEmptyNoCPU(depth_Desc);
    
    //--------------
    // Recreate color Texture
    //--------------
    // delete
    Texture* colTex = textureManager.get(sceneColorTextureID);
    if(colTex && colTex->uploaded){
        ((MTL::Texture*)colTex->gpuTexture)->release();
        colTex->uploaded = false;
    }
    // create
    TextureDesc col_Desc;
    col_Desc.format = TextureFormat::RGBA8Unorm;
    col_Desc.usage = TextureUsage::RenderTarget | TextureUsage::Sampled;
    col_Desc.storageMode = StorageMode::Shared;
    col_Desc.width = fbWidth;
    col_Desc.height = fbHeight;
    sceneColorTextureID = textureManager.createEmptyNoCPU(col_Desc);
    
    //--------------
    // Recreate volume Texture
    //--------------
    // delete
    Texture* volTex = textureManager.get(volumetricTextureID);
    if(volTex && volTex->uploaded){
        ((MTL::Texture*)volTex->gpuTexture)->release();
        volTex->uploaded = false;
    }
    // create
    TextureDesc vol_Desc;
    vol_Desc.format = TextureFormat::RGBA8Unorm;
    vol_Desc.usage = TextureUsage::RenderTarget | TextureUsage::Sampled;
    vol_Desc.storageMode = StorageMode::Shared;
    vol_Desc.width = fbWidth / volumeDownFact;
    vol_Desc.height = fbHeight / volumeDownFact;
    volumetricTextureID = textureManager.createEmptyNoCPU(vol_Desc);
    
    
    //--------------
    // Recreate ID Texture
    //--------------
    // delete
    Texture* IDTex = textureManager.get(IDTextureID);
    if(IDTex && IDTex->uploaded){
        ((MTL::Texture*)IDTex->gpuTexture)->release();
        IDTex->uploaded = false;
    }
    // create
    TextureDesc ID_Desc;
    ID_Desc.format = TextureFormat::R32Uint;
    ID_Desc.usage = TextureUsage::RenderTarget | TextureUsage::Sampled;
    ID_Desc.storageMode = StorageMode::Shared;
    ID_Desc.width = fbWidth;
    ID_Desc.height = fbHeight;
    IDTextureID = textureManager.createEmptyNoCPU(ID_Desc);
    
    //--------------
    // update Camera aspect
    //--------------
    camera.setAspect(float(fbWidth)/ (float)fbHeight);
    
}
