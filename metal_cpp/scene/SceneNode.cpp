//
//  SceneNode.cpp
//  metal_cpp
//
//  Created by Neel on 11/04/26.
//

#include "scene/SceneNode.h"

void Node::assignParent(SceneNode* child, SceneNode* parent){
    if(!child) return;
    child->parent = parent;
    
    if(parent){
        parent->children.push_back(child);
    }
}

SceneNode* Node::createNode(SceneNode* parent){
    SceneNode* node = new SceneNode();
    node->parent = parent;
    if(parent) parent->children.push_back(node);
    return node;
}

SceneNode* Node::findNode(SceneNode* parent, std::string name){
    if(parent && parent->name == name){
        return parent;
    }
    for(auto child : parent->children){
        SceneNode* node = findNode(child, name);
        if(node) return node;
    }
    return nullptr;
}

void Node::updateNode(SceneNode* node){ // updates rotation and full matrices of node and its children
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

void Node::extractRenderables(SceneNode* node, std::vector<Renderable>& renderables){
    if(!node->visible) return;
    
    if(node->renderComponent){
        Renderable r;
        r.renderComponent = node->renderComponent;
        r.objectUniforms.model = node->worldMatrix;
        r.objectUniforms.invModel = simd_inverse(r.objectUniforms.model);
        
        renderables.push_back(std::move(r));
    }
    
    for(auto child : node->children){
        Node::extractRenderables(child, renderables);
    }
}

void Node::assignPipelineID(SceneNode* node, PipelineID pipeID, MaterialManager* matMgr){
    if(node->renderComponent){
        matMgr->setPipeline(node->renderComponent->materialID, pipeID);
    }
    for(auto* child : node->children){
        Node::assignPipelineID(child, pipeID, matMgr);
    }
}

void Node::copyMaterialBindings(SceneNode* node, MaterialID matID, MaterialManager* matMgr){
    if(node->renderComponent){
        auto mat = matMgr->get(matID);
        MaterialID this_matID = node->renderComponent->materialID;
        for(auto texBinding : mat->textures){
            matMgr->setTexture(this_matID, texBinding.slot, texBinding.texture);
        }
        for(auto samplerBinding : mat->samplers){
            matMgr->setSampler(this_matID, samplerBinding.slot, samplerBinding.sampler);
        }
    }
    for(auto* child : node->children){
        Node::copyMaterialBindings(child, matID, matMgr);
    }
}

void Node::assignMaterialBuffer(SceneNode* node, MaterialUniforms MU, MaterialManager* matMgr){
    if(node->renderComponent){
        matMgr->setBuffer(node->renderComponent->materialID, MU);
    }
    for(auto* child : node->children){
        Node::assignMaterialBuffer(child, MU, matMgr);
    }
}

SceneNode* Node::buildSampleSceneNode(RenderDevice renderDevice){

    
    VertexLayout layout;
    layout.stride = sizeof(Vertex3D);
    layout.attributes = {
        {0,0, offsetof(Vertex3D, position), VertexAttributeType::Float3},
        {0,1, offsetof(Vertex3D, color), VertexAttributeType::Float3},
        {0,2, offsetof(Vertex3D, uv), VertexAttributeType::Float2},
        {0,3, offsetof(Vertex3D, normal), VertexAttributeType::Float3}
    };
    
    //--------------
    // Create materials
    //--------------
    //--------------
    // halftone_material
    //--------------
    
    PipelineDesc halftone_pipe_desc;
    halftone_pipe_desc.vertexFunc = "vertexMain3D_dither_2";
    halftone_pipe_desc.fragmentFunc = "fragmentMain3D_dither_2";
    halftone_pipe_desc.colorFormat = TextureFormat::RGBA8Unorm;
    halftone_pipe_desc.normalFormat = TextureFormat::RGBA16Float;
    halftone_pipe_desc.depthFormat = TextureFormat::Depth32Float;
    halftone_pipe_desc.vertexLayout = layout;
    
    auto halftone_pipeID = renderDevice.pipelineManager->createPipeline(halftone_pipe_desc);
    
    //--------------
    // Load Halftone Texture
    //--------------
    std::vector<std::string> filenames;
    int dim = HALFTONE_DIM;
    for (int i=0; i<dim; i++) {
        std::string filename = "halftone_textures/texture" + std::to_string(i) + ".ppm";
        filenames.push_back(filename);
    }
    
    TextureDesc halftone_desc;
    if(HALFTONE_TEXTURE_16BIT)
        halftone_desc.format = TextureFormat::R16Unorm;
    else
        halftone_desc.format = TextureFormat::R8Unorm;
    
    halftone_desc.textureType = TextureType::TextureType2DArray;
    halftone_desc.usage = TextureUsage::Sampled;
    halftone_desc.storageMode = StorageMode::Shared;
    auto halftone_textureID = renderDevice.textureManager->loadPPMArray(filenames, halftone_desc);
    
    auto halftoneMaterialID = renderDevice.materialManager->createMaterial(halftone_pipeID);
    // bind texture (1)
    renderDevice.materialManager->setTexture(halftoneMaterialID, 1, halftone_textureID);
    
    //--------------
    // Make Sampler for HalftoneTexture
    //--------------
    SamplerDesc halftone_samp_desc; // default sampler
    auto halftone_sampler = renderDevice.samplerManager->createSampler(halftone_samp_desc);
    // bind sampler(1)
    renderDevice.materialManager->setSampler(halftoneMaterialID, 1, halftone_sampler);
    
    //--------------
    // Make MaterialUniforms ( depends on the shader for material )
    //--------------
    MaterialUniformsBuilder halftone_uniforms_builder;
    int has_diffuse_false = 0; // no diffuse texture
    halftone_uniforms_builder.add(has_diffuse_false);
    halftone_uniforms_builder.add(dim);
    MaterialUniforms halftoneMU = halftone_uniforms_builder.get_buffer();
    renderDevice.materialManager->setBuffer(halftoneMaterialID, halftoneMU);
    
    
    //--------------
    // Hatch Material
    //--------------
    PipelineDesc hatch_pipe_desc;
    hatch_pipe_desc.vertexFunc = "vertexMain3D_hatch";
    hatch_pipe_desc.fragmentFunc = "fragmentMain3D_hatch";
    hatch_pipe_desc.colorFormat = TextureFormat::RGBA8Unorm;
    hatch_pipe_desc.normalFormat = TextureFormat::RGBA16Float;
    hatch_pipe_desc.depthFormat = TextureFormat::Depth32Float;
    hatch_pipe_desc.vertexLayout = layout;
    
    auto hatch_pipeID = renderDevice.pipelineManager->createPipeline(hatch_pipe_desc);
    
    //--------------
    // Load Hatch Texture
    //--------------
    std::vector<std::string> filenames_hatch;
    int hatch_dim = HATCH_DIM;
    std::cout << hatch_dim << std::endl;
    for (int i=0; i<hatch_dim; i++) {
        std::string filename = "hatch_textures/texture" + std::to_string(i) + ".ppm";
        filenames_hatch.push_back(filename);
    }
    
    TextureDesc hatch_desc;
    if(HALFTONE_TEXTURE_16BIT)
        hatch_desc.format = TextureFormat::R16Unorm;
    else
        hatch_desc.format = TextureFormat::R8Unorm;
    
    hatch_desc.textureType = TextureType::TextureType2DArray;
    hatch_desc.usage = TextureUsage::Sampled;
    hatch_desc.storageMode = StorageMode::Shared;
    auto hatch_TextureID = renderDevice.textureManager->loadPPMArray(filenames_hatch, hatch_desc);
    
    auto hatchMaterialID = renderDevice.materialManager->createMaterial(hatch_pipeID);
    // bind texture (1)
    renderDevice.materialManager->setTexture(hatchMaterialID, 1, hatch_TextureID);
    
    //--------------
    // Make Sampler for HatchTexture
    //--------------
    // bind sampler(1)
    renderDevice.materialManager->setSampler(hatchMaterialID, 1, halftone_sampler);
    
    //--------------
    // Make MaterialUniforms ( depends on the shader for material )
    //--------------
    MaterialUniformsBuilder hatch_uniforms_builder;
    float scale = 0.02;

    float dir_angles[7] = {0, M_PI/2, M_PI/4, 3*M_PI/8, M_PI/8, 3*M_PI/4, -3*M_PI/4};
//    int num_dirs = 7;
//    int num_dirs = 5;
    int num_dirs = 3;
    float main_dir = 0;
    hatch_uniforms_builder.add(scale);
    hatch_uniforms_builder.add(num_dirs);
    hatch_uniforms_builder.add_pointer(dir_angles, 5 * sizeof(float));
    hatch_uniforms_builder.add(main_dir);
    
    hatch_uniforms_builder.add(has_diffuse_false);
    hatch_uniforms_builder.add(hatch_dim);
    MaterialUniforms hatchMU = hatch_uniforms_builder.get_buffer();
    renderDevice.materialManager->setBuffer(hatchMaterialID, hatchMU);
    
    
    
    
    //--------------
    // load Meshes from Mesh Factory
    //--------------
    vertex_index_pair curtainData = MeshFactory::buildCube2(simd::float3{35,35,5});
    MeshID curtainMesh = renderDevice.meshManager->createMesh(curtainData);
    
    vertex_index_pair cubeData = MeshFactory::buildCube2(simd::float3{5,5,5});
    MeshID cubeMesh = renderDevice.meshManager->createMesh(cubeData);
    
    vertex_index_pair sphereData = MeshFactory::buildSphere2(32, 32, 5);
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
    SceneNode* curtainNode = Node::createNode(root);
    curtainNode->name = "Curtain";

    curtainNode->renderComponent = new RenderComponent();
    curtainNode->renderComponent->meshID = curtainMesh;
    curtainNode->renderComponent->materialID = hatchMaterialID;

    curtainNode->localT.position = {0, 0, 20};
    
    //--------------
    SceneNode* cubeNode = Node::createNode(root);
    cubeNode->name = "Cube";

    cubeNode->renderComponent = new RenderComponent();
    cubeNode->renderComponent->meshID = cubeMesh;
    cubeNode->renderComponent->materialID = hatchMaterialID;

    cubeNode->localT.position = {-5, 0, 15};
    cubeNode->localT.scale = {1.0,1.0,1.0};
    
    //--------------
    SceneNode* sphereNode = Node::createNode(root);
    sphereNode->name = "Sphere";
    
    sphereNode->renderComponent = new RenderComponent();
    sphereNode->renderComponent->meshID = sphereMesh;
    sphereNode->renderComponent->materialID = hatchMaterialID;
    
    sphereNode->localT.position = {5, 0, 15};
    
    // asset directory
    std::string assetDirectory = "/Users/admin/Desktop/metal_cpp-2/metal_cpp/assets";
    //--------------
    SceneNode* cactusNode = renderDevice.modelLoader->loadModelAsNode(assetDirectory + "/test_cactus/test_cactus.obj");
    assignParent(cactusNode, root);
    assignPipelineID(cactusNode, halftone_pipeID, renderDevice.materialManager);
    copyMaterialBindings(cactusNode, halftoneMaterialID, renderDevice.materialManager);
    
    // assign material buffer with has_diffuse = 1
    MaterialUniformsBuilder diff_mu_builder;
    int has_diffuse_true = 1;
    diff_mu_builder.add(has_diffuse_true);
    diff_mu_builder.add(dim);
    MaterialUniforms diff_MU = diff_mu_builder.get_buffer();
    assignMaterialBuffer(cactusNode, diff_MU, renderDevice.materialManager);
    
    return root;
}
