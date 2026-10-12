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
#include "view/renderer2.h"
#include "view/renderView.h"

Engine::Engine(GLFWwindow* window) : mouseHandler(window), keyboard(window){
    this->window = window;
}

void Engine::init(){
    currTime = glfwGetTime();
    lastTime = glfwGetTime();
}

void Engine::get_references(RenderDevice rd){
    portal_texture_pool.assign_texManager(rd.textureManager);
}

void Engine::update(Scene& scene){
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
    
    lastTime = currTime;
    currTime = glfwGetTime();
    
    // -------------
    // any other updates on nodes
    // -------------
//    SceneNode* sphereNode = Node::findNode(scene.getRoot(), "Sphere");
//    sphereNode->localT.position.x += 0.01;
//    simd_quatf nudge = simd_quaternion(0.0f,1.0f,0.0f,(float)((currTime-lastTime) * 0.06));
//    sphereNode->localT.rotation = simd_mul(sphereNode->localT.rotation, nudge);
    
//    SceneNode* sphereNode = findNode(scene.getRoot(), "Cube");
//    simd_quatf nudge = simd_quaternion(0.0f,1.0f,0.0f,(float)((currTime-lastTime) * 0.06));
//    sphereNode->localT.rotation = simd_mul(sphereNode->localT.rotation, nudge);
    
    // -------------
    
    int width,height;
    glfwGetWindowSize(window, &width, &height);
    scene.activeCamera().setAspect(float(width) / float(height));
//    scene.activeCamera().setAspect(1);
    
    keyboard.update();
    mouseHandler.update();
    
    // handle input
    handleKeyboard(&scene.activeCamera());
    handleMouse(&scene.activeCamera());
    handleFluidInput();
    
    scene.getSun().update(&scene.activeCamera());
    
    scene.updateMatrices(); // applies the transform changes to matrices of nodes
    
    makeFrameData(scene);
    
    pool->release();
}

void Engine::make_renderViews(Scene& scene, std::vector<RenderView>& views){
    portal_cameras.clear();
    views.clear();
    
    RenderView rv;
    rv.camera = &scene.activeCamera();
    rv.useDrawable = true;
    
    std::vector<TextureID> child_portal_textures;
    for(auto p : scene.portals){
        TextureID p_RT_id = renderView_portal(p, rv.camera, 1, scene, views);
        
        MaterialOverride mat_override;
        
        mat_override.mat = p->materialID;
        
        mat_override.bindings.push_back({
            .slot = DIFFUSE_SLOT,
            .texture = p_RT_id
        });
        rv.mat_overrides.push_back(mat_override);
        child_portal_textures.push_back(p_RT_id);
    }
    
    for(auto c : child_portal_textures)portal_texture_pool.release(c);
    views.push_back(rv);
}

TextureID Engine::renderView_portal(Portal *p, Camera *real_camera, int depth, Scene& scene, std::vector<RenderView>& views){
    if(depth > PORTAL_DEPTH){
        return INVALID_TEXTURE;
    }
    RenderView rv;
    rv.renderTarget = portal_texture_pool.aquire();
    
    auto virtual_cam = p->create_virtual(real_camera);
    portal_cameras.push_back(virtual_cam); // lifetime till renderer renders
    rv.camera = virtual_cam;
    
    std::vector<TextureID> child_portal_textures;
    for(auto p : scene.portals){
        TextureID p_RT_id = renderView_portal(p, rv.camera, depth+1, scene, views);
        
        MaterialOverride mat_override;
        
        mat_override.mat = p->materialID;
        
        mat_override.bindings.push_back({
            .slot = DIFFUSE_SLOT,
            .texture = p_RT_id
        });
        rv.mat_overrides.push_back(mat_override);
        
        child_portal_textures.push_back(p_RT_id);
    }
    for(auto c : child_portal_textures)portal_texture_pool.release(c);
    views.push_back(rv);
    return rv.renderTarget;
}

void Engine::makeFrameData(Scene &scene){
    frameData.camera = &scene.activeCamera();
    frameData.sun = &scene.getSun();
    frameData.dt = 0.016; // temporary
    frameData.time = 0;
}

void Engine::publish(ResourceRegistry &resourceRegistry, FrameDataRegistry &frameDataRegistry){
    frameDataRegistry.publish<FrameInfo>(std::move(frameData));
    fluid.publish(resourceRegistry, frameDataRegistry);
}

void Engine::handleKeyboard(Camera* camera){
    
    // camera handling
    if(keyboard.wasPressed(GLFW_KEY_TAB)){
        camera->relativeMouse = !camera->relativeMouse;
        
        if(camera->relativeMouse){
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }else{
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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
    camera->processKeyboard(dx, dy, dz, deltaTime);
    
}

void Engine::handleMouse(Camera *camera){
    
    if(camera->relativeMouse){
        float dx = float(mouseHandler.currX - mouseHandler.lastX);
        float dy = float(mouseHandler.currY - mouseHandler.lastY);
        camera->processMouse(dx, dy);
    }
}

void Engine::handleFluidInput(){
    // fluid interaction
    
    int curr_window_width,curr_window_height;
    glfwGetWindowSize(window, &curr_window_width, &curr_window_height);
    
    if(keyboard.isDown(GLFW_KEY_K)){
        int gridX = (mouseHandler.currX / curr_window_width) * (fluid.N+2);
        int gridY= (mouseHandler.currY / curr_window_height) * (fluid.N+2);

        gridX = std::max(0,std::min(fluid.N+1, gridX));
        gridY = std::max(0,std::min(fluid.N+1, gridY));
        fluid.addDensity(gridX, gridY, fluid.N/10, simd::float4{5.0,0.0,0.0,0.0});
    }

    if(keyboard.isDown(GLFW_KEY_J)){
        int gridX = (mouseHandler.currX / curr_window_width) * (fluid.N+2);
        int gridY= (mouseHandler.currY / curr_window_height) * (fluid.N+2);

        gridX = std::max(0,std::min(fluid.N+1, gridX));
        gridY = std::max(0,std::min(fluid.N+1, gridY));
        fluid.addDensity(gridX, gridY, fluid.N/10, simd::float4{0.0,5.0,0.0,0.0});
    }

    if(keyboard.isDown(GLFW_KEY_L)){
        int gridX = (mouseHandler.currX / curr_window_width) * (fluid.N+2);
        int gridY= (mouseHandler.currY / curr_window_height) * (fluid.N+2);
        double dX = mouseHandler.currX - mouseHandler.lastX, dY = mouseHandler.currY - mouseHandler.lastY;
        double power = 10;

        fluid.addVelocity(gridX, gridY, fluid.N/20, dX * power, dY * power);
    }
    
}
