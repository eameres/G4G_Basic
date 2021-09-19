
#include "shader_s.h"

#pragma once

class Material {
public:
    Shader* myShader;
    GLint texture;

    glm::vec4 color;

public:
    Material(Shader* _shader, GLint _texture, glm::vec4 _color) {
        myShader = _shader;
        texture = _texture;
        color = _color;
    }

    void use() {

        myShader->use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glUniform4fv(glGetUniformLocation(myShader->ID, "ourColor"), 1, glm::value_ptr(color));
    }
};

class Renderer {

protected:
    unsigned int VBO = 0, VAO = 0, EBO = 0;
    int indexCount;

    glm::mat4 modelMatrix;

    Material* myMaterial;

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
    virtual void render(glm::mat4 vMat, glm::mat4 pMat, double deltaTime, glm::vec3 lightLoc);
};

class nCubeRenderer : public Renderer {
    // ------------------------------------------------------------------
public:
    nCubeRenderer(Material*, glm::mat4 m);
};

class objMesh : public Renderer {
public:
    objMesh(Material*, glm::mat4 m);
};

class torus : public Renderer {
public:
    torus(Material*, glm::mat4 m);
};

class CubeRenderer : public Renderer {
public:
    CubeRenderer(Material*, glm::mat4 m);
};

class QuadRenderer : public Renderer {
public:
    QuadRenderer(Material*, glm::mat4 m);
};

class skybox : public Renderer {
public:
    skybox(Material*, glm::mat4 m);
public :
    void render(glm::mat4 vMat, glm::mat4 pMat, double deltaTime, glm::vec3 lightLoc);
};

class particleCube : public Renderer {
public:
    glm::mat4* iModelMatrices;
public:
    particleCube(Material*, glm::mat4 m);
public:
    void render(glm::mat4 vMat, glm::mat4 pMat, double deltaTime, glm::vec3 lightLoc);
private:
    void setupIMatrices(void);
};

