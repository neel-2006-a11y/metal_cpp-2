//
//  SceneNode.cpp
//  metal_cpp
//
//  Created by Neel on 11/04/26.
//

#include "scene/SceneNode.h"

void assignParent(SceneNode* child, SceneNode* parent){
    if(!child) return;
    child->parent = parent;
    
    if(parent){
        parent->children.push_back(child);
    }
}

SceneNode* createNode(SceneNode* parent){
    SceneNode* node = new SceneNode();
    node->parent = parent;
    if(parent) parent->children.push_back(node);
    return node;
}

void updateNode(SceneNode* node){ // updates rotation and full matrices of node and its children
    node->worldRotation = simd_matrix4x4(node->localT.rotation);
    node->worldMatrix = node->localT.computeModel();
    if(node->parent){
        node->worldRotation = node->parent->worldRotation * node->worldRotation;
        node->worldMatrix = node->parent->worldMatrix * node->worldMatrix;
    }
    for(auto child : node->children){
        updateNode(child);
    }
}

void extractRenderables(SceneNode* node, std::vector<Renderable>& renderables){
    if(!node->visible) return;
    
    if(node->renderComponent){
        Renderable r;
        r.renderComponent = node->renderComponent;
        r.objectUniforms.model = node->worldMatrix;
        r.objectUniforms.invModel = simd_inverse(r.objectUniforms.model);
        r.objectUniforms.tileScale = node->renderComponent->tileScale;
        
        renderables.push_back(std::move(r));
    }
    
    for(auto child : node->children){
        extractRenderables(child, renderables);
    }
}

void assignPipelineID(SceneNode* node, PipelineID pipeID, MaterialManager* matMgr){
    if(node->renderComponent){
        matMgr->setPipeline(node->renderComponent->materialID, pipeID);
    }
    for(auto* child : node->children){
        assignPipelineID(child, pipeID, matMgr);
    }
}

SceneNode* buildSampleSceneNode(RenderDevice renderDevice){

    //--------------
    // Create pipelines for material
    //--------------
    VertexLayout layout;
    layout.stride = sizeof(Vertex3D);
    layout.attributes = {
        {0, offsetof(Vertex3D, position), VertexAttributeType::Float3},
        {1, offsetof(Vertex3D, color), VertexAttributeType::Float3},
        {2, offsetof(Vertex3D, uv), VertexAttributeType::Float2},
        {3, offsetof(Vertex3D, normal), VertexAttributeType::Float3}
    };
    
    PipelineDesc mainDesc;
    mainDesc.vertexFunc = "vertexMain3D_dither_2";
    mainDesc.fragmentFunc = "fragmentMain3D_dither_2";
    mainDesc.colorFormat = TextureFormat::RGBA8Unorm;
    mainDesc.depthFormat = TextureFormat::Depth32Float;
    mainDesc.vertexLayout = layout;
    
    auto halftone_pipeID = renderDevice.pipelineManager->createPipeline(mainDesc);
    
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
    auto halftone_textureID = renderDevice.textureManager->loadPPMArray(filenames, halftone_desc);
    
    auto halftoneMaterialID = renderDevice.materialManager->createMaterial(halftone_pipeID);
    renderDevice.materialManager->setTexture(halftoneMaterialID, 1, halftone_textureID);
    
    //--------------
    // Make Sampler for HalftoneTexture
    //--------------
    SamplerDesc halftone_samp_desc; // default sampler
    auto halftone_sampler = renderDevice.samplerManager->createSampler(halftone_samp_desc);
    renderDevice.materialManager->setSampler(halftoneMaterialID, 1, halftone_sampler);
    
    //--------------
    // load Meshes from Mesh Factory
    //--------------
    vertex_index_pair curtainData = MeshFactory::buildCube2(simd::float3{35,35,5});
    MeshID curtainMesh = renderDevice.meshManager->createMesh(curtainData);
    
    vertex_index_pair cubeData = MeshFactory::buildCube2(simd::float3{5,5,5});
    MeshID cubeMesh = renderDevice.meshManager->createMesh(cubeData);
    
    vertex_index_pair sphereData = MeshFactory::buildSphere2(16, 16, 1);
    MeshID sphereMesh = renderDevice.meshManager->createMesh(sphereData);
    
    
    //--------------
    // setup Scene
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
    SceneNode* root = new SceneNode();
    root->name = "Root";
    
    //--------------
    SceneNode* curtainNode = createNode(root);
    curtainNode->name = "Curtain";

    curtainNode->renderComponent = new RenderComponent();
    curtainNode->renderComponent->meshID = curtainMesh;
    curtainNode->renderComponent->materialID = halftoneMaterialID;
    curtainNode->renderComponent->tileScale = 0.75;

    curtainNode->localT.position = {0, 0, 20};
    
    //--------------
    SceneNode* cubeNode = createNode(root);
    cubeNode->name = "Cube";

    cubeNode->renderComponent = new RenderComponent();
    cubeNode->renderComponent->meshID = cubeMesh;
    cubeNode->renderComponent->materialID = halftoneMaterialID;
    cubeNode->renderComponent->tileScale = 0.75;

    cubeNode->localT.position = {-5, 0, 15};
    cubeNode->localT.scale = {2.0,1.0,1.0};
    
    //--------------
    SceneNode* sphereNode = createNode(root);
    sphereNode->name = "Sphere";
    
    sphereNode->renderComponent = new RenderComponent();
    sphereNode->renderComponent->meshID = sphereMesh;
    sphereNode->renderComponent->materialID = halftoneMaterialID;
    sphereNode->renderComponent->tileScale = 0.75;
    
    sphereNode->localT.position = {5, 0, 15};
    
    // asset directory
    std::string assetDirectory = "/Users/admin/Desktop/metal_cpp-2/metal_cpp/assets";
    //--------------
    SceneNode* cactusNode = renderDevice.modelLoader->loadModelAsNode(assetDirectory + "/test_cactus/test_cactus.obj");
    assignParent(cactusNode, root);
    assignPipelineID(cactusNode, halftone_pipeID, renderDevice.materialManager);
    
    return root;
}
