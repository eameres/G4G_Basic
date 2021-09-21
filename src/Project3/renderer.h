
#include "shader_s.h"

#pragma once

class Material {
public:
    static std::vector<Material*> Material::materialList;
public:
    Shader* myShader;
    GLint texture;
    GLint envTexture = 0;
    float shine = 0.1f;

    glm::vec4 color;

public:
    Material(Shader* _shader, GLint _texture, glm::vec4 _color) {
        myShader = _shader;
        texture = _texture;
        color = _color;

        materialList.push_back(this);
    }
    Material(Shader* _shader, GLint _texture, GLint _envTexture) {
        myShader = _shader;
        texture = _texture;
        envTexture = _envTexture;
        color = glm::vec4(1.0,1.0,1.0,1.0);

        materialList.push_back(this);
    }

    void use() {

        myShader->use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        myShader->setInt("OurTexture", 0);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envTexture);
        myShader->setInt("EnvTexture", 1);

        glUniform4fv(glGetUniformLocation(myShader->ID, "ourColor"), 1, glm::value_ptr(color));
    }
};

class Renderer {

protected:
    unsigned int VBO[8], VAO = 0, EBO = 0;
    int numVBOs = 1;

    int indexCount;
    float elapsedTime = 0.0f;

    glm::mat4 modelMatrix;

    Material* myMaterial;

    void setupColorAttrib();

public:
    ~Renderer() {
        glDeleteBuffers(numVBOs, VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteVertexArrays(1, &VAO);
    }

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
public: void setTranslate(glm::vec3 trans)
{
    modelMatrix = glm::mat4(glm::mat3(modelMatrix));
    modelMatrix = glm::translate(glm::mat4(1.0), trans) * modelMatrix;
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

class ObjRenderer : public Renderer {
public:
    ObjRenderer(Material*, glm::mat4 m);
};

class TorusRenderer : public Renderer {
public:
    TorusRenderer(Material*, glm::mat4 m);
};

class CubeRenderer : public Renderer {
public:
    CubeRenderer(Material*, glm::mat4 m);
};

class QuadRenderer : public Renderer {
public:
    QuadRenderer(Material*, glm::mat4 m);
};

class SkyboxRenderer : public Renderer {
public:
    SkyboxRenderer(Material*, glm::mat4 m);
public :
    void render(glm::mat4 vMat, glm::mat4 pMat, double deltaTime, glm::vec3 lightLoc);
};

class ParticleRenderer : public Renderer {
public:
    int maxParticles = 25000;
    int instances = 250;
    glm::mat4* iModelMatrices;
    glm::vec3* iModelColors;
public:
    ParticleRenderer(Material*, glm::mat4 m);
public:
    void render(glm::mat4 vMat, glm::mat4 pMat, double deltaTime, glm::vec3 lightLoc);
private:
    void setupIMatrices(void);
};

