#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui_impl_opengl3.h>

#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <filesystem>

#include "renderer.h"
#include "ImportedModel.h"

void renderer::render(glm::mat4 vMat, glm::mat4 pMat, double deltaTime, glm::vec3 lightLoc)
{ // here's where the "actual drawing" gets done

    glm::mat4 mvp;

    myShader->use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, myShader->textNum);

    //rotate(glm::value_ptr(glm::vec3(0.0f, 0.0f, 1.0f)), deltaTime); // easter egg!  rotate incrementally with delta time

    mvp = pMat * vMat * modelMatrix;

    glUniformMatrix4fv(glGetUniformLocation(myShader->ID, "m"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myShader->ID, "v"), 1, GL_FALSE, glm::value_ptr(vMat));
    glUniformMatrix4fv(glGetUniformLocation(myShader->ID, "p"), 1, GL_FALSE, glm::value_ptr(pMat));

    glUniformMatrix4fv(glGetUniformLocation(myShader->ID, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

    glUniform3f(glGetUniformLocation(myShader->ID, "cPos"), -glm::vec3(vMat[3])[0], -glm::vec3(vMat[3])[1], -glm::vec3(vMat[3])[2]);
    glUniform3fv(glGetUniformLocation(myShader->ID, "lPos"), 1, glm::value_ptr(lightLoc));

    glBindVertexArray(VAO);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    if (indexCount < 0) {
        glDrawArrays(GL_TRIANGLES, 0, -indexCount);
    }
    else
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

nCubeRenderer :: nCubeRenderer(Shader* shader, glm::mat4 m)
{
    float vertices[216] = {
           0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
           0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
          -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
          -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
          -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
           0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

          -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
           0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
           0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
           0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
          -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
          -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

          -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
          -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
          -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
          -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
          -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
          -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

           0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
           0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
           0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
           0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
           0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
           0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,

          -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
           0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
           0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
           0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
          -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
          -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

           0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
           0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
          -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,

          -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
          -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
           0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f
    };
    // set up vertex data (and buffer(s)) and configure vertex attributes
    modelMatrix = m;

    myShader = shader;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    // vertex buffer object, simple version, just coordinates

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    indexCount = -36; // since we have no indices, we tell the render call to use raw triangles by setting indexCount to -numVertices

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);
}

objMesh:: objMesh(Shader* shader, glm::mat4 m)
{
    // set up vertex data (and buffer(s)) and configure vertex attributes
    modelMatrix = m;

    myShader = shader;

    glGenVertexArrays(1, &VAO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    ImportedModel myModel("data/shuttle.obj_");
    //ImportedModel myModel("data/teapot.obj_");
    //ImportedModel myModel("data/ironman.obj");

    std::vector<glm::vec3> vert = myModel.getVertices();
    std::vector<glm::vec2> tex = myModel.getTextureCoords();
    std::vector<glm::vec3> norm = myModel.getNormals();

    std::vector<float> vbovalues;
    std::vector<int> indices;

    for (int i = 0; i < myModel.getNumVertices(); i++) {
        vbovalues.push_back((vert[i]).x);
        vbovalues.push_back((vert[i]).y);
        vbovalues.push_back((vert[i]).z);
        vbovalues.push_back(norm[i].x);
        vbovalues.push_back(norm[i].y);
        vbovalues.push_back(norm[i].z);
        vbovalues.push_back(0.0f);
        vbovalues.push_back(0.0f);
        vbovalues.push_back(1.0f);
        vbovalues.push_back((tex[i]).s);
        vbovalues.push_back((tex[i]).t);
        indices.push_back(i);
    }

    indexCount = indices.size();

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vbovalues.size() * 4, &vbovalues[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * 4, &indices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal vector attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // color attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture coord attribute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
};

QuadRenderer:: QuadRenderer(Shader* shader, glm::mat4 m)
{
    // ------------------------------------------------------------------
    unsigned int indices[6] = {  // note that we start from 0!
        3, 1, 0,  // first Triangle
        3, 2, 1   // second Triangle
    };
    float vertices[24] = {
         0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  // bottom left
        -0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 1.0f  // top left 
    };

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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

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
};

CubeRenderer:: CubeRenderer(Shader* shader, glm::mat4 m)
{
    float vertices[108] = {
           -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
           1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,

            1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f, -1.0f, 
            1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f, -1.0f,

            1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 
            -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f,

           -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, 
           -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f,

           -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f, 
           1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,

           -1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f, 
           1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f
    };
    // set up vertex data (and buffer(s)) and configure vertex attributes
    modelMatrix = m;

    myShader = shader;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    // vertex buffer object, simple version, just coordinates

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    indexCount = -36; // since we have no indices, we tell the render call to use raw triangles by setting indexCount to -numVertices

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);
};

torus:: torus(Shader* shader, glm::mat4 m)
{
    // set up vertex data (and buffer(s)) and configure vertex attributes
    modelMatrix = m;

    myShader = shader;

    glGenVertexArrays(1, &VAO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    std::vector<float> verts;
    std::vector<int> indices;

    int sides = 40, cs_sides = 20;
    float radius = .3;
    float cs_radius = .1;

    constexpr float twoPi = glm::pi<float>() * 2.0f;

    int numVertices = sides * cs_sides;
    int numIndices = (2 * ((sides + 1) * cs_sides) + cs_sides);

    float angleincs = twoPi / sides;
    float cs_angleincs = twoPi / cs_sides;
    float currentradius, zval;

    //calculating the vertex array
    for (float j = 0; j < twoPi + cs_angleincs; j += cs_angleincs)
    {
        currentradius = radius + (cs_radius * cosf(j));
        zval = cs_radius * sinf(j);

        //int index = (m * sides);
        for (float i = 0; i < twoPi + angleincs; i += angleincs)
        {
            verts.push_back(currentradius * cosf(i)); // x 
            verts.push_back(currentradius * sinf(i)); // y
            verts.push_back(zval);                    // z

            verts.push_back(currentradius * cosf(i) - radius * cos(i));
            verts.push_back(currentradius * sinf(i) - radius * sin(i));
            verts.push_back(zval);

            verts.push_back(1.0);
            verts.push_back(0.0);
            verts.push_back(0.0);

            float u = (float)i * 4.0f / twoPi;
            float v = ((float)j * 4.0f) / twoPi;

            verts.push_back(u);
            verts.push_back(v);
        }
    }
    int nextrow = sides + 1;

    //calculating the index array
    for (int i = 0, n = 0; i < cs_sides; i++) {
        for (int j = 0; j < sides; j++) {
            {
                //indices.push_back(i * nextrow + j);
                //indices.push_back((i + 1) * nextrow + j);
                //indices.push_back(i * nextrow + (j + 1));

                indices.push_back(i * nextrow + (j + 1));
                indices.push_back((i + 1) * nextrow + j);
                indices.push_back(i * nextrow + j);}

            {
                //indices.push_back((i + 1) * nextrow + j);
                //indices.push_back((i + 1) * nextrow + (j + 1));
                //indices.push_back(i * nextrow + (j + 1));

                indices.push_back(i * nextrow + (j + 1));
                indices.push_back((i + 1) * nextrow + (j + 1));
                indices.push_back((i + 1) * nextrow + j);
            }
        }
    }

    indexCount = indices.size();

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * 4, &verts[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * 4, &indices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal vector attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // color attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture coord attribute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindVertexArray(0);
};

unsigned int loadCubemap(std::vector<std::string> faces);

skybox::skybox(Shader* shader, glm::mat4 m)
{
    float cubeVertexPositions[108] =
    { -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f, -1.0f,
        1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f, -1.0f,
        1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f
    };

    // set up vertex data (and buffer(s)) and configure vertex attributes
    modelMatrix = m;

    myShader = shader;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertexPositions), cubeVertexPositions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // load textures
    // -------------
    std::vector<std::string> faces
    {
        "data/cubemap/xp.jpg",
        "data/cubemap/xn.jpg",
        "data/cubemap/yp.jpg",
        "data/cubemap/yn.jpg",
        "data/cubemap/zp.jpg",
        "data/cubemap/zn.jpg",
    };
    shader->textNum = loadCubemap(faces);
}

void skybox::render(glm::mat4 vMat, glm::mat4 pMat, double deltaTime, glm::vec3 lightLoc)
{ // here's where the "actual drawing" gets done

    glm::mat4 mvp;

    myShader->use();
    myShader->setInt("skybox", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, myShader->textNum);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);	// cube is CW, but we are viewing the inside
    glDisable(GL_DEPTH_TEST);

    mvp = pMat * vMat;

    glUniformMatrix4fv(glGetUniformLocation(myShader->ID, "m"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myShader->ID, "v"), 1, GL_FALSE, glm::value_ptr(glm::mat4(glm::mat3(vMat))));
    glUniformMatrix4fv(glGetUniformLocation(myShader->ID, "p"), 1, GL_FALSE, glm::value_ptr(pMat));

    glUniformMatrix4fv(glGetUniformLocation(myShader->ID, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glEnable(GL_DEPTH_TEST);
}

void particleCube::setupIMatrices() {
    unsigned int amount = 100;
    iModelMatrices = new glm::mat4[amount];
    for (unsigned int i = 0; i < amount; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model, glm::vec3((rand() % 100) - 50, (rand() % 100) - 50, (rand() % 100) - 50));
        model = glm::rotate(model, (float)rand(), glm::vec3((rand() % 100) - 50, (rand() % 100) - 50, (rand() % 100) - 50));

        iModelMatrices[i] = model;
    }

    glBindVertexArray(VAO);

    // configure instanced array
    // -------------------------
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &iModelMatrices[0], GL_STATIC_DRAW);

    // set transformation matrices as an instance vertex attribute (with divisor 1)
    // -----------------------------------------------------------------------------------------------------------------------------------

    // set attribute pointers for matrix (4 times vec4)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
    glVertexAttribDivisor(7, 1);
}

particleCube::particleCube(Shader* shader, glm::mat4 m) {

    
    float vertexPositions[108] = 
    {
        // back face
        1.0f, -1.0f, -1.0f,     // bottom right
        -1.0f, -1.0f, -1.0f,    // bottom left
        -1.0f,  1.0f, -1.0f,    // top left

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f,  1.0f, 
        1.0f, -1.0f, -1.0f,

        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,


        1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,

        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,


        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,


        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,


        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f

    };

    float vertexNormals[108] = {
         0.0f,  0.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  0.0f, -1.0f,
         1.0f,  0.0f,  0.0f,   1.0f,  0.0f,  0.0f,   1.0f,  0.0f,  0.0f,   1.0f,  0.0f,  0.0f,   1.0f,  0.0f,  0.0f,   1.0f,  0.0f,  0.0f,
         0.0f,  0.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f,  0.0f,  1.0f,
        -1.0f,  0.0f,  0.0f,  -1.0f,  0.0f,  0.0f,  -1.0f,  0.0f,  0.0f,  -1.0f,  0.0f,  0.0f,  -1.0f,  0.0f,  0.0f,  -1.0f,  0.0f,  0.0f,
         0.0f, -1.0f,  0.0f,   0.0f, -1.0f,  0.0f,   0.0f, -1.0f,  0.0f,   0.0f, -1.0f,  0.0f,   0.0f, -1.0f,  0.0f,   0.0f, -1.0f,  0.0f,
         0.0f,  1.0f,  0.0f,   0.0f,  1.0f,  0.0f,   0.0f,  1.0f,  0.0f,   0.0f,  1.0f,  0.0f,   0.0f,  1.0f,  0.0f,   0.0f,  1.0f,  0.0f,
    };
    float texCoords[72] = {
        1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
        1.0f, 0.0f,  0.0f, 0.0f,  1.0f, 1.0f,  0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
        1.0f, 0.0f,  0.0f, 0.0f,  1.0f, 1.0f,  0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
        1.0f, 0.0f,  0.0f, 0.0f,  1.0f, 1.0f,  0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
        1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
        1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f
    };

    
    modelMatrix = m;
    myShader = shader;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VTO = 0, VNO = 0;

    // position attribute
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // normal attribute
    glGenBuffers(1, &VNO);
    glBindBuffer(GL_ARRAY_BUFFER, VNO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexNormals), vertexNormals, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    indexCount = -36; // since we have no indices, we tell the render call to use raw triangles by setting indexCount to -numVertices
    
    // texture attribute
    glGenBuffers(1, &VTO);
    glBindBuffer(GL_ARRAY_BUFFER, VTO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(2);

    setupIMatrices();
    glBindVertexArray(0);
}

void particleCube::render(glm::mat4 vMat, glm::mat4 pMat, double deltaTime, glm::vec3 lightLoc)
{ // here's where the "actual drawing" gets done

    glm::mat4 mvp;

    myShader->use();

    glUniform3f(glGetUniformLocation(myShader->ID, "cPos"), -glm::vec3(vMat[3])[0], -glm::vec3(vMat[3])[1], -glm::vec3(vMat[3])[2]);
    glUniform3fv(glGetUniformLocation(myShader->ID, "lPos"), 1, glm::value_ptr(lightLoc));

    mvp = pMat * vMat * modelMatrix;

    glUniformMatrix4fv(glGetUniformLocation(myShader->ID, "m"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myShader->ID, "v"), 1, GL_FALSE, glm::value_ptr(vMat));
    glUniformMatrix4fv(glGetUniformLocation(myShader->ID, "p"), 1, GL_FALSE, glm::value_ptr(pMat));

    glUniformMatrix4fv(glGetUniformLocation(myShader->ID, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glBindVertexArray(VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 100);
}