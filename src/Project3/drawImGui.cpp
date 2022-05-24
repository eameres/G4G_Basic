//
//  drawImGui.cpp
//  g4g2
//
//  Created by Eric Ameres on 9/18/21.
//

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <map>
#include <filesystem>

#include "shader_s.h"
#include "ImportedModel.h"

#include "renderer.h"
#include "SceneGraph.h"

#include "drawImGui.hpp"

extern glm::mat4 pMat; // perspective matrix
extern glm::mat4 vMat; // view matrix


extern unsigned int texture[];
extern unsigned int textureColorbuffer;
extern unsigned int depthMap;

static int item_current_idx = 0; // Here we store our selection data as an index.

void drawIMGUI(Renderer *myRenderer, iCubeModel*cubeSystem,
    SceneGraph *sg, std::map<std::string, unsigned int> texMap,treeNode * nodes[]) {
    // Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {


            // used to get values from imGui to the camera (view) matrix
            static float v_axis[] = { 0.0f,1.0f,0.0f };
            static float v_angle = 0.0f;

            static float v_transVec[] = { 0.0f,0.0f,3.0f };
            static float camTarget[] = { 0.0f,0.0f,0.0f };
            float camClip[] = { 1.0f,1000.0f };
            float camFOV, camAspect;

            static bool autoPan = false;
            static float baseTime;

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("Graphics For Games V3");  // Create a window and append into it.

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            static float tFloat = 0.0;
            ImGui::SliderFloat("timeOffset", &tFloat, -5.0f, 5.0f);
            static bool freeze = false;

            ImGui::Checkbox("Freeze Time", &freeze);

            if (freeze)
                sg->time = baseTime + tFloat;
            else
                sg->time = baseTime = glfwGetTime();

            ImGui::Text("Camera Matrix");
            ImGui::SameLine(); ImGui::Checkbox("AutoPan", &autoPan);
            // values we'll use to derive a model matrix
            ImGui::DragFloat3("vTranslate", v_transVec, .1f, -100.0f, 100.0f);
            ImGui::InputFloat3("vAxis", v_axis, "%.2f");
            ImGui::DragFloat3("camTarget", camTarget, .1f, -100.0f, 100.0f);

            if (!autoPan)
                ImGui::SliderAngle("vAngle", &v_angle, -180.0f, 180.0f);
            else
                v_angle = fmod(sg->time / 4.0f, glm::pi<float>() * 2.0) - glm::pi<float>();

            camFOV = glm::degrees(sg->camera.getFOV());
            ImGui::DragFloat("Field Of View", &camFOV, .5f, 12.0f, 120.0f);

            camAspect = sg->camera.getAspect();
            ImGui::SliderFloat("Aspect", &camAspect, -3.0f, 3.0f);

            sg->camera.getClipNearFar(&camClip[0], &camClip[1]);
            ImGui::DragFloat2("Clip Near - Far", camClip, .01f, -1.0f, 1000.0f);

            sg->camera.setPerspective(glm::radians(camFOV), camAspect, camClip[0], camClip[1]);    //  1.0472 radians = 60 degrees

            glm::mat4 vMat = glm::rotate(glm::mat4(1.0f), -v_angle, glm::vec3(v_axis[0], v_axis[1], v_axis[2]));

            sg->camera.position = vMat * glm::vec4(v_transVec[0], v_transVec[1], v_transVec[2], 1.0f);
            sg->camera.target = glm::vec4(camTarget[0], camTarget[1], camTarget[2], 1.0f);

            ImGui::NewLine();

            if (cubeSystem != NULL)
                ImGui::DragInt("particles", &(cubeSystem->instances), 1, 0, cubeSystem->maxParticles);

            ImGui::ShowDemoWindow(); // easter egg!  show the ImGui demo window
        
            ImGui::Image((void*)(intptr_t)texMap["depth"], ImVec2(128, 128));
            ImGui::SameLine();
            ImGui::Image((void*)(intptr_t)texMap["offScreen"], ImVec2(128, 128));
            
            void treeTest(SceneGraph * sg);

            treeTest(sg);
            ImGui::End();

            ShaderEditor(sg);
            ListRenderers(sg, nodes);
            // IMGUI Rendering
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
}

