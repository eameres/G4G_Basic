
#include "shader_s.h"

#pragma once
class renderer {

protected:
    unsigned int VBO = 0, VAO = 0, EBO = 0;
    int indexCount;

    glm::mat4 modelMatrix;

    Shader* myShader;

public: void setXForm(glm::mat4 mat)
{
    modelMatrix = mat;
}

public: void rotate(const float axis[], const float angle)
{
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(axis[0], axis[1], axis[2]));
}    

public: void translate(const float trans[])
{
    modelMatrix = glm::translate(modelMatrix, glm::vec3(trans[0], trans[1], trans[2]));
}

public: void scale(const float scale[])
{
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale[0], scale[1], scale[2]));
}

public: 
    void render(glm::mat4 vMat, glm::mat4 pMat, double deltaTime, glm::vec3 lightLoc);
};

class nCubeRenderer : public renderer {
    // ------------------------------------------------------------------
public:
    nCubeRenderer(Shader* shader, glm::mat4 m);
};

class objMesh : public renderer {
public:
    objMesh(Shader* shader, glm::mat4 m);
};

class torus : public renderer {
public:
    torus(Shader* shader, glm::mat4 m);
};

class CubeRenderer : public renderer {
public:
    CubeRenderer(Shader* shader, glm::mat4 m);
};
class QuadRenderer : public renderer {
public:
    QuadRenderer(Shader* shader, glm::mat4 m);
};