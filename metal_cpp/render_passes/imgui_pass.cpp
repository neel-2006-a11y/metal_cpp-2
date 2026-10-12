//
//  imgui_pass.cpp
//  metal_cpp
//
//  Created by Neel on 06/04/26.
//

#include "render_passes/imgui_pass.h"
#include "scene/SceneNode.h"

void ImGuiPass::init(RenderDevice renderDevice){
    rpDesc = MTL::RenderPassDescriptor::alloc()->init();
    
    rpDesc->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionLoad);
    rpDesc->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
}

void ImGuiPass::execute(RenderContext renderContext, RenderGraph* renderGraph){
    
    rpDesc->colorAttachments()->object(0)->setTexture(renderContext.backBuffer);
//    std::cout << "backBuffer width: " << renderContext.backBuffer->width() << ", " << "backBuffer height: " << renderContext.backBuffer->height() << std::endl;
    
    // ------------
    // Start frame
    // ------------
    ImGui_ImplMetal_NewFrame(rpDesc);
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // ------------
    // UI
    // ------------
    DebugSettings* dbg = renderContext.debugData;
    ImGui::Begin("Debug");
    
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    
    ImGui::SliderFloat3("Sun Dir", (float*)&dbg->sunDirection, -1.0, 1.0);
    
    ImGui::SliderFloat("Bayer Scale", &dbg->BayerScale, 0.0, 1.0);
    
    ImGui::SliderFloat("Fog Density", &dbg->volumeDensity, 0.0, 0.2f);
    
    ImGui::SliderFloat("g", &dbg->g, -1.0, 2.0);
    
    ImGui::SliderFloat("depth_thres", &dbg->depth_thres, 0.0, 1.0);
    ImGui::SliderFloat("normal_thres", &dbg->normal_thres, 0.0, 2.0);
    
    ImGui::SliderInt("ink_width", &dbg->ink_width, 1, 100);

    ImGui::End();
    
    // ------------
    // Scene Graph
    // ------------
    if(root){
        drawSceneHierarchy(root, selected);
    }
    if(selected)
        drawInspector(selected);
    
    
    encoder = renderContext.cmd->renderCommandEncoder(rpDesc);
    
    
    ImGuiIO& io = ImGui::GetIO();

//    printf("DisplaySize = %.0f %.0f\n",
//           io.DisplaySize.x,
//           io.DisplaySize.y);
//
//    printf("FramebufferScale = %.1f %.1f\n",
//           io.DisplayFramebufferScale.x,
//           io.DisplayFramebufferScale.y);

    
    ImGui::Render();
    ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), renderContext.cmd, encoder);
    encoder->endEncoding();
}


void ImGuiPass::resize(RenderContext renderContext){
    
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

        if (node->renderComponent) {
            ImGui::Separator();
            ImGui::Text("Renderer");

            ImGui::Text("MeshID: %d", node->renderComponent->meshID);
            ImGui::Text("MaterialID: %d", node->renderComponent->materialID);
        }
    }

    ImGui::End();
}
