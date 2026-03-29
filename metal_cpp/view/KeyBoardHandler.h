//
//  KeyBoardHandler.h
//  metal_cpp
//
//  Created by Neel on 28/03/26.
//

#pragma once
#include "config.h"

class KeyBoardHandler{
public:
    KeyBoardHandler(GLFWwindow* window);
    
    void update();
    
    bool isDown(int key);
    bool wasPressed(int key);
    bool wasReleased(int key);
    
private:
    GLFWwindow* window;
    
    std::unordered_map<int, bool> current;
    std::unordered_map<int, bool> previous;
};
