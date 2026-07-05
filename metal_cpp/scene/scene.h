//
//  scene.h
//  metal_cpp
//
//  Created by Neel on 04/07/26.
//

#pragma once
#include "scene/SceneNode.h"
#include "view/camera.h"
#include "view/directional_light.h"

class Scene{
public:
    Scene(int width, int height);
    void updateMatrices();
    
    void setRoot(SceneNode root);
    SceneNode* getRoot();
    Camera& activeCamera();
    DirectionalLight& getSun();
    
    void buildSampleScene(RenderDevice rd);
    
private:
    SceneNode root;
    Camera camera;
    DirectionalLight sun;
};
