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
#include "scene/portal.h"


class Scene{
public:
    Scene(int width, int height);
    void updateMatrices();
    
    void setRoot(SceneNode root);
    SceneNode* getRoot();
    Camera& activeCamera();
    DirectionalLight& getSun();
    
    void buildSampleScene(RenderDevice rd);
    void extractRenderables(std::vector<Renderable>& renderables);
    
    std::vector<Portal*> portals;
    
private:
    SceneNode root;
    Camera camera;
    DirectionalLight sun;  
};
