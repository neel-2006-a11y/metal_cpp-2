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
    updateNode(&root);
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
    root = *buildSampleSceneNode(rd);
    // sun
    sun.direction = { 1.0f, 0.3f, 1.0f };
    sun.color = { 1.0f, 0.95f, 0.85f };
    sun.intensity = 1.0f;
    sun.cascades = CASCADES;
}
