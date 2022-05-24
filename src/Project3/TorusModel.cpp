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

TorusModel::TorusModel(Material* material, glm::mat4 m)
{
    Renderer::name = "Torus";
    // set up vertex data (and buffer(s)) and configure vertex attributes
    modelMatrix = m;

    myMaterial = material;

    glGenVertexArrays(1, &VAO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glGenBuffers(numVBOs = 2, VBO);
    glGenBuffers(1, &EBO);

    std::vector<float> verts;
    std::vector<int> indices;

    int sides = 40, cs_sides = 20;
    float radius = .35;
    float cs_radius = .15;

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
                indices.push_back(i * nextrow + (j + 1));
                indices.push_back((i + 1) * nextrow + j);
                indices.push_back(i * nextrow + j); }

            {
                indices.push_back(i * nextrow + (j + 1));
                indices.push_back((i + 1) * nextrow + (j + 1));
                indices.push_back((i + 1) * nextrow + j);
            }
        }
    }

    indexCount = indices.size();

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * 4, &verts[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * 4, &indices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal vector attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // texture coord attribute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);

    setupColorAttrib();

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindVertexArray(0);
};
