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

class QuadRenderer {
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[12] = {
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
    };
    unsigned int indices[6] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

public:
    unsigned int VBO, VAO, EBO;

    glm::mat4 modelMatrix;

    unsigned int transformLoc;

    Shader *myShader;

    void initRenderer(Shader *shader,glm::mat4 m) {

        modelMatrix = m;

        myShader = shader;

        transformLoc = glGetUniformLocation(myShader->ID, "m");

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
        // don't be tempted to do this --->  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
        // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
        glBindVertexArray(0);
    }

    void rotate(float axis[], float angle) {
        modelMatrix = glm::mat4(1.0f);

        modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(axis[0],axis[1],axis[2]));
    }

    void render() {
        myShader->use();
        transformLoc = glGetUniformLocation(myShader->ID, "m");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
};

using namespace std;
/*
char vtext[1024 * 8] =
"#version 330 core\n\n"
"layout (location = 0) in vec3 aPos;\n\n"
"void main()\n"
"{\n"
//"   gl_Position = vec4(aPos, 1.0);\n"
"if (gl_VertexID == 0) gl_Position = vec4(0.25, -0.25, 0.0, 1.0);\n"
"else if (gl_VertexID == 1) gl_Position = vec4(-0.25, -0.25, 0.0, 1.0);\n"
"else gl_Position = vec4(0.25, 0.25, 0.0, 1.0);\n"
"}\n"; // fragment shader source code


char ftext[1024 * 8] =
"#version 330 core\n\n"
"out vec4 FragColor;\n\n"
"uniform vec4 ourColor;\n\n"
"void main()\n"
"{\n"
"   FragColor = ourColor; //vec4(1.0f,0f,0f, 1.0f);\n"
"}\n"; // vertex shader source code


string readFile(const char* filePath) {
    string content;
    ifstream fileStream(filePath, ios::in);
    if (fileStream.fail())
        return (string)"empty";

    string line = "";
    while (!fileStream.eof()) {
        getline(fileStream, line);
        content.append(line + "\n");
    }
    fileStream.close();
    return content;
}

void saveShaders() {
    std::ofstream myfile;

    myfile.open("data/vertex.lgsl");
    myfile << vtext;
    myfile.close();

    myfile.open("data/fragment.lgsl");
    myfile << ftext;
    myfile.close();
}
*/
#pragma warning( disable : 26451 )
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

unsigned int texture;

// image buffer used by raster drawing basics.cpp
extern unsigned char imageBuff[512][512][3];

int myTexture();

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


float axis[3] = { 0.0f,0.0f,1.0f };
float angle = 45.0f;

void drawIMGUI(Shader *ourShader) {
    // Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static float f = 0.0f;

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

        
        if (ImGui::Button("Use Shaders", ImVec2(100, 20)))
            ourShader->reload();

        ImGui::SameLine(115);

        if (ImGui::Button("Save Shaders", ImVec2(120, 20)))
        {
            ourShader->saveShaders();
        }

        ImGui::InputFloat3("Axis", axis,"%.2f");
        ImGui::SameLine();
        ImGui::SliderAngle("Angle", &angle,-180.0f,180.0f);

        ImGui::Image((void*)(intptr_t)texture, ImVec2(512, 512));

        ImGui::End();

        // IMGUI Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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

    Shader ourShader("data/vertex.lgsl", "data/fragment.lgsl");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    //strcpy_s(ftext, readFile("data/fragment.lgsl").c_str());
    //strcpy_s(vtext, readFile("data/vertex.lgsl").c_str());

    // build and compile our shader program
    // ------------------------------------
    ourShader.reload();

    myTexture();

    setupTextures();

    QuadRenderer myQuad;

    myQuad.initRenderer(&ourShader, glm::mat4(1.0f));

    // render loop
    // -----------

    while (!glfwWindowShouldClose(window))
    {
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

        myQuad.render();

        // draw stuff here!

        drawIMGUI(&ourShader);

        myQuad.rotate(axis,angle);

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
}
