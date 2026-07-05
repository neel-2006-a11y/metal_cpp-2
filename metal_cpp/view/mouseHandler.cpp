//
//  mouseHandler.cpp
//  metal_cpp
//
//  Created by Neel on 27/01/26.
//

#include "view/mouseHandler.h"
#include <iostream>

MouseHandler::MouseHandler(GLFWwindow* glfwWindow): glfwWindow(glfwWindow){}


void MouseHandler::update(){
    lastX = currX; lastY = currY;
    
    glfwGetCursorPos(glfwWindow, &currX, &currY);
    
    if(firstMouse){ // first jump
        lastX = currX;
        lastY = currY;
        firstMouse = false;
        return;
    }
}
