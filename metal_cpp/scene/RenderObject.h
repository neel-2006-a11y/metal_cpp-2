//
//  RenderObject.h
//  metal_cpp
//
//  Created by Neel on 23/03/26.
//

#pragma once
#include "config.h"


struct RenderObject
{
    MeshID meshID;
    MaterialID materialID;
    
    ObjectUniforms uniforms;
};

struct Scene
{
    std::vector<RenderObject> objects;
};
