//
//  gizmoFactory.cpp
//  metal_cpp
//
//  Created by Neel on 29/05/26.
//

#include "view/gizmoFactory.h"
#include "view/mesh_factory.h"
#include "backend/mtlm.h"


//struct GizmoHandle{
//    uint32_t pickID;
//    
//    GizmoHandleType type;
//    
//    MeshID visibleMesh;
//    MeshID pickMesh;
//    
//    MaterialID materialID;
//    
//    simd::float4 color;
//    simd::float4x4 localTransform;
//};

Gizmo GizmoFactory::generateTranslateGizmo(MaterialID lineMaterial, MaterialID planeMaterial, MeshManager& meshManager){
    Gizmo gizmo;
    gizmo.type = GizmoType::Translate;
    
    // X_Handle
    GizmoHandle XHandle;
    XHandle.pickID = 1;
    XHandle.type = GizmoHandleType::AxisX;
    
    vertex_index_pair XHandle_visibleMesh = MeshFactory::buildGizmoLine(simd::float3{0,0,0}, simd::float3{1,0,0});
    XHandle.visibleMesh = meshManager.createMesh(XHandle_visibleMesh);
    
    XHandle.materialID = lineMaterial;
    
    XHandle.color = simd::float4{1.0,0.0,0.0,1.0};
    
    XHandle.localTransform = mtlm::identity();
    
    gizmo.handles.push_back(XHandle);
    
    // Y_Handle
    GizmoHandle YHandle;
    YHandle.pickID = 2;
    YHandle.type = GizmoHandleType::AxisY;
    
    vertex_index_pair YHandle_visibleMesh = MeshFactory::buildGizmoLine(simd::float3{0,0,0}, simd::float3{0,1,0});
    YHandle.visibleMesh = meshManager.createMesh(YHandle_visibleMesh);
    
    YHandle.materialID = lineMaterial;
    
    YHandle.color = simd::float4{0.0,1.0,0.0,1.0};
    
    YHandle.localTransform = mtlm::identity();
    
    gizmo.handles.push_back(YHandle);
    
    // Z_Handle
    GizmoHandle ZHandle;
    ZHandle.pickID = 1;
    ZHandle.type = GizmoHandleType::AxisZ;
    
    vertex_index_pair ZHandle_visibleMesh = MeshFactory::buildGizmoLine(simd::float3{0,0,0}, simd::float3{0,0,1});
    ZHandle.visibleMesh = meshManager.createMesh(ZHandle_visibleMesh);
    
    ZHandle.materialID = lineMaterial;
    
    ZHandle.color = simd::float4{0.0,0.0,1.0,1.0};
    
    ZHandle.localTransform = mtlm::identity();
    
    gizmo.handles.push_back(ZHandle);
    
    
    return gizmo;
}
