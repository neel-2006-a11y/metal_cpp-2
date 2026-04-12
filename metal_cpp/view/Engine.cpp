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

Engine::Engine(Renderer2* renderer) : renderer(renderer), pipelineManager(renderer->device, renderer->device->newDefaultLibrary()), camera(float(800)/600), mouseHandler(renderer->glfwWindow), keyboard(renderer->glfwWindow), modelLoader(&meshManager, &textureManager, &materialManager){
    renderer->textureManager = &textureManager;
    renderer->meshManager = &meshManager;
    renderer->materialManager = &materialManager;
    renderer->pipelineManger = &pipelineManager;
    renderer->scene = &scene;
    renderer->root = &root;
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
    
    
    PipelineDesc shadowDesc;
    shadowDesc.vertexFunc = "shadowVertex";
    shadowDesc.depthFormat = TextureFormat::Depth32Float;
    shadowDesc.vertexLayout = layout;
    
    shadowPipelineID = pipelineManager.createPipeline(shadowDesc);
    
    
    PipelineDesc compDesc;
    compDesc.vertexFunc = "compositeVS";
    compDesc.fragmentFunc = "compositeFS";
    compDesc.colorFormat = TextureFormat::RGBA8Unorm;
    
    compositePipelineID = pipelineManager.createPipeline(compDesc);
    
    
    PipelineDesc volDesc;
    volDesc.vertexFunc = "fullScreenVS";
    volDesc.fragmentFunc = "volumetricFS";
    volDesc.colorFormat = TextureFormat::RGBA8Unorm;
    
    volumetricPipelineID = pipelineManager.createPipeline(volDesc);
    
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
    
    
    
    //--------------
    // load Meshes from Mesh Factory
    //--------------
    vertex_index_pair curtainData = MeshFactory::buildCube2(renderer->device, simd::float3{35,35,5});
    MeshID curtainMesh = meshManager.createMesh(curtainData.vertexData.data(), curtainData.vertexData.size(), curtainData.indexData);
    
    vertex_index_pair cubeData = MeshFactory::buildCube2(renderer->device, simd::float3{5,5,5});
    MeshID cubeMesh = meshManager.createMesh(cubeData.vertexData.data(), cubeData.vertexData.size(), cubeData.indexData);
    
    vertex_index_pair sphereData = MeshFactory::buildSphere2(renderer->device, 16, 16, 1);
    MeshID sphereMesh = meshManager.createMesh(sphereData.vertexData.data(), sphereData.vertexData.size(), sphereData.indexData);
    
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
    
    
    //--------------
    //--------------
    //--------------
    Entity curtainE;
    RenderObject& curtain = curtainE.renderObject;
    curtain.meshID = curtainMesh;
    curtain.materialID = halftoneMaterialID;
    curtain.tileScale = 0.75;
    
    curtainE.transform.position = {0,0,20};
    scene.objects.push_back(curtainE);
    
    Entity cubeE;
    RenderObject& cube = cubeE.renderObject;
    
    cube.meshID = cubeMesh;
    cube.materialID = halftoneMaterialID;
    cube.tileScale = 0.75;
    
    cubeE.transform.position = {-5, 0, 15};
    scene.objects.push_back(cubeE);
    
    
    Entity sphereE;
    RenderObject& sphere = sphereE.renderObject;
    
    sphere.meshID = sphereMesh;
    sphere.materialID = halftoneMaterialID;
    sphere.tileScale = 0.75;
    
    sphereE.transform.position = {5, 0, 10};
    scene.objects.push_back(sphereE);
    
    // cactus
    size_t in_ind = scene.objects.size();
    modelLoader.loadModel( assetDirectory + "/test_cactus/test_cactus.obj", &scene);
    size_t fin_ind = scene.objects.size()-1;
    for(size_t i = in_ind; i<=fin_ind; i++){
        materialManager.setPipeline(scene.objects[i].renderObject.materialID, halftone_pipeID);
        scene.objects[i].renderObject.tileScale = 1.0;
    }
    
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
    renderer->initMaterialBuffer(MAX_MATERIALS);
    
    //--------------
    // ImGui init
    //--------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    
    ImGui_ImplGlfw_InitForOther(renderer->glfwWindow, true);
    ImGui_ImplMetal_Init(renderer->device);
}


void Engine::update(){
    lastTime = currTime;
    currTime = glfwGetTime();
    
    keyboard.update();
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
}

void Engine::render(){
    CA::MetalDrawable* drawable = renderer->metalLayer->nextDrawable();
    if (!drawable) {
        return;
    }
    
    //--------------
    // update Main Pass
    //--------------
    mainPass.depthTexture = depthTextureID;
    mainPass.sceneColorTexture = sceneColorTextureID;
    
    volumetricPass.depthTexture = depthTextureID;
    volumetricPass.volumetricColorTexture = volumetricTextureID;
    
    compositePass.inputColor = sceneColorTextureID;
    compositePass.volumeColor = volumetricTextureID;
    compositePass.drawableTexture = drawable->texture();
    
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
//    renderer->uploadAllObjectUniforms(scene.objects);
    //
    std::vector<ObjectUniforms> objs;
    uint32_t index = 0;
    buildObjectBuffer(&root, objs, index);
    renderer->uploadObjectUniforms(objs);
    
    // material
    auto matUniforms = materialManager.getAllUniforms();
    renderer->uploadAllMaterialUniforms(matUniforms);
    
    //----------
    // Execute
    //----------
    renderGraph.execute(*renderer);
    renderer->cmd->presentDrawable(drawable);
    renderer->cmd->commit();
    renderer->cmd->waitUntilCompleted();
    renderGraph.release();
    
    //----------
    // fetch selected node for future
    //----------
    selected = imGuiPass.selected;
    
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
    // update Camera aspect
    //--------------
    camera.setAspect(float(fbWidth)/ (float)fbHeight);
    
}
