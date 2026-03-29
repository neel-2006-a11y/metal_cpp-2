//
//  KeyBoardHandler.cpp
//  metal_cpp
//
//  Created by Neel on 28/03/26.
//

#include "view/KeyBoardHandler.h"

KeyBoardHandler::KeyBoardHandler(GLFWwindow* window): window(window){}

void KeyBoardHandler::update(){
    previous = current;
    
    int keys[] = {
        GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D,
        GLFW_KEY_Q, GLFW_KEY_E,
        GLFW_KEY_TAB
    };
    
    for(int key : keys){
        current[key] = glfwGetKey(window, key) == GLFW_PRESS;
    }
}

bool KeyBoardHandler::isDown(int key){
    return current[key];
}

bool KeyBoardHandler::wasPressed(int key){
    return current[key] && !previous[key];
}

bool KeyBoardHandler::wasReleased(int key){
    return !current[key] && previous[key];
}
