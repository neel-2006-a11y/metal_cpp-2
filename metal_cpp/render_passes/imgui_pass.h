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

class ImGuiPass : public RenderPass{
public:
    MTL::Texture* drawableTexture;
    
    SceneNode* root;
    SceneNode* selected;
    
    void execute(Renderer2& renderer) override;
    void drawSceneHierarchy(SceneNode* root, SceneNode*& selected);
    void release() override;
    
private:
    MTL::RenderCommandEncoder* encoder = nullptr;
    
    void drawSceneNode(SceneNode* node, SceneNode*& selected);
    void drawInspector(SceneNode* node);
};
