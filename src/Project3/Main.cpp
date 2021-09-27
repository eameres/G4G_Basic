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
#include <filesystem>

#include "shader_s.h"
#include "ImportedModel.h"

#include "camera.h"
#include "renderer.h"
#include "textures.h"
#include "SceneGraph.h"

#include "drawImGui.hpp"

//glm::mat4 pMat; // perspective matrix
//glm::mat4 vMat; // view matrix
//glm::vec3 cPos;

glm::mat4 *gCameraProjection; // perspective matrix

Camera camera;

// settings
unsigned int scrn_width = 1280;
unsigned int scrn_height = 720;

unsigned int texture[] = { 0,1,2,3 };

#pragma warning( disable : 26451 )

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

unsigned int framebuffer;
unsigned int textureColorbuffer;
unsigned int textureDepthbuffer;

void setupFrameBuffer() {
    // framebuffer configuration
    // -------------------------

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // create a color attachment texture
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, scrn_width, scrn_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, scrn_width, scrn_height); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
unsigned int depthMapFBO;
unsigned int depthMap;
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

void setupDepthMap() {
    // configure depth map FBO
    // -----------------------
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void cubeOfCubes(RenderContext *RC, Material *m1, Material *m2) 
{
    for (int i = -1; i < 2; i++)
        for (int j = -1; j < 2; j++)
            for (int k = -1; k < 2; k++) {
                glm::mat4 xf;

                float x = i, y = j, z = k;
                xf = glm::rotate(glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(x * .33, y * .33, k * .33)), glm::vec3(.20)), .8f, glm::vec3(1, 1, 0));
                Renderer* nC;
                if ((j == 0) && (k == 0) && (i == 0))
                    nC = new SphereRenderer(m1, xf);
                else if ((j == k) || (i == j))
                    nC = new nCubeRenderer(m2, xf);
                else
                    nC = new nCubeRenderer(m1, xf);
                RC->addRenderer(nC);
            }
}
int main()
{
    RenderContext RC;
    treeNode* base, * l1 = NULL, *l2 = NULL, *l3 = NULL;

    gCameraProjection = &RC.camera.projection;  // hack for the viewport callback function to set the projection matrix

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
    glfwWindowHint(GLFW_SCALE_TO_MONITOR,GL_TRUE);
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

    // needed if we are drawing directly to the window (for now)
    //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

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
    
    glGetIntegerv( GL_VIEWPORT, viewportDims );
    scrn_width = viewportDims[2];
    scrn_height = viewportDims[3];
    
    setupTextures(texture);
    setupFrameBuffer();
    setupDepthMap();

    // 
    // set up the perspective projection for the camera and the light
    //
    RC.camera.projection = glm::perspective(glm::radians(camera.Zoom), camera.Aspect = ((float)scrn_width / (float)scrn_height), 0.01f, 1000.0f);    //  1.0472 radians = 60 degrees
    RC.light.projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 1.0f, 7.5f);
    RC.light.position = glm::vec4(-4.0f, 2.0f, 0.0f, 1.0f);

    RC.camera.position = glm::vec4(0, 0, -5, 1.0f);
    RC.camera.target = glm::vec4(0, 0, 0, 1.0f);

    std::vector<Shader*> shaders;   // pave the way for "scene" rendering
    
    // declare and intialize our base shader
    Shader *baseShader = new Shader("data/vertex.lgsl", "data/fragment.lgsl","base");
    shaders.push_back(baseShader);
    
    // declare and intialize shader with colored vertices
    Shader *colShader = new Shader("data/vertColors.lgsl", "data/fragColors.lgsl","colored");
    shaders.push_back(colShader);
    
    // declare and intialize shader with texture(s)
    Shader *texturedEnvShader = new Shader("data/vertTexture.lgsl", "data/fragTexture.lgsl", "textured");
    shaders.push_back(texturedEnvShader);
    
    // declare and intialize shader with ADS lighting
    Shader *phongShadowedShader = new Shader("data/vFlatLit.lgsl", "data/fFlatLit.lgsl", "FlatLit");
    shaders.push_back(phongShadowedShader);

    // declare and intialize skybox shader
    Shader *skyShader = new Shader("data/vSky.lgsl", "data/fSky.lgsl", "Skybox");
    shaders.push_back(skyShader);

    Shader *particleShader = new Shader("data/vParticle.lgsl", "data/fParticle.lgsl", "Particle"); // declare and intialize skybox shader
    shaders.push_back(particleShader);
    
    Shader *depthShader = new Shader("data/vDepth.lgsl", "data/fDepth.lgsl", "Depth");
    shaders.push_back(depthShader);

    Shader* postShader = new Shader("data/vPost.lgsl", "data/fPost.lgsl", "PostProcessing");
    shaders.push_back(postShader);
    
    Material* white = new Material(baseShader, -1, glm::vec4(1.0, 1.0, 1.0, 1.0));
    Material* coloredVerts = new Material(colShader, -1, glm::vec4(1.0, 1.0, 0.0, 1.0));
    Material* pMaterial = new Material(particleShader, -1, glm::vec4(1.0,0.0,0.0,1.0));
    Material* litMaterial = new Material(phongShadowedShader, texture[0], depthMap, true);

    Material* shuttleMaterial = new Material(texturedEnvShader,texture[2], texture[3]);
    Material* checkers = new Material(texturedEnvShader,texture[0], texture[3]);

    Material* background = new Material(skyShader, texture[3], glm::vec4(-1.0));
    Material* offScreenMaterial = new Material(postShader, textureColorbuffer, glm::vec4(1.0, 1.0, 0.0, 1.0));
    Material* depthMaterial = new Material(depthShader, -1, glm::vec4(1.0, 1.0, 0.0, 1.0));
    
    nCubeRenderer cube(litMaterial, glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f)));
    RC.addRenderer(&cube);

    nCubeRenderer lightCube(white, glm::translate(glm::mat4(1.0f), RC.light.position));
    RC.addRenderer(&lightCube);

    SphereRenderer sphere(litMaterial, glm::translate(glm::mat4(0.5f), glm::vec3(0.0f, -2.0f, 0.0f)));
    RC.addRenderer(&sphere);
    
    ObjRenderer sponza("data/sponza.obj_", litMaterial, glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3.0f, 0.0f)), glm::vec3(.02f)));
    RC.addRenderer(&sponza);

    ObjRenderer shuttle("data/shuttle.obj_", shuttleMaterial, glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f)), glm::vec3(2.0f)));
    RC.addRenderer(&shuttle);
    
    l1 = RC.addChild(glm::mat4(1));

    ParticleRenderer cubeParticles(pMaterial, glm::translate(glm::mat4(.025f), glm::vec3(0.0f, 0.0f, 0.0f)));
    //RC.addRenderer(&cubeParticles);

    QuadRenderer frontQuad(checkers, glm::mat4(1.0f)); // our "first quad"
    RC.addRenderer(&frontQuad); // add it to the render list

    QuadRenderer backQuad(coloredVerts, glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(1.0, 0.0, 0.0))); // our "second quad"
    RC.addRenderer(&backQuad); // add it to the render list

    l2 = RC.addChild(glm::mat4(1));

    TorusRenderer torus(litMaterial, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)));
    RC.addRenderer(&torus);

    RC.getParent();
    l3 = RC.addChild(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)));
    
    //cubeOfCubes(&RC,checkers,litMaterial);
    

    // skybox is special and doesn't belong to the SceneGraph
    SkyboxRenderer mySky(background, glm::mat4(1.0f)); // our "skybox"

    // full screen quad needs to be scaled by 2 since the quad is designed around [ -0.5, +0.5 ]
    QuadRenderer fQuad(offScreenMaterial, glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f))); // our fullScreen Quad
    //fQuad.enabled = false;

    // render loop
    // -----------

    
    //glViewport(0, 0, scrn_width, scrn_height);


    double lastTime = glfwGetTime();

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
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

        // animating the three levels of our hierarchy
        if (l1 != NULL) l1->setXform(glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(1, 0, 0.0f)));
        if (l2 != NULL)l2->setXform(glm::translate(glm::rotate(glm::mat4(1.0f), (float)glfwGetTime() * 2.0f, glm::vec3(0, 1, 0.0f)), glm::vec3(0, 1, 0)));
        if (l3 != NULL)l3->setXform(glm::translate(glm::rotate(glm::mat4(1.0f), (float)glfwGetTime() * -2.0f, glm::vec3(0, 0, 1.0f)), glm::vec3(0.0f, 0.0f, -1.0f)));

        // moving light source, must set it's position...
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(1, 0, 0.0f));
        RC.light.position = rotate * glm::vec4(-4.0f, 2.0f, 0.0f, 1.0f);

        // show a cube from that position
        lightCube.modelMatrix = glm::scale(glm::translate(glm::mat4(1.0f), RC.light.position), glm::vec3(.25f));

        {
            // first we do the "shadow pass"  really just for creating a depth buffer from the light's perspective
           
            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
            glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_DEPTH_BUFFER_BIT);

            // render from the lights perspective and position to create the shadowMap
            RC.renderFrom(RC.light, deltaTime);
        }
        {
            // do the "normal" drawing
            glViewport(0, 0, scrn_width, scrn_height);

            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); // offscreen
            //glBindFramebuffer(GL_FRAMEBUFFER, 0);         // onscreen

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glDisable(GL_DEPTH_TEST);
            mySky.render(glm::mat4(glm::mat3(glm::lookAt(RC.camera.position, RC.camera.target, RC.camera.up))), RC.camera.projection, deltaTime, RC.light.position, RC.camera.position);
            
            glEnable(GL_DEPTH_TEST);

            // render from the cameras position and perspective  this may or may not be offscreen 
            RC.renderFrom(RC.camera, deltaTime);
        }
        {
            // assuming the previous was offscreen, we now need to draw a quad with the results to the screen!
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glDisable(GL_DEPTH_TEST);

            fQuad.render(glm::mat4(1.0f), glm::mat4(1.0f), deltaTime, RC.light.position, RC.camera.position);
        }
        // draw imGui over the top
        drawIMGUI(shaders, &frontQuad,Material::materialList,&cubeParticles,&RC);

        glfwSwapBuffers(window);
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    
    deleteTextures(texture);

     for (Shader* s : shaders)
        delete s;

     static std::vector<Material*> temp = Material::materialList;

     for (Material* m : temp)
         delete m;
    
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
