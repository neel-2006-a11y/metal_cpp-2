////
////  gizmo_translate.h
////  metal_cpp
////
////  Created by Neel on 17/04/26.
////
//
//#pragma once
//
//#include "scene/SceneNode.h"
//#include "view/camera.h"
//#include "view/gizmoState.h"
//
//enum class GizmoAxis{
//    None,
//    X,Y,Z
//};
//
//class GizmoTranslate{
//public:
//    void draw_and_update(GizmoInteractState* gizmo_state, SceneNode* node, const Camera& camera, float width, float height);
//    
//private:
////    GizmoAxis hovered = GizmoAxis::None;
////    GizmoAxis active = GizmoAxis::None;
//    
////    bool dragging = false;
//    
////    simd::float2 dragStartMouse;
////    simd::float3 dragStartLocalPos;
//    
//    bool project(const simd::float3& p, const simd::float4x4& VP, float width, float height, simd::float2& out);
//    float distToSegment(simd::float2 p, simd::float2 a, simd::float2 b);
//};
