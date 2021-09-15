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
#include "renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

glm::mat4 pMat; // perspective matrix
glm::mat4 vMat; // view matrix

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

unsigned int texture;

// image buffer used by raster drawing basics.cpp
extern unsigned char imageBuff[512][512][3];

int myTexture();

class QuadRenderer : public renderer {
    // ------------------------------------------------------------------
    float vertices[12] = {
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
    };

protected: 
    unsigned int indices[6] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    public : QuadRenderer(Shader *shader,glm::mat4 m) 
    {
        // set up vertex data (and buffer(s)) and configure vertex attributes
        modelMatrix = m;

        myShader = shader;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(VAO);


        // vertex buffer object, simple version, just coordinates

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // set up the element array buffer containing the vertex indices for the "mesh"
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        indexCount = sizeof(indices) / sizeof(unsigned int);

        // remember: do NOT unbind the EBO while a VAO is active, as the bound element buffer object IS stored in the VAO; keep the EBO bound.
        // don't be tempted to do this --->  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
        // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
        glBindVertexArray(0);
    }
};

#pragma warning( disable : 26451 )

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void setupTextures()
{
    // create textures 
        // -------------------------
    glGenTextures(1, &texture);

    // texture is a buffer we will be generating for pixel experiments
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // load image, create texture and generate mipmaps
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, (const void*)imageBuff);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void drawIMGUI(Shader *ourShader,renderer *myRenderer) {
    // Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        // used to get values from imGui to the model matrix
        static float axis[] = { 0.0f,0.0f,1.0f };
        static float angle = 0.0f;

        static float transVec[] = { 0.0f,0.0f,0.0f };
        static float scaleVec[] = { 1.0f,1.0f,1.0f };

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Graphics For Games");  // Create a window and append into it.

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
        
        ImGui::Text("Vertex Shader");
        ImGui::SameLine();
        ImGui::Text(std::filesystem::absolute("./data/vertex.glsl").u8string().c_str());
        ImGui::InputTextMultiline("Vertex Shader", ourShader->vtext, IM_ARRAYSIZE(ourShader->vtext), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);
        
        ImGui::Text("Fragment Shader" );
        ImGui::SameLine();
        ImGui::Text(std::filesystem::absolute("./data/fragment.glsl").u8string().c_str());
        ImGui::InputTextMultiline("Fragment Shader", ourShader->ftext, IM_ARRAYSIZE(ourShader->ftext), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);

        if (ImGui::Button("reCompile Shaders"))
            ourShader->reload();

        ImGui::SameLine();

        if (ImGui::Button("Save Shaders"))
            ourShader->saveShaders();

        // values we'll use to derive a model matrix
        ImGui::DragFloat3("Translate", transVec,.01f, -3.0f, 3.0f);
        ImGui::InputFloat3("Axis", axis,"%.2f");
        ImGui::SliderAngle("Angle", &angle,-90.0f,90.0f);
        ImGui::DragFloat3("Scale", scaleVec,.01f,-3.0f,3.0f);

        // show the texture that we generated
        ImGui::Image((void*)(intptr_t)texture, ImVec2(64, 64));

        //ImGui::ShowDemoWindow(); // easter agg!  show the ImGui demo window

        ImGui::End();

        // IMGUI Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // factor in the results of imgui tweaks for the next round...
        myRenderer->setXForm(glm::mat4(1.0f));
        myRenderer->translate(transVec);
        myRenderer->rotate(axis, angle);
        myRenderer->scale(scaleVec);
    }
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

    Shader ourShader("data/vertex.lgsl", "data/fragment.lgsl"); // declare and intialize our shader

    myTexture();
    setupTextures();

    // set up the perspective and the camera
    pMat = glm::perspective(1.0472f, ((float)SCR_WIDTH / (float)SCR_HEIGHT), 0.0f, 100.0f);	//  1.0472 radians = 60 degrees
    vMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,0.0f,-3.0f));

    // pave the way for "scene" rendering
    std::vector<renderer*> renderers;

    QuadRenderer myQuad(&ourShader, glm::mat4(1.0f)); // our "first quad"
    
    renderers.push_back(&myQuad); // add it to the render list

    // easter egg!  add another quad to the render list
    /*
    glm::mat4 tf2 =glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 0.0f, 0.0f));
    tf2 = glm::scale(tf2, glm::vec3(0.5f, 0.5f, 0.5f));

    QuadRenderer myQuad2(&ourShader, tf2);
    renderers.push_back(&myQuad2);
    */    

    // render loop
    // -----------

    double lastTime = glfwGetTime();

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

        // render background
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);

        // call each of the queued renderers
        for(renderer *r : renderers)
        {
            r->render(vMat, pMat, deltaTime);
        }

        // draw imGui over the top
        drawIMGUI(&ourShader,&myQuad);

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

    pMat = glm::perspective(1.0472f, (float)width / (float)height, 0.1f, 1000.0f);	//  1.0472 radians = 60 degrees
}
