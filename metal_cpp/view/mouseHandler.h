//
//  mouseHandler.h
//  metal_cpp
//
//  Created by Neel on 27/01/26.
//

#pragma once

#include "view/camera.h"
#include "GLFW/glfw3.h"

class MouseHandler{
public:
    MouseHandler(GLFWwindow* glfwWindow);
    void updateCamera(Camera* camera);
    
private:
    GLFWwindow* glfwWindow;
    
    double lastX,lastY;
    bool firstMouse = true;
};
