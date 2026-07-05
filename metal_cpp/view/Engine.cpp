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

Engine::Engine(GLFWwindow* window) : mouseHandler(window), keyboard(window){
    this->window = window;
}

void Engine::init(){
    currTime = glfwGetTime();
    lastTime = glfwGetTime();
}


void Engine::update(Scene& scene){
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
    
    lastTime = currTime;
    currTime = glfwGetTime();
    
    int width,height;
    glfwGetWindowSize(window, &width, &height);
    scene.activeCamera().setAspect(float(width) / float(height));
    
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
        fluid.addDensity(gridX, gridY, 10, simd::float4{5.0,0.0,0.0,0.0});
    }

    if(keyboard.isDown(GLFW_KEY_J)){
        int gridX = (mouseHandler.currX / curr_window_width) * (fluid.N+2);
        int gridY= (mouseHandler.currY / curr_window_height) * (fluid.N+2);

        gridX = std::max(0,std::min(fluid.N+1, gridX));
        gridY = std::max(0,std::min(fluid.N+1, gridY));
        fluid.addDensity(gridX, gridY, 10, simd::float4{0.0,5.0,0.0,0.0});
    }

    if(keyboard.isDown(GLFW_KEY_L)){
        int gridX = (mouseHandler.currX / curr_window_width) * (fluid.N+2);
        int gridY= (mouseHandler.currY / curr_window_height) * (fluid.N+2);
        double dX = mouseHandler.currX - mouseHandler.lastX, dY = mouseHandler.currY - mouseHandler.lastY;
        double power = 10;

        fluid.addVelocity(gridX, gridY, 5, dX * power, dY * power);
    }
    
}
