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

#include "camera.h"
#include "renderer.h"
#include "textures.h"
#include "SceneGraph.h"
#include "FrameBufferObjects.h"

#include "drawImGui.hpp"

glm::mat4 *gCameraProjection; // perspective matrix

Camera camera;

// settings
unsigned int scrn_width = 1280;
unsigned int scrn_height = 720;

unsigned int texture[] = { 0,1,2,3 };

#pragma warning( disable : 26451 )

int main();

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void cubeOfCubes(SceneGraph *sg) 
{
    for (int i = -1; i < 2; i++)
        for (int j = -1; j < 2; j++)
            for (int k = -1; k < 2; k++) {
                glm::mat4 xf;

                float x = i, y = j, z = k;
                xf = glm::rotate(glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(x * .33, y * .33, k * .33)), glm::vec3(.20)), .8f, glm::vec3(1, 1, 0));
                Renderer* nC;
                if ((j == 0) && (k == 0) && (i == 0))
                    nC = new SphereRenderer(Material::materials["unicorn"], xf);
                else if ((j == k) || (i == j))
                    nC = new CubeRenderer(Material::materials["rpi"], xf);
                else
                    nC = new CubeRenderer(Material::materials["litMaterial"], xf);
                sg->addRenderer(nC);
            }
}

void setupShadersAndMaterials(std::map<std::string, unsigned int> texMap)
{
    {   // declare and intialize shader with colored vertices
        new Shader("data/vertColors.lgsl", "data/fragColors.lgsl", "colored");

        new Material(Shader::shaders["colored"], "coloredVerts", -1, glm::vec4(1.0, 1.0, 0.0, 1.0));
    }
    {   // declare and intialize shader with texture(s)
        new Shader("data/vertTexture.lgsl", "data/fragTexture.lgsl", "textured");

        new Material(Shader::shaders["textured"], "shuttle", texMap["shuttle"], texMap["sky"]);
        new Material(Shader::shaders["textured"], "checkers", texMap["myTexture"], texMap["sky"]);
        new Material(Shader::shaders["textured"], "unicorn", texMap["unicorn"], texMap["sky"]);
        new Material(Shader::shaders["textured"], "rpi", texMap["rpi"], texMap["sky"]);
        new Material(Shader::shaders["textured"], "brick", texMap["brick"], texMap["sky"]);
    }
    {
        new Shader("data/vParticle.lgsl", "data/fParticle.lgsl", "Particle"); // declare and intialize skybox shader
        new Material(Shader::shaders["Particle"], "pMaterial", -1, glm::vec4(1.0, 0.0, 0.0, 1.0));
    }
}

iCubeRenderer* cubeSystem = NULL;
QuadRenderer* frontQuad = NULL;

