//
//  gizmoState.h
//  metal_cpp
//
//  Created by Neel on 14/05/26.
//

#pragma once
#include "config.h"
#include "resource_managers/resource_types.h"

enum class GizmoType {
    Translate,
    Rotate,
    Scale
};

enum class GizmoHandleType {
    AxisX,
    AxisY,
    AxisZ,
    
    PlaneXY,
    PlaneYZ,
    PlaneZX,
    
    Center
};

struct GizmoHandle{
    uint32_t pickID;
    
    GizmoHandleType type;
    
    MeshID visibleMesh;
    MeshID pickMesh;
    
    MaterialID materialID;
    
    simd::float4 color;
    simd::float4x4 localTransform;
};

struct Gizmo {
    GizmoType type;
    
    std::vector<GizmoHandle> handles;
};

struct GizmoInteractionState {
    bool dragging = false;
    
    uint32_t hoveredID = 0;
    uint32_t activeID = 0;
    
    GizmoHandleType activeHandle;
    
    simd::float3 dragStartHitWS;
    simd::float3 dragStartPosLocal;
    
    simd::float3 dragAxis;
    simd::float3 dragPlaneNormal;
};


//enum GizmoHandle{
//    AxisX,
//    AxisY,
//    AxisZ,
//    
//    PlaneXY,
//    PlaneYZ,
//    PlaneXZ,
//    
//    None
//};
//
//struct GizmoInteractState {
//    GizmoHandle hovered = GizmoHandle::None;
//    GizmoHandle active = GizmoHandle::None;
//    
//    bool dragging = false;
//    
//    simd::float3 dragStartHitWS;
//    simd::float3 startPosLocal;
//    
//};


