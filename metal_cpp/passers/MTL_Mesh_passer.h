//
//  MTL_Mesh_passer.h
//  metal_cpp
//
//  Created by Neel on 21/03/26.
//

#pragma once

#include "resource_managers/mesh.h"

void uploadMeshToGPU(Mesh2& mesh, MTL::Device* device);