void setupScene(SceneGraph *scene,treeNode **nodes)
{
    nodes[0] = scene->getCurrentNode();
    nodes[0]->enabled = true;

    nodes[1] = scene->addChild(glm::mat4(1));
    nodes[1]->enabled = false;

    glm::mat4 floorXF = glm::rotate(glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(0.0f, -1.0f, 0.0f)), glm::vec3(10.0f)), glm::pi<float>()/2.0f, glm::vec3(-1, 0, 0));
    scene->addRenderer(frontQuad = new QuadRenderer(Material::materials["brick"], floorXF)); // our floor quad

    scene->addRenderer(new ObjRenderer("data/sponza.obj_", Material::materials["litMaterial"], glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3.0f, 0.0f)), glm::vec3(.02f))));
    scene->addRenderer(new ObjRenderer("data/shuttle.obj_", Material::materials["shuttle"], glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f)), glm::vec3(2.0f))));

    nodes[2] = scene->addChild(glm::mat4(1));

    scene->addRenderer(cubeSystem = new iCubeRenderer(Material::materials["pMaterial"], glm::translate(glm::mat4(.025f), glm::vec3(0.0f, 0.0f, 0.0f))));

    scene->addRenderer(frontQuad = new QuadRenderer(Material::materials["checkers"], glm::mat4(1.0f))); // our "first quad

    scene->addRenderer(new QuadRenderer(Material::materials["coloredVerts"], glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(1.0, 0.0, 0.0)))); // our "second quad"

    nodes[3] = scene->addChild(glm::mat4(1));

    scene->addRenderer(new TorusRenderer(Material::materials["litMaterial"], glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))));

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
int main()
{
    const unsigned int SHADOW_WIDTH = 1024;
    const unsigned int SHADOW_HEIGHT = 1024;

    SceneGraph scene;

    gCameraProjection = &scene.camera.projection;  // hack for the viewport callback function to set the projection matrix

    namespace fs = std::filesystem;
    std::cout << "Current path is " << fs::current_path() << '\n';

    fs::file_status s = fs::file_status{};

    if (fs::status_known(s) ? fs::exists(s) : fs::exists("../../../data")) {
        std::cout << "Found upper level data directory, therefor assuming we're in a development tree \n";
        fs::current_path("../../..");
    }

    std::cout << "Absolute path for shaders is " << std::filesystem::absolute("./data/") << '\n';

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_TRUE);
#endif
    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Graphics4Games Fall 2021", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    GLint viewportDims[4];

    glGetIntegerv(GL_VIEWPORT, viewportDims);
    scrn_width = viewportDims[2];
    scrn_height = viewportDims[3];

    setupTextures(texture);

    unsigned int depthMapFBO;
    unsigned int offscreenFBO;

    std::map<std::string, unsigned int> texMap;

    texMap["myTexture"] = texture[0];
    texMap["rayTrace"] = texture[1];
    texMap["sky"] = texture[2];
    texMap["depth"] = setupDepthMap(&depthMapFBO,SHADOW_WIDTH,SHADOW_HEIGHT);
    texMap["offScreen"] = setupFrameBuffer(&offscreenFBO,scrn_width,scrn_height);
    texMap["shuttle"] = loadTexture("data/spstob_1.jpg");
    texMap["unicorn"] = loadTexture("data/unicorn.png");
    texMap["rpi"] = loadTexture("data/rpi.png");
    texMap["brick"] = loadTexture("data/brick1.jpg");

    // 
    // set up the perspective projection for the camera and the light
    //
    scene.camera.projection = glm::perspective(glm::radians(camera.Zoom), camera.Aspect = ((float)scrn_width / (float)scrn_height), 0.01f, 1000.0f);    //  1.0472 radians = 60 degrees
    scene.camera.position = glm::vec4(0, 0, -5, 1.0f);
    scene.camera.target = glm::vec4(0, 0, 0, 1.0f);

    scene.light.projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 1.0f, 7.5f);
    scene.light.position = glm::vec4(-4.0f, 2.0f, 0.0f, 1.0f);

    
    // create shaders and then materials that use the shaders (multiple materials can use the same shader)

    // Note that the Shader and Material classes use static maps of created instances of each
    //  this is to avoid any global lists of each of them and avoid scope issues when referencing them
    //  be careful when referencing them since the strings MUST match!
    //  the maps are only used during setup and teardown, and not within the main loop, so efficiency isn't an issue

    {   // declare and intialize our base shader and materials
        new Shader("data/vertex.lgsl", "data/fragment.lgsl", "base");

        new Material(Shader::shaders["base"], "white", -1, glm::vec4(1.0, 1.0, 1.0, 1.0));
        new Material(Shader::shaders["base"], "green", -1, glm::vec4(0.80, 0.80, 0.0, 1.0));
    }
    {   // declare and intialize skybox shader and background material
        new Shader("data/vSky.lgsl", "data/fSky.lgsl", "SkyBox");
        new Material(Shader::shaders["SkyBox"], "background", texMap["sky"], glm::vec4(-1.0));
    }
    {
        new Shader("data/vDepth.lgsl", "data/fDepth.lgsl", "Depth");
        new Material(Shader::shaders["Depth"], "depthMaterial", -1, glm::vec4(1.0, 1.0, 0.0, 1.0));
    }
    {
        new Shader("data/vPost.lgsl", "data/fPost.lgsl", "PostProcessing");
        new Material(Shader::shaders["PostProcessing"], "offScreenMaterial", texMap["offScreen"], glm::vec4(1.0, 1.0, 0.0, 1.0));
    } 
    {   // declare and intialize shader with ADS lighting
        new Shader("data/vFlatLit.lgsl", "data/fFlatLit.lgsl", "PhongShadowed");
        new Material(Shader::shaders["PhongShadowed"], "litMaterial", texMap["myTexture"], texMap["depth"], true);
    }

    setupShadersAndMaterials(texMap);

    // skybox is special and doesn't belong to the SceneGraph
    SkyboxRenderer mySky(Material::materials["background"], glm::mat4(1.0f)); // our "skybox"

    // quad renderer for full screen also not in scene : quad needs to be scaled by 2 since the quad is designed around [ -0.5, +0.5 ]
    QuadRenderer fQuad(Material::materials["offScreenMaterial"], glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f))); // our fullScreen Quad

    //
    // OK, now to the scene stuff...
    // 
    // Setup all of the objects to be rendered and add them to the scene at the appropriate level
    scene.addRenderer(new CubeRenderer(Material::materials["litMaterial"], glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f))));
    scene.addRenderer(new SphereRenderer(Material::materials["litMaterial"], glm::translate(glm::mat4(0.5f), glm::vec3(0.0f, -2.0f, 0.0f))));

    CubeRenderer* lightCube = NULL;
    scene.addRenderer(lightCube = new CubeRenderer(Material::materials["white"], glm::translate(glm::mat4(1.0f), scene.light.position)));
    
    //{
        // crazy scene stuff
         treeNode* nodes[5];
         setupScene(&scene, nodes);
    //}
    
    // render loop
    // -----------

    double lastTime = glfwGetTime();

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // needed if we are drawing directly to the window
    if (offscreenFBO == 0)
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    while (!glfwWindowShouldClose(window))
    {
        // just like in a game engine, it's useful to know the delta time
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // glfw: poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwPollEvents();

        // input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        //animate crazy scene stuff
        animateNodes(nodes,scene.time);

        // moving light source, must set it's position...
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), (float)scene.time, glm::vec3(1, 0, 0.0f));
        scene.light.position = rotate * glm::vec4(-4.0f, 2.0f, 0.0f, 1.0f);

        // show a cube from that position
        lightCube->modelMatrix = glm::scale(glm::translate(glm::mat4(1.0f), scene.light.position), glm::vec3(.25f));

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
            mySky.render(glm::mat4(glm::mat3(glm::lookAt(scene.camera.position, scene.camera.target, scene.camera.up))), scene.camera.projection, deltaTime, &scene);
            
            glEnable(GL_DEPTH_TEST);

            // render from the cameras position and perspective  this may or may not be offscreen 
            scene.renderFrom(scene.camera, deltaTime);
        }
        if (offscreenFBO != 0){
            // assuming the previous was offscreen, we now need to draw a quad with the results to the screen!
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glDisable(GL_DEPTH_TEST);

            fQuad.render(glm::mat4(1.0f), glm::mat4(1.0f), deltaTime, &scene);
        }
        // draw imGui over the top
        drawIMGUI(frontQuad,cubeSystem,&scene,texMap,nodes);
        //scene.time = glfwGetTime();

        glfwSwapBuffers(window);
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    
    deleteTextures(texture);

    static std::map<std::string, Shader*> sTemp = Shader::shaders;

     for (const auto&[key,val] : sTemp)
        delete val;

     static std::map<std::string,Material*> mTemp = Material::materials;

     for (const auto& [key, val] : mTemp)
         delete val;

     static std::vector<Renderer*> tempR = scene.rendererList;

     for (Renderer* r : tempR)
         delete r;
    
    return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions
    glViewport(0, 0, scrn_width = width, scrn_height = height);

    *gCameraProjection = glm::perspective(glm::radians(camera.Zoom), camera.Aspect = (float)width / (float)height, 0.01f, 1000.0f);	//  1.0472 radians = 60 degrees
}
