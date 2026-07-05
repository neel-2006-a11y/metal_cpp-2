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
#include "view/gizmo_translate.h"

struct SceneNode;

class ImGuiPass : public RenderPass{
public:
    SceneNode* root = nullptr;
    SceneNode* selected = nullptr;
    
    void init(RenderDevice renderDevice) override;
    void execute(RenderContext renderContext, RenderGraph* renderGraph) override;
    void drawSceneHierarchy(SceneNode* root, SceneNode*& selected);
    void resize(RenderContext renderContext) override;
    
private:
    MTL::RenderCommandEncoder* encoder = nullptr;
    MTL::RenderPassDescriptor* rpDesc = nullptr;
    
    void drawSceneNode(SceneNode* node, SceneNode*& selected);
    void drawInspector(SceneNode* node);
};
