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

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

glm::mat4 pMat; // perspective matrix
glm::mat4 vMat; // view matrix

Camera camera;

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

unsigned int texture[] = { 0,1,2,3 };

// image buffer used by raster drawing basics.cpp
extern unsigned char imageBuff[512][512][3];

int myTexture();
int RayTracer();

#pragma warning( disable : 26451 )

void framebuffer_size_callback(GLFWwindow* window, int width, int height);


// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int tNum;
    glGenTextures(1, &tNum);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tNum);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    return tNum;
}

void setupTexture(unsigned int tNum,const void *buff,int x,int y, unsigned int fmt) 
{
    glBindTexture(GL_TEXTURE_2D, tNum); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // load image, create texture and generate mipmaps
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, x, y, 0, fmt, GL_UNSIGNED_BYTE, buff);
    glGenerateMipmap(GL_TEXTURE_2D);
}
void setupTextures()
{
    // create textures 
    // -------------------------
    glGenTextures(3, texture);

    myTexture();
    setupTexture(texture[0], (const void*)imageBuff,512,512,GL_RGB);
    // texture is a buffer we will be generating for pixel experiments

    RayTracer();
    setupTexture(texture[1], (const void*)imageBuff,512, 512, GL_RGB);

    // load image, create texture and generate mipmaps
    int width = 0, height = 0, nrChannels = 0;

    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load("data/spstob_1.jpg", &width, &height, &nrChannels, 0);

    setupTexture(texture[2], (const void*)data, width, height, nrChannels == 4 ? GL_RGBA : GL_RGB);
    
    stbi_image_free(data);
    stbi_set_flip_vertically_on_load(false);
}


unsigned int framebuffer;
unsigned int textureColorbuffer;
unsigned int textureDepthbuffer;

void drawIMGUI(std::vector<Shader*> shaders, renderer *myRenderer) {
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

            ImGui::EndTabBar();

            static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
            ImGui::Text("Vertex Shader");
            ImGui::SameLine();
            ImGui::Text(std::filesystem::absolute(gShader->vertexPath).u8string().c_str());
            ImGui::InputTextMultiline("Vertex Shader", gShader->vtext, IM_ARRAYSIZE(gShader->vtext), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);

            ImGui::Text("Fragment Shader");
            ImGui::SameLine();
            ImGui::Text(std::filesystem::absolute(gShader->fragmentPath).u8string().c_str());
            ImGui::InputTextMultiline("Fragment Shader", gShader->ftext, IM_ARRAYSIZE(gShader->ftext), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);

            if (ImGui::Button("reCompile Shaders"))
                gShader->reload();

            ImGui::SameLine();

            if (ImGui::Button("Save Shaders"))
               gShader->saveShaders();
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
        ImGui::InputFloat3("vAxis", v_axis,"%.2f");
        
        if (!autoPan)
            ImGui::SliderAngle("vAngle", &v_angle,-180.0f,180.0f);
        else
            v_angle = fmod(glfwGetTime(), glm::pi<float>() *2.0) - glm::pi<float>();

        ImGui::DragFloat("Zoom", &(camera.Zoom), .5f,12.0f, 120.0f);
        pMat = glm::perspective(glm::radians(camera.Zoom), camera.Aspect, 0.01f, 1000.0f);	//  1.0472 radians = 60 degrees
        
        vMat = glm::mat4(1.0f);
        vMat = glm::translate(glm::mat4(1.0f), -glm::vec3(v_transVec[0],v_transVec[1],v_transVec[2]));
        vMat = glm::rotate(vMat, -v_angle, glm::vec3(v_axis[0], v_axis[1], v_axis[2]));

        for (int i = 0; i < 3; i++)
        {
            ImGui::PushID(i);

            if (ImGui::ImageButton((void*)(intptr_t)texture[i], ImVec2(64, 64)))
                gShader->textNum = texture[i];
            ImGui::PopID();
            ImGui::SameLine();
        }

        //ImGui::ShowDemoWindow(); // easter egg!  show the ImGui demo window

        ImGui::Image((void*)(intptr_t)textureColorbuffer, ImVec2(256, 256));

        ImGui::End();

        // IMGUI Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}


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

    setupTextures();
    setupFrameBuffer();

    std::vector<Shader*> shaders;

    Shader ourShader("data/vertex.lgsl", "data/fragment.lgsl","base"); // declare and intialize our base shader
    shaders.push_back(&ourShader);
    
    Shader tShader("data/vertColors.lgsl", "data/fragColors.lgsl","colored"); // declare and intialize shader with colored vertices
    shaders.push_back(&tShader);
    
    Shader txShader("data/vertTexture.lgsl", "data/fragTexture.lgsl", "textured"); // declare and intialize shader with texture(s)
    shaders.push_back(&txShader);
    
    Shader flShader("data/vFlatLit.lgsl", "data/fFlatLit.lgsl", "FlatLit"); // declare and intialize shader with ADS lighting
    shaders.push_back(&flShader);

    Shader skyShader("data/vSky.lgsl", "data/fSky.lgsl", "Skybox"); // declare and intialize skybox shader
    shaders.push_back(&skyShader);

    Shader particleShader("data/vParticle.lgsl", "data/fParticle.lgsl", "Particle"); // declare and intialize skybox shader
    shaders.push_back(&particleShader);

    txShader.textNum = texture[0];

    // set up the perspective and the camera
    pMat = glm::perspective(glm::radians(camera.Zoom), camera.Aspect = ((float)SCR_WIDTH / (float)SCR_HEIGHT), 0.01f, 1000.0f);	//  1.0472 radians = 60 degrees

    // pave the way for "scene" rendering
    std::vector<renderer*> renderers;

    
    skybox mySky(&skyShader, glm::mat4(1.0f)); // our "first quad"
    renderers.push_back(&mySky); // add it to the render list

    QuadRenderer myQuad(&tShader, glm::mat4(1.0f)); // our "first quad"
    renderers.push_back(&myQuad); // add it to the render list
    
    glm::mat4 tf2 =glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f));
    tf2 = glm::scale(tf2, glm::vec3(2.0f, 2.0f, 2.0f));

    objMesh shuttle(&txShader, tf2);
    renderers.push_back(&shuttle);

    nCubeRenderer myCube2(&flShader, glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f)));
    renderers.push_back(&myCube2);

    torus myTorus(&flShader, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    renderers.push_back(&myTorus);
    
    particleCube myParticle(&particleShader, glm::translate(glm::mat4(.025f), glm::vec3(0.0f, 0.0f, 0.0f)));
    renderers.push_back(&myParticle);

    QuadRenderer fQuad(&txShader, glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f))); // our fullScreen Quad
    // render loop
    // -----------

    double lastTime = glfwGetTime();

    glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_ALWAYS);

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


        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        // render background
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //glClearDepthf(999.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(1, 0, 0.0f));
        glm::vec4 lightPos = rotate * glm::vec4(-2, 0, 5.0, 1.0);

        // call each of the queued renderers
        for(renderer *r : renderers)
        {
            r->render(vMat, pMat, deltaTime, lightPos);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        unsigned int tempText = txShader.textNum;
        txShader.textNum = textureColorbuffer;
        fQuad.render(glm::mat4(1.0f), glm::mat4(1.0f), deltaTime, lightPos);
        txShader.textNum = tempText;
        // draw imGui over the top
        drawIMGUI(shaders, &myQuad);

        glfwSwapBuffers(window);
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
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
