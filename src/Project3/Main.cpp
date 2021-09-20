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

#include "drawImGui.hpp"

glm::mat4 pMat; // perspective matrix
glm::mat4 vMat; // view matrix

Camera camera;

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main()
{
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
    glfwWindowHint(GLFW_SCALE_TO_MONITOR,GL_FALSE);
#endif
    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Graphics4Games Fall 2021", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

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

    setupTextures(texture);
    setupFrameBuffer();

    std::vector<Shader*> shaders;   // pave the way for "scene" rendering
    std::vector<Renderer*> renderers;
    
    // declare and intialize our base shader
    Shader ourShader("data/vertex.lgsl", "data/fragment.lgsl","base");
    shaders.push_back(&ourShader);
    
    // declare and intialize shader with colored vertices
    Shader tShader("data/vertColors.lgsl", "data/fragColors.lgsl","colored");
    shaders.push_back(&tShader);
    
    // declare and intialize shader with texture(s)
    Shader txShader("data/vertTexture.lgsl", "data/fragTexture.lgsl", "textured");
    shaders.push_back(&txShader);
    
    // declare and intialize shader with ADS lighting
    Shader flShader("data/vFlatLit.lgsl", "data/fFlatLit.lgsl", "FlatLit");
    shaders.push_back(&flShader);

    // declare and intialize skybox shader
    Shader skyShader("data/vSky.lgsl", "data/fSky.lgsl", "Skybox");
    shaders.push_back(&skyShader);

    Shader particleShader("data/vParticle.lgsl", "data/fParticle.lgsl", "Particle"); // declare and intialize skybox shader
    shaders.push_back(&particleShader);
    
    Material white(&ourShader, -1, glm::vec4(1.0, 1.0, 1.0, 1.0));
    Material coloredVerts(&tShader, -1, glm::vec4(1.0, 1.0, 0.0, 1.0));
    Material pMaterial(&particleShader, -1, glm::vec4(1.0,0.0,0.0,1.0));
    Material litMaterial(&flShader, -1, glm::vec4(0.0, 1.0, 1.0, 1.0));
    Material background(&skyShader,texture[3], glm::vec4(-1.0));
    Material shuttleMaterial(&txShader,texture[2], texture[3]);
    Material checkers(&txShader,texture[0], texture[3]);
    Material offScreenMaterial(&txShader, textureColorbuffer, glm::vec4(1.0, 1.0, 0.0, 1.0));
    
    SkyboxRenderer mySky(&background, glm::mat4(1.0f)); // our "first quad"
    renderers.push_back(&mySky); // add it to the render list

    QuadRenderer myQuad(&checkers, glm::mat4(1.0f)); // our "first quad"
    renderers.push_back(&myQuad); // add it to the render list   

    QuadRenderer myQuad2(&coloredVerts, glm::rotate(glm::mat4(1.0f), glm::pi<float>(),glm::vec3(1.0,0.0,0.0))); // our "second quad"
    renderers.push_back(&myQuad2); // add it to the render list
    
    glm::mat4 tf2 =glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f));
    tf2 = glm::scale(tf2, glm::vec3(2.0f, 2.0f, 2.0f));

    ObjRenderer shuttle(&shuttleMaterial, tf2);
    renderers.push_back(&shuttle);

    nCubeRenderer myCube2(&litMaterial, glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f)));
    renderers.push_back(&myCube2);

    TorusRenderer myTorus(&litMaterial, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    renderers.push_back(&myTorus);
    
    Renderer *pCube = new ParticleRenderer(&pMaterial, glm::translate(glm::mat4(.025f), glm::vec3(0.0f, 0.0f, 0.0f)));
    renderers.push_back(pCube);
   
    QuadRenderer fQuad(&offScreenMaterial, glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f))); // our fullScreen Quad
    
    nCubeRenderer lightCube(&white, glm::mat4(0.33f));
    renderers.push_back(&lightCube);

    // render loop
    // -----------

    // set up the perspective and the camera
    pMat = glm::perspective(glm::radians(camera.Zoom), camera.Aspect = ((float)SCR_WIDTH / (float)SCR_HEIGHT), 0.01f, 1000.0f);    //  1.0472 radians = 60 degrees

    double lastTime = glfwGetTime();

    glEnable(GL_DEPTH_TEST);

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

        //glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        // render background
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //glClearDepthf(999.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(1, 0, 0.0f));
        glm::vec4 lightPos = rotate * glm::vec4(-2, 0, 2.0, 1.0);

        lightCube.setTranslate((glm::vec3)lightPos);

        // call each of the queued renderers
        for(Renderer *r : renderers)
        {
            r->render(vMat, pMat, deltaTime, lightPos);
        }
        /*
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        fQuad.render(glm::mat4(1.0f), glm::mat4(1.0f), deltaTime, lightPos);
        */
        // draw imGui over the top
        drawIMGUI(shaders, &myQuad,&checkers,(ParticleRenderer *) pCube);

        glfwSwapBuffers(window);
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    
    delete pCube;
    deleteTextures(texture);
    
    return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions
    glViewport(0, 0, width, height);

    pMat = glm::perspective(glm::radians(camera.Zoom), camera.Aspect = (float)width / (float)height, 0.01f, 1000.0f);	//  1.0472 radians = 60 degrees
}
