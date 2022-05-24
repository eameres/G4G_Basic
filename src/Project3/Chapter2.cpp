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

SceneGraph scene;
SceneGraph* globalScene;

void Chapter2::dragDrop(GLFWwindow* window, int count, const char** paths) {
    int i;

    std::string objFile, textureFile;

    while (scene.getRoot() != scene.getCurrentNode())
        scene.getParent();

    std::string temp = paths[0];

    if (count > 1) {
        for (i = 0; i < count; i++) {
            temp = paths[i];
            if (temp.find("obj") != std::string::npos) {
                objFile = temp;
            }
            else if((temp.find("jpg") != std::string::npos) || (temp.find("png") != std::string::npos))
                textureFile = temp;
        }        
        Material *temp = new Material(Shader::shaders["textured"], textureFile, loadTexture(textureFile.c_str()), 4, true);
        scene.addRenderer(new ObjModel(objFile.c_str(), temp, glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3.0f, 0.0f)), glm::vec3(1.0f))));
    }else if (temp.find("obj") != std::string::npos)
        scene.addRenderer(new ObjModel(paths[0], Material::materials["litMaterial"], glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)), glm::vec3(1.0f))));
}
void cubeOfCubes(SceneGraph* sg)
{
    for (int i = -1; i < 2; i++)
        for (int j = -1; j < 2; j++)
            for (int k = -1; k < 2; k++) {
                glm::mat4 xf;

                float x = i, y = j, z = k;
                xf = glm::rotate(glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(x * .33, y * .33, k * .33)), glm::vec3(.20)), .8f, glm::vec3(1, 1, 0));
                Renderer* nC;
                if ((j == 0) && (k == 0) && (i == 0))
                    nC = new SphereModel(Material::materials["unicorn"], xf);
                else if ((j == k) || (i == j))
                    nC = new CubeModel(Material::materials["rpi"], xf);
                else
                    nC = new CubeModel(Material::materials["litMaterial"], xf);
                sg->addRenderer(nC);
                nC->name = "subCube "  + std::to_string(i)  + ':' + std::to_string(j) + ':' + std::to_string(k);
            }
}

void setupShadersAndMaterials(std::map<std::string, unsigned int> texMap)
{
    {   // declare and intialize shader with colored vertices
        new Shader("data/vertColors.glsl", "data/fragColors.glsl", "colored");

        new Material(Shader::shaders["colored"], "coloredVerts", -1, glm::vec4(1.0, 1.0, 0.0, 1.0));
    }
    {   // declare and intialize shader with texture(s)
        new Shader("data/vertTexture.glsl", "data/fragTexture.glsl", "textured");

        new Material(Shader::shaders["textured"], "shuttle", texMap["shuttle"], texMap["sky"]);
        new Material(Shader::shaders["textured"], "checkers", texMap["myTexture"], texMap["sky"]);
        new Material(Shader::shaders["textured"], "unicorn", texMap["unicorn"], texMap["sky"]);
        new Material(Shader::shaders["textured"], "rpi", texMap["rpi"], texMap["sky"]);
        new Material(Shader::shaders["textured"], "brick", texMap["brick"], texMap["sky"]);
    }
    {
        new Shader("data/vParticle.glsl", "data/fParticle.glsl", "Particle"); // declare and intialize skybox shader
        new Material(Shader::shaders["Particle"], "pMaterial", -1, glm::vec4(1.0, 0.0, 0.0, 1.0));
    }
}

iCubeModel* cubeSystem = NULL;
QuadModel* frontQuad = NULL;

void setupScene(SceneGraph* scene, treeNode** nodes)
{
    nodes[0] = scene->getCurrentNode();
    nodes[0]->enabled = true;

    nodes[1] = scene->addChild(glm::mat4(1));
    nodes[1]->enabled = false;

    //glm::mat4 floorXF = glm::rotate(glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(0.0f, -1.0f, 0.0f)), glm::vec3(10.0f)), glm::pi<float>() / 2.0f, glm::vec3(-1, 0, 0));
    //scene->addRenderer(frontQuad = new QuadModel(Material::materials["brick"], floorXF)); // our floor quad

    //scene->addRenderer(new ObjModel("data/Sponza-master/sponza.obj_", Material::materials["litMaterial"], glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3.0f, 0.0f)), glm::vec3(.02f))));
    //scene->addRenderer(new ObjModel("data/fireplace/fireplace_room.obj_", Material::materials["litMaterial"], glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3.0f, 0.0f)), glm::vec3(.02f))));
    //scene->addRenderer(new ObjModel("data/shuttle.obj_", Material::materials["shuttle"], glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f)), glm::vec3(2.0f))));

    nodes[2] = scene->addChild(glm::mat4(1));

    scene->addRenderer(cubeSystem = new iCubeModel(Material::materials["pMaterial"], glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(.025f)), glm::vec3(0.0f, 0.0f, 0.0f))));

    scene->addRenderer(frontQuad = new QuadModel(Material::materials["rpi"], glm::mat4(1.0f))); // our "first quad

    scene->addRenderer(new QuadModel(Material::materials["coloredVerts"], glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(1.0, 0.0, 0.0)))); // our "second quad"

    nodes[3] = scene->addChild(glm::mat4(1));

    scene->addRenderer(new TorusModel(Material::materials["litMaterial"], glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))));

    scene->getParent();
    nodes[4] = scene->addChild(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)));

    cubeOfCubes(scene);

}

