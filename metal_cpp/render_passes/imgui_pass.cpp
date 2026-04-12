//
//  imgui_pass.cpp
//  metal_cpp
//
//  Created by Neel on 06/04/26.
//

#include "render_passes/imgui_pass.h"

void ImGuiPass::execute(Renderer2 &renderer){
    MTL::RenderPassDescriptor* rpDesc = MTL::RenderPassDescriptor::alloc()->init();
    
    rpDesc->colorAttachments()->object(0)->setTexture(drawableTexture);
    rpDesc->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionLoad);
    rpDesc->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
    
    // ------------
    // Start frame
    // ------------
    ImGui_ImplMetal_NewFrame(rpDesc);
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // ------------
    // UI
    // ------------
    auto& dbg = renderer.debug;
    ImGui::Begin("Debug");
    
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    
    ImGui::SliderFloat3("Sun Dir", (float*)&dbg.sunDirection, -1.0, 1.0);
    
    ImGui::SliderFloat("Bayer Scale", &dbg.BayerScale, 0.0, 0.1);
    
    ImGui::SliderFloat("Fog Density", &dbg.volumeDensity, 0.0, 0.2f);
    
    ImGui::SliderFloat("g", &dbg.g, -1.0, 2.0);
    ImGui::End();
    
    // ------------
    // Scene Graph
    // ------------
    if(root){
        drawSceneHierarchy(root, selected);
    }
    drawInspector(selected);
    
    encoder = renderer.cmd->renderCommandEncoder(rpDesc);
    
    ImGui::Render();
    ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), renderer.cmd, encoder);
    encoder->endEncoding();
    rpDesc->release();
}

void ImGuiPass::release(){
    if(encoder){
        encoder->release();
        encoder = nullptr;
    }
}

void ImGuiPass::drawSceneHierarchy(SceneNode *root, SceneNode *&selected){
    ImGui::Begin("Scene");
    
    if(root){
        drawSceneNode(root, selected);
    }
    
    ImGui::End();
}


void ImGuiPass::drawSceneNode(SceneNode *node, SceneNode *&selected){
    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_SpanAvailWidth;

    if (node == selected)
        flags |= ImGuiTreeNodeFlags_Selected;

    bool opened = ImGui::TreeNodeEx(
        (void*)node,
        flags,
        "%s",
        node->name.c_str()
    );

    // Click selection
    if (ImGui::IsItemClicked()) {
        selected = node;
    }

    if (opened) {
        for (auto* child : node->children) {
            drawSceneNode(child, selected);
        }
        ImGui::TreePop();
    }
}

void ImGuiPass::drawInspector(SceneNode* node)
{
    ImGui::Begin("Inspector");

    if (node) {
        ImGui::Text("Name: %s", node->name.c_str());

        // Position
        ImGui::DragFloat3("Position", (float*)&node->localT.position, 0.1f);

        // Scale
        ImGui::DragFloat3("Scale", (float*)&node->localT.scale, 0.1f);

        // Rotation (quaternion)
        ImGui::DragFloat4("Rotation (quat)",
            (float*)&node->localT.rotation, 0.01f);

        if (node->renderObject) {
            ImGui::Separator();
            ImGui::Text("Renderer");

            ImGui::Text("MeshID: %d", node->renderObject->meshID);
            ImGui::Text("MaterialID: %d", node->renderObject->materialID);

            ImGui::DragFloat("TileScale",
                (float*)&node->renderObject->tileScale, 0.1f);
        }
    }

    ImGui::End();
}
