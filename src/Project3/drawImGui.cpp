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

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <filesystem>

#include "shader_s.h"
#include "ImportedModel.h"

#include "camera.h"
#include "renderer.h"
#include "SceneGraph.h"

#include "drawImGui.hpp"

extern glm::mat4 pMat; // perspective matrix
extern glm::mat4 vMat; // view matrix

extern Camera camera;

extern unsigned int texture[];
extern unsigned int textureColorbuffer;
extern unsigned int depthMap;

void drawIMGUI(std::vector<Shader*> shaders, Renderer *myRenderer, 
    std::vector<Material*>materials,ParticleRenderer *particleSystem,
    SceneGraph *sg) {
    // Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        // used to get values from imGui to the model matrix
        static float axis[] = { 0.0f,0.0f,1.0f };
        static float angle = 0.0f;

        static float transVec[] = { 0.0f,0.0f,0.0f };
        static float scaleVec[] = { 1.0f,1.0f,1.0f };

        // used to get values from imGui to the camera (view) matrix
        static float v_axis[] = { 0.0f,1.0f,0.0f };
        static float v_angle = 0.0f;

        static float v_transVec[] = { 0.0f,0.0f,3.0f };
        static float camTarget[] = { 0.0f,0.0f,0.0f };
        
        static bool autoPan = false;
        
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Graphics For Games V3");  // Create a window and append into it.

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

        Shader* gShader = shaders[0];

        if (ImGui::BeginTabBar("Shaders", tab_bar_flags))
        {

            for (Shader* s : shaders) {
                if (ImGui::BeginTabItem(s->name))
                {
                    gShader = s;
                    ImGui::EndTabItem();
                }
            }
            bool showShaders = true;

            if (ImGui::BeginTabItem("close")) {
                showShaders = false;
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();

            if (showShaders) {
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

        ImGui::Text("Model Matrix");
        // values we'll use to derive a model matrix
        ImGui::DragFloat3("Translate", transVec,.01f, -3.0f, 3.0f);
        ImGui::InputFloat3("Axis", axis,"%.2f");
        ImGui::SliderAngle("Angle", &angle,-90.0f,90.0f);
        ImGui::DragFloat3("Scale", scaleVec,.01f,-3.0f,3.0f);
        
        // factor in the results of imgui tweaks for the next round...
        myRenderer->setXForm(glm::mat4(1.0f));
        myRenderer->translate(transVec);
        myRenderer->rotate(axis, angle);
        myRenderer->scale(scaleVec);
        
        ImGui::Text("Camera Matrix");
        ImGui::SameLine(); ImGui::Checkbox("AutoPan", &autoPan);
        // values we'll use to derive a model matrix
        ImGui::DragFloat3("vTranslate", v_transVec,.1f, -100.0f, 100.0f);
        ImGui::InputFloat3("vAxis", v_axis, "%.2f");
        ImGui::DragFloat3("camTarget", camTarget, .1f, -100.0f, 100.0f);
        
        if (!autoPan)
            ImGui::SliderAngle("vAngle", &v_angle,-180.0f,180.0f);
        else
            v_angle = fmod(glfwGetTime()/4.0f, glm::pi<float>() *2.0) - glm::pi<float>();

        ImGui::DragFloat("Zoom", &(camera.Zoom), .5f,12.0f, 120.0f);
        sg->camera.projection = glm::perspective(glm::radians(camera.Zoom), camera.Aspect, 0.01f, 1000.0f);    //  1.0472 radians = 60 degrees
        
        glm::mat4 vMat = glm::rotate(glm::mat4(1.0f), -v_angle, glm::vec3(v_axis[0], v_axis[1], v_axis[2]));

        sg->camera.position = vMat * glm::vec4(v_transVec[0], v_transVec[1], v_transVec[2], 1.0f);
        sg->camera.target = glm::vec4(camTarget[0], camTarget[1], camTarget[2], 1.0f);

        for (int i = 0; i < 3; i++)
        {
            ImGui::PushID(i);

            if (ImGui::ImageButton((void*)(intptr_t)texture[i], ImVec2(64, 64)))
                materials[3]->textures[0] = texture[i];
            ImGui::PopID();
            ImGui::SameLine();
        }

        ImGui::NewLine();

        ImGui::SliderFloat("shine5", &materials[2]->shine, 0.0, 1.0);
        ImGui::SliderFloat("shine6", &materials[3]->shine, 0.0, 1.0);

        ImGui::DragInt("particles", &(particleSystem->instances), 1, 0, particleSystem->maxParticles);

        //ImGui::ShowDemoWindow(); // easter egg!  show the ImGui demo window
        
        ImGui::Image((void*)(intptr_t)depthMap, ImVec2(128, 128));

        ImGui::Image((void*)(intptr_t)textureColorbuffer, ImVec2(128, 128));

        ImGui::End();

        // IMGUI Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}