void animateNodes(treeNode** nodes, double time)
{
    // animating the three levels of our hierarchy
    if (nodes[2] != NULL)nodes[2]->setXform(glm::rotate(glm::mat4(1.0f), (float)time, glm::vec3(1, 0, 0.0f)));
    if (nodes[3] != NULL)nodes[3]->setXform(glm::translate(glm::rotate(glm::mat4(1.0f), (float)time * 2.0f, glm::vec3(0, 1, 0.0f)), glm::vec3(0, 1, 0)));
    if (nodes[4] != NULL)nodes[4]->setXform(glm::translate(glm::rotate(glm::mat4(1.0f), (float)time * -2.0f, glm::vec3(0, 0, 1.0f)), glm::vec3(0.0f, 0.0f, -1.0f)));

}

unsigned int depthMapFBO = 0;
unsigned int offscreenFBO = 0;
treeNode* nodes[5];

std::map<std::string, unsigned int> texMap;

const unsigned int SHADOW_WIDTH = 1024;
const unsigned int SHADOW_HEIGHT = 1024;

// settings
extern unsigned int scrn_width;
extern unsigned int scrn_height;


unsigned int texture[] = { 0,1,2,3 };

CubeModel* lightCube = NULL;
SkyboxModel* mySky;
QuadModel* fQuad;