static void showChildren(treeNode* tn) {


    std::vector<treeNode*> *children = tn->getChildren();
    int i = 0;

    for (treeNode *t : *children) {
        if (ImGui::TreeNode((void*)(intptr_t)i, "Child %d", i)) {
            if (t->getChildren()->size() > 0)
                showChildren(t);
            ImGui::TreePop();
        }
        i++;
    }
}
void treeTest(SceneGraph *sg) {

    treeNode *start = sg->getRoot();

    if (ImGui::TreeNode("Scene"))
    {
        showChildren(start);
        ImGui::TreePop();
    }
}

void ListRenderers(SceneGraph *sg, treeNode* nodes[])
{
    bool p_open = true;

    if (ImGui::Begin("Model Editor", &p_open, ImGuiWindowFlags_MenuBar))
    {
        ImGui::Checkbox("Node 0", &nodes[0]->enabled);
        ImGui::SameLine(); ImGui::Checkbox("Node1", &nodes[1]->enabled);
        ImGui::SameLine(); ImGui::Checkbox("Node2", &nodes[2]->enabled);
        ImGui::SameLine(); ImGui::Checkbox("Node3", &nodes[3]->enabled);
        ImGui::SameLine(); ImGui::Checkbox("Node4", &nodes[4]->enabled);
        // Left
        static int selected = 0;
        {
            ImGui::BeginChild("left pane", ImVec2(150, 0), true);
                        for (int n = 0; n < Renderer::renderList.size(); n++)
            {
                const bool is_selected = (item_current_idx == n);

                ImGui::PushID(n);           // Push i to the id stack
                if (ImGui::Selectable(Renderer::renderList[n]->name.c_str(), is_selected))
                    item_current_idx = n;
                ImGui::PopID();
                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndChild();
        }
        ImGui::SameLine();

        // Right
        {
            ImGui::BeginGroup();
            //ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),false, ImGuiWindowFlags_NoScrollbar); // Leave room for 1 line below us

            if ((Renderer::renderList.size() > 0) && (item_current_idx < Renderer::renderList.size())) {
                char cName[128];
                strcpy(cName, Renderer::renderList[item_current_idx]->name.c_str());
                ImGui::InputText("##edit", cName, IM_ARRAYSIZE(cName));
                Renderer::renderList[item_current_idx]->name = cName;

                ImGui::Separator();
                if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
                {
                    if (ImGui::BeginTabItem("Transform"))
                    {
                        // used to get values from imGui to the model matrix
                        static float axis[] = { 0.0f,0.0f,1.0f };
                        static float angle = 0.0f;

                        static float transVec[] = { 0.0f,0.0f,0.0f };
                        static float scaleVec[] = { 1.0f,1.0f,1.0f };

                        if (item_current_idx >= Renderer::renderList.size())
                            item_current_idx = Renderer::renderList.size() - 1;

                        if ((Renderer::renderList.size() > 0) && (Renderer::renderList[item_current_idx] != NULL)) {

                            glm::vec3 scale;
                            glm::quat rotation;
                            glm::vec3 translation;
                            glm::vec3 skew;
                            glm::vec4 perspective;
                            glm::decompose(Renderer::renderList[item_current_idx]->modelMatrix, scale, rotation, translation, skew, perspective);

                            transVec[0] = translation.x;
                            transVec[1] = translation.y;
                            transVec[2] = translation.z;

                            scaleVec[0] = scale.x;
                            scaleVec[1] = scale.y;
                            scaleVec[2] = scale.z;

                            rotation = glm::conjugate(rotation);

                            angle = 2 * acos(rotation.w);

                            double s = sqrt(1 - rotation.w * rotation.w);

                            if (s < 0.001) s = 1;

                            axis[0] = rotation.x / s;
                            axis[1] = rotation.y / s;
                            axis[2] = rotation.z / s;

                            if (axis[0] == axis[1] == axis[2] == 0.0) {
                                axis[0] = 1.0;
                            }
                            std::string mName = "mMatrix for model " + Renderer::renderList[item_current_idx]->name;
                            ImGui::Text(mName.c_str());
                            // values we'll use to derive a model matrix
                            ImGui::DragFloat3("Translate", transVec, .01f, -30.0f, 30.0f);
                            ImGui::InputFloat3("Axis", axis, "%.2f");
                            ImGui::SliderAngle("Angle", &angle, 0.0f, 360.0f);
                            ImGui::DragFloat3("Scale", scaleVec, .01f, 0.01f, 3.0f);


                            // factor in the results of imgui tweaks for the next round...
                            Renderer::renderList[item_current_idx]->setXForm(glm::mat4(1.0f));
                            Renderer::renderList[item_current_idx]->translate(transVec);
                            Renderer::renderList[item_current_idx]->rotate(axis, angle);
                            Renderer::renderList[item_current_idx]->scale(scaleVec);

                        }

                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Material"))
                    {
                        ImGui::Text("Coming Soon!");
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }

                if (ImGui::BeginPopup("nonDeletable"))
                {
                    ImGui::Text("Skybox and Canvas Quad cannot be deleted");
                    ImGui::EndPopup();
                }
                if (ImGui::Button("Delete")) {
                    if (item_current_idx > 1) {
                        if ((Renderer::renderList.size() > 0) && (Renderer::renderList[item_current_idx] != NULL)) {
                            sg->purgeRenderer(Renderer::renderList[item_current_idx]);
                        }
                    }else {
                        ImGui::OpenPopup("nonDeletable");
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Delete All")) {
                    if (Renderer::renderList.size() > 2) {
                        for (int i = Renderer::renderList.size() - 1; i >= 2; i--)
                            sg->purgeRenderer(Renderer::renderList[i]);
                    }
                }
            }
            if (ImGui::Button("Add Torus")) {
                sg->getRoot()->addRenderer(new TorusModel(Material::materials["litMaterial"], glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))));
            }
            ImGui::SameLine();
            if (ImGui::Button("Add Cube")) {
                sg->getRoot()->addRenderer(new CubeModel(Material::materials["litMaterial"], glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))));
            }
            
            if (ImGui::Button("Add Sphere")) {
                sg->getRoot()->addRenderer(new SphereModel(Material::materials["litMaterial"], glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))));
            }
            ImGui::EndGroup();
        }
    }
    ImGui::End();
}

