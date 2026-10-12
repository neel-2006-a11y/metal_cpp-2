//
//  scene.cpp
//  metal_cpp
//
//  Created by Neel on 04/07/26.
//

#include "scene/scene.h"

Scene::Scene(int width, int height) : camera(float(width)/height){
}

void Scene::updateMatrices(){
    Node::updateNode(&root);
}

void Scene::setRoot(SceneNode root){
    this->root = root;
}

SceneNode* Scene::getRoot(){
    return &root;
}

Camera& Scene::activeCamera(){
    return camera;
}

DirectionalLight& Scene::getSun(){
    return sun;
}

void Scene::buildSampleScene(RenderDevice rd){
    root = *Node::buildSampleSceneNode(rd);
    // sun
    sun.direction = { 1.0f, 0.3f, 1.0f };
    sun.color = { 1.0f, 0.95f, 0.85f };
    sun.intensity = 1.0f;
    sun.cascades = CASCADES;
    
    // portals
    Transform t1;
    t1.position = simd::float3{0,0,-10};
    
    Transform t2;
    t2.position = simd::float3{0,0,-5};
    t2.rotation = simd_quaternion((float)M_PI, simd_make_float3(0.0f, 1.0f, 0.0f));
    
    Portal* portal1 = new Portal(5,5, t1);
    Portal* portal2 = new Portal(5,5, t2);
    
    portal1->assign_other_portal(portal2);
    portal2->assign_other_portal(portal1);
    
    this->portals.push_back(portal1);
    this->portals.push_back(portal2);
    
    
    VertexLayout layout;
    layout.stride = sizeof(Vertex3D);
    layout.attributes = {
        {0,0, offsetof(Vertex3D, position), VertexAttributeType::Float3},
        {0,1, offsetof(Vertex3D, color), VertexAttributeType::Float3},
        {0,2, offsetof(Vertex3D, uv), VertexAttributeType::Float2},
        {0,3, offsetof(Vertex3D, normal), VertexAttributeType::Float3}
    };
    
    PipelineDesc portal_pipe_desc;
    portal_pipe_desc.vertexFunc = "vertexMain3D_portal";
    portal_pipe_desc.fragmentFunc = "fragmentMain3D_portal";
    portal_pipe_desc.colorFormat = TextureFormat::RGBA8Unorm;
    portal_pipe_desc.normalFormat = TextureFormat::RGBA16Float;
    portal_pipe_desc.depthFormat = TextureFormat::Depth32Float;
    portal_pipe_desc.vertexLayout = layout;
    
    PipelineID portal_pipeID = rd.pipelineManager->createPipeline(portal_pipe_desc);
    
    SamplerDesc portal_samp_desc; // default sampler
    auto portal_sampler = rd.samplerManager->createSampler(portal_samp_desc);
    
    for(auto p : portals){
        p->assign_render_materials(rd.meshManager, rd.materialManager, portal_pipeID, portal_sampler);
    }
}

void Scene::extractRenderables(std::vector<Renderable> &renderables){
    Node::extractRenderables(&root, renderables);
    for(auto p : portals){
        Renderable r;
        r.objectUniforms.model = p->transform.computeModel();
        r.objectUniforms.invModel = simd_inverse(r.objectUniforms.model);
        
        r.renderComponent = new RenderComponent;
        r.renderComponent->materialID = p->materialID;
        r.renderComponent->meshID = p->meshID;
        renderables.push_back(r);
    }
}

