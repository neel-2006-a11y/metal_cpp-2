//
//  mouseHandler.cpp
//  metal_cpp
//
//  Created by Neel on 27/01/26.
//

#include "view/mouseHandler.h"

MouseHandler::MouseHandler(GLFWwindow* glfwWindow): glfwWindow(glfwWindow){}

void MouseHandler::updateCamera(Camera *camera){
    double x,y;
    glfwGetCursorPos(glfwWindow, &x, &y);
    int width, height;
    glfwGetWindowSize(glfwWindow, &width, &height);

    camera->setAspect(float(width)/height);
    
    if(firstMouse){
        lastX = x;
        lastY = y;
        firstMouse = false;
        return;
    }
    
    float dx = float(x - lastX);
    float dy = float(y - lastY);
    lastX = x;
    lastY = y;
    if(camera->relativeMouse){
        camera->processMouse(dx, dy);
    }
}
