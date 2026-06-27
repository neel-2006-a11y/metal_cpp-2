//
//  gizmoFactory.h
//  metal_cpp
//
//  Created by Neel on 29/05/26.
//

#pragma once
#include "view/gizmoState.h"
#include "resource_managers/resource_types.h"
#include "resource_managers/mesh_manager.h"

namespace GizmoFactory{
    Gizmo generateTranslateGizmo(MaterialID lineMaterial, MaterialID planeMaterial, MeshManager& meshManager);
}

