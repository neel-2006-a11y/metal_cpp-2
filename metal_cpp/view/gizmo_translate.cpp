////
////  gizmo_translate.cpp
////  metal_cpp
////
////  Created by Neel on 17/04/26.
////
//
//#include "view/gizmo_translate.h"
//#include "ImGui/imgui.h"
//
//
//simd::float3 ray_axis_intersect(simd::float3 rayOrigin, simd::float3 rayDir, GizmoHandle handleAxis, simd::float3 axisOrigin){
//    simd::float3 axis = handleAxis == GizmoHandle::AxisX ? simd::float3{1.0, 0.0, 0.0} :
//                        handleAxis == GizmoHandle::AxisY ? simd::float3{0.0, 1.0, 0.0} :
//                        simd::float3{0.0, 0.0, 1.0};
//    
//    float dt = simd::dot(axis, rayDir);
//    float t = simd::dot(axisOrigin - rayOrigin, rayDir * dt - axis) / (1 - dt*dt);
//    
//    return axisOrigin + t * axis;
//}
//
////simd::float3 ray_plane_intersect(simd::float3 rayOrigin, simd::float3 rayDir, GizmoHandle handlePlane, simd::float3 plane_p){
////    
////}
//
//simd::float3 ScreenToWorldRay(float mouseX, float mouseY, float width, float height, simd::float4x4 invVP){
//    float x = 2.0f * (mouseX / width) - 1.0f;
//    float y = 1.0f - (2.0f * mouseY / height);
//    
//    simd::float4 nearPoint = { x, y, 0.0f, 1.0f};
//    simd::float4 farPoint = { x, y, 1.0f, 1.0f};
//    
//    simd::float4 nearWS = invVP * nearPoint;
//    simd::float4 farWS = invVP * farPoint;
//    
//    nearWS /= nearWS.w;
//    farWS /= farWS.w;
//    
//    simd::float3 dir = simd::normalize(farWS.xyz - nearWS.xyz);
//    return dir;
//}
//
//
//void GizmoTranslate::draw_and_update(GizmoInteractState* gizmoState, SceneNode *node, const Camera &camera, float width, float height){
//    if(!node) return;
//    
//    simd::float3 origin = node->worldMatrix.columns[3].xyz;
//    float dist = simd::length(camera.position() - origin);
//    float gizmoScale =  dist * 0.1f;
//    
//    simd::float3 axisX = {1,0,0};
//    simd::float3 axisY = {0,1,0};
//    simd::float3 axisZ = {0,0,1};
//    
//    simd::float3 xEnd = origin + axisX * gizmoScale;
//    simd::float3 yEnd = origin + axisY * gizmoScale;
//    simd::float3 zEnd = origin + axisZ * gizmoScale;
//    
//    simd::float2 pOrigin, pX, pY, pZ;
//    bool bOrigin, bX, bY, bZ;
//    simd::float4x4 VP = camera.viewProjection();
//    bOrigin = project(origin, VP, width, height, pOrigin);
//    bX = project(xEnd, VP, width, height, pX);
//    bY = project(yEnd, VP, width, height, pY);
//    bZ = project(zEnd, VP, width, height, pZ);
//    
//    if(!bOrigin)return;
//    
//    // ------------
//    // draw
//    // ------------
//    auto* draw = ImGui::GetForegroundDrawList();
//    
//    ImU32 colX = (gizmoState->hovered == GizmoHandle::AxisX) ? IM_COL32(255,255,0,255) : IM_COL32(255,0,0,255);
//    ImU32 colY = (gizmoState->hovered == GizmoHandle::AxisY) ? IM_COL32(255,255,0,255) : IM_COL32(0,255,0,255);
//    ImU32 colZ = (gizmoState->hovered == GizmoHandle::AxisZ) ? IM_COL32(255,255,0,255) : IM_COL32(0,0,255,255);
//
//    ImVec2 pOrigin_v = ImVec2(pOrigin.x, pOrigin.y);
//    
//    if (bX){
//        ImVec2 pX_v = ImVec2(pX.x, pX.y);
//        draw->AddLine(pOrigin_v, pX_v, colX, 2.0f);
//    }
//    if(bY){
//        ImVec2 pY_v = ImVec2(pY.x, pY.y);
//        draw->AddLine(pOrigin_v, pY_v, colY, 2.0f);
//    }
//    if(bZ){
//        ImVec2 pZ_v = ImVec2(pZ.x, pZ.y);
//        draw->AddLine(pOrigin_v, pZ_v, colZ, 2.0f);
//    }
//    
//    simd::float3 x_disp = axisX * gizmoScale * 0.5;
//    simd::float3 y_disp = axisY * gizmoScale * 0.5;
//    simd::float3 z_disp = axisZ * gizmoScale * 0.5;
//    
//    simd::float3 xEnd1 = origin + y_disp;
//    simd::float3 xEnd2 = origin + y_disp + z_disp;
//    simd::float3 xEnd3 = origin + z_disp;
//    
//    simd::float3 yEnd1 = origin + z_disp;
//    simd::float3 yEnd2 = origin + z_disp + x_disp;
//    simd::float3 yEnd3 = origin + x_disp;
//    
//    simd::float3 zEnd1 = origin + x_disp;
//    simd::float3 zEnd2 = origin + x_disp + y_disp;
//    simd::float3 zEnd3 = origin + y_disp;
//    
//    simd::float2 pX_p_1, pX_p_2, pX_p_3;
//    simd::float2 pY_p_1, pY_p_2, pY_p_3;
//    simd::float2 pZ_p_1, pZ_p_2, pZ_p_3;
//    bool bX_p_1, bX_p_2, bX_p_3;
//    bool bY_p_1, bY_p_2, bY_p_3;
//    bool bZ_p_1, bZ_p_2, bZ_p_3;
//    
//    bX_p_1 = project(xEnd1, VP, width, height, pX_p_1);
//    bX_p_2 = project(xEnd2, VP, width, height, pX_p_2);
//    bX_p_3 = project(xEnd3, VP, width, height, pX_p_3);
//    
//    bY_p_1 = project(yEnd1, VP, width, height, pY_p_1);
//    bY_p_2 = project(yEnd2, VP, width, height, pY_p_2);
//    bY_p_3 = project(yEnd3, VP, width, height, pY_p_3);
//    
//    bZ_p_1 = project(zEnd1, VP, width, height, pZ_p_1);
//    bZ_p_2 = project(zEnd2, VP, width, height, pZ_p_2);
//    bZ_p_3 = project(zEnd3, VP, width, height, pZ_p_3);
//    
//    if(bOrigin)
//        if(bX_p_1 && bX_p_2 && bX_p_3){
//            ImVec2 pX_p_1_v = ImVec2(pX_p_1.x, pX_p_1.y);
//            ImVec2 pX_p_2_v = ImVec2(pX_p_2.x, pX_p_2.y);
//            ImVec2 pX_p_3_v = ImVec2(pX_p_3.x, pX_p_3.y);
//            
//            
//        }
//    
//    // ------------
//    // Hover detection
//    // ------------
//    auto io = ImGui::GetIO();
//    simd::float2 mouse = {io.MousePos.x, io.MousePos.y};
//    
//    gizmoState->hovered = GizmoHandle::None;
//    
//    float threshold = 10.0f;
//    float dir_threshold = 0.99f;
//    
//    if(bOrigin && bX && distToSegment(mouse, pOrigin, pX) < threshold && abs(simd::dot(axisX, camera.forward())) < dir_threshold)
//        gizmoState->hovered = GizmoHandle::AxisX;
//    if(bOrigin && bY && distToSegment(mouse, pOrigin, pY) < threshold && abs(simd::dot(axisY, camera.forward())) < dir_threshold)
//        gizmoState->hovered = GizmoHandle::AxisY;
//    if(bOrigin && bZ && distToSegment(mouse, pOrigin, pZ) < threshold && abs(simd::dot(axisZ, camera.forward())) < dir_threshold)
//        gizmoState->hovered = GizmoHandle::AxisZ;
//    
//    // ------------
//    // Mouse Handling
//    // ------------
//    bool mouseDown = ImGui::IsMouseDown(0);
//    bool mouseClicked = ImGui::IsMouseClicked(0);
//    
//    if(mouseClicked && gizmoState->hovered != GizmoHandle::None){
//        gizmoState->active = gizmoState->hovered;
//        gizmoState->dragging = true;
//        
//        // if hovered is an Axis
//        if(gizmoState->active == GizmoHandle::AxisX ||
//           gizmoState->active == GizmoHandle::AxisY ||
//           gizmoState->active == GizmoHandle::AxisZ){
//            
//            simd::float3 rayDir = ScreenToWorldRay(mouse.x, mouse.y, width, height, simd::inverse(VP));
//            
//            gizmoState->dragStartHitWS = ray_axis_intersect(
//                camera.position(),
//                rayDir,
//                gizmoState->active,
//                origin
//            );
//            
//        }
//        
//        // if hovered is a Plane
//        if(gizmoState->active == GizmoHandle::PlaneXY ||
//           gizmoState->active == GizmoHandle::PlaneYZ ||
//           gizmoState->active == GizmoHandle::PlaneXZ){
//            
//            simd::float3 rayDir = ScreenToWorldRay(mouse.x, mouse.y, width, height, simd::inverse(VP));
//            
////            gizmoState->dragStartHitWS = ray_plane_intersect(camera.position(), rayDir, gizmoState->active, origin);
//            
//        }
//        
//        gizmoState->startPosLocal = node->localT.position;
//    }
//    
//    if(!mouseDown){
//        gizmoState->dragging = false;
//        gizmoState->active = GizmoHandle::None;
//    }
//    
//    // ------------
//    // Dragging
//    // ------------
//    if(gizmoState->dragging && gizmoState->active != GizmoHandle::None){
//        
//        simd::float3 rayDir = ScreenToWorldRay(mouse.x, mouse.y, width, height, simd::inverse(VP));
//        
//        simd::float3 HitWS = ray_axis_intersect(camera.position(), rayDir, gizmoState->active, origin);
//        
//        simd::float3 worldDelta = (HitWS - gizmoState->dragStartHitWS) * 0.5;
//        // ------------
//        // Convert to LocalSpace
//        // ------------
//        simd::float3 localDelta = worldDelta;
//        
//        if(node->parent){
//            auto parentRot = node->parent->worldRotation;
//            localDelta = ( simd::inverse(parentRot) * simd::float4{worldDelta.x, worldDelta.y, worldDelta.z, 1.0} ).xyz;
//        }
//        
//        node->localT.position = gizmoState->startPosLocal + localDelta;
//    }
//}
//
//
//bool GizmoTranslate::project(const simd::float3& p, const simd::float4x4& VP, float width, float height, simd::float2& out){
//        
//        simd::float4 clip = VP * simd::float4{p.x, p.y, p.z, 1};
//        if(clip.w <= 0.0f) return false;
//        simd::float3 ndc = clip.xyz / clip.w;
//        
//        out = {
//            (ndc.x * 0.5f + 0.5f) * width,
//            (1.0f - (ndc.y * 0.5f + 0.5f)) * height
//        };
//        return true;
//}
//float GizmoTranslate::distToSegment(simd::float2 p, simd::float2 a, simd::float2 b){
//    simd::float2 ab = b - a;
//    float t = simd::dot(p - a, ab) / simd::dot(ab, ab);
//    t = simd::clamp(t, 0.0f, 1.0f);
//
//    simd::float2 closest = a + t * ab;
//    return simd::length(p - closest);
//}