void Chapter2::start()
{
    globalScene = &scene;

    setupTextures(texture);

    texMap["myTexture"] = texture[0];
    texMap["rayTrace"] = texture[1];
    texMap["sky"] = texture[2];
    texMap["depth"] = setupDepthMap(&depthMapFBO, SHADOW_WIDTH, SHADOW_HEIGHT);
    texMap["offScreen"] = setupFrameBuffer(&offscreenFBO, scrn_width, scrn_height);
    texMap["shuttle"] = loadTexture("data/spstob_1.jpg");
    texMap["unicorn"] = loadTexture("data/unicorn.png");
    texMap["rpi"] = loadTexture("data/rpi.png");
    texMap["brick"] = loadTexture("data/brick1.jpg");

    // 
    // set up the perspective projection for the camera and the light
    //
    scene.camera.setPerspective(glm::radians(60.0f), ((float)scrn_width / (float)scrn_height), 0.01f, 1000.0f);    //  1.0472 radians = 60 degrees
    scene.camera.position = glm::vec4(0, 0, -5, 1.0f);
    scene.camera.target = glm::vec4(0, 0, 0, 1.0f);

    //scene.light.setOrtho(-4.0f, 4.0f, -4.0f, 4.0f, 1.0f, 50.0f);
    scene.light.setPerspective(glm::radians(60.0f), 1.0, 1.0f, 1000.0f);    //  1.0472 radians = 60 degrees
    scene.light.position = glm::vec4(-4.0f, 2.0f, 0.0f, 1.0f);


    // create shaders and then materials that use the shaders (multiple materials can use the same shader)

    // Note that the Shader and Material classes use static maps of created instances of each
    //  this is to avoid any global lists of each of them and avoid scope issues when referencing them
    //  be careful when referencing them since the strings MUST match!
    //  the maps are only used during setup and teardown, and not within the main loop, so efficiency isn't an issue

    {   // declare and intialize our base shader and materials
        new Shader("data/vertex.glsl", "data/fragment.glsl", "base");

        new Material(Shader::shaders["base"], "white", -1, glm::vec4(1.0, 1.0, 1.0, 1.0));
        new Material(Shader::shaders["base"], "green", -1, glm::vec4(0.80, 0.80, 0.0, 1.0));
    }
    {   // declare and intialize skybox shader and background material
        new Shader("data/vSky.glsl", "data/fSky.glsl", "SkyBox");
        new Material(Shader::shaders["SkyBox"], "background", texMap["sky"], glm::vec4(-1.0));
    }
    {
        new Shader("data/vDepth.glsl", "data/fDepth.glsl", "Depth");
        new Material(Shader::shaders["Depth"], "depthMaterial", -1, glm::vec4(1.0, 1.0, 0.0, 1.0));
    }
    {
        new Shader("data/vPost.glsl", "data/fPost.glsl", "PostProcessing");
        new Material(Shader::shaders["PostProcessing"], "offScreenMaterial", texMap["offScreen"], glm::vec4(1.0, 1.0, 0.0, 1.0));
    }
    {   // declare and intialize shader with ADS lighting
        new Shader("data/vFlatLit.glsl", "data/fFlatLit.glsl", "PhongShadowed");
        new Material(Shader::shaders["PhongShadowed"], "litMaterial", texMap["myTexture"], texMap["depth"], true);
    }

    setupShadersAndMaterials(texMap);

    // skybox is special and doesn't belong to the SceneGraph
    mySky = new SkyboxModel(Material::materials["background"], glm::mat4(1.0f)); // our "skybox"

    // quad renderer for full screen also not in scene : quad needs to be scaled by 2 since the quad is designed around [ -0.5, +0.5 ]
    fQuad = new QuadModel(Material::materials["offScreenMaterial"], glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f))); // our fullScreen Quad

    //
    // OK, now to the scene stuff...
    // 
    // Setup all of the objects to be rendered and add them to the scene at the appropriate level
    scene.addRenderer(new CubeModel(Material::materials["litMaterial"], glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f))));
    scene.addRenderer(new SphereModel(Material::materials["litMaterial"], glm::translate(glm::mat4(0.5f), glm::vec3(0.0f, -2.0f, 0.0f))));

    scene.addRenderer(new SphereModel(Material::materials["green"], glm::translate(glm::mat4(0.5f), glm::vec3(0.0f, -2.0f, 0.0f))));

    scene.addRenderer(new CubeModel(Material::materials["litMaterial"], glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f))));

    scene.addRenderer(lightCube = new CubeModel(Material::materials["white"], glm::translate(glm::mat4(1.0f), scene.light.position)));

    //{
        // crazy scene stuff
    setupScene(&scene, nodes);
    //}

}

void Chapter2::update(double deltaTime) {

    //animate crazy scene stuff
    animateNodes(nodes, scene.time);

    // moving light source, must set it's position...
    glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), (float)scene.time, glm::vec3(1, 0, 0.0f));
    scene.light.position = glm::vec4(-4.0f, 2.0f, 0.0f, 1.0f);


    // show a cube from that position
    lightCube->modelMatrix = rotate * glm::scale(glm::translate(glm::mat4(1.0f), scene.light.position), glm::vec3(.25f));

    scene.light.position = lightCube->modelMatrix * glm::vec4(0.0, 0.0, 0.0, 1.0);

    {
        // first we do the "shadow pass"  really just for creating a depth buffer from the light's perspective

        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_DEPTH_BUFFER_BIT);

        // render from the lights perspective and position to create the shadowMap
        scene.renderFrom(scene.light, deltaTime);
    }
    {
        // do the "normal" drawing
        glViewport(0, 0, scrn_width, scrn_height);

        glBindFramebuffer(GL_FRAMEBUFFER, offscreenFBO); // offscreen

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        mySky->render(glm::mat4(glm::mat3(glm::lookAt(scene.camera.position, scene.camera.target, scene.camera.up))), scene.camera.projection(), deltaTime, &scene);

        glEnable(GL_DEPTH_TEST);

        // render from the cameras position and perspective  this may or may not be offscreen 
        scene.renderFrom(scene.camera, deltaTime);
    }
    if (offscreenFBO != 0) {
        // assuming the previous was offscreen, we now need to draw a quad with the results to the screen!
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glDisable(GL_DEPTH_TEST);

        fQuad->render(glm::mat4(1.0f), glm::mat4(1.0f), deltaTime, &scene);
    }
    // draw imGui over the top
    drawIMGUI(frontQuad, cubeSystem, &scene, texMap, nodes);
    //scene.time = glfwGetTime();
}

void Chapter2::end() {
    deleteTextures(texture);

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
void Chapter2::callback(GLFWwindow* window, int width, int height)
{
    globalScene->camera.setAspect((float)width / (float)height);
}
