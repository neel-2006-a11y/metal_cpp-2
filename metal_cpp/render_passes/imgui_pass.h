//
//  imgui_pass.h
//  metal_cpp
//
//  Created by Neel on 06/04/26.
//

#pragma once
#include "render_passes/render_pass.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "imgui_impl_metal.h"
#include "scene/SceneNode.h"
#include "view/gizmo_translate.h"

class ImGuiPass : public RenderPass{
public:
    MTL::Texture* drawableTexture;
    
    SceneNode* root;
    SceneNode* selected;
    
    void init() override;
    void execute(RenderContext renderContext) override;
    void drawSceneHierarchy(SceneNode* root, SceneNode*& selected);
    
private:
    MTL::RenderCommandEncoder* encoder = nullptr;
    MTL::RenderPassDescriptor* rpDesc = nullptr;
    
//    GizmoInteractState gizmoState;
//    GizmoTranslate translateGizmo;
    
    void drawSceneNode(SceneNode* node, SceneNode*& selected);
    void drawInspector(SceneNode* node);
};
