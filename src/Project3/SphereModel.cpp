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

#include "SphereModel.h"

SphereModel::SphereModel(Material* material, glm::mat4 m)
{
    Renderer::name = "Sphere";
    Sphere mySphere;
    // set up vertex data (and buffer(s)) and configure vertex attributes
    setXForm( m );

    myMaterial = material;

    glGenVertexArrays(1, &VAO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glGenBuffers(numVBOs = 2, VBO);
    glGenBuffers(1, &EBO);

    int numIndices = mySphere.getIndices().size();
    //
    // Notice!!!  Since this is a unit sphere, 
    // we can use the vertex coordinates as the vertex normals !!!
    //
    indexCount = numIndices;

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, mySphere.getVerts().size() * 4, mySphere.getVerts().data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mySphere.getIndices().size() * 4, mySphere.getIndices().data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal vector attribute  :  REUSE THE VERTEX COORDINATES !!!
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    // texture coord attribute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(3);

    setupColorAttrib();

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindVertexArray(0);
};


Sphere::Sphere() {
    init(34);
}

Sphere::Sphere(int prec) {
    init(prec);
}

float Sphere::toRadians(float degrees) { return (degrees * 2.0f * 3.14159f) / 360.0f; }

void Sphere::init(int prec) {
    numVertices = (prec + 1) * (prec + 1);
    numIndices = prec * prec * 6;

    // calculate triangle vertices
    for (int i = 0; i <= prec; i++) {
        float y = (float)cos(toRadians(180.0f - i * 180.0f / prec));

        for (int j = 0; j <= prec; j++) {
            float x = -(float)cos(toRadians(j * 360.0f / prec)) * (float)abs(cos(asin(y)));
            float z = (float)sin(toRadians(j * 360.0f / (float)(prec))) * (float)abs(cos(asin(y)));

            verts.push_back(x);
            verts.push_back(y);
            verts.push_back(z);

            verts.push_back((float)j / prec);
            verts.push_back((float)i / prec);
        }
    }

    int nextrow = prec + 1;

    //calculating the index array
    for (int i = 0, n = 0; i < prec; i++) {
        for (int j = 0; j < prec; j++) {
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
}

int Sphere::getNumVertices() { return numVertices; }
int Sphere::getNumIndices() { return numIndices; }
std::vector<int> Sphere::getIndices() { return indices; }
std::vector<float> Sphere::getVerts() { return verts; }

