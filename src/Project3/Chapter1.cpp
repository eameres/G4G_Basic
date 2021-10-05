//
// Sandbox program for Computer Graphics For Games (G4G)
// created May 2021 by Eric Ameres for experimenting
// with OpenGL and various graphics algorithms
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
#include <string>
#include <map>
#include <filesystem>

#include "shader_s.h"
#include "ImportedModel.h"

#include "renderer.h"
#include "textures.h"
#include "SceneGraph.h"
#include "FrameBufferObjects.h"

#include "drawImGui.hpp"

#include "Chapter0.h"

// settings
extern unsigned int scrn_width;
extern unsigned int scrn_height;

static SceneGraph scene;
static SceneGraph* globalScene;

Renderer* triangle;

static void drawMyGUI(SceneGraph* sg,Renderer *myRenderer) {
    // Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        // used to get values from imGui to the model matrix
        static float axis[] = { 0.0f,0.0f,1.0f };
        static float angle = 0.0f;

        static float transVec[] = { -2.0f,0.0f,0.0f };
        static float scaleVec[] = { 1.0f,1.0f,1.0f };

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

        ImGui::Begin("Graphics For Games Chapter 1 - First Triangle");  // Create a window and append into it.

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        static float tFloat = 0.0;
        ImGui::SliderFloat("timeOffset", &tFloat, -5.0f, 5.0f);
        static bool freeze = false;

        ImGui::Checkbox("Freeze Time", &freeze);

        if (freeze)
            sg->time = baseTime + (double)tFloat;
        else
            sg->time = baseTime = glfwGetTime();

        if (myRenderer != NULL) {
            ImGui::Text("Model Matrix");
            // values we'll use to derive a model matrix
            ImGui::DragFloat3("Translate", transVec, .01f, -3.0f, 3.0f);
            ImGui::InputFloat3("Axis", axis, "%.2f");
            ImGui::SliderAngle("Angle", &angle, -90.0f, 90.0f);
            ImGui::DragFloat3("Scale", scaleVec, .01f, -3.0f, 3.0f);

            // factor in the results of imgui tweaks for the next round...
            myRenderer->setXForm(glm::mat4(1.0f));
            myRenderer->translate(transVec);
            myRenderer->rotate(axis, angle);
            myRenderer->scale(scaleVec);
        }
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

        ShaderEditor(sg);
        ImGui::End();

        // IMGUI Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}
void Chapter1::start()
{
    globalScene = &scene;

    // 
    // set up the perspective projection for the camera and the light
    //
    scene.camera.setPerspective(glm::radians(60.0f), ((float)scrn_width / (float)scrn_height), 0.01f, 1000.0f);    //  1.0472 radians = 60 degrees
    scene.camera.position = glm::vec4(0, 0, -5, 1.0f);
    scene.camera.target = glm::vec4(0, 0, 0, 1.0f);

    // create shaders and then materials that use the shaders (multiple materials can use the same shader)

    // Note that the Shader and Material classes use static maps of created instances of each
    //  this is to avoid any global lists of each of them and avoid scope issues when referencing them
    //  be careful when referencing them since the strings MUST match!
    //  the maps are only used during setup and teardown, and not within the main loop, so efficiency isn't an issue

    {   // declare and intialize our base shader and materials
        new Shader("data/vertex.glsl", "data/fragment.glsl", "base");

        new Material(Shader::shaders["base"], "green", -1, glm::vec4(0.80, 0.80, 0.0, 1.0));
        new Material(Shader::shaders["base"], "red", -1, glm::vec4(0.90, 0.10, 0.1, 1.0));
    }

    //
    // OK, now to the scene stuff...
    // 
    triangle = new TriangleModel(Material::materials["red"], glm::mat4(1.0)); // our first triangle

    // if you plan on rendering it as part of a scene, add it to the scene hierarchy
    //scene.addRenderer(triangle);
}

void Chapter1::update(double deltaTime) {
    {
        // do the "normal" drawing
        glViewport(0, 0, scrn_width, scrn_height);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw the triangle directly, no camera, no perspective
        triangle->render(glm::mat4(1.0), glm::mat4(1.0), deltaTime, &scene);

        // render from the cameras position and perspective 
        //scene.renderFrom(scene.camera, deltaTime);
    }
    // draw imGui over the top
    drawMyGUI(&scene, 0L);
}

void Chapter1::end() {

    // housekeeping, remove all the shaders, materials and renderers created

    static std::map<std::string, Shader*> sTemp = Shader::shaders;

    for (const auto& [key, val] : sTemp)
        delete val;

    static std::map<std::string, Material*> mTemp = Material::materials;

    for (const auto& [key, val] : mTemp)
        delete val;

    static std::vector<Renderer*> tempR = scene.rendererList;

    for (Renderer* r : tempR)
        delete r;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void Chapter1::callback(GLFWwindow* window, int width, int height)
{
   globalScene->camera.setAspect((float)width / (float)height);
}