void ShaderEditor(SceneGraph* sg) {
    // Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            ImGui::Begin("Shader Editor");  // Create a window and append into it.

            
            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

            Shader* gShader = Shader::shaders["base"];

            if (ImGui::BeginTabBar("Shaders", tab_bar_flags))
            {
                for (const auto& [key, s] : Shader::shaders) {
                    if (ImGui::BeginTabItem(s->name))
                    {
                        gShader = s;
                        ImGui::EndTabItem();
                    }
                }

                ImGui::EndTabBar();

                {
                    static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
                    ImGui::Text("Vertex Shader");
                    ImGui::SameLine();
                    ImGui::Text("%s", std::filesystem::absolute(gShader->vertexPath).u8string().c_str());
                    ImGui::InputTextMultiline("Vertex Shader", gShader->vtext, IM_ARRAYSIZE(gShader->vtext), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);

                    ImGui::Text("Fragment Shader");
                    ImGui::SameLine();
                    ImGui::Text("%s", std::filesystem::absolute(gShader->fragmentPath).u8string().c_str());
                    ImGui::InputTextMultiline("Fragment Shader", gShader->ftext, IM_ARRAYSIZE(gShader->ftext), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);

                    if (ImGui::Button("reCompile Shaders"))
                        gShader->reload();

                    ImGui::SameLine();

                    if (ImGui::Button("Save Shaders"))
                        gShader->saveShaders();
                }
            }

            if (Material::materials["checkers"] != NULL) {
                for (int i = 0; i < 2; i++)
                {
                    ImGui::PushID(i);

                    if (ImGui::ImageButton((void*)(intptr_t)texture[i], ImVec2(64, 64)))
                        Material::materials["checkers"]->textures[0] = texture[i];
                    ImGui::PopID();
                    ImGui::SameLine();
                }
            }

            ImGui::NewLine();

            if (Material::materials["checkers"] != NULL)
                ImGui::SliderFloat("shine5", &Material::materials["checkers"]->shine, 0.0, 1.0);

            if (Material::materials["shuttle"] != NULL)
                ImGui::SliderFloat("shine6", &Material::materials["shuttle"]->shine, 0.0, 1.0);

            ImGui::End();

        }
}
